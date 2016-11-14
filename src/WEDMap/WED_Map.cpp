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

#include "WED_Map.h"
#include "WED_MapLayer.h"
#include "WED_MapToolNew.h"
#include "WED_Entity.h"
#include "GUI_GraphState.h"
#include "XESConstants.h"
#include "IGIS.h"
#include "GISUtils.h"
#include "WED_Airport.h"
#include "WED_Thing.h"
#include "ISelection.h"
#include "MathUtils.h"
#include "WED_ToolUtils.h"
#include "WED_Messages.h"
#include "IResolver.h"
#include "GUI_Fonts.h"
#include <time.h>

// This is the size that a GIS composite must be to cause us to skip iterating down into it, in pixels.
// The idea is that when we are zoomed way out and we have a bunch of global airports, we don't want to 
// iterate into each airport just to realize that all details are too small to draw.  
//
// So we measure the container to make a judgment.  
//
// Because we have to add the object hang-over slop to our cull decision, the size of cull in screen space 
// is surprisingly big.  In other words, we might pick 20 pixels as the cutoff because we have 1 pixel of
// airport and 19 pixels of slop.
#define TOO_SMALL_TO_GO_IN 20.0

int	gDMS = 0;

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#define SHOW_FPS 0

WED_Map::WED_Map(IResolver * in_resolver) :
	mResolver(in_resolver)
{
	mTool = NULL;
	mClickLayer = NULL;
	mIsDownCount = 0;
	mIsDownExtraCount = 0;
}

WED_Map::~WED_Map()
{
}

void		WED_Map::SetTool(WED_MapToolNew * tool)
{
	if (mTool) mTool->KillOperation(mClickLayer == mTool);
	mTool = tool;
}

void		WED_Map::AddLayer(WED_MapLayer * layer)
{
	layer->SetFilter(&mHideFilter, &mLockFilter);
	mLayers.push_back(layer);
}

void		WED_Map::SetFilter(const string& filterName, const vector<const char *>& hide_filter, const vector<const char *>& lock_filter)
{
	mFilterName = filterName;
	mHideFilter = hide_filter;
	mLockFilter = lock_filter;

	Refresh();
}


void		WED_Map::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);
	SetPixelBounds(x1,y1,x2,y2);
}

void		WED_Map::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	SetPixelBounds(inBounds[0],inBounds[1],inBounds[2],inBounds[3]);

}

void		WED_Map::Draw(GUI_GraphState * state)
{
	WED_MapLayer * cur = mTool;

	Bbox2 bounds;

	bool draw_ent_v, draw_ent_s, wants_sel, wants_clicks;

	GetMapVisibleBounds(bounds.p1.x_,bounds.p1.y_,bounds.p2.x_,bounds.p2.y_);
	ISelection * sel = GetSel();
	IGISEntity * base = GetGISBase();

	vector<WED_MapLayer *>::iterator l;
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	if((*l)->IsVisible())
	{
		(*l)->GetCaps(draw_ent_v, draw_ent_s, wants_sel, wants_clicks);
		if (base && draw_ent_v) DrawVisFor(*l, cur == *l, bounds, base, state, wants_sel ? sel : NULL, 0);
		(*l)->DrawVisualization(cur == *l, state);
	}

	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	if((*l)->IsVisible())
	{
		(*l)->GetCaps(draw_ent_v, draw_ent_s, wants_sel, wants_clicks);
		if (base && draw_ent_s) DrawStrFor(*l, cur == *l, bounds, base, state, wants_sel ? sel : NULL, 0);
		(*l)->DrawStructure(cur == *l, state);
	}

	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	if((*l)->IsVisible())
	{
		(*l)->DrawSelected(cur == *l, state);
	}

	int x, y;
	GetMouseLocNow(&x,&y);


	if (mIsDownExtraCount)
	{
		state->SetState(0,0,0,1,1,0,0);
		glColor4f(1,1,1,0.4);
		glBegin(GL_LINES);
		glVertex2i(mX_Orig,mY_Orig);
		glVertex2i(x,y);
		glEnd();
	}

	int b[4];
	GetBounds(b);
	float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[3] - 15, mTool ? mTool->GetToolName() : "");

	{
		WED_Airport * apt = WED_GetCurrentAirport(mResolver);
		string n = "(No current airport.)";
		if (apt)
		{
			string an, icao;
			apt->GetName(an);
			apt->GetICAO(icao);
			n = an + string("(") + icao + string(")");
		}
		GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[3] - 45, n.c_str());
	}
	
	if(!mFilterName.empty())
		GUI_FontDraw(state, font_UI_Basic, white, b[0]+5, b[3] - 30, mFilterName.c_str());

	const char * status = mTool ? mTool->GetStatusText() : NULL;
	if (status)
	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[1] + 15, status);

	char mouse_loc[350];
	char * p = mouse_loc;

	bool	has_a1 = false, has_a2 = false, has_d = false, has_h = false;
	double	head,dist;
	Point2	anchor1, anchor2;

	if (mTool)
	{
		has_a1 = mTool->GetAnchor1(anchor1);
		has_a2 = mTool->GetAnchor2(anchor2);
		has_d = mTool->GetDistance(dist);
		has_h = mTool->GetHeading(head);
	}

	if (!has_a1 && !has_a2 && !has_d && !has_h)
	{
		Point2 o,n;
		o.x_ = XPixelToLon(mX_Orig);
		o.y_ = YPixelToLat(mY_Orig);
		n.x_ = XPixelToLon(x);
		n.y_ = YPixelToLat(y);

		if (mIsDownExtraCount)
		{
			has_d = 1;	dist = LonLatDistMeters(o.x(),o.y(),n.x(),n.y());
			has_h = 1;	head = VectorDegs2NorthHeading(o, o, Vector2(o,n));
			has_a1 = 1; anchor1 = o;
			has_a2 = 1; anchor2 = n;
		}
		else
		{
			has_a1 = 1; anchor1 = n;
		}
	}

	#define GET_NS(x)	((x) > 0.0 ? 'N' : 'S')
	#define GET_EW(x)	((x) > 0.0 ? 'E' : 'W')
	#define GET_DEGS(x) ((int) floor(fabs(x)))
	#define GET_MINS(x) ((int) (  (fabs(x) - floor(fabs(x))  ) * 60.0) )
	#define GET_SECS(x) (  (fabs(x * 60.0) - floor(fabs(x * 60.0))  ) * 60.0)

	if (has_a1 && !gDMS)	p += sprintf(p, "%+010.6lf %+011.6lf", anchor1.y(),anchor1.x());
	if (has_a1 &&  gDMS)	p += sprintf(p, "%c%02d %02d %04.2lf %c%03d %02d %04.2lf",
												GET_NS(anchor1.y()),GET_DEGS(anchor1.y()),GET_MINS(anchor1.y()),GET_SECS(anchor1.y()),
												GET_EW(anchor1.x()),GET_DEGS(anchor1.x()),GET_MINS(anchor1.x()),GET_SECS(anchor1.x()));
	if (has_a1 && has_a2)	p += sprintf(p, " -> ");
	if (has_a2 && !gDMS)	p += sprintf(p, "%+010.6lf %+011.6lf", anchor2.y(),anchor2.x());
	if (has_a2 &&  gDMS)	p += sprintf(p, "%c%02d %02d %04.2lf %c%03d %02d %04.2lf",
												GET_NS(anchor1.y()),GET_DEGS(anchor1.y()),GET_MINS(anchor1.y()),GET_SECS(anchor1.y()),
												GET_EW(anchor1.x()),GET_DEGS(anchor1.x()),GET_MINS(anchor1.x()),GET_SECS(anchor1.x()));


	#undef GET_DEGS
	#undef GET_MINS
	#undef GET_SECS

	if (has_d)				p += sprintf(p," %.1lf %s",dist * (gIsFeet ? MTR_TO_FT : 1.0), gIsFeet? "feet" : "meters");
	if (has_h)				p += sprintf(p," heading: %.1lf", head);

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

void		WED_Map::DrawVisFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g, ISelection * sel, int depth)
{
	if(!what->Cull(bounds))	return;
	IGISComposite * c;

	if(!layer->IsVisibleNow(what))	return;

	if (layer->DrawEntityVisualization(current, what, g, sel && sel->IsSelected(what)))
	if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
	{
		Bbox2	on_screen;
		what->GetBounds(gis_Geo, on_screen);
		on_screen.expand(GLOBAL_WED_ART_ASSET_FUDGE_FACTOR);
		Point2 p1 = this->LLToPixel(on_screen.p1);
		Point2 p2 = this->LLToPixel(on_screen.p2);
		Vector2 span(p1,p2);
		if(max(span.dx, span.dy) > TOO_SMALL_TO_GO_IN || (p1 == p2) || depth == 0)		// Why p1 == p2?  If the composite contains ONLY ONE POINT it is zero-size.  We'd LOD out.  But if it contains one thing
		{																				// then we might as well ALWAYS draw it - it's relatively cheap!
			int t = c->GetNumEntities();												// Depth == 0 means we draw ALL top level objects -- good for airports.
			for (int n = t-1; n >= 0; --n)
				DrawVisFor(layer, current, bounds, c->GetNthEntity(n), g, sel, depth+1);
		}
	}
}

void		WED_Map::DrawStrFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g, ISelection * sel, int depth)
{
	if(!what->Cull(bounds))	return;
	IGISComposite * c;

	if(!layer->IsVisibleNow(what))	return;

	if (layer->DrawEntityStructure(current, what, g, sel && sel->IsSelected(what)))
	if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
	{
		Bbox2	on_screen;
		what->GetBounds(gis_Geo, on_screen);
		on_screen.expand(GLOBAL_WED_ART_ASSET_FUDGE_FACTOR);
		
		Point2 p1 = this->LLToPixel(on_screen.p1);
		Point2 p2 = this->LLToPixel(on_screen.p2);
		Vector2 span(p1,p2);
		if(max(span.dx, span.dy) > TOO_SMALL_TO_GO_IN || (p1 == p2) || depth == 0)
		{
			int t = c->GetNumEntities();
			for (int n = t-1; n >= 0; --n)
				DrawStrFor(layer, current, bounds, c->GetNthEntity(n), g, sel, depth+1);
		}
	}
}

int			WED_Map::MouseDown(int x, int y, int button)
{
	if (mIsDownCount++==0)
	{
		mX_Orig = x;
		mY_Orig = y;
	}
	if (button > 1) ++mIsDownExtraCount;

	if(button==0)
	{
		mClickLayer = NULL;	
		for(vector<WED_MapLayer *>::iterator l = mLayers.begin(); l != mLayers.end(); ++l)
		{
			bool draw_ent_v, draw_ent_s, wants_sel, wants_clicks;
			(*l)->GetCaps(draw_ent_v, draw_ent_s, wants_sel, wants_clicks);
			if(wants_clicks)
			{
				if((*l)->HandleClickDown(x,y,button, GetModifiersNow())) { 
					mClickLayer = *l;
					break;
				}
			}
		}
	}
	if(button == 0 && mClickLayer == NULL && mTool && mTool->HandleClickDown(x,y,button, GetModifiersNow())) { mClickLayer=mTool; }	

	if(button==1)
	{
		mX = x;
		mY = y;
	}
	// Refresh - map tools don't have access to a GUI_Pane and can't force refreshes.  But they are likely to do per-gesture drawing.
	Refresh();
	return 1;
}

void		WED_Map::MouseDrag(int x, int y, int button)
{
	if (button==0 && mClickLayer) mClickLayer->HandleClickDrag(x,y,button, GetModifiersNow());
	if (button==1)
	{
		this->PanPixels(mX, mY, x, y);
		mX = x;
		mY = y;
	}
	Refresh();
}

void		WED_Map::MouseUp  (int x, int y, int button)
{
	--mIsDownCount;
	if (button > 1) --mIsDownExtraCount;

	if (button==0&&mClickLayer)	mClickLayer->HandleClickUp(x,y,button, GetModifiersNow());
	if (button==1)				this->PanPixels(mX, mY, x, y);
	if(button==0)mClickLayer = NULL;
	Refresh();
}

int	WED_Map::MouseMove(int x, int y)
{
	Refresh();
	return 1;
}

int			WED_Map::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (mTool) 	return mTool->HandleToolKeyPress(inKey, inVK, inFlags);
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
	Refresh();
	return 1;
}

void		WED_Map::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
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
