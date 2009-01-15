/* 
 * Copyright (c) 2009, Laminar Research.  All rights reserved.
 *
 */

#include "hl_types.h"
#include "WED_TCEVertexTool.h"
#include "ISelection.h"
#include "IOperation.h"
#include "WED_ToolUtils.h"
#include "AssertUtils.h"
#include "IGIS.h"

WED_TCEVertexTool::WED_TCEVertexTool(
							const char *			tool_name,
							GUI_Pane *				host,
							WED_MapZoomerNew *		zoomer,
							IResolver *				resolver) :
	WED_HandleToolBase(tool_name, host, zoomer, resolver)
{
	SetCanSelect(0);
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
	op->StartOperation("Marquee Drag");
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
			pt->GetUV(*p);
		}
		break;
	case gis_Point_Bezier:
		if((pt_bt = dynamic_cast<IGISPoint_Bezier *>(who)) != NULL)
		switch(n) {
		case 0:
			*active = 1;
			*con_type = handle_Vertex;
			pt_bt->GetUV(*p);
			break;
		case 1:
			*active = pt_bt->GetControlHandleLo(*p);
			if(*active)
			{
				*con_type = handle_Bezier;
				pt_bt->GetUVLo(*p);
			}
			break;
		case 2:
			*active = pt_bt->GetControlHandleHi(*p);
			if(*active)
			{
				*con_type = handle_Bezier;
				pt_bt->GetUVHi(*p);
			}
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
			if(!s->GetSideUV(n/4, seg, bez))
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
			if(n == 0)	*ltype = pt_bt->GetControlHandleHi(p) ? link_BezierCtrl : link_None;
			else		*ltype = pt_bt->GetControlHandleHi(p) ? link_BezierCtrl : link_None;
		}
	} else {
		if(active) *active = false;
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
	return false;
}

void		WED_TCEVertexTool::ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_handle)
{
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
			pt->GetUV(p);
			p += delta;
			pt->SetUV(p);
		}
		break;
	case gis_Point_Bezier:
		if((pt_bt = dynamic_cast<IGISPoint_Bezier *>(who)) != NULL)
		{
			io_pt += delta;
			pt_bt->GetUV(p);
			p += delta;
			pt_bt->SetUV(p);

			pt_bt->GetUVLo(p);
			p += delta;
			pt_bt->SetUVLo(p);

			pt_bt->GetUVHi(p);
			p += delta;
			pt_bt->SetUVHi(p);

		}
		break;
	}
}

void		WED_TCEVertexTool::ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta)
{
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
	sel->IterateSelection(Iterate_CollectEntitiesUV,&who);
	for(vector<IGISEntity *>::iterator w = who.begin(); w != who.end(); ++w)
		SyncRecurse(*w, sel);
}
