#include "WED_Map.h"
#include "WED_MapLayer.h"
#include "WED_MapToolNew.h"
#include "WED_Entity.h"
#include "IGIS.h"
#include "WED_Airport.h"
#include "WED_Thing.h"
#include "ISelection.h"
#include "mathutils.h"
#include "WED_ToolUtils.h"
#include "WED_Messages.h"
#include "IResolver.h"
#include "GUI_Fonts.h"

#define SHOW_FPS 1

WED_Map::WED_Map(IResolver * in_resolver) :
	mResolver(in_resolver)
{
	mTool = NULL;
	mIsToolClick = 0;
	mIsMapDrag = 0;
	
}

WED_Map::~WED_Map()
{
}

void		WED_Map::SetTool(WED_MapToolNew * tool)
{
	if (mTool) mTool->KillOperation(mIsToolClick);
	mTool = tool;
}

void		WED_Map::AddLayer(WED_MapLayer * layer)
{
	mLayers.push_back(layer);
}

void		WED_Map::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);

//	double	old_bounds[4], new_bounds[4], log_bounds[4];
//	GetPixelBounds(old_bounds[0],old_bounds[1],old_bounds[2],old_bounds[3]);
//	GetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	
//	old_bounds[2] = dobmax2(old_bounds[0]+1,old_bounds[2]);
//	old_bounds[3] = dobmax2(old_bounds[1]+1,old_bounds[3]);
	
//	new_bounds[0] = x1;
//	new_bounds[1] = y1;
//	new_bounds[2] = dobmax2(x1+1.0,x2);
//	new_bounds[3] = dobmax2(y1+1.0,y2);
	
//	log_bounds[2] = log_bounds[0] + extrap(0.0,0.0,old_bounds[2]-old_bounds[0],new_bounds[2]-new_bounds[0],log_bounds[2] - log_bounds[0]);
//	log_bounds[3] = log_bounds[1] + extrap(0.0,0.0,old_bounds[3]-old_bounds[1],new_bounds[3]-new_bounds[1],log_bounds[3] - log_bounds[1]);

//	SetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	SetPixelBounds(x1,y1,x2,y2);
}

void		WED_Map::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);

//	double	old_bounds[4], new_bounds[4], log_bounds[4];
//	GetPixelBounds(old_bounds[0],old_bounds[1],old_bounds[2],old_bounds[3]);
//	GetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	
//	old_bounds[2] = dobmax2(old_bounds[0]+1,old_bounds[2]);
//	old_bounds[3] = dobmax2(old_bounds[1]+1,old_bounds[3]);
	
//	new_bounds[0] = inBounds[0];
//	new_bounds[1] = inBounds[1];
//	new_bounds[2] = dobmax2(inBounds[0]+1.0,inBounds[2]);
//	new_bounds[3] = dobmax2(inBounds[1]+1.0,inBounds[3]);
		
//	log_bounds[2] = log_bounds[0] + extrap(0.0,0.0,old_bounds[2]-old_bounds[0],new_bounds[2]-new_bounds[0],log_bounds[2] - log_bounds[0]);
//	log_bounds[3] = log_bounds[1] + extrap(0.0,0.0,old_bounds[3]-old_bounds[1],new_bounds[3]-new_bounds[1],log_bounds[3] - log_bounds[1]);

//	SetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	SetPixelBounds(inBounds[0],inBounds[1],inBounds[2],inBounds[3]);

}

void		WED_Map::Draw(GUI_GraphState * state)
{
	WED_MapLayer * cur = mTool;

	Bbox2 bounds;
	
	int draw_ent_v, draw_ent_s, wants_sel;
	
	GetMapVisibleBounds(bounds.p1.x,bounds.p1.y,bounds.p2.x,bounds.p2.y);
	ISelection * sel = GetSel();
	IGISEntity * base = GetGISBase();

	vector<WED_MapLayer *>::iterator l;
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->GetCaps(draw_ent_v, draw_ent_s, wants_sel);
		if (base && draw_ent_v) DrawVisFor(*l, cur == *l, bounds, base, state, wants_sel ? sel : NULL);
		(*l)->DrawVisualization(cur == *l, state);
	}

	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->GetCaps(draw_ent_v, draw_ent_s, wants_sel);
		if (base && draw_ent_s) DrawStrFor(*l, cur == *l, bounds, base, state, wants_sel ? sel : NULL);
		(*l)->DrawStructure(cur == *l, state);
	}
	
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->DrawSelected(cur == *l, state);
	}
	
	int b[4];
	GetBounds(b);
	float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[3] - 15, mTool ? mTool->GetToolName() : "");
	
	WED_Airport * apt = WED_GetCurrentAirport(mResolver);
	string n = "(No current airport.)";
	if (apt) 
	{
		string an, icao;
		apt->GetName(an);
		apt->GetICAO(icao);
		n = an + string("(") + icao + string(")");
	}
	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[3] - 30, n.c_str());
	
	const char * status = mTool ? mTool->GetStatusText() : NULL;
	if (status)
	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[1] + 15, status);
	
	
	char mouse_loc[50];
	int x, y;
	GetMouseLocNow(&x,&y);
	sprintf(mouse_loc, "%+010.6lf %+011.6lf (ppm: %lf/%lf)", XPixelToLon(x),YPixelToLat(y), GetPPM(), mPixel2DegLat);

	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[1] + 30, mouse_loc);
	
	#if SHOW_FPS
	static clock_t  last_time = 0;
	static float	fps = 0.0f;
	static int		cycle = 0;
		   ++cycle;
		   if (cycle > 100)
		   {
				clock_t now = clock();		   
				fps = ((float) (now - last_time) / ((float) CLOCKS_PER_SEC * 100.0f));		   
				last_time = now;
				cycle = 0;
		   }
		   sprintf(mouse_loc, "%lf FPS", fps);
		   GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[1] + 45, mouse_loc);
		   Refresh();
		   
	#endif
	
}

void		WED_Map::DrawVisFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g, ISelection * sel)
{
	Bbox2	what_bounds;
	what->GetBounds(what_bounds);
	if (!bounds.overlap(what_bounds)) return;
	IGISComposite * c;

	WED_Entity * e = dynamic_cast<WED_Entity *>(what);
	if (e && e->GetHidden()) return;
	
	if (layer->DrawEntityVisualization(current, what, g, sel && sel->IsSelected(what)))
	if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
	{
		int t = c->GetNumEntities();
		for (int n = t-1; n >= 0; --n)
			DrawVisFor(layer, current, bounds, c->GetNthEntity(n), g, sel);
	}
}

void		WED_Map::DrawStrFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g, ISelection * sel)
{
	Bbox2	what_bounds;
	what->GetBounds(what_bounds);
	if (!bounds.overlap(what_bounds)) return;
	IGISComposite * c;

	WED_Entity * e = dynamic_cast<WED_Entity *>(what);
	if (e && e->GetHidden()) return;

	if (layer->DrawEntityStructure(current, what, g, sel && sel->IsSelected(what)))
	if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
	{
		int t = c->GetNumEntities();
		for (int n = t-1; n >= 0; --n)
			DrawStrFor(layer, current, bounds, c->GetNthEntity(n), g, sel);
	}
}

int			WED_Map::MouseDown(int x, int y, int button)
{
	mIsToolClick = 1;
	mIsMapDrag = 0;
	if (mTool && mTool->HandleClickDown(x,y,button, GetModifiersNow())) { Refresh(); return 1; }
	if (button == 0) return 1;
	mIsToolClick = 0;
	mIsMapDrag = 1;
	mX = x;
	mY = y;
	// Refresh - map tools don't have access to a GUI_Pane and can't force refreshes.  But they are likely to do per-gesture drawing.
	Refresh();
	return 1;
}

void		WED_Map::MouseDrag(int x, int y, int button)
{
	if (mIsToolClick && mTool) mTool->HandleClickDrag(x,y,button, GetModifiersNow());
	if (mIsMapDrag)
	{
		this->PanPixels(mX, mY, x, y);
		mX = x; 
		mY = y;
	}
	Refresh();
}

void		WED_Map::MouseUp  (int x, int y, int button)
{
	if (mIsToolClick && mTool)	mTool->HandleClickUp(x,y,button, GetModifiersNow());
	if (mIsMapDrag)				this->PanPixels(mX, mY, x, y);
	mIsToolClick = mIsMapDrag = 0;
	Refresh();
}

int	WED_Map::MouseMove(int x, int y)
{
	Refresh();
	return 1;
}

int			WED_Map::KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (mTool) 	return mTool->HandleKeyPress(inKey, inVK, inFlags);
	else		return 0;
}


int			WED_Map::ScrollWheel(int x, int y, int dist, int axis)
{
	double	zoom = 1.0;
	while (dist > 0)
	{
		zoom *= 1.1;
		--dist;
	}
	while (dist < 0)
	{
		zoom /= 1.1;
		++dist;
	}
	
	if (zoom != 1.0)
		this->ZoomAround(zoom, x, y);
	return 1;
}

void		WED_Map::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	if(inMsg == msg_ArchiveChanged)	Refresh();
}

IGISEntity *	WED_Map::GetGISBase()
{
	return dynamic_cast<IGISEntity *>(WED_GetWorld(mResolver));
}

ISelection *	WED_Map::GetSel()
{
	return WED_GetSelect(mResolver);
}
