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
#if !defined(__i386__) && defined(IBM)
#define __i386__
#define __i386__defined 1
#endif
#include <CGAL/Sweep_line_2_algorithms.h>
#if __i386__defined
#undef __i386__
#undef __i386__defined
#endif
#include "AssertUtils.h"

inline Rat_point_2 ben2rat(const Point2& p) { return Rat_point_2(p.x(),p.y()); }
inline Point2 rat2ben(const Rat_point_2& p) { return Point2(p.x().doubleValue(),p.y().doubleValue()); }

#define	BEZ_HACK	0.001

Bezier_curve_2	ben2cgal(const Bezier2& b)
{
	Point2 hack_pt;
	vector<Rat_point_2>	cp;

	if(b.p1 == b.c1 && b.p2 == b.c2)
	{
		cp.push_back(ben2rat(b.p1));
		cp.push_back(ben2rat(b.p2));
	}
	else
	{
		cp.push_back(ben2rat(b.p1));

		if(b.p1 == b.c1)
		{
			hack_pt = Segment2(b.p1,b.c2).midpoint(BEZ_HACK);
			if(hack_pt != b.p1)
				cp.push_back(ben2rat(hack_pt));
		} else
			cp.push_back(ben2rat(b.c1));

		if(b.p2 == b.c2)
		{
			hack_pt = Segment2(b.p2,b.c1).midpoint(BEZ_HACK);
			if(hack_pt != b.p2)
				cp.push_back(ben2rat(hack_pt));
		} else
			cp.push_back(ben2rat(b.c2));

		cp.push_back(ben2rat(b.p2));
	}


	return Bezier_curve_2(cp.begin(),cp.end());
}

Bezier2 cgal2ben(const Bezier_curve_2& b)
{
	// We should not have 3 control points - that would be a quadratic bezier, which we do not support. Since we never put one in, we never get one out.
	Bezier2 ret;
	int n = b.number_of_control_points();
	DebugAssert(n == 1 || n == 2 || n == 4);
	switch(n) {
	case 1:
		ret.p1 = ret.p2 = ret.c1 = ret.c2 = rat2ben(b.control_point(0));			// Degenerate line in floating point
		break;
	case 2:
		ret.p1 = ret.c1 = rat2ben(b.control_point(0));								// segment in floating point
		ret.c2 = ret.p2 = rat2ben(b.control_point(1));
		break;
	case 4:
		ret.p1 = rat2ben(b.control_point(0));
		ret.c1 = rat2ben(b.control_point(1));
		ret.c2 = rat2ben(b.control_point(2));
		ret.p2 = rat2ben(b.control_point(3));
		break;
	}
	return ret;
}

Bezier2 cgal2ben(const Bezier_polygon_2::X_monotone_curve_2& e)
{
	// This isn't entirely obvious: first...CGAL stores x-monotone curves as their supporting original bezier and the time-range that we selected.
	// Second, the time "t" where interesting things happen are algebraic, not rational.  (This is because the roots of a polynomial may not be
	// rational if the degree is higher than one.)  So...we can't EXACTLY get the time "t" when things happen.)

	// So our conversion process is to get the supporting curve (which, since it was made by us, does have rational control points), convert that,
	// and then take our sub-range in floating point.

	// There is a problem with that, too: we want adjacent curevs in a general polygon to share an end point, but the approximations we use mean that
	// might not be true.  To work around this, we use the approximation of the "source" and "target" accessors, which produce consistent results
	// across curves.  That is: because the source and target are shared, their approximations are always equal.

	// Finally, we have one special case: if we detect that the underlying polygon is a linear (only end points) we go back and "fix" c1/c2 to be exactly
	// the same as the end points - with the amount of rounding we've done, it is unlikely that this will turn out to be true on its own.

	Bezier2	sub;

	Bezier_curve_2 support = e.supporting_curve();									// Convert supporting
	pair<double,double>	range = e.parameter_range();
	Bezier2 support_ben = cgal2ben(support);
	support_ben.subcurve(sub,range.first,range.second);								// Find sub-curve in floating point

	if(support_ben.p1 == support_ben.c1 && support_ben.p2 == support_ben.c2)		// For 'segment' supporting curve, force segment sub-curve
	{
		sub.p1.x_ = e.source().approximate().first;										// Rebuild end points from src/dst.  sub-interp is innacurate for linear case.
		sub.p1.y_ = e.source().approximate().second;
		sub.p2.x_ = e.target().approximate().first;
		sub.p2.y_ = e.target().approximate().second;

		sub.c1 = sub.p1;
		sub.c2 = sub.p2;
	}

	return sub;
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
		printf("Crossing beziers blew up.  Listing %zd curves:\n", in_curves.size());
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

	out_xons.clear();
	for(vector<Bezier2>::const_iterator b = in_curves.begin(); b != in_curves.end(); ++b)
	{
		if(b->p1 != b->p2 && (b->p1 != b->c1 || b->p2 != b->c2) && (b->c1 == b->p2 || b->c2 == b->p1))
		{
			out_xons.push_back(b->c1);
		}
		else if(b->p1 != b->c1 ||
		   b->c1 != b->c2 ||
		   b->c2 != b->p2)
			curves.push_back(ben2cgal(*b));
	}
	find_crossing_beziers(curves,pts);
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
		curves.push_back(ben2cgal(*b));
	}
	return do_beziers_cross(curves);
}




void	clip_bezier_polygon(
					const Bezier_polygon_with_holes_2&		in_poly,
					vector<Bezier_polygon_with_holes_2>&	out_remaining,
					const Bbox_2&							clip_bounds)
{
	out_remaining.clear();
	out_remaining.push_back(in_poly);
}

void	clip_bezier_chain(
					const vector<Bezier_curve_2>&			in_curves,
					vector<Bezier_curve_2>&					out_curves,
					const Bbox_2&							clip_bounds)
{
	out_curves = in_curves;
}

