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

#ifndef WED_TruckNetworkValidate_H
#define WED_TruckNetworkValidate_H

#include <span>
#include <vector>

class WED_Airport;
class WED_TaxiRoute;
class WED_TruckParkingLocation;
class WED_TruckDestination;
class WED_RampPosition;
struct validation_error_t;
typedef std::vector<validation_error_t> validation_error_vector;

// Detect ground-service-vehicle consumers (gates wanting pushback; truck destinations
// wanting any service-truck type) that have a candidate provider within X-Plane's
// 1500 m search radius but where every candidate sits on a different connected
// component of the 1206 truck network than the consumer. This is the LPPT
// "No pushback service available" data defect.
//
// Multiple disjoint components are themselves OK (runways split networks; civil
// vs. military must stay separate). The warning fires only when X-Plane would
// actually find a candidate and then reject it for dijk_diff_grids.
//
// `GT_routes` is the AllowTrucks() subset of taxiroutes that the caller has
// already prepared; this function does not re-filter.
void WED_DoTruckNetworkReachability(
    WED_Airport *                                       apt,
    validation_error_vector&                            msgs,
    std::span<WED_TaxiRoute *            const>         GT_routes,
    std::span<WED_TruckParkingLocation * const>         truck_parking_locs,
    std::span<WED_TruckDestination *     const>         truck_destinations,
    std::span<WED_RampPosition *         const>         ramps);

#endif
