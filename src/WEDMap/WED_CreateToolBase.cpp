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

#include "WED_CreateToolBase.h"
#include "WED_MapZoomerNew.h"
#include "GUI_GraphState.h"
#include "WED_ToolUtils.h"
#include "GISUtils.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

const float kDoubleClickTime = 0.2;
const int kDoubleClickDist = 3;
const int kCloseLoopDist = 5;
const int BEZ_STEPS = 50;

inline bool	within_dist(const Point2& p1, const Point2& p2, WED_MapZoomerNew * z, float dist)
{
	return Segment2(z->LLToPixel(p1),z->LLToPixel(p2)).squared_length() < (dist * dist);
}

static void ogl_bezier(const Bezier2& b, WED_MapZoomerNew * z)
{
	if (b.p1 == b.c1 && b.p2 == b.c2)
	{
		glBegin(GL_LINE_STRIP);
		glVertex2f(z->LonToXPixel(b.p1.x()),z->LatToYPixel(b.p1.y()));
		glVertex2f(z->LonToXPixel(b.p2.x()),z->LatToYPixel(b.p2.y()));
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
		for (int t = 0; t <= BEZ_STEPS; ++t)
		{
			float r = (float) t / (float) BEZ_STEPS;
			Point2 m = b.midpoint(r);
			glVertex2f(z->LonToXPixel(m.x()),z->LatToYPixel(m.y()));
		}
		glEnd();
	}
}

WED_CreateToolBase::WED_CreateToolBase(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									int					min_num_pts,
									int					max_num_pts,
									int					can_curve,
									int					must_curve,
									int					can_close,
									int					must_close) :
	WED_HandleToolBase(tool_name, host,zoomer,resolver),
	mArchive(archive),
	mLastTime(-9.9e9),
	mCreating(0),
	mMinPts(min_num_pts),
	mMaxPts(max_num_pts),
	mCanClose(can_close),
	mMustClose(must_close),
	mCanCurve(can_curve),
	mMustCurve(must_curve)
{
	SetControlProvider(this);
	SetCanSelect(0);
}

WED_CreateToolBase::~WED_CreateToolBase()
{
}

void	WED_CreateToolBase::BeginEdit(void)
{
	mEditStarted = 0;
}

void	WED_CreateToolBase::EndEdit(void)
{
}

int		WED_CreateToolBase::CountEntities(void) const
{
	return mPts.empty() ? 0 : 1;
}

intptr_t	WED_CreateToolBase::GetNthEntityID(int n) const
{
	return 0;
}

int		WED_CreateToolBase::CountControlHandles(intptr_t id						  ) const
{
	return mPts.size() * 3;
}

void	WED_CreateToolBase::GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const
{
		int idx = n / 3;
		int kind = n % 3;

	if (active) *active = 0;
	GUI_KeyFlags mods = GetHost()->GetModifiersNow();
	if (mods & (gui_ControlFlag+gui_OptionAltFlag+gui_ShiftFlag) && active)
	{
		if (mHasDirs[idx]) *active = 1;
		else { *active = (kind != 0) == ((mods & gui_OptionAltFlag) != 0);
			if (kind == 1) *active = 0; }
	}

	if (con_type) *con_type = (mods & (gui_ControlFlag+gui_OptionAltFlag+gui_ShiftFlag)) ? (kind == 0 ? (mHasDirs[idx] ? handle_Vertex : handle_VertexSharp) : handle_Bezier) : handle_None;
	if (direction) *direction = Vector2();
	if (direction && con_type && *con_type == handle_Bezier)
		*direction = Vector2((kind == 1) ? mControlLo[idx] : mControlHi[idx], mPts[idx]);

	// If we can, make the first point have the hollow circle to show it.
	if (con_type && *con_type == handle_None && idx == 0 && mCanClose  && kind == 0)					*con_type = handle_ClosePt;
	// First point for runways or points: cross shows placement
	if (con_type && *con_type == handle_None && idx == 0 && mMaxPts<=2 && kind == 0)					*con_type = handle_Cross;
	// For a directional point, draw arrow head to hi control point.
	if (con_type && *con_type == handle_None && idx == 0 && mMaxPts==1 && kind == 2 && mMustCurve)
	if (mPts[idx] != mControlHi[idx])
	{
		*con_type = handle_Rotate;
		if (direction) *direction = Vector2(mPts[idx],mControlHi[idx]);
	}

	if (p)
	{
		switch(kind){
		case 0: *p = mPts[idx];			break;
		case 1: *p = mControlLo[idx];	break;
		case 2: *p = mControlHi[idx];	break;
		}
	}
}


int		WED_CreateToolBase::GetLinks		    (intptr_t id) const
{
	return mPts.size() * 3;
}

void	WED_CreateToolBase::GetNthLinkInfo(intptr_t id, int n, bool * active, LinkType_t * ltype) const
{
	if (active) *active = 0;
	if (!ltype) return;

	int idx = n / 3;
	int kind = n % 3;

	int m = (idx+1)%mPts.size();
	switch(kind) {
	case 0:
		if (m <= idx) *ltype = mMustClose ? link_Ghost : link_None;
		else		 *ltype = link_Solid;
		break;
	case 1:
		*ltype = mHasDirs[idx] ? link_BezierCtrl : link_None;
		if (mMaxPts == 1 && mMustCurve) *ltype = link_None;	// for directional pts.
		break;
	case 2:
		*ltype = mHasDirs[idx] ? link_BezierCtrl : link_None;
		break;
	}
}


int		WED_CreateToolBase::GetNthLinkSource   (intptr_t id, int n) const
{
	int idx = n / 3;
	int kind = n % 3;
	return idx*3;
}


int		WED_CreateToolBase::GetNthLinkSourceCtl(intptr_t id, int n) const
{
	int idx = n / 3;
	int kind = n % 3;
	if (kind != 0) return -1;
	return idx*3+2;
}

int		WED_CreateToolBase::GetNthLinkTarget   (intptr_t id, int n) const
{
	int idx = n / 3;
	int kind = n % 3;
	switch(kind) {
	case 0:	return ((idx+1)%mPts.size()) * 3;
	case 1: return idx*3+1;
	case 2: return idx*3+2;
	}
	return 0;
}

int		WED_CreateToolBase::GetNthLinkTargetCtl(intptr_t id, int n) const
{
	int idx = n / 3;
	int kind = n % 3;
	if (kind != 0) return -1;
	return ((idx+1)%mPts.size())*3+1;
}

bool	WED_CreateToolBase::PointOnStructure(intptr_t id, const Point2& p) const
{
	return false;
}

void	WED_CreateToolBase::ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	int idx = c / 3;
	int kind = c % 3;
	if (!mEditStarted)
	{
		mEditStarted = 1;
		GUI_KeyFlags mods = GetHost()->GetModifiersNow();
		if ((kind) != 0 && (mods & gui_OptionAltFlag))
		{
			if (mHasDirs[idx])
				mIsSplit[idx] = 1;
			else
				mHasDirs[idx] = 1;
		}

		if ((kind) != 0 && (mods & gui_ShiftFlag))
		{
			if (mIsSplit[idx]) {
				mIsSplit[idx] = 0;
				if (kind == 1) mControlHi[idx] = mPts[idx] + Vector2(mControlLo[idx],mPts[idx]);
				if (kind == 2) mControlLo[idx] = mPts[idx] + Vector2(mControlHi[idx],mPts[idx]);
			} else
				mHasDirs[idx] = 0;
		}
	}

	switch(kind) {
	case 0:		mPts[idx] += delta;	mControlLo[idx] += delta; mControlHi[idx] += delta;		break;
	case 1:		mControlLo[idx] += delta;	if (!mIsSplit[idx]) mControlHi[idx] -= delta;	if (!mHasDirs[idx]) mControlHi[idx] = mControlLo[idx] = mPts[idx]; break;
	case 2:		mControlHi[idx] += delta;	if (!mIsSplit[idx]) mControlLo[idx] -= delta;	if (!mHasDirs[idx]) mControlHi[idx] = mControlLo[idx] = mPts[idx]; break;
	}

	io_pt += delta;
}

void	WED_CreateToolBase::ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	if (c != mPts.size()-1)
	{
		mPts[c+1] += delta;
		mControlHi[c+1] += delta;
		mControlLo[c+1] += delta;
	}

	mPts[c] += delta;
	mControlHi[c] += delta;
	mControlLo[c] += delta;
}

void	WED_CreateToolBase::ControlsMoveBy	 (intptr_t id,        const Vector2& delta, Point2& io_track)
{
}

int			WED_CreateToolBase::CreationDown(const Point2& start_pt)
{
	if (!CanCreateNow()) return 0;

	float now = GetHost()->GetTimeNow();
	if (now-mLastTime < kDoubleClickTime &&
		within_dist(start_pt, mLastPt, GetZoomer(), kDoubleClickDist))
	{
		if (mPts.size() >= mMinPts)
			DoEmit(0);
		mCreating = 0;
	}
	else
	{
		mPts.push_back(start_pt);
		mControlLo.push_back(start_pt);
		mControlHi.push_back(start_pt);
		mHasDirs.push_back(mMustCurve);
		mIsSplit.push_back(0);
		mCreating = 1;
	}

	RecalcHeadings();

	mLastTime = now;
	mLastPt = start_pt;
	return 1;
}


void		WED_CreateToolBase::CreationDrag(const Point2& start_pt, const Point2& now_pt)
{
	if (!mCreating) return;

	if (!mHasDirs.back() && mCanCurve &&
		!within_dist(start_pt,now_pt,GetZoomer(), kDoubleClickDist))
	{
		mHasDirs.back() = 1;
	}

	if (mHasDirs.back())
		mControlHi.back() = now_pt;
	else
		mControlHi.back() = mPts.back() = now_pt;

	RecalcHeadings();

	mControlLo.back() = mPts.back() + (Vector2(mControlHi.back(), mPts.back()));

}

void		WED_CreateToolBase::CreationUp(const Point2& start_pt, const Point2& now_pt)
{
	if (!mCreating) return;

	CreationDrag(start_pt, now_pt);

	if(mPts.size() >= mMaxPts)
		DoEmit(0);
	// We can only check for a closed loop on:
	// - closed-loop-possible chains with
	// - at least 3 points (so one can be redundent) and
	// - no curve at the end
	// - we have enough pts that throwing one out is okay
	else if (mCanClose && mPts.size() > 2 && !mHasDirs.back() && mPts.size() > mMinPts)
	{
		if (within_dist(mPts.front(), mPts.back(), GetZoomer(), kCloseLoopDist))
			DoEmit(1);
	}

	RecalcHeadings();

	mCreating = 0;
}

void		WED_CreateToolBase::DoEmit(int do_close)
{
	int closed = mMustClose || do_close;

	if (do_close)
	{
		do_close = 1;
		mPts.pop_back();
		mIsSplit.pop_back();
		mHasDirs.pop_back();
		mControlLo.pop_back();
		mControlHi.pop_back();
	}

	this->AcceptPath(mPts,mControlLo, mControlHi, mHasDirs, mIsSplit, closed);
	mPts.clear();
	mHasDirs.clear();
	mIsSplit.clear();
	mControlLo.clear();
	mControlHi.clear();

	ClearAnchor1();
	ClearAnchor2();
	ClearDistance();
	ClearHeading();


}

void			WED_CreateToolBase::KillOperation(bool mouse_is_down)
{
	WED_HandleToolBase::KillOperation(mouse_is_down);
		if (mPts.size() >= mMinPts)
		{
			DoEmit(0);
			RecalcHeadings();
		}

	mPts.clear();
	mHasDirs.clear();
	mIsSplit.clear();
	mControlLo.clear();
	mControlHi.clear();
	mCreating = 0;
}

int			WED_CreateToolBase::HandleToolKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  )
{
	if (!mCreating && (inFlags & gui_DownFlag))
	switch (inKey) {
	case GUI_KEY_BACK:
	case GUI_KEY_DELETE:
		if (!mPts.empty()) {
			mPts.pop_back();
			mHasDirs.pop_back();
			mIsSplit.pop_back();
			mControlLo.pop_back();
			mControlHi.pop_back();
			GetHost()->Refresh();
			return 1;
		}
		break;
	case GUI_KEY_ESCAPE:
		if (!mPts.empty()) {
			mPts.clear();
			mHasDirs.clear();
			mIsSplit.clear();
			mControlLo.clear();
			mControlHi.clear();
			GetHost()->Refresh();
			return 1;
		}
		break;
	case GUI_KEY_RETURN:
		if (mPts.size() >= mMinPts)
		{
			DoEmit(0);
			RecalcHeadings();
			return 1;
		}
		break;
	}
	return WED_HandleToolBase::HandleToolKeyPress(inKey, inVK, inFlags);
}

/*
bool		WED_CreateToolBase::HasDragNow(
				Point2&	p,
				Point2& c)
{
	if (!mCreating || mPts.empty()) return false;
	p = mPts.back();
	c = mControlHi.back();
	return true;
}

bool		WED_CreateToolBase::HasPrevNow(
							Point2& p,
							Point2& c)
{
	int idx_want = mPts.size();
	--idx_want;
	if (mCreating) --idx_want;
	if (idx_want < 0) return false;
	p = mPts[idx_want];
	c = mControlHi[idx_want];
	return true;
}
*/

void		WED_CreateToolBase::RecalcHeadings(void)
{
	ClearHeading();
	ClearDistance();
	ClearAnchor1();
	ClearAnchor2();

	if (mPts.size() > 1)
	{
		SetAnchor1(mPts[mPts.size()-2]);
		SetAnchor2(mPts[mPts.size()-1]);
		SetDistance (LonLatDistMeters(mPts[mPts.size()-2].x(),mPts[mPts.size()-2].y(),mPts[mPts.size()-1].x(),mPts[mPts.size()-1].y()));
		SetHeading(VectorMeters2NorthHeading(mPts[mPts.size()-2],mPts[mPts.size()-2],Vector2(mPts[mPts.size()-2],mPts[mPts.size()-1])));
	}
	else if (mPts.size() > 0)
	{
		SetAnchor1(mPts[mPts.size()-1]);
		if (mHasDirs[mPts.size()-1])
		{
			SetHeading(VectorMeters2NorthHeading(mPts[mPts.size()-1],mPts[mPts.size()-1],Vector2(mPts[mPts.size()-1],mControlHi[mPts.size()-1])));
		}
	}
}
