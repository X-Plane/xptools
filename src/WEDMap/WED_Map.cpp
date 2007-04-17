#include "WED_Map.h"
#include "WED_MapLayer.h"
#include "WED_MapToolNew.h"
#include "IGIS.h"
#include "ISelection.h"
#include "mathutils.h"
#include "WED_Messages.h"

WED_Map::WED_Map()
{
	mTool = NULL;
	mGISBase = NULL;
	mSel = NULL;
	mIsToolClick = 0;
	mIsMapDrag = 0;
	
}

WED_Map::~WED_Map()
{
}

void		WED_Map::SetTool(WED_MapToolNew * tool)
{
	mTool = tool;
}

void		WED_Map::AddLayer(WED_MapLayer * layer)
{
	mLayers.push_back(layer);
}

void		WED_Map::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);

	double	old_bounds[4], new_bounds[4], log_bounds[4];
	GetPixelBounds(old_bounds[0],old_bounds[1],old_bounds[2],old_bounds[3]);
	GetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	
	old_bounds[2] = dobmax2(old_bounds[0]+1,old_bounds[2]);
	old_bounds[3] = dobmax2(old_bounds[1]+1,old_bounds[3]);
	
	new_bounds[0] = x1;
	new_bounds[1] = y1;
	new_bounds[2] = dobmax2(x1+1.0,x2);
	new_bounds[3] = dobmax2(y1+1.0,y2);
	
	log_bounds[2] = log_bounds[0] + extrap(0.0,0.0,old_bounds[2]-old_bounds[0],new_bounds[2]-new_bounds[0],log_bounds[2] - log_bounds[0]);
	log_bounds[3] = log_bounds[1] + extrap(0.0,0.0,old_bounds[3]-old_bounds[1],new_bounds[3]-new_bounds[1],log_bounds[3] - log_bounds[1]);

	SetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	SetPixelBounds(new_bounds[0],new_bounds[1],new_bounds[2],new_bounds[3]);
}

void		WED_Map::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);

	double	old_bounds[4], new_bounds[4], log_bounds[4];
	GetPixelBounds(old_bounds[0],old_bounds[1],old_bounds[2],old_bounds[3]);
	GetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	
	old_bounds[2] = dobmax2(old_bounds[0]+1,old_bounds[2]);
	old_bounds[3] = dobmax2(old_bounds[1]+1,old_bounds[3]);
	
	new_bounds[0] = inBounds[0];
	new_bounds[1] = inBounds[1];
	new_bounds[2] = dobmax2(inBounds[0]+1.0,inBounds[2]);
	new_bounds[3] = dobmax2(inBounds[1]+1.0,inBounds[3]);
		
	log_bounds[2] = log_bounds[0] + extrap(0.0,0.0,old_bounds[2]-old_bounds[0],new_bounds[2]-new_bounds[0],log_bounds[2] - log_bounds[0]);
	log_bounds[3] = log_bounds[1] + extrap(0.0,0.0,old_bounds[3]-old_bounds[1],new_bounds[3]-new_bounds[1],log_bounds[3] - log_bounds[1]);

	SetMapVisibleBounds(log_bounds[0],log_bounds[1],log_bounds[2],log_bounds[3]);
	SetPixelBounds(new_bounds[0],new_bounds[1],new_bounds[2],new_bounds[3]);

}

void		WED_Map::Draw(GUI_GraphState * state)
{
	WED_MapLayer * cur = mTool;

	Bbox2 bounds;

	vector<WED_MapLayer *>::iterator l;
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->DrawVisualization(cur == *l, state);
		if (mGISBase) DrawVisFor(*l, cur == *l, bounds, mGISBase, state);
	}

	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->DrawStructure(cur == *l, state);
		if (mGISBase) DrawStrFor(*l, cur == *l, bounds, mGISBase, state);
	}
	
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->DrawSelected(cur == *l, state);
		if (mGISBase) DrawSelFor(*l, cur == *l, bounds, mGISBase, state);
	}
}

void		WED_Map::DrawVisFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g)
{
	if (!what->WithinBox(bounds)) return;
	IGISComposite * c;
	layer->DrawEntityVisualization(current, what, g);
	if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
	{
		int t = c->GetNumEntities();
		for (int n = 0; n < t; ++n)
			DrawVisFor(layer, current, bounds, c->GetNthEntity(n), g);
	}
}

void		WED_Map::DrawStrFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g)
{
	if (!what->WithinBox(bounds))	return;
	IGISComposite * c;
	layer->DrawEntityStructure(current, what, g);
	if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
	{
		int t = c->GetNumEntities();
		for (int n = 0; n < t; ++n)
			DrawStrFor(layer, current, bounds, c->GetNthEntity(n), g);
	}
}

void		WED_Map::DrawSelFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g)
{
	if (!mSel)						return;
	if (!what->WithinBox(bounds))	return;
	IGISComposite * c;
	if (mSel->IsSelected(what))
		layer->DrawEntitySelected(current, what, g);
	if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
	{
		int t = c->GetNumEntities();
		for (int n = 0; n < t; ++n)
			DrawSelFor(layer, current, bounds, c->GetNthEntity(n), g);
	}
}


int			WED_Map::MouseDown(int x, int y, int button)
{
	mIsToolClick = 1;
	mIsMapDrag = 0;
	if (mTool && mTool->HandleClickDown(x,y,button)) return 1;
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
	if (mIsToolClick && mTool) mTool->HandleClickDrag(x,y,button);
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
	if (mIsToolClick && mTool)	mTool->HandleClickUp(x,y,button);
	if (mIsMapDrag)				this->PanPixels(mX, mY, x, y);
	mIsToolClick = mIsMapDrag = 0;
	Refresh();
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
