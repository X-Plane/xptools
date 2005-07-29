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
#ifndef CGALTYPES_H
#define CGALTYPES_H

THIS HEADER IS OBSOLETE

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/double.h>

//typedef	CGAL::Simple_cartesian<double>						Kernel;
//typedef	CGAL::Exact_predicates_exact_constructions_kernel	Kernel;

//typedef Kernel::Aff_transformation_2						Aff_transformation_2;
//typedef	Kernel::Bbox_2										Bbox_2;
typedef Kernel::Circle_2									Circle_2;
typedef Kernel::Direction_2									Direction_2;
typedef	Kernel::Iso_rectangle_2								Iso_rectangle_2;
typedef	Kernel::Line_2										Line_2;
typedef	Kernel::Point_2										Point_2;
typedef Kernel::Ray_2										Ray_2;
typedef Kernel::Segment_2									Segment_2;
typedef Kernel::Triangle_2									Triangle_2;
typedef Kernel::Vector_2									Vector_2;

typedef	CGAL::Simple_cartesian<double>						FastKernel;


class	_PM_Traits : public CGAL::Arr_segment_traits_2<Kernel> {
public:

 CGAL::Comparison_result curves_compare_y_at_x_right(const X_monotone_curve_2 & cv1,
                                                const X_monotone_curve_2 & cv2,
                                                const Point_2 & q) const
  {
    // The two curves must not be vertical.
    CGAL_precondition(! curve_is_vertical(cv1));
    CGAL_precondition(! curve_is_vertical(cv2));

    // The two curve must be defined at q and also to its right.
    CGAL_precondition_code(
        Construct_vertex_2 construct_vertex = construct_vertex_2_object();
	Less_x_2 less_x = less_x_2_object();
	const Point_2 & source1 = construct_vertex(cv1, 0);
	const Point_2 & target1 = construct_vertex(cv1, 1);
	const Point_2 & source2 = construct_vertex(cv2, 0);
	const Point_2 & target2 = construct_vertex(cv2, 1);
	);

    CGAL_precondition (less_x(q, source1) || less_x(q, target1));
    CGAL_precondition (!(less_x(q, source1) && less_x(q, target1)));
    
    CGAL_precondition (less_x(q, source2) || less_x(q, target2));
    CGAL_precondition (!(less_x(q, source2) && less_x(q, target2)));
    
    // Since the curves are continuous, if they are not equal at q, the same
    // result also applies to q's left.
    CGAL_precondition (curves_compare_y_at_x(cv1, cv2, q) == CGAL::EQUAL);     
    
    // <cv1> and <cv2> meet at a point with the same x-coordinate as q
    // compare their derivatives
    return compare_slope_2_object()(cv1, cv2);
  }
  
  CGAL::Comparison_result curves_compare_y_at_x_left(const X_monotone_curve_2 & cv1,
                                               const X_monotone_curve_2 & cv2, 
                                               const Point_2 & q) const 
  {
    // The two curves must not be vertical.
    CGAL_precondition(! curve_is_vertical(cv1));
    CGAL_precondition(! curve_is_vertical(cv2));

    // The two curve must be defined at q and also to its left.
    CGAL_precondition_code(
        Construct_vertex_2 construct_vertex = construct_vertex_2_object();
	Less_x_2 less_x = less_x_2_object();
	const Point_2 & source1 = construct_vertex(cv1, 0);
	const Point_2 & target1 = construct_vertex(cv1, 1);
	const Point_2 & source2 = construct_vertex(cv2, 0);
	const Point_2 & target2 = construct_vertex(cv2, 1);
	);

    CGAL_precondition (less_x(source1, q) || less_x(target1, q));
    CGAL_precondition (!(less_x(source1, q) && less_x(target1, q)));
    
    CGAL_precondition (less_x(source2, q) || less_x(target2, q));
    CGAL_precondition (!(less_x(source2, q) && less_x(target2, q)));
    
    // Since the curves are continuous, if they are not equal at q, the same
    // result also applies to q's left.
//    CGAL_precondition (compare_y_at_x_2_object()(q, cv1, cv2) == CGAL::EQUAL);
//  BAS sez: why are the left and right versions different?!?!  The above 
//	avoids my bottlenecks!
    CGAL_precondition (curves_compare_y_at_x(cv1, cv2, q) == CGAL::EQUAL);     
    
    // <cv2> and <cv1> meet at a point with the same x-coordinate as q
    // compare their derivatives.
    return compare_slope_2_object()(cv2, cv1);
  }  

  CGAL::Comparison_result curves_compare_y_at_x(const X_monotone_curve_2 & cv1, 
                                          const X_monotone_curve_2 & cv2, 
                                          const Point_2 & q) const
  {
  	// In the vertical case we return the relationship of their projections...not hard
  	// to special case but for now doesn't seem necessary.  One thing is clear: if they 
  	// don't intersect and we don't catch this case, we return EQUAL and we get hosed.  
  	// Not good. :-(  (In our planar map, two vertical lines NEVER intersect because..well...
  	// that'd be a hosed map.)
  	if (cv1.is_vertical() || cv2.is_vertical())
	    return CGAL::Arr_segment_traits_2<Kernel>::compare_y_at_x_2_object()(q, cv1, cv2);
  	bool	cv1s = compare_x(cv1.source(), q) == CGAL::EQUAL;
  	bool	cv2s = compare_x(cv2.source(), q) == CGAL::EQUAL;
  	bool	cv1t = compare_x(cv1.target(), q) == CGAL::EQUAL;
  	bool	cv2t = compare_x(cv2.target(), q) == CGAL::EQUAL;

	Point_2	p1, p2;
	bool	has_one = true, has_two = true;

	if (cv1s && cv1t)
	{
		if (compare_y(cv1.source(), cv1.target()) == CGAL::SMALLER)
			p1 = cv1.source();
		else
			p1 = cv1.target();
	} else if (cv1s) {
		p1 = cv1.source();
	} else if (cv1t) {
		p1 = cv1.target();
	} else {
		has_one = false;
	}
	
	if (cv2s && cv2t)
	{
		if (compare_y(cv2.source(), cv2.target()) == CGAL::SMALLER)
			p2 = cv2.source();
		else
			p2 = cv2.target();
	} else if (cv2s) {
		p2 = cv2.source();
	} else if (cv2t) {
		p2 = cv2.target();
	} else {
		has_two = false;
	}
	
	if (has_one && has_two)
	{
		return compare_y(p1, p2);
	} else {
	    return CGAL::Arr_segment_traits_2<Kernel>::compare_y_at_x_2_object()(q, cv1, cv2);
	}
  }
};


typedef	CGAL::Arr_segment_traits_2<Kernel>					PM_Traits;
typedef PM_Traits::X_monotone_curve_2						PM_Curve_2;

typedef	CGAL::Polygon_2<Kernel>								Polygon_2;

#endif
