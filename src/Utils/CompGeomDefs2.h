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
#include <algorithm>

#include "AssertUtils.h"

using std::vector;
using std::swap;
using std::min;
using std::max;

struct	Point2;
struct	Vector2;
struct	Bbox2;
struct  Line2;
struct Bezier2;

//This code is only for MSVC
#if defined(_MSC_VER)
	inline double cbrt(double x) { return pow(x, 1.0/3.0); }
#endif

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

// Root-finders...they're only approximations, but they're still useful for some of the weirder
// algs here.
int linear_formula(double a, double b, double roots[1]);
int quadratic_formula(double a, double b, double c, double roots[2]);
int cubic_formula(double a, double b, double c, double d, double roots[3]);

// Some polygon algorithms are available as a templated alg on a sequence of Point2s.  This
// might be useful if you have your polygon in something other than a stand-alone Polygon2.
template <class __Iterator>
bool inside_polygon_pt(__Iterator begin, __Iterator end, const Point2& inPoint);
template <class __Iterator>
bool inside_polygon_seg(__Iterator begin, __Iterator end, const Point2& inPoint);
template <class __Iterator>
bool inside_polygon_bez(__Iterator begin, __Iterator end, const Point2& inPoint);
template <class __Iterator>
bool is_ccw_polygon_pt(__Iterator begin, __Iterator end);
template <class __Iterator>
bool is_ccw_polygon_seg(__Iterator begin, __Iterator end);
template <class __Iterator>
double signed_area_pt(__Iterator begin, __Iterator end);

// This takes a Bezier2 and recursively output-iterates it into a point container.  A quick way to get guaranteed-approximation
// bezier approximations.  Endpoint is omitted - for chaining!
template <class __output_iterator>
void approximate_bezier_epsi(const Bezier2& b, double err, __output_iterator output, double t_start=0.0, double t_middle=0.5, double t_end=1.0);

// This takes a "bezier sequence", which is an iterator pair of Point2 and int codes.  If you just ahve a "bezier polygon" (that is, a vector of Bezier2)
// just iterate on it directly and call approximate_bezier_epsi.
template <class __input_iterator, class __output_iterator>
void approximate_bezier_sequence_epsi(
						__input_iterator		s,
						__input_iterator		e, 
						__output_iterator		o,
						double					epsi);

// Same as above, but we split TWO beziers.  "b" is used for error metrics, but b2 is output to output2 at the same T intervals.
// We can use this to get the UV map for a bezier as we approximate it.
template <class __output_iterator>
void approximate_bezier_epsi_2(const Bezier2& b, const Bezier2& b2, double err, __output_iterator output1, __output_iterator output2, double t_start=0.0, double t_middle=0.5, double t_end=1.0);



/****************************************************************************************************
 * Point2
 ****************************************************************************************************/

struct Point2 {
	Point2() : x_(0.0), y_(0.0) { }
	Point2(double ix, double iy) : x_(ix), y_(iy) { }
	Point2(const Point2& rhs) : x_(rhs.x_), y_(rhs.y_) { }

	Point2& operator=(const Point2& rhs) { x_ = rhs.x_; y_ = rhs.y_; return *this; }
	bool operator==(const Point2& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }
	bool operator!=(const Point2& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_; }

	Point2& operator += (const Vector2& v);
	Point2& operator -= (const Vector2& v);
	Point2 operator+(const Vector2& v) const;
	Point2 operator-(const Vector2& v) const;

	inline double squared_distance(const Point2& p) const { return (p.x_ - x_) * (p.x_ - x_) + (p.y_ - y_) * (p.y_ - y_); }

	inline double x() const { return x_; }
	inline double y() const { return y_; }
	double	x_;
	double	y_;
};

/****************************************************************************************************
 * Vector2
 ****************************************************************************************************/

struct	Vector2 {
	Vector2() : dx(0.0), dy(0.0) { }
	Vector2(double ix, double iy) : dx(ix), dy(iy) { }
	Vector2(const Vector2& rhs) : dx(rhs.dx), dy(rhs.dy) { }
	explicit Vector2(const Point2& rhs) : dx(rhs.x_), dy(rhs.y_) { }
	Vector2(const Point2& p1, const Point2& p2) : dx(p2.x_ - p1.x_), dy(p2.y_ - p1.y_) { }

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
	double 	normalize(void) { 
		double len;
		if (dx == 0.0) 
		{
			if(dy >= 0.0)
			{
				len = dy;
				dy = 1.0;
			}
			else
			{
				len = -dy;
				dy = -1.0;
			}
		}
		else if (dy == 0.0) 
		{
			if(dx >= 0.0)
			{
				len = dx;
				dx = 1.0;
			}						
			else
			{
				len = -dx;
				dx = -1.0;
			}
		}
		else { 
			len = sqrt(dx * dx + dy * dy); 
			if (len != 0.0) { double l = 1.0 / len; dx *= l; dy *= l; } 
		} 
		return len;
	}
	Vector2 perpendicular_cw() const { return Vector2(dy, -dx); }
	Vector2 perpendicular_ccw() const { return Vector2(-dy, dx); }

	double	dot(const Vector2& v) const { return dx * v.dx + dy * v.dy; }
	bool	left_turn(const Vector2& v) const { return (-dy * v.dx + dx * v.dy) > 0.0; }
	bool	right_turn(const Vector2& v) const { return (-dy * v.dx + dx * v.dy) < 0.0; }
	bool	no_turn(const Vector2& v) const { return (-dy * v.dx + dx * v.dy) == 0.0; }
	int		turn_direction(const Vector2& v) const { double d = -dy * v.dx + dx * v.dy; if (d > 0.0) return LEFT_TURN; if (d < 0.0) return RIGHT_TURN; return COLLINEAR; }

	void	rotate_by_degrees(const double& degrees);

	// Signed area = the area of the triangle formed by placing V at our origin.
	// If they form a counter-clockwise triangle (that is, rotate us ccw to get to V) then
	// the area is positive, clockwise = negative.
	// Why does this work?  If we are the base of the triangle, we can scale the other vector
	// to be 'height' by multiplying by the cosine of the that vector with the real height vector.
	// That height vector is us.ccw_perpendicular.  So...we want 0.5 * A * B * cos(|A'| |B|)
	// which is the same as 0.5 * dot(A', B).  Thus this is just the dot product on our ccw_perpendicular.
	// 
	// (NOTE: this is really a special case - the magnitude of the cross-product of two vectors
	// is the signed area.  In the 2-d case the normal vector is entirely in the Z component, because
	// the source vecotrs are in the XY plane, so we can "extract" the magnitude without the usual
	// pythag theorem (saving a square route).
	double	signed_area(const Vector2& v) const { return (dx * v.dy - dy * v.dx) * 0.5; }

	// Vector projection of other onto US!
	Vector2	projection(const Vector2& other) const;

	inline double x() const { return dx; }
	inline double y() const { return dy; }
	double	dx;
	double	dy;
};

/****************************************************************************************************
 * Segment2
 ****************************************************************************************************/

struct	Segment2 {
	Segment2() : p1(), p2() { }
	Segment2(const Point2& ip1, const Point2& ip2) : p1(ip1), p2(ip2) { }
	Segment2(const Point2& p, const Vector2& v) : p1(p), p2(p.x_ + v.dx, p.y_ + v.dy) { }
	Segment2(const Segment2& rhs) : p1(rhs.p1), p2(rhs.p2) { }

	Segment2& operator=(const Segment2& rhs) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	bool operator==(const Segment2& rhs) const { return p1 == rhs.p1 && p2 == rhs.p2; }
	bool operator!=(const Segment2& rhs) const { return p1 != rhs.p1 || p2 != rhs.p2; }

	Segment2& operator+=(const Vector2& rhs) { p1 += rhs; p2 += rhs; return *this; }

	double	squared_length(void) const { return (p2.x_ - p1.x_) * (p2.x_ - p1.x_) + (p2.y_ - p1.y_) * (p2.y_ - p1.y_); }
	Point2	midpoint(double s=0.5) const { if (s==0.0) return p1; if (s==1.0) return p2; double ms = 1.0 - s; return Point2(
													p1.x_ == p2.x_ ? p1.x_ : p1.x_ * ms + p2.x_ * s,
													p1.y_ == p2.y_ ? p1.y_ : p1.y_ * ms + p2.y_ * s); }
	Point2	projection(const Point2& pt) const;
	double	squared_distance_supporting_line(const Point2& p) const;		// Squared distance to our supporting line.
	double	squared_distance(const Point2& p) const;
	
	//Moves this segment by some vector to a new location, keeping the segments orientation
	void	move_by_vector(const Vector2& v);

	bool	collinear_has_on(const Point2& p) const;
	bool	on_left_side(const Point2& p) const { return Vector2(p1, p2).left_turn(Vector2(p1, p)); }
	bool	on_right_side(const Point2& p) const { return Vector2(p1, p2).right_turn(Vector2(p1, p)); }
	bool	collinear(const Point2& p) const { return Vector2(p1, p2).no_turn(Vector2(p1, p)); }
	int		side_of_line(const Point2& p) const { return Vector2(p1, p2).turn_direction(Vector2(p1, p));  }

	bool	could_intersect(const Segment2& rhs) const;
	bool	intersect(const Segment2& rhs, Point2& p) const;

	bool	is_near(const Point2& p, double distance) const;
	
	bool	clip_to(const Bbox2& bbox);
	bool	clip_to(const Segment2& bbox);
	
	double	y_at_x(double x) const { 	if (p1.x_ == p2.x_) 	return p1.y_;
										if (x == p1.x_) 		return p1.y_;
										if (x == p2.x_) 		return p2.y_;
										return p1.y_ + (p2.y_ - p1.y_) * (x - p1.x_) / (p2.x_ - p1.x_); }
	double	x_at_y(double y) const { 	if (p1.y_ == p2.y_) 	return p1.x_;
										if (y == p1.y_) 		return p1.x_;
										if (y == p2.y_) 		return p2.x_;
										return p1.x_ + (p2.x_ - p1.x_) * (y - p1.y_) / (p2.y_ - p1.y_); }

	bool	is_vertical(void) const { return p1.x_ == p2.x_; }
	bool	is_horizontal(void) const { return p1.y_ == p2.y_; }
	bool	is_empty(void) const { return p1 == p2; }

	inline	Point2	source() const { return p1; }
	inline	Point2	target() const { return p2; }
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
	Line2(const Point2& p1, const Point2& p2) : a(p1.y_ - p2.y_), b(p2.x_ - p1.x_), c((p2.y_ - p1.y_) * p1.x_ - (p2.x_ - p1.x_) * p1.y_) { }
	Line2(const Point2& p, const Vector2& v) : a(-v.dy), b(v.dx), c(v.dy * p.x_ - v.dx * p.y_) { }
	Line2(const Line2& l) : a(l.a), b(l.b), c(l.c) { }
	Line2(double ia, double ib, double ic) : a(ia), b(ib), c(ic) { }
	explicit Line2(const Segment2& s) : a(s.p1.y_ - s.p2.y_), b(s.p2.x_ - s.p1.x_), c((s.p2.y_ - s.p1.y_) * s.p1.x_ - (s.p2.x_ - s.p1.x_) * s.p1.y_) { }

	Line2& operator=(const Line2& rhs) { a = rhs.a; b = rhs.b; c = rhs.c; return *this; }
	bool operator==(const Line2& rhs) const { return (a * rhs.b == rhs.a * b) && (a * rhs.c == rhs.a * c) && (b * rhs.c == rhs.b * c); }
	bool operator!=(const Line2& rhs) const { return (a * rhs.b != rhs.a * b) || (a * rhs.c != rhs.a * c) || (b * rhs.c != rhs.b * c); }

	bool intersect(const Line2& l, Point2& p) const;
	double squared_distance(const Point2& p) const;
	double distance_denormaled(const Point2& p) const;
	void normalize();

	bool	on_left_side(const Point2& p) const { return (a * p.x_ + b * p.y_ + c) > 0; }
	bool	on_right_side(const Point2& p) const { return (a * p.x_ + b * p.y_ + c) < 0; }
	bool	collinear(const Point2& p) const { return (a * p.x_ + b * p.y_ + c) == 0; }
	int		side_of_line(const Point2& p) const { double v = (a * p.x_ + b * p.y_ + c); if (v > 0.0) return LEFT_TURN; if (v < 0.0) return RIGHT_TURN; return COLLINEAR; }

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
				Bbox2(double x1, double y1, double x2, double y2) : p1(x1, y1), p2(x2, y2) { if (p1.x_ > p2.x_) swap(p1.x_, p2.x_); if (p1.y_ > p2.y_) swap(p1.y_, p2.y_); }
				Bbox2(const Point2& in_p1, const Point2& in_p2) : p1(in_p1), p2(in_p2) { if (p1.x_ > p2.x_) swap(p1.x_, p2.x_); if (p1.y_ > p2.y_) swap(p1.y_, p2.y_); }
				Bbox2(const Point2& inp) : p1(inp), p2(inp) { }
				Bbox2(const Segment2& s) : p1(s.p1), p2(s.p2) { if (p1.x_ > p2.x_) swap(p1.x_, p2.x_); if (p1.y_ > p2.y_) swap(p1.y_, p2.y_); }
				Bbox2(const Bbox2& rhs) : p1(rhs.p1), p2(rhs.p2) { }
	Bbox2&		operator=(const Bbox2& rhs) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	Bbox2&		operator=(const Point2& rhs) { p1 = rhs; p2 = rhs; return *this; }
	bool		operator==(const Bbox2& rhs) const { return p1 == rhs.p1 && p2 == rhs.p2; }
	bool		operator!=(const Bbox2& rhs) const { return p1 != rhs.p1 || p2 != rhs.p2; }

	Bbox2&		operator+=(const Point2& p);
	Bbox2&		operator+=(const Vector2& p);
	Bbox2&		operator+=(const Bbox2& o);

	double		xmin() const { return p1.x_; }
	double		ymin() const { return p1.y_; }
	double		xmax() const { return p2.x_; }
	double		ymax() const { return p2.y_; }
	double		xspan() const { return p2.x_ - p1.x_; }
	double		yspan() const { return p2.y_ - p1.y_; }
	double		area() const { return max(p2.x_ - p1.x_,0.0) * max(p2.y_ - p1.y_,0.0); }
	
	Point2		bottom_left (void) const { return Point2(p1.x(),p1.y()); }
	Point2		bottom_right(void) const { return Point2(p2.x(),p1.y()); }
	Point2		top_left    (void) const { return Point2(p1.x(),p2.y()); }
	Point2		top_right   (void) const { return Point2(p2.x(),p2.y()); }
	Segment2	bottom_side (void) const { return Segment2(bottom_left(), bottom_right()); }
	Segment2	top_side    (void) const { return Segment2(top_right(), top_left()); }
	Segment2	left_side   (void) const { return Segment2(top_left(), bottom_left()); }
	Segment2	right_side  (void) const { return Segment2(bottom_right(), top_right()); }

	bool		is_empty() const { return p1.x_ == p2.x_ || p1.y_ == p2.y_; }
	bool		is_point() const { return p1 == p2; }
	bool		is_null() const { return p1.x_ > p2.x_ || p1.y_ > p2.y_; }

	bool		overlap(const Bbox2& rhs) const;
	bool		interior_overlap(const Bbox2& rhs) const;
	bool		contains(const Bbox2& rhs) const;

	bool		contains(const Point2& p) const;
	bool		contains(const Segment2& p) const;

	void		expand(double v) { p1.x_ -= v; p1.y_ -= v; p2.x_ += v; p2.y_ += v; }
	void		expand(double vx, double vy) { p1.x_ -= vx; p1.y_ -= vy; p2.x_ += vx; p2.y_ += vy; }
	Point2		centroid(void) const { return Point2((p1.x_ + p2.x_) * 0.5,(p1.y_+p2.y_) * 0.5); }

	Point2		clamp(const Point2& p) const { return Point2(
						p.x() < xmin() ? xmin() : (p.x() > xmax() ? xmax() : p.x()),
						p.y() < ymin() ? ymin() : (p.y() > ymax() ? ymax() : p.y())); };

	double		rescale_to_x (const Bbox2& new_box, double x) const;
	double		rescale_to_y (const Bbox2& new_box, double y) const;
	double		rescale_to_xv(const Bbox2& new_box, double x) const;
	double		rescale_to_yv(const Bbox2& new_box, double y) const;

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
	template <typename __Iterator>
				Polygon2(__Iterator s, __Iterator e) : vector<Point2>(s,e) { }

	struct const_side_iterator : public std::iterator<forward_iterator_tag, Segment2> {
	
		const Polygon2 * p_;
		int n_;
		Segment2 operator*(){ return p_->side(n_); }
		const_side_iterator& operator++() { ++n_; return *this; }
		bool operator==(const const_side_iterator& rhs) const { return p_ == rhs.p_ && n_ == rhs.n_; }
		bool operator!=(const const_side_iterator& rhs) const { return p_ != rhs.p_ || n_ != rhs.n_; }
		const_side_iterator(const Polygon2 * p, int n) : p_(p), n_(n) { }
	};
	
	const_side_iterator sides_begin() const { return const_side_iterator(this,0); }
	const_side_iterator sides_end() const { return const_side_iterator(this,size()); }

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

	// Returns true if the segment intersects any part of the polygon, very simple and not optimized
	bool		intersects(const Segment2& inSegment) const;

	int			prev(int index) const { return (index + size() - 1) % size(); }
	int			next(int index) const { return (index + 1) % size(); }

	bool		is_ccw(void) const;

};

/****************************************************************************************************
 * Triangle2
 ****************************************************************************************************/

struct	Triangle2 {

	Triangle2() { }
	Triangle2(const Point2& ip1, const Point2& ip2, const Point2& ip3) : p1(ip1), p2(ip2), p3(ip3) {}

	double	signed_area(void) const { return Vector2(p1,p2).signed_area(Vector2(p1,p3)); }
	bool	is_ccw(void) const { return Vector2(p1,p2).left_turn(Vector2(p2,p3)); }
	bool	is_cw (void) const { return Vector2(p1,p2).right_turn(Vector2(p2,p3)); }
	bool	inside_ccw(const Point2& p) const { return 
					Vector2(p1,p2).left_turn(Vector2(p2,p)) && 
					Vector2(p2,p3).left_turn(Vector2(p3,p)) && 
					Vector2(p3,p1).left_turn(Vector2(p1,p)); }
	double	squared_distance_ccw(const Point2& p) const { 
		if(inside_ccw(p)) return 0.0; 
		return min(min(Segment2(p1,p2).squared_distance(p),
						Segment2(p2,p3).squared_distance(p)),Segment2(p3,p1).squared_distance(p)); }
						
	bool	bathymetric_interp(const Point2& p, double& v1, double& v2, double& v3) const {
		double sa_me = signed_area();
		if(sa_me == 0.0) return false;
		sa_me = 1.0 / sa_me;
		v1 = sa_me * Triangle2(p2,p3,p).signed_area();
		v2 = sa_me * Triangle2(p3,p1,p).signed_area();
		v3 = sa_me * Triangle2(p1,p2,p).signed_area();
		return true;
	}

	Point2	p1;
	Point2	p2;
	Point2	p3;
	
};

/****************************************************************************************************
 * Bezier2
 ****************************************************************************************************/

struct	Bezier2 {
	Bezier2() { }
	Bezier2(const Point2& ip1, const Point2& ic1, const Point2& ic2, const Point2& ip2) : p1(ip1), p2(ip2), c1(ic1), c2(ic2) { }
	Bezier2(const Point2& ip1, const Point2& ic, const Point2& ip2);	// quadratic-equivalent...
	Bezier2(const Bezier2& x) : p1(x.p1), p2(x.p2), c1(x.c1), c2(x.c2) { }
	Bezier2(const Segment2& x) : p1(x.p1), p2(x.p2), c1(x.p1), c2(x.p2) { }
	Bezier2& operator=(const Bezier2& x) { p1 = x.p1; p2 = x.p2; c1 = x.c1; c2 = x.c2; return *this; }

	bool operator==(const Bezier2& x) const { return p1 == x.p1 && p2 == x.p2 && c1 == x.c1 && c2 == x.c2; }
	bool operator!=(const Bezier2& x) const { return p1 != x.p1 || p2 != x.p2 || c1 != x.c1 || c2 != x.c2; }

	Point2	midpoint(double t=0.5) const;											// Returns a point on curve at time T
	Vector2 derivative(double t) const;												// Gives derivative vector at time T.  Length may be zero!
	void	partition(Bezier2& lhs, Bezier2& rhs, double t=0.5) const;				// Splits curve at time T
	void	subcurve(Bezier2& sub, double t1, double t2) const;						// Sub-curve from T1 to T2.
	void	bounds_fast(Bbox2& bounds) const;										// Returns reasonable bounding box
	int		x_monotone(void) const;
	int		y_monotone(void) const;
	int		x_monotone_regions(double t[2]) const;									// Returns up to 2 "t's" where the curve changes dirs.
	int		y_monotone_regions(double t[2]) const;									// Returns up to 2 "t's" where the curve changes dirs.
	int		monotone_regions(double t[4]) const;									// Returns up to 4 "t's" where the curve changes dirs.  NOT sorted!
	void	bounds(Bbox2& bounds) const;											// Returns true bounding box

	bool	self_intersect(int depth) const;										// True if curve intersects itself except at end-points
	bool	intersect(const Bezier2& rhs, int depth) const;							// True if curves intersect except at end-points
	bool	is_near(const Point2& p, double distance) const;
	
	bool		is_segment(void) const { return p1 == c1 && p2 == c2; }
	Segment2	as_segment(void) const { return Segment2(p1,p2); }

	double	y_at_x(double x) const;
	double	x_at_y(double y) const;
	
	int		t_at_x(double x, double t[3]) const;
	int		t_at_y(double y, double t[3]) const;
	double	approx_t_for_xy(double x, double y) const;
	
	Point2	p1;
	Point2	p2;
	Point2	c1;
	Point2	c2;
	
	inline	Point2	source() const { return p1; }
	inline	Point2	target() const { return p2; }	

private:
	bool	self_intersect_recursive(const Bezier2& rhs, int depth) const;
};

// "Bezier point" - a simplified version of what WED does.  lo or hi control handles being colocated with pt is used to signal no control handle.
struct	BezierPoint2 {
	Point2		lo;
	Point2		pt;
	Point2		hi;
	bool		has_lo(void) const { return lo != pt; }
	bool		has_hi(void) const { return hi != pt; }
	bool		is_split(void) const { return Vector2(lo,pt) != Vector2(pt,hi); }
	
	bool operator==(const BezierPoint2& rhs) const { return lo == rhs.lo && pt == rhs.pt && hi == rhs.hi; }
};


struct BezierPolygon2 : public vector<Bezier2> {
				BezierPolygon2() 						: vector<Bezier2>() 		{ }
				BezierPolygon2(const BezierPolygon2& rhs)   : vector<Bezier2>(rhs) 	{ }
				BezierPolygon2(int x) 				: vector<Bezier2>(x) 	{ }
	template <typename __Iterator>
				BezierPolygon2(__Iterator s, __Iterator e) : vector<Bezier2>(s,e) { }


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

struct lesser_y {	bool	operator()(const Point2& lhs, const Point2& rhs) const { return (lhs.y_ < rhs.y_);}};
struct lesser_x {	bool	operator()(const Point2& lhs, const Point2& rhs) const { return (lhs.x_ < rhs.x_);}};


//Implemented to follow Strict Weak Ordering requirements of the STL
struct lesser_y_then_x {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.y_ == rhs.y_) ? (lhs.x_ < rhs.x_) : (lhs.y_ < rhs.y_);
	}
	bool	operator()(const Segment2& lhs, const Segment2& rhs) const {
		return (lhs.p2.y_ == rhs.p2.y_) ? (lhs.p2.x_ < rhs.p2.x_) : (lhs.p2.y_ < rhs.p2.y_);
	}
	bool	operator()(const Bezier2& lhs, const Bezier2& rhs) const {
		return (lhs.p2.y_ == rhs.p2.y_) ? (lhs.p2.x_ < rhs.p2.x_) : (lhs.p2.y_ < rhs.p2.y_);
	}
};

struct greater_y_then_x {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.y_ == rhs.y_) ? (lhs.x_ > rhs.x_) : (lhs.y_ > rhs.y_);
	}
	bool	operator()(const Segment2& lhs, const Segment2& rhs) const {
		return (lhs.p2.y_ == rhs.p2.y_) ? (lhs.p2.x_ > rhs.p2.x_) : (lhs.p2.y_ > rhs.p2.y_);
	}
	bool	operator()(const Bezier2& lhs, const Bezier2& rhs) const {
		return (lhs.p2.y_ == rhs.p2.y_) ? (lhs.p2.x_ > rhs.p2.x_) : (lhs.p2.y_ > rhs.p2.y_);
	}
};

struct lesser_x_then_y {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.x_ == rhs.x_) ? (lhs.y_ < rhs.y_) : (lhs.x_ < rhs.x_);
	}
	bool	operator()(const Segment2& lhs, const Segment2& rhs) const {
		return (lhs.p2.x_ == rhs.p2.x_) ? (lhs.p2.y_ < rhs.p2.y_) : (lhs.p2.x_ < rhs.p2.x_);
	}
	bool	operator()(const Bezier2& lhs, const Bezier2& rhs) const {
		return (lhs.p2.x_ == rhs.p2.x_) ? (lhs.p2.y_ < rhs.p2.y_) : (lhs.p2.x_ < rhs.p2.x_);
	}
};

struct greater_x_then_y {
	bool	operator()(const Point2& lhs, const Point2& rhs) const {
		return (lhs.x_ == rhs.x_) ? (lhs.y_ > rhs.y_) : (lhs.x_ > rhs.x_);
	}
	bool	operator()(const Segment2& lhs, const Segment2& rhs) const {
		return (lhs.p2.x_ == rhs.p2.x_) ? (lhs.p2.y_ > rhs.p2.y_) : (lhs.p2.x_ > rhs.p2.x_);
	}
	bool	operator()(const Bezier2& lhs, const Bezier2& rhs) const {
		return (lhs.p2.x_ == rhs.p2.x_) ? (lhs.p2.y_ > rhs.p2.y_) : (lhs.p2.x_ > rhs.p2.x_);
	}
};

/****************************************************************************************************
 * Inline Funcs
 ****************************************************************************************************/

// These must be defined below because Point2 is declared before Vector2.
inline Point2& Point2::operator += (const Vector2& v) { x_ += v.dx; y_ += v.dy; return *this; }
inline Point2& Point2::operator -= (const Vector2& v) { x_ -= v.dx; y_ -= v.dy; return *this; }
inline Point2 Point2::operator+(const Vector2& v) const { return Point2(x_ + v.dx, y_ + v.dy); }
inline Point2 Point2::operator-(const Vector2& v) const { return Point2(x_ - v.dx, y_ - v.dy); }

inline void	Vector2::rotate_by_degrees(const double& degrees)
{
	double cs = cos(0.0174532925199432958 * degrees);
	double sn = sin(0.0174532925199432958 * degrees);

	double rx = dx * cs - dy * sn;
	double ry = dx * sn + dy * cs;

	dx = rx;
	dy = ry;
}

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


inline double	Segment2::squared_distance_supporting_line(const Point2& p) const
{
	// TODO - would we get more efficiency from (1) calculating the line L that supports P and then
	// (2) appling the line distance formula?
	// BAS: NO!  To use a line to get distance we must normalize -- that means a sqrt and a divide.
	// This runs with just adds and mults.  (Projection and squared length are both secretly dot products.)
	// If we wanted the distance (not sqrd distance) we might be closer.
	return Segment2(p, projection(p)).squared_length();
}


// Determines if this point is within the scope of the segment.
// We assume colinear-ness...really it checks to see if we're
// within both of two half planes, normal to our segment's end points.
inline bool	Segment2::collinear_has_on(const Point2& p) const
{
	if (is_horizontal()) return (p.x_ >= p1.x_ && p.x_ <= p2.x_) || (p.x_ >= p2.x_ && p.x_ <= p1.x_);
	if (is_vertical())   return (p.y_ >= p1.y_ && p.y_ <= p2.y_) || (p.y_ >= p2.y_ && p.y_ <= p1.y_);
	if (Vector2(p1, p2).dot(Vector2(p1, p)) < 0.0) return false;
	if (Vector2(p2, p1).dot(Vector2(p2, p)) < 0.0) return false;
	return true;
}

inline double Segment2::squared_distance(const Point2& p) const 
{
	// If we are "colinear on" (that is, in between the two half planes that cover our expanse and left-right neighrborhoods
	// then the supporting line distance is the closest and best distance.
	// Otherwise, just take the closer of the two end points.
	if(collinear_has_on(p)) return squared_distance_supporting_line(p);
	else					return min(p1.squared_distance(p), p2.squared_distance(p));
}

inline void	Segment2::move_by_vector(const Vector2& v)
{
	this->p1 += v;
	this->p2 += v;
}

inline bool Segment2::could_intersect(const Segment2& rhs) const
{
	// This is basically a bounding-box quick check for segment-intersection.
	register double	xmin1 = (p1.x_ < p2.x_) ? p1.x_ : p2.x_;
	register double	xmax1 = (p1.x_ > p2.x_) ? p1.x_ : p2.x_;
	register double	ymin1 = (p1.y_ < p2.y_) ? p1.y_ : p2.y_;
	register double	ymax1 = (p1.y_ > p2.y_) ? p1.y_ : p2.y_;

	register double	xmin2 = (rhs.p1.x_ < rhs.p2.x_) ? rhs.p1.x_ : rhs.p2.x_;
	register double	xmax2 = (rhs.p1.x_ > rhs.p2.x_) ? rhs.p1.x_ : rhs.p2.x_;
	register double	ymin2 = (rhs.p1.y_ < rhs.p2.y_) ? rhs.p1.y_ : rhs.p2.y_;
	register double	ymax2 = (rhs.p1.y_ > rhs.p2.y_) ? rhs.p1.y_ : rhs.p2.y_;

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
	if (rhs.p1 == p1 && rhs.p2 == p2) return false;     // co-located segments
	if (rhs.p1 == p2 && rhs.p2 == p1) return false;     // reversed, co-located segments
	if (rhs.p1 == p1 || rhs.p2 == p1) { p = p1; return true; }  // segments sharing node 1
	if (rhs.p1 == p2 || rhs.p2 == p2) { p = p2; return true; }  // segments sharing node 2

	if (is_horizontal())
	{
		if (rhs.is_horizontal())
		{
			return false;
		}
		else if (rhs.is_vertical())
		{
			p.x_ = rhs.p1.x_;
			p.y_ = p1.y_;
		}
		else
		{
			p.x_ = rhs.x_at_y(p1.y_);
			p.y_ = p1.y_;
		}
	}
	else if (is_vertical())
	{
		if (rhs.is_horizontal())
		{
			p.x_ = p1.x_;
			p.y_ = rhs.p1.y_;
		}
		else if (rhs.is_vertical())
		{
			return false;
		}
		else
		{
			p.x_ = p1.x_;
			p.y_ = rhs.y_at_x(p1.x_);
		}
	}
	else
	{
		if (rhs.is_horizontal())
		{
			p.x_ = x_at_y(rhs.p1.y_);
			p.y_ = rhs.p1.y_;
		}
		else if (rhs.is_vertical())
		{
			p.x_ = rhs.p1.x_;
			p.y_ = y_at_x(rhs.p1.x_);
		}
		else
		{
			if (!Line2(*this).intersect(Line2(rhs), p)) return false;
		}
	}
	return collinear_has_on(p) && rhs.collinear_has_on(p);
}

inline bool	Segment2::is_near(const Point2& p, double distance) const
{
	distance *= distance;
	if (p.squared_distance(p1) < distance) return true;
	if (p.squared_distance(p2) < distance) return true;

	if (!collinear_has_on(p)) return false;

	Point2 proj = projection(p);
	return (p.squared_distance(proj) < distance);
}

inline bool	Segment2::clip_to(const Segment2& l)
{
	bool out1 = l.on_right_side(p1);
	bool out2 = l.on_right_side(p2);
	
	if(out1 && out2)	return false;
	if(!out1 && !out2)	return true;
	
	Point2	x;
	if (!l.intersect((*this), x))
//		Assert(!"Reliability problem.");
		return false;
	
	if(out1)	p1 = x;
	else		p2 = x;
	return true;	
}

inline bool	Segment2::clip_to(const Bbox2& bbox)
{
	Point2	c1(bbox.xmin(), bbox.ymin());
	Point2	c2(bbox.xmax(), bbox.ymin());
	Point2	c3(bbox.xmax(), bbox.ymax());
	Point2	c4(bbox.xmin(), bbox.ymax());
	
	Segment2	l1(c1,c2);
	Segment2	l2(c2,c3);
	Segment2	l3(c3,c4);
	Segment2	l4(c4,c1);
	
	if(!clip_to(l1))	return false;
	if(!clip_to(l2))	return false;
	if(!clip_to(l3))	return false;
	if(!clip_to(l4))	return false;
	return true;
}




inline bool Line2::intersect(const Line2& l, Point2& p) const
{
	// Once we have lines in ax + by + c = 0 form, the parallel-equation solution
	// is pretty straight-forward to derive.
	// This does not tell us intervals though.

	if ((a * l.b) == (b * l.a))	return false;	// This means the lines are parallel.
	p.x_ = (b * l.c - l.b * c) / (l.b * a - b * l.a);
	p.y_ = (a * l.c - l.a * c) / (l.a * b - a * l.b);
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
	double num = a * p.x_ + b * p.y_ + c;
	return num * num / (a * a + b * b);
}

// DENORMALIZED DISTANCE: this is the distance from the line where the normal side of the line
// is positive, the other is negative, and the units are funny unless a^2 + b^2 == 1.0.
inline double Line2::distance_denormaled(const Point2& p) const
{
	return a * p.x_ + b * p.y_ + c;
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
		p1.x_ = min(p1.x_,rhs.x_);
		p1.y_ = min(p1.y_,rhs.y_);
		p2.x_ = max(p2.x_,rhs.x_);
		p2.y_ = max(p2.y_,rhs.y_);
	}
	return *this;
}

inline Bbox2& Bbox2::operator+=(const Vector2& rhs)
{
	if (!is_null())
	{
			p1 += rhs;
			p2 += rhs;
	}
	return *this;
}

inline Bbox2& Bbox2::operator+=(const Bbox2& rhs)
{
	if (rhs.is_null()) 	return *this;
	if (is_null()) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	else {
		p1.x_ = min(p1.x_,rhs.p1.x_);
		p1.y_ = min(p1.y_,rhs.p1.y_);
		p2.x_ = max(p2.x_,rhs.p2.x_);
		p2.y_ = max(p2.y_,rhs.p2.y_);
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
	return (xmin() <= p.x_ && p.x_ <= xmax() &&
			ymin() <= p.y_ && p.y_ <= ymax());
}

inline	bool		Bbox2::contains(const Segment2& s) const
{
	return contains(s.p1) && contains(s.p2);
}

inline double		Bbox2::rescale_to_x(const Bbox2& new_box, double x) const
{
	if (p2.x_ == p1.x_) return (new_box.p1.x_ + new_box.p2.x_) * 0.5;
	return new_box.p1.x_ + (x - p1.x_) * (new_box.p2.x_ - new_box.p1.x_) / (p2.x_ - p1.x_);
}

inline double		Bbox2::rescale_to_y(const Bbox2& new_box, double y) const
{
	if (p2.y_ == p1.y_) return (new_box.p1.y_ + new_box.p2.y_) * 0.5;
	return new_box.p1.y_ + (y - p1.y_) * (new_box.p2.y_ - new_box.p1.y_) / (p2.y_ - p1.y_);
}

inline double		Bbox2::rescale_to_xv(const Bbox2& new_box, double x) const
{
	if (p2.x_ == p1.x_) return x;
	return x * (new_box.p2.x_ - new_box.p1.x_) / (p2.x_ - p1.x_);
}

inline double		Bbox2::rescale_to_yv(const Bbox2& new_box, double y) const
{
	if (p2.y_ == p1.y_) return y;
	return y * (new_box.p2.y_ - new_box.p1.y_) / (p2.y_ - p1.y_);
}



inline	Point2	Midpoint2(const Point2& p1, const Point2& p2)	{ return Point2((p1.x_ + p2.x_) * 0.5, (p1.y_ + p2.y_) * 0.5); }

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
		x += at(n).x_;
		y += at(n).y_;
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
	return inside_polygon_pt(begin(),end(),inPoint);
}

inline bool		Polygon2::intersects(const Segment2& inSegment) const
{
	Point2 p;
	for (int i = 0; i < this->size(); i++)
	{
		if(inSegment.intersect(this->side(i),p))
		{
			return true;
		}
	}

	return false;
}

inline bool Polygon2::is_ccw(void) const
{
	if(size() < 3)	return true;
	
	int b = 0;		// best (lexicog highest then rightmost)
	greater_y_then_x	comp;
	
	for(int i = 1; i < size(); ++i)
	if(comp(at(i),at(b)))
		b = i;
		
	int n = next(b);
	int p = prev(b);
	
	if(at(n).y() == at(b).y())	return true;				// next is same height - top is flat, we are CCW
	if(at(p).y() == at(b).y())	return false;				// prev is same height? - top is flat, we are CW
	
	return Vector2(at(p),at(b)).left_turn(Vector2(at(b),at(n)));	// we are truly highest.  Check for left-turn.
}

inline	Bezier2::Bezier2(const Point2& ip1, const Point2& ic, const Point2& ip2) :
	p1(ip1),
	c1(Point2(
			(ip1.x() * 2.0 + ic.x()) / 3.0,
			(ip1.y() * 2.0 + ic.y()) / 3.0)),
	c2(Point2(
			(ip2.x() * 2.0 + ic.x()) / 3.0,
			(ip2.y() * 2.0 + ic.y()) / 3.0)),
	p2(ip2)
{
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
	return Point2(w0 * p1.x_ + w1 * c1.x_ + w2 * c2.x_ + w3 * p2.x_,
				  w0 * p1.y_ + w1 * c1.y_ + w2 * c2.y_ + w3 * p2.y_);
}

inline Vector2 Bezier2::derivative(double t) const
{
	// Derivative taking by putting the formula in Ax^3+Bx^2+Cx+D form and taking 1st derivative.
	if(t == 0.0)
		return Vector2(-3.0 * p1.x_ + 3.0 * c1.x_,
					   -3.0 * p1.y_ + 3.0 * c1.y_);

	if(t == 1.0)
		return Vector2(-3.0 * c2.x_ + 3.0 * p2.x_,
					   -3.0 * c2.y_ + 3.0 * p2.y_);
	return Vector2(
		3.0 * t * t * (       -p1.x_ + 3.0 * c1.x_ - 3.0 * c2.x_ + p2.x_) +
		2.0 * t     * (  3.0 * p1.x_ - 6.0 * c1.x_ + 3.0 * c2.x_		) +
					  ( -3.0 * p1.x_ + 3.0 * c1.x_						),
		3.0 * t * t * (       -p1.y_ + 3.0 * c1.y_ - 3.0 * c2.y_ + p2.y_) +
		2.0 * t     * (  3.0 * p1.y_ - 6.0 * c1.y_ + 3.0 * c2.y_		) +
					  ( -3.0 * p1.y_ + 3.0 * c1.y_						)
	);
}


inline void	Bezier2::subcurve(Bezier2& sub, double t1, double t2) const
{
	DebugAssert(t1 >= 0.0);
	DebugAssert(t2 <= 1.0);
	DebugAssert(t1 < t2);
	
	Bezier2 dummy, sub1;
	if(t1 <= 0.0 && t2 >= 1.0)
		sub = *this;
	else if (t1 <= 0.0)
		partition(sub,dummy,t2);
	else if (t2 >= 1.0)
		partition(dummy, sub, t1);
	else
	{
		partition(sub1, dummy, t2);
		sub1.partition(dummy, sub, t1 / t2);
	}
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
		bounds.p1.x_ = min(bounds.p1.x_,c1.x_);
		bounds.p1.x_ = min(bounds.p1.x_,c2.x_);
		bounds.p2.x_ = max(bounds.p2.x_,c1.x_);
		bounds.p2.x_ = max(bounds.p2.x_,c2.x_);
	}
	if (!y_monotone())
	{
		bounds.p1.y_ = min(bounds.p1.y_,c1.y_);
		bounds.p1.y_ = min(bounds.p1.y_,c2.y_);
		bounds.p2.y_ = max(bounds.p2.y_,c1.y_);
		bounds.p2.y_ = max(bounds.p2.y_,c2.y_);
	}
}


inline int		Bezier2::x_monotone(void) const
{
	// The weighting of the control points is for (1-t) and t...
	// this gives us an explicit equation in terms of x = At^3 + Bt^2 + Ct + D.
	double A =       -p1.x_ + 3.0 * c1.x_ - 3.0 * c2.x_ + p2.x_;
	double B =  3.0 * p1.x_ - 6.0 * c1.x_ + 3.0 * c2.x_;
	double C = -3.0 * p1.x_ + 3.0 * c1.x_;
	double D =		  p1.x_;

	// This is the derivative - which is a quadratic in the form of x = at^2 + bt + c
	double a = 3.0 * A;
	double b = 2.0 * B;
	double c = C;

	double roots[2];
	int num_roots = quadratic_formula(a,b,c,roots);
	if (num_roots > 1 && roots[1] > 0.0 && roots[1] < 1.0)	return 0;
	if (num_roots > 0 && roots[0] > 0.0 && roots[0] < 1.0)	return 0;
															return 1;
}

inline int		Bezier2::y_monotone(void) const
{
	// The weighting of the control points is for (1-t) and t...
	// this gives us an explicit equation in terms of x = At^3 + Bt^2 + Ct + D.
	double A =     -p1.y_ + 3 * c1.y_ - 3 * c2.y_ + p2.y_;
	double B =  3 * p1.y_ - 6 * c1.y_ + 3 * c2.y_;
	double C = -3 * p1.y_ + 3 * c1.y_;
	double D =		p1.y_;

	// This is the derivative - which is a quadratic in the form of x = at^2 + bt + c
	double a = 3 * A;
	double b = 2 * B;
	double c = C;

	double roots[2];
	int num_roots = quadratic_formula(a,b,c,roots);
	if (num_roots > 1 && roots[1] > 0.0 && roots[1] < 1.0)	return 0;
	if (num_roots > 0 && roots[0] > 0.0 && roots[0] < 1.0)	return 0;
															return 1;
}

inline bool	Bezier2::intersect(const Bezier2& rhs, int d) const
{
	Bbox2	lhs_bbox, rhs_bbox;

	this->bounds(lhs_bbox);
	rhs.bounds(rhs_bbox);

	if (d < 0)
	{
		//	return true;    //  In case of the current checked dub-segments connecting to a common node, any curves that connect there under 
							//  an acute angles are in many cases having overlapping bounding boxes without actually crossing. 
							// To avoid these false positive, we finish with check for a straigt segment aproximation here.
							
		Segment2 lhs_seg, rhs_seg;
		lhs_seg.p1 = this->p1;
		lhs_seg.p2 = this->p2;
		rhs_seg.p1 = rhs.p1;
		rhs_seg.p2 = rhs.p2;
		
		if (lhs_seg.p1 != rhs_seg.p1 &&      // check if segments are adjacent, i.e. share a node,
		    lhs_seg.p1 != rhs_seg.p2 &&      // as linear segment cross check returns a false positive here
		    lhs_seg.p2 != rhs_seg.p1 &&      // (unlike the bezier intersect test)
		    lhs_seg.p2 != rhs_seg.p2)
		{		
			Point2 x;
			return lhs_seg.intersect(rhs_seg, x);
		}
		else
			return false;                     // we assume the sub-segments connecting to a common node never intersect.
	}
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

inline bool	Bezier2::is_near(const Point2& p, double d) const
{
	Bbox2	me, pt_box;
	this->bounds(me);
	pt_box = Bbox2(p-Vector2(d,d),p+Vector2(d,d));

	if (!me.overlap(pt_box)) return false;

	if (me.xspan() < d && me.yspan() < d) return true;

	Bezier2	ls, rs;
	this->partition(ls,rs);
	return ls.is_near(p,d) || rs.is_near(p,d);
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

inline int		Bezier2::monotone_regions(double times[4]) const
{
	// Basic idea: we do two derivatives - one of the X cubic and one of the Y.
	// These are quadratic and have up to 2 roots each.  This gives us any
	// point the curve changes directions.
	int ret = 0;

	double Ax =       -p1.x_ + 3.0 * c1.x_ - 3.0 * c2.x_ + p2.x_;
	double Bx =  3.0 * p1.x_ - 6.0 * c1.x_ + 3.0 * c2.x_;
	double Cx = -3.0 * p1.x_ + 3.0 * c1.x_;
	double Dx =		   p1.x_;
	double ax = 3.0 * Ax;
	double bx = 2.0 * Bx;
	double cx =		  Cx;

	double Ay =       -p1.y_ + 3.0 * c1.y_ - 3.0 * c2.y_ + p2.y_;
	double By =  3.0 * p1.y_ - 6.0 * c1.y_ + 3.0 * c2.y_;
	double Cy = -3.0 * p1.y_ + 3.0 * c1.y_;
	double Dy =		   p1.y_;
	double ay = 3.0 * Ay;
	double by = 2.0 * By;
	double cy =		  Cy;

	int res = 0;
	double r1[2], r2[2];
	int o1 = quadratic_formula(ax,bx,cx,r1);
	int o2 = quadratic_formula(ay,by,cy,r2);

	if (o1 > 0 && r1[0] > 0.0 && r1[0] < 1.0)	times[res++] = r1[0];
	if (o1 > 1 && r1[1] > 0.0 && r1[1] < 1.0)	times[res++] = r1[1];

	if (o2 > 0 && r2[0] > 0.0 && r2[0] < 1.0)	times[res++] = r2[0];
	if (o2 > 1 && r2[1] > 0.0 && r2[1] < 1.0)	times[res++] = r2[1];

	return res;
}

inline int		Bezier2::x_monotone_regions(double times[2]) const
{
	int ret = 0;

	double Ax =       -p1.x_ + 3.0 * c1.x_ - 3.0 * c2.x_ + p2.x_;
	double Bx =  3.0 * p1.x_ - 6.0 * c1.x_ + 3.0 * c2.x_;
	double Cx = -3.0 * p1.x_ + 3.0 * c1.x_;
	double Dx =		   p1.x_;
	double ax = 3.0 * Ax;
	double bx = 2.0 * Bx;
	double cx =		  Cx;

	int res = 0;
	double r1[2];
	int o1 = quadratic_formula(ax,bx,cx,r1);
	if (o1 > 0 && r1[0] > 0.0 && r1[0] < 1.0)	times[res++] = r1[0];
	if (o1 > 1 && r1[1] > 0.0 && r1[1] < 1.0)	times[res++] = r1[1];
	return res;
}

inline int Bezier2::y_monotone_regions(double times[2]) const
{
	int ret = 0;

	double Ay =       -p1.y_ + 3.0 * c1.y_ - 3.0 * c2.y_ + p2.y_;
	double By =  3.0 * p1.y_ - 6.0 * c1.y_ + 3.0 * c2.y_;
	double Cy = -3.0 * p1.y_ + 3.0 * c1.y_;
	double Dy =		   p1.y_;
	double ay = 3.0 * Ay;
	double by = 2.0 * By;
	double cy =		  Cy;

	int res = 0;
	double r1[2];
	int o1 = quadratic_formula(ay,by,cy,r1);
	if (o1 > 0 && r1[0] > 0.0 && r1[0] < 1.0)	times[res++] = r1[0];
	if (o1 > 1 && r1[1] > 0.0 && r1[1] < 1.0)	times[res++] = r1[1];
	return res;
}


inline void	Bezier2::bounds(Bbox2& bounds) const
{
	double t[4];
	int n = monotone_regions(t);
	bounds = Bbox2(p1,p2);				// bounding box is extended by the endpoints and any point where it 
	for (int i = 0; i < n; ++i)			// changes monotonicity.  This is equivalent to breaking the curve up into
		bounds += midpoint(t[i]);		// 4 monotone pieces, taking those 4 bounding box endpoints and unioning them all !
}


inline double	Bezier2::y_at_x(double x) const
{
	double Ax =       -p1.x_ + 3.0 * c1.x_ - 3.0 * c2.x_ + p2.x_;
	double Bx =  3.0 * p1.x_ - 6.0 * c1.x_ + 3.0 * c2.x_;
	double Cx = -3.0 * p1.x_ + 3.0 * c1.x_;
	double Dx =		   p1.x_;

	double Ay =       -p1.y_ + 3.0 * c1.y_ - 3.0 * c2.y_ + p2.y_;
	double By =  3.0 * p1.y_ - 6.0 * c1.y_ + 3.0 * c2.y_;
	double Cy = -3.0 * p1.y_ + 3.0 * c1.y_;
	double Dy =		   p1.y_;

	Dx -= x;

	double roots[3];
	int num_roots = cubic_formula(Ax,Bx,Cx,Dx, roots);

	if (num_roots < 1) return p1.y_;

	for (int n = 0; n < num_roots; ++n)
	if (roots[n] >= 0.0 && roots[n] <= 1.0)
		return Ay * roots[n] * roots[n] * roots[n] + By * roots[n] * roots[n] + Cy * roots[n] + Dy;
	return p1.y_;
}

inline double	Bezier2::x_at_y(double y) const
{
	// These are the cubic equation coefficients for X and Y in terms of T, as in
	// x = At^3 + Bt^2 + C^t + D.
	// In other words, we've taken the control points and merged them into coefficients so we can use
	// "standard" analysis formula.
	double Ax =       -p1.x_ + 3.0 * c1.x_ - 3.0 * c2.x_ + p2.x_;
	double Bx =  3.0 * p1.x_ - 6.0 * c1.x_ + 3.0 * c2.x_;
	double Cx = -3.0 * p1.x_ + 3.0 * c1.x_;
	double Dx =		   p1.x_;

	double Ay =       -p1.y_ + 3.0 * c1.y_ - 3.0 * c2.y_ + p2.y_;
	double By =  3.0 * p1.y_ - 6.0 * c1.y_ + 3.0 * c2.y_;
	double Cy = -3.0 * p1.y_ + 3.0 * c1.y_;
	double Dy =		   p1.y_;

	Dy -= y;

	double roots[3];
	int num_roots = cubic_formula(Ay,By,Cy,Dy, roots);

	if (num_roots < 1) return p1.x_;

	for (int n = 0; n < num_roots; ++n)
	if (roots[n] >= 0.0 && roots[n] <= 1.0)
		return Ax * roots[n] * roots[n] * roots[n] + Bx * roots[n] * roots[n] + Cx * roots[n] + Dx;
	return p1.x_;
}

template <typename T>
bool in_order(T& lhs, T& rhs)	// Assure lhs <= rhs
{
	if(lhs > rhs)
	{
		swap(lhs, rhs);
		return true;
	} else
		return false;
}

inline int	Bezier2::t_at_x(double x, double t[3]) const
{
	// If the intercept is out of the bounding box, early exit.
	// For hugely "off curve" values of X, the cubic formula runs out of internal
	// precision and returns bogus T values.
	if(x < p1.x_ && x < p2.x_ && x < c1.x_ && x < c2.x_)
		return 0;
	if(x > p1.x_ && x > p2.x_ && x > c1.x_ && x > c2.x_)
		return 0;

	double Ax =       -p1.x_ + 3.0 * c1.x_ - 3.0 * c2.x_ + p2.x_;
	double Bx =  3.0 * p1.x_ - 6.0 * c1.x_ + 3.0 * c2.x_;
	double Cx = -3.0 * p1.x_ + 3.0 * c1.x_;
	double Dx =		   p1.x_;

	Dx -= x;

	double roots[3];
	int num_roots = cubic_formula(Ax,Bx,Cx,Dx, roots);

	int r = 0;

	for (int n = 0; n < num_roots; ++n)
	if (roots[n] >= 0.0 && roots[n] <= 1.0)
		t[r++] = roots[n];

	if(r > 1) in_order(t[0], t[1]);			// Assure bottom 2 are in order
	if(r > 2) if(in_order(t[1], t[2]))		// Now since we know 1 is bigger than 0, if 2 is OOTO this makes 2 right
					in_order(t[0], t[1]);	// If we moved 1 up to 2, maybe the old 2 needs to go all the wy down to zero?

	#if DEV
		if(r > 1)
			DebugAssert(t[0] < t[1]);
		if(r > 2)
			DebugAssert(t[1] < t[2]);
	#endif

	return r;
}

inline int	Bezier2::t_at_y(double y, double t[3]) const
{
	if(y < p1.y_ && y < p2.y_ && y < c1.y_ && y < c2.y_)
		return 0;
	if(y > p1.y_ && y > p2.y_ && y > c1.y_ && y > c2.y_)
		return 0;

	double Ay =       -p1.y_ + 3.0 * c1.y_ - 3.0 * c2.y_ + p2.y_;
	double By =  3.0 * p1.y_ - 6.0 * c1.y_ + 3.0 * c2.y_;
	double Cy = -3.0 * p1.y_ + 3.0 * c1.y_;
	double Dy =		   p1.y_;

	Dy -= y;

	double roots[3];
	int num_roots = cubic_formula(Ay,By,Cy,Dy, roots);

	int r = 0;

	for (int n = 0; n < num_roots; ++n)
	if (roots[n] >= 0.0 && roots[n] <= 1.0)
		t[r++] = roots[n];

	if(r > 1) in_order(t[0], t[1]);			// Assure bottom 2 are in order
	if(r > 2) if(in_order(t[1], t[2]))		// Now since we know 1 is bigger than 0, if 2 is OOTO this makes 2 right
					in_order(t[0], t[1]);	// If we moved 1 up to 2, maybe the old 2 needs to go all the wy down to zero?


	#if DEV
		if(r > 1)
			DebugAssert(t[0] < t[1]);
		if(r > 2)
			DebugAssert(t[1] < t[2]);
	#endif

	return r;
}

inline double	Bezier2::approx_t_for_xy(double x, double y) const
{
	// There isn't a really good way to do this - so we use sort of a bad way.
	// Basically an X or Y probe may return up to 3 hits if it's "over" the airspace
	// of the curve and we have an S curve.  So when we hit one root in either direction,
	// we take it.  if we hit more than one root in both, we compare each pair of roots
	// and take the X root for which there exists the smallest root-pair distance.
	// For points ON the curve, this difference should be rounding error.
	//
	// For points off the curve...YMMV.
	double x_roots[3], y_roots[3];
	int xc = t_at_x(x,x_roots);
	int yc = t_at_y(y,y_roots);
	
	if(yc == 0 && xc == 0)	return 0.5;
	if(xc == 0) return y_roots[0];
	if(yc == 0) return x_roots[0];
	
	int idx = -1;
	double d = 9.9e9;
	for(int ix = 0; ix < xc; ++ix)
	{
		double xd = 9.9e9;
		for(int iy = 0; iy < yc; ++iy)
		{
			double rel = fabs(x_roots[ix] - y_roots[iy]);
			if(rel < xd)
				xd = rel;
		}
		if(xd < d)
		{
			 d = xd;
			 idx = ix;
		}
	}
	if(idx >= 0)
		return x_roots[idx];
	
	DebugAssert(!"How did we get here?");
	return 0.5;
}










void	TEST_CompGeomDefs2(void);

inline int linear_formula(double a, double b, double roots[1])
{
	if (a == 0.0) return (b == 0.0 ? -1 : 0);
	roots[0] = -b / a;
	return 1;
}

inline int quadratic_formula(double a, double b, double c, double roots[2])
{
	if (a == 0.0) return linear_formula(b,c, roots);

	double radical = b * b - 4.0 * a * c;
	if (radical < 0) return 0;

	if (radical == 0.0)
	{
		roots[0] = -b / (2.0 * a);
		return 1;
	}

	double srad = sqrt(radical);
	roots[0] = (-b - srad) / (2.0 * a);
	roots[1] = (-b + srad) / (2.0 * a);
	if (a < 0.0) swap(roots[0],roots[1]);
	return 2;
}

inline int cubic_formula(double a, double b, double c, double d, double roots[3])
{
	// I got this code from formulas at: http://www.1728.com/cubic2.htm
	if (a == 0.0) return quadratic_formula(b,c,d,roots);

	double f = ((3.0 * c / a) - ((b * b) / (a * a))) / 3.0;
	double g = ((2.0 * b * b * b) / (a * a * a) - (9.0 * b * c) / (a * a) + (27.0 * d) / (a)) / 27.0;
	double h = g * g / 4.0 + f * f * f / 27.0;

	if (f == 0.0 && g == 0.0 && h == 0.0)
	{
		roots[0] = -cbrt(d / a);
		return 1;
	}
	else if (h > 0.0)
	{
		// 1 root
		double R = -(g / 2.0) + sqrt(h);
		double S = cbrt(R);
		double T = -(g / 2.0) - sqrt(h);
		double U = cbrt(T);

		roots[0] = (S + U) - (b / (3.0 * a));
		return 1;
	}
	else
	{
		// 3 roots
		double i = sqrt((g * g / 4.0) - h);
		double j = cbrt(i);
		double k = acos(-(g / (2.0 * i)));
		double l = -j;
		double m = cos(k / 3.0);
		double n = sqrt(3.0) * sin(k / 3.0);
		double p = -b / (3.0 * a);
		roots[0] = 2.0 * j * cos(k / 3.0) - (b / (3.0 * a));
		roots[1] = l * (m + n) + p;
		roots[2] = l * (m - n) + p;
		return 3;
	}
}


template <class __Iterator>
bool inside_polygon_pt(__Iterator begin, __Iterator end, const Point2& inPoint)
{
	int cross_counter = 0;
	Point2		first_p(*begin);
	Segment2	s;
	s.p1 = *begin;
	++begin;

	while (begin != end)
	{
		s.p2 = *begin;
		if ((s.p1.x_ < inPoint.x_ && inPoint.x_ <= s.p2.x_) ||
			(s.p2.x_ < inPoint.x_ && inPoint.x_ <= s.p1.x_))
		if (inPoint.y_ > s.y_at_x(inPoint.x_))
			++cross_counter;

		s.p1 = s.p2;
		++begin;
	}
	s.p2 = first_p;
	if ((s.p1.x_ < inPoint.x_ && inPoint.x_ <= s.p2.x_) ||
		(s.p2.x_ < inPoint.x_ && inPoint.x_ <= s.p1.x_))
	if (inPoint.y_ > s.y_at_x(inPoint.x_))
		++cross_counter;
	return (cross_counter % 2) == 1;
}

template <class __Iterator>
bool inside_polygon_seg(__Iterator begin, __Iterator end, const Point2& inPoint)
{
	int cross_counter = 0;

	while (begin != end)
	{
		// How do we solve the problem of edges?  It turns out the easiest solution is to
		// treat all "on-the-line" ray hits (e.g. the end of the segment is at the horizontal
		// of what we are shooting) as forced to one side.
		// In our case, we are shooting a vertical ray, so points on the line are treated as
		// being to the right.  That means that a line ending on the line is only counted if
		// its other point is the left of the line.

		// Also see p701 of "Computational Geometry Topics" -- if one end is strictly above and the other is on or below...
		// we have done "if one is strictly to the left and the other is on or to the right.
		// The code only has the equal case because the BASIC case ("span") is the "or to the right)

		Segment2 s(*begin);
		if ((s.p1.x_ < inPoint.x_ && inPoint.x_ <= s.p2.x_) ||
			(s.p2.x_ < inPoint.x_ && inPoint.x_ <= s.p1.x_))
		if (inPoint.y_ > s.y_at_x(inPoint.x_))
			++cross_counter;
		++begin;
	}
	return (cross_counter % 2) == 1;
}

template <class __Iterator>
bool inside_polygon_bez(__Iterator begin, __Iterator end, const Point2& inPoint)
{
	int cross_counter = 0;

	while (begin != end)
	{
		Bezier2 b(*begin);
		Bezier2 b1,b2,b3,bp;

		double t[2];
		int r = b.y_monotone_regions(t);
		if (r == 0)
		{
			b1 = b;
		}
		else if (r == 1 || t[1] == 1.0)
		{
			b.partition(b1,b2,t[0]);
		}
		else
		{
			b.partition(b1,bp,t[0]);
			bp.partition(b2,b3,(t[1]-t[0]) / (1.0-t[0]));
		}

		if ((b1.p1.x_ < inPoint.x_ && inPoint.x_ <= b1.p2.x_) ||
			(b1.p2.x_ < inPoint.x_ && inPoint.x_ <= b1.p1.x_))
		if (inPoint.y_ > b1.y_at_x(inPoint.x_))
			++cross_counter;

		if (r > 0)
		if ((b2.p1.x_ < inPoint.x_ && inPoint.x_ <= b2.p2.x_) ||
			(b2.p2.x_ < inPoint.x_ && inPoint.x_ <= b2.p1.x_))
		if (inPoint.y_ > b2.y_at_x(inPoint.x_))
			++cross_counter;

		if (r > 1)
		if ((b3.p1.x_ < inPoint.x_ && inPoint.x_ <= b3.p2.x_) ||
			(b3.p2.x_ < inPoint.x_ && inPoint.x_ <= b3.p1.x_))
		if (inPoint.y_ > b3.y_at_x(inPoint.x_))
			++cross_counter;

		++begin;
	}
	return (cross_counter % 2) == 1;
}

// Simple ccw test: find the [lower] leftmost corner and make sure it is a left turn.
// complexity is only added by avoiding a double-scan of the iterated sequence.
template <class __Iterator>
bool is_ccw_polygon_pt(__Iterator begin, __Iterator end)
{
	if (begin == end) return true;			// Degenerate: 0 pts
	Point2	first(*begin);
	++begin;
	if (begin == end) return true;			// Degenerate: 1 pts
	Point2 second(*begin);
	++begin;
	if (begin == end) return true;			// Degenerate: 2 pts

	Point2	prevprev(first);
	Point2	prev	(second);

	lesser_x_then_y	better;

	Point2	best(first.x_ + 1, first.y_);

	bool	is_ccw = false;

	while(begin != end)
	{
		if (better(prev, best))
		{
			best = prev;
			is_ccw = left_turn(prevprev,prev,*begin);
		}

		prevprev = prev;
		prev = *begin;
		++begin;
	}

	// prevprev, prev, first
	if (better(prev,best))
	{
		best = prev;
		is_ccw = left_turn(prevprev, prev, first);
	}
	// prev, first, second
	if (better(first,best))
	{
		best = first;
		is_ccw = left_turn(prev, first, second);
	}
	return is_ccw;
}

template <class __Iterator>
bool is_ccw_polygon_seg(__Iterator begin, __Iterator end)
{
	if(begin == end)
		return true;

	lesser_x_then_y	better;
		
	__Iterator orig(begin), best(begin), next;

	++begin;
	if(begin == end)
		return true;
		
	while(begin != end)
	{
		if(better(*begin,*best))
			best = begin;
		++begin;
	}
	
	next = best;
	++next;
	if(next == end) 
		next = orig;
		
	return left_turn(best->p1, best->p2,next->p2);
}


template <class __Iterator>
double signed_area_pt(__Iterator begin, __Iterator end)
{
	if(begin == end) return 0.0;
	Point2	o(*begin++);
	if(begin == end) return 0.0;
	Vector2 prev(o, *begin++);
	if(begin == end) return 0.0;
	double total = 0.0;	
	while(begin != end)
	{
		Vector2	curr(o,*begin++);
		total += prev.signed_area(curr);
		prev = curr;
	}
	return total;
}


// Given a single bezier curve, this routine outputs the start and enough intermediate points that the err
// to the curve is less than "err".  Note that the LAST point is NOT emitted!!!  Why?  So that you
// can chain a sequence or ring of these guys together.
template <class __output_iterator>
void approximate_bezier_epsi(const Bezier2& b, double err, __output_iterator output, double t_start, double t_middle, double t_end)
{
	if(t_middle <= t_start || t_middle >= t_end) return;	// No "space"?  Probably too much recursion, just bail.
	
	Segment2	s;
	s.p1=b.midpoint(t_start);
	s.p2=b.midpoint(t_end);
	
	Point2	x = b.midpoint(t_middle);
	if(s.squared_distance_supporting_line(x) > (err*err))
	{
		// We are definitely going to emit X.  Better try two recursions too.
		// This will emit t_start and t_midle (since each call MUST emit a point).		
		approximate_bezier_epsi(b,err,output,t_start,(t_start + t_middle) * 0.5, t_middle);		
		approximate_bezier_epsi(b,err,output,t_middle,(t_middle + t_end) * 0.5, t_end);
	} else {
		// We're done.  We awlays emit t_start - our caller certified that p1 is a good thing to emit.
		*output = s.p1;
		++output;
	}
}

template <class __output_iterator>
void approximate_bezier_epsi_2(const Bezier2& b, const Bezier2& b2, double err, __output_iterator output, __output_iterator output2, double t_start, double t_middle, double t_end)
{
	if(t_middle <= t_start || t_middle >= t_end) return;	// No "space"?  Probably too much recursion, just bail.
	
	Segment2	s;
	Segment2	u;
	s.p1=b.midpoint(t_start);
	s.p2=b.midpoint(t_end);

	u.p1=b2.midpoint(t_start);
	u.p2=b2.midpoint(t_end);
	
	Point2	x = b.midpoint(t_middle);
	if(s.squared_distance_supporting_line(x) > (err*err))
	{
		// We are definitely going to emit X.  Better try two recursions too.
		// This will emit t_start and t_midle (since each call MUST emit a point).		
		approximate_bezier_epsi_2(b,b2,err,output,output2,t_start,(t_start + t_middle) * 0.5, t_middle);		
		approximate_bezier_epsi_2(b,b2,err,output,output2,t_middle,(t_middle + t_end) * 0.5, t_end);
	} else {
		// We're done.  We awlays emit t_start - our caller certified that p1 is a good thing to emit.
		*output  = s.p1;
		*output2 = u.p1;
		++output;
		++output2;
	}
}


// Given a bezier-iterator type input iterator sequence and an output iterator that takes points,
// this translates the entire point sequence, converting curves to epsi as needed.
template <class __input_iterator, class __output_iterator>
void approximate_bezier_sequence_epsi(
						__input_iterator		s,
						__input_iterator		e, 
						__output_iterator		o,
						double					epsi)
{
	pair<Point2,int>	lp;
	Bezier2	b;	
	if(s == e) return;
	lp = *s;
	b.p1 = b.c1 = lp.first;
//	DebugAssert(lp.second == 0);
	++s;
	bool has_1 = false;
	bool has_2 = false;
	while(s != e)
	{
		lp = *s; ++s;
		switch(lp.second) {
		case -1:	b.c2 = lp.first;	has_2 = true; break;
		case 1:		b.c1 = lp.first;	has_1 = true; break;
		case 0:			
			if(!has_1 && !has_2)
			{
				// No curve..emit straight segment.
				*o = b.p1;
				++o;
				// Now remember the last
				has_1=has_2=false;
				b.p1 = lp.first;
			}
			else
			{
				// We have a bezier - patch it up.
				b.p2 = lp.first;
				if(!has_1) b.c1 = b.p1;
				if(!has_2) b.c2 = b.p2;
				approximate_bezier_epsi(b,epsi,o);
				// Now remember the last
				has_1=has_2=false;
				b.p1 = b.p2;
			}			
			break;
//		default:
//			DebugAssert(!"Logice rror");
		}
	}
	// All done - push the last point.
	*o = b.p1;
	++o;
}




#endif
