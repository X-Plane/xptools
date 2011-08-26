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

struct Point2c : public Point2 {

	Point2c() : c(false) {}
	Point2c(const Point2& rhs) : Point2(rhs), c(false) {}
	Point2c(const Point2& p, bool cc) : Point2(p), c(cc) {}
	Point2c(const Point2c& rhs) : Point2(rhs), c(rhs.c) {}

	int		c;
	
	bool operator==(const Point2c& rhs) const { return c == rhs.c && Point2::operator==(rhs); }

};

double best_bezier_approx(
					list<Point2c>::iterator orig_first,
					list<Point2c>::iterator orig_last,	// INCLUSIVE! REALLY!
					Point2c			 approx[4],
					double&			 t1_best,
					double&			 t2_best,
					double			 frac_ratio,
					int				 steps);					

void bezier_multi_simplify(
					list<Point2c>::iterator	first,
					list<Point2c>::iterator	last,			// INCLUSIVE! REALLY!
					list<Point2c>&			simplified,
					double					max_err);

void bezier_multi_simplify_straight_ok(
					list<Point2c>&		seq,
					double				max_err);


#endif /* BEZIERAPPROX_H */
