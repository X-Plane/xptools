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

#ifndef AIRPORTS_H
#define AIRPORTS_H

#include "AptDefs.h"
#include "ProgressUtils.h"
class	Pmwx;
struct	DEMGeo;

void	GenBoundary(
				AptInfo_t * 	ioAirport);


void	AptPolygonToBezier(
				const AptPolygon_t&			inPoly,
				vector<vector<Bezier2> >&	outPoly);

void	BezierToSegments(
				const vector<Bezier2>&		inWinding,
				Polygon2&					outWinding,
				float						inSimplify);

void BurnInAirport(
				const AptInfo_t * 	inAirport,
				Pmwx&				ioMap,
				bool				inFillWater);


void ProcessAirports(
				const AptVector& 	inAirports,
				Pmwx& 				ioMap,
				DEMGeo& 			ioElevation,
				DEMGeo& 			ioTransport,
				bool				inCrop,
				bool				inProcessDEMs,
				bool				kill_rivers,
				ProgressFunc		inProgress);

#endif /* AIRPORTS_H */
