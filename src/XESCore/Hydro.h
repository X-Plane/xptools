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

#ifndef HYDRO_H
#define HYDRO_H

class	Pmwx;
class	DEMGeoMap;
class	GISHalfedge;
class	GISFace;
class	Bbox2;

#include "ProgressUtils.h"

// Fully rebuild a map based on elevation and other DEM params.
void	HydroReconstruct(Pmwx& ioMap, DEMGeoMap& ioDem, const char * mask_file,const char * hydro_dir, ProgressFunc inFunc);

// Simplify the coastlines of a complex map based on certain areas to cover.
void	SimplifyCoastlines(Pmwx& ioMap, const Bbox2& bounds, ProgressFunc inFunc);

// For testing
//void	SimplifyCoastlineFace(Pmwx& ioMap, GISFace * face);

bool	MakeWetMask(const char * inShapeDir, int lon, int lat, const char * inMaskDir);

#endif
