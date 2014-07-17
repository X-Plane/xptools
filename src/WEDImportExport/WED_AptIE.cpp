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
#include "WED_AptImportDialog.h"
#include "GUI_Application.h"
#include "WED_Validate.h"

#include "AptIO.h"
#include "WED_ToolUtils.h"
#include "WED_UIDefs.h"

#include "PlatformUtils.h"
#include <stdarg.h>


#if ERROR_CHECK
error checking here and in apt-io
#endif

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

/**
 * "Exports" the list of nodes into the airport network
 */
static void MakeNodeRouting(vector<WED_TaxiRouteNode *>& nodes, AptNetwork_t& net)
{
	for(vector<WED_TaxiRouteNode *>::iterator n = nodes.begin(); n != nodes.end(); ++n)
	{
		AptRouteNode_t nd;
		// Node's ID is its index in file order
		nd.id = std::distance(nodes.begin(), n);
		(*n)->GetName(nd.name);
		
		IGISPoint * p = dynamic_cast<IGISPoint*>(*n);
		p->GetLocation(gis_Geo, nd.location);
		net.nodes.push_back(nd);	
	}
}

/**
 * "Exports" the list of edges into the airport network
 * @param nodes The list of all taxi route nodes in the airport (required to get accurate IDs for the nodes)
 */
static void MakeEdgeRouting(vector<WED_TaxiRoute *>& edges, AptNetwork_t& net, vector<WED_TaxiRouteNode *> * nodes)
{
	for(vector<WED_TaxiRoute *>::iterator e = edges.begin(); e != edges.end(); ++e)
	{
		AptRouteEdge_t ne;
		(*e)->Export(ne);
		
		// Node's ID is its index in file order
		vector<WED_TaxiRouteNode *>::iterator pos_src = std::find(nodes->begin(), nodes->end(), (*e)->GetNthSource(0));
		ne.src = std::distance(nodes->begin(), pos_src);
		
		vector<WED_TaxiRouteNode *>::iterator pos_dst = std::find(nodes->begin(), nodes->end(), (*e)->GetNthSource(1));
		ne.dst = std::distance(nodes->begin(), pos_dst);;
		
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
		vector<WED_TaxiRouteNode *> nodes;
		CollectAllElementsOfType<WED_TaxiRouteNode>(apt, nodes);
		MakeNodeRouting(nodes, apts.back().taxi_route);
		
		vector<WED_TaxiRoute *> edges;
		CollectAllElementsOfType<WED_TaxiRoute>(apt, edges);
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
	WriteAptFile(file_path,apts, gExportTarget == wet_xplane_900 ? 850 : 1000);
}

void	WED_AptExport(
				WED_Thing *		container,
				int (*			print_func)(void *, const char *, ...),
				void *			ref)
{
	AptVector	apts;
	AptExportRecursive(container, apts);
	WriteAptFileProcs(print_func, ref, apts, gExportTarget == wet_xplane_900 ? 850 : 1000);
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
			(!is_curved(prev->code) || !is_curved(next->code)))
		{
			prev = next;
//			printf("Skip: %d %lf,%lf (%lf,%lf)\n", next->code, next->pt.x,next->pt.y,next->ctrl.x,next->ctrl.y);
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

void	WED_AptImport(
				WED_Archive *			archive,
				WED_Thing *				container,
				const char *			file_path,
				AptVector&				apts,
				vector<WED_Thing *> *	out_airports)
{
	char path[1024];
	strcpy(path,file_path);
	strcat(path,".log");

	LazyLog_t log = { path, NULL };

	for (AptVector::iterator apt = apts.begin(); apt != apts.end(); ++apt)
	{
		ConvertForward(*apt);

		WED_Airport * new_apt = WED_Airport::CreateTyped(archive);
		new_apt->SetParent(container,container->CountChildren());
		new_apt->Import(*apt, LazyPrintf, &log);
		if(out_airports) out_airports->push_back(new_apt);

		for (AptRunwayVector::iterator rwy = apt->runways.begin(); rwy != apt->runways.end(); ++rwy)
		{
			WED_Runway *		new_rwy = WED_Runway::CreateTyped(archive);
			WED_RunwayNode *	source = WED_RunwayNode::CreateTyped(archive);
			WED_RunwayNode *	target = WED_RunwayNode::CreateTyped(archive);
			new_rwy->SetParent(new_apt,new_apt->CountChildren());
			source->SetParent(new_rwy,0);
			target->SetParent(new_rwy,1);
			new_rwy->Import(*rwy, LazyPrintf, &log);
		}

		for (AptSealaneVector::iterator sea = apt->sealanes.begin(); sea != apt->sealanes.end(); ++sea)
		{
			WED_Sealane *		new_sea = WED_Sealane::CreateTyped(archive);
			WED_RunwayNode *	source = WED_RunwayNode::CreateTyped(archive);
			WED_RunwayNode *	target = WED_RunwayNode::CreateTyped(archive);
			new_sea->SetParent(new_apt,new_apt->CountChildren());
			source->SetParent(new_sea,0);
			target->SetParent(new_sea,1);
			new_sea->Import(*sea, LazyPrintf, &log);
		}

		for (AptHelipadVector::iterator hel = apt->helipads.begin(); hel != apt->helipads.end(); ++hel)
		{
			WED_Helipad * new_hel = WED_Helipad::CreateTyped(archive);
			new_hel->SetParent(new_apt,new_apt->CountChildren());
			new_hel->Import(*hel, LazyPrintf, &log);
		}

		for (AptTaxiwayVector::iterator tax = apt->taxiways.begin(); tax != apt->taxiways.end(); ++tax)
		{
			WED_Taxiway * new_tax = WED_Taxiway::CreateTyped(archive);
			new_tax->SetParent(new_apt,new_apt->CountChildren());
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
			new_bou->SetParent(new_apt,new_apt->CountChildren());
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
			ImportLinearPath(lin->area, archive, new_apt, &new_lin, LazyPrintf, &log);
			for (vector<WED_AirportChain *>::iterator li = new_lin.begin(); li != new_lin.end(); ++li)
				(*li)->Import(*lin, LazyPrintf, &log);
		}

		for (AptLightVector::iterator lit = apt->lights.begin(); lit != apt->lights.end(); ++lit)
		{
			WED_LightFixture * new_lit = WED_LightFixture::CreateTyped(archive);
			new_lit->SetParent(new_apt,new_apt->CountChildren());
			new_lit->Import(*lit, LazyPrintf, &log);
		}

		for (AptSignVector::iterator sin = apt->signs.begin(); sin != apt->signs.end(); ++sin)
		{
			WED_AirportSign * new_sin = WED_AirportSign::CreateTyped(archive);
			new_sin->SetParent(new_apt,new_apt->CountChildren());
			new_sin->Import(*sin, LazyPrintf, &log);
		}

		for (AptGateVector::iterator gat = apt->gates.begin(); gat != apt->gates.end(); ++gat)
		{
			WED_RampPosition * new_gat = WED_RampPosition::CreateTyped(archive);
			new_gat->SetParent(new_apt,new_apt->CountChildren());
			new_gat->Import(*gat, LazyPrintf, &log);
		}

		if (apt->tower.draw_obj != -1)
		{
			WED_TowerViewpoint * new_twr = WED_TowerViewpoint::CreateTyped(archive);
			new_twr->SetParent(new_apt,new_apt->CountChildren());
			new_twr->Import(apt->tower, LazyPrintf, &log);
		}

		if (apt->beacon.color_code != apt_beacon_none)
		{
			WED_AirportBeacon * new_bea = WED_AirportBeacon::CreateTyped(archive);
			new_bea->SetParent(new_apt,new_apt->CountChildren());
			new_bea->Import(apt->beacon, LazyPrintf, &log);
		}

		for (AptWindsockVector::iterator win = apt->windsocks.begin(); win != apt->windsocks.end(); ++win)
		{
			WED_Windsock * new_win = WED_Windsock::CreateTyped(archive);
			new_win->SetParent(new_apt,new_apt->CountChildren());
			new_win->Import(*win, LazyPrintf, &log);
		}

		for (AptATCFreqVector::iterator atc = apt->atc.begin(); atc != apt->atc.end(); ++atc)
		{
			WED_ATCFrequency * new_atc = WED_ATCFrequency::CreateTyped(archive);
			new_atc->SetParent(new_apt,new_apt->CountChildren());
			new_atc->Import(*atc, LazyPrintf, &log);
		}
		
#if AIRPORT_ROUTING
		for(AptFlowVector::iterator flw = apt->flows.begin(); flw != apt->flows.end(); ++flw)
		{
			WED_ATCFlow * new_flw = WED_ATCFlow::CreateTyped(archive);
			new_flw->SetParent(new_apt,new_apt->CountChildren());
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
		
		map<int,WED_TaxiRouteNode *>	nodes;
		for(vector<AptRouteNode_t>::iterator n = apt->taxi_route.nodes.begin(); n != apt->taxi_route.nodes.end(); ++n)
		{
			WED_TaxiRouteNode * new_n = WED_TaxiRouteNode::CreateTyped(archive);
			new_n->SetParent(new_apt,new_apt->CountChildren());
			new_n->SetName(n->name);
			new_n->SetLocation(gis_Geo,n->location);
			nodes[n->id] = new_n;
			printf("Found node %s\n", n->name.c_str());
		}
		for(vector<AptRouteEdge_t>::iterator e = apt->taxi_route.edges.begin(); e != apt->taxi_route.edges.end(); ++e)
		{
			WED_TaxiRoute * new_e = WED_TaxiRoute::CreateTyped(archive);
			new_e->AddSource(nodes[e->src], 0);
			new_e->AddSource(nodes[e->dst], 1);
			new_e->SetParent(new_apt,new_apt->CountChildren());
			new_e->Import(*e,LazyPrintf, &log);
		}
#endif		
	}

	if (log.fi)
	{
		fclose(log.fi);
		DoUserAlert("There were problems during the import.  A log file has been created in the same file as the apt.dat file.");
	}
}

int		WED_CanImportApt(IResolver * resolver)
{
	return 1;
}

void	WED_DoImportApt(WED_Document * resolver, WED_Archive * archive, WED_MapPane * pane)
{
	vector<string>	fnames;
	
	#if GATEWAY_IMPORT_FEATURES
	
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
	
	#else
		char path[1024];
		strcpy(path,"");	
		if (GetFilePathFromUser(getFile_Open,"Import apt.dat...", "Import", FILE_DIALOG_IMPORT_APTDAT, path, sizeof(path)))
		fnames.push_back(path);
	#endif
	
	if(fnames.empty())
		return;
		
	AptVector	apts, one_apt;
	
	for(vector<string>::iterator f = fnames.begin(); f != fnames.end(); ++f)
	{
		string result = ReadAptFile(f->c_str(), one_apt);
		if (!result.empty())
		{
			string msg = string("Unable to read apt.dat file '") + path + string("': ") + result;
			DoUserAlert(msg.c_str());
			return;
		}
		
		apts.insert(apts.end(),one_apt.begin(),one_apt.end());
	}
	
	WED_AptImportDialog * importer = new WED_AptImportDialog(gApplication, apts, path, resolver, archive, pane);
}

void	WED_ImportOneAptFile(
				const string&			in_path,
				WED_Thing *				in_parent)
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
			NULL);
}
