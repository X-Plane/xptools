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
#ifndef COMPGEOMDEFS2_H
#define COMPGEOMDEFS2_H

// TODO: add fully parametric 2-d line intersection - this is useful
// for segment crossing because we get an interval in the form of 't'.
// Hrm - would this really improve our intersection quality?

#include <math.h>
#include <vector>
using std::vector;

struct	Point2;
struct	Vector2;

/****************************************************************************************************
 * Point2
 ****************************************************************************************************/

struct Point2 {
	Point2() : x(0.0), y(0.0) { }
	Point2(double ix, double iy) : x(ix), y(iy) { }
	Point2(const Point2& rhs) : x(rhs.x), y(rhs.y) { }

	Point2& operator=(const Point2& rhs) { x = rhs.x; y = rhs.y; return *this; }
	bool operator==(const Point2& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const Point2& rhs) const { return x != rhs.x || y != rhs.y; }
	
	Point2& operator += (const Vector2& v);
	Point2& operator -= (const Vector2& v);
	Point2 operator+(const Vector2& v) const;
	Point2 operator-(const Vector2& v) const;
	
	inline double squared_distance(const Point2& p) const { return (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y); }
	
	double	x;
	double	y;
};

/****************************************************************************************************
 * Vector2
 ****************************************************************************************************/

struct	Vector2 {
	Vector2() : dx(0.0), dy(0.0) { }
	Vector2(double ix, double iy) : dx(ix), dy(iy) { }
	Vector2(const Vector2& rhs) : dx(rhs.dx), dy(rhs.dy) { }
	explicit Vector2(const Point2& rhs) : dx(rhs.x), dy(rhs.y) { }
	Vector2(const Point2& p1, const Point2& p2) : dx(p2.x - p1.x), dy(p2.y - p1.y) { }
	
	Vector2& operator=(const Vector2& rhs) { dx = rhs.dx; dy = rhs.dy; return *this; }
	bool operator==(const Vector2& rhs) const { return dx == rhs.dx && dy == rhs.dy; }
	bool operator!=(const Vector2& rhs) const { return dx != rhs.dx || dy != rhs.dy; }

	Vector2& operator += (const Vector2& rhs) { dx += rhs.dx; dy += rhs.dy; return *this; }
	Vector2& operator -= (const Vector2& rhs) { dx -= rhs.dx; dy -= rhs.dy; return *this; }
	Vector2& operator *= (double scalar) { dx *= scalar; dy *= scalar; return *this; }
	Vector2& operator/= (double scalar) { dx /= scalar; dy /= scalar; return *this; }
	Vector2	 operator+ (const Vector2& v) const { return Vector2(dx + v.dx, dy + v.dy); }
	Vector2	 operator- (const Vector2& v) const { return Vector2(dx - v.dx, dy - v.dy); }
	Vector2  operator* (double scalar) const { return Vector2(dx * scalar, dy * scalar); }
	Vector2  operator/ (double scalar) const { return Vector2(dx / scalar, dy / scalar); }
	Vector2	 operator- (void) const { return Vector2(-dx, -dy); }
	
	double	squared_length(void) const { return dx * dx + dy * dy; }
	void 	normalize(void) { if (dx == 0.0) dy = (dy >= 0.0) ? 1.0 : -1.0; else if (dy == 0.0) dx = (dx >= 0.0) ? 1.0 : -1.0; else { double len = sqrt(dx * dx + dy * dy); if (len != 0.0) { len = 1.0 / len; dx *= len; dy *= len; } } }
	Vector2 perpendicular_cw() { return Vector2(dy, -dx); }
	Vector2 perpendicular_ccw() { return Vector2(-dy, dx); }

	double	dot(const Vector2& v) const { return dx * v.dx + dy * v.dy; }
	bool	left_turn(const Vector2& v) const { return (-dy * v.dx + dx * v.dy) > 0.0; }
	bool	right_turn(const Vector2& v) const { return (-dy * v.dx + dx * v.dy) < 0.0; }
	bool	no_turn(const Vector2& v) const { return (-dy * v.dx + dx * v.dy) == 0.0; }
	
	// Signed area = the area of the triangle formed by placing V after this.  
	// If they form a counter-clockwise triangle, the area is positive, clockwise = negative
	// Why does this work?  Well this is really half the dot product of this vector and the next
	// one rotated right 90 degrees.  What we really want is half base height sin theta - the 
	// rotate right lets us write the thing in terms of cosine - the dot product then gives us
	// the quanty len v1 * len v2 * cos theta.
	double	signed_area(const Vector2& v) const { return (dx * v.dy - dy * v.dx) * 0.5; }

	// Vector projection of other onto US!
	Vector2	projection(const Vector2& other) const;

	double	dx;
	double	dy;
};	

/****************************************************************************************************
 * Segment2
 ****************************************************************************************************/

struct	Segment2 {	
	Segment2() : p1(), p2() { }
	Segment2(const Point2& ip1, const Point2& ip2) : p1(ip1), p2(ip2) { }
	Segment2(const Point2& p, const Vector2& v) : p1(p), p2(p.x + v.dx, p.y + v.dy) { }
	Segment2(const Segment2& rhs) : p1(rhs.p1), p2(rhs.p2) { }

	Segment2& operator=(const Segment2& rhs) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	bool operator==(const Segment2& rhs) const { return p1 == rhs.p1 && p2 == rhs.p2; }
	bool operator!=(const Segment2& rhs) const { return p1 != rhs.p1 || p2 != rhs.p2; }

	Segment2& operator+=(const Vector2& rhs) { p1 += rhs; p2 += rhs; return *this; }

	double	squared_length(void) const { return (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y); }
	Point2	midpoint(double s=0.5) const { if (s==0.0) return p1; if (s==1.0) return p2; double ms = 1.0 - s; return Point2(p1.x * ms + p2.x * s,p1.y * ms + p2.y * s); }
	Point2	projection(const Point2& pt) const;
	double	squared_distance(const Point2& p) const;
	bool	collinear_has_on(const Point2& p) const;
	bool	on_left_side(const Point2& p) const { return Vector2(p1, p2).left_turn(Vector2(p1, p)); }
	bool	on_right_side(const Point2& p) const { return Vector2(p1, p2).right_turn(Vector2(p1, p)); }
	bool	collinear(const Point2& p) const { return Vector2(p1, p2).no_turn(Vector2(p1, p)); }
	
	
	bool	could_intersect(const Segment2& rhs) const;
	bool	intersect(const Segment2& rhs, Point2& p) const;
	
	double	y_at_x(double x) const { 	if (p1.x == p2.x) 	return p1.y; 
										if (x == p1.x) 		return p1.y; 
										if (x == p2.x) 		return p2.y; 
										return p1.y + (p2.y - p1.y) * (x - p1.x) / (p2.x - p1.x); }
	double	x_at_y(double y) const { 	if (p1.y == p2.y) 	return p1.x; 
										if (y == p1.y) 		return p1.x; 
										if (y == p2.y) 		return p2.x; 
										return p1.x + (p2.x - p1.x) * (y - p1.y) / (p2.y - p1.y); }
										
	bool	is_vertical(void) const { return p1.x == p2.x; }
	bool	is_horizontal(void) const { return p1.y == p2.y; }
	bool	is_empty(void) const { return p1 == p2; }


	Point2	p1;
	Point2	p2;
};

/****************************************************************************************************
 * Line2
 ****************************************************************************************************/


/*
 * Line2
 *
 * This is a line in the form Ax + By + C = 0.  You can think of the vector A, B being the normal to the line.
 * Technically this line has a 'direction' since the vector A,B faces to one side.  In the vector constructor
 * this alignment is preserved.
 *
 * (The derivation of this is similar to the definition of a plane...the line is the set of points where
 * the vector from P0 to P dot N (the normal) is 0.  P0 is a point on the line and N the normal.
 *
 */
struct	Line2 {
	Line2() : a(0.0), b(1.0), c(0.) { }
	Line2(const Point2& p1, const Point2& p2) : a(p1.y - p2.y), b(p2.x - p1.x), c((p2.y - p1.y) * p1.x - (p2.x - p1.x) * p1.y) { }
	Line2(const Point2& p, const Vector2& v) : a(-v.dy), b(v.dx), c(v.dy * p.x - v.dx * p.y) { }
	Line2(const Line2& l) : a(l.a), b(l.b), c(l.c) { }
	explicit Line2(const Segment2& s) : a(s.p1.y - s.p2.y), b(s.p2.x - s.p1.x), c((s.p2.y - s.p1.y) * s.p1.x - (s.p2.x - s.p1.x) * s.p1.y) { }
	
	Line2& operator=(const Line2& rhs) { a = rhs.a; b = rhs.b; c = rhs.c; return *this; }
	bool operator==(const Line2& rhs) const { return (a * rhs.b == rhs.a * b) && (a * rhs.c == rhs.a * c) && (b * rhs.c == rhs.b * c); }
	bool operator!=(const Line2& rhs) const { return (a * rhs.b != rhs.a * b) || (a * rhs.c != rhs.a * c) || (b * rhs.c != rhs.b * c); }
	
	bool intersect(const Line2& l, Point2& p) const;
	double squared_distance(const Point2& p) const;
	
	bool	on_left_side(const Point2& p) const { return (a * p.x + b * p.y + c) > 0; }
	bool	on_right_side(const Point2& p) const { return (a * p.x + b * p.y + c) < 0; }
	bool	collinear(const Point2& p) const { return (a * p.x + b * p.y + c) == 0; }
	
	double	a;
	double	b;
	double	c;
};

/****************************************************************************************************
 * Bbox2
 ****************************************************************************************************/

class	Bbox2 {
public:
				Bbox2() : p1(0,0), p2(0, 0) { }
				Bbox2(double x1, double y1, double x2, double y2) : p1(x1, y1), p2(x2, y2) { if (p1.x > p2.x) swap(p1.x, p2.x); if (p1.y > p2.y) swap(p1.y, p2.y); }
				Bbox2(const Point2& in_p1, const Point2& in_p2) : p1(in_p1), p2(in_p2) { if (p1.x > p2.x) swap(p1.x, p2.x); if (p1.y > p2.y) swap(p1.y, p2.y); }
				Bbox2(const Point2& inp) : p1(inp), p2(inp) { }
				Bbox2(const Bbox2& rhs) : p1(rhs.p1), p2(rhs.p2) { }
	Bbox2&		operator=(const Bbox2& rhs) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	Bbox2&		operator=(const Point2& rhs) { p1 = rhs; p2 = rhs; return *this; }
	bool		operator==(const Bbox2& rhs) const { return p1 == rhs.p1 && p2 == rhs.p2; }
	bool		operator!=(const Bbox2& rhs) const { return p1 != rhs.p1 || p2 != rhs.p2; }
	
	Bbox2&		operator+=(const Point2& p) { 	p1.x = min(p1.x, p.x);	p1.y = min(p1.y, p.y); 
												p2.x = max(p2.x, p.x);	p2.y = max(p2.y, p.y); return *this; }

	double		xmin() const { return p1.x; }
	double		ymin() const { return p1.y; }
	double		xmax() const { return p2.x; }
	double		ymax() const { return p2.y; }

	bool		empty() { return p1 == p2; }
	
	bool		overlap(const Bbox2& rhs) const {	
	    return 	(xmax() >= rhs.xmin() && rhs.xmax() >= xmin() &&
				 ymax() >= rhs.ymin() && rhs.ymax() >= ymin()); 			}

	bool		contains(const Bbox2& rhs) const {	
	    return 	(xmax() >= rhs.xmax() && rhs.xmin() >= xmin() &&
				 ymax() >= rhs.ymax() && rhs.ymin() >= ymin()); 			}
				 
	bool		contains(const Point2& p) const {
		return (xmin() <= p.x && p.x <= xmax() &&
				ymin() <= p.y && p.y <= ymax()); }

	void		expand(double v) { p1.x -= v; p1.y -= v; p2.x += v; p2.y += v; }

	Point2	p1;
	Point2	p2;

};

/****************************************************************************************************
 * Polygon2
 ****************************************************************************************************/

class	Polygon2 : public vector<Point2> {
public:
				Polygon2() 						: vector<Point2>() 		{ }
				Polygon2(const Polygon2& rhs)   : vector<Point2>(rhs) 	{ }
				Polygon2(int x) 				: vector<Point2>(x) 	{ }
	
	// This returns side N, where side 0 goes from point 0 to point 1.
	Segment2	side(int n) const { if (n == (size()-1)) return Segment2(at(n),at(0)); return Segment2(at(n), at(n+1)); }
	
	Point2		centroid(void) const;
	
	// This computes the signed area of the polygon, meaning positive for a counter-clockwise polygon, 
	// negative for a clockwise one.  Works with convex and concave simple polyogns.
	double		area(void) const;
	
	// Calculate bounds
	Bbox2		bounds(void) const;
	
	// This returns the index number of the longest side.
	int			longest_side(void) const;
	
	// Returns true if the polygon is convex.  NOTE: works for CCW polygons.
	bool		convex(void) const;
	
	// Returns true if the point is inside/outside the polygon.  NOTE: works only for convex CCW polygons.
	bool		inside_convex(const Point2& inPoint) const;
	
	// Returns true if the point is inside the polygon.  Works on any polygon.
	bool		inside(const Point2& inPoint) const;
	
	int			prev(int index) const { return (index + size() - 1) % size(); }
	int			next(int index) const { return (index + 1) % size(); }
	
};

/****************************************************************************************************
 * Bezier2
 ****************************************************************************************************/

class	Bezier2 {
public:
	Bezier2() { }
	Bezier2(const Point2& ip1, const Point2& ic1, const Point2& ic2, const Point2& ip2) : p1(ip1), p2(ip2), c1(ic1), c2(ic2) { }
	Bezier2(const Bezier2& x) : p1(x.p1), p2(x.p2), c1(x.c1), c2(x.c2) { }
	Bezier2& operator=(const Bezier2& x) { p1 = x.p1; p2 = x.p2; c1 = x.c1; c2 = x.c2; return *this; }
	
	bool operator==(const Bezier2& x) const { return p1 == x.p1 && p2 == x.p2 && c1 == x.c1 && c2 == x.c2; }
	bool operator!=(const Bezier2& x) const { return p1 != x.p1 || p2 != x.p2 || c1 != x.c1 || c2 != x.c2; }
	
	Point2	midpoint(double t=0.5) const;	
	
	Point2	p1;
	Point2	p2;
	Point2	c1;
	Point2	c2;
};

/****************************************************************************************************
 * Comparison
 ****************************************************************************************************/

struct lesser_y_then_x {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.y == rhs.y) ? (lhs.x < rhs.x) : (lhs.y < rhs.y);
	}
};

struct greater_y_then_x {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.y == rhs.y) ? (lhs.x > rhs.x) : (lhs.y > rhs.y);
	}
};

struct lesser_x_then_y {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.x == rhs.x) ? (lhs.y < rhs.y) : (lhs.x < rhs.x);
	}
};

struct greater_x_then_y {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.x == rhs.x) ? (lhs.y > rhs.y) : (lhs.x > rhs.x);
	}
};

/****************************************************************************************************
 * Inline Funcs
 ****************************************************************************************************/

// These must be defined below because Point2 is declared before Vector2.
inline Point2& Point2::operator += (const Vector2& v) { x += v.dx; y += v.dy; return *this; }
inline Point2& Point2::operator -= (const Vector2& v) { x -= v.dx; y -= v.dy; return *this; }
inline Point2 Point2::operator+(const Vector2& v) const { return Point2(x + v.dx, y + v.dy); }
inline Point2 Point2::operator-(const Vector2& v) const { return Point2(x - v.dx, y - v.dy); }

inline Vector2	Vector2::projection(const Vector2& rhs) const 
{
//	http://home.xnet.com/~fidler/triton/math/review/mat135/vector/dot/apps/proj-1.htm
	// The scalar projection (how much shorter we need to be) for rhs is
	// len(rhs) * cos(theta) where theta is angle between us and them.

	// the actual projection of other onto us is the scalar projection * us / len(us)
	// e..g scalar projection * our direction
	
	// dot product is cos theta * len(rhs) * len(us), so whwat we want is really
	// us dot rhs / len(us) * us / len(us)
	// in other words (us dot rhs) * us / (len(us))^2
	//
	// hrm len(us) = sqrt(dx*dx+dy*dy) so len(us) sqr = us dot us...who knew?!?
	
	// Put it all together, scalar proj = (us dot rhs) * us / us dot us
	return (*this) * dot(rhs) / dot(*this);	
}

inline Point2	Segment2::projection(const Point2& pt) const
{
	// Simply find the projection of a vector from p1 to pt along a vector that is
	// our length.  Offset from p1, go home happy.
	return p1 + Vector2(p1,p2).projection(Vector2(p1,pt));
}

inline double	Segment2::squared_distance(const Point2& p) const 
{
	// TODO - would we get more efficiency from (1) calculating the line L that supports P and then
	// (2) appling the line distance formula?
	return Segment2(p, projection(p)).squared_length(); 
}

// Determines if this point is within the scope of the segment.
// We assume colinear-ness...really it checks to see if we're 
// within both of two half planes, normal to our segment's end points.
inline bool	Segment2::collinear_has_on(const Point2& p) const
{
	if (is_horizontal()) return (p.x >= p1.x && p.x <= p2.x) || (p.x >= p2.x && p.x <= p1.x);
	if (is_vertical())   return (p.y >= p1.y && p.y <= p2.y) || (p.y >= p2.y && p.y <= p1.y);
	if (Vector2(p1, p2).dot(Vector2(p1, p)) < 0.0) return false;
	if (Vector2(p2, p1).dot(Vector2(p2, p)) < 0.0) return false;
	return true;
}

inline bool Segment2::could_intersect(const Segment2& rhs) const
{
	// This is basically a bounding-box quick check for segment-intersection.
	register double	xmin1 = (p1.x < p2.x) ? p1.x : p2.x;
	register double	xmax1 = (p1.x > p2.x) ? p1.x : p2.x;
	register double	ymin1 = (p1.y < p2.y) ? p1.y : p2.y;
	register double	ymax1 = (p1.y > p2.y) ? p1.y : p2.y;

	register double	xmin2 = (rhs.p1.x < rhs.p2.x) ? rhs.p1.x : rhs.p2.x;
	register double	xmax2 = (rhs.p1.x > rhs.p2.x) ? rhs.p1.x : rhs.p2.x;
	register double	ymin2 = (rhs.p1.y < rhs.p2.y) ? rhs.p1.y : rhs.p2.y;
	register double	ymax2 = (rhs.p1.y > rhs.p2.y) ? rhs.p1.y : rhs.p2.y;
	
	return (xmax1 >= xmin2 &&
			xmax2 >= xmin1 &&
			ymax1 >= ymin2 &&
			ymax2 >= ymin1);
}

inline bool	Segment2::intersect(const Segment2& rhs, Point2& p) const
{
	// Interest uses Line2 for the general case and then colinear-has-on to see
	// if we're off the end of the "caps".  But first we special-case all horizontal
	// and vertical cases to get "perfect" intersections.
	if (rhs.p1 == p1 && rhs.p2 == p2) return false;
	if (rhs.p1 == p2 && rhs.p2 == p1) return false;
	if (rhs.p1 == p1 || rhs.p2 == p1) { p = p1; return true; }
	if (rhs.p1 == p2 || rhs.p2 == p2) { p = p2; return true; }
	
	if (is_horizontal())
	{
		if (rhs.is_horizontal())
		{
			return false;
		} 
		else if (rhs.is_vertical())
		{
			p.x = rhs.p1.x;
			p.y = p1.y;
		} 
		else 
		{
			p.x = rhs.x_at_y(p1.y);
			p.y = p1.y;
		}
	} 
	else if (is_vertical())
	{
		if (rhs.is_horizontal())
		{
			p.x = p1.x;
			p.y = rhs.p1.y;
		} 
		else if (rhs.is_vertical())
		{
			return false;
		} 
		else 
		{
			p.x = p1.x;
			p.y = rhs.y_at_x(p1.x);
		}
	} 
	else 
	{
		if (rhs.is_horizontal())
		{
			p.x = x_at_y(rhs.p1.y);
			p.y = rhs.p1.y;
		} 
		else if (rhs.is_vertical())
		{
			p.x = rhs.p1.x;
			p.y = y_at_x(rhs.p1.x);
		} 
		else 
		{
			if (!Line2(*this).intersect(Line2(rhs), p)) return false;
		}
	}
	return collinear_has_on(p) && rhs.collinear_has_on(p);
}

inline bool Line2::intersect(const Line2& l, Point2& p) const
{
	// Once we have lines in ax + by + c = 0 form, the parallel-equation solution
	// is pretty straight-forward to derive.
	// This does not tell us intervals though.
	
	if ((a * l.b) == (b * l.a))	return false;	// This means the lines are parallel.
	p.x = (b * l.c - l.b * c) / (l.b * a - b * l.a);
	p.y = (a * l.c - l.a * c) / (l.a * b - a * l.b);
	return true;
}

inline 	double Line2::squared_distance(const Point2& p) const
{
	// http://www.newton.dep.anl.gov/newton/askasci/1995/math/MATH061.HTM
	// distance is abs(ax +by +c) / sqrt(a^2 + b^2).
	// (to be technical, (ax + by + c) / sqrt(a^2 + b^2) yields a positive distance
	// when the point is on the same side of the line as its "normal" (that is, the 
	// vector a,b).  
	// Why does this work?  Well, you can think of ax + by + c as a formula for the
	// distance of a point P from the line in "units" of the normal vector len.
	// (This is why the line eq sets this to 0, or "on the line", and if a and b 
	// are normalized, sqrt(a^2+b^2) == 1).
	double num = a * p.x + b * p.y + c;
	return num * num / (a * a + b * b);
}


inline	Point2	Midpoint2(const Point2& p1, const Point2& p2)	{ return Point2((p1.x + p2.x) * 0.5, (p1.y + p2.y) * 0.5); }

inline	double	Polygon2::area(void) const
{
	// Area is just the sum of all triangular areas - concave areas turn out negative and cancel out.
	double s = 0.0;
	for (int n = 2; n < size(); ++n)
	{
		s += Vector2(at(0), at(n-1)).signed_area(Vector2(at(0), at(n)));
	}
	return s;
}

inline Bbox2 Polygon2::bounds(void) const
{
	if (empty()) return Bbox2();
	
	Bbox2 b((*this)[0]);
	for (int n = 1; n < this->size(); ++n)
		b += ((*this)[n]);
	return b;
}

inline int  Polygon2::longest_side(void) const
{
	// Linear time search for longest side.
	if (size() < 2) return 0;
	Vector2 v(at(0), at(1));	
	double	best_len = v.dx * v.dx + v.dy * v.dy;
	int		best_index = 0;
	for (int n = 2; n < size(); ++n)
	{
		v = Vector2(at(n-1), at(n));
		double l = v.dx * v.dx + v.dy * v.dy;
		if (l > best_len)
		{
			best_len = l;
			best_index = n - 1;
		}
	}
	return best_index;
}

inline Point2		Polygon2::centroid(void) const
{
	// Centroid is calculated by averaging.
	if (empty()) return Point2();
	double	x = 0.0, y = 0.0;
	for (int n = 0; n < size(); ++n)
	{
		x += at(n).x;
		y += at(n).y;
	}
	return Point2(x / (float) size(), y / (float) size());
}

inline bool Polygon2::convex(void) const
{
	// Convex-test - a 'normal' to the side of a polygon facing into the polygon is
	// handy because a positive dot product between this and another vector tells us that
	// that other vector is pointing into the interior side of the halfline.  For our
	// purposes this means that the vector is making a left turn.  
	int sz = size();
	if (sz < 3) return false;
	for (int n = 0; n < sz; ++n)
	{
		Vector2	s1(at(n       ), at((n+1) % sz));
		Vector2	s2(at((n+1)%sz), at((n+2) % sz));
		if (s1.perpendicular_ccw().dot(s2) < 0.0)
			return false;
	}
	return true;
}

// Inside/outside convex - does a normal test.  The advantage: if you're going
// to FAIL this test you get an early exit.  Disadvantage, requires convex-ness.
inline bool		Polygon2::inside_convex(const Point2& inPoint) const
{
	int sz = size();
	if (sz < 3) return false;
	for (int n = 0; n < sz; ++n)
	{
		Vector2	s1(at(n       ), at((n+1) % sz));
		Vector2	s2(at(n), inPoint);
		if (s1.perpendicular_ccw().dot(s2) <= 0.0)
			return false;		
	}
	return true;
}

// This is a classic "ray-shoot" test...shoot north - if we cross
// an odd bunch o' sides, we're inside.
inline bool		Polygon2::inside(const Point2& inPoint) const 
{
	int cross_counter = 0;
	for (int n = 0; n < size(); ++n)
	{
		// How do we solve the problem of edges?  It turns out the easiest solution is to
		// treat all "on-the-line" ray hits (e.g. the end of the segment is at the horizontal
		// of what we are shooting) as forced to one side.
		// In our case, we are shooting a vertical ray, so points on the line are treated as
		// being to the right.  That means that a line ending on the line is only counted if
		// its other point is the left of the line.
		
		Segment2 s(side(n));
		if ((s.p1.x < inPoint.x && inPoint.x < s.p2.x) ||
			(s.p2.x < inPoint.x && inPoint.x < s.p1.x) ||
			(s.p1.x == inPoint.x && s.p1.x < inPoint.x) ||
			(s.p2.x == inPoint.x && s.p2.x < inPoint.x))
		if (inPoint.y > s.y_at_x(inPoint.x))
			++cross_counter;
	}

	return (cross_counter % 2) == 1;
}

inline	Point2	Bezier2::midpoint(double t) const
{
	// A bezier curve is just a cubic interpolation
	// between four points weighted via a cub equation, 
	// hence A(1-t)^3 + 3B(1-t)^2t + 3C(1-t)t^2 + Dt^3
	double nt = 1.0 - t;
	double w0 = nt * nt * nt;
	double w1 = 3.0 * nt * nt * t;
	double w2 = 3.0 * nt * t * t;
	double w3 = t * t * t;
	return Point2(w0 * p1.x + w1 * c1.x + w2 * c2.x + w3 * p2.x,
				  w0 * p1.y + w1 * c1.y + w2 * c2.y + w3 * p2.y);
}


void	TEST_CompGeomDefs2(void);

#endif
