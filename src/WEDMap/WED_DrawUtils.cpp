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
#if APL
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

/*
bool IsBezierSequenceScrewed(IGISPointSequence * ps, vector<Point2> * where)
{
	if(!ps->IsClosed()) return false;
	
	vector<Bezier2>	der_sides;
	for(int n = 0; n < ps->GetNumSides(); ++n)
	{	
		Bezier2 b;
		Segment2 s;
		ps->GetSide(n,s,b);
		der_sides.push_back(b);
	}
	if(where == NULL) 
		return do_beziers_cross(der_sides);
	vector<Point2>	p;
	if(where == NULL) where = &p;
	find_crossing_beziers(der_sides,*where);
	return !where->empty();
}
*/

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
		if(get_uv) ps->GetSideUV(i,suv,buv);
		if (ps->GetSide(i,s,b))
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

