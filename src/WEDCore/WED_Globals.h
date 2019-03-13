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
#ifndef WED_GLOBALS_H
#define WED_GLOBALS_H

#if DEV || DEBUG_VIS_LINES

#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

extern vector<pair<Point2,Point3> >		gMeshPoints;
extern vector<pair<Point2,Point3> >		gMeshLines;
extern vector<pair<Polygon2,Point3> >	gMeshPolygons;

//Note: These expect points to be in lat/lon
void	debug_mesh_bbox(const Bbox2& bb1, float r1, float g1, float b1, float r2, float g2, float b2);
void	debug_mesh_segment(const Segment2& s1, float r1, float g1, float b1, float r2, float g2, float b2);
void	debug_mesh_line(const Point2& p1, const Point2& p2, float r1, float g1, float b1, float r2, float g2, float b2);
void	debug_mesh_point(const Point2& p1, float r1, float g1, float b1);
void	debug_mesh_polygon(const Polygon2& p1, float r1, float g1, float b1);

#endif /* DEV */

/* Is WED running in English or metric units?  (feet == 0 -> metric.) */
extern int gIsFeet;
/* Infobar at bottom of Map in DD.DDD == 0 or DD MM SS == 1 */
extern int gInfoDMS;
/* Changes the listing in the gateway Import for GW moderation purposes */
extern int gModeratorMode;
/* Changes the listing in the gateway Import for GW moderation purposes */
extern int gFontSize;

enum WED_Export_Target {
		wet_xplane_900,		// X-Plane 9-compatible DSFs.
		wet_xplane_1000,	// X-Plane 10-compatible DSFs - includes, ATC, etc.
		wet_xplane_1021,	// Adds out-of-DSF overlays
		wet_xplane_1050,	// Adds next-gen apt.dat stuff
		wet_xplane_1100,	// Adds ground vehicles stuff
		wet_xplane_1130,	// Adds 8.33kHz raster frequencies
		wet_gateway,		// Latest format but with strict checking for gateway.
		wet_latest_xplane = wet_xplane_1130,	// meta-token for whatever the very newest x-plane export is
};

/* What target output format does WED want? */
extern WED_Export_Target gExportTarget;

/* Changes the listing in the gateway Import for GW moderation purposes */
extern string gCustomSlippyMap;

#endif
