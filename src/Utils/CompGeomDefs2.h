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

	


enum {
	LEFT_TURN = 1,
	COLLINEAR = 0,
	RIGHT_TURN = -1,
	
	CLOCKWISE = -1,
	COUNTERCLOCKWISE = 1,
	
	ON_UNBOUNDED_SIDE = -1,
	ON_BOUNDARY = 0,
	ON_BOUNDED_SIDE = 1

};


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
	int		turn_direction(const Vector2& v) const { double d = -dy * v.dx + dx * v.dy; if (d > 0.0) return LEFT_TURN; if (d < 0.0) return RIGHT_TURN; return COLLINEAR; }
	
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
	Point2	midpoint(double s=0.5) const { if (s==0.0) return p1; if (s==1.0) return p2; double ms = 1.0 - s; return Point2(
													p1.x == p2.x ? p1.x : p1.x * ms + p2.x * s,
													p1.y == p2.y ? p1.y : p1.y * ms + p2.y * s); }
	Point2	projection(const Point2& pt) const;
//	double	squared_distance(const Point2& p) const;
	bool	collinear_has_on(const Point2& p) const;
	bool	on_left_side(const Point2& p) const { return Vector2(p1, p2).left_turn(Vector2(p1, p)); }
	bool	on_right_side(const Point2& p) const { return Vector2(p1, p2).right_turn(Vector2(p1, p)); }
	bool	collinear(const Point2& p) const { return Vector2(p1, p2).no_turn(Vector2(p1, p)); }
	int		side_of_line(const Point2& p) const { return Vector2(p1, p2).turn_direction(Vector2(p1, p));  }
	
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
	Line2(double ia, double ib, double ic) : a(ia), b(ib), c(ic) { }
	explicit Line2(const Segment2& s) : a(s.p1.y - s.p2.y), b(s.p2.x - s.p1.x), c((s.p2.y - s.p1.y) * s.p1.x - (s.p2.x - s.p1.x) * s.p1.y) { }
	
	Line2& operator=(const Line2& rhs) { a = rhs.a; b = rhs.b; c = rhs.c; return *this; }
	bool operator==(const Line2& rhs) const { return (a * rhs.b == rhs.a * b) && (a * rhs.c == rhs.a * c) && (b * rhs.c == rhs.b * c); }
	bool operator!=(const Line2& rhs) const { return (a * rhs.b != rhs.a * b) || (a * rhs.c != rhs.a * c) || (b * rhs.c != rhs.b * c); }
	
	bool intersect(const Line2& l, Point2& p) const;
	double squared_distance(const Point2& p) const;
	double distance_denormaled(const Point2& p) const;
	void normalize();	
	
	bool	on_left_side(const Point2& p) const { return (a * p.x + b * p.y + c) > 0; }
	bool	on_right_side(const Point2& p) const { return (a * p.x + b * p.y + c) < 0; }
	bool	collinear(const Point2& p) const { return (a * p.x + b * p.y + c) == 0; }
	int		side_of_line(const Point2& p) const { double v = (a * p.x + b * p.y + c); if (v > 0.0) return LEFT_TURN; if (v < 0.0) return RIGHT_TURN; return COLLINEAR; }
	
	double	a;
	double	b;
	double	c;
};

/****************************************************************************************************
 * Bbox2
 ****************************************************************************************************/

/*
	Bbox2 - this is a bounding box, used for a lot of spatial indexing.  A few notes:
	An EMPTY bbox has either an equal lower/upper side of equal left/rgiht side.  In otherwords, it CONTAINS no area.
	A Point bbox has the extent of a single point.
	A NULL bbox is "non-existent" and is symbolized by having p2 < p1 for some coordinate.  This is used to define an "empty set" when unioning bboxes.
	
	So...the empty ctor makes a null bbox.
	Operator += is a union and is null-box aware.
	A point makes a point bbox
	
	"overlap" and "contains" are both defined with the boundary as "in" - this seems to be the most useful op for bboxes - we'll provide more specific
	predicates later if needed.  Usually a bbox is a proxy for another shape with a different boundary, which is why specific boundary tests are
	usually not useful.
*/

struct	Bbox2 {
				Bbox2() : p1(0,0), p2(-1, -1) { }
				Bbox2(double x1, double y1, double x2, double y2) : p1(x1, y1), p2(x2, y2) { if (p1.x > p2.x) swap(p1.x, p2.x); if (p1.y > p2.y) swap(p1.y, p2.y); }
				Bbox2(const Point2& in_p1, const Point2& in_p2) : p1(in_p1), p2(in_p2) { if (p1.x > p2.x) swap(p1.x, p2.x); if (p1.y > p2.y) swap(p1.y, p2.y); }
				Bbox2(const Point2& inp) : p1(inp), p2(inp) { }
				Bbox2(const Bbox2& rhs) : p1(rhs.p1), p2(rhs.p2) { }
	Bbox2&		operator=(const Bbox2& rhs) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	Bbox2&		operator=(const Point2& rhs) { p1 = rhs; p2 = rhs; return *this; }
	bool		operator==(const Bbox2& rhs) const { return p1 == rhs.p1 && p2 == rhs.p2; }
	bool		operator!=(const Bbox2& rhs) const { return p1 != rhs.p1 || p2 != rhs.p2; }
	
	Bbox2&		operator+=(const Point2& p);
	Bbox2&		operator+=(const Bbox2& o);

	double		xmin() const { return p1.x; }
	double		ymin() const { return p1.y; }
	double		xmax() const { return p2.x; }
	double		ymax() const { return p2.y; }

	bool		is_empty() const { return p1.x == p2.x || p1.y == p2.y; }
	bool		is_point() const { return p1 == p2; }
	bool		is_null() const { return p1.x > p2.x || p1.y > p2.y; }
	
	bool		overlap(const Bbox2& rhs) const;
	bool		contains(const Bbox2& rhs) const;
	bool		contains(const Point2& p) const;
	bool		interior_overlap(const Bbox2& rhs) const;

	void		expand(double v) { p1.x -= v; p1.y -= v; p2.x += v; p2.y += v; }	
	Point2		centroid(void) const { return Point2((p1.x + p2.x) * 0.5,(p1.y+p2.y) * 0.5); }

	Point2	p1;
	Point2	p2;

};

/****************************************************************************************************
 * Polygon2
 ****************************************************************************************************/

struct	Polygon2 : public vector<Point2> {
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

struct	Bezier2 {
	Bezier2() { }
	Bezier2(const Point2& ip1, const Point2& ic1, const Point2& ic2, const Point2& ip2) : p1(ip1), p2(ip2), c1(ic1), c2(ic2) { }
	Bezier2(const Bezier2& x) : p1(x.p1), p2(x.p2), c1(x.c1), c2(x.c2) { }
	Bezier2& operator=(const Bezier2& x) { p1 = x.p1; p2 = x.p2; c1 = x.c1; c2 = x.c2; return *this; }
	
	bool operator==(const Bezier2& x) const { return p1 == x.p1 && p2 == x.p2 && c1 == x.c1 && c2 == x.c2; }
	bool operator!=(const Bezier2& x) const { return p1 != x.p1 || p2 != x.p2 || c1 != x.c1 || c2 != x.c2; }
	
	Point2	midpoint(double t=0.5) const;											// Returns a point on curve at time T
	Vector2 derivative(double t) const;												// Gives derivative vector at time T.  Length may be zero!
	void	partition(Bezier2& lhs, Bezier2& rhs, double t=0.5) const;				// Splits curve at time T
	void	bounds_fast(Bbox2& bounds) const;										// Returns reasonable bounding box
	int		x_monotone(void) const;									
	int		y_monotone(void) const;
	int		monotone_regions(double t[4]) const;									// Returns up to 4 "t's" where the curve changes dirs.  NOT sorted!
	void	bounds(Bbox2& bounds) const;											// Returns true bounding box
	
	bool	self_intersect(int depth) const;										// True if curve intersects itself except at end-points
	bool	intersect(const Bezier2& rhs, int depth) const;							// True if curves intersect except at end-points
	
	Point2	p1;
	Point2	p2;
	Point2	c1;
	Point2	c2;
	
private:
	bool	self_intersect_recursive(const Bezier2& rhs, int depth) const;
};

/****************************************************************************************************
 * FREE FUNCS
 ****************************************************************************************************/

inline bool	right_turn(const Point2& p1, const Point2& p2, const Point2& p3)
{
	return Vector2(p1,p2).right_turn(Vector2(p2,p3));
}

inline bool	left_turn(const Point2& p1, const Point2& p2, const Point2& p3)
{
	return Vector2(p1,p2).left_turn(Vector2(p2,p3));
}

inline bool	collinear(const Point2& p1, const Point2& p2, const Point2& p3)
{
	return Vector2(p1,p2).no_turn(Vector2(p2,p3));
}

inline int	turn_direction(const Point2& p1, const Point2& p2, const Point2& p3)
{
	return Vector2(p1,p2).turn_direction(Vector2(p2,p3));
}

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

/*
inline double	Segment2::squared_distance(const Point2& p) const 
{
	// TODO - would we get more efficiency from (1) calculating the line L that supports P and then
	// (2) appling the line distance formula?
	return Segment2(p, projection(p)).squared_length(); 
}
*/

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

// DENORMALIZED DISTANCE: this is the distance from the line where the normal side of the line
// is positive, the other is negative, and the units are funny unless a^2 + b^2 == 1.0.
inline double Line2::distance_denormaled(const Point2& p) const
{
	return a * p.x + b * p.y + c;
}

inline void Line2::normalize()
{
	double dlen = sqrt(a * a + b * b);
	if (dlen != 0.0)
	{
		dlen = 1.0 / dlen;
		a *= dlen;
		b *= dlen;
		c *= dlen;
		
		if (a == 0.0) b = b > 0 ? 1.0 : -1.0 ;
		if (b == 0.0) a = a > 0 ? 1.0 : -1.0 ;
	}
}

inline Bbox2& Bbox2::operator+=(const Point2& rhs)
{
	if (is_null())	p1 = p2 = rhs;
	else {
		p1.x = min(p1.x,rhs.x);
		p1.y = min(p1.y,rhs.y);
		p2.x = max(p2.x,rhs.x);
		p2.y = max(p2.y,rhs.y);
	}
	return *this;
}

inline Bbox2& Bbox2::operator+=(const Bbox2& rhs)
{
	if (rhs.is_null()) 	return *this;
	if (is_null()) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	else {
		p1.x = min(p1.x,rhs.p1.x);
		p1.y = min(p1.y,rhs.p1.y);
		p2.x = max(p2.x,rhs.p2.x);
		p2.y = max(p2.y,rhs.p2.y);
		return *this;
	}
}	

inline	bool		Bbox2::overlap(const Bbox2& rhs) const 
{		
	if (is_null()) return false;
	if (rhs.is_null()) return false;
	return 	(xmax() >= rhs.xmin() && rhs.xmax() >= xmin() &&
			 ymax() >= rhs.ymin() && rhs.ymax() >= ymin()); 			
}

inline	bool		Bbox2::interior_overlap(const Bbox2& rhs) const 
{		
	if (is_null()) return false;
	if (rhs.is_null()) return false;
	return 	(xmax() > rhs.xmin() && rhs.xmax() > xmin() &&
			 ymax() > rhs.ymin() && rhs.ymax() > ymin()); 			
}

inline	bool		Bbox2::contains(const Bbox2& rhs) const 
{	
	if (is_null()) return false;
	if (rhs.is_null()) return false;
	return 	(xmax() >= rhs.xmax() && rhs.xmin() >= xmin() &&
			 ymax() >= rhs.ymax() && rhs.ymin() >= ymin()); 			
}
				 
inline	bool		Bbox2::contains(const Point2& p) const 
{
	if (is_null()) return false;
	return (xmin() <= p.x && p.x <= xmax() &&
			ymin() <= p.y && p.y <= ymax()); 
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
			(s.p1.x == inPoint.x && s.p2.x < inPoint.x) ||
			(s.p2.x == inPoint.x && s.p1.x < inPoint.x))
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

inline Vector2 Bezier2::derivative(double t) const 
{
	// Derivative taking by putting the formula in Ax^3+Bx^2+Cx+D form and taking 1st derivative.
	return Vector2(
		3.0 * t * t * (       -p1.x + 3.0 * c1.x - 3.0 * c2.x + p2.x) +
		2.0 * t     * ( -3.0 * p1.x - 6.0 * c1.x + 3.0 * c2.x		) +
					  ( -3.0 * p1.x + 3.0 * c1.x					),
		3.0 * t * t * (       -p1.y + 3.0 * c1.y - 3.0 * c2.y + p2.y) +
		2.0 * t     * ( -3.0 * p1.y - 6.0 * c1.y + 3.0 * c2.y		) +
					  ( -3.0 * p1.y + 3.0 * c1.y					)
	);
}




inline void	Bezier2::partition(Bezier2& lhs, Bezier2& rhs, double t) const
{
	// Partition based on de Casteljau's algorithm - 
	// http://www.math.washington.edu/~king/coursedir/m445w01/lab/lab04/lab04.html
	// http://www.faqs.org/faqs/graphics/algorithms-faq/
	lhs.p1 = p1;
	rhs.p2 = p2;	

	Point2 m = Segment2(c1,c2).midpoint(t);

	lhs.c1 = Segment2(p1,c1).midpoint(t);
	rhs.c2 = Segment2(c2,p2).midpoint(t);
	lhs.c2 = Segment2(lhs.c1,m).midpoint(t);
	rhs.c1 = Segment2(m,rhs.c2).midpoint(t);
	lhs.p2 = rhs.p1 = Segment2(lhs.c2,rhs.c1).midpoint(t);
}

inline void Bezier2::bounds_fast(Bbox2& bounds) const
{	
	// If a curve is monotone, it doesn't exceed the span of its end-points in 
	// any dimension.
	// If a curve is not monotone, it doesn't exceed the span of its bounding or control points.
	// Use this to find the best fast bounding box we can.
	
	// If we needed better bounding boxes, we could calculate the 4 roots (2 roots in a quadratic
	// derivative, for X and Y) and use all points at T = 0,1, and any roots within (0,1).  This
	// would give us a true bounding box.
	bounds = Bbox2(p1,p2);
	if (!x_monotone())
	{
		bounds.p1.x = min(bounds.p1.x,c1.x);
		bounds.p1.x = min(bounds.p1.x,c2.x);
		bounds.p2.x = max(bounds.p2.x,c1.x);
		bounds.p2.x = max(bounds.p2.x,c2.x);
	}
	if (!y_monotone())
	{
		bounds.p1.y = min(bounds.p1.y,c1.y);
		bounds.p1.y = min(bounds.p1.y,c2.y);
		bounds.p2.y = max(bounds.p2.y,c1.y);
		bounds.p2.y = max(bounds.p2.y,c2.y);
	}	
}


inline int		Bezier2::x_monotone(void) const
{
	// The weighting of the control points is for (1-t) and t...
	// this gives us an explicit equation in terms of x = At^3 + Bt^2 + Ct + D.
	double A =       -p1.x + 3.0 * c1.x - 3.0 * c2.x + p2.x;
	double B =  3.0 * p1.x - 6.0 * c1.x + 3.0 * c2.x;
	double C = -3.0 * p1.x + 3.0 * c1.x;
	double D =		  p1.x;
	
	// This is the derivative - which is a quadratic in the form of x = at^2 + bt + c
	double a = 3.0 * A;
	double b = 2.0 * B;
	double c = C;

	if (a == 0) 
	{
		// we have a linear equation for the derivative:
		// x = bt + c
		
		if (b == 0) return 1; // actually we have a CONSTANT equation - in other words, the bezier itself is a line!  So of COURSE it's monotone.
		
		double t = -c / b;
		return (t <= 0.0 || t >= 1.0);		// If dir change is outside the parametric range, this curve is monotone.
	}
	
	// r is the determinant in quad equation - how many roots do we have?
	double r = b * b - 4.0 * a * c;
	if (r < 0) return 1;		// No roots - never changes dir - is monotone!
	
	if (r == 0)
	{
		double t = -b / (2.0 * a);
		return (t <= 0.0 || t >= 1.0);		// If dir change is outside the parametric range, this curve is monotone.
	}
	else
	{
		r = sqrt(r);
		double t1 = (-b + r) / (2.0 * a);
		double t2 = (-b - r) / (2.0 * a);
		return  (t1 <= 0.0 || t1 >= 1.0) && (t2 <= 0.0 || t2 >= 1.0);	// If either dir change is out of 0..1 we're monotone
	}
}

inline int		Bezier2::y_monotone(void) const
{
	// The weighting of the control points is for (1-t) and t...
	// this gives us an explicit equation in terms of x = At^3 + Bt^2 + Ct + D.
	double A =     -p1.y + 3 * c1.y - 3 * c2.y + p2.y;
	double B =  3 * p1.y - 6 * c1.y + 3 * c2.y;
	double C = -3 * p1.y + 3 * c1.y;
	double D =		p1.y;
	
	// This is the derivative - which is a quadratic in the form of x = at^2 + bt + c
	double a = 3 * A;
	double b = 2 * B;
	double c = C;

	if (a == 0) 
	{
		// we have a linear equation for the derivative:
		// x = bt + c
		
		if (b == 0) return 1; // actually we have a CONSTANT equation - in other words, the bezier itself is a line!  So of COURSE it's monotone.
		
		double t = -c / b;
		return (t <= 0.0 || t >= 1.0);		// If dir change is outside the parametric range, this curve is monotone.
	}
	
	// r is the determinant in quad equation - how many roots do we have?
	double r = b * b - 4 * a * c;
	if (r < 0) return 1;		// No roots - never changes dir - is monotone!
	
	if (r == 0)
	{
		double t = -b / (2 * a);
		return (t <= 0.0 || t >= 1.0);		// If dir change is outside the parametric range, this curve is monotone.
	}
	else
	{
		r = sqrt(r);
		double t1 = (-b + r) / (2 * a)	;
		double t2 = (-b -r ) / (2 * a);
		return  (t1 <= 0.0 || t1 >= 1.0) && (t2 <= 0.0 || t2 >= 1.0);	// If either dir change is out of 0..1 we're monotone
	}
}

inline bool	Bezier2::intersect(const Bezier2& rhs, int d) const
{
	Bbox2	lhs_bbox, rhs_bbox;
	
	this->bounds(lhs_bbox);
	rhs.bounds(rhs_bbox);

	if (d < 0)									return true;	
	if (!lhs_bbox.interior_overlap(rhs_bbox))	return false;

	// Ben says: it is NOT good enough to say that if the underlying segments cross, the curves cross EVEN if they're monotone.  Why?
	// One can curve AROUND the other.  
	
	// If we could CLIP the curves to the bounding box that is the INTERSECTION of the two bounding 
	// boxes, we would get shorter curves (smaller time range T).  This would clip off the section where the curve goes "around" and
	// give us a new segment.  Those NEW segments would not overlap and we would get a correct "no intersect" result in the around case.

	// However, that approach is totally not worth it because:
	// - we'd have to, given XY-space clip rects, find T such that we cross the clip bounds.  That would involve cubic root finding, which
	// while O(k) time is still a PITA.  
	// - we'd only get an approximate intersection anyway, so just recursing a few steps more with bounding boxes gives us about the same
	// level of info.
/*
	bool l_mono = lhs.x_monotone() && lhs.y_monotone();
	bool r_mono = rhs.x_monotone() && rhs.y_monotone();

	if (l_mono && r_mono)
	{
		Segment2 l(lhs.p1,lhs.p2);
		Segment2 r(rhs.p1,rhs.p2);
		b1 = lhs_bbox;
		b2 = rhs_bbox;		
		Point2 dummy;
		return l.intersect(r, dummy) ? 2 : 0;
	}
*/
	
	Bezier2	l1,l2,r1,r2;
	
	this->partition(l1,l2);
	rhs.partition(r1,r2);
	
	return 
		l1.intersect(r1,d-1) ||
		l1.intersect(r2,d-1) ||
		l2.intersect(r1,d-1) ||
		l2.intersect(r2,d-1);
}


inline bool Bezier2::self_intersect_recursive(const Bezier2& rhs, int d) const
{
	if (d < 1) return this->intersect(rhs,d);

	Bezier2 l1,l2,r1,r2;
	this->partition(l1,l2);
	rhs.partition(r1,r2);
		
	return this->intersect(rhs,d-1) ||
		l1.self_intersect_recursive(l2,d-1) ||
		r1.self_intersect_recursive(r2,d-1);
}

inline bool	Bezier2::self_intersect(int d) const
{
	Bbox2 d1,d2;
	Bezier2	b1, b2;
	this->partition(b1,b2,0.5f);
	return b1.self_intersect_recursive(b2,d);
}

inline void insert_into(double v, int& num, double * values, double low, double high)
{
	if (v <= low || v >= high) return;		// Ignore anything outside bounds
	for (int n = 0; n < num; ++n)			// Ignore repeated roots!
		if (v == values[n]) return;
	values[num++] = v;
}

inline int		Bezier2::monotone_regions(double times[4]) const
{
	// Basic idea: we do two derivatives - one of the X cubic and one of the Y.
	// These are quadratic and have up to 2 roots each.  This gives us any
	// point the curve changes directions.
	int ret = 0;
	
	double Ax =       -p1.x + 3.0 * c1.x - 3.0 * c2.x + p2.x;
	double Bx =  3.0 * p1.x - 6.0 * c1.x + 3.0 * c2.x;
	double Cx = -3.0 * p1.x + 3.0 * c1.x;
	double Dx =		   p1.x;
	double ax = 3.0 * Ax;
	double bx = 2.0 * Bx;
	double cx =		  Cx;

	double Ay =       -p1.y + 3.0 * c1.y - 3.0 * c2.y + p2.y;
	double By =  3.0 * p1.y - 6.0 * c1.y + 3.0 * c2.y;
	double Cy = -3.0 * p1.y + 3.0 * c1.y;
	double Dy =		   p1.y;
	double ay = 3.0 * Ay;
	double by = 2.0 * By;
	double cy =		  Cy;

	double r;

	if (ax == 0) {
		if (bx != 0)						insert_into( -cx      /        bx,  ret, times, 0.0, 1.0);		// Linear case - slope of curve is linear and crosses X axis.
	} else {
		r = bx *  bx - 4.0 * ax * cx;
		if (r == 0)							insert_into( -bx      / (2.0 * ax), ret, times, 0.0, 1.0);		// One root quadratic - apex of parabola touches X axis.
		else if (r > 0.0) { r = sqrt(r);	insert_into((-bx - r) / (2.0 * ax), ret, times, 0.0, 1.0);		// Two root cases - quadratic crosses X axis twice.
											insert_into((-bx + r) / (2.0 * ax), ret, times, 0.0, 1.0); }
	}

	if (ay == 0) {
		if (by != 0)						insert_into( -cy      /        by,  ret, times, 0.0, 1.0);		// Linear case - slope of curve is linear and crosses X axis.
	} else {
		r = by *  by - 4.0 * ay * cy;
		if (r == 0)							insert_into( -by      / (2.0 * ay), ret, times, 0.0, 1.0);		// One root quadratic - apex of parabola touches X axis.
		else if (r > 0.0) { r = sqrt(r);	insert_into((-by - r) / (2.0 * ay), ret, times, 0.0, 1.0);		// Two root cases - quadratic crosses X axis twice.
											insert_into((-by + r) / (2.0 * ay), ret, times, 0.0, 1.0); }	
	}

	return ret;
}

inline void	Bezier2::bounds(Bbox2& bounds) const
{
	double t[4];
	int n = monotone_regions(t);
	bounds = Bbox2(p1,p2);				// bounding box is exetended by the endpoints and
	for (int i = 0; i < n; ++i)			// any point where we change monotonacity.  This is equivalent to breaking the 
		bounds += midpoint(t[i]);		// curve into up to 4 monotone pieces, taking those 4 bounding box endpoints, and unioning them all!
}



void	TEST_CompGeomDefs2(void);

#endif
