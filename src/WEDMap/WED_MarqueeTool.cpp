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
#include "WED_Entity.h"
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

// How much to transform each point when we drag a LINK!
static const double kApplyLinkX1[8] = { 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
static const double kApplyLinkX2[8] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0 };
static const double kApplyLinkY1[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0 };
static const double kApplyLinkY2[8] = { 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 };

WED_MarqueeTool::WED_MarqueeTool(
										const char *			tool_name,
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver) :
				WED_HandleToolBase(tool_name, host, zoomer, resolver),
				mCacheKeyArchive(-1),
				mInEdit(0),
				mIsRotate(0)
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
	mInEdit=0;
	mIsRotate=0;
}

void	WED_MarqueeTool::EndEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->CommitOperation();
	mIsRotate=0;
	mInEdit=0;
}

intptr_t		WED_MarqueeTool::CountEntities(void) const
{
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);

	if (sel->GetSelectionCount() == 0)	return 0;
										return 1;
}

intptr_t		WED_MarqueeTool::GetNthEntityID(intptr_t n) const
{
	return 0;
}

intptr_t		WED_MarqueeTool::CountControlHandles(intptr_t id						  ) const
{
	if (!GetTotalBounds())			return 0;
///	if (mCacheBounds.is_point())	return 1;
									return 9;
}

void	WED_MarqueeTool::GetNthControlHandle(intptr_t id, intptr_t n, intptr_t * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const
{
	if (mIsRotate)
	{
		if(p) *p = (n == 0) ? mRotateCtr : mRotatePt;
		if(active) *active=(n<2);
		if(con_type) *con_type = n==1 ? handle_Rotate : (n==0 ? handle_Square : handle_None);
		if(direction) *direction=Vector2(mRotateCtr,mRotatePt);
	}
	else
	{
		int could_rotate = !mInEdit && (GetHost()->GetModifiersNow() & gui_OptionAltFlag) && !mCacheBounds.is_point();
		if(active) *active=(mCacheBounds.is_point() ? (n == 8) : 1);
		if (con_type) *con_type = mCacheIconic ? handle_Icon : ((could_rotate && n < 8) ? handle_RotateHead : handle_Square);
		if (radius && mCacheIconic) *radius = GetFurnitureIconRadius();
		if (direction) *direction=Vector2(0,1);
		if (p)
		{
			if (!GetTotalBounds())
			{
				*p = Point2(); return;
			}

			if (mCacheBounds.is_point()) n = 8;

			p->x = mCacheBounds.p1.x * kControlsX1[n] + mCacheBounds.p2.x * kControlsX2[n];
			p->y = mCacheBounds.p1.y * kControlsY1[n] + mCacheBounds.p2.y * kControlsY2[n];
		}
		if (p && direction && could_rotate)
		{
			Point2 ctr;
			ctr.x = mCacheBounds.p1.x * kControlsX1[8] + mCacheBounds.p2.x * kControlsX2[8];
			ctr.y = mCacheBounds.p1.y * kControlsY1[8] + mCacheBounds.p2.y * kControlsY2[8];
			*direction = Vector2(ctr,*p);
		}
	}
}


intptr_t		WED_MarqueeTool::GetLinks		    (intptr_t id) const
{
	if (!GetTotalBounds())		return 0;
	if (mCacheBounds.is_point())return 0;
								return 8;
}

void	WED_MarqueeTool::GetNthLinkInfo		(intptr_t id, intptr_t n, intptr_t * active, LinkType_t * ltype) const
{
	int could_rotate = !mInEdit && (GetHost()->GetModifiersNow() & gui_OptionAltFlag);
	if (could_rotate)
	{
		if (active) *active=1;
		if (ltype) *ltype = link_Marquee;
	}
	else
	{
		if (active) *active=1;
		if (ltype) *ltype = link_Marquee;
	}
}

intptr_t		WED_MarqueeTool::GetNthLinkSource   (intptr_t id, intptr_t n) const
{
	return n;
}

intptr_t		WED_MarqueeTool::GetNthLinkSourceCtl(intptr_t id, intptr_t n) const
{
	return -1;
}

intptr_t		WED_MarqueeTool::GetNthLinkTarget   (intptr_t id, intptr_t n) const
{
	return (n+1)%8;
}

intptr_t		WED_MarqueeTool::GetNthLinkTargetCtl(intptr_t id, intptr_t n) const
{
	return -1;
}

bool	WED_MarqueeTool::PointOnStructure(intptr_t id, const Point2& p) const
{
	return false;
}

void	WED_MarqueeTool::ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;
	new_b = mCacheBounds;
	new_b.p1 += delta;
	new_b.p2 += delta;
	ApplyRescale(mCacheBounds,new_b);
}

void	WED_MarqueeTool::ControlsHandlesBy(intptr_t id, intptr_t c, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;

	if (mInEdit==0)
	{
		mInEdit=1;
		GUI_KeyFlags flags = GetHost()->GetModifiersNow();
		mIsRotate = (flags & gui_OptionAltFlag) && (c != 8);

		if (mIsRotate)
		{
			mRotateCtr.x = mCacheBounds.p1.x * kControlsX1[8] + mCacheBounds.p2.x * kControlsX2[8];
			mRotateCtr.y = mCacheBounds.p1.y * kControlsY1[8] + mCacheBounds.p2.y * kControlsY2[8];
//			mRotatePt.x = mCacheBounds.p1.x * kControlsX1[c] + mCacheBounds.p2.x * kControlsX2[c];
//			mRotatePt.y = mCacheBounds.p1.y * kControlsY1[c] + mCacheBounds.p2.y * kControlsY2[c];
		}

		if ((flags & gui_OptionAltFlag) && (c == 8))
		{
			WED_DoDuplicate(GetResolver(), false);
		}
	}

	if (mIsRotate)
	{
		Point2 new_p;

		new_p = io_pt + delta;

		double a1 = VectorDegs2NorthHeading(mRotateCtr, mRotateCtr, Vector2(mRotateCtr, io_pt));
		double b1 = VectorDegs2NorthHeading(mRotateCtr, mRotateCtr, Vector2(mRotateCtr, new_p));
		ApplyRotate(mRotateCtr,WED_CalcDragAngle(mRotateCtr, io_pt, delta));

		io_pt = new_p;
		mRotatePt = io_pt;

	}
	else
	{
		new_b = mCacheBounds;

		if (mCacheBounds.is_point()) c = 8;

		new_b.p1.x += (delta.dx * kApplyCtrlX1[c]);
		new_b.p2.x += (delta.dx * kApplyCtrlX2[c]);
		new_b.p1.y += (delta.dy * kApplyCtrlY1[c]);
		new_b.p2.y += (delta.dy * kApplyCtrlY2[c]);

		ApplyRescale(mCacheBounds,new_b);
	}
}

void	WED_MarqueeTool::ControlsLinksBy	 (intptr_t id, intptr_t c, const Vector2& delta)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;
	new_b = mCacheBounds;

	new_b.p1.x += (delta.dx * kApplyLinkX1[c]);
	new_b.p2.x += (delta.dx * kApplyLinkX2[c]);
	new_b.p1.y += (delta.dy * kApplyLinkY1[c]);
	new_b.p2.y += (delta.dy * kApplyLinkY2[c]);

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
	hrm - ths is a case where bulk fetch would be more efficient by a factor of, um, 8??
	but - this is a special case.  in most cases the data model can produce answers quickly,
	and having to COPY the handle set sucks.  So probably its better for the  whole app to
	solve this with caching.  Thought: if the sel had generation change numbers, we could
	inval the cache when the sel changes.   We could also just respod to an "any changed" msg.
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
	int ret = false;

	sel->GetSelectionVector(iu);
	if (iu.empty()) return false;
	bool iconic = iu.size() == 1;
	for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		WED_Entity * went = SAFE_CAST(WED_Entity,*i);
		if (went)
		{
			if (went->GetLocked()) continue;
			if (went->GetHidden()) continue;
		}

		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		if (ent)
		{
			if (iconic && !WED_IsIconic(ent)) iconic = false;
			Bbox2 local;
			ent->GetBounds(local);
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

	sel->GetSelectionVector(iu);
	for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		WED_Entity * went = SAFE_CAST(WED_Entity,*i);
		if (went)
		{
			if (went->GetLocked()) continue;
			if (went->GetHidden()) continue;
		}

		if (ent)
		{
			ent->Rescale(old_bounds,new_bounds);
		}
	}

}

void	WED_MarqueeTool::ApplyRotate(const Point2& ctr, double angle)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);

	vector<ISelectable *>	iu;

	sel->GetSelectionVector(iu);
	for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		WED_Entity * went = SAFE_CAST(WED_Entity,*i);
		if (went)
		{
			if (went->GetLocked()) continue;
			if (went->GetHidden()) continue;
		}

		if (ent)
		{
			ent->Rotate(ctr, angle);
		}
	}

}
