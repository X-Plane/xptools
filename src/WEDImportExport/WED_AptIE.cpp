/*
 * Copyright (c) 2007, Laminar Research.
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

#include "WED_AptImportDialog.h"
#include "WED_AptIE.h"
#include "WED_Airport.h"
#include "WED_AirportBeacon.h"
#include "WED_AirportBoundary.h"
#include "WED_AirportChain.h"
#include "WED_AirportNode.h"
#include "WED_AirportSign.h"
#include "WED_ATCFrequency.h"
#include "WED_Helipad.h"
#include "WED_LightFixture.h"
#include "WED_RampPosition.h"
#include "WED_Runway.h"
#include "WED_RunwayNode.h"
#include "WED_Sealane.h"
#include "WED_Taxiway.h"
#include "WED_TowerViewpoint.h"
#include "WED_Windsock.h"
#include "WED_EnumSystem.h"
#include "WED_OverlayImage.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
#include "WED_ATCTimeRule.h"
#include "WED_ATCWindRule.h"
#include "WED_TaxiRoute.h"
#include "WED_TaxiRouteNode.h"
#include "WED_Group.h"
#include "GUI_Application.h"
#include "WED_Validate.h"
#include "WED_TruckParkingLocation.h"
#include "WED_TruckDestination.h"

#include "AptIO.h"

//Utils
#include "PlatformUtils.h"
#include "STLUtils.h"

//WEDUtils
#include "WED_HierarchyUtils.h"
#include "WED_ToolUtils.h"

#include "WED_UIDefs.h"
#include <stdarg.h>


#if ERROR_CHECK
error checking here and in apt-io
#endif

static int get_apt_export_version()
{
	int target = gExportTarget;
	
	if(target == wet_gateway)
		target = wet_latest_xplane;
	
	int version = 1000;
	switch(target)
	{
	case wet_xplane_900:
		version = 850;
		break;
	case wet_xplane_1021:
	case wet_xplane_1000:
		version = 1000;
		break;
	case wet_xplane_1050:
		version = 1050;
		break;
	case wet_xplane_1100:
		version = 1100;
		break;
	default:
		DebugAssert(!"You forgot to add a case!");
		break;
	}
	return version;
}

inline Point2	recip(const Point2& pt, const Point2& ctrl) { return pt + Vector2(ctrl,pt); }
inline	void	accum(AptPolygon_t& poly, int code, const Point2& pt, const Point2& ctrl, const set<int>& attrs)
{
	poly.push_back(AptLinearSegment_t());
	poly.back().code = code;
	poly.back().pt = pt;
	poly.back().ctrl = ctrl;
	poly.back().attributes = attrs;
}

inline bool is_curved(int code) { return code == apt_lin_crv || code == apt_rng_crv ||  code == apt_end_crv; }


static void ExportLinearPath(WED_AirportChain * chain, AptPolygon_t& poly)
{
	int n = chain->GetNumPoints();
	int l = n-1;
	bool closed = chain->IsClosed();
	set<int>	no_attrs;

	Point2	pt, hi, lo;
	for (int i = 0; i < n; ++i)
	{
		bool last = i == l;
		bool first = i == 0;
		WED_AirportNode * node = dynamic_cast<WED_AirportNode *>(chain->GetNthPoint(i));
		if (node)
		{
			node->GetLocation(gis_Geo,pt);
			bool has_hi = node->GetControlHandleHi(gis_Geo,hi);
			bool has_lo = node->GetControlHandleLo(gis_Geo,lo);
			bool is_split = node->IsSplit();
			if (!closed && last)
			{
				// Special case - write out the last point...that's all we have to do.
				accum(poly, has_lo ? apt_end_crv : apt_end_seg, pt, recip(pt,lo), no_attrs);
			}
			else
			{
				set<int>	attrs, no_attrs;
				set<int>	enums;
				node->GetAttributes(enums);
				for (set<int>::iterator e = enums.begin(); e != enums.end(); ++e)
					attrs.insert(ENUM_Export(*e));

				if (first && !closed) is_split = false;	// optimization - even if user split the first point, who cares - hi control is all that used, so we do not
				// need to separately export the low one!

				if (is_split)
				{
					// Note that we have to know which is the last of the three we write and write the ring if needed.

					if (has_lo)	accum(poly,									  apt_lin_crv, pt, recip(pt, lo)	,		  no_attrs		   );
								accum(poly, (!has_hi && last) ? apt_rng_seg : apt_lin_seg, pt, pt				, has_hi ? no_attrs : attrs);
					if (has_hi) accum(poly,             last  ? apt_rng_crv : apt_lin_crv, pt, hi				,					  attrs);
				}
				else
				{
					// Simple case: we aren't split, so one point does it.
					if (has_hi)	accum(poly, last ? apt_rng_crv : apt_lin_crv, pt, hi, attrs);
					else		accum(poly, last ? apt_rng_seg : apt_lin_seg, pt, pt, attrs);
				}

			}
		}

	}
}

#if AIRPORT_ROUTING
/**
 * Recursively walks the root tree to collect all elements of the specified type.
 * 
 * For instance, you might use this to collect all WED_TaxiRouteNode objects
 * within an airport (and store them in the order the same relative order that they
 * were listed in the editor).
 */
template<class T>
static void CollectAllElementsOfType(WED_Thing * root, vector<T *> & elements)
{
	T * r = dynamic_cast<T*>(root);
	if(r)
	{
		elements.push_back(r);
	}
	
	int nn = root->CountChildren();
	for(int n = 0; n < nn; ++n)
	{
		CollectAllElementsOfType<T>(root->GetNthChild(n), elements);
	}
}

// Just like above, but limits to a specific subset of nodes.  WHY not just copy keepers to elements?  
// The resulting vector is IN HIERARCHY ORDER but only contains keepers.

template<class T>
static void CollectAllElementsOfTypeInSet(WED_Thing * root, vector<T *> & elements, const set<T *>& keepers)
{
	T * r = dynamic_cast<T*>(root);
	if(r)
	if(keepers.count(r))
	{
		elements.push_back(r);
	}
	
	int nn = root->CountChildren();
	for(int n = 0; n < nn; ++n)
	{
		CollectAllElementsOfTypeInSet<T>(root->GetNthChild(n), elements, keepers);
	}
}



/**
 * "Exports" the list of nodes into the airport network
 */
static void MakeNodeRouting(vector<IGISPoint *>& nodes, AptNetwork_t& net)
{
	for(vector<IGISPoint *>::iterator n = nodes.begin(); n != nodes.end(); ++n)
	{
		AptRouteNode_t nd;
		// Node's ID is its index in file order	
		nd.id = std::distance(nodes.begin(), n);
		WED_Thing * t = dynamic_cast<WED_Thing *>(*n);
		if(t)
			t->GetName(nd.name);
		
		(*n)->GetLocation(gis_Geo, nd.location);
		net.nodes.push_back(nd);	
	}
}

/**
 * "Exports" the list of edges into the airport network
 * @param nodes The list of all taxi route nodes in the airport (required to get accurate IDs for the nodes)
 */
static void MakeEdgeRouting(vector<WED_TaxiRoute *>& edges, AptNetwork_t& net, vector<IGISPoint *> * nodes)
{
	for(vector<WED_TaxiRoute *>::iterator e = edges.begin(); e != edges.end(); ++e)
	{
		AptRouteEdge_t ne;
		AptServiceRoadEdge_t se;
		(*e)->Export(ne,se);
		bool is_truxiroute = (*e)->AllowTrucks();

		AptEdgeBase_t * base = is_truxiroute ? (AptEdgeBase_t *) &se : (AptEdgeBase_t *) &ne;
		
		// Node's ID is its index in file order
		vector<IGISPoint *>::iterator pos_src = std::find(nodes->begin(), nodes->end(), (*e)->GetNthPoint(0));
		base->src = std::distance(nodes->begin(), pos_src);
		
		vector<IGISPoint *>::iterator pos_dst = std::find(nodes->begin(), nodes->end(), (*e)->GetNthPoint(1));
		base->dst = std::distance(nodes->begin(), pos_dst);;
		
		Segment2 s;
		Bezier2 b;
		if((*e)->GetSide(gis_Geo, 0, s, b))
		{
			base->shape.push_back(make_pair(b.c1,true));
			base->shape.push_back(make_pair(b.c2,true));
		}
		
		if(is_truxiroute)
			net.service_roads.push_back(se);
		else
			net.edges.push_back(ne);
	}
}
#endif

void	AptExportRecursive(WED_Thing * what, AptVector& apts)
{
	WED_Airport *			apt;
	WED_AirportBeacon *		bcn;
	WED_AirportBoundary *	bou;
	WED_AirportChain *		cha;
	WED_AirportSign *		sgn;
	WED_ATCFrequency *		atc;
	WED_Helipad *			hel;
	WED_LightFixture *		lit;
	WED_RampPosition *		ram;
	WED_Runway *			rwy;
	WED_Sealane *			sea;
	WED_Taxiway *			tax;
	WED_TowerViewpoint *	twr;
	WED_Windsock *			win;
	
#if AIRPORT_ROUTING
	WED_ATCFlow *			flw;
	WED_ATCRunwayUse *		use;
	WED_ATCTimeRule *		tim;
	WED_ATCWindRule *		wnd;
	WED_TruckDestination *	dst;
	WED_TruckParkingLocation*trk;
#endif
	int holes, h;
	
	WED_Entity * ent = dynamic_cast<WED_Entity *>(what);
	if (ent && ent->GetHidden()) return;
	

	/* Special case bug fix: for old alphas we used the airport ring type (not the generic ring) to
	 * build the interior ring of an overlay image.  If we recurse through the overlay image we  get
	 * a bogus export. */
	if(dynamic_cast<WED_OverlayImage *>(what)) return;

	if (apt = dynamic_cast<WED_Airport *>(what))
	{
		apts.push_back(AptInfo_t());
		apt->Export(apts.back());
		
#if AIRPORT_ROUTING
		vector<WED_TaxiRoute *> edges;			// These are in
		vector<IGISPoint *>		nodes;			// hierarchy order for stability!
		set<IGISPoint *>		wanted_nodes;

		CollectRecursive(apt, back_inserter(edges), WED_TaxiRoute::sClass);

		for(vector<WED_TaxiRoute *>::iterator e = edges.begin(); e != edges.end(); ++e)
		{
			IGISPoint* point_0 = (*e)->GetNthPoint(0);
			IGISPoint* point_1 = (*e)->GetNthPoint(1);
			DebugAssert(point_0 != NULL && point_1 != NULL);

			wanted_nodes.insert(point_0);
			wanted_nodes.insert(point_1);
		}
		
		CollectAllElementsOfTypeInSet<IGISPoint>(apt, nodes, wanted_nodes);

		MakeNodeRouting(nodes, apts.back().taxi_route);
		MakeEdgeRouting(edges, apts.back().taxi_route, &nodes);
		
#endif
	}
	else if (bcn = dynamic_cast<WED_AirportBeacon *>(what))
	{
		bcn->Export(apts.back().beacon);
	}
	else if (bou = dynamic_cast<WED_AirportBoundary *>(what))
	{
		apts.back().boundaries.push_back(AptBoundary_t());
		bou->Export(apts.back().boundaries.back());
		cha = dynamic_cast<WED_AirportChain*>(bou->GetOuterRing());
		if (cha) ExportLinearPath(cha, apts.back().boundaries.back().area);
		holes = bou->GetNumHoles();
		for (h = 0; h < holes; ++h)
		{
			cha = dynamic_cast<WED_AirportChain *>(bou->GetNthHole(h));
			if (cha) ExportLinearPath(cha, apts.back().boundaries.back().area);
		}
		return;	// bail out - we already got the children.

	}
	else if (cha = dynamic_cast<WED_AirportChain *>(what))
	{
		apts.back().lines.push_back(AptMarking_t());
		cha->Export(apts.back().lines.back());
		ExportLinearPath(cha, apts.back().lines.back().area);
		return;	// don't waste time with nodes - for speed
	}
	else if (sgn = dynamic_cast<WED_AirportSign *>(what))
	{
		apts.back().signs.push_back(AptSign_t());
		sgn->Export(apts.back().signs.back());
	}
	else if (hel = dynamic_cast<WED_Helipad *>(what))
	{
		apts.back().helipads.push_back(AptHelipad_t());
		hel->Export(apts.back().helipads.back());
	}
	else if (lit = dynamic_cast<WED_LightFixture *>(what))
	{
		apts.back().lights.push_back(AptLight_t());
		lit->Export(apts.back().lights.back());
	}
	else if (ram = dynamic_cast<WED_RampPosition *>(what))
	{
		apts.back().gates.push_back(AptGate_t());
		ram->Export(apts.back().gates.back());
	}
	else if (rwy = dynamic_cast<WED_Runway *>(what))
	{
		apts.back().runways.push_back(AptRunway_t());
		rwy->Export(apts.back().runways.back());
	}
	else if (sea = dynamic_cast<WED_Sealane *>(what))
	{
		apts.back().sealanes.push_back(AptSealane_t());
		sea->Export(apts.back().sealanes.back());
	}
	else if (tax = dynamic_cast<WED_Taxiway *>(what))
	{
		apts.back().taxiways.push_back(AptTaxiway_t());
		tax->Export(apts.back().taxiways.back());

		cha = dynamic_cast<WED_AirportChain*>(tax->GetOuterRing());
		if (cha) ExportLinearPath(cha, apts.back().taxiways.back().area);
		holes = tax->GetNumHoles();
		for (h = 0; h < holes; ++h)
		{
			cha = dynamic_cast<WED_AirportChain *>(tax->GetNthHole(h));
			if (cha) ExportLinearPath(cha, apts.back().taxiways.back().area);
		}
		return; // bail out - we already got the children
	}
	else if (twr = dynamic_cast<WED_TowerViewpoint *>(what))
	{
		twr->Export(apts.back().tower);
	}
	else if (win = dynamic_cast<WED_Windsock *>(what))
	{
		apts.back().windsocks.push_back(AptWindsock_t());
		win->Export(apts.back().windsocks.back());
	}
	else if (atc = dynamic_cast<WED_ATCFrequency *>(what))
	{
		apts.back().atc.push_back(AptATCFreq_t());
		atc->Export(apts.back().atc.back());
	}
#if AIRPORT_ROUTING	
	else if(flw = dynamic_cast<WED_ATCFlow *>(what))
	{
		apts.back().flows.push_back(AptFlow_t());
		flw->Export(apts.back().flows.back());
	}
	else if(use = dynamic_cast<WED_ATCRunwayUse *>(what))
	{
		apts.back().flows.back().runway_rules.push_back(AptRunwayRule_t());
		use->Export(apts.back().flows.back().runway_rules.back());
	}
	else if(tim = dynamic_cast<WED_ATCTimeRule *>(what))
	{
		apts.back().flows.back().time_rules.push_back(AptTimeRule_t());
		tim->Export(apts.back().flows.back().time_rules.back());
	}
	else if(wnd = dynamic_cast<WED_ATCWindRule *>(what))
	{
		apts.back().flows.back().wind_rules.push_back(AptWindRule_t());
		wnd->Export(apts.back().flows.back().wind_rules.back());
	}
	else if(trk = dynamic_cast<WED_TruckParkingLocation*>(what))
	{
		apts.back().truck_parking.push_back(AptTruckParking_t());
		trk->Export(apts.back().truck_parking.back());
	}
	else if(dst = dynamic_cast<WED_TruckDestination*>(what))
	{
		apts.back().truck_destinations.push_back(AptTruckDestination_t());
		dst->Export(apts.back().truck_destinations.back());
	}
	
	
#endif	

	int cc = what->CountChildren();
	for (int i = 0; i < cc; ++i)
		AptExportRecursive(what->GetNthChild(i), apts);
}

void	WED_AptExport(
				WED_Thing *		container,
				const char *	file_path)
{
	AptVector	apts;
	AptExportRecursive(container, apts);
	WriteAptFile(file_path,apts, get_apt_export_version());
}

void	WED_AptExport(
				WED_Thing *		container,
				int (*			print_func)(void *, const char *, ...),
				void *			ref)
{
	AptVector	apts;
	AptExportRecursive(container, apts);
	WriteAptFileProcs(print_func, ref, apts, get_apt_export_version());
}


int		WED_CanExportApt(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	return wrl->CountChildren() > 0;
}

void	WED_DoExportApt(IResolver * resolver)
{
	if (!WED_ValidateApt(resolver)) return;

	WED_Thing * wrl = WED_GetWorld(resolver);
	char path[1024];
	strcpy(path,"apt.dat");
	if (GetFilePathFromUser(getFile_Save,"Export airport data as...", "Export", FILE_DIALOG_EXPORT_APTDAT, path, sizeof(path)))
	{
		WED_AptExport(wrl, path);
	}
}

static WED_AirportChain * ImportLinearPath(const AptPolygon_t& path, WED_Archive * archive, WED_Thing * parent, vector<WED_AirportChain *> * chains, void (* print_func)(void *, const char *, ...), void * ref)
{
	WED_AirportChain * chain = NULL;
	WED_AirportChain * ret = NULL;
	for (AptPolygon_t::const_iterator cur = path.begin(); cur != path.end(); ++cur)
	{
		if (chain == NULL)
		{
			chain = WED_AirportChain::CreateTyped(archive);
			chain->SetParent(parent, parent->CountChildren());
			ret = chain;
			if (chains) chains->push_back(chain);
			chain->SetClosed(false);
		}

		// Ben says: Okay, what is about to happen here??  Well, due to my own stupidity, you can't have "split beziers" in an apt.dat file.  (A split bezier is one that has
		// bezier control points on one side of a vertex but not the other, or different bezier control points.  So we encode this in an apt.dat by a number of points with the
		// same location, attributes, but not the same control points.  Typical patterns:
		// - Split bezier, curve on both sides: curve, segment, curve
		// - Split bezier, curve on one side: curve, segment
		// - Split bezier, curve on other side: segment, curve
		// So what we do here is:
		// 1. We find our "low-side" control point first.
		// 2. We scan forward for as many colocated points as possible that form zero-length segments (which we skip).  BUT: two control points on two different points DO
		//    make a loop even if the points are colocated.
		// 3. The final point is used for the "high-side" control point.

		// 1. Low side control point
		bool has_lo = is_curved(cur->code);
		Point2	lo_pt;
		if (has_lo) lo_pt = recip(cur->pt, cur->ctrl);

//		printf("Low pt is: %d %lf,%lf (%lf,%lf)\n",cur->code, cur->pt.x,cur->pt.y,cur->ctrl.x,cur->ctrl.y);

		// 2. Iterate forward: run until our point changes or we hit a span that forms a non-zero-length curve.
		AptPolygon_t::const_iterator next = cur, orig = cur, prev = cur;
		++next;
		while (next != path.end() && next->pt == cur->pt &&
			(!is_curved(prev->code) || !is_curved(next->code)) && 
			(is_curved(prev->code) || is_curved(next->code)) ) // skip only if the co-located points are actually part of a bezier-node
		{
			prev = next;
//			printf("Skip: %d %lf,%lf (%lf,%lf)\n", next->code, next->pt.x(),next->pt.y(),next->ctrl.x(),next->ctrl.y());
			++next;
		}
//		printf("stopped due to: %d %lf,%lf (%lf,%lf)\n", next->code, next->pt.x,next->pt.y,next->ctrl.x,next->ctrl.y);
		--next;
		cur = next;
//		printf("hi-end pt is: %d %lf,%lf (%lf,%lf)\n",cur->code, cur->pt.x,cur->pt.y,cur->ctrl.x,cur->ctrl.y);


		// 3. High side control point.
		bool has_hi = is_curved(cur->code);
		Point2 hi_pt;
		if (has_hi) hi_pt = cur->ctrl;

		// Convert attributes
		set<int>	attrs;
		for (set<int>::const_iterator e = cur->attributes.begin(); e != cur->attributes.end(); ++e)
		{
			int i = ENUM_Import(LinearFeature, *e);
			if (i != -1)
			attrs.insert(i);
			else if (i != 0) print_func(ref,"Ignoring unknown airport line attribute %d.\n", i);
		}
		//	Now we can form a node.
		WED_AirportNode * n = WED_AirportNode::CreateTyped(archive);
		n->SetParent(chain, chain->CountChildren());
		n->SetLocation(gis_Geo,cur->pt);
		n->SetSplit(has_lo != has_hi || orig != cur);
		if (has_lo) n->SetControlHandleLo(gis_Geo,lo_pt);
		if (has_hi) n->SetControlHandleHi(gis_Geo,hi_pt);

		n->SetAttributes(attrs);

		if (cur->code == apt_end_seg || cur->code == apt_end_crv)
		{
			chain = NULL;
		}
		else if (cur->code == apt_rng_seg || cur->code == apt_rng_crv)
		{
			chain->SetClosed(true);
			chain = NULL;
		}
	}
	return ret;
}

struct	LazyLog_t {
	const char *	path;
	FILE *			fi;
};

void LazyPrintf(void * ref, const char * fmt, ...)
{
	va_list	arg;
	va_start(arg, fmt);
	LazyLog_t * l = (LazyLog_t *) ref;
	if (l->fi == NULL) l->fi = fopen(l->path,"w");
	if (l->fi) vfprintf(l->fi,fmt,arg);
}

//A set of values describing the desired hierarchy order
// pair<Parent Group Name, Child Group Name>
// "" can be used if the group is intended to be under the world root
typedef set<string> hierarchy_order_set;

static hierarchy_order_set build_order_set()
{
	hierarchy_order_set h_set;
	//BEWARE: Stringified-data abounds!
	//If this has to be editted more than twice a year, we'll create
	//an enum + dictionary solution that is more type safe
	//"/" is like a dir seperator
	h_set.insert("/ATC");
	h_set.insert("/Ground Vehicles");
	h_set.insert("/Ground Vehicles/Dynamic");
	h_set.insert("/Ground Vehicles/Static");
	h_set.insert("/Lights");
	h_set.insert("/Markings");
	h_set.insert("/Ramp Starts");
	h_set.insert("/Runways");
	h_set.insert("/Signs");
	h_set.insert("/Taxi Routes");
	h_set.insert("/Ground Routes");
	h_set.insert("/Taxiways");
	h_set.insert("/Tower, Beacon and Boundaries");
	h_set.insert("/Windsocks");
	h_set.insert("/Exclusion Zones");
	h_set.insert("/Objects");
	h_set.insert("/Objects/Buildings");
	h_set.insert("/Objects/Vehicles");
	h_set.insert("/Objects/Trees");
	h_set.insert("/Objects/Other");
	h_set.insert("/Facades");
	h_set.insert("/Forests");
	h_set.insert("/Lines");
	h_set.insert("/Draped Polygons");
	return h_set;
}

static const hierarchy_order_set prefered_hierarchy_order = build_order_set();
struct compare_bucket_order : public less<string>
{
	bool operator()(const string& lhs, const string& rhs)
	{
		hierarchy_order_set::iterator lhs_pos = prefered_hierarchy_order.end();
		hierarchy_order_set::iterator rhs_pos = prefered_hierarchy_order.end();
		hierarchy_order_set::iterator pref_end = prefered_hierarchy_order.end();

		hierarchy_order_set::iterator itr = prefered_hierarchy_order.begin();
		
		while(itr != pref_end && lhs_pos == pref_end && rhs_pos == pref_end)
		{
			lhs_pos = lhs == *itr ? itr : pref_end;
			rhs_pos = rhs == *itr ? itr : pref_end;
			++itr;
		}

		DebugAssert(lhs_pos != pref_end);
		DebugAssert(rhs_pos != pref_end);
		
		return std::distance(prefered_hierarchy_order.begin(), lhs_pos) < std::distance(prefered_hierarchy_order.begin(), rhs_pos);
	}
};

typedef  map<string, WED_Thing *> hierarchy_bucket_map;

//
static hierarchy_bucket_map::iterator create_buckets(WED_Thing* apt, const string& name, hierarchy_bucket_map& io_buckets, WED_Thing* parent_group = NULL)
{
	WED_Thing * new_bucket = WED_Group::CreateTyped(apt->GetArchive());
	new_bucket->SetName(name);

	if (parent_group != NULL)
	{
		new_bucket->SetParent(parent_group, parent_group->CountChildren());
	}
	else
	{
		new_bucket->SetParent(apt, apt->CountChildren());
	}
	return io_buckets.insert(make_pair(name, new_bucket)).first;
}

static void add_to_bucket(WED_Thing * child, WED_Thing * apt, const string& name, hierarchy_bucket_map& io_buckets)
{
	DebugAssert(io_buckets.find(name) != io_buckets.end());
	hierarchy_bucket_map::iterator b = io_buckets.find(name);
		//set<string> group_names;
		//tokenize_string(name.begin(), name.end(), back_inserter(group_names), '/');
		//for (set<string>::iterator itr = group_names.begin(); itr != group_names.end(); ++itr)
		//{
		//	WED_Thing * new_bucket = WED_Group::CreateTyped(apt->GetArchive());
		//	new_bucket->SetName(*itr);
		//	new_bucket->SetParent(last_parent, last_parent->CountChildren());
		//	last_parent = new_bucket;
		//	io_buckets.insert(make_pair(name, new_bucket)).first;
		//}

	child->SetParent(b->second, b->second->CountChildren());
}

void recursive_delete_empty_groups(WED_Thing* group)
{
}

void	WED_AptImport(
				WED_Archive *			archive,
				WED_Thing *				container,
				const string&			file_path,
				AptVector&				apts,
				vector<WED_Airport *> *	out_airports)
{
	bool import_ok = true;
	for (AptVector::iterator apt = apts.begin(); apt != apts.end(); ++apt)
	{
		string log_path(file_path);
		log_path += ".log";
		LazyLog_t log = { log_path.c_str(), NULL };

		bool apt_ok = CheckATCRouting(*apt);
		if(!apt_ok)
		{
			LazyPrintf(&log,"Airport %s (%s) had a problem with its taxi routes.\n", apt->name.c_str(), apt->icao.c_str());
		}

		ConvertForward(*apt);

		hierarchy_bucket_map buckets;
		WED_Airport * new_apt = WED_Airport::CreateTyped(archive);
		new_apt->SetParent(container,container->CountChildren());
		new_apt->Import(*apt, LazyPrintf, &log);
		if(out_airports)
			out_airports->push_back(new_apt);

		create_buckets(new_apt, "ATC",                          buckets);
		create_buckets(new_apt, "Ground Vehicles",              buckets);
		create_buckets(new_apt, "Lights",                       buckets);
		create_buckets(new_apt, "Markings",                     buckets);
		create_buckets(new_apt, "Ramp Starts",                  buckets);
		create_buckets(new_apt, "Runways",                      buckets);
		create_buckets(new_apt, "Signs",                        buckets);
		create_buckets(new_apt, "Taxi Routes",                  buckets);
		create_buckets(new_apt, "Ground Routes",                buckets);
		create_buckets(new_apt, "Taxiways",                     buckets);
		create_buckets(new_apt, "Tower, Beacon and Boundaries", buckets);
		create_buckets(new_apt, "Windsocks",                    buckets);

		for (AptRunwayVector::iterator rwy = apt->runways.begin(); rwy != apt->runways.end(); ++rwy)
		{
			WED_Runway *		new_rwy = WED_Runway::CreateTyped(archive);
			WED_RunwayNode *	source = WED_RunwayNode::CreateTyped(archive);
			WED_RunwayNode *	target = WED_RunwayNode::CreateTyped(archive);
			add_to_bucket(new_rwy,new_apt,"Runways",buckets);
			source->SetParent(new_rwy,0);
			target->SetParent(new_rwy,1);
			new_rwy->Import(*rwy, LazyPrintf, &log);
		}

		for (AptSealaneVector::iterator sea = apt->sealanes.begin(); sea != apt->sealanes.end(); ++sea)
		{
			WED_Sealane *		new_sea = WED_Sealane::CreateTyped(archive);
			WED_RunwayNode *	source = WED_RunwayNode::CreateTyped(archive);
			WED_RunwayNode *	target = WED_RunwayNode::CreateTyped(archive);
			add_to_bucket(new_sea,new_apt,"Runways",buckets);
			source->SetParent(new_sea,0);
			target->SetParent(new_sea,1);
			new_sea->Import(*sea, LazyPrintf, &log);
		}

		for (AptHelipadVector::iterator hel = apt->helipads.begin(); hel != apt->helipads.end(); ++hel)
		{
			WED_Helipad * new_hel = WED_Helipad::CreateTyped(archive);
			add_to_bucket(new_hel,new_apt,"Runways",buckets);
			new_hel->Import(*hel, LazyPrintf, &log);
		}

		for (AptTaxiwayVector::iterator tax = apt->taxiways.begin(); tax != apt->taxiways.end(); ++tax)
		{
			WED_Taxiway * new_tax = WED_Taxiway::CreateTyped(archive);
			add_to_bucket(new_tax,new_apt,"Taxiways",buckets);
			new_tax->Import(*tax, LazyPrintf, &log);

			if (!ImportLinearPath(tax->area, archive, new_tax, NULL, LazyPrintf, &log))
			{
				new_tax->SetParent(NULL,0);
				new_tax->Delete();
			}
		}

		for (AptBoundaryVector::iterator bou = apt->boundaries.begin(); bou != apt->boundaries.end(); ++bou)
		{
			WED_AirportBoundary * new_bou = WED_AirportBoundary::CreateTyped(archive);
			add_to_bucket(new_bou,new_apt,"Tower, Beacon and Boundaries",buckets);
			new_bou->Import(*bou, LazyPrintf, &log);

			if (!ImportLinearPath(bou->area, archive, new_bou, NULL, LazyPrintf, &log))
			{
				new_bou->SetParent(NULL,0);
				new_bou->Delete();
			}
		}

		for (AptMarkingVector::iterator lin = apt->lines.begin(); lin != apt->lines.end(); ++lin)
		{
			vector<WED_AirportChain *> new_lin;
			WED_Thing * markings = buckets["Markings"];
			if(markings == NULL)
			{
				DebugAssert(false);
				markings = WED_Group::CreateTyped(new_apt->GetArchive());
				markings->SetName("Markings");
				buckets["Markings"] = markings;
			}
			
			ImportLinearPath(lin->area, archive, markings, &new_lin, LazyPrintf, &log);
			for (vector<WED_AirportChain *>::iterator li = new_lin.begin(); li != new_lin.end(); ++li)
				(*li)->Import(*lin, LazyPrintf, &log);
		}

		for (AptLightVector::iterator lit = apt->lights.begin(); lit != apt->lights.end(); ++lit)
		{
			WED_LightFixture * new_lit = WED_LightFixture::CreateTyped(archive);
			add_to_bucket(new_lit,new_apt,"Lights",buckets);
			new_lit->Import(*lit, LazyPrintf, &log);
		}

		for (AptSignVector::iterator sin = apt->signs.begin(); sin != apt->signs.end(); ++sin)
		{
			WED_AirportSign * new_sin = WED_AirportSign::CreateTyped(archive);
			add_to_bucket(new_sin,new_apt,"Signs",buckets);
			new_sin->Import(*sin, LazyPrintf, &log);
		}

		for (AptGateVector::iterator gat = apt->gates.begin(); gat != apt->gates.end(); ++gat)
		{
			WED_RampPosition * new_gat = WED_RampPosition::CreateTyped(archive);
			add_to_bucket(new_gat,new_apt,"Ramp Starts",buckets);
			new_gat->Import(*gat, LazyPrintf, &log);
		}

		if (apt->tower.draw_obj != -1)
		{
			WED_TowerViewpoint * new_twr = WED_TowerViewpoint::CreateTyped(archive);
			add_to_bucket(new_twr,new_apt,"Tower, Beacon and Boundaries",buckets);
			new_twr->Import(apt->tower, LazyPrintf, &log);
		}

		if (apt->beacon.color_code != apt_beacon_none)
		{
			WED_AirportBeacon * new_bea = WED_AirportBeacon::CreateTyped(archive);
			add_to_bucket(new_bea,new_apt,"Tower, Beacon and Boundaries",buckets);
			new_bea->Import(apt->beacon, LazyPrintf, &log);
		}

		for (AptWindsockVector::iterator win = apt->windsocks.begin(); win != apt->windsocks.end(); ++win)
		{
			WED_Windsock * new_win = WED_Windsock::CreateTyped(archive);
			add_to_bucket(new_win,new_apt,"Windsocks",buckets);
			new_win->Import(*win, LazyPrintf, &log);
		}

		for (AptATCFreqVector::iterator atc = apt->atc.begin(); atc != apt->atc.end(); ++atc)
		{
			WED_ATCFrequency * new_atc = WED_ATCFrequency::CreateTyped(archive);
			add_to_bucket(new_atc,new_apt,"ATC",buckets);
			new_atc->Import(*atc, LazyPrintf, &log);
		}
		
#if AIRPORT_ROUTING

		for(AptTruckParkingVector::iterator trk = apt->truck_parking.begin(); trk != apt->truck_parking.end(); ++trk)
		{
			WED_TruckParkingLocation * new_trk = WED_TruckParkingLocation::CreateTyped(archive);
			add_to_bucket(new_trk,new_apt,"Ground Vehicles",buckets);
			new_trk->Import(*trk, LazyPrintf, &log);
		}

		for(AptTruckDestinationVector::iterator dst = apt->truck_destinations.begin(); dst != apt->truck_destinations.end(); ++dst)
		{
			WED_TruckDestination * new_dst = WED_TruckDestination::CreateTyped(archive);
			add_to_bucket(new_dst,new_apt,"Ground Vehicles",buckets);
			new_dst->Import(*dst, LazyPrintf, &log);
		}

		for(AptFlowVector::iterator flw = apt->flows.begin(); flw != apt->flows.end(); ++flw)
		{
			WED_ATCFlow * new_flw = WED_ATCFlow::CreateTyped(archive);
			add_to_bucket(new_flw,new_apt,"ATC",buckets);
			new_flw->Import(*flw, LazyPrintf, &log);
			
			for(AptRunwayRuleVector::iterator use = flw->runway_rules.begin(); use != flw->runway_rules.end(); ++use)
			{
				WED_ATCRunwayUse * new_use = WED_ATCRunwayUse::CreateTyped(archive);
				new_use->SetParent(new_flw, new_flw->CountChildren());
				new_use->Import(*use, LazyPrintf, &log);
			}
			for(AptWindRuleVector::iterator wnd = flw->wind_rules.begin(); wnd != flw->wind_rules.end(); ++wnd)
			{
				WED_ATCWindRule * new_wnd = WED_ATCWindRule::CreateTyped(archive);
				new_wnd->SetParent(new_flw, new_flw->CountChildren());
				new_wnd->Import(*wnd, LazyPrintf, &log);
			}
			for(AptTimeRuleVector::iterator tim = flw->time_rules.begin(); tim != flw->time_rules.end(); ++tim)
			{
				WED_ATCTimeRule * new_tim = WED_ATCTimeRule::CreateTyped(archive);
				new_tim->SetParent(new_flw, new_flw->CountChildren());
				new_tim->Import(*tim, LazyPrintf, &log);
			}
		}
		
		if(apt_ok)
		{
			if ( (apt->taxi_route.edges.size() > 0 || apt->taxi_route.service_roads.size() > 0) 
						&& apt->taxi_route.nodes.size() < 2 )                  // bug in WED 1.6.0 beta1: no nodes saved
			{
				LazyPrintf(&log,"Not enough taxi_route nodes, skipping all Taxi/Ground Routes");
			}
			else
			{
				map<int,WED_TaxiRouteNode *>	nodes;
				for(vector<AptRouteNode_t>::iterator n = apt->taxi_route.nodes.begin(); n != apt->taxi_route.nodes.end(); ++n)
				{
					WED_TaxiRouteNode * new_n = WED_TaxiRouteNode::CreateTyped(archive);
					add_to_bucket(new_n,new_apt,"Taxi Routes",buckets);
					new_n->SetName(n->name);
					new_n->SetLocation(gis_Geo,n->location);
					nodes[n->id] = new_n;
				}
				
				// COPY PASTA WARNING PART 1
				for(vector<AptRouteEdge_t>::iterator e = apt->taxi_route.edges.begin(); e != apt->taxi_route.edges.end(); ++e)
				{
					vector<pair<Point2, bool> >		shape(e->shape);
					Point2 start_geo, end_geo;
					nodes[e->src]->GetLocation(gis_Geo, start_geo);
					nodes[e->dst]->GetLocation(gis_Geo, end_geo);
					shape.insert(shape.begin(),make_pair(start_geo, false));
					shape.insert(shape.end(),make_pair(end_geo, false));
				
					vector<pair<Point2, bool> >::iterator p1, p2, stop;
					p1 = p2 = shape.begin();
					stop = shape.end();
					--stop;
					WED_TaxiRouteNode * next, * prev = nodes[e->src];
					while(p1 != stop)
					{
						DebugAssert(!p1->second);
						p2 = p1;
						++p2;
						while(p2->second) ++p2;
						
						if(p2 == stop)
							next = nodes[e->dst];
						else
						{
							next = WED_TaxiRouteNode::CreateTyped(archive);
							add_to_bucket(next,new_apt,"Taxi Routes",buckets);
							next->SetName(e->name);
							next->SetLocation(gis_Geo, p2->first);
						}
						
						int cc = p2 - p1;
						if(cc > 3)
							LazyPrintf(&log,"Too many control points");
				
						WED_TaxiRoute * new_e = WED_TaxiRoute::CreateTyped(archive);
						new_e->AddSource(prev,0);
						new_e->AddSource(next,1);
						new_e->Import(*e,LazyPrintf,&log);
						add_to_bucket(new_e,new_apt,"Taxi Routes", buckets);

						if(cc == 2)
						{
							Bezier2 b;
							b.p1 = p1->first;
							b.p2 = p2->first;
							++p1;
							
							Vector2 to_c1 = Vector2(b.p1,p1->first);
							Vector2 to_c2 = Vector2(b.p2,p1->first);
							
							b.c1 = b.p1 + to_c1 * 2.0 / 3.0;
							b.c2 = b.p2 + to_c2 * 2.0 / 3.0;
							
							new_e->SetSideBezier(gis_Geo,b);
						}
						else if(cc == 3)
						{
							Bezier2 b;
							b.p1 = p1->first;
							++p1;
							DebugAssert(p1->second);
							b.c1 = p1->first;
							++p1;
							DebugAssert(p1->second);
							b.c2 = p1->first;
							b.p2 = p2->first;
							new_e->SetSideBezier(gis_Geo,b);
						}
						
						prev = next;
						
						p1 = p2;
					}
				}
				
				// COPY PASTA WARNING PART 2
				for(vector<AptServiceRoadEdge_t>::iterator e = apt->taxi_route.service_roads.begin(); e != apt->taxi_route.service_roads.end(); ++e)
				{
					vector<pair<Point2, bool> >		shape(e->shape);
					Point2 start_geo, end_geo;
					nodes[e->src]->GetLocation(gis_Geo, start_geo);
					nodes[e->dst]->GetLocation(gis_Geo, end_geo);
					shape.insert(shape.begin(),make_pair(start_geo, false));
					shape.insert(shape.end(),make_pair(end_geo, false));
				
					vector<pair<Point2, bool> >::iterator p1, p2, stop;
					p1 = p2 = shape.begin();
					stop = shape.end();
					--stop;
					WED_TaxiRouteNode * next, * prev = nodes[e->src];
					while(p1 != stop)
					{
						DebugAssert(!p1->second);
						p2 = p1;
						++p2;
						while(p2->second) ++p2;
						
						if(p2 == stop)
							next = nodes[e->dst];
						else
						{
							next = WED_TaxiRouteNode::CreateTyped(archive);
							add_to_bucket(next,new_apt,"Ground Routes",buckets);
							next->SetName(e->name);
							next->SetLocation(gis_Geo, p2->first);
						}
						
						int cc = p2 - p1;
						if(cc > 3)
							LazyPrintf(&log,"Too many control points");
				
						WED_TaxiRoute * new_e = WED_TaxiRoute::CreateTyped(archive);
						new_e->AddSource(prev,0);
						new_e->AddSource(next,1);
						new_e->Import(*e,LazyPrintf,&log);
						add_to_bucket(new_e,new_apt,"Ground Routes", buckets);

						if(cc == 2)
						{
							Bezier2 b;
							b.p1 = p1->first;
							b.p2 = p2->first;
							++p1;
							
							Vector2 to_c1 = Vector2(b.p1,p1->first);
							Vector2 to_c2 = Vector2(b.p2,p1->first);
							
							b.c1 = b.p1 + to_c1 * 2.0 / 3.0;
							b.c2 = b.p2 + to_c2 * 2.0 / 3.0;
							
							new_e->SetSideBezier(gis_Geo,b);
						}
						else if(cc == 3)
						{
							Bezier2 b;
							b.p1 = p1->first;
							++p1;
							DebugAssert(p1->second);
							b.c1 = p1->first;
							++p1;
							DebugAssert(p1->second);
							b.c2 = p1->first;
							b.p2 = p2->first;
							new_e->SetSideBezier(gis_Geo,b);
						}
						
						prev = next;
						
						p1 = p2;
					}
				}
				// END COPY PASTA WARNING

				for (hierarchy_bucket_map::reverse_iterator ritr = buckets.rbegin(); ritr != buckets.rend(); ++ritr)
				{
					if (ritr->second->CountChildren() == 0)
					{
						ritr->second->SetParent(NULL, 0);
						ritr->second->Delete();
					}
				}
			}
		}
#endif		

		if (log.fi)
		{
			fclose(log.fi);
			import_ok = false;
		}
	}

	if(!import_ok)
		DoUserAlert("There were problems during the import.  A log file has been created in the same file as the apt.dat file.");
}

int		WED_CanImportApt(IResolver * resolver)
{
	return 1;
}

void	WED_DoImportApt(WED_Document * resolver, WED_Archive * archive, WED_MapPane * pane)
{
	vector<string>	fnames;
		
	char * path = GetMultiFilePathFromUser("Import apt.dat...", "Import", FILE_DIALOG_IMPORT_APTDAT);
	if(!path)
		return;
		
	char * free_me = path;
		
	while(*path)
	{
		fnames.push_back(path);
		path += (strlen(path)+1);
	}
		
	free(free_me);
		
	if(fnames.empty())
		return;
		
	AptVector	apts, one_apt;
	
	for(vector<string>::iterator f = fnames.begin(); f != fnames.end(); ++f)
	{
		string result = ReadAptFile(f->c_str(), one_apt);
		if (!result.empty())
		{
			string msg = string("The apt.dat file '") + *f + string("' could not be imported:\n") + result;
			DoUserAlert(msg.c_str());
			return;
		}
		
		apts.insert(apts.end(),one_apt.begin(),one_apt.end());
	}
	
	WED_AptImportDialog * importer = new WED_AptImportDialog(gApplication, apts, fnames[0], resolver, archive, pane);
}

void	WED_ImportOneAptFile(
				const string&			in_path,
				WED_Thing *				in_parent,
				vector<WED_Airport *> *	out_apts)
{
	AptVector		apts;
	string result = ReadAptFile(in_path.c_str(), apts);
	if(!result.empty())
	{
		string msg = string("Unable to read apt.dat file '") + in_path + string("': ") + result;
		DoUserAlert(msg.c_str());
		return;
	}
	
	WED_AptImport(
			in_parent->GetArchive(),
			in_parent,
			in_path.c_str(),
			apts,
			out_apts);
}
