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
#ifndef GISUTILS_H
#define GISUTILS_H

struct	Polygon2;
struct	Vector2;
struct	Point2;
struct	CoordTranslator;
typedef	struct tiff TIFF;

// This routine returns the corners of a GeoTIFF file in the order: 
// SW, SE, NW, NE, lon before lat.  It returns true if all four corners
// could be successfully fetched; all corners are returned in lat/lon.
bool	FetchTIFFCorners(const char * inFileName, double corners[8]);
bool	FetchTIFFCornersWithTIFF(TIFF * inTiff, double corners[8]);

// This routine converts UTM to lat/lon coordinates.  X and Y should be
// in meters.  Zone should be positive 1-60 for north or -1-60 for south.
// The pts to outLon and outLat can point to the same storage as x and y;
// this routine will work in-place.
void	UTMToLonLat(double x, double y, int zone, double * outLon, double * outLat);

double	LonLatDistMeters(double lon1, double lat1, double lon2, double lat2);
double	LonLatDistMetersWithScale(double lon1, double lat1, double lon2, double lat2, 
								double deg_to_mtr_x, double deg_to_mtr_y);

void	CreateTranslatorForPolygon(
					const Polygon2&		inPolygon,
					CoordTranslator&	outTranslator);

inline int	latlon_bucket(int p)
{
	if (p > 0) return (p / 10) * 10;
	else return ((-p + 9) / 10) * -10;
}

void NorthHeading2Vector(const Point2& ref, const Point2& p, double heading, Vector2& dir);
void MetersToLLE(const Point2& ref, int count, Point2 * pts);


#endif
