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

#include "Bezier.h"
#include <CGAL/Sweep_line_2_algorithms.h>

static void foo()
{
	Bezier_arrangement_2 arr;
	Bezier_polygon_set_2 gps;
	Bezier_polygon_2	 h;
	Bezier_polygon_with_holes_2	hh;
}

void	find_crossing_beziers(
			const vector<Bezier_curve_2>& in_curves,
			      vector<Bezier_point_2>& out_xons)
{
//	Bezier_polygon_2	p(in_curves.begin(), in_curves.end());
//	printf("Simple? %s\n", p.is_simple());

	out_xons.clear();
	Bezier_traits_2 traits;
	try {
		CGAL::compute_intersection_points(in_curves.begin(),in_curves.end(),back_inserter(out_xons), false, traits);
	} catch(...) {
		printf("Crossing beziers blew up.  Listing %d curves:\n", in_curves.size());
		for(int n = 0; n < in_curves.size(); ++n)
		{
			printf("  Curve %d has %d points:\n", n, in_curves[n].number_of_control_points());
			for(int i = 0; i < in_curves[n].number_of_control_points(); ++i)
				printf("     %d: %.12lf,%.12lf\n", i, CGAL::to_double(in_curves[n].control_point(i).x()),CGAL::to_double(in_curves[n].control_point(i).y()));
		}
		out_xons.clear();
	}
}

void	find_crossing_beziers(
					const vector<Bezier2>&	in_curves,
						  vector<Point2>&	out_xons)
{
	vector<Bezier_curve_2>	curves;
	vector<Bezier_point_2>		pts;
	
	for(vector<Bezier2>::const_iterator b = in_curves.begin(); b != in_curves.end(); ++b)
	{
		vector<Rat_point_2>	cp;
		bool is_seg = (b->p1 == b->c1 && b->p2 == b->c2);
						cp.push_back(Rat_point_2(b->p1.x(),b->p1.y()));
			if(!is_seg)	cp.push_back(Rat_point_2(b->c1.x(),b->c1.y()));
			if(!is_seg)	cp.push_back(Rat_point_2(b->c2.x(),b->c2.y()));
						cp.push_back(Rat_point_2(b->p2.x(),b->p2.y()));

		Bezier_curve_2	bc(cp.begin(),cp.end());
	
		curves.push_back(bc);
	}
	
	find_crossing_beziers(curves,pts);
	out_xons.clear();
	for(vector<Bezier_point_2>::iterator p = pts.begin(); p != pts.end(); ++p)
	{
		pair<double,double>	pp = p->approximate();
		out_xons.push_back(Point2(pp.first,pp.second));
	}
}

bool	do_beziers_cross(
					const vector<Bezier_curve_2>& in_curves)
{
	Bezier_traits_2 traits;
	try {
		return CGAL::do_curves_intersect(in_curves.begin(),in_curves.end(),traits);
	} catch(...) {
		return true;
	}
}
					
bool	do_beziers_cross(
					const vector<Bezier2>&	in_curves)
{
	vector<Bezier_curve_2>	curves;
	
	for(vector<Bezier2>::const_iterator b = in_curves.begin(); b != in_curves.end(); ++b)
	{
		vector<Rat_point_2>	cp;
		bool is_seg = (b->p1 == b->c1 && b->p2 == b->c2);
						cp.push_back(Rat_point_2(b->p1.x(),b->p1.y()));
			if(!is_seg)	cp.push_back(Rat_point_2(b->c1.x(),b->c1.y()));
			if(!is_seg)	cp.push_back(Rat_point_2(b->c2.x(),b->c2.y()));
						cp.push_back(Rat_point_2(b->p2.x(),b->p2.y()));

		Bezier_curve_2	bc(cp.begin(),cp.end());
	
		curves.push_back(bc);
	}
	return do_beziers_cross(curves);
}
