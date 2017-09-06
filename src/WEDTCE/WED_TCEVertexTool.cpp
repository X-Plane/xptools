/*
 * Copyright (c) 2009, Laminar Research.  All rights reserved.
 *
 */

#include "WED_TCEVertexTool.h"
#include "ISelection.h"
#include "IOperation.h"
#include "WED_ToolUtils.h"
#include "AssertUtils.h"
#include "IGIS.h"
#include "WED_EnumSystem.h"
#include "WED_DrapedOrthophoto.h"

WED_TCEVertexTool::WED_TCEVertexTool(
							const char *			tool_name,
							GUI_Pane *				host,
							WED_MapZoomerNew *		zoomer,
							IResolver *				resolver) :
	WED_HandleToolBase(tool_name, host, zoomer, resolver),
	mGrid(this,"Snap To Grid", XML_Name("",""), TCE_GridSnap, tce_Grid_None)	
{
	SetCanSelect(0);
	SetDrawAlways(1);
	SetControlProvider(this);
}

WED_TCEVertexTool::~WED_TCEVertexTool()
{
}

void		WED_TCEVertexTool::BeginEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->StartOperation("UVmap Modification");
}

void		WED_TCEVertexTool::EndEdit(void)
{
	ISelection * sel = WED_GetSelect(GetResolver());
	IOperation * op = dynamic_cast<IOperation *>(sel);
	DebugAssert(sel != NULL && op != NULL);
	op->CommitOperation();
}

int			WED_TCEVertexTool::CountEntities(void) const
{
	SyncCache();
	return mCache.size();
}

intptr_t		WED_TCEVertexTool::GetNthEntityID(int n) const
{
	SyncCache();
	return reinterpret_cast<intptr_t>(mCache[n]);
}

int			WED_TCEVertexTool::CountControlHandles(intptr_t id						  ) const
{
	IGISPointSequence * s = NULL;
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	switch(who->GetGISClass()) {
	case gis_Point:
	case gis_Point_Heading:
	case gis_Point_HeadingWidthLength:
		return 1;
	case gis_Point_Bezier:
		return 3;
	case gis_PointSequence:
	case gis_Line:
	case gis_Line_Width:
	case gis_Ring:
	case gis_Chain:
		s = dynamic_cast<IGISPointSequence *>(who);
		DebugAssert(s);
		return s ? s->GetNumSides() * 4: 0;
	default:
		return 0;
	}
}

void		WED_TCEVertexTool::GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const
{
	bool dummy_a;
	Point2 dummy_p;
	HandleType_t dummy_h;
	if(!active) active=&dummy_a;
	if(!p)		p=&dummy_p;
	if(!con_type) con_type = &dummy_h;

	if(direction) *direction = Vector2(1,0);

	*active = 0;
	*con_type = handle_None;
	*p = Point2(0,0);

	IGISPointSequence * s = NULL;
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	IGISPoint * pt;
	IGISPoint_Bezier * pt_bt;


	switch(who->GetGISClass()) {
	case gis_Point:
	case gis_Point_Heading:
	case gis_Point_HeadingWidthLength:
		if((pt = dynamic_cast<IGISPoint *>(who)) != NULL)
		{
			*active = 1;
			*con_type = handle_VertexSharp;
			pt->GetLocation(gis_UV,*p);
		}
		break;
	case gis_Point_Bezier:
		if((pt_bt = dynamic_cast<IGISPoint_Bezier *>(who)) != NULL)
		switch(n) {
		case 0:
			*active = 1;
			*con_type = handle_Vertex;
			pt_bt->GetLocation(gis_UV,*p);
			break;
		case 1:
			*con_type = handle_Bezier;
			*active = pt_bt->GetControlHandleLo(gis_UV,*p);
			break;
		case 2:
			*con_type = handle_Bezier;
			*active = pt_bt->GetControlHandleHi(gis_UV,*p);
			break;
		}
		break;
	case gis_PointSequence:
	case gis_Line:
	case gis_Line_Width:
	case gis_Ring:
	case gis_Chain:
		s = dynamic_cast<IGISPointSequence *>(who);
		DebugAssert(s);
		if(s)
		{
			*active = 0;
			*con_type = handle_None;
			Bezier2 bez; Segment2 seg;
			if(!s->GetSide(gis_UV,n/4, seg, bez))
			{
				bez.p1 = bez.c1 = seg.p1;
				bez.p2 = bez.c2 = seg.p2;
			}
			switch(n % 4) {
			case 0: *p = bez.p1;	break;
			case 1: *p = bez.c1;	break;
			case 2: *p = bez.p2;	break;
			case 3: *p = bez.c2;	break;
			}
		}
		break;
	}
}

int			WED_TCEVertexTool::GetLinks		    (intptr_t id) const
{
	IGISPointSequence * s = NULL;
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	switch(who->GetGISClass()) {
	case gis_Point_Bezier:
		return 2;
	case gis_PointSequence:
	case gis_Line:
	case gis_Line_Width:
	case gis_Ring:
	case gis_Chain:
		s = dynamic_cast<IGISPointSequence*>(who);
		return s ? s->GetNumSides() : 0;
	default:
		return 0;
	}
}

void		WED_TCEVertexTool::GetNthLinkInfo		(intptr_t id, int n, bool * active, LinkType_t * ltype) const
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	if (who->GetGISClass() == gis_Point_Bezier)
	{
		IGISPoint_Bezier * pt_bt = dynamic_cast<IGISPoint_Bezier*>(who);
		if(active) *active = 0;
		if(ltype && pt_bt)
		{
			Point2 p;
			if(n == 0)	*ltype = pt_bt->GetControlHandleHi(gis_UV, p) ? link_BezierCtrl : link_None;
			else		*ltype = pt_bt->GetControlHandleHi(gis_UV, p) ? link_BezierCtrl : link_None;
		}
	} else {
		if(active) *active = true;
		if(ltype) *ltype = link_Solid;
	}
}

int			WED_TCEVertexTool::GetNthLinkSource   (intptr_t id, int n) const
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	if (who->GetGISClass() == gis_Point_Bezier)
		return 0;
	else
		return n*4;
}

int			WED_TCEVertexTool::GetNthLinkSourceCtl(intptr_t id, int n) const
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	if (who->GetGISClass() == gis_Point_Bezier)
		return -1;
	else
		return n*4+1;
}

int			WED_TCEVertexTool::GetNthLinkTarget   (intptr_t id, int n) const
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	if (who->GetGISClass() == gis_Point_Bezier)
		return n+1;
	else
		return n*4+2;
}

int			WED_TCEVertexTool::GetNthLinkTargetCtl(intptr_t id, int n) const
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	if (who->GetGISClass() == gis_Point_Bezier)
		return -1;
	else
		return n*4+3;
}

bool		WED_TCEVertexTool::PointOnStructure(intptr_t id, const Point2& p) const
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	if (who->GetGISClass() == gis_Polygon)
	if (who->PtWithin(gis_UV,p)) return true;
	
	return false;
}

static void updateUVbounds(IGISEntity * who)
{
	WED_Thing * node = dynamic_cast <WED_Thing *> (who);
	node = node->GetParent();
	WED_DrapedOrthophoto * ortho = SAFE_CAST(WED_DrapedOrthophoto,node);
	if (!ortho)
	{
		if (node) node = node->GetParent();
		ortho = SAFE_CAST(WED_DrapedOrthophoto,node);
	}
	if (ortho)
	{
		Bbox2 new_b;
//		allow continued use of automatic redraping
//		who->GetBounds(gis_UV,new_b);
//		disable automatic redraping
		new_b = Bbox2(0,0,0,0);
		ortho->SetSubTexture(new_b);
	}
}

void		WED_TCEVertexTool::ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_handle)
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	io_handle += delta;
	Bbox2	old_b(0,0,1,1);
	Bbox2	new_b(0,0,1,1);
	new_b.p1 += delta;
	new_b.p2 += delta;
	who->Rescale(gis_UV,old_b,new_b);
	
	// now that we used TCE to modify the UVmaping, update UVbounds
	updateUVbounds(who);
}

void		WED_TCEVertexTool::ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	IGISPoint * pt;
	IGISPoint_Bezier * pt_bt;

	Point2 p;

	switch(who->GetGISClass()) {
	case gis_Point:
	case gis_Point_Heading:
	case gis_Point_HeadingWidthLength:
		if((pt = dynamic_cast<IGISPoint *>(who)) != NULL)
		{
			io_pt += delta;
			pt->GetLocation(gis_UV,p);
			p = io_pt;
			HandleSnap(p,delta);
			pt->SetLocation(gis_UV,p);
		}
		break;
	case gis_Point_Bezier:
		if((pt_bt = dynamic_cast<IGISPoint_Bezier *>(who)) != NULL)
		{
			if(c == 0)
			{
				pt_bt->GetLocation(gis_UV,p);
				io_pt += delta;
				p = io_pt;
				HandleSnap(p,delta);
				pt_bt->SetLocation(gis_UV,p);
			}
			if(c == 1)
			{
				pt_bt->GetControlHandleLo(gis_UV,p);
				io_pt += delta;				
				p = io_pt;
				HandleSnap(p,delta);
				pt_bt->SetControlHandleLo(gis_UV,p);
				io_pt = p;
			}
			if(c == 2)
			{
				pt_bt->GetControlHandleHi(gis_UV,p);
				io_pt += delta;				
				p = io_pt;
				HandleSnap(p,delta);
				pt_bt->SetControlHandleHi(gis_UV,p);
				io_pt = p;
			}
		}
		break;
	}

	// now that we used TCE to modify the UVmaping, update UVbounds
	updateUVbounds(who);
}

void		WED_TCEVertexTool::ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt)
{
	IGISEntity * who = reinterpret_cast<IGISEntity *>(id);
	IGISPointSequence * s;
	switch(who->GetGISClass()) {
	case gis_PointSequence:
	case gis_Line:
	case gis_Line_Width:
	case gis_Ring:
	case gis_Chain:
		s = dynamic_cast<IGISPointSequence*>(who);
		if(s)
		{
			IGISPoint * p1 = s->GetNthPoint(c);
			IGISPoint * p2 = s->GetNthPoint((c+1) % s->GetNumPoints());
			Point2 p;
			p1->GetLocation(gis_UV,p);			p += delta;			p1->SetLocation(gis_UV,p);
			p2->GetLocation(gis_UV,p);			p += delta;			p2->SetLocation(gis_UV,p);
		}
		break;
	}
	// now that we used TCE to modify the UVmaping, update UVbounds
	updateUVbounds(who);
}

void	WED_TCEVertexTool::HandleSnap(Point2& io_pt, const Vector2& delta)
{
	if(mGrid.value != tce_Grid_None)
	{
		double sub_div_h = 1, sub_div_v = 1;
		switch(mGrid.value) {
		case tce_Grid_3x3:	sub_div_h = sub_div_v = 3;	break;
		case tce_Grid_4x4:	sub_div_h = sub_div_v = 4;	break;
		case tce_Grid_5x5:	sub_div_h = sub_div_v = 5;	break;
		case tce_Grid_3x4:	sub_div_h = 3; sub_div_v = 4;	break;
		case tce_Grid_4x3:	sub_div_h = 4; sub_div_v = 3;	break;
		}
		io_pt.x_ = round(io_pt.x_ * sub_div_h) / sub_div_h;
		io_pt.y_ = round(io_pt.y_ * sub_div_v) / sub_div_v;
	}
}

void	WED_TCEVertexTool::SyncRecurse(IGISEntity * who, ISelection * sel) const
{
	IGISPointSequence * s;
	IGISComposite * c;
	IGISPolygon * p;

	switch(who->GetGISClass()) {
	case gis_Point:
	case gis_Point_Bezier:
	case gis_Point_Heading:
	case gis_Point_HeadingWidthLength:
		mCache.push_back(who);
		break;
	case gis_PointSequence:
	case gis_Line:
	case gis_Line_Width:
	case gis_Ring:
	case gis_Chain:
		mCache.push_back(who);
		if((s = dynamic_cast<IGISPointSequence *>(who)) != NULL)
		for(int n = 0; n < s->GetNumPoints(); ++n)
			SyncRecurse(s->GetNthPoint(n), sel);
		break;
	case gis_Polygon:
		mCache.push_back(who);
		if((p = dynamic_cast<IGISPolygon *>(who)) != NULL)
		{
			SyncRecurse(p->GetOuterRing(), sel);
			for(int n = 0; n < p->GetNumHoles(); ++n)
				SyncRecurse(p->GetNthHole(n), sel);
		}
		break;
	case gis_Composite:
		if((c = dynamic_cast<IGISComposite *>(who)) != NULL)
		for(int n = 0; n < c->GetNumEntities(); ++n)
			SyncRecurse(c->GetNthEntity(n),sel);
	}
}

void		WED_TCEVertexTool::SyncCache(void) const
{
	mCache.clear();
	vector<IGISEntity *> who;
	ISelection * sel = WED_GetSelect(GetResolver());
	sel->IterateSelectionOr(Iterate_CollectEntitiesUV,&who);
	for(vector<IGISEntity *>::iterator w = who.begin(); w != who.end(); ++w)
		SyncRecurse(*w, sel);
}
