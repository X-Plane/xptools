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

#include "WED_MapBkgnd.h"
#include "WED_MapZoomerNew.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "WED_Colors.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

WED_MapBkgnd::WED_MapBkgnd(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * i) : WED_MapLayer(h, zoomer, i)
{
}

WED_MapBkgnd::~WED_MapBkgnd()
{
}

void		WED_MapBkgnd::DrawVisualization(bool inCurrent, GUI_GraphState * g)
{
//	double pl,pr,pb,pt;	// pixel boundary
	double ll,lb,lr,lt;	// logical boundary
	double vl,vb,vr,vt;	// visible boundry

//	GetZoomer()->GetPixelBounds(pl,pb,pr,pt);
	GetZoomer()->GetMapLogicalBounds(ll,lb,lr,lt);
	GetZoomer()->GetMapVisibleBounds(vl,vb,vr,vt);

	vl = max(vl,ll);
	vb = max(vb,lb);
	vr = min(vr,lr);
	vt = min(vt,lt);

	ll = GetZoomer()->LonToXPixel(ll);
	lr = GetZoomer()->LonToXPixel(lr);
	lb = GetZoomer()->LatToYPixel(lb);
	lt = GetZoomer()->LatToYPixel(lt);

	g->SetState(false,false,false, false, false, false,false); 
	glBegin(GL_QUADS);
#if 0
	//This is really obsolete - we have that nice background gradient already - lets show it iff !!

	// First: splat the whole area with the matte color.  This is clipped to
	// pixel bounds cuz we don't need to draw where we can't see.
	glColor4fv(WED_Color_RGBA(wed_Map_Matte));
	glVertex2d(pl,pb);
	glVertex2d(pl,pt);
	glVertex2d(pr,pt);
	glVertex2d(pr,pb);
#endif
	// Next, splat the whole world area with the world background color.  No need
	// to intersect this to the visible area - graphics ard culls good enough.

	glColor4fv(WED_Color_RGBA(wed_Map_Bkgnd));
	glVertex2d(ll,lb);
	glVertex2d(ll,lt);
	glVertex2d(lr,lt);
	glVertex2d(lr,lb);
	glEnd();
	glGetError();  // swallow any error that may be created - eases debugging down the line
}

void		WED_MapBkgnd::DrawStructure(bool inCurrent, GUI_GraphState * g)
{
	double ll,lb,lr,lt;	// logical boundary
	double vl,vb,vr,vt;	// visible boundry


	GetZoomer()->GetMapLogicalBounds(ll,lb,lr,lt);
	GetZoomer()->GetMapVisibleBounds(vl,vb,vr,vt);

	vl = max(vl,ll);
	vb = max(vb,lb);
	vr = min(vr,lr);
	vt = min(vt,lt);

	ll = GetZoomer()->LonToXPixel(ll);
	lr = GetZoomer()->LonToXPixel(lr);
	lb = GetZoomer()->LatToYPixel(lb);
	lt = GetZoomer()->LatToYPixel(lt);

	// Gridline time...
	glColor4fv(WED_Color_RGBA(wed_Map_Gridlines));
	glLineWidth(1.0);
	g->SetState(false,false,false, false,true, false,false);

	double lon_span = vr - vl;
	double lat_span = vt - vb;
	double longest_span = max(lon_span,lat_span);
	int divisions = 1;
	if (longest_span > 20)	divisions = 10;
	if (longest_span > 90)	divisions = 45;

	int cl = floor(vl / divisions) * divisions;
	int cb = floor(vb / divisions) * divisions;
	int cr = ceil(vr / divisions) * divisions;
	int ct = ceil(vt / divisions) * divisions;

	glBegin(GL_LINES);
	for(int t = cl; t <= cr; t += divisions)
	{
		glVertex2d(GetZoomer()->LonToXPixel(t), lb);
		glVertex2d(GetZoomer()->LonToXPixel(t), lt);
	}
	for(int t = cb; t <= ct; t += divisions)
	{
		glVertex2d(ll, GetZoomer()->LatToYPixel(t));
		glVertex2d(lr, GetZoomer()->LatToYPixel(t));
	}
	glEnd();
	
	if(longest_span > 1.0 && longest_span < 5.0)
		for(int lon = cl; lon < cr; lon += divisions)
			for(int lat = cb; lat < ct; lat += divisions)
			{
				char tilename[16];
				snprintf(tilename,16,"%+02d%+03d.dsf",lat,lon);
				GUI_FontDraw(g,font_UI_Small,WED_Color_RGBA(wed_Map_Gridlines),GetZoomer()->LonToXPixel(lon)+1.0,GetZoomer()->LatToYPixel(lat)+3.0,tilename, align_Left);
			}
}

void		WED_MapBkgnd::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}
