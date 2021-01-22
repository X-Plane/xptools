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

#if LIBTESS
	#include "tesselator.h"
#else
	#if APL
		#include <OpenGL/glu.h>
	#else
		#include <GL/glu.h>
	#endif
#endif

static int BezierPtsCount(const Bezier2& b, const WED_Camera * camera)
{
	int pixels_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
						sqrt(Vector2(b.c1,b.c2).squared_length()) +
						sqrt(Vector2(b.c2,b.p2).squared_length());
	int point_count = intlim(pixels_approx / BEZ_PIX_PER_SEG, BEZ_MIN_SEGS, BEZ_MAX_SEGS);

	if(camera && point_count > BEZ_MIN_SEGS)
	{
		Bbox2 bb;
		double zb[4];
		bool visible = true;

		b.bounds_fast(bb);

		if (!camera->BboxVisible(Bbox3(bb)))
			point_count = BEZ_MIN_SEGS; // greatly simplify not visible bezierss
	}
	return point_count;
}

int BezierPtsCount(const Bezier2& b, WED_MapZoomerNew * z)
{
	return BezierPtsCount(b, static_cast<const WED_Camera *>(z));
}

void PointSequenceToVector(
			IGISPointSequence *			ps,
			const WED_MapProjection&	projection,
			const WED_Camera&			camera,
			vector<Point2>&				pts,
			bool						get_uv,
			vector<int>&				contours,
			int							is_hole,
			bool						dupFirst)
{
	int n = ps->GetNumSides();
	
	for (int i = 0; i < n; ++i)
	{
		Bezier2		b, buv;
		if(get_uv) ps->GetSide(gis_UV,i,buv);
		if (ps->GetSide(gis_Geo,i,b))
		{
			b.p1 = projection.LLToXY(b.p1);
			b.p2 = projection.LLToXY(b.p2);
			b.c1 = projection.LLToXY(b.c1);
			b.c2 = projection.LLToXY(b.c2);

			int point_count = BezierPtsCount(b, &camera);

			pts.reserve(pts.capacity() + point_count * (get_uv ? 2 : 1));
			contours.reserve(contours.capacity() + point_count);
			for (int k = 0; k < point_count; ++k)
			{
							pts.push_back(b.midpoint((float) k / (float) point_count));
				if(get_uv)	pts.push_back(buv.midpoint((float) k / (float) point_count));
				contours.push_back((k == 0 && i == 0) ? is_hole : 0);
			}

			if (i == n-1 && (!ps->IsClosed() || dupFirst))
			{
							pts.push_back(b.p2);
				if(get_uv)	pts.push_back(buv.p2);
				contours.push_back(0);
			}
		}
		else
		{
							pts.push_back(projection.LLToXY(b.p1));
			if(get_uv)		pts.push_back(buv.p1);
			contours.push_back(i == 0 ? is_hole : 0);
			if (i == n-1 && (!ps->IsClosed() || dupFirst))
			{
							pts.push_back(projection.LLToXY(b.p2));
				if(get_uv)	pts.push_back(buv.p2);
				contours.push_back(0);
			}
		}
	}
}

void PointSequenceToVector(
	IGISPointSequence *			ps,
	WED_MapZoomerNew *			z,
	vector<Point2>&				pts,
	bool						get_uv,
	vector<int>&				contours,
	int							is_hole,
	bool						dupFirst)
{
	PointSequenceToVector(ps, z->Projection(), *z, pts, get_uv, contours, is_hole, dupFirst);
}

#if !LIBTESS
  #if !IBM
	#define CALLBACK
  #endif
static void CALLBACK TessBegin(GLenum mode)		{ glBegin(mode);}
static void CALLBACK TessEnd(void)				{ glEnd();		}
static void CALLBACK TessVertex(const Point2 * p)
{
	glVertex2d(p->x(),p->y());
}
static void CALLBACK TessVertexUV(const Point2 * p)
{
	const Point2 * uv = p; ++uv; 
	glTexCoord2f(uv->x(), uv->y());
	glVertex2d(p->x(),p->y());
}
static void CALLBACK TessVertexUVh(const Point2 * p, float * h)
{
	const Point2 * uv = p; ++uv; 
	glTexCoord2f(uv->x(), uv->y());
	glVertex3d(p->x(), *h, p->y());
}
#endif

void glPolygon2(const Point2 * pts, bool has_uv, const int * contours, int n, float height)
{
#if LIBTESS
	TESStesselator * tess = tessNewTess(NULL);
	const Point2 * pts_p(pts);
	vector<GLfloat>	raw_pts;
	raw_pts.reserve(n * 2);

	for(int i = 0; i < n; ++i)
	{
		if(contours && contours[i])
		{
			if(!raw_pts.empty())
			{
				tessAddContour(tess, 2, &raw_pts[0], 2 * sizeof(GLfloat), raw_pts.size() / 2);
				raw_pts.clear();
			}
		}
		raw_pts.push_back(pts_p->x());
		raw_pts.push_back(pts_p->y());
		pts_p++;
		if(has_uv)	pts_p++;
	}
	if(!raw_pts.empty())
		tessAddContour(tess, 2, &raw_pts[0], 2 * sizeof(GLfloat), raw_pts.size() / 2);

	if(tessTesselate(tess, TESS_WINDING_NONZERO, TESS_POLYGONS, 3, 2, 0))
	if(tessGetVertexCount(tess) == n) // don't be better than gluTess (used in XP) and show textureing with self-intersecting contours
	{
		const TESSindex* vert_idx = tessGetElements(tess);
		int tri_count = tessGetElementCount(tess);
		const TESSreal * verts = tessGetVertices(tess);

		if(has_uv)
		{
			const TESSindex * vidx = tessGetVertexIndices(tess);
			glBegin(GL_TRIANGLES);
			if (height == -1.0f)
				while (tri_count--)
					for (int i = 0; i < 3; i++)
					{
						if (*vert_idx == TESS_UNDEF) break;
						glTexCoord2(pts[1 + 2 * vidx[*vert_idx]]); glVertex2fv(&verts[2 * (*vert_idx)]);
						vert_idx++;
					}
			else
				while(tri_count--)
					for (int i = 0 ; i < 3; i++)
					{
						if(*vert_idx == TESS_UNDEF) break;
						glTexCoord2(pts[1 + 2 * vidx[*vert_idx]]); glVertex3f(verts[2 * (*vert_idx)], height, verts[2 * (*vert_idx) + 1]);
						vert_idx++;
					}
			glEnd();
		}
		else
		{
	#if 1
			glBegin(GL_TRIANGLES);
			if (height == -1.0f)
				while (tri_count--)
					for (int i = 0; i < 3; i++)
						glVertex2fv(&verts[2 * (*vert_idx++)]);
			else
				while(tri_count--)
					for (int i = 0 ; i < 3; i++)
					{
						glVertex3f(verts[2 * (*vert_idx)], height, verts[2 * (*vert_idx) + 1]);
						vert_idx++;
					}
			glEnd();
	#else  // not any faster, cuz of too many state changes -> cache those ?
			glVertexPointer(2, GL_FLOAT, 2 * sizeof(TESSreal), verts);
			glEnableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDrawElements(GL_TRIANGLES, 3 * tri_count, GL_INT, vert_idx);
			glDisableClientState(GL_VERTEX_ARRAY);
	#endif
		}
	}
	tessDeleteTess(tess);
#else // not LIBTESS
	GLUtesselator * tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN,	(void (CALLBACK *)(void))TessBegin);
	gluTessCallback(tess, GLU_TESS_END,		(void (CALLBACK *)(void))TessEnd);
	if(has_uv)
	{
		if(height == -1.0f)
			gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))TessVertexUV);
		else
			gluTessCallback(tess, GLU_TESS_VERTEX_DATA,	(void (CALLBACK *)(void))TessVertexUVh);
	}
	else
		gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))TessVertex);
		
	gluTessBeginPolygon(tess,(void *) &height);
	while(n--)
	{
		if (contours && *contours++)	gluNextContour(tess, GLU_INTERIOR);

		double	xyz[3] = { pts->x(), pts->y(), 0 };
		gluTessVertex(tess, xyz, (void*) pts++);
		if(has_uv) pts++;
	}
	gluEndPolygon (tess);
	gluDeleteTess(tess);
#endif
}

#define 	line_TaxiWayHatch  line_BoundaryEdge+1
#define 	line_BChequered    line_BoundaryEdge+2
#define 	line_BBrokenWhite  line_BoundaryEdge+3

void DrawLineAttrs(const Point2 * pts, int cnt, const set<int>& attrs)
{
//  Its assumed that this is all set up already at this point
//	glColor4f()
//	glLineWidth(1);
//	glDisable(GL_LINE_STIPPLE);

	if (attrs.empty())
	{
		glShape2v(GL_LINE_STRIP, pts, cnt);
		return;
	}
	else
	{ 
		for(set<int>::const_iterator a = attrs.begin(); a != attrs.end(); ++a)  // first layer: draw only line styles
		{
			int b = *a;
			// do *some* guessing on closest aproximation for XP11.25 added line types. Don't want to put too much effort into this.
			if(b > line_BoundaryEdge)
			{
				int e = ENUM_Export(*a);
					  if(e == 11) b = line_ILSCriticalCenter;
				else if(e == 12) b = line_RunwayHold;
				else if(e == 13) b = line_OtherHold;
				else if(e == 14) b = line_ILSHold;
				else if(e == 19) b = line_TaxiWayHatch;
				else if(e <= 20) b = line_SolidYellow;
				else if(e == 61) b = line_BILSCriticalCenter;
				else if(e == 62) b = line_BRunwayHold;
				else if(e == 63) b = line_BOtherHold;
				else if(e == 64) b = line_BILSHold;
				else if(e >= 60 && e <  70) b = line_BSolidYellow;
				else if(e == 71) b = line_BChequered;
				else if(e == 72) b = line_BBrokenWhite;
			}
			
			switch(b) {
			// ------------ STANDARD TAXIWAY LINES ------------
			case line_BSolidYellow:
				glColor4f(0,0,0,1);
				glLineWidth(3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(1);
			case line_SolidYellow:
				glColor4f(1,1,0,1);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			case line_BBrokenYellow:
				glColor4f(0,0,0,1);
				glLineWidth(3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(1);
			case line_BrokenYellow:
				glColor4f(1,1,0,1);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xF0F0);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			case line_BDoubleSolidYellow:
				glColor4f(0,0,0,1);
				glLineWidth(5);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glLineWidth(3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0,0,0,1);
				glLineWidth(1);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_DoubleSolidYellow:
				glColor4f(1,1,0,1);
				glLineWidth(3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0.4,0.4,0.4,1);
				glLineWidth(1);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			case line_BRunwayHold:
				glColor4f(0,0,0,1);
				glLineWidth(9);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(1);
				glColor4f(1,1,0,1);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,-1);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,-3);
				glLineStipple(1,0xF0F0);
				glEnable(GL_LINE_STIPPLE);
				glLineWidth(3);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,2);
				glColor4f(0,0,0,1);
				glLineWidth(1);
				glDisable(GL_LINE_STIPPLE);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,2);
				break;
			case line_RunwayHold:
				glColor4f(1,1,0,1);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,-1);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,-3);
				glLineStipple(1,0xF0F0);
				glEnable(GL_LINE_STIPPLE);
				glLineWidth(3);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,2);
				glColor4f(0.4,0.4,0.4,1);
				glLineWidth(1);
				glDisable(GL_LINE_STIPPLE);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,2);
				break;

			case line_BOtherHold:
				glColor4f(0,0,0,1);
				glLineWidth(5);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(1);
			case line_OtherHold:
				glColor4f(1,1,0,1);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1,0xF0F0);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,1.4);
				glDisable(GL_LINE_STIPPLE);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,-1.4);
				break;

			case line_BILSHold:
				glColor4f(0,0,0,1);
				glLineWidth(9);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glLineWidth(5);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0,0,0,1);
				glLineWidth(3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0x1111);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_ILSHold:
				glColor4f(1,1,0,1);
				glLineWidth(5.3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0.4,0.4,0.4,1);
				glLineWidth(3.3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0x0303);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			case line_BILSCriticalCenter:
				glColor4f(0,0,0,1);
				glLineWidth(5);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glLineWidth(5);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xFF00);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0.3,0.3,0.3,1);
				glLineWidth(3);
				glDisable(GL_LINE_STIPPLE);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glLineWidth(1);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_ILSCriticalCenter:
				glColor4f(1,1,0,1);
				glLineWidth(5);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xFF00);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0.4,0.4,0.4,1);
				glLineWidth(3);
				glDisable(GL_LINE_STIPPLE);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glLineWidth(1);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			case line_BWideBrokenSingle:
				glColor4f(0,0,0,1);
				glLineWidth(3);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(1);
			case line_WideBrokenSingle:
				glColor4f(1,1,0,1);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xFF00);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			case line_BWideBrokenDouble:
				glColor4f(0,0,0,1);
				glLineWidth(5);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,1,0,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xFF00);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0,0,0,1);
				glLineWidth(1);
				glDisable(GL_LINE_STIPPLE);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_WideBrokenDouble:
				glColor4f(1,1,0,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xFF00);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0.4,0.4,0.4,1);
				glLineWidth(1);
				glDisable(GL_LINE_STIPPLE);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			case line_TaxiWayHatch:
				glColor4f(1,1,0,0.8);
				glLineWidth(4);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			// ------------ ROADWAY TAXIWAY LINES ------------
			case line_SolidWhite:
				glColor4f(1,1,1,1);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_BChequered:
				glColor4f(0,0,0,1);
				glLineWidth(4);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(1);
			case line_Chequered:
				glColor4f(1,1,1,0.8);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(2);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xF0F0);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,-1.2);
				glLineStipple(1, 0x0F0F);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,+1.2);
				break;
			case line_BBrokenWhite:
				glLineWidth(3);
				glColor4f(0,0,0,1);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glLineWidth(1);
			case line_BrokenWhite:
				glColor4f(1,1,1,1);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xFF00);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;

			default:
				if(ENUM_Export(*a) < 100)
				{
					glColor4f(1,1,1,1);
					glShape2v(GL_LINE_STRIP, pts, cnt);
				}
			}
		}
		for(set<int>::const_iterator a = attrs.begin(); a != attrs.end(); ++a)  // second layer: do only draw lights, so they end up ontop of line styles 
		{
			int b = *a;
			// do *some* guessing on closest aproximation for XP11.25 added light types. Don't want to put too much effort into this.
			if(b > line_BoundaryEdge)
			{
				int e = ENUM_Export(*a);
					 if(e == 107) b = line_TaxiCenter;
				else if(e == 108) b = line_HoldShortCenter;
			}

			switch(b) {
			// ------------ LIGHTS ------------
			case line_TaxiCenter:
				glColor4f(0.3,1,0.3,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1,0x7000);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_TaxiEdge:
				glColor4f(0,0,1,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1,0x7000);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt,-5);
				break;
			case line_HoldLights:
				glColor4f(1,0.5,0,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1,0x7070);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_HoldLightsPulse:
				glColor4f(1,0.5,0,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1,0x7000);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(0.3,0.1,0,1);
				glLineStipple(1,0x0070);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_HoldShortCenter:
				glColor4f(0.3,1,0.3,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(3,0x1010);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				glColor4f(1,0.5,0,1);
				glLineStipple(3,0x0101);
				glShape2v(GL_LINE_STRIP, pts, cnt);
				break;
			case line_BoundaryEdge:
				glColor4f(1,0,0,1);
				glLineWidth(3);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1,0x7000);
				glShapeOffset2v(GL_LINE_STRIP, pts, cnt, -5);
				break;
			default:
				break; // for unknown stuff, we draw nothing in this layer
			}
		}		
	}
	glLineWidth(1);
	glDisable(GL_LINE_STIPPLE);
}

void SideToPoints(IGISPointSequence * ps, int i, const WED_MapProjection& projection, const WED_Camera& camera, vector<Point2>& pts)
{
	Bezier2		b;
	if (ps->GetSide(gis_Geo,i,b))
	{
		b.p1 = projection.LLToXY(b.p1);
		b.p2 = projection.LLToXY(b.p2);
		b.c1 = projection.LLToXY(b.c1);
		b.c2 = projection.LLToXY(b.c2);

		int point_count = BezierPtsCount(b,&camera);

		pts.reserve(point_count+1);
		for (int n = 0; n <= point_count; ++n)
			pts.push_back(b.midpoint((float) n / (float) point_count));

	}
	else
	{
		pts.push_back(projection.LLToXY(b.p1));
		pts.push_back(projection.LLToXY(b.p2));
	}
}