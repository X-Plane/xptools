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

#ifndef GISTOOL_GLOBALS_H
#define GISTOOL_GLOBALS_H

#include "MapDefs.h"
#include "ProgressUtils.h"
#include "AptDefs.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

//class Pmwx;
class CDT;
class DEMGeoMap;
struct	PmwxIndex_t;


enum rf_region {
	rf_usa = 0,
	rf_eu = 1
};

extern bool					gMobile; // true if we're building for Mobile; false if we're building for Desktop
extern rf_region			gRegion;

extern Pmwx					gMap;
extern DEMGeoMap			gDem;
//extern CDT					gTriangulationLo;
extern CDT					gTriangulationHi;

extern bool					gVerbose;
extern bool					gTiming;
extern ProgressFunc			gProgress;

extern	int					gMapWest;
extern	int					gMapSouth;
extern	int					gMapEast;
extern	int					gMapNorth;

extern AptVector			gApts;
extern AptIndex				gAptIndex;

extern vector<pair<Point2,Point3> >					gMeshPoints;
extern vector<pair<Point2,Point3> >					gMeshLines;
extern vector<pair<Bezier2,pair<Point3, Point3> > >	gMeshBeziers;

#if OPENGL_MAP
extern PmwxIndex_t			gMapIndex;
#endif

#if DEV
void	debug_mesh_line	 (const Point2& p1, const Point2& p2, float r1, float g1, float b1, float r2, float g2, float b2);
void	debug_mesh_bezier(const Point2& p1, const Point2& p2, const Point2& p3, float r1, float g1, float b1, float r2, float g2, float b2);	// quadratic bezier! Hah!
void	debug_mesh_bezier(const Point2& p1, const Point2& p2, const Point2& p3, const Point2& p4, float r1, float g1, float b1, float r2, float g2, float b2);	// quadratic bezier! Hah!
void	debug_mesh_point (const Point2& p1, float r1, float g1, float b1);
#endif

#endif
