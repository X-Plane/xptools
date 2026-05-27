/*
 * Copyright (c) 2026, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "WED_TruckNetworkValidate.h"

#include "WED_RampPosition.h"
#include "WED_TaxiRoute.h"
#include "WED_TaxiRouteNode.h"
#include "WED_TruckDestination.h"
#include "WED_TruckParkingLocation.h"
#include "WED_EnumSystem.h"
#include "WED_Validate.h"

#include "AptDefs.h"
#include "AssertUtils.h"
#include "GISUtils.h"
#include "IGIS.h"

#include <limits>
#include <sstream>
#include <unordered_map>

// Mirrors the direct-distance search radius X-Plane's ground-service code uses
// when picking candidate trucks for a service request.
static constexpr double kXPlaneSearchRadiusMeters = 1500.0;

// Sanity cap when snapping an off-graph entity (TruckParkingLocation, TruckDestination)
// to the nearest 1206 node. Anything farther is treated as "off-network" and excluded
// from the connectivity test. RampPositions get this cap as well, but silently skipped
// rather than warned, since many small airports legitimately have ramps far from any
// service road.
static constexpr double kMaxSnapMeters = 200.0;

namespace
{

// Endpoint nodes of a service-road edge: the two outer "sources" in WED's edge
// representation (intermediate sources are bezier shape points, topologically
// meaningless). Returns nullptr if the edge isn't well-formed or its endpoints
// aren't taxi-route nodes (shouldn't happen in valid data).
std::optional<std::pair<WED_TaxiRouteNode*, WED_TaxiRouteNode*>> edge_endpoints(WED_TaxiRoute * e)
{
    DebugAssert(e != nullptr);
    std::pair<WED_TaxiRouteNode*, WED_TaxiRouteNode*> ret{};
    const int n = e->CountSources();
    if (n < 2)
        return std::nullopt;
    ret.first = dynamic_cast<WED_TaxiRouteNode *>(e->GetNthSource(0));
    ret.second = dynamic_cast<WED_TaxiRouteNode *>(e->GetNthSource(n - 1));
    return ret;
}

class UnionFind
{
public:
    void add(WED_TaxiRouteNode * n)
    {
        DebugAssert(n != nullptr);
        mParent.emplace(n, n);
    }

    bool contains(WED_TaxiRouteNode * n) const { return mParent.find(n) != mParent.end(); }

    // Precondition: n was previously add()-ed and is not null. .at() throws on
    // misuse rather than have operator[] silently insert spurious {n -> nullptr}
    // entries.
    WED_TaxiRouteNode * find(WED_TaxiRouteNode * n)
    {
        DebugAssert(n != nullptr);
        DebugAssert(mParent.find(n) != mParent.end());
        WED_TaxiRouteNode * parent = mParent.at(n);
        while (parent != n)
        {
            WED_TaxiRouteNode * grand = mParent.at(parent);
            mParent.at(n) = grand;            // path compression (one step toward root)
            n = grand;
            parent = mParent.at(n);
        }
        return n;
    }

    void unite(WED_TaxiRouteNode * a, WED_TaxiRouteNode * b)
    {
        DebugAssert(a != nullptr);
        DebugAssert(b != nullptr);
        a = find(a);
        b = find(b);
        if (a != b) mParent.at(a) = b;
    }

    const std::unordered_map<WED_TaxiRouteNode *, WED_TaxiRouteNode *>& nodes() const { return mParent; }

private:
    std::unordered_map<WED_TaxiRouteNode *, WED_TaxiRouteNode *> mParent;
};

// Snap one off-graph entity to its nearest node participating in the truck graph.
// Returns the component representative (find()-ed) and the distance to the snapped
// node.
//
// Precondition: `uf` is non-empty; callers must short-circuit before reaching here
// when there are no truck-network nodes.
struct SnapResult { WED_TaxiRouteNode * component = nullptr; double distance_m = 0.0; };

SnapResult snap_to_graph(const Point2& loc, UnionFind& uf)
{
    DebugAssert(!uf.nodes().empty());
    SnapResult best{ nullptr, std::numeric_limits<double>::infinity() };
    for (auto& kv : uf.nodes())
    {
        Point2 npos;
        kv.first->GetLocation(gis_Geo, npos);
        const double d = LonLatDistMeters(loc, npos);
        if (d < best.distance_m)
        {
            best.distance_m = d;
            best.component  = kv.first;
        }
    }
    if (best.component) best.component = uf.find(best.component);
    return best;
}

struct Source
{
    WED_TruckParkingLocation * who        = nullptr;
    int                        truck_type = 0;        // WED enum (atc_ServiceTruck_*)
    Point2                     loc;
    WED_TaxiRouteNode *        component  = nullptr;
};

struct Sink
{
    WED_Thing *                who        = nullptr;
    std::set<int>              needed_types;          // WED enum values
    Point2                     loc;
    WED_TaxiRouteNode *        component  = nullptr;
    const char *               kind_label = nullptr;  // for the warning text
};

} // namespace

void WED_DoTruckNetworkReachability(
    WED_Airport *                                       apt,
    validation_error_vector&                            msgs,
    std::span<WED_TaxiRoute *            const>         GT_routes,
    std::span<WED_TruckParkingLocation * const>         truck_parking_locs,
    std::span<WED_TruckDestination *     const>         truck_destinations,
    std::span<WED_RampPosition *         const>         ramps)
{
    DebugAssert(apt != nullptr);

    // Degenerate cases are already covered by sibling checks
    // (err_truck_parking_no_ground_taxi_routes, warn_truckroutes_but_no_starts).
    if (GT_routes.empty() || truck_parking_locs.empty()) return;

    // Step 1: build union-find of WED_TaxiRouteNode identities, unioning edge endpoints.
    UnionFind uf;
    for (auto * e : GT_routes)
    {
        if (auto endpoints = edge_endpoints(e); endpoints)
        {
            uf.add(endpoints->first);
            uf.add(endpoints->second);
            uf.unite(endpoints->first, endpoints->second);
        }
    }

    if (uf.nodes().empty())
        return;

    // Step 2: snap consumers and providers to graph components.
    std::vector<Source> sources;
    sources.reserve(truck_parking_locs.size());
    for (auto * p : truck_parking_locs)
    {
        Point2 loc;
        p->GetLocation(gis_Geo, loc);
        SnapResult s = snap_to_graph(loc, uf);
        if (s.distance_m > kMaxSnapMeters)
        {
            std::ostringstream os;
            os << "This truck parking location is more than " << static_cast<int>(kMaxSnapMeters)
               << " m from any ground-truck route; it cannot be matched to the service vehicle network.";
            msgs.push_back(validation_error_t(os.str(),
                warn_truck_endpoint_off_network, p, apt));
            continue;
        }
        sources.push_back({ p, p->GetTruckType(), loc, s.component });
    }

    std::vector<Sink> sinks;
    sinks.reserve(truck_destinations.size() + ramps.size());

    for (auto * d : truck_destinations)
    {
        std::set<int> types;
        d->GetTruckTypes(types);
        if (types.empty()) continue; // already caught by err_truck_dest_must_have_at_least_one_truck_type_selected
        Point2 loc;
        d->GetLocation(gis_Geo, loc);
        SnapResult s = snap_to_graph(loc, uf);
        if (s.distance_m > kMaxSnapMeters)
        {
            std::ostringstream os;
            os << "This truck destination is more than " << static_cast<int>(kMaxSnapMeters)
               << " m from any ground-truck route; it cannot be matched to the service vehicle network.";
            msgs.push_back(validation_error_t(os.str(),
                warn_truck_endpoint_off_network, d, apt));
            continue;
        }
        sinks.push_back({ d, std::move(types), loc, s.component, "service destination" });
    }

    for (auto * r : ramps)
    {
        // Only ramps where large aircraft can park request pushback. Skip Hangar /
        // TieDown (no service operations there), None / GeneralAviation operation
        // (small props don't request pushback), and ramps that don't admit any
        // heavy/jet/fighter equipment.
        const int rt = r->GetType();
        if (rt != atc_Ramp_Gate && rt != atc_Ramp_Misc)
            continue;

        const int op = r->GetRampOperationType();
        if (op != ramp_operation_Airline && op != ramp_operation_Cargo && op != ramp_operation_Military)
            continue;

        std::set<int> equip;
        r->GetEquipment(equip);
        if (!equip.count(atc_Heavies) && !equip.count(atc_Jets) && !equip.count(atc_Fighters))
            continue;

        Point2 loc;
        r->GetLocation(gis_Geo, loc);
        SnapResult s = snap_to_graph(loc, uf);
        if (s.distance_m > kMaxSnapMeters)
            continue; // ramps far from service roads are common; do not warn

        std::set<int> needs;
        needs.insert(atc_ServiceTruck_Pushback);
        sinks.push_back({ r, std::move(needs), loc, s.component, "ramp start" });
    }

    // Step 3: for each (consumer, needed truck type), look for in-radius candidates
    // and verify at least one shares the consumer's component.
    for (auto& sk : sinks)
    {
        for (int t : sk.needed_types)
        {
            std::vector<WED_TruckParkingLocation *> in_radius;
            bool any_reachable = false;
            for (auto& src : sources)
            {
                if (src.truck_type != t) continue;
                if (LonLatDistMeters(sk.loc, src.loc) > kXPlaneSearchRadiusMeters) continue;
                in_radius.push_back(src.who);
                if (src.component == sk.component) { any_reachable = true; break; }
            }
            if (any_reachable) continue;
            if (in_radius.empty()) continue; // no provider in range -- under-provisioned, not a connectivity bug

            // bad_objects holds only the consumer so Zoom-To frames just the affected
            // stand/destination. The "Zoom Out" button widens progressively if the
            // user wants to see the unreachable candidates.
            std::ostringstream os;
            os << "This " << sk.kind_label << " needs vehicle type '" << ENUM_Desc(t)
               << "' and " << in_radius.size() << " candidate truck"
               << (in_radius.size() == 1 ? "" : "s")
               << " within " << static_cast<int>(kXPlaneSearchRadiusMeters)
               << " m, but every candidate is on a different component of the ground-truck network.";
            msgs.push_back(validation_error_t(os.str(),
                warn_truck_consumer_unreachable_in_radius, sk.who, apt));
        }
    }
}
