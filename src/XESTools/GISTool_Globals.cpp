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

#include "GISTool_Globals.h"

#include "MapDefs.h"
#include "DEMDefs.h"
#include "MeshDefs.h"

#if OPENGL_MAP
#include "RF_DrawMap.h"
#endif

Pmwx				gMap;
#if OPENGL_MAP
PmwxIndex_t			gMapIndex;
#endif

DEMGeoMap			gDem;
//CDT					gTriangulationLo;
CDT					gTriangulationHi;

//vector<Point2>		gMeshPoints;
//vector<Point2>		gMeshLines;

vector<pair<Point2,Point3> >		gMeshPoints;
vector<pair<Point2,Point3> >		gMeshLines;
vector<pair<Bezier2,pair<Point3, Point3> > >		gMeshBeziers;
bool				gVerbose = true;
bool				gTiming = false;
ProgressFunc		gProgress = ConsoleProgressFunc;

int					gMapWest  = -180;
int					gMapSouth = -90;
int					gMapEast  =  180;
int					gMapNorth =  90;

AptVector			gApts;
AptIndex				gAptIndex;

#if DEV
void	debug_mesh_line(const Point2& p1, const Point2& p2, float r1, float g1, float b1, float r2, float g2, float b2)
{
	gMeshLines.push_back(pair<Point2,Point3>(p1,Point3(r1,g1,b1)));
	gMeshLines.push_back(pair<Point2,Point3>(p2,Point3(r2,g2,b2)));
}

void	debug_mesh_point(const Point2& p1, float r1, float g1, float b1)
{
	gMeshPoints.push_back(pair<Point2,Point3>(p1,Point3(r1,g1,b1)));
}

void	debug_mesh_bezier(const Point2& p1, const Point2& p2, const Point2& p3, float r1, float g1, float b1, float r2, float g2, float b2)
{
	gMeshBeziers.push_back(pair<Bezier2,pair<Point3,Point3> >(Bezier2(p1,p2,p3), pair<Point3,Point3>(Point3(r1,g1,b1),Point3(r2,g2,b2))));
}

void	debug_mesh_bezier(const Point2& p1, const Point2& p2, const Point2& p3, const Point2& p4, float r1, float g1, float b1, float r2, float g2, float b2)
{
	gMeshBeziers.push_back(pair<Bezier2,pair<Point3,Point3> >(Bezier2(p1,p2,p3, p4), pair<Point3,Point3>(Point3(r1,g1,b1),Point3(r2,g2,b2))));
}

#endif
