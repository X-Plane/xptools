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
#ifndef COMPGEOM_H
#define COMPGEOM_H

I THINK THIS FILE IS OBSOLETE

#include "CGALTypes.h"

bool	IsNearColinear(const Point_2& a, const Point_2& b, const Point_2& c);
bool	IsNearColinear(const Direction_2& a, const Direction_2& b);
bool	IsNearColinear(const Vector_2& a, const Vector_2& b);

bool	IsOpposite(const Segment_2& a, const Segment_2& b);
bool	IsOpposite(const Direction_2& a, const Direction_2& b);
bool	IsNearParallel(const Segment_2& a, const Segment_2& b);
bool	IsNearParallel(const Direction_2& a, const Direction_2& b);
bool	IsNearParallel(const Vector_2& a, const Vector_2& b);


/* 
 * InsetPolygon
 *
 * Given a CCW polygon (or a segment chain), this routine insets
 * by a ratio for each segment multiplied by an inset.  For CW polygon,
 * use a negative ratio.  If the inset ratios are too large, the polygon
 * will become non-simple.
 *
 */
void	InsetPolygon(
				const Polygon_2&			inChain,
				const vector<double>		inRatios,
				double						inInset,
				bool						inIsRing,
				Polygon_2&					outChain);

/*
 * CalcMaxInset
 *
 * Given a CCW polygon and a vector of distances to inset the
 * polygon, this routine will calculate the maximum multiplyer
 * for those distances that the polygon may be inset before 
 * the inset polygon becomes non-simple.  At this inset point,
 * at least two adjacent points will be colocated.
 *
 * inIsRing is true for a polygon, false for a connected chain, but
 * see warnings below.
 *
 * WARNING: THIS ROUTINE IS NOT CORRECT!
 *
 * In the following case it will not work: if you create a 
 * concave simple polygon (for example, a thick letter H)
 * where concave offshoots would degenerate to triangles that 
 * would be islands in the degeneration, this routine will not
 * catch the islands, and instead return a non-simple polygon.
 *
 * Therefore a precondition of correct insetting is that 
 * the maximum simple inset not contain islands!
 *
 * When calculating a ring, collision of the beginning and end
 * of the ring are not detected; for this reason, set inIsRing
 * to true even for non-ring collisions.  This is not perfect but
 * will produce strange results less often.
 *
 */
double	CalcMaxInset(
				const Polygon_2&			inChain,
				const vector<double>		inRatios,
				bool						inIsRing);

/*
 * CalcMaxSideInset
 *
 * Given a CCW polygon and one of its sides (indexed by its source
 * being the Nth zero-based vertex), figure out how far in we can inset
 * this side.
 *
 */
double	CalcMaxSideInset(
				const Polygon_2&			inPolygon,
				int							inSideIndex);

#endif