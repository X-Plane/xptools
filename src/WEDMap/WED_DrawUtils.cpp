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

#include "WED_DrawUtils.h"
#include "IGIS.h"
#include "WED_MapZoomerNew.h"
#include "WED_UIDefs.h"
#include "MathUtils.h"
#include "WED_EnumSystem.h"
#if APL
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#if !NO_CGAL_BEZIER
#include "Bezier.h"

bool IsBezierPolyScrewed(IGISPolygon * poly, vector<Point2> * where)
{
	vector<Bezier2>	der_sides;
	IGISPointSequence * outer = poly->GetOuterRing();	
	for(int n = 0; n < outer->GetNumSides(); ++n)
	{	
		Bezier2 b;
		Segment2 s;
		outer->GetSide(gis_Geo, n,s,b);
		der_sides.push_back(b);
	}
	for(int h = 0; h < poly->GetNumHoles(); ++h)
	{
		IGISPointSequence * hh = poly->GetNthHole(h);
		for(int n = 0; n < hh->GetNumSides(); ++n)
		{
			Bezier2 b;
			Segment2 s;
			hh->GetSide(gis_Geo, n,s,b);
			der_sides.push_back(b);
		}
	}
	if(where == NULL) 
		return do_beziers_cross(der_sides);
	vector<Point2>	p;
	if(where == NULL) where = &p;
	find_crossing_beziers(der_sides,*where);
	return !where->empty();
}


bool IsBezierSequenceScrewed(IGISPointSequence * ps, vector<Point2> * where)
{
	if(!ps->IsClosed()) return false;
	
	vector<Bezier2>	der_sides;
	for(int n = 0; n < ps->GetNumSides(); ++n)
	{	
		Bezier2 b;
		Segment2 s;
		ps->GetSide(gis_Geo, n,s,b);
		der_sides.push_back(b);
	}
	if(where == NULL) 
		return do_beziers_cross(der_sides);
	vector<Point2>	p;
	if(where == NULL) where = &p;
	find_crossing_beziers(der_sides,*where);
	return !where->empty();
}
#endif

void PointSequenceToVector(
			IGISPointSequence *		ps,
			WED_MapZoomerNew *		z,
			vector<Point2>&			pts,
			bool					get_uv,
			vector<int>&			contours,
			int						is_hole)
{
	int n = ps->GetNumSides();
	for (int i = 0; i < n; ++i)
	{
		Segment2	s, suv;
		Bezier2		b, buv;
		if(get_uv) ps->GetSide(gis_UV,i,suv,buv);
		if (ps->GetSide(gis_Geo,i,s,b))
		{
			b.p1 = z->LLToPixel(b.p1);
			b.p2 = z->LLToPixel(b.p2);
			b.c1 = z->LLToPixel(b.c1);
			b.c2 = z->LLToPixel(b.c2);

			int pixels_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
								sqrt(Vector2(b.c1,b.c2).squared_length()) +
								sqrt(Vector2(b.c2,b.p2).squared_length());
			int point_count = intlim(pixels_approx / BEZ_PIX_PER_SEG, BEZ_MIN_SEGS, BEZ_MAX_SEGS);
					 pts. reserve(pts. capacity() + point_count * (get_uv ? 2 : 1));
			contours.reserve(contours.capacity() + point_count);
			for (int k = 0; k < point_count; ++k)
			{
							pts.push_back(b.midpoint((float) k / (float) point_count));
				if(get_uv)	pts.push_back(buv.midpoint((float) k / (float) point_count));
				contours.push_back((k == 0 && i == 0) ? is_hole : 0);
			}

			if (i == n-1 && !ps->IsClosed())
			{
							pts.push_back(b.p2);
				if(get_uv)	pts.push_back(buv.p2);
				contours.push_back(0);
			}
		}
		else
		{
							pts.push_back(z->LLToPixel(s.p1));
			if(get_uv)		pts.push_back(suv.p1);
			contours.push_back(i == 0 ? is_hole : 0);
			if (i == n-1 && !ps->IsClosed())
			{
							pts.push_back(s.p2);
				if(get_uv)	pts.push_back(suv.p2);
				contours.push_back(0);
			}
		}
	}
}

#if !IBM
#define CALLBACK
#endif

static void CALLBACK TessBegin(GLenum mode)		{ glBegin(mode);				}
static void CALLBACK TessEnd(void)				{ glEnd();						}
static void CALLBACK TessVertex(const Point2 * p){																  glVertex2d(p->x(),p->y());	}
static void CALLBACK TessVertexUV(const Point2 * p){ const Point2 * uv = p; ++uv; glTexCoord2f(uv->x(), uv->y()); glVertex2d(p->x(),p->y());	}

void glPolygon2(const Point2 * pts, bool has_uv, const int * contours, int n)
{
	GLUtesselator * tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN,	(void (CALLBACK *)(void))TessBegin);
	gluTessCallback(tess, GLU_TESS_END,		(void (CALLBACK *)(void))TessEnd);
	if(has_uv)
	gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))TessVertexUV);
	else
	gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))TessVertex);

	gluBeginPolygon(tess);

	while(n--)
	{
		if (contours && *contours++)	gluNextContour(tess, GLU_INTERIOR);

		double	xyz[3] = { pts->x(), pts->y(), 0 };
		gluTessVertex(tess, xyz, (void*) pts++);
		if(has_uv)
			++pts;
	}

	gluEndPolygon (tess);
	gluDeleteTess(tess);
}


void DrawLineAttrs(GUI_GraphState * state, const Point2 * pts, int count, const set<int>& attrs, WED_Color c)
{
	if (attrs.empty())
	{
		glColor4fv(WED_Color_RGBA(c));
		glShape2v(GL_LINE_STRIP, pts, count);
		return;
	}
	else for(set<int>::const_iterator a = attrs.begin(); a != attrs.end(); ++a)
	switch(*a) {
	// ------------ STANDARD TAXIWAY LINES ------------
	case line_SolidYellow:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BrokenYellow:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_DoubleSolidYellow:

		glColor4f(1,1,0,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_RunwayHold:

		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		break;
	case line_OtherHold:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,1.5);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-1.5);
		break;
	case line_ILSHold:

		glColor4f(1,1,0,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x1111);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_ILSCriticalCenter:

		glColor4f(1,1,0,1);
		glLineWidth(5);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_WideBrokenSingle:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_WideBrokenDouble:

		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;

	// ------------ ROADWAY TAXIWAY LINES ------------
	case line_SolidWhite:

		glColor4f(1,1,1,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_Chequered:

		glColor4f(1,1,1,1);
		glLineWidth(6);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(2);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,1,1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0xCCCC);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BrokenWhite:

		glColor4f(1,1,1,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;

	// ------------ BLACK-BACKED TAXIWAY LINES ------------
	case line_BSolidYellow:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BBrokenYellow:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BDoubleSolidYellow:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BRunwayHold:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(12);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		break;
	case line_BOtherHold:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(6);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,1.5);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-1.5);
		break;
	case line_BILSHold:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(7);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x1111);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BILSCriticalCenter:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(7);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BWideBrokenSingle:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BWideBrokenDouble:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;

	// ------------ LIGHTS ------------
	case line_TaxiCenter:

		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_TaxiEdge:

		glColor4f(0,0,1,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-5);
		break;
	case line_HoldLights:

		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7070);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_HoldLightsPulse:

		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.1,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_HoldShortCenter:

		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,0.5,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BoundaryEdge:

		glColor4f(1,0,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShapeOffset2v(GL_LINE_STRIP, pts, count, -5);
		break;
	}

	glLineWidth(1);
	glDisable(GL_LINE_STIPPLE);
}

void SideToPoints(IGISPointSequence * ps, int i, WED_MapZoomerNew * z,  vector<Point2>& pts)
{
	Segment2	s;
	Bezier2		b;
	if (ps->GetSide(gis_Geo,i,s,b))
	{
		s.p1 = b.p1;
		s.p2 = b.p2;

		b.p1 = z->LLToPixel(b.p1);
		b.p2 = z->LLToPixel(b.p2);
		b.c1 = z->LLToPixel(b.c1);
		b.c2 = z->LLToPixel(b.c2);


		int pixels_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
							sqrt(Vector2(b.c1,b.c2).squared_length()) +
							sqrt(Vector2(b.c2,b.p2).squared_length());
		int point_count = intlim(pixels_approx / BEZ_PIX_PER_SEG, BEZ_MIN_SEGS, BEZ_MAX_SEGS);
		pts.reserve(point_count+1);
		for (int n = 0; n <= point_count; ++n)
			pts.push_back(b.midpoint((float) n / (float) point_count));

	}
	else
	{
		pts.push_back(z->LLToPixel(s.p1));
		pts.push_back(z->LLToPixel(s.p2));
	}
}
