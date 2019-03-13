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

#ifndef WED_DrawUtils_H
#define WED_DrawUtils_H

#include "CompGeomDefs2.h"
#include "WED_Colors.h"
#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

class	GUI_GraphState;
class	IGISPointSequence;
class	IGISPolygon;
class	WED_MapZoomerNew;

inline void glVertex2(const Point2& p) { glVertex2d(p.x(),p.y()); }
inline void glTexCoord2(const Point2& p) { glTexCoord2d(p.x(),p.y()); }

inline void	glVertex2v(const Point2 * p, int n) { while(n--) { glVertex2d(p->x(),p->y()); ++p; } }
inline void glShape2v(GLenum mode,  const Point2 * p, int n) { glBegin(mode); glVertex2v(p,n); glEnd(); }

inline void glShapeOffset2v(GLenum mode,  const Point2 * pts, int n, double offset)
{
	glBegin(mode);
	for (int i = 0; i < n; ++i)
	{
		Vector2	dir1,dir2;
		if (i > 0  ) dir1 = Vector2(pts[i-1],pts[i  ]);
		if (i < n-1) dir2 = Vector2(pts[i  ],pts[i+1]);
		Vector2	dir = dir1+dir2;
		dir = dir.perpendicular_ccw();
		dir.normalize();
		dir *= offset;
		glVertex2d(pts[i].x() + dir.dx, pts[i].y() + dir.dy);
	}
	glEnd();
}


void DrawLineAttrs(const Point2 * pts, int cnt, const set<int>& attrs);
int BezierPtsCount(const Bezier2& b, WED_MapZoomerNew * z);

// A note on UV mapping: we encode a point sequence for UV mapping as a pair of points, the vertex coord followed by the UV coords.
// So it's an interleaved array.  This is what PointSequenceToVector returns too.
void glPolygon2(const Point2 * pts, bool has_uv, const int * contours, int n);
void PointSequenceToVector(IGISPointSequence * ps, WED_MapZoomerNew * z, vector<Point2>& pts, bool get_uv, vector<int>& contours,
	int is_hole, bool dupFirst = false);  // dupFirst == duplicate first/last node even on closed rings. Not desired to build polygons, but desired to draw lines
void SideToPoints(IGISPointSequence * ps, int n, WED_MapZoomerNew * z,  vector<Point2>& out_pts);


#endif /* WED_DrawUtils_H */
