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

#include "WED_Airport.h"
#include "WED_HandleToolBase.h"
#include "WED_MapZoomerNew.h"
#include "WED_ToolUtils.h"
#include "WED_Entity.h"
#include "WED_Colors.h"
#include "XESConstants.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "WED_DrawUtils.h"
#include "WED_Entity.h"
#include "IControlHandles.h"
#include "IResolver.h"
#include "WED_Thing.h"
#include "IGIS.h"
#include "ISelection.h"
#include "WED_GroupCommands.h"
#include "IOperation.h"
#include "WED_UIDefs.h"
#include "MathUtils.h"
#include "PlatformUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

// snap distance for control handles, in pixels
#define LINE_DIST 4
// selection circle radius for control handles, in pixels
#define	HANDLE_RAD 5

// distance in pixels a drag_Move needs to drag before actually moving selection, i.e. item is initially "sticky"
#define DRAG_START_DIST 6

#if DEV
#define DEBUG_PRINTF_N_LINES 0
#endif

// half width of selection box for nodes, aka node selection radius (but its actually a square box), in pixels
#define SELECTION_BOX_SIZE 8

// This util routine forms the line segment or bezier for a given "link" in a handles, converting from lat/lon to pixels.
// returns true for bezier, false for segment.
static bool	ControlLinkToCurve(
				IControlHandles *		h,
				intptr_t				ei,
				int						l,
				Bezier2&				b,
				Segment2&				s,
				WED_MapZoomerNew *		z)
{
	int sp,sc,tp,tc;
	sp = h->GetNthLinkSource   (ei,l);
	sc = h->GetNthLinkSourceCtl(ei,l);
	tp = h->GetNthLinkTarget   (ei,l);
	tc = h->GetNthLinkTargetCtl(ei,l);

	if (sc == -1 && tc == -1)
	{
		h->GetNthControlHandle(ei,sp, NULL, NULL, &s.p1, NULL, NULL);
		h->GetNthControlHandle(ei,tp, NULL, NULL, &s.p2, NULL, NULL);

		if (z)
		{
		s.p1.x_ = z->LonToXPixel(s.p1.x());
		s.p2.x_ = z->LonToXPixel(s.p2.x());
		s.p1.y_ = z->LatToYPixel(s.p1.y());
		s.p2.y_ = z->LatToYPixel(s.p2.y());
		}
		return false;
	}
	else
	{
		if(sc==-1)sc=sp;
		if(tc==-1)tc=tp;
		h->GetNthControlHandle(ei,sp,NULL,NULL,&b.p1, NULL, NULL);
		h->GetNthControlHandle(ei,sc,NULL,NULL,&b.c1, NULL, NULL);
		h->GetNthControlHandle(ei,tp,NULL,NULL,&b.p2, NULL, NULL);
		h->GetNthControlHandle(ei,tc,NULL,NULL,&b.c2, NULL, NULL);

		if (z) {
		b.p1.x_ = z->LonToXPixel(b.p1.x());
		b.c1.x_ = z->LonToXPixel(b.c1.x());
		b.p2.x_ = z->LonToXPixel(b.p2.x());
		b.c2.x_ = z->LonToXPixel(b.c2.x());
		b.p1.y_ = z->LatToYPixel(b.p1.y());
		b.c1.y_ = z->LatToYPixel(b.c1.y());
		b.p2.y_ = z->LatToYPixel(b.p2.y());
		b.c2.y_ = z->LatToYPixel(b.c2.y());
		}
		return true;
	}
}

inline bool within_seg(const Segment2& s, const Point2& p, double r)
{
	double rs = r*r;
	if (s.p1.squared_distance(p) < rs) return true;
	if (s.p2.squared_distance(p) < rs) return true;

	if (s.p1 == s.p2) return false;

	if (Vector2(s.p1, s.p2).dot(Vector2(s.p1, p)) < 0.0) return false;
	if (Vector2(s.p2, s.p1).dot(Vector2(s.p2, p)) < 0.0) return false;

	Line2 l(s);
	l.normalize();
	return fabs(l.distance_denormaled(p)) < r;
}

WED_HandleToolBase::WED_HandleToolBase(
				const char *			tool_name,
				GUI_Pane *				host,
				WED_MapZoomerNew *		zoomer,
				IResolver *				resolver) :
	WED_MapToolNew(tool_name, host, zoomer,resolver),
	mHandles(NULL),
	mDragType(drag_None),
	mCanSelect(1),
	mDrawAlways(0)
{

}

WED_HandleToolBase::~WED_HandleToolBase()
{
}

void				WED_HandleToolBase::SetCanSelect(int can_select)
{
	mCanSelect = can_select;
}

void				WED_HandleToolBase::SetDrawAlways(int can_draw_always)
{
	mDrawAlways = can_draw_always;
}


void		WED_HandleToolBase::SetControlProvider(IControlHandles	* provider)
{
	mHandles = provider;
}

int			WED_HandleToolBase::HandleClickDown			(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
	if (inButton > 0) return 0;
	mDragX = inX;
	mDragY = inY;
	mSelX = inX;
	mSelY = inY;

	Point2	click_pt(inX, inY);	//GetZoomer()->XPixelToLon(inX),GetZoomer()->YPixelToLat(inY));

	int ei_count = mHandles ? mHandles->CountEntities() : 0;
	int  c_count;
	int  l_count;
	intptr_t eid;
	int ei, n;

	DebugAssert(mDragType == drag_None);
	mDragType = drag_None;

	//------------------------------ CONTROL HANDLE TAG-UP -------------------------------------------------------

	mHandleEntity = -1;
	mHandleIndex = -1;
	double best_dist = 9.9e9;
	double this_dist;

	for (ei = 0; ei < ei_count; ++ei)
	{
		eid = mHandles->GetNthEntityID(ei);
		c_count = mHandles->CountControlHandles(eid);
		for (n = 0; n < c_count; ++n)
		{
			bool active;
			Point2	cloc;
			Point2	cloc_pixels;
			HandleType_t ht;
			float radius = HANDLE_RAD;
			mHandles->GetNthControlHandle(eid, n, &active, &ht, &cloc, NULL, &radius);
			if (!active) continue;

			radius *= radius;
			cloc_pixels = GetZoomer()->LLToPixel(cloc);
			if ((this_dist=click_pt.squared_distance(cloc_pixels)) < best_dist)
			if (this_dist < radius)
			{
				mHandleEntity = eid;
				mHandleIndex = n;
				best_dist = this_dist;
				if (mDragType == drag_None)
					mHandles->BeginEdit();
				mDragType = drag_Handles;
				mTrackPoint = cloc;
			}
		}
	}

	//-------------------------------- CONTROL LINK TAG-UP -------------------------------------------------------

	if (mDragType == drag_None && ei_count > 0)
		for (ei = 0; ei < ei_count && mDragType == drag_None; ++ei)
		{
			eid = mHandles->GetNthEntityID(ei);
			l_count = mHandles->GetLinks(eid);
			for (n = 0; n < l_count; ++n)
			{
				bool active;
				mHandles->GetNthLinkInfo(eid,n,&active, NULL);
				if (!active) continue;

				Bezier2		b;
				Segment2	s;
				if (ControlLinkToCurve(mHandles,eid,n,b,s,GetZoomer()))
				{
					if (b.is_near(click_pt, LINE_DIST))
					{
						mHandleIndex = n;
						mDragType = drag_Links;
						mHandleEntity = eid;
						mHandles->BeginEdit();
						mTrackPoint = GetZoomer()->PixelToLL(click_pt);
						break;
					}
				}
				else
				{
					if (within_seg(s,click_pt,LINE_DIST))
					{
						mHandleIndex = n;
						mDragType = drag_Links;
						mHandleEntity = eid;
						mHandles->BeginEdit();
						mTrackPoint = GetZoomer()->PixelToLL(click_pt);
						break;
					}
				}
			}
		}
	//----------------------------------- ENTITY DRAG ------------------------------------------------------------

	click_pt = GetZoomer()->PixelToLL(click_pt);
	if (mDragType == drag_None && ei_count > 0)
		for (ei = 0; ei < ei_count; ++ei)
		{
			eid = mHandles->GetNthEntityID(ei);
			if (mHandles->PointOnStructure(eid, click_pt))
			{
				mDragType = drag_PreEnt;
				mHandleEntity = eid;
				mHandles->BeginEdit();
				mTrackPoint = click_pt;
				break;
			}
		}

	//----------------------------------- CREATION DRAG ----------------------------------------------------------

	if (mDragType == drag_None && this->CreationDown(click_pt))
	{
		mDragType = drag_Create;
	}

	//------------------------------------- SELECTION-DRAG -------------------------------------------------------

	if (mDragType == drag_None && mCanSelect)
	{
		mSelManip.clear();
		bool has_click = false;
		ISelection * sel = WED_GetSelect(GetResolver());
		set<ISelectable *> sel_set;
		sel->GetSelectionSet(sel_set);
		WED_Thing * t, *p;
		IGISEntity * e;
		for(set<ISelectable *>::iterator s = sel_set.begin(); s != sel_set.end(); ++s)
		if ((t = dynamic_cast<WED_Thing *>(*s)) != NULL)
		if ((e = dynamic_cast<IGISEntity *>(*s)) != NULL)
		{
			bool par_sel = false;
			p = t->GetParent();
			while (p)
			{
				if (sel_set.count(p)) { par_sel = true; break; }
				p = p->GetParent();
			}
			if (!par_sel)
			{
				mSelManip.push_back(e);
				if (!has_click)
				{
					if(!IsLockedNow(t))
					if(IsVisibleNow(t))
					{
						double	frame_dist = fabs(GetZoomer()->YPixelToLat(0)-GetZoomer()->YPixelToLat(3));
						double	icon_dist_v = fabs(GetZoomer()->YPixelToLat(0)-GetZoomer()->YPixelToLat(GetFurnitureIconRadius()));
						double	icon_dist_h = fabs(GetZoomer()->XPixelToLon(0)-GetZoomer()->XPixelToLon(GetFurnitureIconRadius()));
						double	max_slop_h = max(icon_dist_h,frame_dist);
						double	max_slop_v = max(icon_dist_v,frame_dist);
						if(WED_IsIconic(e))
							frame_dist = max(icon_dist_h,icon_dist_v);

						Bbox2		ent_bounds;
						e->GetBounds(gis_Geo,ent_bounds);
						ent_bounds.p1 -= Vector2(max_slop_h,max_slop_v);
						ent_bounds.p2 += Vector2(max_slop_h,max_slop_v);
						if (ent_bounds.contains(click_pt))
						{
							if (e->PtWithin(gis_Geo,click_pt) || e->PtOnFrame(gis_Geo,click_pt, frame_dist))
								has_click = true;
						}
					}
				}
			}
		}
		if (has_click)
		{
			mDragType = drag_PreMove;
			mTrackPoint = click_pt;
			IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));
			if (GetHost()->GetModifiersNow() & gui_OptionAltFlag)
			{
				op->StartOperation("Copy");
				WED_DoDuplicate(GetResolver(), false);	// dupe does not keep command open!

				// Yuck - we have to rebuild our cache of selected things now!
				mSelManip.clear();
				sel_set.clear();
				sel->GetSelectionSet(sel_set);
				for(set<ISelectable *>::iterator s = sel_set.begin(); s != sel_set.end(); ++s)
				if ((t = dynamic_cast<WED_Thing *>(*s)) != NULL)
				if ((e = dynamic_cast<IGISEntity *>(*s)) != NULL)
				{
					bool par_sel = false;
					p = t->GetParent();
					while (p)
					{
						if (sel_set.count(p)) { par_sel = true; break; }
						p = p->GetParent();
					}
					if (!par_sel)
					{
						mSelManip.push_back(e);
					}
				}
			}
			else
			op->StartOperation("Drag");

		}
		else
			mSelManip.clear();
	}

	//------------------------------------- SELECTION ------------------------------------------------------------

	if (mDragType == drag_None && mCanSelect)
	{
		mDragType = drag_Sel;

		mDragX = inX;
		mDragY = inY;
		IGISEntity * ent_base = SAFE_CAST(IGISEntity, WED_GetWorld(GetResolver()));
		ISelection * sel = SAFE_CAST(ISelection, WED_GetSelect(GetResolver()));
		IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));

		SetAnchor1(Point2(GetZoomer()->XPixelToLon(mDragX),GetZoomer()->YPixelToLat(mDragY)));

		if (sel && ent_base)
		{
			set<IGISEntity *>	sel_set;
			Bbox2	bounds(
							GetZoomer()->XPixelToLon(mDragX),
							GetZoomer()->YPixelToLat(mDragY),
							GetZoomer()->XPixelToLon(inX),
							GetZoomer()->YPixelToLat(inY));

			ProcessSelection(ent_base, bounds, sel_set);
			if(op) op->StartOperation("Change Selection");

			GUI_KeyFlags mods = GetHost()->GetModifiersNow();
			if ((mods & (gui_ShiftFlag + gui_ControlFlag)) == 0)
				sel->Clear();
			mSelToggle = (mods & gui_ControlFlag) != 0;

			sel->GetSelectionVector(mSelSave);

			for (set<IGISEntity *>::iterator i = sel_set.begin(); i != sel_set.end(); ++i)
			if (mSelToggle)
				sel->Toggle((*i));
			else
				sel->Insert((*i));


			#if BENTODO
				right vs left select does touch vs include?
			#endif
		}
	}

	if (mDragType != drag_None)
		GUI_Commander::RegisterNotifiable(this);

	return (mDragType != drag_None);
}

#if BENTODO
doc and clean this
/*
This seems to be the point where a selections starts. Its goes down recursively in the project hierachy to see if something selectable
is within reach. If so, its added to "result" returns 1 to indicate something was found.

The selection has 2 different modes of operation: For selection boxes that are a point (i.e. a single click) it selectes anything
i.e. bounding box around and objects or line/point/node hit dead on (some slop is actually always added), but it aborts upon the very 
first hit.

If the bounds are an area, i.e. a drag-click or marquee selection, it ignores all area-style objects that only overlap the selection,
but takes all point objects, plus all area objects that are completely enclosed in the bounds.

To add type-based selection, the search now never stops upon a first find. But rather adds a layer ontop, that in case of 
single-point searches filter the result and only keeps a single find - according to the a certain "selection priority list".

As this select function is also used to add/extend existing selections, this also requires the initail selection set to be saved
and only "new" finds after the filtering to be added.

*/
#endif


void WED_HandleToolBase::ProcessSelection(
							IGISEntity *		entity,
							Bbox2&				bounds,
							set<IGISEntity *>&	result)
{
	Point2 sel_p1(bounds.p1);
	bool pt_sel(bounds.is_point());

	double	icon_dist_h = fabs(GetZoomer()->XPixelToLon(0)-GetZoomer()->XPixelToLon(SELECTION_BOX_SIZE));
	double	icon_dist_v = fabs(GetZoomer()->YPixelToLat(0)-GetZoomer()->YPixelToLat(SELECTION_BOX_SIZE));
	if(pt_sel) bounds.expand(icon_dist_h,icon_dist_v); // select things even a bit further away
	
//	set<IGISEntity *> result_old;
//	if(pt_sel) { result_old = result; result.clear(); } // start from scratch, so can filter only the new selections later
#if DEBUG_PRINTF_N_LINES
	gMeshLines.clear();
#else
	#define printf(x)
#endif
	ProcessSelectionRecursive(entity, bounds, pt_sel, icon_dist_h, icon_dist_v, result);
	
	if(pt_sel)             // filter list, keep only 1 result, selected by a special priority list
	{
		IGISEntity * keeper = NULL;
		if(!keeper)
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)
			{
				if( (*i)->GetGISClass() ==  gis_Composite &&                                     //  for Marquee tool only, prevents selecting the world
					strcmp((*i)->GetGISSubtype(), "WED_AirportBoundary") == 0 )  { printf("AirportBdry\n"); keeper = *i;  break; }
			}
		if(!keeper)
		{	vector<IGISEntity *> candidates;
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)         // anything point-like comes first
			{
				if( (*i)->GetGISClass() == gis_Point ||
					(*i)->GetGISClass() == gis_Point_Bezier ||
					(*i)->GetGISClass()	== gis_Point_Heading ||
					(*i)->GetGISClass()	== gis_Point_HeadingWidthLength ) { printf("Point\n");  candidates.push_back(*i); }
			}
			if(candidates.size() == 1)
				keeper = candidates.front();
			else if(candidates.size() > 1)                                                       // find the closest point
			{
				double min_distance = 1.0E9;
				for(vector<IGISEntity *> ::iterator i = candidates.begin(); i != candidates.end(); ++i)  // anything point-like comes first
				{
					IGISPoint * poi = SAFE_CAST(IGISPoint, *i);
					Point2 pos;
					poi->GetLocation(gis_Geo,pos);
					if (sel_p1.squared_distance(pos) < min_distance)
					{
						min_distance = sel_p1.squared_distance(pos);
						keeper = *i;
					}
				}
			}
		}
		if(!keeper)
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)     // area-type objects that create 3D "above ground" stuff
			{
				if( ((*i)->GetGISClass() ==  gis_Polygon || (*i)->GetGISClass() ==  gis_Composite) && (
					strcmp((*i)->GetGISSubtype(), "WED_ForestPlacement") == 0 ||
					strcmp((*i)->GetGISSubtype(), "WED_FacadePlacement") == 0 ) ) { printf("Forest/Facade\n"); keeper = *i;  break; }
			}
		if(!keeper)
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)     // then any kind of line-type objects
			{
				if( (*i)->GetGISClass() ==  gis_Line || 
					(*i)->GetGISClass() ==  gis_Edge ||
					(*i)->GetGISClass() ==  gis_Ring ||                                      // APT Boundaries, but only the ring part, not the inner area
					(*i)->GetGISClass() ==  gis_Chain   ) { printf("Line\n");  keeper = *i;  break; }
			}
		if(!keeper)
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)     // now the polygons, in similar order as typical LAYER_GROUP assignments
			{
				if( ((*i)->GetGISClass() ==  gis_Polygon || (*i)->GetGISClass() ==  gis_Composite) && (
					strcmp((*i)->GetGISSubtype(),"WED_PolygonPlacement") == 0 ||                           // Textured Polys
					strcmp((*i)->GetGISSubtype(),"WED_DrapedOrthophoto") == 0 ) ) { printf("Polygon\n"); keeper = *i;  break; } // Draped Polys - also Ground Painted Signs
			}
		if(!keeper)
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)     // Runways
			{
				if( (*i)->GetGISClass() ==  gis_Line_Width )                      { printf("Runway\n"); keeper = *i;  break; }
			}
		if(!keeper)
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)
			{
				if( (*i)->GetGISClass() ==  gis_Polygon ||
					(*i)->GetGISClass() ==  gis_Area )                            { printf("Taxiway\n"); keeper = *i;  break; }     // all other polygons and areas - like Taxiways
			}                                                                                                             // consider it a catch-all
		if(!keeper)
			for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)
			{
				if( (*i)->GetGISClass() ==  gis_BoundingBox )                       { printf("Exclusion\n"); keeper = *i;  break; }              //  thats exclusion zones
			}

		//	gis_PointSequence,  gis_Composite - not expected to ever come up.
		
#undef printf
#if DEBUG_PRINTF_N_LINES
		for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)
			printf("Total selected GISClass #%d Subtype %s\n", (*i)->GetGISClass()-gis_Point, (*i)->GetGISSubtype());
#endif		
		if(keeper) { result.clear(); result.insert(keeper); }    // ok found something from our priority list, only keep that one
#if DEBUG_PRINTF_N_LINES
		else
		{
			if (result.empty())
				printf("FYI, nothing to select here.\n");
			else
			{	
				printf("duh - this should not happen. Multiple items are selected,\nbut none of them are in the priority list for object selection:\n");
				for(set<IGISEntity *> ::iterator i = result.begin(); i != result.end(); ++i)
					printf("Selected are GISClass #%d Subtype %s\n", (*i)->GetGISClass()-gis_Point, (*i)->GetGISSubtype());
			}
		}
#endif		
//	result.insert(result_old.begin(), result_old.end());   // merge back in what we took out initially
	}
}


void WED_HandleToolBase::ProcessSelectionRecursive(
							IGISEntity *	entity,
							const Bbox2&	bounds,
							int				pt_sel,
							double			icon_dist_h,
							double			icon_dist_v,
							set<IGISEntity *>&	result)
{
	Point2	psel; if(pt_sel) psel = bounds.centroid();
	double	frame_dist  = icon_dist_v/2;

	{   //  speedup: do not traverse into entities which have their own bounding box already out of reach
		Bbox2	ent_bounds;
		entity->GetBounds(gis_Geo,ent_bounds);
		ent_bounds.expand(icon_dist_h,icon_dist_v);
#if DEBUG_PRINTF_N_LINES
		#define DBG_LIN_COLOR .4,0,.4,.4,0,.4
		debug_mesh_segment(ent_bounds.right_side(), DBG_LIN_COLOR);
		debug_mesh_segment(ent_bounds.top_side(),   DBG_LIN_COLOR);
		debug_mesh_segment(ent_bounds.left_side(),  DBG_LIN_COLOR);
		debug_mesh_segment(ent_bounds.bottom_side(),DBG_LIN_COLOR);
#endif

		if (pt_sel) { if (!ent_bounds.contains(psel))	return; }
		else		{ if (!ent_bounds.overlap(bounds))	return; }
	}

	if(!IsVisibleNow(entity))	return;
	if(IsLockedNow(entity))		return;

	// if(is_root && pt_sel) bounds.expand(icon_dist_h,icon_dist_v); // select things even a bit further away
		
	EntityHandling_t choice = TraverseEntity(entity,pt_sel);
	IGISComposite *     com = SAFE_CAST(IGISComposite, entity);
	IGISPointSequence * seq = SAFE_CAST(IGISPointSequence, entity);
	IGISPolygon *      poly = SAFE_CAST(IGISPolygon, entity);
	if(com  &&  com->GetGISClass() != gis_Composite) com = NULL;
	if(seq  &&  seq->GetGISClass() == gis_Composite) seq = NULL;
	if(poly && poly->GetGISClass() != gis_Polygon)  poly = NULL;

	switch(choice) {
	case ent_Atomic:
//		if(entity->IntersectsBox(gis_Geo,bounds))                 result.insert(entity);   // includes the inner area of BoundingBoxes aka Exclusions
		if(entity->WithinBox(gis_Geo,bounds))                     result.insert(entity);   // excludes the inner area of BoundingBoxes aka Exclusions
		if(pt_sel && entity->PtOnFrame(gis_Geo,psel, frame_dist)) result.insert(entity);
		break;
	case ent_Container:
		if (pt_sel && entity->PtOnFrame(gis_Geo,psel, frame_dist))  result.insert(entity);   // select the GisComposite as well, for Forest and Facade Chains

		if (com)
		{
			int count = com->GetNumEntities();
			for (int n = 0; n < count; ++n)
				ProcessSelectionRecursive(com->GetNthEntity(n),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
		}
		else if (seq)
		{
			int count = seq->GetNumPoints();
			for (int n = 0; n < count; ++n)
				ProcessSelectionRecursive(seq->GetNthPoint(n),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
		}
		else if (poly)
		{
			int count = poly->GetNumHoles();
			ProcessSelectionRecursive(poly->GetOuterRing(),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
			for (int n = 0; n < count; ++n)
				ProcessSelectionRecursive(poly->GetNthHole(n),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
		}
		break;
	case ent_AtomicOrContainer:
		if ( !pt_sel &&  entity->WithinBox(gis_Geo,bounds) ) // select the container, if possible instead of its innards
			result.insert(entity); 
		else if (com)
		{
			int count = com->GetNumEntities();
			for (int n = 0; n < count; ++n)
				ProcessSelectionRecursive(com->GetNthEntity(n),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
		}
		else if (seq)
		{
			int count = seq->GetNumPoints();
			for (int n = 0; n < count; ++n)
				ProcessSelectionRecursive(seq->GetNthPoint(n),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
		}
		else if (poly)
		{
			int count = poly->GetNumHoles();
			ProcessSelectionRecursive(poly->GetOuterRing(),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
			for (int n = 0; n < count; ++n)
				ProcessSelectionRecursive(poly->GetNthHole(n),bounds,pt_sel, icon_dist_h, icon_dist_v, result);
		}
		if (pt_sel && entity->PtWithin(gis_Geo,psel))				result.insert(entity);
		if (pt_sel && entity->PtOnFrame(gis_Geo,psel, frame_dist))  result.insert(entity);
		break;
	}
	return;
}


void		WED_HandleToolBase::HandleClickDrag			(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
	if (inButton > 0) return;

	switch(mDragType) {
	case drag_PreEnt:
		if(WantSticky())
		{
			Point2 mSel(mSelX, mSelY);
			double drag_dist = mSel.squared_distance(Point2(inX,inY));

			if (drag_dist <	DRAG_START_DIST * DRAG_START_DIST)	// see if we drag'd far enough to "break loose" and actually move the object
			{
				break;
			}
		}
		mDragType = drag_Ent;
	case drag_Handles:
	case drag_Links:
	case drag_Ent:
		{
			Point2	op(GetZoomer()->XPixelToLon(mDragX),GetZoomer()->YPixelToLat(mDragY));
			Point2	np(GetZoomer()->XPixelToLon(   inX),GetZoomer()->YPixelToLat(   inY));
			Vector2 delta(op,np);
			mDragX = inX; mDragY = inY;
			switch(mDragType) {
			case drag_Handles:				mHandles->ControlsHandlesBy(mHandleEntity,	mHandleIndex,	delta, mTrackPoint);	break;
			case drag_Links:				mHandles->ControlsLinksBy(mHandleEntity,	mHandleIndex,	delta, mTrackPoint);	break;
			case drag_Ent:					mHandles->ControlsMoveBy(mHandleEntity,						delta, mTrackPoint);	break;
			}
		}
		break;
	case drag_Create:
		this->CreationDrag(
					GetZoomer()->PixelToLL(Point2(mDragX, mDragY)),
					GetZoomer()->PixelToLL(Point2(inX, inY)));
		break;
	case drag_Move:
		{
			Point2	op(GetZoomer()->XPixelToLon(mDragX),GetZoomer()->YPixelToLat(mDragY));
			Point2	np(GetZoomer()->XPixelToLon(   inX),GetZoomer()->YPixelToLat(   inY));
			Vector2 delta(op,np);
			mDragX = inX; mDragY = inY;
			for (vector<IGISEntity *>::iterator e =	mSelManip.begin(); e != mSelManip.end(); ++e)
			{
				Bbox2	old_b;
				(*e)->GetBounds(gis_Geo,old_b);
				Bbox2	new_b(old_b);
				new_b+=delta;
				(*e)->Rescale(gis_Geo,old_b, new_b);
			}
		}
		break;
	case drag_PreMove:
		{
			Point2 mSel(mSelX, mSelY);
			double drag_dist = mSel.squared_distance(Point2(inX,inY));

			if (drag_dist >	DRAG_START_DIST * DRAG_START_DIST)	// see if we drag'd far enough to "break loose" and actually move the object
			{
				mDragType = drag_Move;
				break;
			}
			else if(drag_dist > 0.0)                            // we've drag'd some, but not much. Do nothing, for now.
				break;
			else 			// its a drag-move, but we really did not drag a single pixel. So we instead execute a single click select
			{
				IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));
				if(op)
				{
					op->AbortOperation();
					op->StartOperation("Change Selection");
				}
				mDragType = drag_Sel;
			}
		}
	case drag_Sel:
		{
			mSelX = inX;
			mSelY = inY;
			SetAnchor2(Point2(GetZoomer()->XPixelToLon(inX),GetZoomer()->YPixelToLat(inY)));
			IGISEntity * ent_base = SAFE_CAST(IGISEntity, WED_GetWorld(GetResolver()));
			ISelection * sel = SAFE_CAST(ISelection, WED_GetSelect(GetResolver()));
			if (sel && ent_base)
			{
				set<IGISEntity *>	sel_set;
				Bbox2	bounds(
								GetZoomer()->XPixelToLon(mDragX),
								GetZoomer()->YPixelToLat(mDragY),
								GetZoomer()->XPixelToLon(inX),
								GetZoomer()->YPixelToLat(inY));

				ProcessSelection(ent_base, bounds, sel_set);

				sel->Clear();
				for (vector<ISelectable *>::iterator u = mSelSave.begin(); u != mSelSave.end(); ++u)
					sel->Insert(*u);
				#if OPTIMIZE
					provide accelerated sel-save-restore ops!
				#endif

				for (set<IGISEntity *>::iterator i = sel_set.begin(); i != sel_set.end(); ++i)
				if (mSelToggle)
					sel->Toggle((*i));
				else
					sel->Insert((*i));
			}
		}
		break;
	}
}

void		WED_HandleToolBase::HandleClickUp			(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
	if (inButton > 0) return;

	this->HandleClickDrag(inX, inY, inButton, modifiers);
	if (mDragType == drag_Move)
	{
		ISelection * sel = WED_GetSelect(GetResolver());
		IOperation * op = SAFE_CAST(IOperation, sel);
		if (op)
		{
			int includes_airport = sel->IterateSelectionOr(Iterate_IsClass, (void*) WED_Airport::sClass);
			if (includes_airport)
			{
				if(ConfirmMessage("This will move a whole Airport !", "Yes, move it", "No, cancel move"))
					op->CommitOperation();
				else
					op->AbortOperation();
			}
			else
				op->CommitOperation();
		}
		mSelManip.clear();
	}
	else if (mDragType == drag_PreMove || mDragType == drag_PreEnt)
	{
		IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));
		if(op) op->AbortOperation();
		mSelManip.clear();
	} 
	else if (mDragType == drag_Sel)
	{
		ClearAnchor1();
		ClearAnchor2();
		IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));
		if(op) op->CommitOperation();
		mSelSave.clear();
	} else if ( mDragType == drag_Links ||
				mDragType == drag_Handles ||
				mDragType == drag_Ent)
	{
		mHandles->EndEdit();
	} else if ( mDragType == drag_Create )
		this->CreationUp(
					GetZoomer()->PixelToLL(Point2(mDragX, mDragY)),
					GetZoomer()->PixelToLL(Point2(inX, inY)));

	GUI_Commander::UnregisterNotifiable(this);
	mDragType = drag_None;
}

int			WED_HandleToolBase::HandleToolKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	return 0;
}

void		WED_HandleToolBase::KillOperation(bool mouse_is_down)
{
	if (mDragType == drag_Sel)
	{
		IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));
		if(op) op->AbortOperation();
		mSelSave.clear();
	} else if ( mDragType == drag_Links ||
				mDragType == drag_Handles ||
				mDragType == drag_Ent ||
				mDragType == drag_PreEnt )
	{
		mHandles->EndEdit();
	}
	GUI_Commander::UnregisterNotifiable(this);
	mDragType = drag_None;
}

void		WED_HandleToolBase::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}

void		WED_HandleToolBase::DrawStructure			(bool inCurrent, GUI_GraphState * g)
{
	if (!inCurrent && !mDrawAlways) return;
	if (mHandles != NULL)
	{
		int ei_count = mHandles->CountEntities();
		for (int ei = 0; ei < ei_count; ++ei)
		{
			intptr_t eid = mHandles->GetNthEntityID(ei);

			int ch_count = mHandles->CountControlHandles(eid);
			int ch_links = mHandles->GetLinks(eid);

			g->SetState(false,false,false, false,true, false,false);

			glBegin(GL_LINES);
			for (int l = 0; l < ch_links; ++l)
			{
				Segment2	s;
				Bezier2		b;

				LinkType_t lt;
				mHandles->GetNthLinkInfo(eid,l,NULL, &lt);
				if (lt != link_None)
				{
					switch(lt) {
					case link_Solid:		glColor4fv(WED_Color_RGBA(wed_Link));			break;
					case link_Ghost:		glColor4fv(WED_Color_RGBA(wed_GhostLink));		break;
					case link_BezierCtrl:	glColor4fv(WED_Color_RGBA(wed_ControlLink));	break;
					case link_Marquee:		glColor4fv(WED_Color_RGBA(wed_Marquee));		break;
					}
					if (ControlLinkToCurve(mHandles,eid,l,b,s,GetZoomer()))
					{
						int point_count = BezierPtsCount(b,GetZoomer());

						for (int n = 0; n < point_count; ++n)
						{
							float t1 = (float) n / (float) point_count;
							float t2 = (float) (n+1) / (float) point_count;
							Point2	p1 = b.midpoint(t1);
							Point2	p2 = b.midpoint(t2);
							glVertex2d(p1.x(),p1.y());
							glVertex2d(p2.x(),p2.y());
						}
					}
					else
					{
						glVertex2d(s.p1.x(),s.p1.y());
						glVertex2d(s.p2.x(),s.p2.y());
					}
				}
			}
			glEnd();

			if(inCurrent)
			for (int cp = 0; cp < ch_count; ++cp)
			{
				Vector2		dir;
				Point2	cpt, scrpt;
				HandleType_t	ht;
				mHandles->GetNthControlHandle(eid,cp,NULL, &ht, &cpt, &dir, NULL);
				scrpt = GetZoomer()->LLToPixel(cpt);

				Vector2	orient;

				if (ht == handle_None || ht == handle_Icon) continue;

				glColor4fv(WED_Color_RGBA(wed_ControlHandle));


				if (ht == handle_ArrowHead || ht == handle_Arrow || ht == handle_Bezier || ht == handle_RotateHead || ht == handle_Rotate)
				{
					Point2 bscrp = GetZoomer()->LLToPixel(cpt - dir);
					if (ht == handle_Arrow || ht == handle_Rotate)
					{
						g->SetState(0,0,0,   0, 0, 0, 0);
						glBegin(GL_LINES);
						glVertex2d(bscrp.x(), bscrp.y());
						glVertex2d(scrpt.x(), scrpt.y());
						glEnd();
					}
					orient = Vector2(bscrp,scrpt);
				}

				switch(ht) {
				case handle_Square:			GUI_PlotIcon(g,"handle_square.png", scrpt.x(),scrpt.y(),0,1.0);		break;
				case handle_Vertex:			GUI_PlotIcon(g,"handle_vertexround.png", scrpt.x(),scrpt.y(),0,1.0);break;
				case handle_VertexSharp:	GUI_PlotIcon(g,"handle_vertexsharp.png", scrpt.x(),scrpt.y(),0,1.0);break;
				case handle_Bezier:			GUI_PlotIcon(g,"handle_control.png", scrpt.x(),scrpt.y(),atan2(orient.dx,orient.dy) * RAD_TO_DEG,1.0);	break;
				case handle_ClosePt:		GUI_PlotIcon(g,"handle_closeloop.png", scrpt.x(),scrpt.y(),0,1.0);	break;
				case handle_Cross:			GUI_PlotIcon(g,"handle_cross.png", scrpt.x(),scrpt.y(),0,1.0);		break;
				case handle_ArrowHead:
				case handle_Arrow:			GUI_PlotIcon(g,"handle_arrowhead.png", scrpt.x(),scrpt.y(),atan2(orient.dx,orient.dy) * RAD_TO_DEG,1.0);	break;
				case handle_RotateHead:
				case handle_Rotate:			GUI_PlotIcon(g,"handle_rotatehead.png", scrpt.x(),scrpt.y(),atan2(orient.dx,orient.dy) * RAD_TO_DEG,1.0);	break;
				}
			}
		}
	}
	if (mDragType == drag_Sel)
	{
		g->SetState(false,false,false, false,false, false,false);
		glColor4fv(WED_Color_RGBA(wed_Marquee));
		glBegin(GL_LINE_LOOP);
		glVertex2i(min(mDragX, mSelX),min(mDragY,mSelY));
		glVertex2i(min(mDragX, mSelX),max(mDragY,mSelY));
		glVertex2i(max(mDragX, mSelX),max(mDragY,mSelY));
		glVertex2i(max(mDragX, mSelX),min(mDragY,mSelY));
		glEnd();
	}
}

void		WED_HandleToolBase::PreCommandNotification(GUI_Commander * focus_target, int command)
{
	if (mDragType == drag_Sel)
	{
		IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));
		if(op) op->CommitOperation();
		mSelSave.clear();
	} else if ( mDragType == drag_Links ||
				mDragType == drag_Handles ||
				mDragType == drag_Ent ||
				mDragType == drag_PreEnt )
	{
		mHandles->EndEdit();
	}
	mDragType = drag_None;
	GUI_Commander::UnregisterNotifiable(this);
}

