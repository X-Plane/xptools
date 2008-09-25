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
#include "CompGeom.h"
#include "XESConstants.h"

I THINK THIS FILE IS OBSOLETE

bool	IsNearColinear(const Point_2& a, const Point_2& b, const Point_2& c)
{
	return IsNearColinear(Vector_2(a, b), Vector_2(b, c));
}

bool	IsNearColinear(const Direction_2& d1, const Direction_2& d2)
{
	return IsNearColinear(d1.to_vector(), d2.to_vector());
}

bool	IsNearColinear(const Vector_2& v1, const Vector_2& v2)
{
	Vector_2	v1n = v1 / sqrt(CGAL::to_double(Segment_2(CGAL::ORIGIN, CGAL::ORIGIN + v1).squared_length()));
	Vector_2	v2n = v2 / sqrt(CGAL::to_double(Segment_2(CGAL::ORIGIN, CGAL::ORIGIN + v2).squared_length()));

	return CGAL::to_double(v1n*v2n) > 0.90;
}

bool	IsOpposite(const Segment_2& a, const Segment_2& b)
{
	return IsOpposite(a.direction(), b.direction());
}

bool	IsOpposite(const Direction_2& a, const Direction_2& b)
{
	return ((-a) == b);
}

bool	IsNearParallel(const Segment_2& a, const Segment_2& b)
{
	return IsNearParallel(a.to_vector(), b.to_vector());
}

bool	IsNearParallel(const Direction_2& a, const Direction_2& b)
{
	return IsNearParallel(a.to_vector(), b.to_vector());
}

bool	IsNearParallel(const Vector_2& v1, const Vector_2& v2)
{
	Vector_2	v1n = v1 / sqrt(CGAL::to_double(Segment_2(CGAL::ORIGIN, CGAL::ORIGIN + v1).squared_length()));
	Vector_2	v2n = v2 / sqrt(CGAL::to_double(Segment_2(CGAL::ORIGIN, CGAL::ORIGIN + v2).squared_length()));

	return CGAL::to_double(v1n*v2n) > 0.99999;
}

inline double	cot(double theta) { return cos(theta) / sin(theta); }
inline double	csc(double theta) { return 1.0        / sin(theta); }

//#define to_double(x) (x)

static	double	CCW_Angle(const Point_2& p1, const Point_2& p2, const Point_2& p3)
{
	Direction_2	d1(Vector_2(p1, p2));
	Direction_2	d2(Vector_2(p2, p3));

	double	a1 = atan2(CGAL::to_double(d1.dy()), CGAL::to_double(d1.dx()));
	double	a2 = atan2(CGAL::to_double(d2.dy()), CGAL::to_double(d2.dx()));

	double a_turn = PI - (a2 - a1);

	if (a_turn > PI)
		a_turn -= PI2;

	return a_turn;
}

static	void	MoveSegLeft(const Segment_2& l1, double dist, Segment_2& l2)
{
	Vector_2 v = l1.to_vector().perpendicular(CGAL::COUNTERCLOCKWISE);

	double	dstv = sqrt(CGAL::to_double(v.x() * v.x() + v.y() * v.y()));

	Aff_transformation_2	transformer(CGAL::Translation(), v * (dist / dstv));
	l2 = l1.transform(transformer);
}

void	InsetPolygon(
				const Polygon_2&			inChain,
				const vector<double>		inRatios,
				double						inInset,
				bool						inIsRing,
				Polygon_2&					outChain)
{
	if (!outChain.is_empty())
		outChain.erase(outChain.vertices_begin(), outChain.vertices_end());

	int n = 0;
	vector<Segment_2>	segments, orig_segments;

	// First we calculate the inset edges of each side of the polygon.

	for (Polygon_2::Edge_const_iterator	edge = inChain.edges_begin();
		edge != inChain.edges_end(); ++edge, ++n)
	{
		Segment_2	seg;
		orig_segments.push_back(*edge);
		MoveSegLeft(*edge, inRatios[n] * inInset, seg);
		segments.push_back(seg);
	}

	// Now we go through and find each vertex, the intersection of the supporting
	// lines of the edges.  For the very first and last point if not in a polygon,
	// we don't use the intersection, we just find where that segment ends for a nice
	// crips 90 degree angle.

	int last_vertex = segments.size() - 1;
	for (int outgoing_n = 0; outgoing_n < segments.size(); ++outgoing_n)
	{
		// the Nth segment goes from the Nth vertex to the Nth + 1 vertex.
		// Therefore it is the "outgoing" segment.
		int incoming_n = outgoing_n - 1;
		if (incoming_n < 0)
			incoming_n = last_vertex;

		if (outgoing_n == 0 && !inIsRing)
		{
			// Special case 1: we're the first in a chain.  Outgoing vertex is always right.
			outChain.insert(outChain.vertices_end(), segments[outgoing_n].source());
		} else if (outgoing_n == last_vertex && !inIsRing)
		{
			// Special case 2: we're the last in a chain.  Incoming vertex is always right
			outChain.insert(outChain.vertices_end(), segments[incoming_n].target());
		} else 	if (IsOpposite(orig_segments[incoming_n], orig_segments[outgoing_n])) {
			// Special case 3: are the two sides in exactly opposite directions?  Special case...we have to add a vertex.
			// (This is almost always an "antenna" in the data, that's why we have to add the new side, the point of the antenna
			// becomes thick.  Since antennas have equal coordinates, an exact opposite test works.)
			Segment_2	new_side(segments[incoming_n].target(), segments[outgoing_n].source()), new_side2;
			MoveSegLeft(new_side, inRatios[outgoing_n] * inInset, new_side2);
			outChain.insert(outChain.vertices_end(), new_side2.source());
			outChain.insert(outChain.vertices_end(), new_side2.target());
		} else {
			// General case: intersect the supporting line of two segments.
			CGAL::Object o = intersection(segments[outgoing_n].supporting_line(),
									segments[incoming_n].supporting_line());

			// In the freak-case that they're colinear...well, these two segments
			// are coallesced, so just drop the vertex for now.  Also, we check for
			// "near-parallel" because if we run out of intersection precision, our intersect
			// point will end up off in Deep Space 9. :-(
			Point_2	pt;
			if (CGAL::assign(pt, o) && !IsNearParallel(orig_segments[incoming_n], orig_segments[outgoing_n]))
				outChain.insert(outChain.vertices_end(), pt);
		}
	}
}

double	CalcMaxInset(
				const Polygon_2&			inChain,
				const vector<double>		inRatios,
				bool						inIsRing)
{
	/*
		For any side of length L, with an inset of D, the left and right
		sides have angles A1 and A2, and insets D1 and D2, and an inset factor F

		We are ok as long as
		L > F*D/tan(A1) + F*D1/sin(A1) + F*D/tan(A2) + F*D2/sin(A2)
		So our max inset is:

		L/ ( D/tan(A1) + D1/sin(A1) + D/tan(A2) + D2/sin(A2) )
	*/

	int	sides = inChain.size();
	if (!inIsRing) sides--;
	double	best_f = 0.0;

	for (int n = 0; n < sides; ++n)
	{
		double	prev = n-1;
		if (prev < 0) prev = inChain.size() - 1;
		double next = n+1;
		if (next >= inChain.size()) next = 0;
		double nextnext = next+1;
		if (nextnext >= inChain.size()) nextnext = 0;

		double	D = inRatios[n];
		double 	D1 = inRatios[prev];
		double  D2 = inRatios[next];

		double	A1 = CCW_Angle(inChain[prev], inChain[n], inChain[next]);
		double	A2 = CCW_Angle(inChain[n], inChain[next], inChain[nextnext]);

		double	divisor = 0;
		if (inIsRing || n != 0)
			divisor += (D * cot(A1) + D1 * csc(A1));
		if (inIsRing || n != (sides - 1))
			divisor += (D * cot(A2) + D1 * csc(A2));

		if (divisor > 0.0)
		{
			double seglen = sqrt(CGAL::to_double(Segment_2(inChain[n], inChain[next]).squared_length()));
			double f = seglen / divisor;
			if (n == 0 || f < best_f)
				best_f = f;
		}
	}
	return best_f;
}

/* To implement this algorithm we need to do a few checks:
 * For our previous and next sides, we simply figure out how far
 * the vertex we don't share is from our supporting line, so we don't
 * consume those sides.
 *
 * For sides we don't touch, we need to see how they limit us.  They will
 * limit us in one of four places:
 * 1. Our source might be closest to them, limiting growth via the source side.
 * 2. Our target might be closest to them, limiting growth via the source side.
 * 3. Their source might be blocking us.
 * 4. Their target might be blocking us.
 *
 */

double	CalcMaxSideInset(
				const Polygon_2&			inPolygon,
				int							inSideIndex)
{
	Kernel::FT	dist, best;
	bool		has = false;

	Polygon_2::Edge_const_circulator	me = inPolygon.edges_circulator();
										me += inSideIndex;
	Polygon_2::Edge_const_circulator	prev = me;
										--prev;
	Polygon_2::Edge_const_circulator	next = me;
										++next;
	Line_2	support = (*me).supporting_line();
	Line_2	src_edge = support.perpendicular((*me).source());
	Line_2	trg_edge = support.perpendicular((*me).target());

	Polygon_2::Edge_const_circulator	stop = inPolygon.edges_circulator();
	Polygon_2::Edge_const_circulator	iter = stop;

	Point_2								proj;
	do {
		if (iter == prev && !IsNearColinear((*prev).source(), (*me).source(), (*me).target()))
		{
			// If the previous vertex forms a convex joint, limit our growth.
			if (support.oriented_side((*prev).source()) == CGAL::ON_POSITIVE_SIDE)
			{
				proj = support.projection((*prev).source());
				dist = Segment_2((*prev).source(), proj).squared_length();
				if (!has || dist < best)
					has = true, best = dist;
			}
		} else if (iter == next && !IsNearColinear((*me).source(), (*me).target(), (*next).target()))
		{
			// If the next vertex forms a concave joint, limit our growht.
			if (support.oriented_side((*next).target()) == CGAL::ON_POSITIVE_SIDE)
			{
				proj = support.projection((*next).target());
				dist = Segment_2((*next).target(), proj).squared_length();
				if (!has || dist < best)
					has = true, best = dist;
			}
		} else if (iter != me && iter != prev && iter != next)
		{
			Point_2	inter;
			CGAL::Object	obj;

			// Check how far we could extend before our source corner hits them.
			obj = CGAL::intersection(src_edge, *iter);
			if (CGAL::assign(inter, obj) && support.oriented_side(inter) == CGAL::ON_POSITIVE_SIDE)
			{
				dist = Segment_2(inter, (*me).source()).squared_length();
				if (!has || dist < best)
					has = true, best = dist;
			}
			// Check how far we could extend before our target corner hits them.
			obj = CGAL::intersection(trg_edge, *iter);
			if (CGAL::assign(inter, obj) && support.oriented_side(inter) == CGAL::ON_POSITIVE_SIDE)
			{
				dist = Segment_2(inter, (*me).target()).squared_length();
				if (!has || dist < best)
					has = true, best = dist;
			}
			// Check how far we can extend before their source hits us.
			inter = support.projection((*iter).source());
			if ((*me).collinear_has_on(inter) && support.oriented_side((*iter).source()) == CGAL::ON_POSITIVE_SIDE)
			{
				dist = Segment_2(inter, (*iter).source()).squared_length();
				if (!has || dist < best)
					has = true, best = dist;
			}
			// Check how far we can extend before their target hits us.
			inter = support.projection((*iter).target());
			if ((*me).collinear_has_on(inter) && support.oriented_side((*iter).target()) == CGAL::ON_POSITIVE_SIDE)
			{
				dist = Segment_2(inter, (*iter).target()).squared_length();
				if (!has || dist < best)
					has = true, best = dist;
			}
		}

		++iter;
	} while(iter != stop);
	return sqrt(CGAL::to_double(best));
}
