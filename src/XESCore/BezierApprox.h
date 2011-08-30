/* 
 * Copyright (c) 2011, Laminar Research.
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

#ifndef BEZIERAPPROX_H
#define BEZIERAPPROX_H

#include "CompGeomDefs2.h"
#include <list>

// Bezier 2 point with a control flag.  C = 1 means a control handle, C = 0 means a regular end-point.
struct Point2c : public Point2 {

	Point2c() : c(false) {}
	Point2c(const Point2& rhs) : Point2(rhs), c(false) {}
	Point2c(const Point2& p, bool cc) : Point2(p), c(cc) {}
	Point2c(const Point2c& rhs) : Point2(rhs), c(rhs.c) {}

	int		c;
	
	bool operator==(const Point2c& rhs) const { return c == rhs.c && Point2::operator==(rhs); }

};

// This routine "scrubs" for a bezier approximation.  Approx is loaded with our first guess approximation; 
// it is modified and t1/t2_best end up with the scaling factors we used.  frac_ratio is how much to jump
// each T vaue, and steps are the total steps - we do steps^2 tries so keep steps low!!
// max_err is a linear error metric that we can NEVER exceed - basically this is a qiuck exit if a point is
// too far from the curve.  Smaller max_err makes huge speed improvements since this code uses a spatial index.
// This returns the variance of the linear distance along the line over a reasonably large number of points.
double best_bezier_approx(
					list<Point2c>::iterator orig_first,
					list<Point2c>::iterator orig_last,	// INCLUSIVE! REALLY!
					Point2c			 approx[4],
					double&			 t1_best,
					double&			 t2_best,
					double			 frac_ratio,
					int				 steps,
					double			 max_err);

// Simplify a string of bezier curves, all of whom must be piece-wies quadratic or bezier.
// max_err is the maximum variance in distance we will accept; we stop simplifying when we hit
// this.  lim_err is the absolute linear limit in error, used to clip out grossly wrong points.  Generally
// lim_err should be significantly larger than max_err, since we want to do a lot better than "every point is
// nearly at its worst case".
void bezier_multi_simplify(
					list<Point2c>::iterator	first,
					list<Point2c>::iterator	last,			// INCLUSIVE! REALLY!
					list<Point2c>&			simplified,
					double					max_err,
					double					lim_err);

// Same as above, except there can be line segments in the piece-wise curves; this operation works in-place.
void bezier_multi_simplify_straight_ok(
					list<Point2c>&		seq,
					double				max_err,
					double				lim_err);


#endif /* BEZIERAPPROX_H */
