/*
 * Copyright (c) 2021, Laminar Research.
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
#ifndef WED_MAPPROJECTION_H
#define WED_MAPPROJECTION_H

#include "CompGeomDefs2.h"

/*
	WED_MapProjection

	A map projection that converts lat/lon coordinates to XY coordinates (i.e.
	world coordinates as used in OpenGL drawing) and back.

	The specific projection that is used is an equirectangular projection
	(https://en.wikipedia.org/wiki/Equirectangular_projection). Among other
	things, this guarantees that the coordinate axes can be converted
	independently of each other.
*/

class WED_MapProjection
{
public:
	WED_MapProjection();

	// Sets the point in lon/lat coordinates that corresponds to the origin in
	// XY coordinates.
	void SetOriginLL(const Point2& ll);

	// Sets the standard parallel. This is the latitude at which the scale of the
	// projection is true, i.e. where the projection does not introduce
	// distortion. Note that this does not need to be the same latitude as the
	// origin set with SetOriginLL().
	void SetStandardParallel(double lat);

	// Sets the number of units in XY space that correspond to one degree of
	// latitude or one meter.
	void SetXYUnitsPerDegLat(double unitsPerDeg);
	void SetXYUnitsPerMeter(double unitsPerMeter);

	// Returns the number of units in XY space that correspond to one meter.
	double XYUnitsPerMeter() const;

	// Converts between lat/lon and XY along a single axis.
	double XToLon(double x) const;
	double YToLat(double y) const;
	double LonToX(double lon) const;
	double LatToY(double lat) const;

	// Converts a single point between lat/lon and XY.
	Point2 XYToLL(const Point2& p) const;
	Point2 LLToXY(const Point2& p) const;

	// Converts multiple points between lat/lon and XY.
	void XYToLLv(Point2 * dst, const Point2 * src, int n) const;
	void LLToXYv(Point2 * dst, const Point2 * src, int n) const;

private:
	double mOriginLat;
	double mOriginLon;
	double mUnitsPerDegLat;
	double mStandardParallelCos;
};

#endif