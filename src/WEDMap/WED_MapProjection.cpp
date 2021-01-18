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
#include "WED_MapProjection.h"

#include "XESConstants.h"

WED_MapProjection::WED_MapProjection()
	: mOriginLat(0),
	  mOriginLon(0),
      mUnitsPerDegLat(1),
      mStandardParallelCos(1)
{
}

void WED_MapProjection::SetOriginLL(const Point2& ll)
{
	mOriginLon = ll.x();
	mOriginLat = ll.y();
}

void WED_MapProjection::SetStandardParallel(double lat)
{
	mStandardParallelCos = cos(lat * DEG_TO_RAD);
}

void WED_MapProjection::SetXYUnitsPerDegLat(double unitsPerDeg)
{
	mUnitsPerDegLat = unitsPerDeg;
}

void WED_MapProjection::SetXYUnitsPerMeter(double unitsPerMeter)
{
	mUnitsPerDegLat = unitsPerMeter * DEG_TO_MTR_LAT;
}

double WED_MapProjection::XYUnitsPerMeter() const
{
	return mUnitsPerDegLat * MTR_TO_DEG_LAT;
}

double WED_MapProjection::XToLon(double x) const
{
	return mOriginLon + x / mUnitsPerDegLat / mStandardParallelCos;
}

double WED_MapProjection::YToLat(double y) const
{
	return mOriginLat + y / mUnitsPerDegLat;
}

double WED_MapProjection::LonToX(double lon) const
{
	return (lon - mOriginLon) * mUnitsPerDegLat * mStandardParallelCos;
}

double WED_MapProjection::LatToY(double lat) const
{
	return (lat - mOriginLat) * mUnitsPerDegLat;
}

Point2 WED_MapProjection::XYToLL(const Point2& p) const
{
	return Point2(XToLon(p.x()), YToLat(p.y()));
}

Point2 WED_MapProjection::LLToXY(const Point2& p) const
{
	return Point2(LonToX(p.x()), LatToY(p.y()));
}

void WED_MapProjection::XYToLLv(Point2 * dst, const Point2 * src, int n) const
{
	while (n--)
		*dst++ = XYToLL(*src++);
}

void WED_MapProjection::LLToXYv(Point2 * dst, const Point2 * src, int n) const
{
	while (n--)
		*dst++ = LLToXY(*src++);
}