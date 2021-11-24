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
#include "WED_DrawUtils.h"

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

void		WED_MapBkgnd::DrawVisualization(bool inCurrent, GUI_GraphState* g)
{
	double ll, lb, lr, lt;	// logical boundary
	double vl, vb, vr, vt;	// visible boundry

	GetZoomer()->GetMapLogicalBounds(ll, lb, lr, lt);
	GetZoomer()->GetMapVisibleBounds(vl, vb, vr, vt);

	// Splat the whole world area with the world background color.
	g->SetState(false, false, false, false, false, false, false);
	glColor4fv(WED_Color_RGBA(wed_Map_Bkgnd));
	if(vr - vl > 90.0 && vt - vb > 90.0)     // trace the outline of the whole world for 'compromise' projections
	{
		glBegin(GL_POLYGON);
			for (double lat = lb; lat < lt; lat += 10.0)
				glVertex2(GetZoomer()->LLToPixel(Point2(ll, lat)));
			glVertex2(GetZoomer()->LLToPixel(Point2(ll, lt)));
			for (double lat = lt; lat > lb; lat -= 10.0)
				glVertex2(GetZoomer()->LLToPixel(Point2(lr, lat)));
			glVertex2(GetZoomer()->LLToPixel(Point2(lr, lb)));
		glEnd();
	}
	else
	{
		vl = max(vl, ll);
		vb = max(vb, lb);
		vr = min(vr, lr);
		vt = min(vt, lt);
#if 1
		vl = GetZoomer()->LonToXPixel(vl);
		vr = GetZoomer()->LonToXPixel(vr);
		vb = GetZoomer()->LatToYPixel(vb);
		vt = GetZoomer()->LatToYPixel(vt);
		glBegin(GL_TRIANGLE_STRIP);
		glVertex2d(vl, vb);
		glVertex2d(vl, vt);
		glVertex2d(vr, vb);
		glVertex2d(vr, vt);
		glEnd();
#else                                            // trace the exact outline of some complex projection
		int l = floor(vl);
		int b = floor(vb);
		int r = ceil(vr);
		int t = ceil(vt);

		for (int y = b; y < t; y += 1)
		{
			glBegin(GL_TRIANGLE_STRIP);
/*			for (int x = l; x <= r; x += 1)
			{
				glVertex2(GetZoomer()->LLToPixel(Point2(x, y)));
				glVertex2(GetZoomer()->LLToPixel(Point2(x, y + 1)));
			}
*/
			glVertex2(GetZoomer()->LLToPixel(Point2(l, y)));
			glVertex2(GetZoomer()->LLToPixel(Point2(l, y + 1)));
			glVertex2(GetZoomer()->LLToPixel(Point2((l + r) / 2.0, y)));
			glVertex2(GetZoomer()->LLToPixel(Point2((l + r) / 2.0, y + 1)));
			glVertex2(GetZoomer()->LLToPixel(Point2(r, y)));
			glVertex2(GetZoomer()->LLToPixel(Point2(r, y + 1)));
			glEnd();
		}
#endif
	}
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

	// Gridline time...
	glColor4fv(WED_Color_RGBA(wed_Map_Gridlines));
	glLineWidth(1.0);
	g->SetState(false,false,false, false,true, false,false);

	double lon_span = vr - vl;
	double lat_span = vt - vb;
	int divisions = 1;
	if (lat_span > 12)	divisions = 10;
	if (lat_span > 50)	divisions = 30;
//	if (GetZoomer()->GetPPM() < 8e-4)	divisions = 10;
//	if (GetZoomer()->GetPPM() < 1.5e-4)	divisions = 30;

	int cl = floor(vl / divisions) * divisions;
	int cb = floor(vb / divisions) * divisions;
	int cr = ceil(vr / divisions) * divisions;
	int ct = ceil(vt / divisions) * divisions;

	for(int t = cl; t <= cr; t += divisions)
	{
		glBegin(GL_LINE_STRIP);
		for(int y = floor(vb); y < vt; y += 10)
			glVertex2(GetZoomer()->LLToPixel(Point2(t, y)));
		glVertex2(GetZoomer()->LLToPixel(Point2(t, vt)));
		glEnd();
	}

	int sub_div = min(divisions, 5);
	for(int t = cb; t <= ct; t += divisions)
	{
		glBegin(GL_LINE_STRIP);
		int x = floor(vl);
		glVertex2(GetZoomer()->LLToPixel(Point2(x, t)));
		for(x = sub_div * ceil(vl / sub_div); x < vr; x += sub_div)
			glVertex2(GetZoomer()->LLToPixel(Point2(x, t)));
		glVertex2(GetZoomer()->LLToPixel(Point2(vr, t)));
		glEnd();
	}

	if(lon_span > 1.0 && lon_span < 8.0)
		for(int lon = cl; lon < cr; lon += divisions)
			for(int lat = cb; lat < ct; lat += divisions)
			{
				char tilename[16];
				snprintf(tilename,16,"%+03d%+0d.dsf",lat,lon);
				Point2 pt(lon, lat);
				pt = GetZoomer()->LLToPixel(pt);
				GUI_FontDraw(g,font_UI_Basic,WED_Color_RGBA(wed_StructureLocked),pt.x()+2,pt.y()+5,tilename, align_Left);
			}
}

void		WED_MapBkgnd::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}
