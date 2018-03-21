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

#include "WED_MarqueeTool.h"
#include "ISelection.h"
#include "IResolver.h"
#include "WED_ToolUtils.h"
#include "WED_MapZoomerNew.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_Ring.h"
#include "AssertUtils.h"
#include "IGIS.h"
#include "WED_GroupCommands.h"
#include "GISUtils.h"

//	HANDLES			LINKS
// 2-3-4			+2-3+
// |   |			1	4
// 1 8 5			|   |
// |   |			0	5
// 0-7-6			+7-6+

// This maps the relative contribution of a box corner to a handle.  So the 0th handle
// is made entirely of the first point (for both X and Y).
static const double kControlsX1[9] = {	1.0, 1.0, 1.0, 0.5, 0.0, 0.0, 0.0, 0.5, 0.5 };
static const double kControlsX2[9] = {	0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5 };
static const double kControlsY1[9] = {	1.0, 0.5, 0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 0.5 };
static const double kControlsY2[9] = {	0.0, 0.5, 1.0, 1.0, 1.0, 0.5, 0.0, 0.0, 0.5 };

// How much to transform each point given a handle move!
static const double kApplyCtrlX1[9] = { 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
static const double kApplyCtrlX2[9] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0 };
static const double kApplyCtrlY1[9] = { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
static const double kApplyCtrlY2[9] = { 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

static const double kApplyCtrlCtrX1[9] = { 1.0, 1.0, 1.0, 0.0,-1.0,-1.0,-1.0, 0.0, 1.0 };
static const double kApplyCtrlCtrX2[9] = {-1.0,-1.0,-1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0 };
static const double kApplyCtrlCtrY1[9] = { 1.0, 0.0,-1.0,-1.0,-1.0, 0.0, 1.0, 1.0, 1.0 };
static const double kApplyCtrlCtrY2[9] = {-1.0, 0.0, 1.0, 1.0, 1.0, 0.0,-1.0,-1.0, 1.0 };

static const double kApplyCtrlPropX1[9] = { 1.0, 1.0, 1.0,-0.5, 0.0, 0.0, 0.0, 0.5, 1.0 };
static const double kApplyCtrlPropX2[9] = { 0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0,-0.5, 1.0 };
static const double kApplyCtrlPropY1[9] = { 1.0, 0.5, 0.0, 0.0, 0.0,-0.5, 1.0, 1.0, 1.0 };
static const double kApplyCtrlPropY2[9] = { 0.0,-0.5, 1.0, 1.0, 1.0, 0.5, 0.0, 0.0, 1.0 };

static const double kApplyCtrlPropCtrX1[9] = { 1.0, 1.0, 1.0,-0.5,-1.0,-1.0,-1.0, 0.5, 1.0 };
static const double kApplyCtrlPropCtrX2[9] = {-1.0,-1.0,-1.0, 0.5, 1.0, 1.0, 1.0,-0.5, 1.0 };
static const double kApplyCtrlPropCtrY1[9] = { 1.0, 0.5,-1.0,-1.0,-1.0,-0.5, 1.0, 1.0, 1.0 };
static const double kApplyCtrlPropCtrY2[9] = {-1.0,-0.5, 1.0, 1.0, 1.0, 0.5,-1.0,-1.0, 1.0 };

// 1. Make CORNER prop work, probably average dx,dy for now, and then update prop and prop ctr tables
// 2. Modify icons for the non-drag case to make this easier to visualize.
// 3. Copy to TCE code too!

// How much to transform each point when we drag a LINK!
static const double kApplyLinkX1[8] = { 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
static const double kApplyLinkX2[8] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0 };
static const double kApplyLinkY1[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0 };
static const double kApplyLinkY2[8] = { 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 };

#define MIN_MARQUEE_PIXELS 4

static marquee_mode_t	mode_for_modifiers(GUI_KeyFlags flags, bool rotate_ok)
{
	if (rotate_ok) if (flags & gui_OptionAltFlag) return mm_Rotate;
	if (flags & gui_ShiftFlag)
		return (flags & gui_ControlFlag) ? mm_Prop_Center : mm_Center;
	else
		return (flags & gui_ControlFlag) ? mm_Prop : mm_Drag ;
}

WED_MarqueeTool::WED_MarqueeTool(
										const char *			tool_name,
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver) :
				WED_HandleToolBase(tool_name, host, zoomer, resolver),
				mCacheKeyArchive(-1),
				mEditMode(mm_None)
{
	SetControlProvider(this);
}

WED_MarqueeTool::~WED_MarqueeTool()
{
}

void	WED_MarqueeTool::BeginEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->StartOperation("Marquee Drag");
	mEditMode=mm_None;
}

void	WED_MarqueeTool::EndEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->CommitOperation();
	mEditMode=mm_None;
}

int		WED_MarqueeTool::CountEntities(void) const
{
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);

	if (sel->GetSelectionCount() == 0)	return 0;
										return 1;
}

intptr_t		WED_MarqueeTool::GetNthEntityID(int n) const
{
	return 0;
}

int		WED_MarqueeTool::CountControlHandles(intptr_t id						  ) const
{
	if (!GetTotalBounds())			return 0;
//	if (mCacheBounds.is_point())	return 1;
									return 9;
}

void	WED_MarqueeTool::GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const
{
	marquee_mode_t show_mode = (mEditMode == mm_None) ? mode_for_modifiers(GetHost()->GetModifiersNow(),true) : mEditMode;
	
	if(mEditMode == mm_Rotate)
	{
		if(p) *p = (n == 0) ? mRotateCtr : mRotatePt;
		if(active) *active=(n<2);
		if(con_type) *con_type = n==1 ? handle_Rotate : (n==0 ? handle_Square : handle_None);
		if(direction) *direction=Vector2(mRotateCtr,mRotatePt);

	} else {

		double min_size = GetZoomer()->GetClickRadius(MIN_MARQUEE_PIXELS);

		if(mCacheBounds.xspan() < min_size && mCacheBounds.yspan() < min_size)
		{
			if(active) *active = (n == 8);
			if (con_type) *con_type = handle_Icon;
			if (radius) *radius = GetFurnitureIconRadius();
			if (direction) *direction=Vector2(0,1);
			if (p)
			{
				if (!GetTotalBounds())
					*p = Point2(); return;

				p->x_ = mCacheBounds.p1.x() * kControlsX1[8] + mCacheBounds.p2.x() * kControlsX2[8];
				p->y_ = mCacheBounds.p1.y() * kControlsY1[8] + mCacheBounds.p2.y() * kControlsY2[8];
			}
		}
		else if (show_mode == mm_Rotate)
		{
			if(active) *active=1;
			if (con_type) *con_type = (n == 8) ? handle_Square : handle_RotateHead;
			if (p)
			{
				if (!GetTotalBounds())
				{
					*p = Point2(); return;
				}
				p->x_ = mCacheBounds.p1.x() * kControlsX1[n] + mCacheBounds.p2.x() * kControlsX2[n];
				p->y_ = mCacheBounds.p1.y() * kControlsY1[n] + mCacheBounds.p2.y() * kControlsY2[n];
			}
			if (direction) *direction=Vector2(
					Point2(
						mCacheBounds.p1.x() * kControlsX1[8] + mCacheBounds.p2.x() * kControlsX2[8],
						mCacheBounds.p1.y() * kControlsY1[8] + mCacheBounds.p2.y() * kControlsY2[8]),
					Point2(
						mCacheBounds.p1.x() * kControlsX1[n] + mCacheBounds.p2.x() * kControlsX2[n],
						mCacheBounds.p1.y() * kControlsY1[n] + mCacheBounds.p2.y() * kControlsY2[n]));
		}
		else
		{	
			if(active) *active=1;

			if(active)
			if(mCacheBounds.xspan() < min_size)
			if(n != 8 && n != 3 && n != 7)
				*active=0;
		
			if(active)
			if(mCacheBounds.yspan() < min_size)
			if(n != 8 && n != 1 && n != 5)
				*active=0;
		
			if (con_type) *con_type = handle_Square;
			if (direction) *direction=Vector2(0,1);
			if (p)
			{
				if (!GetTotalBounds())
				{
					*p = Point2(); return;
				}
				p->x_ = mCacheBounds.p1.x() * kControlsX1[n] + mCacheBounds.p2.x() * kControlsX2[n];
				p->y_ = mCacheBounds.p1.y() * kControlsY1[n] + mCacheBounds.p2.y() * kControlsY2[n];
			}
		}
	}
}


int		WED_MarqueeTool::GetLinks		    (intptr_t id) const
{
	if (!GetTotalBounds())		return 0;
	if (mCacheBounds.is_point())return 0;
								return 8;
}

void	WED_MarqueeTool::GetNthLinkInfo		(intptr_t id, int n, bool * active, LinkType_t * ltype) const
{
	if (active) *active=1;
	if (ltype) *ltype = link_Marquee;
}

int		WED_MarqueeTool::GetNthLinkSource   (intptr_t id, int n) const
{
	return n;
}

int		WED_MarqueeTool::GetNthLinkSourceCtl(intptr_t id, int n) const
{
	return -1;
}

int		WED_MarqueeTool::GetNthLinkTarget   (intptr_t id, int n) const
{
	return (n+1)%8;
}

int		WED_MarqueeTool::GetNthLinkTargetCtl(intptr_t id, int n) const
{
	return -1;
}

bool	WED_MarqueeTool::PointOnStructure(intptr_t id, const Point2& p) const
{
	if (!GetTotalBounds()) return false;
	return mCacheBounds.contains(p);
}

void	WED_MarqueeTool::ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;
	
	if(mEditMode == mm_None)
	{
		GUI_KeyFlags flags = GetHost()->GetModifiersNow();
		if (flags & gui_OptionAltFlag)
			WED_DoDuplicate(GetResolver(), false);
	}
	
	mEditMode = mm_Drag;

	new_b = mCacheBounds;
	new_b.p1 += delta;
	new_b.p2 += delta;
	ApplyRescale(mCacheBounds,new_b);
}

void	WED_MarqueeTool::ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;

	if (mEditMode == mm_None)
	{
		GUI_KeyFlags flags = GetHost()->GetModifiersNow();
		mEditMode = mode_for_modifiers(flags, c != 8);

		if (mEditMode == mm_Rotate)
		{
			mRotateCtr.x_ = mCacheBounds.p1.x() * kControlsX1[8] + mCacheBounds.p2.x() * kControlsX2[8];
			mRotateCtr.y_ = mCacheBounds.p1.y() * kControlsY1[8] + mCacheBounds.p2.y() * kControlsY2[8];
//			mRotatePt.x = mCacheBounds.p1.x * kControlsX1[c] + mCacheBounds.p2.x * kControlsX2[c];
//			mRotatePt.y = mCacheBounds.p1.y * kControlsY1[c] + mCacheBounds.p2.y * kControlsY2[c];
		}

		if ((flags & gui_OptionAltFlag) && (c == 8))
		{
			WED_DoDuplicate(GetResolver(), false);
		}
	}
	
	Vector2	d(delta);
	
	if(mCacheBounds.xspan() != 0.0 && mCacheBounds.yspan() != 0.0)			// Don't run if bbox is degenerate - we can't preserve its aspect ratio, which is 0 or infinite.
	if(delta.dx != 0.0 || delta.dy != 0.0)									// Don't run if no delta, we get a div-by-zero in vector project.
	if (mEditMode == mm_Prop_Center || mEditMode == mm_Prop)
	{
		if(c == 1 || c == 5)
			d.dy = d.dx * mCacheBounds.yspan() / mCacheBounds.xspan();
		else if (c == 3 || c == 7)
			d.dx = d.dy * mCacheBounds.xspan() / mCacheBounds.yspan();
		else if (c < 8)
		{
			double l = sqrt(delta.squared_length());
			Vector2 n = Vector2(fabs(mCacheBounds.xspan()), fabs(mCacheBounds.yspan()));
			n.normalize();
			n *= l;
			
				 if (c == 0) n    *= -1.0;
			else if (c == 2) n.dx *= -1.0;
			else if (c == 6) n.dy *= -1.0;

			d = n.projection(d);
		}			
	}
	

	switch(mEditMode) {
	case mm_Rotate:
		{
			Point2 new_p;

			new_p = io_pt + d;

			double a1 = VectorDegs2NorthHeading(mRotateCtr, mRotateCtr, Vector2(mRotateCtr, io_pt));
			double b1 = VectorDegs2NorthHeading(mRotateCtr, mRotateCtr, Vector2(mRotateCtr, new_p));
			ApplyRotate(mRotateCtr,WED_CalcDragAngle(mRotateCtr, io_pt, d));

			io_pt = new_p;
			mRotatePt = io_pt;

		}
		break;
	case mm_Center:
		{
			new_b = mCacheBounds;

			if (mCacheBounds.is_point()) c = 8;

			new_b.p1.x_ += (d.dx * kApplyCtrlCtrX1[c]);
			new_b.p2.x_ += (d.dx * kApplyCtrlCtrX2[c]);
			new_b.p1.y_ += (d.dy * kApplyCtrlCtrY1[c]);
			new_b.p2.y_ += (d.dy * kApplyCtrlCtrY2[c]);

			ApplyRescale(mCacheBounds,new_b);
		}
		break;
	case mm_Prop_Center:
		{
			new_b = mCacheBounds;

			if (mCacheBounds.is_point()) c = 8;

			new_b.p1.x_ += (d.dx * kApplyCtrlPropCtrX1[c]);
			new_b.p2.x_ += (d.dx * kApplyCtrlPropCtrX2[c]);
			new_b.p1.y_ += (d.dy * kApplyCtrlPropCtrY1[c]);
			new_b.p2.y_ += (d.dy * kApplyCtrlPropCtrY2[c]);

			ApplyRescale(mCacheBounds,new_b);
		}
		break;
	case mm_Prop:
		{
			new_b = mCacheBounds;

			if (mCacheBounds.is_point()) c = 8;

			new_b.p1.x_ += (d.dx * kApplyCtrlPropX1[c]);
			new_b.p2.x_ += (d.dx * kApplyCtrlPropX2[c]);
			new_b.p1.y_ += (d.dy * kApplyCtrlPropY1[c]);
			new_b.p2.y_ += (d.dy * kApplyCtrlPropY2[c]);

			ApplyRescale(mCacheBounds,new_b);
		}
		break;
	case mm_Drag:
	default:
		{
			new_b = mCacheBounds;

			if (mCacheBounds.is_point()) c = 8;

			new_b.p1.x_ += (d.dx * kApplyCtrlX1[c]);
			new_b.p2.x_ += (d.dx * kApplyCtrlX2[c]);
			new_b.p1.y_ += (d.dy * kApplyCtrlY1[c]);
			new_b.p2.y_ += (d.dy * kApplyCtrlY2[c]);

			ApplyRescale(mCacheBounds,new_b);
		}
		break;
	}
}

void	WED_MarqueeTool::ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;

	mEditMode = mm_Drag;
	
	new_b = mCacheBounds;

	new_b.p1.x_ += (delta.dx * kApplyLinkX1[c]);
	new_b.p2.x_ += (delta.dx * kApplyLinkX2[c]);
	new_b.p1.y_ += (delta.dy * kApplyLinkY1[c]);
	new_b.p2.y_ += (delta.dy * kApplyLinkY2[c]);

	ApplyRescale(mCacheBounds,new_b);
}

/*
void WED_MarqueeTool::GetEntityInternal(vector<IGISEntity *>& e)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);

	vector<IBase *>	iu;
	vector<IGISEntity *> en;

	e.clear();

	sel->GetSelectionVector(iu);
	if (iu.empty()) return;
	en.reserve(iu.size());
	for (vector<IBase *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		if (ent) e.push_back(ent);
	}
}
*/
#if OPTIMIZE
/*	hrm - ths is a case where bulk fetch would be more efficient by a factor of, um, 8??
	but - this is a special case.  in most cases the data model can produce answers quickly,
	and having to COPY the handle set sucks.  So probably its better for the  whole app to
	solve this with caching.  Thought: if the sel had generation change numbers, we could
	inval the cache when the sel changes.   We could also just respod to an "any changed" msg. */
#endif

bool	WED_MarqueeTool::GetTotalBounds(void) const
{
	long long key_a = WED_GetWorld(GetResolver())->GetArchive()->CacheKey();

	if (key_a == mCacheKeyArchive) return !mCacheBounds.is_null();
	mCacheKeyArchive = key_a;

	mCacheBounds = Bbox2();
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);
	mCacheIconic = false;

	vector<ISelectable *>	iu;

	sel->GetSelectionVector(iu);
	if (iu.empty()) return false;
	bool iconic = iu.size() == 1;
	for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		WED_Entity * went = SAFE_CAST(WED_Entity,*i);
		if (went)
		{
			if(IsLockedNow(went))		continue;
			if(!IsVisibleNow(went))	continue;
		}

		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		if (ent)
		{
			if (iconic && !WED_IsIconic(ent)) iconic = false;
			Bbox2 local;
			ent->GetBounds(gis_Geo,local);
			mCacheBounds += local;
		}
	}
	mCacheIconic = iconic;
	return !mCacheBounds.is_null();
}

void	WED_MarqueeTool::ApplyRescale(const Bbox2& old_bounds, const Bbox2& new_bounds)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);

	vector<ISelectable *>	iu;
	set<IGISEntity *>		ent_set;

	sel->GetSelectionVector(iu);
	for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		WED_Entity * went = SAFE_CAST(WED_Entity,*i);
		if (went)
		{
			if(IsLockedNow(went))		continue;
			if(!IsVisibleNow(went))	continue;
		}

		// This is a mess.  GIS edge's violate the tree structure of the GIS class by 
		// referring to nodes outside of themselves.  The bug is that when you select 
		// two edges, if you tell them both to move, they will EACH tell their attached
		// common vertex to move - it will move by 2x the distance.
		//
		// So the marquee tool hacks hte hell around this by breaking down all edges into
		// their sources in a set, which de-dupes the vertices.
		if (ent)
		{
			if(ent->GetGISClass() == gis_Edge)
			{
				IGISPointSequence * ps = dynamic_cast<IGISPointSequence *>(ent);
				if(ps)
				{
					int np = ps->GetNumPoints();
					for(int n = 0; n < np; ++n)
						ent_set.insert(ps->GetNthPoint(n));
				}
			}
			else
				ent_set.insert(ent);
		}
		
		if (ent->GetGISClass() == gis_Point_Bezier)        // one or more nodes were selected
			went = dynamic_cast <WED_Entity *> (went->GetParent());
		if (went->GetClass() == WED_Ring::sClass)          // a hole was selected
			went = dynamic_cast <WED_Entity *>  (went->GetParent());
		if (went->GetClass() == WED_DrapedOrthophoto::sClass)
			dynamic_cast <WED_DrapedOrthophoto *> (went)->Redrape();
	}
	
	for(set<IGISEntity *>::iterator e = ent_set.begin(); e != ent_set.end(); ++e)
		(*e)->Rescale(gis_Geo,old_bounds,new_bounds);

}

void	WED_MarqueeTool::ApplyRotate(const Point2& ctr, double angle)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);

	vector<ISelectable *>	iu;
	set<IGISEntity *>		ent_set;

	sel->GetSelectionVector(iu);
	for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		WED_Entity * went = SAFE_CAST(WED_Entity,*i);

		if (went)
		{
			if(IsLockedNow(went))		continue;
			if(!IsVisibleNow(went))	continue;
		}

		if (ent)
		{
			if(ent->GetGISClass() == gis_Edge)
			{
				IGISPointSequence * ps = dynamic_cast<IGISPointSequence *>(ent);
				if(ps)
				{
					int np = ps->GetNumPoints();
					for(int n = 0; n < np; ++n)
						ent_set.insert(ps->GetNthPoint(n));
				}
			}
			else
				ent_set.insert(ent);
		}
#if 1                                    // orthophotos can have their texture heading rotated along with the polygon
                                         // its just my user interface choice, not a necessity in any way:
		if (went->GetClass() == WED_DrapedOrthophoto::sClass) 
		{ 
			WED_DrapedOrthophoto * ortho = dynamic_cast <WED_DrapedOrthophoto *> (went);
			ortho->SetHeading(ortho->GetHeading() + angle);
		}
		else
#endif
		{
			if (ent->GetGISClass() == gis_Point_Bezier)       // one or more nodes were selected
				went = dynamic_cast <WED_Entity *> (went->GetParent());
			if (went->GetClass() == WED_Ring::sClass)         // a hole was selected
				went = dynamic_cast <WED_Entity *>  (went->GetParent());
			if (went->GetClass() == WED_DrapedOrthophoto::sClass)
				dynamic_cast <WED_DrapedOrthophoto *> (went)->Redrape();
		}
	}
	
	for(set<IGISEntity *>::iterator e = ent_set.begin(); e != ent_set.end(); ++e)
		(*e)->Rotate(gis_Geo,ctr, angle);
}
