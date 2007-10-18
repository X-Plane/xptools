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

#include "ProgressUtils.h"
#include "AptDefs.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

class Pmwx;
class CDT;
class DEMGeoMap;

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

extern vector<pair<Point2,Point3> >		gMeshPoints;
extern vector<pair<Point2,Point3> >		gMeshLines;

#endif
