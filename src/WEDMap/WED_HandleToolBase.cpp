#include "WED_HandleToolBase.h"
#include "WED_MapZoomerNew.h"
#include "WED_ToolUtils.h"
#include "WED_Entity.h"
#include "WED_Colors.h"
#include "XESConstants.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "IControlHandles.h"
#include "IResolver.h"
#include "WED_Thing.h"
#include "IGIS.h"
#include "ISelection.h"
#include "IOperation.h"
#include "WED_UIDefs.h"
#include "MathUtils.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

#define LINE_DIST 3
#define	HANDLE_RAD 4

// This util routine forms the line segment or bezier for a given "link" in a handles, converting from lat/lon to pixels.
// returns true for bezier, false for segment.
static bool	ControlLinkToCurve(
				IControlHandles *		h,
				int						ei,
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
		s.p1.x = z->LonToXPixel(s.p1.x);
		s.p2.x = z->LonToXPixel(s.p2.x);
		s.p1.y = z->LatToYPixel(s.p1.y);
		s.p2.y = z->LatToYPixel(s.p2.y);
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
		b.p1.x = z->LonToXPixel(b.p1.x);
		b.c1.x = z->LonToXPixel(b.c1.x);
		b.p2.x = z->LonToXPixel(b.p2.x);
		b.c2.x = z->LonToXPixel(b.c2.x);
		b.p1.y = z->LatToYPixel(b.p1.y);
		b.c1.y = z->LatToYPixel(b.c1.y);
		b.p2.y = z->LatToYPixel(b.p2.y);
		b.c2.y = z->LatToYPixel(b.c2.y);
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
	mCanSelect(1)
{

}
										
WED_HandleToolBase::~WED_HandleToolBase()
{
}

void				WED_HandleToolBase::SetCanSelect(int can_select)
{ 
	mCanSelect = can_select;
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
	int eid;
	int ei, n;

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
			int active;
			Point2	cloc;
			HandleType_t ht;
			float radius = HANDLE_RAD;
			mHandles->GetNthControlHandle(eid, n, &active, &ht, &cloc, NULL, &radius);
			if (!active) continue;
			
			radius *= radius;
			cloc.x = GetZoomer()->LonToXPixel(cloc.x);
			cloc.y = GetZoomer()->LatToYPixel(cloc.y);
			if ((this_dist=click_pt.squared_distance(cloc)) < best_dist)
			if (this_dist < radius)
			{
				mHandleEntity = eid;
				mHandleIndex = n;
				best_dist = this_dist;
				if (mDragType == drag_None)
					mHandles->BeginEdit();			
				mDragType = drag_Handles;	
			}
		}
	}

	//-------------------------------- CONTROL LINK TAG-UP -------------------------------------------------------

	best_dist = 9.9e9;
	
	if (mDragType == drag_None && ei_count > 0)
	for (ei = 0; ei < ei_count; ++ei)
	{
		eid = mHandles->GetNthEntityID(ei);	
		l_count = mHandles->GetLinks(eid);
		for (n = 0; n < l_count; ++n)
		{
			int active;
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
					break;
				}
			}			
		}
	}
	//----------------------------------- ENTITY DRAG ------------------------------------------------------------
	
	click_pt = GetZoomer()->PixelToLL(click_pt);
	if (mDragType == drag_None && ei_count > 0)
	for (ei = 0; ei < ei_count; ++ei)
	if (mHandles->PointOnStructure(ei, click_pt))
	{
		mDragType = drag_Move;
		mHandleEntity = ei;
		mHandles->BeginEdit();								
		break;
	}

	//----------------------------------- CREATION DRAG ------------------------------------------------------------

	if (mDragType == drag_None && this->CreationDown(click_pt))
	{
		mDragType = drag_Create;
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
		if (sel && ent_base)
		{
			set<IGISEntity *>	sel_set;
			Bbox2	bounds(
							GetZoomer()->XPixelToLon(mDragX),
							GetZoomer()->YPixelToLat(mDragY),
							GetZoomer()->XPixelToLon(inX),
							GetZoomer()->YPixelToLat(inY));
		
			ProcessSelectionRecursive(ent_base, bounds, sel_set);
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

#if !DEV
doc and clean this
#endif

int		WED_HandleToolBase::ProcessSelectionRecursive(
									IGISEntity *		entity,
									const Bbox2&		bounds,
									set<IGISEntity *>&	result)
{
	int pt_sel = bounds.is_point();
	Point2	psel = bounds.p1;

	double	frame_dist = fabs(GetZoomer()->YPixelToLat(0)-GetZoomer()->YPixelToLat(3));
	double	icon_dist_v = fabs(GetZoomer()->YPixelToLat(0)-GetZoomer()->YPixelToLat(GetFurnitureIconRadius()));
	double	icon_dist_h = fabs(GetZoomer()->XPixelToLon(0)-GetZoomer()->XPixelToLon(GetFurnitureIconRadius()));
	double	max_slop_h = max(icon_dist_h,frame_dist);
	double	max_slop_v = max(icon_dist_v,frame_dist);
	if(WED_IsIconic(entity))
		frame_dist = max(icon_dist_h,icon_dist_v);
				
	Bbox2		ent_bounds;
	entity->GetBounds(ent_bounds);
//	if (pt_sel)
	{
		ent_bounds.p1 -= Vector2(max_slop_h,max_slop_v);
		ent_bounds.p2 += Vector2(max_slop_h,max_slop_v);
	}
		
	if (pt_sel) { if (!ent_bounds.contains(psel))				return 0;	}
	else		{ if (!ent_bounds.overlap(bounds))				return 0;	}

	WED_Entity * thang = dynamic_cast<WED_Entity *>(entity);
	if (thang) {
		if (thang->GetLocked()) return 0;
		if (thang->GetHidden()) return 0;
	}
	
	EntityHandling_t choice = TraverseEntity(entity);
	IGISComposite * com = SAFE_CAST(IGISComposite, entity);
	IGISPointSequence * seq = SAFE_CAST(IGISPointSequence, entity);
	IGISPolygon * poly = SAFE_CAST(IGISPolygon, entity);
	switch(choice) {	
	case ent_Atomic:
		if (pt_sel)	{ if (entity->PtWithin(psel) || entity->PtOnFrame(psel, frame_dist))	result.insert(entity); return 1;	}
		else		{ if (entity->WithinBox(bounds))										result.insert(entity);	}
		break;
	case ent_Container:
		if (com)
		{
			int count = com->GetNumEntities();
			for (int n = 0; n < count; ++n)
				if (ProcessSelectionRecursive(com->GetNthEntity(n),bounds,result) && pt_sel) return 1;
		}
		else if (seq) 
		{
			int count = seq->GetNumPoints();
			for (int n = 0; n < count; ++n)
				if (ProcessSelectionRecursive(seq->GetNthPoint(n),bounds,result) && pt_sel) return 1;
		}
		else if(poly) 
		{
			int count = poly->GetNumHoles();
			if (ProcessSelectionRecursive(poly->GetOuterRing(),bounds,result) && pt_sel) return 1;
			for (int n = 0; n < count; ++n)
				if (ProcessSelectionRecursive(poly->GetNthHole(n),bounds,result) && pt_sel) return 1;
		}
		break;
	case ent_AtomicOrContainer:
		if (!pt_sel && entity->WithinBox(bounds)) result.insert(entity);	
		else if (com)
		{
			int count = com->GetNumEntities();
			for (int n = 0; n < count; ++n)
				if (ProcessSelectionRecursive(com->GetNthEntity(n),bounds,result) && pt_sel) return 1;
		}
		else if (seq) 
		{
			int count = seq->GetNumPoints();
			for (int n = 0; n < count; ++n)
				if (ProcessSelectionRecursive(seq->GetNthPoint(n),bounds,result) && pt_sel) return 1;
		}
		else if (poly) 
		{
			int count = poly->GetNumHoles();
			if (ProcessSelectionRecursive(poly->GetOuterRing(),bounds,result) && pt_sel) return 1;
			for (int n = 0; n < count; ++n)
				if (ProcessSelectionRecursive(poly->GetNthHole(n),bounds,result) && pt_sel) return 1;
		}	
		if (pt_sel && entity->PtWithin(psel))				{ result.insert(entity); return 1; }
		if (pt_sel && entity->PtOnFrame(psel, frame_dist))  { result.insert(entity); return 1; }
			
		break;
	}
	return 0;
}


void		WED_HandleToolBase::HandleClickDrag			(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
	if (inButton > 0) return;

	switch(mDragType) {
	case drag_Handles:
	case drag_Links:
	case drag_Move:
		{
			Point2	op(GetZoomer()->XPixelToLon(mDragX),GetZoomer()->YPixelToLat(mDragY));
			Point2	np(GetZoomer()->XPixelToLon(   inX),GetZoomer()->YPixelToLat(   inY));
			Vector2 delta(op,np);
			mDragX = inX; mDragY = inY;
			switch(mDragType) {
			case drag_Handles:				mHandles->ControlsHandlesBy(mHandleEntity,	mHandleIndex,	delta);	break;
			case drag_Links:				mHandles->ControlsLinksBy(mHandleEntity,	mHandleIndex,	delta);	break;
			case drag_Move:					mHandles->ControlsMoveBy(mHandleEntity,						delta);	break;
			}
		}
		break;
	case drag_Create:
		this->CreationDrag(
					GetZoomer()->PixelToLL(Point2(mDragX, mDragY)),
					GetZoomer()->PixelToLL(Point2(inX, inY)));
		break;
	case drag_Sel:
		{	
			mSelX = inX;
			mSelY = inY;
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
			
				ProcessSelectionRecursive(ent_base, bounds, sel_set);

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
	if (mDragType == drag_Sel)
	{
		IOperation * op = SAFE_CAST(IOperation, WED_GetSelect(GetResolver()));	
		if(op) op->CommitOperation();
		mSelSave.clear();
	} else if ( mDragType == drag_Links || 
				mDragType == drag_Handles ||
				mDragType == drag_Move)
	{
		mHandles->EndEdit();
	} else if ( mDragType == drag_Create )
		this->CreationUp(
					GetZoomer()->PixelToLL(Point2(mDragX, mDragY)),
					GetZoomer()->PixelToLL(Point2(inX, inY)));

	GUI_Commander::UnregisterNotifiable(this);
	mDragType = drag_None;
}

int			WED_HandleToolBase::HandleKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
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
				mDragType == drag_Move)
	{
		mHandles->EndEdit();
	} 
	GUI_Commander::UnregisterNotifiable(this);
	mDragType = drag_None;
}

void		WED_HandleToolBase::GetCaps(int& draw_ent_v, int& draw_ent_s, int& cares_about_sel)
{
	draw_ent_v = draw_ent_s = cares_about_sel = 0;
}

void		WED_HandleToolBase::DrawStructure			(int inCurrent, GUI_GraphState * g)
{
	if (!inCurrent) return;
	if (mHandles != NULL)
	{
		int ei_count = mHandles->CountEntities();
		for (int ei = 0; ei < ei_count; ++ei)
		{
			int eid = mHandles->GetNthEntityID(ei);
			
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
						int pixels_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
											sqrt(Vector2(b.c1,b.c2).squared_length()) +
											sqrt(Vector2(b.c2,b.p2).squared_length());
						int point_count = intlim(pixels_approx / BEZ_PIX_PER_SEG, BEZ_MIN_SEGS, BEZ_MAX_SEGS);

						for (int n = 0; n < point_count; ++n)
						{
							float t1 = (float) n / (float) point_count;
							float t2 = (float) (n+1) / (float) point_count;
							Point2	p1 = b.midpoint(t1);
							Point2	p2 = b.midpoint(t2);
							glVertex2d(p1.x,p1.y);
							glVertex2d(p2.x,p2.y);
						}
					}
					else
					{
						glVertex2d(s.p1.x,s.p1.y);
						glVertex2d(s.p2.x,s.p2.y);									
					}
				}
			}
			glEnd();
			
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

				
				if (ht == handle_ArrowHead || ht == handle_Arrow || ht == handle_Bezier) 
				{
					Point2 bscrp = GetZoomer()->LLToPixel(cpt - dir);
					if (ht == handle_Arrow)
					{
						g->SetState(0,0,0,   0, 0, 0, 0);
						glBegin(GL_LINES);
						glVertex2d(bscrp.x, bscrp.y);
						glVertex2d(scrpt.x, scrpt.y);						
						glEnd();
					}
					orient = Vector2(bscrp,scrpt);
				}
						
				switch(ht) {
				case handle_Square:			GUI_PlotIcon(g,"handle_square.png", scrpt.x,scrpt.y,0,1.0);		break;
				case handle_Vertex:			GUI_PlotIcon(g,"handle_vertexround.png", scrpt.x,scrpt.y,0,1.0);break;
				case handle_VertexSharp:	GUI_PlotIcon(g,"handle_vertexsharp.png", scrpt.x,scrpt.y,0,1.0);break;
				case handle_Bezier:			GUI_PlotIcon(g,"handle_control.png", scrpt.x,scrpt.y,atan2(orient.dx,orient.dy) * RAD_TO_DEG,1.0);	break;
				case handle_ClosePt:		GUI_PlotIcon(g,"handle_closeloop.png", scrpt.x,scrpt.y,0,1.0);	break;
				case handle_Cross:			GUI_PlotIcon(g,"handle_cross.png", scrpt.x,scrpt.y,0,1.0);		break;
				case handle_ArrowHead:		
				case handle_Arrow:			GUI_PlotIcon(g,"handle_arrowhead.png", scrpt.x,scrpt.y,atan2(orient.dx,orient.dy) * RAD_TO_DEG,1.0);	break;
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
				mDragType == drag_Move)
	{
		mHandles->EndEdit();
	} 
	mDragType = drag_None;
	GUI_Commander::UnregisterNotifiable(this);
}

