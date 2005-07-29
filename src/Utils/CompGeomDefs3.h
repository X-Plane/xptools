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
#ifndef COMPGEOMDEFS3_H
#define COMPGEOMDEFS3_H


#include <math.h>
#include <vector>
using std::vector;


/*

	KNOWN BUGS:
	operator== is incorrect for:
		Line3
		Plane3
		

 */

struct	Point3;
struct	Vector3;

/****************************************************************************************************
 * Point3
 ****************************************************************************************************/

struct Point3 {
	Point3() : x(0.0), y(0.0), z(0.0) { }
	Point3(double ix, double iy, double iz) : x(ix), y(iy), z(iz) { }
	Point3(const Point3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) { }

	Point3& operator=(const Point3& rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
	bool operator==(const Point3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
	bool operator!=(const Point3& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }
	
	Point3& operator += (const Vector3& v);
	Point3& operator -= (const Vector3& v);
	Point3 operator+(const Vector3& v) const;
	Point3 operator-(const Vector3& v) const;

	inline double squared_distance(const Point3& p) const { return (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y) + (p.z - z) * (p.z - z); }
	
	double	x;
	double	y;
	double	z;
};

/****************************************************************************************************
 * Vector3
 ****************************************************************************************************/


struct	Vector3 {
	Vector3() : dx(0.0), dy(0.0), dz(0.0) { }
	Vector3(double ix, double iy, double iz) : dx(ix), dy(iy), dz(iz) { }
	Vector3(const Vector3& rhs) : dx(rhs.dx), dy(rhs.dy), dz(rhs.dz) { }
	explicit Vector3(const Point3& rhs) : dx(rhs.x), dy(rhs.y), dz(rhs.z) { }
	Vector3(const Point3& p1, const Point3& p2) : dx(p2.x - p1.x), dy(p2.y - p1.y), dz(p2.z - p1.z) { }
	
	Vector3& operator=(const Vector3& rhs) { dx = rhs.dx; dy = rhs.dy; dz = rhs.dz; return *this; }
	bool operator==(const Vector3& rhs) const { return dx == rhs.dx && dy == rhs.dy && dz == rhs.dz; }
	bool operator!=(const Vector3& rhs) const { return dx != rhs.dx || dy != rhs.dy || dz != rhs.dz; }

	Vector3& operator+= (const Vector3& v) { dx += v.dx; dy += v.dy; dz += v.dz; return *this; }
	Vector3& operator-= (const Vector3& v) { dx -= v.dx; dy -= v.dy; dz -= v.dz; return *this; }
	Vector3& operator*= (double scalar) { dx *= scalar; dy *= scalar; dz *= scalar; return *this; }
	Vector3& operator/= (double scalar) { dx /= scalar; dy /= scalar; dz /= scalar; return *this; }

	Vector3  operator+ (const Vector3& v) const { return Vector3(dx + v.dx, dy + v.dy, dz + v.dz); }
	Vector3  operator- (const Vector3& v) const { return Vector3(dx - v.dx, dy - v.dy, dz - v.dz); }
	Vector3  operator* (double scalar) const { return Vector3(dx * scalar, dy * scalar, dz * scalar); }
	Vector3  operator/ (double scalar) const { return Vector3(dx / scalar, dy / scalar, dz / scalar); }	
	Vector3  operator- (void) const { return Vector3(-dx, -dy, -dz); }
	
	operator Point3() const { return Point3(dx, dy, dz); }
	
	double	squared_length(void) const { return dx * dx + dy * dy + dz * dz; }
	// TODO: do we want to special case unit vectors for perfect normalization?
	void 	normalize(void) { double len = sqrt(dx * dx + dy * dy + dz * dz); if (len != 0.0) { len = 1.0 / len; dx *= len; dy *= len; dz *= len; } }
	
	double	dot (const Vector3& v) const { return dx * v.dx + dy * v.dy + dz * v.dz; }
	Vector3	cross (const Vector3& v) const;
	
	Vector3 projection(const Vector3& v) const;

	double	dx;
	double	dy;
	double	dz;
};	

/****************************************************************************************************
 * Segment3
 ****************************************************************************************************/

struct	Segment3 {	
	Segment3() : p1(), p2() { }
	Segment3(const Point3& ip1, const Point3& ip2) : p1(ip1), p2(ip2) { }
	Segment3(const Point3& p, const Vector3& v) : p1(p), p2(p.x + v.dx, p.y + v.dy, p.z + v.dz) { }
	Segment3(const Segment3& rhs) : p1(rhs.p1), p2(rhs.p2) { }

	Segment3& operator=(const Segment3& rhs) { p1 = rhs.p1; p2 = rhs.p2; return *this; }
	bool operator==(const Segment3& rhs) const { return p1 == rhs.p1 && p2 == rhs.p2; }
	bool operator!=(const Segment3& rhs) const { return p1 != rhs.p1 || p2 != rhs.p2; }

	Segment3& operator+=(const Vector3& rhs) { p1 += rhs; p2 += rhs; return *this; }

	double	squared_length(void) const { return (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) + (p2.z - p1.z) * (p2.z - p1.z); }
	Point3	midpoint(double s=0.5) const { double ms = 1.0 - s; return Point3(p1.x * ms + p2.x * s,p1.y * ms + p2.y * s, p1.z * ms + p2.z * s); }
	Point3	projection(const Point3& pt) const;
	double	squared_distance(const Point3& p) const;
	bool	collinear_has_on(const Point3& p) const;

	Point3	p1;
	Point3	p2;
};

/****************************************************************************************************
 * Line3
 ****************************************************************************************************/

struct Line3 {
	Line3() : p(), v() { }
	Line3(const Point3& ip, const Vector3& iv) : p(ip), v(iv) { }
	Line3(const Point3& p1, const Point3& p2) : p(p1), v(p1, p2) { }
	Line3(const Line3& rhs) : p(rhs.p), v(rhs.v) { }
	explicit Line3(const Segment3& s) : p(s.p1), v(s.p1, s.p2) { }
	
	Line3& operator=(const Line3& rhs) { p = rhs.p; v = rhs.v; return *this; }
	// WARNING: these operators are too restrictive!
	bool operator==(const Line3& rhs) const { return p == rhs.p && v == rhs.v; }
	bool operator!=(const Line3& rhs) const { return p != rhs.p || v != rhs.v; }
	
	bool	on_line(const Point3& p) const;
	bool	parallel(const Line3& l) const;
	Point3	projection(const Point3& p) const;
	
	Point3	p;
	Vector3	v;
};		

/****************************************************************************************************
 * Sphere3
 ****************************************************************************************************/


struct	Sphere3 {
	Sphere3() : c(), radius_squared(0.0) { }
	Sphere3(const Point3& center, double radius) : c(center), radius_squared(radius * radius) { }
	Sphere3(const Sphere3& rhs) : c(rhs.c), radius_squared(rhs.radius_squared) { }

	Sphere3& operator=(const Sphere3& rhs) { c = rhs.c; radius_squared = rhs.radius_squared; return *this; }
	bool operator==(const Sphere3& rhs) const { return c == rhs.c && radius_squared == rhs.radius_squared; }
	bool operator!=(const Sphere3& rhs) const { return c != rhs.c || radius_squared != rhs.radius_squared; }

	bool	contains(const Point3& p) const { return Vector3(c, p).squared_length() <= radius_squared; }
	Sphere3& operator+=(const Vector3& v) { c += v; return *this; }
	Sphere3& operator*=(double scale) { radius_squared *= (scale * scale); return *this; }

	Point3	c;
	double	radius_squared;
};

/****************************************************************************************************
 * Plane3
 ****************************************************************************************************/

// Note: if we wanted Ax + By + Cz + D = 0 form then A = n.dx, B = n.dy, C = n.dz, D = -ndotp.

struct	Plane3 {
	Plane3() : ndotp(0), n() { }
	Plane3(const Point3& point, const Vector3& normal) : n(normal), ndotp(0.0) { n.normalize(); ndotp = n.dot(Vector3(point)); }
	Plane3(const Plane3& rhs) : n(rhs.n), ndotp(rhs.ndotp) { }
	
	Plane3& operator=(const Plane3& rhs) { n = rhs.n; ndotp = rhs.ndotp; return *this; }
	// WARNING: these operators are TOO restrictive for equivalent planes.
	bool operator==(const Plane3& rhs) const { return n == rhs.n && ndotp == rhs.ndotp; }
	bool operator!=(const Plane3& rhs) const { return n != rhs.n || ndotp != rhs.ndotp; }
	
	bool	on_normal_side(const Point3& p) const { return n.dot(Vector3(p)) >  ndotp; }
	bool	on_plane      (const Point3& p) const { return n.dot(Vector3(p)) == ndotp; }
	bool	intersect(const Segment3& s, Point3& p) const;
	bool	intersect(const Line3& l, Point3& p) const;
	bool	intersect(const Plane3& p, Line3& l) const;
	bool	intersect(const Plane3& p1, const Plane3& p2, Point3& p) const;
	double	squared_distance(const Point3& p) const;
	
	Vector3		n;
	double		ndotp;	
};	

/****************************************************************************************************
 * Polygon3
 ****************************************************************************************************/


class	Polygon3 : public vector<Point3> {
public:

	Segment3	side(int n) const { if (n == (size()-1)) return Segment3(at(n),at(0)); return Segment3(at(n), at(n+1)); }
};

/****************************************************************************************************
 * Bezier3
 ****************************************************************************************************/

class	Bezier3 {
public:
	Bezier3() { }
	Bezier3(const Point3& ip1, const Point3& ic1, const Point3& ic2, const Point3& ip2) : p1(ip1), p2(ip2), c1(ic1), c2(ic2) { }
	Bezier3(const Bezier3& x) : p1(x.p1), p2(x.p2), c1(x.c1), c2(x.c2) { }
	Bezier3& operator=(const Bezier3& x) { p1 = x.p1; p2 = x.p2; c1 = x.c1; c2 = x.c2; return *this; }
	
	bool operator==(const Bezier3& x) const { return p1 == x.p1 && p2 == x.p2 && c1 == x.c1 && c2 == x.c2; }
	bool operator!=(const Bezier3& x) const { return p1 != x.p1 || p2 != x.p2 || c1 != x.c1 || c2 != x.c2; }
	
	Point3	midpoint(double t=0.5) const;	
	
	Point3	p1;
	Point3	p2;
	Point3	c1;
	Point3	c2;
};

/****************************************************************************************************
 * Comparison
 ****************************************************************************************************/

struct lesser_y_then_x_then_z {
	bool	operator()(const Point3& lhs, const Point3& rhs) const {
		return (lhs.y == rhs.y) ? ((lhs.x == rhs.x) ? (lhs.z < rhs.z) : (lhs.x < rhs.x)) : (lhs.y < rhs.y);
	}
};

struct greater_y_then_x_then_z {
	bool	operator()(const Point3& lhs, const Point3& rhs) const {
		return (lhs.y == rhs.y) ? ((lhs.x == rhs.x) ? (lhs.z > rhs.z) : (lhs.x > rhs.x)) : (lhs.y > rhs.y);
	}
};

/****************************************************************************************************
 * Inline Funcs
 ****************************************************************************************************/


inline Point3& Point3::operator += (const Vector3& v) { x += v.dx; y += v.dy; z += v.dz; return *this; }
inline Point3& Point3::operator -= (const Vector3& v) { x -= v.dx; y -= v.dy; z -= v.dz; return *this; }
inline Point3 Point3::operator+(const Vector3& v) const { return Point3(x + v.dx, y + v.dy, z + v.dz); }
inline Point3 Point3::operator-(const Vector3& v) const { return Point3(x - v.dx, y - v.dy, z - v.dz); }

inline	Vector3	Vector3::cross (const Vector3& v) const
{
	return Vector3(
		dy * v.dz - dz * v.dy,
		dz * v.dx - dx * v.dz,
		dx * v.dy - dy * v.dx);
}

inline Vector3 Vector3::projection(const Vector3& rhs) const 
{
	return (*this) * dot(rhs) / dot(*this);	
}


inline Point3	Segment3::projection(const Point3& pt) const
{
	// Simply find the projection of a vector from p1 to pt along a vector that is
	// our length.  Offset from p1, go home happy.
	return p1 + Vector3(p1,p2).projection(Vector3(p1,pt));
}

inline double	Segment3::squared_distance(const Point3& p) const 
{
	return Segment3(p, projection(p)).squared_length(); 
}

inline bool	Segment3::collinear_has_on(const Point3& p) const
{
	if (Vector3(p1, p2).dot(Vector3(p1, p)) < 0.0) return false;
	if (Vector3(p2, p1).dot(Vector3(p2, p)) < 0.0) return false;
	return true;
}


inline bool	Line3::on_line(const Point3& pt) const
{
	// BAS Question: is the third check here necessary, or does transitivity cover this case?!?
	// BAS question: shbould thsi be written in terms of dot products?!?
	Vector3	mv(p,pt);
	return  (mv.dx * v.dy == mv.dy * v.dx) &&
			(mv.dz * v.dy == mv.dy * v.dz) &&
			(mv.dx * v.dz == mv.dz * v.dx);
}

inline bool	Line3::parallel(const Line3& l) const
{
	return  (l.v.dx * v.dy == l.v.dy * v.dx) &&
			(l.v.dz * v.dy == l.v.dy * v.dz) &&
			(l.v.dx * v.dz == l.v.dz * v.dx);
}

inline Point3	Line3::projection(const Point3& pt) const
{
	return p + v.projection(Vector3(p,pt));
}


inline bool	Plane3::intersect(const Segment3& s, Point3& p) const
{
	// See definition below - segment and lnie intersect are basically the same.
	double	denom = n.dot(Vector3(s.p1, s.p2));
	if (denom == 0.0) return false;
	double	num = ndotp - n.dot(Vector3(s.p1));
	
	double	t = num / denom;
	
	if (t < 0.0 || t > 1.0) return false;
	
	double	mt = 1.0 - t;
	
	p = Point3(s.p1.x * mt + s.p2.x * t,
			   s.p1.y * mt + s.p2.y * t,
			   s.p1.z * mt + s.p2.z * t);
	return true;
}

inline bool	Plane3::intersect(const Line3& l, Point3& p) const
{
	// How does this work?  Well, basically when N dot vector(P0, Lp + t Lv) == 0
	// that's when the point Lp + t Lv (a parametric point on line L) is on the plane
	// defined by N dot P0.
	// First - compare the dir of our normal to the dir of our line.  If they're
	// at a rigiht angle, we have a line parallel to the plane.
	double	denom = n.dot(l.v);
	if (denom == 0.0) return false;
	
	// Now lets look at that equation N dot vector(P0, Lp + t Lv), remembering that the
	// dot product, bless it, is associative.
	//
	// N dot (vector(P0, Lp + t Lv)
	// N dot (Lp + t Lv) - N dot P0 = 0
	// N dot Lp + t n dot LV - N dot P0 = 0
	// t = ((-n dot p0) + (n dot L p)) / (n dot LV)

	// So our numerator is the dot product of the normal and the anchor point of our plane
	// - this is ndotp in our object.  Subraact from that the dot product of te normal and a point on the line.
	// The denominator is the dot product of the normal and the line vector.

	double	num = ndotp - n.dot(Vector3(l.p));
	
	double	t = num / denom;
	
	p =  Point3(l.p + l.v * t);	
	return true;
}

inline	bool Plane3::intersect(const Plane3& pl, Line3& line) const
{
	// First a quick check - if we're parallel, we know we're borked.
	if (n == pl.n) return false;
	
	// The determinant - this is basically telling us if our normal vector
	// is 0-length - another test for parallel.
	double	determinant = (n.dot(n)) * (pl.n.dot(pl.n)) - n.dot(pl.n) * n.dot(pl.n);
	if (determinant == 0.0) 
		return false;
		
	// Um, it is NOT clear to me why this works.
	double	c1 = ndotp * (pl.n.dot(pl.n)) - pl.ndotp * (n.dot(pl.n));
	double  c2 = pl.ndotp * (n.dot(n)) - ndotp * (n.dot(pl.n));
	c1 /= determinant;
	c2 /= determinant;
	
	line.p = Point3(n * c1 + pl.n * c2);
	// This makes sense though - cross the normals of the two planes to get
	// the intersection line's direction.
	line.v = n.cross(pl.n);
	return true;
}

inline bool	Plane3::intersect(const Plane3& p1, const Plane3& p2, Point3& p) const
{
//
//                                               a b c
//In general, the inverse matrix of a 3X3 matrix d e f
//                                               g h i
//
//is 
//
//            1                       (ei-fh)   (bi-ch)   (bf-ce)
//-----------------------------   x   (fg-di)   (ai-cg)   (cd-af)
//a(ei-fh) - b(di-fg) + c(dh-eg)      (dh-eg)   (bg-ah)   (ae-bd)
//
//http://www.everything2.com/index.pl?node_id=1271704


	// 3-way plane intersect.  Two planes form a line, so a third plane cuts this to a point, unless any two planes
	// are paralle....in that case, we have no intersection or a full plane or line of intersections.
	// We can do this by solving 3 parallel equations (Ax + By + Cz + D = 0) to get a single intersection point.

	// We do this using the matrix method: the vector (x,y,z) * M (3x3 coefficients) forms a vector of the D params.
	// We invert the vector to get the final result.
	
	// Treat the original equation as:
	// [X] [a b c]   [j]
	// [Y] [d e f] = [k]
	// [Z] [g h i]   [l]
	
	// We'll find teh inverse of the 3x3, call it a_p -> i->p, then multiply by jkl to get the XYZ point.

// BEN SEZ: WE HAVE A BAD TERM - SHOULD BE
//            1                       (ei-fh)   (ch-bi)   (bf-ce)
//-----------------------------   x   (fg-di)   (ai-cg)   (cd-af)
//a(ei-fh) - b(di-fg) + c(dh-eg)      (dh-eg)   (bg-ah)   (ae-bd)
//
	
#define	M_a	n.dx
#define M_b n.dy
#define M_c n.dz
#define M_d p1.n.dx
#define M_e p1.n.dy
#define M_f p1.n.dz
#define M_g p2.n.dx
#define M_h p2.n.dy
#define M_i p2.n.dz  
#define M_j	   ndotp
#define M_k p1.ndotp
#define M_l p2.ndotp

	// NOTE: (why is ndotp positive?  Because this solves Ax + By + Cz = D, NOT Ax + By + Cz + D = 0

	// The determinant is the divisor of the matrix we will calculate...if it is zero, 
	// then there is no intersection!  Note that all inputs come from the normals - we're 
	// checking for parallel planes here!
	double	det = 		M_a * (M_e * M_i - M_f * M_h) - 
						M_b * (M_d * M_i - M_f * M_g) + 
						M_c * (M_d * M_h - M_e * M_g);
	if (det == 0.0) return false;
	det = 1.0 / det;
	
	double	I_a = M_e * M_i - M_f * M_h;
	double	I_b = M_c * M_h - M_b * M_i;
	double	I_c = M_b * M_f - M_c * M_e;
	double	I_d = M_f * M_g - M_d * M_i;
	double	I_e = M_a * M_i - M_c * M_g;
	double	I_f = M_c * M_d - M_a * M_f;
	double	I_g = M_d * M_h - M_e * M_g;
	double	I_h = M_b * M_g - M_a * M_h;
	double	I_i = M_a * M_e - M_b * M_d;
	
	// Now we can multiply JKL by the inverse matrix to get the final point.
	
	p.x = det * (M_j * I_a + M_k * I_b + M_l * I_c);
	p.y = det * (M_j * I_d + M_k * I_e + M_l * I_f);
	p.z = det * (M_j * I_g + M_k * I_h + M_l * I_i);
	return true;
}

inline	double Plane3::squared_distance(const Point3& p) const
{
	double num = n.dot(Vector3(p)) - ndotp;
	return num * num / (n.dot(n));
}

inline	Point3	Bezier3::midpoint(double t) const
{
	double nt = 1.0 - t;
	double w0 = nt * nt * nt;
	double w1 = 3.0 * nt * nt * t;
	double w2 = 3.0 * nt * t * t;
	double w3 = t * t * t;
	return Point3(w0 * p1.x + w1 * c1.x + w2 * c2.x + w3 * p2.x,
				  w0 * p1.y + w1 * c1.y + w2 * c2.y + w3 * p2.y,
				  w0 * p1.z + w1 * c1.z + w2 * c2.z + w3 * p2.z);
}



#endif

