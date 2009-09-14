/* 
 * Copyright (c) 2009, Laminar Research.
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

#ifndef Bezier_H
#define Bezier_H
// janos says: huh?
//#error We are not ready to use this header yet.  We need to get gmp into the general build environment.

#include "CGALDefs.h"

#include <CGAL/Cartesian.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/General_polygon_2.h>
#include <CGAL/General_polygon_with_holes_2.h>
#include <CGAL/Gps_segment_traits_2.h>

#include "CompGeomDefs2.h"

typedef CGAL::CORE_algebraic_number_traits					Nt_traits;
typedef Nt_traits::Rational									Rational;
typedef Nt_traits::Algebraic								Algebraic;
typedef CGAL::Cartesian<Rational>							Rat_kernel;
typedef CGAL::Cartesian<Algebraic>							Alg_kernel;
typedef Rat_kernel::Point_2									Rat_point_2;
typedef Alg_kernel::Point_2									Alg_point_2;
typedef std::vector<Rat_point_2>							Bezier_container_;
typedef CGAL::Arr_Bezier_curve_traits_2<Rat_kernel,
                                        Alg_kernel,
                                        Nt_traits>			Bezier_traits_base_;
typedef CGAL::Gps_segment_traits_2<Rat_kernel, Bezier_container_, Bezier_traits_base_>	Bezier_traits_2;
										
typedef Bezier_traits_2::Curve_2							Bezier_curve_2;

typedef Bezier_traits_2::Point_2							Bezier_point_2;
typedef CGAL::General_polygon_set_2<Bezier_traits_2>		Bezier_polygon_set_2;
typedef Bezier_polygon_set_2::Arrangement_2					Bezier_arrangement_2;
typedef CGAL::General_polygon_2<Bezier_traits_2>			Bezier_polygon_2;
typedef CGAL::General_polygon_with_holes_2<Bezier_traits_2>	Bezier_polygon_with_holes_2;

void	find_crossing_beziers(
					const vector<Bezier_curve_2>& in_curves,
						  vector<Bezier_point_2>& out_xons);

void	find_crossing_beziers(
					const vector<Bezier2>&	in_curves,
						  vector<Point2>&	out_xons);


bool	do_beziers_cross(
					const vector<Bezier_curve_2>& in_curves);
					
bool	do_beziers_cross(
					const vector<Bezier2>&	in_curves);

#endif /* Bezier_H */
