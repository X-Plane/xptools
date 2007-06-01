#include "WED_CreateToolBase.h"
#include "WED_MapZoomerNew.h"
#include "GUI_GraphState.h"
#include "WED_ToolUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
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
		glVertex2f(z->LonToXPixel(b.p1.x),z->LatToYPixel(b.p1.y));
		glVertex2f(z->LonToXPixel(b.p2.x),z->LatToYPixel(b.p2.y));
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
		for (int t = 0; t <= BEZ_STEPS; ++t)
		{
			float r = (float) t / (float) BEZ_STEPS;
			Point2 m = b.midpoint(r);
			glVertex2f(z->LonToXPixel(m.x),z->LatToYPixel(m.y));
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

int		WED_CreateToolBase::GetNthEntityID(int n) const
{
	return 0;
}

int		WED_CreateToolBase::CountControlHandles(int id						  ) const
{
	return mPts.size() * 3;
}

void	WED_CreateToolBase::GetNthControlHandle(int id, int n, int * active, HandleType_t * con_type, Point2 * p, Vector2 * direction) const
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

	// If we can, make the first point have the hollow circle to show it.
	if (con_type && *con_type == handle_None && idx == 0 && mCanClose  && kind == 0)					*con_type = handle_ClosePt;
	// First point for runways or points: cross shows placement
	if (con_type && *con_type == handle_None && idx == 0 && mMaxPts<=2 && kind == 0)					*con_type = handle_Cross;
	// For a directional point, draw arrow head to hi control point.
	if (con_type && *con_type == handle_None && idx == 0 && mMaxPts==1 && kind == 2 && mMustCurve)
	if (mPts[idx] != mControlHi[idx])
	{
		*con_type = handle_Arrow;
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

		
int		WED_CreateToolBase::GetLinks		    (int id) const
{
	return mPts.size() * 3;
}

void	WED_CreateToolBase::GetNthLinkInfo(int id, int n, int * active, LinkType_t * ltype) const
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


int		WED_CreateToolBase::GetNthLinkSource   (int id, int n) const
{
	int idx = n / 3;
	int kind = n % 3;
	return idx*3;
}


int		WED_CreateToolBase::GetNthLinkSourceCtl(int id, int n) const
{
	int idx = n / 3;
	int kind = n % 3;
	if (kind != 0) return -1;
	return idx*3+2;
}

int		WED_CreateToolBase::GetNthLinkTarget   (int id, int n) const
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

int		WED_CreateToolBase::GetNthLinkTargetCtl(int id, int n) const
{
	int idx = n / 3;
	int kind = n % 3;
	if (kind != 0) return -1;
	return ((idx+1)%mPts.size())*3+1;
}

bool	WED_CreateToolBase::PointOnStructure(int id, const Point2& p) const
{
	return 0;
}

void	WED_CreateToolBase::ControlsHandlesBy(int id, int c, const Vector2& delta)
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
}

void	WED_CreateToolBase::ControlsLinksBy	 (int id, int c, const Vector2& delta)
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

void	WED_CreateToolBase::ControlsMoveBy	 (int id,        const Vector2& delta)
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

}

void			WED_CreateToolBase::KillOperation(void)
{
	WED_HandleToolBase::KillOperation();
	mPts.clear();
	mHasDirs.clear();
	mIsSplit.clear();
	mControlLo.clear();
	mControlHi.clear();
	mCreating = 0;
}

int			WED_CreateToolBase::HandleKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  )
{
	if (!mCreating && (inFlags & gui_DownFlag))
	switch (inKey) {
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
			return 1;
		}
		break;
	}
	return WED_HandleToolBase::HandleKeyPress(inKey, inVK, inFlags);
}
