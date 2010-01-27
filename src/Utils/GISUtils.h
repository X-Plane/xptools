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

#include <math.h>
#include "CGALDefs.h"

struct	Polygon2;
struct	Vector2;
struct	Point2;
struct	CoordTranslator2;
typedef	struct tiff TIFF;

// This routine returns the corners of a GeoTIFF file in the order:
// SW, SE, NW, NE, lon before lat.  It returns true if all four corners
// could be successfully fetched; all corners are returned in lat/lon.
// Important: these are CENTER-PIXEL corners, e.g. where EXACTLY does the MIDDLE
// of the CORNERS of the image go?  If the image is an area pixel WITHIN a tile,
// these are going to seem to be a bit small for a tile!
bool	FetchTIFFCorners(const char * inFileName, double corners[8], int& post_pos);
bool	FetchTIFFCornersWithTIFF(TIFF * inTiff, double corners[8], int& post_pos);

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
					CoordTranslator2&	outTranslator);
					
void	CreateTranslatorForBounds(
					const Bbox_2&		inBox,
					CoordTranslator_2&	outTranslator);

inline int	latlon_bucket(int p)
{
	if (p > 0) return (p / 10) * 10;
	else return ((-p + 9) / 10) * -10;
}


// Round a floating point number to fall as closely as possible onto a grid of N parts. 
// We use this to try to clean up screwed up DEM coordinates...1/1200 = floating point
// rounding error!
inline double round_by_parts(double c, int parts)
{
	double fparts=parts;
	return round(c * fparts) / fparts;
}

// Given a DEM of parts wide, this tries to round.  The trick is: we will round based on
// EITHER "point pixels" or "area pixels".  But how to know which one?  Well, most DEM
// samplings are even post spacings, e.g. 120, 1200, 3600, etc.  So if we see an odd number,
// assume that we have pixel-center and subtract.  If we see even, assume area center and add.
inline double round_by_parts_guess(double c, int parts)
{
	if(parts % 2)
		return round_by_parts(c, parts-1);	
	else
		return round_by_parts(c, parts  );	
}



void NorthHeading2VectorMeters(const Point2& ref, const Point2& p, double heading, Vector2& dir);
double VectorMeters2NorthHeading(const Point2& ref, const Point2& p, const Vector2& dir);
void NorthHeading2VectorDegs(const Point2& ref, const Point2& p, double heading, Vector2& dir);
double VectorDegs2NorthHeading(const Point2& ref, const Point2& p, const Vector2& dir);

void MetersToLLE(const Point2& ref, int count, Point2 * pts);
//double VectorLengthMeters(const Point2& ref, const Vector2& vec);

Vector2 VectorLLToMeters(const Point2& ref, const Vector2& v);
Vector2 VectorMetersToLL(const Point2& ref, const Vector2& v);



void	Quad_2to4(const Point2 ends[2], double width_mtr, Point2 corners[4]);
void	Quad_4to2(const Point2 corners[4], Point2 ends[2], double& width_mtr);
void	Quad_1to4(const Point2& ctr, double heading, double len_mtr, double width_mtr, Point2 corners[4]);
void	Quad_4to1(const Point2 corners[4], Point2& ctr, double& heading, double& len_mtr, double& width_mtr);

void	Quad_2to1(const Point2 ends[2], Point2& ctr, double& heading, double& len_mtr);
void	Quad_1to2(const Point2& ctr, double heading, double len_mtr, Point2 ends[2]);
void	Quad_diagto1(const Point2 ends[2], double width_mtr, Point2& ctr, double& heading, double& len_mtr, int swapped);

void	Quad_MoveSide2(Point2 ends[2], double& width_mtr, int side, const Vector2& delta);
void	Quad_ResizeSide4(Point2 corners[4], int side, const Vector2& move, bool symetric);
void	Quad_ResizeCorner1(Point2& ctr, double heading, double& l, double& w, int corner, const Vector2& move, bool symetric);

#endif
