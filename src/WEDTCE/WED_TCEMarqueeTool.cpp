/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_TCEMarqueeTool.h"
#include "ISelection.h"
#include "IOperation.h"
#include "WED_ToolUtils.h"
#include "AssertUtils.h"
#include "GUI_Pane.h"
#include "WED_GroupCommands.h"
#include "GISUtils.h"
#include "WED_Thing.h"
#include "WED_Entity.h"
#include "IGIS.h"
#include "WED_Ring.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_ResourceMgr.h"


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

// How much to transform each point when we drag a LINK!
static const double kApplyLinkX1[8] = { 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
static const double kApplyLinkX2[8] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0 };
static const double kApplyLinkY1[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0 };
static const double kApplyLinkY2[8] = { 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 };

static tce_marquee_mode_t	mode_for_modifiers(GUI_KeyFlags flags, bool rotate_ok)
{
	if (rotate_ok) if (flags & gui_OptionAltFlag) return tmm_Rotate;
	if (flags & gui_ShiftFlag)
		return (flags & gui_ControlFlag) ? tmm_Prop_Center : tmm_Center;
	else
		return (flags & gui_ControlFlag) ? tmm_Prop : tmm_Drag ;
}


WED_TCEMarqueeTool::WED_TCEMarqueeTool(
										const char *			tool_name,
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver) :
				WED_HandleToolBase(tool_name, host, zoomer, resolver),
				mSnap(this,"Click selects Subtexture", XML_Name("",""), true),
				mCacheKeyArchive(-1),
				mEditMode(tmm_None)
{
	SetCanSelect(0);
	SetControlProvider(this);
}

WED_TCEMarqueeTool::~WED_TCEMarqueeTool()
{
}

void	WED_TCEMarqueeTool::BeginEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->StartOperation("UVbounds Modification");
	mEditMode=tmm_None;
}

void	WED_TCEMarqueeTool::EndEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->CommitOperation();
	mEditMode=tmm_None;
}

int		WED_TCEMarqueeTool::CountEntities(void) const
{
	ISelection * sel = WED_GetSelect(GetResolver());
	DebugAssert(sel != NULL);

	if (sel->GetSelectionCount() == 0)	return 0;
										return 1;
}

intptr_t		WED_TCEMarqueeTool::GetNthEntityID(int n) const
{
	return 0;
}

int		WED_TCEMarqueeTool::CountControlHandles(intptr_t id						  ) const
{
	if (!GetTotalBounds())			return 0;
///	if (mCacheBounds.is_point())	return 1;
									return 9;
}

void	WED_TCEMarqueeTool::GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const
{
	tce_marquee_mode_t show_mode = (mEditMode == tmm_None) ? mode_for_modifiers(GetHost()->GetModifiersNow(),true) : mEditMode;

	if(mEditMode == tmm_Rotate)
	{
		if(p) *p = (n == 0) ? mRotateCtr : mRotatePt;
		if(active) *active=(n<2);
		if(con_type) *con_type = n==1 ? handle_Rotate : (n==0 ? handle_Square : handle_None);
		if(direction) *direction=Vector2(mRotateCtr,mRotatePt);

	} else {

		if(mCacheBounds.is_point())
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
		else if (show_mode == tmm_Rotate)
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


int		WED_TCEMarqueeTool::GetLinks		    (intptr_t id) const
{
	if (!GetTotalBounds())		return 0;
	if (mCacheBounds.is_point())return 0;
								return 8;
}

void	WED_TCEMarqueeTool::GetNthLinkInfo		(intptr_t id, int n, bool * active, LinkType_t * ltype) const
{
	if (active) *active=1;
	if (ltype) *ltype = link_Marquee;
}

int		WED_TCEMarqueeTool::GetNthLinkSource   (intptr_t id, int n) const
{
	return n;
}

int		WED_TCEMarqueeTool::GetNthLinkSourceCtl(intptr_t id, int n) const
{
	return -1;
}

int		WED_TCEMarqueeTool::GetNthLinkTarget   (intptr_t id, int n) const
{
	return (n+1)%8;
}

int		WED_TCEMarqueeTool::GetNthLinkTargetCtl(intptr_t id, int n) const
{
	return -1;
}

bool	WED_TCEMarqueeTool::PointOnStructure(intptr_t id, const Point2& p) const
{
	if (!GetTotalBounds()) 
		return false;
	if (mSnap)
		return true;  // we care about clicking anywhere in the TCE windows
	else
		return mCacheBounds.contains(p);
}

void	WED_TCEMarqueeTool::ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;
	
	mEditMode = tmm_Drag;
	
	if (mSnap)
	{
		// find texture who's UVmap we modify here
		ISelection * sel = WED_GetSelect(GetResolver());
		DebugAssert(sel != NULL);
		WED_DrapedOrthophoto * ortho = SAFE_CAST(WED_DrapedOrthophoto,sel->GetNthSelection(0));
		DebugAssert(ortho != NULL);
		
		WED_ResourceMgr * mResMgr = WED_GetResourceMgr(GetResolver());
		string mRes; ortho->GetResource(mRes);

		pol_info_t pol;
		mResMgr->GetPol(mRes,pol);
		
		io_pt +=delta;
		
		// find where we clicked
		if (pol.mSubBoxes.size())
		{
			// go through list of subtexture boxes and find if we clicked inside one
			static int lastBox = -1;                // the box we clicked on the last time. Helps to cycle trough overlapping boxes
			int        firstBox = 999;              // the first box that fits this click location
			int n;
			for (n=0; n < pol.mSubBoxes.size(); ++n)
			{
				if (pol.mSubBoxes[n].contains(io_pt))
				{
					if (n < firstBox) firstBox = n; // memorize the first of all boxes that fits the click
					if (n > lastBox)                // is it a new-to-us box ?
					{
						new_b = pol.mSubBoxes[n];
						lastBox = n;
						break;
					}
				}
			}

			if (n >= pol.mSubBoxes.size())         // apparently there is no new-to-us box here
			{
				if (firstBox < 999)
				{
					new_b = pol.mSubBoxes[firstBox];    // so we go with the first best box we found
					lastBox = firstBox;
				}
				else
				{
					new_b = Bbox2(0,0,1,1);   // there is no box where we clicked -> select whole texture
					lastBox = -1;
				}
			}
		}
		else
			new_b = Bbox2();                 // there are no subboxes defined at all
	}
	else
	{
		new_b = mCacheBounds;
		new_b.p1 += delta;
		new_b.p2 += delta;
	}
	ApplyRescale(mCacheBounds,new_b);
}

void	WED_TCEMarqueeTool::ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;

	if (mEditMode == tmm_None)
	{
		GUI_KeyFlags flags = GetHost()->GetModifiersNow();
		mEditMode = mode_for_modifiers(flags, c != 8);

		if (mEditMode == tmm_Rotate)
		{
			mRotateCtr.x_ = mCacheBounds.p1.x() * kControlsX1[8] + mCacheBounds.p2.x() * kControlsX2[8];
			mRotateCtr.y_ = mCacheBounds.p1.y() * kControlsY1[8] + mCacheBounds.p2.y() * kControlsY2[8];
//			mRotatePt.x = mCacheBounds.p1.x * kControlsX1[c] + mCacheBounds.p2.x * kControlsX2[c];
//			mRotatePt.y = mCacheBounds.p1.y * kControlsY1[c] + mCacheBounds.p2.y * kControlsY2[c];
		}
	}
	
	Vector2	d(delta);
	
	if(mCacheBounds.xspan() != 0.0 && mCacheBounds.yspan() != 0.0)			// Don't run if bbox is degenerate - we can't preserve its aspect ratio, which is 0 or infinite.
	if(delta.dx != 0.0 || delta.dy != 0.0)									// Don't run if no delta, we get a div-by-zero in vector project.
	if (mEditMode == tmm_Prop_Center || mEditMode == tmm_Prop)
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
	case tmm_Rotate:
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
	case tmm_Center:
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
	case tmm_Prop_Center:
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
	case tmm_Prop:
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
	case tmm_Drag:
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

void	WED_TCEMarqueeTool::ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	Bbox2	new_b;
	if (!GetTotalBounds()) return;
	new_b = mCacheBounds;

	new_b.p1.x_ += (delta.dx * kApplyLinkX1[c]);
	new_b.p2.x_ += (delta.dx * kApplyLinkX2[c]);
	new_b.p1.y_ += (delta.dy * kApplyLinkY1[c]);
	new_b.p2.y_ += (delta.dy * kApplyLinkY2[c]);

	ApplyRescale(mCacheBounds,new_b);
}


bool	WED_TCEMarqueeTool::GetTotalBounds(void) const
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
			if(IsLockedNow(went))		continue;
			if(!IsVisibleNow(went))	continue;
		}

		IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
		if (ent)
		{
			if (iconic && !WED_IsIconic(ent)) iconic = false;
			Bbox2 local;
			ent->GetBounds(gis_UV,local);
			mCacheBounds += local;
		}
	}
	mCacheIconic = iconic;
	return !mCacheBounds.is_null();
}

void	WED_TCEMarqueeTool::ApplyRescale(const Bbox2& old_bounds, const Bbox2& new_bounds)
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
			if(IsLockedNow(went))		continue;
			if(!IsVisibleNow(went))	continue;
		}

		if (ent)
		{
			WED_DrapedOrthophoto * ortho = SAFE_CAST (WED_DrapedOrthophoto, went);
			if (ortho)
				ortho->SetSubTexture(new_bounds);
			else
				ent->Rescale(gis_UV,old_bounds,new_bounds);
			
		}
	}
}

void	WED_TCEMarqueeTool::ApplyRotate(const Point2& ctr, double angle)
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
			if(IsLockedNow(went))		continue;
			if(!IsVisibleNow(went))	continue;
		}

		if (ent)
		{
			WED_DrapedOrthophoto * ortho = SAFE_CAST (WED_DrapedOrthophoto, went);
			if (ortho)
			{
				float hdg;
				hdg = ortho->GetHeading() - angle;
				ortho->SetHeading(hdg);
				                        // this isn't needed to show the new mapping - but it forces
				                        // a re-calculation of the heading in 4-sided non-bezier orthos,
				ortho->Redrape();       // which are to be behave differently wrt heading/UV mapping
			}
			else
				ent->Rotate(gis_UV,ctr, angle);
		}
	}
}
