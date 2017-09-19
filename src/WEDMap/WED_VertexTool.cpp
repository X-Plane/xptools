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

#include "WED_VertexTool.h"
#include "IResolver.h"
#include "ISelection.h"
#include "IOperation.h"
#include "AssertUtils.h"
#include "WED_Persistent.h"
#include "CompGeomDefs2.h"
#include "WED_ToolUtils.h"
#include "WED_RunwayNode.h"
#include "WED_OverlayImage.h"
#include "WED_TextureNode.h"
#include "WED_ExclusionZone.h"
#include "WED_Taxiway.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_Runway.h"
#include "WED_MapZoomerNew.h"
#include "GISUtils.h"
#include "MathUtils.h"
#include "XESConstants.h"
#include "GUI_GraphState.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#define	MIN_HANDLE_RECURSE_SIZE 20
#define SNAP_RADIUS 4

const double kRunwayBlend0[4] = { 0.75,		0.0,	0.75,	0.0		};
const double kRunwayBlend1[4] = { 0.0,		0.25,	0.0,	0.25	};
const double kRunwayBlend2[4] = { 0.0,		0.75,	0.0,	0.75	};
const double kRunwayBlend3[4] = { 0.25,		0.0,	0.25,	0.0		};

const double kQuadBlend0[9] = { 1.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.5, 0.25 };
const double kQuadBlend1[9] = { 0.0, 1.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.25 };
const double kQuadBlend2[9] = { 0.0, 0.0, 1.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.25 };
const double kQuadBlend3[9] = { 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.5, 0.5, 0.25 };

const int kSourceIndex[5] = { 2, 2, 2, 2, 2 };
const int kTargetIndex[5] = { 0, 1, 3, 4, 5 };


WED_VertexTool::WED_VertexTool(
				const char *			tool_name,
				GUI_Pane *				host,
				WED_MapZoomerNew *		zoomer,
				IResolver *				resolver,
				int						sel_verts) :
		WED_HandleToolBase(tool_name, host, zoomer, resolver),
		mEntityCacheKeyArchive(-1),
		mEntityCacheKeyZoomer(-1),
		mSnapCacheKeyArchive(-1),
		mSnapCacheKeyZoomer(-1),
		mInEdit(0),
		mIsRotate(0),
		mIsSymetric(0),
		mIsTaxiSpin(0),
		mNewSplitPoint(NULL),
		mIsScale(0),
		mRotateIndex(-1),
		mSnapToGrid(this,"Snap To Vertices", XML_Name("",""), 0)
{
	SetControlProvider(this);
}

WED_VertexTool::~WED_VertexTool()
{
}

void	WED_VertexTool::BeginEdit(void)
{
	mInEdit = 0;
	mIsRotate = 0;
	mIsSymetric = 0;
	mIsScale = 0;
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->StartOperation("Vertex Modification");  // can be any of - split ATC edge - move ATC edge node
	                                 // - drag a node or modify a bezier node of any previously selected feature
}

void	WED_VertexTool::EndEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->CommitOperation();
	mInEdit = 0;
	mIsRotate = 0;
	mIsSymetric = 0;
	mIsScale = 0;
	mIsTaxiSpin = 0;
}

int		WED_VertexTool::CountEntities(void) const
{
	GetEntityInternal();
	return mEntityCache.size();
}

intptr_t	WED_VertexTool::GetNthEntityID(int n) const
{
	GetEntityInternal();
	return reinterpret_cast<intptr_t>(mEntityCache[n]);
}

int		WED_VertexTool::CountControlHandles(intptr_t id						  ) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	IGISQuad * quad = (en->GetGISSubtype() == WED_ExclusionZone::sClass || en->GetGISSubtype() == WED_OverlayImage::sClass || en->GetGISClass() == gis_Point_HeadingWidthLength || en->GetGISClass() == gis_Line_Width) ? dynamic_cast<IGISQuad *>(en) : NULL;
	WED_Runway * rwy = (en->GetGISSubtype() == WED_Runway::sClass) ? SAFE_CAST(WED_Runway, en) : NULL;

	IGISPoint * pt;
	IGISPoint_Bezier * pt_b;
	IGISPoint_Heading * pt_h;
	IGISLine * ln;
	IGISPointSequence * s;
	IGISEdge * e;

	if (rwy)													return 13;
	else if (quad)												return 9;
	else switch(en->GetGISClass()) {
	case gis_Point:
		if ((pt = SAFE_CAST(IGISPoint,en)) != NULL)				return 1;
		break;
	case gis_Point_Bezier:
		if ((pt_b = SAFE_CAST(IGISPoint_Bezier,en)) != NULL)	return 3;
		break;
	case gis_Point_Heading:
		if ((pt_h = SAFE_CAST(IGISPoint_Heading, en)) != NULL)	return 5;
		break;
	case gis_Line:
		if ((ln = SAFE_CAST(IGISLine,en)) != NULL)				return 2;
		break;
	case gis_PointSequence:
	case gis_Ring:
	case gis_Chain:
		s = dynamic_cast<IGISPointSequence *>(en);
		DebugAssert(s);
		return s ? s->GetNumSides() * 4 : 0;
	case gis_Edge:
		e = dynamic_cast<IGISEdge *>(en);
		DebugAssert(e);
		return e ? 4 : 0;
	}
	return 0;
}

void	WED_VertexTool::GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * dir, float * radius) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	WED_Runway * rwy = (en->GetGISSubtype() == WED_Runway::sClass) ? SAFE_CAST(WED_Runway, en) : NULL;
	IGISQuad * quad = (en->GetGISSubtype() == WED_ExclusionZone::sClass || en->GetGISSubtype() == WED_OverlayImage::sClass || en->GetGISClass() == gis_Point_HeadingWidthLength || en->GetGISClass() == gis_Line_Width) ? dynamic_cast<IGISQuad *>(en) : NULL;

	if (active) *active=1;
	if (con_type) *con_type = handle_Square;
	if (dir) *dir = Vector2();
	if (!p) return;

	IGISPoint * pt;
	IGISPoint_Bezier * pt_b;
	IGISPoint_Heading * pt_h;
	IGISLine * ln;
	IGISPointSequence * s;
	IGISEdge * e;
	if (rwy && n >= 9)
	{
		Point2 corners[4];
		switch(n) {
		case 9:	rwy->GetCornersBlas1(corners);		break;
		case 10:	rwy->GetCornersDisp1(corners);	break;
		case 11:	rwy->GetCornersDisp2(corners);	break;
		case 12:	rwy->GetCornersBlas2(corners);	break;
		}
		p->x_ =	corners[0].x() * kRunwayBlend0[n-9] + corners[1].x() * kRunwayBlend1[n-9] + corners[2].x() * kRunwayBlend2[n-9] + corners[3].x() * kRunwayBlend3[n-9];
		p->y_ =	corners[0].y() * kRunwayBlend0[n-9] + corners[1].y() * kRunwayBlend1[n-9] + corners[2].y() * kRunwayBlend2[n-9] + corners[3].y() * kRunwayBlend3[n-9];

		if (dir)
		{
			rwy->GetCorners(gis_Geo,corners);
			*dir = Vector2(Segment2(corners[0],corners[3]).midpoint(),Segment2(corners[1],corners[2]).midpoint());
			if (n == 9 || n == 11) *dir = -*dir;
		}
		if (con_type) *con_type = handle_ArrowHead;

		return;
	}
	else if (quad)
	{
		Point2	corners[4];
		quad->GetCorners(gis_Geo,corners);

		Point2 ctr;
		ctr.x_ =	corners[0].x() * kQuadBlend0[8] + corners[1].x() * kQuadBlend1[8] + corners[2].x() * kQuadBlend2[8] + corners[3].x() * kQuadBlend3[8];
		ctr.y_ =	corners[0].y() * kQuadBlend0[8] + corners[1].y() * kQuadBlend1[8] + corners[2].y() * kQuadBlend2[8] + corners[3].y() * kQuadBlend3[8];

		if(p)
		{
			p->x_ =	corners[0].x() * kQuadBlend0[n] + corners[1].x() * kQuadBlend1[n] + corners[2].x() * kQuadBlend2[n] + corners[3].x() * kQuadBlend3[n];
			p->y_ =	corners[0].y() * kQuadBlend0[n] + corners[1].y() * kQuadBlend1[n] + corners[2].y() * kQuadBlend2[n] + corners[3].y() * kQuadBlend3[n];
		}
		if (dir && p) *dir = Vector2(ctr,*p);
		if (n < 8 && con_type)
		{
			if (mInEdit)
			{
				if (mIsRotate) *con_type = handle_RotateHead;
				if (mIsScale) *con_type = mIsSymetric ? handle_Arrow : handle_ArrowHead;
			}
			else
			{
				GUI_KeyFlags mods = GetHost()->GetModifiersNow();
				if (mods & gui_OptionAltFlag)
				{
					if (mods & gui_ShiftFlag)	*con_type = handle_Arrow;
					else						*con_type = handle_RotateHead;
				} else {
					if (mods & gui_ShiftFlag)	*con_type = handle_ArrowHead;
				}
			}
		}
		return;
	}
	else switch(en->GetGISClass()) {
	case gis_Point:
		if ((pt = SAFE_CAST(IGISPoint,en)) != NULL)
		{
			if (con_type || radius)
			{
				if (WED_IsIconic(en))
				{
					if (con_type) *con_type = handle_Icon;
					if (radius) *radius = GetFurnitureIconRadius();
				}
				else
				{
					if (con_type) *con_type = handle_VertexSharp;
				}
			}
			pt->GetLocation(gis_Geo,*p);
			return;
		}
		break;
	case gis_Point_Bezier:
		if ((pt_b = SAFE_CAST(IGISPoint_Bezier,en)) != NULL)
		{
				Point2 dummy;
			if (active) *active = (n == 0);
			switch(n) {
			case 0:	pt_b->GetLocation(gis_Geo,*p);
					if (con_type)
					{
						*con_type = handle_VertexSharp;
						if (pt_b->GetControlHandleLo(gis_Geo,dummy) || pt_b->GetControlHandleHi(gis_Geo,dummy))
							*con_type = handle_Vertex;
					}
					break;
			case 1:	if (pt_b->GetControlHandleLo(gis_Geo,*p) && active) *active=1;if (con_type) *con_type = handle_Bezier;if (dir) {pt_b->GetLocation(gis_Geo,dummy);*dir=Vector2(*p,dummy);}break;
			case 2: if (pt_b->GetControlHandleHi(gis_Geo,*p) && active)	*active=1;if (con_type) *con_type = handle_Bezier;if (dir) {pt_b->GetLocation(gis_Geo,dummy);*dir=Vector2(*p,dummy);}break;
			}

			if (active)
			{
				GUI_KeyFlags mods = GetHost()->GetModifiersNow();
				if ((mods & gui_OptionAltFlag) && n == 0 && !pt_b->GetControlHandleHi(gis_Geo,dummy))	*active = 0;
				if ((mods & gui_OptionAltFlag) && n == 2 && !pt_b->GetControlHandleHi(gis_Geo,dummy))	*active = 1;
			}
			return;
		}
		break;
	case gis_Point_Heading:
		if ((pt_h = SAFE_CAST(IGISPoint_Heading, en)) != NULL)
		{
			if(mInEdit)
			{
				if(n!=0)
				{
					if(n == mRotateIndex)
						*p = mTaxiDest;
					else
					{
						Vector2 to_real_handle(mRotateCtr, mTaxiDest);
						
						to_real_handle = VectorLLToMeters(mRotateCtr,to_real_handle);
						int cw_steps = n - mRotateIndex;
						while(cw_steps > 0)
						{
							cw_steps--;
							to_real_handle = to_real_handle.perpendicular_ccw();
						}
						while(cw_steps < 0)
						{
							cw_steps++;
							to_real_handle = to_real_handle.perpendicular_cw();
						}
						to_real_handle = VectorMetersToLL(mRotateCtr, to_real_handle);
						*p = mRotateCtr + to_real_handle;
					}	
					if (dir) *dir = Vector2(mRotateCtr, *p);
					if (con_type) *con_type = handle_Rotate;
				}
				else
				{
					*p = mRotateCtr;
					if (con_type) *con_type = handle_Icon;
					if (radius) *radius = GetFurnitureIconRadius();
				}
			}
			else
			{
				pt_h->GetLocation(gis_Geo,*p);
				Vector2	vdir;
				NorthHeading2VectorMeters(*p,*p,pt_h->GetHeading()+dobmax2(n-1,0)*90.0,vdir);
				
				if(n > 0)
				{
					Point2 orig (*p);
					*p = GetZoomer()->LLToPixel(*p);
					*p += vdir * 15.0;
					*p = GetZoomer()->PixelToLL(*p);
					if (dir) *dir = Vector2(orig, *p);
					if (con_type) *con_type = handle_Rotate;
				} else {
					if (con_type) *con_type = handle_Icon;
					if (radius) *radius = GetFurnitureIconRadius();
				}
			}
			return;
		}
		break;
	case gis_Line:
		if ((ln = SAFE_CAST(IGISLine,en)) != NULL)
		{
			if (n == 0) ln->GetSource()->GetLocation(gis_Geo,*p);
			else		ln->GetTarget()->GetLocation(gis_Geo,*p);
			return;
		}
		break;
	case gis_PointSequence:
	case gis_Chain:
	case gis_Ring:
		s = dynamic_cast<IGISPointSequence *>(en);
		DebugAssert(s);
		if(s)
		{
			if(active) *active = 0;
			if(con_type) *con_type = handle_None;
			Bezier2 bez; Segment2 seg;
			if(!s->GetSide(gis_Geo,n/4, seg, bez))
			{
				bez.p1 = bez.c1 = seg.p1;
				bez.p2 = bez.c2 = seg.p2;
			}
			if(p)
			switch(n % 4) {
			case 0: *p = bez.p1;	break;
			case 1: *p = bez.c1;	break;
			case 2: *p = bez.p2;	break;
			case 3: *p = bez.c2;	break;
			}
			return;
		}
		break;
	case gis_Edge:
		e = dynamic_cast<IGISEdge *>(en);
		DebugAssert(e);
		if(e)
		{
			Bezier2	b;
			Segment2 s;
			if(con_type) *con_type = handle_None;
			if(active) *active = 0;
			
			if(e->GetSide(gis_Geo, 0, s, b))
			{
				switch(n) {
				case 0:
					*p = b.p1;
					if(con_type) *con_type = handle_Vertex;
					break;
				case 1:
					*p = b.c1;
					if(con_type) *con_type = handle_Bezier;
					if(active) *active = 1;
					if(dir) *dir = Vector2(b.c1,b.p1);
					break;
				case 2:
					*p = b.c2;
					if(con_type) *con_type = handle_Bezier;
					if(active) *active = 1;
					if(dir) *dir = Vector2(b.c2,b.p2);
					break;
				case 3:
					*p = b.p2;
					if(con_type) *con_type = handle_Vertex;
					break;
				}
			}
			else
			{
				switch(n) {
				case 0:
					*p = s.p1;
					if(con_type) *con_type = handle_VertexSharp;
					break;
				case 1:
					*p = s.p1;
					if(con_type) *con_type = handle_None;
					break;
				case 2:
					*p = s.p2;
					if(con_type) *con_type = handle_None;
					break;
				case 3:
					*p = s.p2;
					if(con_type) *con_type = handle_VertexSharp;
					break;
				}
			}
			return;
		}
		break;
	}	
	DebugAssert(!"Cast failed!");
	return;
}


int		WED_VertexTool::GetLinks		    (intptr_t id) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	WED_Runway * rwy = (en->GetGISSubtype() == WED_Runway::sClass) ? SAFE_CAST(WED_Runway, en) : NULL;
	IGISQuad * quad = (en->GetGISSubtype() == WED_ExclusionZone::sClass || en->GetGISSubtype() == WED_OverlayImage::sClass || en->GetGISClass() == gis_Point_HeadingWidthLength || en->GetGISClass() == gis_Line_Width) ? dynamic_cast<IGISQuad *>(en) : NULL;

	IGISPoint_Bezier * pt_b;
	IGISPoint_Heading * pt_h;
	IGISLine * ln;
	IGISPointSequence * s;
	IGISEdge * e;
	
	if (quad)															return 4;
	else switch(en->GetGISClass()) {
	case gis_Point_Bezier:
		if ((pt_b = SAFE_CAST(IGISPoint_Bezier,en)) != NULL)			return 2;
		break;
	case gis_Point_Heading:
		if ((pt_h = SAFE_CAST(IGISPoint_Heading, en)) != NULL)			return 1;
		break;
	case gis_Line:
		if ((ln = SAFE_CAST(IGISLine,en)) != NULL)						return 1;
		break;
	case gis_PointSequence:
	case gis_Ring:
	case gis_Chain:
		s = dynamic_cast<IGISPointSequence*>(en);
		return s ? s->GetNumSides() : 0;
	case gis_Edge:
		e = dynamic_cast<IGISEdge*>(en);
		return e ? 3 : 0;
	}
	return 0;
}

void	WED_VertexTool::GetNthLinkInfo		(intptr_t id, int n, bool * active, LinkType_t * ltype) const
{
	if (active) *active=0;
	if (ltype) *ltype = link_Solid;

	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	IGISPoint_Bezier * pt_b;
	IGISEdge * e;

	switch(en->GetGISClass()) {
	case gis_Point_Bezier:
		if ((pt_b = SAFE_CAST(IGISPoint_Bezier,en)) != NULL)
		if (ltype)
			*ltype = link_BezierCtrl;
		break;
	case gis_PointSequence:
	case gis_Ring:
	case gis_Chain:
		if(ltype) *ltype = link_None;
		if(active) *active = 1;
		break;
	case gis_Edge:
		if(n == 0)
		{
			if(ltype) *ltype = link_None;
			if(active) *active = 1;
		}
		else
		{
			if(ltype) *ltype = link_BezierCtrl;
			if(active) *active = 0;
		}
	}
}


int		WED_VertexTool::GetNthLinkSource   (intptr_t id, int n) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	WED_Runway * rwy = (en->GetGISSubtype() == WED_Runway::sClass) ? SAFE_CAST(WED_Runway, en) : NULL;
	IGISQuad * quad = (en->GetGISSubtype() == WED_ExclusionZone::sClass || en->GetGISSubtype() == WED_OverlayImage::sClass || en->GetGISClass() == gis_Point_HeadingWidthLength || en->GetGISClass() == gis_Line_Width) ? dynamic_cast<IGISQuad *>(en) : NULL;

	if (quad) return n;
	switch(en->GetGISClass()) {
	case gis_Point_Bezier:		return 0;
	case gis_Point_Heading:		return 0;
	case gis_Line:				return 0;
	case gis_Line_Width:		return kSourceIndex[n];
	case gis_PointSequence:
	case gis_Ring:
	case gis_Chain:				return n*4;
	case gis_Edge:
		switch(n) {
		case 0: return 0;
		case 1: return 0;
		case 2: return 3;
		}
	}
	return 0;
}

int		WED_VertexTool::GetNthLinkSourceCtl(intptr_t id, int n) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	switch(en->GetGISClass()) {
	case gis_PointSequence:
	case gis_Ring:
	case gis_Chain:				return n*4+1;
	case gis_Edge:
		switch(n) {
		case 0: return 1;
		case 1: return 0;
		case 2: return 3;
		}
	}
	return -1;
}

int		WED_VertexTool::GetNthLinkTarget   (intptr_t id, int n) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	WED_Runway * rwy = (en->GetGISSubtype() == WED_Runway::sClass) ? SAFE_CAST(WED_Runway, en) : NULL;
	IGISQuad * quad = (en->GetGISSubtype() == WED_ExclusionZone::sClass || en->GetGISSubtype() == WED_OverlayImage::sClass || en->GetGISClass() == gis_Point_HeadingWidthLength || en->GetGISClass() == gis_Line_Width) ? dynamic_cast<IGISQuad *>(en) : NULL;

	if (quad) return (n+1)%4;
	switch(en->GetGISClass()) {
	case gis_Point_Bezier:		return n+1;
	case gis_Point_Heading:		return 1;
	case gis_Line:				return 1;
	case gis_PointSequence:
	case gis_Ring:
	case gis_Chain:				return n*4+2;
	case gis_Edge:
		switch(n) {
		case 0: return 3;
		case 1: return 1;
		case 2: return 2;
		}
	}
	return 0;
}

int		WED_VertexTool::GetNthLinkTargetCtl(intptr_t id, int n) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	switch(en->GetGISClass()) {
	case gis_PointSequence:
	case gis_Ring:
	case gis_Chain:				return n*4+3;
	case gis_Edge:
		switch(n) {
		case 0: return 2;
		case 1: return 1;
		case 2: return 2;
		}
	}
	return -1;
}

bool	WED_VertexTool::PointOnStructure(intptr_t id, const Point2& p) const
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	WED_Taxiway * taxi = (en->GetGISSubtype() == WED_Taxiway::sClass) ? SAFE_CAST(WED_Taxiway, en) : NULL;
	WED_PolygonPlacement * poly = (en->GetGISSubtype() == WED_PolygonPlacement::sClass) ? SAFE_CAST(WED_PolygonPlacement, en) : NULL;
	if (taxi || poly)
	{
		if (GetHost()->GetModifiersNow() & gui_ShiftFlag)
		if (en->PtWithin(gis_Geo,p))
		{
			mRotateCtr = p;
			mTaxiDest = p;
			return true;
		}
	}
	return false;
}

void	WED_VertexTool::ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_handle)
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	WED_Taxiway * taxi = (en->GetGISSubtype() == WED_Taxiway::sClass) ? SAFE_CAST(WED_Taxiway, en) : NULL;
	WED_PolygonPlacement * poly = (en->GetGISSubtype() == WED_PolygonPlacement::sClass) ? SAFE_CAST(WED_PolygonPlacement, en) : NULL;
	io_handle += delta;
	if (taxi)
	{
		taxi->SetHeading(VectorDegs2NorthHeading(mRotateCtr, mRotateCtr, Vector2(mRotateCtr,io_handle)));
		mIsTaxiSpin = 1;
		mTaxiDest = io_handle;
	}
	if(poly)
	{
		poly->SetHeading(VectorDegs2NorthHeading(mRotateCtr, mRotateCtr, Vector2(mRotateCtr,io_handle)));
		mIsTaxiSpin = 1;
		mTaxiDest = io_handle;
	}
}

void	WED_VertexTool::ControlsHandlesBy(intptr_t id, int n, const Vector2& delta, Point2& io_pt)
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	WED_Runway * rwy = (en->GetGISSubtype() == WED_Runway::sClass) ? SAFE_CAST(WED_Runway, en) : NULL;
	IGISQuad * quad = (en->GetGISSubtype() == WED_ExclusionZone::sClass || en->GetGISSubtype() == WED_OverlayImage::sClass || en->GetGISClass() == gis_Point_HeadingWidthLength || en->GetGISClass() == gis_Line_Width) ? dynamic_cast<IGISQuad *>(en) : NULL;

	IGISPoint * pt;
	IGISPoint_Bezier * pt_b;
	IGISPoint_Heading * pt_h;
	IGISLine * ln;
	IGISEdge * e;

	Point2	p;
	if (rwy && n >= 9)
	{
		Point2	handle;
		this->GetNthControlHandle(id, n, NULL, NULL, &handle, NULL, NULL);
		if (!mInEdit)
		{
			mInEdit = 1;
			io_pt = handle;
		}
		Vector2	slop(handle, io_pt);
		Point2	p1, p2;
		rwy->GetSource()->GetLocation(gis_Geo,p1);
		rwy->GetTarget()->GetLocation(gis_Geo,p2);
		Vector2	axis(p1,p2);
		Vector2 delta_m;
		axis = VectorLLToMeters(p1,axis);
		if (n == 9 || n == 11) axis = -axis;
		axis.normalize();
		delta_m = VectorLLToMeters(p1,delta + slop);

		switch(n) {
		case 9 :	rwy->SetBlas1(rwy->GetBlas1() + axis.dot(delta_m));	break;
		case 10:	rwy->SetDisp1(rwy->GetDisp1() + axis.dot(delta_m));	break;
		case 11:	rwy->SetDisp2(rwy->GetDisp2() + axis.dot(delta_m));	break;
		case 12:	rwy->SetBlas2(rwy->GetBlas2() + axis.dot(delta_m));	break;
		}

		io_pt += delta;
		return;
	}
	else if (quad)
	{
		if (!mInEdit)
		{
			mInEdit = 1;
			if (n < 8)
			{
				GUI_KeyFlags mods = GetHost()->GetModifiersNow();
				mIsScale = mods & gui_ShiftFlag;
				if (mods & gui_OptionAltFlag)
				{
					if (mIsScale)	mIsSymetric = 1;
					else			mIsRotate = 1;
				}
				if (mIsRotate)
				{
					Point2	corners[4];
					quad->GetCorners(gis_Geo,corners);
					mRotateCtr.x_ = corners[0].x() * 0.25 + corners[1].x() * 0.25 + corners[2].x() * 0.25 + corners[3].x() * 0.25;
					mRotateCtr.y_ = corners[0].y() * 0.25 + corners[1].y() * 0.25 + corners[2].y() * 0.25 + corners[3].y() * 0.25;

					GetNthControlHandle(id, n, NULL, NULL, &io_pt, NULL, NULL);
				}
			}
		}

		if (mIsRotate)
		{
			quad->Rotate(gis_Geo,mRotateCtr,WED_CalcDragAngle(mRotateCtr,io_pt,delta));
			io_pt += delta;
		}
		else if (mIsScale)
		{
			if (n >= 4)				quad->ResizeSide(gis_Geo,n-4, delta, mIsSymetric);
			else					quad->ResizeCorner(gis_Geo,n, delta, mIsSymetric);
		}
		else if (n >= 8)
		{
			Bbox2	oldb, newb;
			quad->GetBounds(gis_Geo,oldb);
			newb = oldb;
			newb.p1 += delta;
			newb.p2 += delta;
			quad->Rescale(gis_Geo,oldb,newb);
		}
		else if (n >= 4)			quad->MoveSide(gis_Geo,n-4, delta);
		else						quad->MoveCorner(gis_Geo,n,delta);
		return;
	}
	else switch(en->GetGISClass()) {
	case gis_Point:
		if ((pt = SAFE_CAST(IGISPoint,en)) != NULL)
		{
			io_pt += delta;
			pt->GetLocation(gis_Geo,p);
			SnapMovePoint(io_pt,p, en);
			pt->SetLocation(gis_Geo,p);
			return;
		}
		break;
	case gis_Point_Bezier:
		if ((pt_b = SAFE_CAST(IGISPoint_Bezier,en)) != NULL)
		{
			if (!mInEdit)
			{
				pt_b->GetLocation(gis_Geo,p);
				Point2 dummy;
				mInEdit = 1;
				GUI_KeyFlags mods = GetHost()->GetModifiersNow();
				if ((mods & gui_OptionAltFlag) && (mods & gui_ShiftFlag))
				{
					if (n == 1) { pt_b->SetSplit(true);	if (pt_b->GetControlHandleLo(gis_Geo,dummy))	pt_b->DeleteHandleLo(); else pt_b->SetControlHandleLo(gis_Geo,p+delta); }
					if (n == 2) { pt_b->SetSplit(true); if (pt_b->GetControlHandleHi(gis_Geo,dummy))	pt_b->DeleteHandleHi(); else pt_b->SetControlHandleHi(gis_Geo,p+delta); }
				}
				else if (mods & gui_OptionAltFlag)
				{
					if (n == 1) { if (pt_b->GetControlHandleLo(gis_Geo,dummy))	pt_b->SetSplit(true); else { pt_b->SetSplit(false); pt_b->SetControlHandleLo(gis_Geo,p+delta); } }
					if (n == 2) { if (pt_b->GetControlHandleHi(gis_Geo,dummy))	pt_b->SetSplit(true); else { pt_b->SetSplit(false); pt_b->SetControlHandleHi(gis_Geo,p+delta); } }
				}
				else if (mods & gui_ShiftFlag && n != 0)
				{
					if (pt_b->IsSplit()) pt_b->SetSplit(false);
					else {
						if (n == 1) { pt_b->DeleteHandleLo(); return; }
						if (n == 2) { pt_b->DeleteHandleHi(); return; }
					}
				}
			}
			switch(n) {
			case 0:	pt_b->GetLocation(gis_Geo,p);						break;
			case 1:	if (!pt_b->GetControlHandleLo(gis_Geo,p)) n=3;	break;
			case 2: if (!pt_b->GetControlHandleHi(gis_Geo,p)) n=3;	break;
			}
			io_pt += delta;
			SnapMovePoint(io_pt,p, en);
			switch(n) {
			case 0:	pt_b->SetLocation(gis_Geo,p);	break;
			case 1:	pt_b->SetControlHandleLo(gis_Geo,p);	break;
			case 2: pt_b->SetControlHandleHi(gis_Geo,p);	break;
			}
#if 1   // redrape upon modificatoin of bezier node handles or location
			WED_Thing * node = dynamic_cast <WED_Thing *> (en);
			node = node->GetParent();
			node = node->GetParent();
			WED_DrapedOrthophoto * ortho = SAFE_CAST (WED_DrapedOrthophoto,node);
			if (ortho) ortho->Redrape();
#endif
			return;
		}
		break;
	case gis_Point_Heading:
		if ((pt_h = SAFE_CAST(IGISPoint_Heading, en)) != NULL)
		{
			pt_h->GetLocation(gis_Geo,p);
			if (n == 0)
			{
				io_pt += delta;
				SnapMovePoint(io_pt,p, en);
				pt_h->SetLocation(gis_Geo,p);
			} else {
				if(!mInEdit)
				{
					mRotateIndex = n;
					mInEdit = 1;
					mRotateCtr = p;
					Point2 me = p;
					Vector2	dir;
					mRotateOffset = (dobmax2(0,n-1)) * 90.0;
					NorthHeading2VectorMeters(p,p,pt_h->GetHeading()+mRotateOffset,dir);
					p = GetZoomer()->LLToPixel(p);
					p += dir * 15.0;
					p = GetZoomer()->PixelToLL(p);
					mTaxiDest = p;
				}

				mTaxiDest += delta;				
				Vector2 dir = Vector2(mRotateCtr,mTaxiDest);
				dir.normalize();
				pt_h->SetHeading(VectorDegs2NorthHeading(mRotateCtr,mRotateCtr,dir)-mRotateOffset);
			}
			return;
		}
		break;
	case gis_Line:
		if ((ln = SAFE_CAST(IGISLine,en)) != NULL)
		{
			if (n == 0) ln->GetSource()->GetLocation(gis_Geo,p);
			else		ln->GetTarget()->GetLocation(gis_Geo,p);
			p += delta;
			if (n == 0) ln->GetSource()->SetLocation(gis_Geo,p);
			else		ln->GetTarget()->SetLocation(gis_Geo,p);
			return;
		}
		break;
	case gis_Edge:
		if((e = SAFE_CAST(IGISEdge,en)) != NULL)
		{
			Segment2 s;
			Bezier2 b;
			if(e->GetSide(gis_Geo, 0, s, b))
			{
				if(n == 1)
					b.c1 += delta;
				if(n == 2)
					b.c2 += delta;
				e->SetSideBezier(gis_Geo, b);
			}
			return;
		}
	}
	DebugAssert(!"Cast failed!");
	return;
}

void	WED_VertexTool::ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	IGISEntity * en = reinterpret_cast<IGISEntity *>(id);
	IGISPointSequence * seq = dynamic_cast<IGISPointSequence *>(en);
	GUI_KeyFlags mods = GetHost()->GetModifiersNow();
	if(seq && !mInEdit)
	{
		mInEdit = 1;
		if (mods & gui_OptionAltFlag)
		{
			mNewSplitPoint = seq->SplitSide(io_pt, GetZoomer()->GetClickRadius(4));
		}
		else
		{
			Point2 p1,p2;
			int np = seq->GetNumPoints();
			seq->GetNthPoint(c)->GetLocation(gis_Geo,p1);
			seq->GetNthPoint((c+1) % np)->GetLocation(gis_Geo,p2);
			mPointOffset1 = Vector2(io_pt,p1);
			mPointOffset2 = Vector2(io_pt,p2);

			mNewSplitPoint = NULL;
		}
	}

	Bbox2	old_b(Point2(0,0),Point2(1,1));
	Bbox2	new_b(old_b);
	
	if(mNewSplitPoint)
	{
		new_b += delta;
		mNewSplitPoint->Rescale(gis_Geo, old_b, new_b);
	}
	else if(seq)
	{
		int np = seq->GetNumPoints();
		IGISPoint * gp1 = seq->GetNthPoint(c);
		IGISPoint * gp2 = seq->GetNthPoint((c+1) % np);

		Point2 p1,p2;
		gp1->GetLocation(gis_Geo,p1);
		gp2->GetLocation(gis_Geo,p2);

		io_pt += delta;

		if (mods & gui_ControlFlag)
		{
			mPointOffset1 = Vector2(io_pt,p1);
			mPointOffset2 = Vector2(io_pt,p2);

			Vector2 n = VectorLLToMeters(p1,Vector2(p1,p2));
			n = n.perpendicular_ccw();
			Vector2 delta_m = VectorLLToMeters(p1,delta);
			new_b += VectorMetersToLL(p1,n.projection(delta_m)); 

			gp1->Rescale(gis_Geo, old_b, new_b);
			gp2->Rescale(gis_Geo, old_b, new_b);
		}
		else
		{
			double dist1 = 9.9e9;
			double dist2 = 9.9e9;
			Point2 sp1,sp2;

			if(SnapMovePoint(io_pt + mPointOffset1,sp1,gp1))
			{
				dist1 = Vector2(
				GetZoomer()->LLToPixel(p1),
				GetZoomer()->LLToPixel(sp1)).squared_length();
			}

			if(SnapMovePoint(io_pt + mPointOffset2,sp2,gp2))
			{
				dist2 = Vector2(
				GetZoomer()->LLToPixel(p2),
				GetZoomer()->LLToPixel(sp2)).squared_length();
			}
			
			//mroe: thats to makes sure that [at least] the snap points exactly matches . 
			//the vector math is precise  , so this is possibly a bit paranoid .  
			if(dist1 <= dist2) 
			{
				gp1->SetLocation(gis_Geo,sp1);
				gp2->SetLocation(gis_Geo,p2 + Vector2(p1,sp1));
			}
			else
			{
				gp1->SetLocation(gis_Geo,p1 + Vector2(p2,sp2));
				gp2->SetLocation(gis_Geo,sp2);
			}
		}
	}

#if 1    // redrape upon splitting of a segment
	WED_Thing * node = dynamic_cast <WED_Thing *> (en);
	WED_DrapedOrthophoto * ortho = SAFE_CAST(WED_DrapedOrthophoto,node->GetParent());
	if (ortho) ortho->Redrape();
#endif	
}

WED_HandleToolBase::EntityHandling_t	WED_VertexTool::TraverseEntity(IGISEntity * ent, int pt_sel)
{
	// Ben says: we tried always selecting INTO "fake" composite entities - that is, entities that are
	// not technically composite but have a hierarchy.  So we would always selected polygon nodes and not
	// the whole polygon.
	// BUT: atomic-or-container is better:
	// 1.	if we select the whole poly, we can STILL drag each vertex, that works automagically as part of
	//		the object-decomposition we do.
	// 2.	it is weird to the user to marquee an object and NOT get the whole thing selected.
	switch(ent->GetGISClass()) {
	case gis_Composite:		return	ent_Container;
	case gis_Polygon:		return	pt_sel ? ent_AtomicOrContainer : ent_Container;
	case gis_PointSequence: return	ent_Container;
	case gis_Ring:			return	pt_sel ? ent_AtomicOrContainer : ent_Container;		// single click on the chain edge?  Grab the chain.  But drag to encompass?  collect vertices but don't "move up" to the chain.
	case gis_Chain:			return	pt_sel ? ent_AtomicOrContainer : ent_Container;		// single click on the chain edge?  Grab the chain.  But drag to encompass?  collect vertices but don't "move up" to the chain.
	case gis_Line:			return	ent_AtomicOrContainer;
	case gis_Line_Width:	return	ent_AtomicOrContainer;
	case gis_Edge:			return	ent_AtomicOrContainer;
	default:				return	ent_Atomic;
	}
}

void WED_VertexTool::GetEntityInternal(void) const
{
	ISelection * sel = WED_GetSelect(GetResolver());
	WED_Thing * wrl = WED_GetWorld(GetResolver());
	long long key_a = wrl->GetArchive()->CacheKey();
	long long key_z = GetZoomer()->CacheKey();
	if (key_a == mEntityCacheKeyArchive && key_z == mEntityCacheKeyZoomer) return;

	mEntityCacheKeyArchive = key_a;
	mEntityCacheKeyZoomer = key_z;

	DebugAssert(sel != NULL);
	vector<ISelectable *>	iu;

	mEntityCache.clear();


	sel->GetSelectionVector(iu);
	if (iu.empty()) return;
	mEntityCache.reserve(iu.size());

	Bbox2	bounds;
	GetZoomer()->GetMapVisibleBounds(bounds.p1.x_,bounds.p1.y_,bounds.p2.x_,bounds.p2.y_);

	for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
	{
		IGISEntity * gent = SAFE_CAST(IGISEntity,*i);
		if (gent) AddEntityRecursive(gent, bounds);
	}
}

void		WED_VertexTool::AddEntityRecursive(IGISEntity * e, const Bbox2& vis_area ) const
{
	if(!IsVisibleNow(e))	return;
	if(IsLockedNow(e))		return;
	if(!e->Cull(vis_area)) return;

	Bbox2	ent_bounds;
	e->GetBounds(gis_Geo,ent_bounds);

	ent_bounds.p1 = GetZoomer()->LLToPixel(ent_bounds.p1);
	ent_bounds.p2 = GetZoomer()->LLToPixel(ent_bounds.p2);

	IGISPointSequence * ps;
	IGISPolygon * poly;
	IGISComposite * cmp;
	int c, n;

	switch(e->GetGISClass()) {
	case gis_Point:
	case gis_Point_Bezier:
	case gis_Point_Heading:
	case gis_Point_HeadingWidthLength:
	case gis_Line:
	case gis_Line_Width:
		mEntityCache.push_back(e);
		break;
	case gis_PointSequence:
	case gis_Ring:
	case gis_Edge:
	case gis_Chain:
		if (ent_bounds.xspan() < MIN_HANDLE_RECURSE_SIZE &&
			ent_bounds.yspan() < MIN_HANDLE_RECURSE_SIZE) return;

		if ((ps = SAFE_CAST(IGISPointSequence, e)) != NULL)
		{
			mEntityCache.push_back(e);		
			c = ps->GetNumPoints();
			for (n = 0; n < c; ++n)
				AddEntityRecursive(ps->GetNthPoint(n),vis_area);
		}
		break;
	case gis_BoundingBox:
		if (ent_bounds.xspan() < MIN_HANDLE_RECURSE_SIZE &&
			ent_bounds.yspan() < MIN_HANDLE_RECURSE_SIZE) return;
		mEntityCache.push_back(e);
		break;
	case gis_Polygon:
		if (ent_bounds.xspan() < MIN_HANDLE_RECURSE_SIZE &&
			ent_bounds.yspan() < MIN_HANDLE_RECURSE_SIZE) return;

		if (e->GetGISSubtype() == WED_OverlayImage::sClass && SAFE_CAST(WED_OverlayImage, e))
			mEntityCache.push_back(e);
		else if ((poly = SAFE_CAST(IGISPolygon, e)) != NULL)
		{
			if (e->GetGISSubtype() == WED_Taxiway::sClass && SAFE_CAST(WED_Taxiway, e))
				mEntityCache.push_back(e);
			if (e->GetGISSubtype() == WED_PolygonPlacement::sClass && SAFE_CAST(WED_PolygonPlacement, e))
				mEntityCache.push_back(e);
			AddEntityRecursive(poly->GetOuterRing(),vis_area);
			c = poly->GetNumHoles();
			for (n = 0; n < c; ++n)
				AddEntityRecursive(poly->GetNthHole(n),vis_area);
		}
		break;
	case gis_Composite:
		if (ent_bounds.xspan() < MIN_HANDLE_RECURSE_SIZE &&
			ent_bounds.yspan() < MIN_HANDLE_RECURSE_SIZE) return;

		if ((cmp = SAFE_CAST(IGISComposite, e)) != NULL)
		{
			c = cmp->GetNumEntities();
			for (n = 0; n < c; ++n)
				AddEntityRecursive(cmp->GetNthEntity(n),vis_area);
		}
		break;
	}
}


void		WED_VertexTool::AddSnapPointRecursive(IGISEntity * e, const Bbox2& vis_area, ISelection * sel ) const
{
	if(!IsVisibleNow(e))	return;

	Bbox2	ent_bounds;
	e->GetBounds(gis_Geo,ent_bounds);

	if (!ent_bounds.overlap(vis_area))	return;

	ent_bounds.p1 = GetZoomer()->LLToPixel(ent_bounds.p1);
	ent_bounds.p2 = GetZoomer()->LLToPixel(ent_bounds.p2);

	IGISPointSequence * ps;
	IGISPolygon * poly;
	IGISComposite * cmp;
	int c, n;
	IGISPoint * pt;
	IGISPoint_Bezier * bt;
	Point2	loc;

	switch(e->GetGISClass()) {
	case gis_Point:
	case gis_Point_Heading:
	case gis_Point_HeadingWidthLength:
		pt = SAFE_CAST(IGISPoint, e);
		if (pt)
		{
			pt->GetLocation(gis_Geo,loc);
			mSnapCache.push_back(pair<Point2,IGISEntity *>(loc, e));
		}
		break;
	case gis_Point_Bezier:
		bt = SAFE_CAST(IGISPoint_Bezier, e);
		if (bt)
		{
			bt->GetLocation(gis_Geo,loc);
			mSnapCache.push_back(pair<Point2,IGISEntity *>(loc, e));
//			if (sel->IsSelected(e))
			{
				if (bt->GetControlHandleLo(gis_Geo,loc))
					mSnapCache.push_back(pair<Point2,IGISEntity *>(loc, e));
				if (bt->GetControlHandleHi(gis_Geo,loc))
					mSnapCache.push_back(pair<Point2,IGISEntity *>(loc, e));
			}
		}
		break;
	case gis_Line:
	case gis_Line_Width:
	case gis_PointSequence:
	case gis_Ring:
	case gis_Edge:
	case gis_Chain:
		if (ent_bounds.xspan() < MIN_HANDLE_RECURSE_SIZE &&
			ent_bounds.yspan() < MIN_HANDLE_RECURSE_SIZE) return;

		if ((ps = SAFE_CAST(IGISPointSequence, e)) != NULL)
		{
			c = ps->GetNumPoints();
			for (n = 0; n < c; ++n)
				AddSnapPointRecursive(ps->GetNthPoint(n),vis_area, sel);
		}
		break;
	case gis_Polygon:
		if (ent_bounds.xspan() < MIN_HANDLE_RECURSE_SIZE &&
			ent_bounds.yspan() < MIN_HANDLE_RECURSE_SIZE) return;

		if ((poly = SAFE_CAST(IGISPolygon, e)) != NULL)
		{
			AddSnapPointRecursive(poly->GetOuterRing(),vis_area, sel);
			c = poly->GetNumHoles();
			for (n = 0; n < c; ++n)
				AddSnapPointRecursive(poly->GetNthHole(n),vis_area, sel);
		}
		break;
	case gis_Composite:
		if (ent_bounds.xspan() < MIN_HANDLE_RECURSE_SIZE &&
			ent_bounds.yspan() < MIN_HANDLE_RECURSE_SIZE) return;

		if ((cmp = SAFE_CAST(IGISComposite, e)) != NULL)
		{
			c = cmp->GetNumEntities();
			for (n = 0; n < c; ++n)
				AddSnapPointRecursive(cmp->GetNthEntity(n),vis_area, sel);
		}
		break;
	}
}

bool		WED_VertexTool::SnapMovePoint(
					const Point2&			ideal_track_pt,		// This is the ideal place the user is TRYING to drag the thing, without snapping
					Point2&					io_thing_pt,		// And this is where the thing is right now - we will move it to a NEW loc
					IGISEntity *			who)
{
	Point2	modi(ideal_track_pt);
	double smallest_dist=9.9e9;
	Point2	best(modi);
	bool IsSnap = false;

	if (mSnapToGrid)
	{
		ISelection * sel = WED_GetSelect(GetResolver());
		WED_Thing * wrl = WED_GetWorld(GetResolver());
		long long key_a = wrl->GetArchive()->CacheKey();
		long long key_z = GetZoomer()->CacheKey();
		if (key_a != mSnapCacheKeyArchive || key_z != mSnapCacheKeyZoomer)
		{
			mSnapCache.clear();
			Bbox2	bounds;
			GetZoomer()->GetMapVisibleBounds(bounds.p1.x_,bounds.p1.y_,bounds.p2.x_,bounds.p2.y_);

			AddSnapPointRecursive(dynamic_cast<IGISEntity *>(wrl), bounds, sel);
		}

		Point2  posi;
		IGISPoint * pt;
		GetEntityInternal();
		for(int n = 0; n < mSnapCache.size(); ++n)
		if (mSnapCache[n].second != who)
		{
			posi = mSnapCache[n].first;

			double dist = Vector2(
				GetZoomer()->LLToPixel(posi),
				GetZoomer()->LLToPixel(modi)).squared_length();
			if (dist < (SNAP_RADIUS * SNAP_RADIUS) &&
				dist < smallest_dist)
			{
				smallest_dist = dist;
				best = posi;
				IsSnap = true;
			}
		}
	}

	io_thing_pt = best;
	return IsSnap;
}


void		WED_VertexTool::DrawSelected			(bool inCurrent, GUI_GraphState * g)
{
	WED_HandleToolBase::DrawSelected(inCurrent, g);
	if (mIsTaxiSpin)
	{
		g->SetState(false,false, false, true, true, false, false);
		glColor4f(1,1,1,0.5);
		glBegin(GL_LINES);
		glVertex2f(GetZoomer()->LonToXPixel(mRotateCtr.x()),GetZoomer()->LatToYPixel(mRotateCtr.y()));
		glVertex2f(GetZoomer()->LonToXPixel(mTaxiDest.x()),GetZoomer()->LatToYPixel(mTaxiDest.y()));
		glEnd();
	}
}
