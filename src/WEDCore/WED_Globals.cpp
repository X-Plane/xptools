/*
 * Copyright (c) 2004, Laminar Research.
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
#include "WED_Globals.h"

#if DEV || DEBUG_VIS_LINES

vector<pair<Point2,Point3> >		gMeshPoints;
vector<pair<Point2,Point3> >		gMeshLines;
vector<pair<Polygon2,Point3> >		gMeshPolygons;

void	debug_mesh_bbox(const Bbox2& bb1, float r1, float g1, float b1, float r2, float g2, float b2)
{
	debug_mesh_segment(bb1.left_side(),   r1, g1, b1, r2, g2, b2);
	debug_mesh_segment(bb1.top_side(),    r1, g1, b1, r2, g2, b2);
	debug_mesh_segment(bb1.right_side(),  r1, g1, b1, r2, g2, b2);
	debug_mesh_segment(bb1.bottom_side(), r1, g1, b1, r2, g2, b2);
}

void	debug_mesh_segment(const Segment2& s1, float r1, float g1, float b1, float r2, float g2, float b2)
{
	debug_mesh_line(s1.p1, s1.p2, r1, g1, b1, r2, g2, b2);
}

void	debug_mesh_line(const Point2& p1, const Point2& p2, float r1, float g1, float b1, float r2, float g2, float b2)
{
	gMeshLines.push_back(pair<Point2,Point3>(p1,Point3(r1,g1,b1)));
	gMeshLines.push_back(pair<Point2,Point3>(p2,Point3(r2,g2,b2)));
}

void	debug_mesh_point(const Point2& p1, float r1, float g1, float b1)
{
	gMeshPoints.push_back(pair<Point2,Point3>(p1,Point3(r1,g1,b1)));
}

void	debug_mesh_polygon(const Polygon2& p1, float r1, float g1, float b1)
{
	gMeshPolygons.push_back(pair<Polygon2,Point3>(p1,Point3(r1,g1,b1)));
}
#endif

WED_Export_Target gExportTarget = wet_latest_xplane;
