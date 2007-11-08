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
#include "CompGeomUtils.h"
#include "CompGeomDefs2.h"
#include "MapDefs.h"
#include "AssertUtils.h"
#include "XESConstants.h"
#if DEV
#  include <stdio.h>
#endif
#include <algorithm>
using namespace std;

Point2	CoordTranslator::Forward(const Point2& input)
{
	return Point2(
			mDstMin.x + (input.x - mSrcMin.x) * (mDstMax.x - mDstMin.x) / (mSrcMax.x - mSrcMin.x),
			mDstMin.y + (input.y - mSrcMin.y) * (mDstMax.y - mDstMin.y) / (mSrcMax.y - mSrcMin.y));
}
Point2	CoordTranslator::Reverse(const Point2& input)
{
	return Point2(
			mSrcMin.x + (input.x - mDstMin.x) * (mSrcMax.x - mSrcMin.x) / (mDstMax.x - mDstMin.x),
			mSrcMin.y + (input.y - mDstMin.y) * (mSrcMax.y - mSrcMin.y) / (mDstMax.y - mDstMin.y));
}


inline double	cot(double theta) { return cos(theta) / sin(theta); }
inline double	csc(double theta) { return 1.0        / sin(theta); }

// Calculate the angle from p1 to p2 to p3 going counterclockwise around
// the wedge, regardless of whether it is accute, obtuse, etc.
static	double	CCW_Angle(const Point2& p1, const Point2& p2, const Point2& p3)
{
	Vector2 v1(p1, p2);
	Vector2 v2(p2, p3);
	
	double	a1 = atan2(v1.dy, v1.dx);
	double	a2 = atan2(v2.dy, v2.dx);
	
	double a_turn = PI - (a2 - a1);

	if (a_turn > PI)
		a_turn -= PI2;
	
	return a_turn;
}

// Given a segment, move it to the left (based on its directionality) by a distance.
static	void	MoveSegLeft(const Segment2& l1, double dist, Segment2& l2)
{
	Vector2	v = Vector2(l1.p1, l1.p2).perpendicular_ccw();
	v.normalize();
	v *= dist;
	l2 = Segment2(l1.p1 + v, l1.p2 + v);
}

static	void	MoveSegLeft(const Segment3& l1, double dist, Segment3& l2, const Vector3& inUp)
{
	Vector3	v = inUp.cross(Vector3(l1.p1, l1.p2));
	v.normalize();
	v *= dist;
	l2 = Segment3(l1.p1 + v, l1.p2 + v);
}


void	InsetPolygon2(
				const Polygon2&				inChain,
				const double *				inRatios,
				double						inInset,
				bool						inIsRing,
				Polygon2&					outChain,
				void						(* antennaFunc)(int n, void * ref),
				void *						ref)
{
	if (!outChain.empty())
		outChain.clear();
	
	int n = 0;
	vector<Segment2>	segments, orig_segments;
	
	// First we calculate the inset edges of each side of the polygon.

	for (int n = 0, m = 1; n < inChain.size(); ++n, ++m)
	{
		Segment2	edge(inChain[n], inChain[m % inChain.size()]);
		orig_segments.push_back(edge);
		Segment2	seg;
		MoveSegLeft(edge, (inRatios == NULL) ? inInset : (inRatios[n] * inInset), seg);
		segments.push_back(seg);
	}
	
	// Now we go through and find each vertex, the intersection of the supporting
	// lines of the edges.  For the very first and last point if not in a polygon,
	// we don't use the intersection, we just find where that segment ends for a nice
	// crips 90 degree angle.
	
	int num_inserted = 0;	
	int last_vertex = segments.size() - 1;
	
	for (int outgoing_n = 0; outgoing_n < segments.size(); ++outgoing_n)
	{
		// the Nth segment goes from the Nth vertex to the Nth + 1 vertex.
		// Therefore it is the "outgoing" segment.
		int 				incoming_n = outgoing_n - 1;
		if (incoming_n < 0)	incoming_n = last_vertex;

		/* We are going through vertex by vertex and determining the point(s) added
		 * by each pair of sides.  incoming is the first side and outgoing is the second
		 * in a CCW rotation.  There are 5 special cases:
		 *
		 * (1) The first point in a non-ring is determined only by the second side.
		 * (2) the last point in a non-ring is determined only by the first side.
		 * (3) If we have a side that overlaps exactly backward onto itself, we generate two
		 *     points to make a nice square corner around this 'antenna'.  Please note the 
		 *     requirement that both sides be the same length!!
		 * (4) If two sides are almost colinear (or are colinear) then the intersection we would
		 *     normally use to find the intersect point will have huge precision problems.  In 
		 *     this case we take an approximate point by just treating it as straight and splitting
		 *     the difference.  The inset will be a bit too thin, but only by a fractional amount that
		 *     is close to our precision limits anyway.
		 * (5) If two sides are an outward bend over sixty degrees, the bend would produce a huge jagged
		 *     sharp end.  We "mitre" this end by adding two points to prevent absurdity.
		 *
		 * GENERAL CASE: when all else fails, we inset both sides, and intersect - that's where the inset
		 * polygon turns a corner. 
		 *
		 *****/

		if (outgoing_n == 0 && !inIsRing)
		{
			/* CASE 1 */
			// We're the first in a chain.  Outgoing vertex is always right.
			outChain.insert(outChain.end(), segments[outgoing_n].p1);
		} 
		else if (outgoing_n == last_vertex && !inIsRing)
		{
			/* CASE 2 */
			// We're the last in a chain.  Incoming vertex is always right
			outChain.insert(outChain.end(), segments[incoming_n].p2);			
		}  
		else if (orig_segments[incoming_n].p1 == orig_segments[outgoing_n].p2) 
		{
			/* CASE 3 */
			// Are the two sides in exactly opposite directions?  Special case...we have to add a vertex.
			// (This is almost always an "antenna" in the data, that's why we have to add the new side, the point of the antenna
			// becomes thick.  Since antennas have equal coordinates, an exact opposite test works.)
			Segment2	new_side(segments[incoming_n].p2, segments[outgoing_n].p1), new_side2;
			MoveSegLeft(new_side, (inRatios != NULL) ? (inRatios[outgoing_n] * inInset) : inInset, new_side2);
//			new_side2 = new_side;
			outChain.insert(outChain.end(), new_side2.p1);
			outChain.insert(outChain.end(), new_side2.p2);
			if (antennaFunc) antennaFunc(outgoing_n + (num_inserted++), ref);
		} else {

			// These are the intersecting cases - we need a dot product to determine what to do.
			Vector2 v1(segments[incoming_n].p1,segments[incoming_n].p2);
			Vector2 v2(segments[outgoing_n].p1,segments[outgoing_n].p2);
			v1.normalize();
			v2.normalize();
			double dot = v1.dot(v2);
						
			if (dot > 0.999961923064)
			{
				/* CASE 4 */
				// Our sides are nearly colinear - don't trust intersect!
				outChain.insert(outChain.end(), Segment2(segments[incoming_n].p2, segments[outgoing_n].p1).midpoint());
			} 
			else if (dot < -0.5 && !v1.left_turn(v2))
			{
				/* CASE 5 */
				// A sharp outward turn of more than 60 degrees - at this point the intersect point will be over
				// twice the road thickness from the intersect point.  Not good!  
				Point2	p1(segments[incoming_n].p2);
				Point2	p2(segments[outgoing_n].p1);
				p1 += (v1 * ((inRatios == NULL) ? 1.0 : inRatios[outgoing_n]) *  inInset);
				p2 += (v2 * ((inRatios == NULL) ? 1.0 : inRatios[outgoing_n]) * -inInset);
				outChain.insert(outChain.end(), p1);
				outChain.insert(outChain.end(), p2);
				if (antennaFunc) antennaFunc(outgoing_n + (num_inserted++), ref);				
			}
			else  
			{
				/* GENERAL CASE */
				// intersect the supporting line of two segments.
				Line2	line1(segments[incoming_n]);
				Line2	line2(segments[outgoing_n]);
				Point2	p;
				if (line1.intersect(line2, p))
					outChain.insert(outChain.end(), p);
				else
					outChain.insert(outChain.end(), Segment2(segments[incoming_n].p2, segments[outgoing_n].p1).midpoint());			
			}
		}
	}	
}				

void	InsetPolygon3(
				const Polygon3&				inChain,
				const double *				inRatios,
				double						inInset,
				bool						inIsRing,
				const Vector3&				inUp,
				Polygon3&					outChain)
{
	if (!outChain.empty())
		outChain.clear();
	
	int n = 0;
	vector<Segment3>	segments, orig_segments;
	
	// First we calculate the inset edges of each side of the polygon.

	for (int n = 0, m = 1; n < inChain.size(); ++n, ++m)
	{
		Segment3	edge(inChain[n], inChain[m % inChain.size()]);
		orig_segments.push_back(edge);
		Segment3	seg;
		MoveSegLeft(edge, (inRatios == NULL) ? inInset : (inRatios[n] * inInset), seg, inUp);
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
			outChain.insert(outChain.end(), segments[outgoing_n].p1);
		} else if (outgoing_n == last_vertex && !inIsRing)
		{
			// Special case 2: we're the last in a chain.  Incoming vertex is always right
			outChain.insert(outChain.end(), segments[incoming_n].p2);			
		}  else if (orig_segments[incoming_n].p1 == orig_segments[outgoing_n].p2) {
			// Special case 3: are the two sides in exactly opposite directions?  Special case...we have to add a vertex.
			// (This is almost always an "antenna" in the data, that's why we have to add the new side, the point of the antenna
			// becomes thick.  Since antennas have equal coordinates, an exact opposite test works.)
			Segment3	new_side(segments[incoming_n].p2, segments[outgoing_n].p1), new_side2;
			MoveSegLeft(new_side, (inRatios != NULL) ? (inRatios[outgoing_n] * inInset) : inInset, new_side2, inUp);
			outChain.insert(outChain.end(), new_side2.p1);
			outChain.insert(outChain.end(), new_side2.p2);			
		}else {
			// General case: intersect the supporting line of two segments.
			// Note: we can't intersect 2 lines reliably so much...make line2 into a plane.
			Line3	line1(segments[outgoing_n]);
			Line3	line2(segments[incoming_n]);
			Vector3	plane2normal = Vector3(segments[incoming_n].p1,segments[incoming_n].p2).cross(inUp);
			Plane3	plane2(segments[incoming_n].p2, plane2normal);
			Point3	p;
			if (plane2.intersect(line1, p))
				outChain.insert(outChain.end(), p);
		}
	}	
}				


double	CalcMaxInset(
				const Polygon2&				inChain,
				const double *				inRatios,
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
		int	prev = n-1;
		if (prev < 0) prev = inChain.size() - 1;
		int next = n+1;
		if (next >= inChain.size()) next = 0;
		int nextnext = next+1;
		if (nextnext >= inChain.size()) nextnext = 0;
	
		double	D = (inRatios == NULL) ? 1.0 : inRatios[n];
		double 	D1 = (inRatios == NULL) ? 1.0 : inRatios[prev];
		double  D2 = (inRatios == NULL) ? 1.0 : inRatios[next];
		
		double	A1 = CCW_Angle(inChain[prev], inChain[n], inChain[next]);
		double	A2 = CCW_Angle(inChain[n], inChain[next], inChain[nextnext]);
		
		double	divisor = 0;
		if (inIsRing || n != 0)
			divisor += (D * cot(A1) + D1 * csc(A1));
		if (inIsRing || n != (sides - 1))
			divisor += (D * cot(A2) + D1 * csc(A2));
			
		if (divisor > 0.0)
		{
			double seglen = sqrt(Segment2(inChain[n], inChain[next]).squared_length());
			double f = seglen / divisor;
			if (n == 0 || f < best_f)
				best_f = f;
		}
	}
	return best_f;
}

#pragma mark -

void ExtendBoundingSphereToSphere(const Sphere3& newSphere, Sphere3& ioSphere)
{
	if (newSphere.radius_squared == 0.0) return;
	if (ioSphere.radius_squared == 0.0) { ioSphere = newSphere; return; }
	Sphere3 smaller(ioSphere);
	smaller.radius_squared = sqrt(ioSphere.radius_squared) - sqrt(newSphere.radius_squared);
	smaller.radius_squared *= smaller.radius_squared;
	if (smaller.contains(newSphere.c)) return;
	
	// Find a vector from the old to the new scale, and normalize it for direction.
	Vector3	to_new = Vector3(ioSphere.c, newSphere.c);
	to_new.normalize();
	// Build vectors in the same and opposite direction of radius length - 
	// these go to the farthest points.
	Vector3 new_r(to_new), old_r(to_new);
	old_r *= -sqrt(ioSphere.radius_squared);
	new_r *=  sqrt(newSphere.radius_squared);
	// Build a segment spanning the diameter of the new sphere.
	// Cut it in half to get the center and radius
	Segment3	seg(newSphere.c + new_r, ioSphere.c + old_r);
	ioSphere.c = seg.midpoint();
	seg.p1 = ioSphere.c;
	ioSphere.radius_squared = seg.squared_length();	
}

// Given a bounding sphere and a point not in the sphere, grow the sphere
// to contain it.  We have this routine broken out so that we don't 
// have to make a function call for included points in the fast bounding
// sphere routine below.
void ExtendBoundingSphereToPt1(const Point3& p, Sphere3& ioSphere)
{
	// Start with a vector from the sphere to the new point.
	Vector3	to_pt = Vector3(ioSphere.c, p);
	// Normalize it to a unit vector in the direction of our sphere.  This is also
	// a vector toward a pt on the sphere that is closest to the new pt.
	to_pt.normalize();
	// Now rescale it to go the other way and to the edge of the sphere.  This is the
	// now pointing at the pt farthest from the new pt on the sphere.
	to_pt *= -sqrt(ioSphere.radius_squared);
	Point3	opposite_pt = ioSphere.c + to_pt;
	// The sphere must contain opposite_pt and p, so put the midpoint halfway and 
	// recalc the bounds.	
	ioSphere.c = Segment3(opposite_pt, p).midpoint();
	ioSphere.radius_squared = Vector3(ioSphere.c, p).squared_length();
}

void ExtendBoundingSphereToPt(const Point3& p, Sphere3& ioSphere)
{
	if (!ioSphere.contains(p))
		ExtendBoundingSphereToPt1(p, ioSphere);
}

void	FastBoundingSphere(
				const vector<Point3>&		inPoints,
				Sphere3&					outSphere)
{
	if (inPoints.empty())	return;
	
	// sort the points first so we can take the widest span
	// to build the sphere.
	vector<Point3>	pts(inPoints);
	sort(pts.begin(), pts.end(), lesser_y_then_x_then_z());
	
	// Make an initial sphere from the two ends...this will hopefully
	// contain almost everyone.
	outSphere.c = Segment3(pts[0], pts[pts.size()-1]).midpoint();
	outSphere.radius_squared = Vector3(outSphere.c, pts[0]).squared_length();
	
	// Now grow the sphere for any points we missed.
	for (int n = 1; n < (pts.size() - 1); ++n)
	{
		if (!outSphere.contains(pts[n]))
			ExtendBoundingSphereToPt1(pts[n], outSphere);
	}
}

bool	PointInPolygon3(
				const Polygon3&				inPolygon,
				const Point3&				inPoint)
{
	// Our strategy is this: for each side of the polygon, 
	// build a plane normal to the polygon along that edge and see if 
	// the point is on the inside of it.  Note that we must have a convex
	// polygon for this to work!

	if (inPolygon.size() < 3)
		return false;
		
	// First find the normal for the polygon.  This normal points
	// toward a viewer looking at the clockwise face.
	Vector3	v1 = Vector3(inPolygon[0], inPolygon[1]);
	Vector3	v2 = Vector3(inPolygon[2], inPolygon[1]);
	Vector3	polyNormal = v1.cross(v2);
	polyNormal.normalize();
	
	for (int n = 0; n < inPolygon.size(); ++n)
	{
		// Construct a half-plane throught this side of the polygon perpendicular
		// to the polygon itself.  Do this by crossing the side with the polygon normal.
		// The half-plane is pointing away from the inside of the polygon.
		Vector3	side = Vector3(inPolygon[n], inPolygon[(n+1)%inPolygon.size()]);
		Vector3 faceVector = polyNormal.cross(side);
		faceVector.normalize();				
		double	d = -faceVector.dot(Vector3(inPolygon[n]));
		
		// If the point is on the positive side of the half-plane, it's outside
		// the polygon; bail.
		double	eq = faceVector.dot(Vector3(inPoint)) + d;
		if (eq > 0)
			return false;		
	}	
	return true;
}

/*
	AN EXPLANATION OF THE PINCHING PROBLEM

	Normally rendering a road path is a two-step process:
	1. The set of segments and control points are converted into a set of simple segments with many
	   small segments forming the curve, using this function, BezierCurve.
	2. Those points and a series of width and up vectors are fed into ChainToQuadStrip to form a
	   3-d quad strip that can then be rendered (or covered with pavement.
	
	The problem happens when we combine (1) a bezier curve with many polygons for smoothness with (2)
	a very sharp hard turn in the curve.
	
	Normallly ChainToQuadStrip intersects two adjacent quads to form a sharp corner...for example in
	a 90 degree turn the two quads around the turn are angled so the "inside" of the turn quads are 
	reduced in length and the outside of the turn is extended, forming an L-joint that flows smoothly.
	
	The problem happens when the segments leading up to a hard turn are very short (typically due to a 
	very detailed bezier curve).  The result is that many quads overlap each other, and ChainToQuadStrip
	cannot unpinch them all the way it would need to.  Instead it ends up causing part of the road to
	go inside out.
	
	Our _hacky_ solution:
	
	"Doctor it hurts when I do this."
	"Well, then don't do this."
	
	You can pass a length to BezierCurve...this is an initial length of each side of the curve that 
	_must_ be made of one straight segment.  This reduces the number of total segments.  The result is
	that when a tight corner hits, if your "protected" length is wider than the effective angled 
	amount of the road on the inside of the curve, you are guaranteed only one quad on each side for
	that length and no pinching problem happens.
	
	How long should that length be?  Well, it should for our side be:
	width_us / tan(angle) + width_them / sin(angle)
	
*/	

void	BezierCurve(
				const Point3&				inStart,
				const Point3&				inEnd,
				bool						inHasStartCurve,
				bool						inHasEndCurve,
				const Point3&				inStartCurve,
				const Point3&				inEndCurve,
				int							inNumSegments,
				double						inProtectStart,
				double						inProtectEnd,
				vector<Point3>&				outPoints)
{
	if (inNumSegments < 1) inNumSegments = 1;
	if (!inHasStartCurve && !inHasEndCurve) inNumSegments = 1;
	
	outPoints.clear();
	outPoints.push_back(inStart);
	
	Point3	startCurve = inHasStartCurve ? inStartCurve : inStart;
	Point3	endCurve =   inHasEndCurve   ? inEndCurve   : inEnd;
	
	if (inNumSegments > 1)
	{
		Line3	base_line(inStart, inEnd);
		if (base_line.on_line(startCurve) && base_line.on_line(endCurve))
			inNumSegments = 1;
	}
	
	inProtectStart *= inProtectStart;
	inProtectEnd *= inProtectEnd;
	
	for (int t = 1; t < inNumSegments; ++t)
	{
		float	p = (float) t / (float) inNumSegments;
		float	mp = 1.0 - p;
		
		float	w0 = mp * mp * mp;
		float	w1 = 3.0 * mp * mp * p;
		float	w2 = 3.0 * mp * p  * p;
		float	w3 = p  * p  * p;
		
		Point3	loc(
			inStart.x * w0 + startCurve.x * w1 + endCurve.x * w2 + inEnd.x * w3,
			inStart.y * w0 + startCurve.y * w1 + endCurve.y * w2 + inEnd.y * w3,
			inStart.z * w0 + startCurve.z * w1 + endCurve.z * w2 + inEnd.z * w3);
			
		bool	skip = false;	
		if (inProtectStart > 0.0 && Segment3(inStart, loc).squared_length() < inProtectStart)
			skip = true;
		if (inProtectEnd > 0.0 && Segment3(inEnd, loc).squared_length() < inProtectEnd)
			skip = true;
		if (!skip)
			outPoints.push_back(loc);
	}

	outPoints.push_back(inEnd);
}

#define NEAR_COLINEAR 0.9995

void	ChainToQuadStrip(
				const vector<Point3>&		inChain,
				const vector<Vector3>&		inUp,
				const vector<double>&		inWidth,
				vector<Point3>&				outQuadStrip)
{
	Vector3	centerline, right;

	if (inChain.size() < 2) return;
	outQuadStrip.clear();
	int	last = inChain.size() - 1;

	for (int n = 0; n < inChain.size(); ++n)
	{
		if (n == 0)
		{
			centerline = Vector3(inChain[0], inChain[1]);
			right = centerline.cross(inUp[0]);
			right.normalize();
			right *= (inWidth[0] * 0.5);
			
			outQuadStrip.push_back(inChain[n] + right);
			outQuadStrip.push_back(inChain[n] - right);
		} else if (n == last) {
			centerline = Vector3(inChain[last-1], inChain[last]);
			right = centerline.cross(inUp[last]);
			right.normalize();
			right *= (inWidth[last] * 0.5);
			
			outQuadStrip.push_back(inChain[n] + right);
			outQuadStrip.push_back(inChain[n] - right);
		} else {
			
			// First we construct four planes...these planes define
			// the sides of the outgoing and incoming segments.
			Vector3 centerline1(inChain[n-1], inChain[n  ]);
			Vector3 centerline2(inChain[n  ], inChain[n+1]);
			centerline1.normalize();
			centerline2.normalize();
			Vector3 normal1 = centerline1.cross(inUp[n]);
			Vector3 normal2 = centerline2.cross(inUp[n]);
			normal1.normalize();
			normal2.normalize();

			// SPECIAL CASE: if the noramsl are near colinear basically
			// that means that we have a segment that really isn't turning
			// at all.  Using an intersection to calculate the edges is a BAD
			// idea - intersect is imprecise for nearly parallel lines.  So
			// just punt and use one set of points.
			if (normal1.dot(normal2) > NEAR_COLINEAR)
			{
				outQuadStrip.push_back(inChain[n] - normal1);
				outQuadStrip.push_back(inChain[n] + normal1);				
			} else if (normal1.dot(normal2) < -NEAR_COLINEAR) {
				outQuadStrip.push_back(inChain[n] - normal1);
				outQuadStrip.push_back(inChain[n] + normal1);				
				outQuadStrip.push_back(inChain[n] - normal2);
				outQuadStrip.push_back(inChain[n] + normal2);								
			} else {
			
			normal1 *= (inWidth[n] * 0.5);
			normal2 *= (inWidth[n] * 0.5);
			
			Point3	left1 = inChain[n] - normal1;
			Point3	left2 = inChain[n] - normal2;
			Point3	right1 = inChain[n] + normal1;
			Point3	right2 = inChain[n] + normal2;
			
			Plane3	leftWall1 = Plane3(left1, normal1);
			Plane3	leftWall2 = Plane3(left2, normal2);
			Plane3	rightWall1 = Plane3(right1, normal1);
			Plane3	rightWall2 = Plane3(right2, normal2);

			// Next we intersect them.  This forms two lines that 
			// represent the valid set of possible end points for 
			// the planes where the widths of the planes are consistent.

			// If we don't have an intersection, that means that the
			// side wall planes are the same, which makes life easy.
			// Just pick the end of one of the segments because they're straight.
			
			Line3	leftEdge, rightEdge;
			if (!leftWall1.intersect(leftWall2, leftEdge))
				leftEdge = Line3(left1, inUp[n]);
			if (!rightWall1.intersect(rightWall2, rightEdge))
				rightEdge = Line3(right1, inUp[n]);
			
			// Build the plane that contains both pavement end lines.  This
			// is the plane within which we'd like our end points to occur if
			// we're going to interpolate our end lines reasonably.
			
			Plane3	junctionPlane(inChain[n], inUp[n]);
			
			// Now intersect those end lines with that plane...that is 
			// theoretically a good compromise of road continuity and
			// outer distance.  (Yeah right!)
			
			Point3	leftPt, rightPt;
			
			if (!junctionPlane.intersect(leftEdge, leftPt))
				leftPt = left1;
				
			if (!junctionPlane.intersect(rightEdge, rightPt))
				rightPt = right1;
				
			outQuadStrip.push_back(rightPt);
			outQuadStrip.push_back(leftPt);	
			}
		}
	}
}	

void	ReverseQuadStrip(
				vector<Point3>&				ioQuadStrip)
{
	int	p = ioQuadStrip.size();
	int h = p/2;
	for (int n = 0; n < h; ++n)
	{
		std::swap(ioQuadStrip[n], ioQuadStrip[p-n-1]);
	}
}
			


void	RemoveFromQuadStripFront(
				vector<Point3>&				ioChain,
				double						inRemoveFromStart,
				vector<Point3>&				outFront,
				bool						inTrimOffAngle)
{
	outFront.clear();
	if (ioChain.size() < 4)	return;
	
//	double	left_first_dist = sqrt(Segment3(ioChain[1], ioChain[3]).squared_length());
//	double	right_first_dist = sqrt(Segment3(ioChain[0], ioChain[2]).squared_length());
	
	double	removeFromStartLeft = inRemoveFromStart, removeFromStartRight = inRemoveFromStart;
	
	if (inTrimOffAngle)
	{
		double	trim_dist = LongerSideOfQuad(ioChain);
		if (trim_dist >= 0.0)
			removeFromStartRight += trim_dist;
		else
			removeFromStartLeft -= trim_dist;
	}
	
	int	last_left = ioChain.size() - 1;
	int last_right = ioChain.size() - 2;
	int	left, right;
	double	length, accum;
	
	if (inRemoveFromStart > 0.0)
	{
		Point3	left_split, right_split;
		int		cut_left = -1, cut_right = -1;
		accum = 0.0;
		for (left = 1; left < last_left; left += 2)
		{
			Segment3	seg(ioChain[left], ioChain[left+2]);
			length = sqrt(seg.squared_length());
			
			if ((accum + length) > removeFromStartLeft)
			{
				left_split = seg.midpoint((removeFromStartLeft - accum) / length);
				cut_left = left;
				break;
			} else 
				accum += length;			
		}
		
		accum = 0.0;
		for (right = 0; right < last_right; right += 2)
		{
			Segment3	seg(ioChain[right], ioChain[right+2]);
			length = sqrt(seg.squared_length());
			
			if ((accum + length) >= removeFromStartRight)
			{
				right_split = seg.midpoint((removeFromStartRight - accum) / length);
				cut_right = right;
				break;
			} else 
				accum += length;			
		}
		
		if (cut_left != -1 && cut_right != -1)
		{
			int		cut_proj_left = cut_right + 1;
			int		vertices_to_copy = std::min(cut_proj_left, cut_left) + 1;
			int		vertices_to_nuke = std::max(cut_proj_left, cut_left) + 1;

			outFront.insert(outFront.end(), ioChain.begin(), ioChain.begin() + vertices_to_copy);
			ioChain.erase(ioChain.begin(), ioChain.begin() + vertices_to_nuke);

			
			outFront.push_back(right_split);
			outFront.push_back(left_split);
			
			// If our split points _both_ hit on exactly the edge of a quad strip unit (damn unlikely)
			// then our split points are the same as the start of the old strip.  Check this case.
			// If they both match, no need to put the splits in.  But if one doesn't match, push back
			// the point and accumulate a degenerate quad (a triangle).
			if (ioChain[0] != right_split && ioChain[1] != left_split)
			{
				ioChain.insert(ioChain.begin(), left_split);
				ioChain.insert(ioChain.begin(), right_split);
			}
		}		
	}	
}				

void	RemoveFromQuadStripBack(
				vector<Point3>&				ioChain,
				double						inRemoveFromEnd,
				vector<Point3>&				outBack,
				bool						inTrimOffAngle)
{
	ReverseQuadStrip(ioChain);
	RemoveFromQuadStripFront(ioChain, inRemoveFromEnd, outBack, inTrimOffAngle);
	ReverseQuadStrip(ioChain);
}

double	LongerSideOfQuad(
				const vector<Point3>&		inChain)
{
	Vector3	left_v(inChain[3], inChain[1]);
	Vector3	right_v(inChain[2], inChain[0]);
	
	double	tl = left_v.dot(Vector3(inChain[3], inChain[0])) / left_v.dot(left_v);
	double	tr = right_v.dot(Vector3(inChain[2], inChain[1])) / right_v.dot(right_v);
	
	if (tl < 1.0)
		return -sqrt(Segment3(inChain[1], inChain[3] + left_v * tl).squared_length());
	else
		return sqrt(Segment3(inChain[0], inChain[2] + right_v * tr).squared_length());
}



void	ClipToHalfPlane3(
				const Polygon3&				inPolygon,
				const Plane3&				inPlane,
				Polygon3&					outPolygon)
{
	outPolygon.clear();
	if (inPolygon.empty()) return;
	
	bool culled = inPlane.on_normal_side(inPolygon[0]);
	Point3	pt;
	
	for (int n = 0; n < inPolygon.size(); ++n)
	{
		int nn = (n + 1) % inPolygon.size();
		Segment3	seg(inPolygon[n], inPolygon[nn]);
		if (!culled)
			outPolygon.push_back(inPolygon[n]);
		if (inPlane.intersect(seg, pt))
		{
			if (pt != inPolygon[n] && pt != inPolygon[nn])
				outPolygon.push_back(pt);
			culled = !culled;
		}
	}
}

bool	IntersectLinesAroundJunction(
				const Line3&				inLine1,
				const Line3&				inLine2,
				const Point3&				inJunctionPt,
				Point3&						outIntersection)
{
	if (inLine1.parallel(inLine2))	return false;
	
	Vector3	ground_plane_normal = inLine1.v.cross(inLine2.v);
	ground_plane_normal.normalize();
	
	Plane3	ground_plane(inJunctionPt, ground_plane_normal);
	
	Vector3	plane1_normal = inLine1.v.cross(ground_plane_normal);
	Vector3	plane2_normal = inLine2.v.cross(ground_plane_normal);
	
	Plane3	plane1(inLine1.p, plane1_normal);
	Plane3	plane2(inLine2.p, plane2_normal);
	
	Line3	plane_crossing;
	if (plane1.intersect(plane2, plane_crossing))
	{
		return ground_plane.intersect(plane_crossing, outIntersection);
	} else 
		return false;
}				

// TODO: theoretically lack of atan2 precision can cause this to have problems.
// Pragmatically I'm not sure this happens, but it'd be better to solve it
// by case-by-case analysis.

bool	Is_CCW_Between(const Vector2& v1, const Vector2& v2, const Vector2& v3)
{
	double	angle1 = atan2(v1.dy, v1.dx);
	double	angle2 = atan2(v2.dy, v2.dx);
	double	angle3 = atan2(v3.dy, v3.dx);
	
	// Special case...if angle 1 and 3 are the same, then treat the whole circle as
	// inside and always return true.  This is important when an antenna gets near its
	// end...the two choices are a return path equal to the current path and an alternate
	// continuation.  We have to take the continuation or we'll orphan the antenna, which blows.
	// Is this computationally safe?  It is because two vectors of equal angle will be overlapping
	// and are therefore two sides of the same edge.  They MUST have the same extent in opposite
	// direction by topological rules, so their angles should be deterministically the same!
	if (angle1 == angle3) 
		return true;
	// This was a special case to try to detect funky data.		
	if (angle1 == angle2 || angle3 == angle2)
	{
#if DEV
		printf("WARNING: trying to place a vector that overlaps one of the edges.\n");
		printf("Vec1: %lf,%lf\n", v1.dx, v1.dy);
		printf("Vec2: %lf,%lf\n", v2.dx, v2.dy);
		printf("Vec3: %lf,%lf\n", v3.dx, v3.dy);
#endif		
		return true;
	}
	
	// Normalize all angles to be from 0 to 360.
	if (angle1 <= -PI)	angle1 += PI2;
	if (angle2 <= -PI)	angle2 += PI2;
	if (angle3 <= -PI)	angle3 += PI2;
	
	// Special case - if we have to increase BOTH angle 2 and 3,
	// we know that (1) angle 1 will be smaller than angle 2 after the increase AND
	// (2) the compare of angle 2 and 3 is the smae whether or not we add.
	// BUT adding PI2 causes floating point rounding to give us wrong answers for a few 
	// rare cases.  So special-case this to avoid the add.
	if (angle2 < angle1 && angle3 < angle1)
	{
		return angle2 < angle3;
	}
	
	// Normalize angles 2 and 3 to be at least as big as angle 1 (represents counterclockwise rotation
	// from angle 1), even if exceeds 360.
	if (angle2 < angle1)	angle2 += PI2;
	if (angle3 < angle1)	angle3 += PI2;
	
	// Now if the headings go 1 2 3 we're CCW.
	return (angle1 < angle2 && angle2 < angle3);
}

#if 0
THIS USES NEAREST_ON_SIDE, WHICH IS BUSTED!
void	ReducePolygon(Polygon2& ioPolygon, double tolerance, double angle, double min_len, double cut_len)
{
	min_len *= min_len;
	cut_len *= cut_len;
	bool	got_one = true;
	while (got_one)
	{
		got_one = false;
		for (int i = 0; i < ioPolygon.size();++i)
		{
			if (ioPolygon.size() < 3) return;
			Point2	me = ioPolygon[i];
			Point2	prev = ioPolygon[(i+ioPolygon.size()-1)%ioPolygon.size()];
			Point2	next = ioPolygon[(i+1)%ioPolygon.size()];

			Segment2 seg(prev,next);
			Point2	nn = seg.nearest_on_segment(me);
			double	err = Segment2(me, nn).squared_length();

			Vector2 v1(prev, me), v2(me, next);
			if (v1.squared_length() < cut_len || v2.squared_length() < cut_len)
			{
				got_one = true;
				Polygon2::iterator ii = ioPolygon.begin();
				advance(ii, i);
				ioPolygon.erase(ii);
			} else if (v1.squared_length() > min_len && v2.squared_length() > min_len)
			{
				v1.normalize();
				v2.normalize();
				if (err < tolerance && v1.dot(v2) > angle)
				{
					got_one = true;
					Polygon2::iterator ii = ioPolygon.begin();
					advance(ii, i);
					ioPolygon.erase(ii);
				}				
			}
		}
	}
}
#endif

// HACK UTILIITY: is this point an edge of a DSF
inline bool IsIntegral(const Point2& p)
{
	return (p.x == (double) (int) p.x ||
			p.y == (double) (int) p.y); 
}

// HACK UTILIITY: is this point a corner of a DSF
inline bool IsCorner(const Point2& p)
{
	return (p.x == (double) (int) p.x &&
			p.y == (double) (int) p.y); 
}

// UTILITY: give na span of points in a polygon defined by P and deltas from
// it, calculate the max error any point between the two edges in terms of
// a deviation from the line from the end points. 
inline double	calc_error(Polygon2& ioPolygon, int p, int d_prev, int d_next, bool allow_in, bool allow_out, double max_err)
{
	double worst = 0.0;
	int sz = ioPolygon.size();
	int p1 = (p + d_prev + sz) % sz;
	int p2 = (p + d_next + sz) % sz;
	Segment2	seg(ioPolygon[p1],ioPolygon[p2]);

	p1 = (p + d_prev) + 1;
	p2 = (p + d_next) - 1;
	
	Vector2		inside(Vector2(seg.p1, seg.p2).perpendicular_ccw());
	
	
	for (int i = p1; i <= p2; ++i)
	{
		Point2	t = ioPolygon[(i + sz) % sz];
	
		// ILLEGAL MOVE CHECK - we can restrict our operation to only widening or narrowing
		// the poly.  Here we look at whether the original point is on the left or right side
		// of the new simple edge.  This is counter-intuitive...if the old point is INSIDE
		// the simple edge, then we have EXPANDED the polygon.
		double dot = inside.dot(Vector2(seg.p1));
		if (dot < 0.0 && !allow_in) return max_err;
		if (dot > 0.0 && !allow_out) return max_err;
	
		// ERROR CALCULATION
		double me = Line2(seg).squared_distance(t);
		if (me > worst) worst = me;
	}
	return worst;
}

// Polygon simplification
// 
// Basic idea: for any given point, removing the pt introduces a
// certain amount of error - the max of which is the max of the
// distance from the new line to any of the points the line crosses
// (including ourselves).
// SO...
// We make a priority Q of all pts by their removal errors.  We then
// remove pts by smallest error and recalc the affected errors.  We
// stop when we exceed the max error.
void	SimplifyPolygonMaxMove(Polygon2& ioPolygon, double max_err, bool allow_in, bool allow_out)
{
	DebugAssert(allow_in || allow_out);
	typedef multimap<double, int>	q_type;								// Our priority qeueu type, from error -> point index
	q_type						q;										// Our queue
	vector<int>					prev_delta(ioPolygon.size(), -1);		// Relative index ptr to the prev pt still in the poly, or 0 if removed.
	vector<int>					next_delta(ioPolygon.size(), 1);		// Relative index ptr to the next pt still in the poly, or 0 if removed.
	vector<q_type::iterator>	q_backlinks(ioPolygon.size());			// Per pt iterator into the queue (back-links)
	int sz = ioPolygon.size();
	double						err;
	int							pts_total = ioPolygon.size();
	
	double max_err_sq = max_err * max_err;
	
	// SETUP - Calculate each pts error and queue it.
	for (int n = 0; n < ioPolygon.size(); ++n)
	{
		err = calc_error(ioPolygon, n, prev_delta[n], next_delta[n], allow_in, allow_out, max_err_sq);
		if (IsIntegral(ioPolygon[n])) err = max_err_sq;
		q_backlinks[n] = q.insert(q_type::value_type(err, n));
	}
	
	while (!q.empty() && q.begin()->first < max_err_sq && pts_total > 3)
	{	
		--pts_total;
		// Find the point with the least error and rmeove it.
		int k = q.begin()->second;
		DebugAssert(prev_delta[k] != 0);
		DebugAssert(next_delta[k] != 0);
		DebugAssert(q_backlinks[k] != q.end());
		int c1 = k + prev_delta[k];
		int c2 = k + next_delta[k];
		prev_delta[k] = 0;
		next_delta[k] = 0;
		q.erase(q_backlinks[k]);
		q_backlinks[k] = q.end();
		c1 = (c1 + sz) % sz;
		c2 = (c2 + sz) % sz;
		
		DebugAssert(prev_delta[c1] != 0);
		DebugAssert(next_delta[c1] != 0);
		DebugAssert(q_backlinks[c1] != q.end());
		DebugAssert(prev_delta[c2] != 0);
		DebugAssert(next_delta[c2] != 0);
		DebugAssert(q_backlinks[c2] != q.end());
		
		// Recalculate the error of the points that
		// formed lines with our removed point.  Those
		// points (c1 and c2) now link to each other
		// and that new line may have lousy error
		// that needs to be reconsidered.
		DebugAssert(c1 != c2);
		q.erase(q_backlinks[c1]);
		q.erase(q_backlinks[c2]);

		prev_delta[c2] = c1 - c2;
		while (prev_delta[c2] > 0)
			prev_delta[c2] -= sz;
		next_delta[c1] = c2 - c1;
		while (next_delta[c1] < 0)
			next_delta[c1] += sz;

		err = calc_error(ioPolygon, c1, prev_delta[c1], next_delta[c1], allow_in, allow_out, max_err_sq);
		if (IsIntegral(ioPolygon[c2])) err = max_err_sq;
		q_backlinks[c1] = q.insert(q_type::value_type(err, c1));

		err = calc_error(ioPolygon, c2, prev_delta[c2], next_delta[c2], allow_in, allow_out, max_err_sq);
		if (IsIntegral(ioPolygon[c2])) err = max_err_sq;
		q_backlinks[c2] = q.insert(q_type::value_type(err, c2));		
	}
	
	// Put the polygon back together with only remaining pts.
	Polygon2	npoly(q.size());
	int ni = 0;
	for (int n = 0; n < sz; ++n)
	{
		if (prev_delta[n] != 0)
			npoly[ni++] = ioPolygon[n];
	}
	DebugAssert(pts_total == npoly.size());
	DebugAssert(q.size() == ni);
	ioPolygon.swap(npoly);
}

void	MidpointSimplifyPolygon(Polygon2& ioPolygon)
{
	Polygon2	dst(ioPolygon.size());
	for (int n = 0; n < ioPolygon.size(); ++n)
	{
		dst[n] = (ioPolygon.side(n).midpoint());
	}
	
	ioPolygon.clear();
	
	for (int n = 0; n < dst.size(); ++n)
	{
		Vector2	prev(dst[dst.prev(n)], dst[n]);
		Vector2	next(dst[n], dst[dst.next(n)]);
		
		if (prev.dx * next.dy != prev.dy * next.dx)
			ioPolygon.push_back(dst[n]);
	}
}


// Polygon smoothing
//
// Basically go through and iterate across a bezier curve for any point exceeding the
// acceptable angle.
void	SmoothPolygon(Polygon2& ioPolygon, double smooth_radius, double max_turn_deg)
{
	int sz = ioPolygon.size();
	Polygon2	npoly;
	int n;
	
	for (n = 0; n < sz; ++n)
	{
		Point2	prev(ioPolygon[(n+sz-1)%sz]);
		Point2	now (ioPolygon[(n     )   ]);
		Point2	next(ioPolygon[(n+   1)%sz]);
		
		Vector2	v1(prev, now);
		Vector2 v2(now, next);
		v1.normalize();
		v2.normalize();
		double angle = RAD_TO_DEG * acos(v1.dot(v2));
		if (angle <= max_turn_deg || IsCorner(now))
			npoly.push_back(now);
		else {
			Point2	turn_start = now - (v1 * smooth_radius);
			Point2	turn_end   = now + (v2 * smooth_radius);
			Point2	c1 = now - (v1 * (smooth_radius*0.5));
			Point2	c2 = now + (v2 * (smooth_radius*0.5));
			Bezier2	curve(turn_start, c1, c2, turn_end);
			
			int num_segs = angle / max_turn_deg + 1;
			
			for (int t = 0; t <= num_segs; ++t)
			{
				double param = (double) t/ (double) num_segs;
				npoly.push_back(curve.midpoint(param));
			}
		}
	}
	ioPolygon.swap(npoly);
}

// Given our point now and the best pt we've found so far, can we do better?
// First we do a dot product of the right-side normal to the best vector - if the newer
// is to the right (positive dot product) it's better, to the left it's worse
// Then we have the colinear case.  First deal with equal points.  Then if the newer point
// makes opposite dor product vectors to the anchor and best, the newer is in the middle 
// and is superior.
inline bool	IsBetterHullPt(const Point2& anchor, const Point2& best_so_far, const Point2& newer)
{
	// Is the new pt to the right of the directed candidate side?  If so, the candidate side
	// is NOT on the hull - the new pt is better.
	double dot_v = Vector2(anchor, best_so_far).perpendicular_cw().dot(Vector2(anchor, newer));
	if (dot_v > 0.0) return true;
	if (dot_v < 0.0) return false;
	
	// Colinear cases - don't ever accept a zero length side.	
	if (best_so_far == anchor) return true;
	if (best_so_far == newer) return false;
	if (newer == anchor) return false;
	
	// Take the shortest side possible.
	return Vector2(anchor, best_so_far).dot(Vector2(anchor, newer)) < 0.0;
	
}

void	MakePolygonConvex(Polygon2& ioPolygon)
{
// This is "Jarvis March" (gift wrapping): start at the highest point (cause we KNOW it's on the hull)
// and then for each pt see if it makes a better hull pt.  Since we know that in point in the poly is
// to the 'right' of a side (going CCW) in the hull, we can test this with a candidate side of the hull
// (from the last pt found in the hull) to another pt in the hull.  If the other pt is outside, IT must
// be in the hull.
//
// Time complexity is O(n*h) where N = number of pts in the dataset, H = number of pts in the hull. 
// for complex hulls this is SLOW, worst case (circle) = O(N^2).  Best is a triangular hole where the
// alg runs in O(N).
//
// http://www.cs.princeton.edu/~ah/alg_anim/version1/JarvisMarch.html

	if (ioPolygon.size() < 3) return;
	
	Polygon2		new_poly;
	int				start_pt = 0;
	int				now_pt;
	int				best_pt;
	int				i;
	
	for (i = 1; i < ioPolygon.size(); ++i)
	if (ioPolygon[i].y > ioPolygon[start_pt].y)
		start_pt = i;

	now_pt = start_pt;	
	do {
		new_poly.push_back(ioPolygon[now_pt]);
		best_pt = 0;
		for (i = 1; i < ioPolygon.size(); ++i)
		{
			if (IsBetterHullPt(ioPolygon[now_pt], ioPolygon[best_pt], ioPolygon[i]))
				best_pt = i;
		}
				
		now_pt = best_pt;
		
	} while (start_pt != now_pt);
	
	ioPolygon.swap(new_poly);
	
	DebugAssert(ioPolygon.convex());

}

// UTILITY:
// Counts the number of added edges and makes sure we're not introducing splits.
void	CheckSplit(GISHalfedge * a, GISHalfedge * b, void * ref)
{
	int * foo = (int *) ref;
	DebugAssert(a == NULL || b == NULL);
	(*foo)++;
		
}

#define SLIVER_PROTECTION	0.9999

// UTILITY:
// Returns true if there is no edge adjacent to (and pointing to) V that
// is nearly incident with vec (vec is revsersed by the bool param).
inline bool NoHalfedgeInDir(GISVertex * v, const Vector2& vec, bool reverse)
{
	double mul = reverse ? -1.0 : 1.0;
	Pmwx::Halfedge_around_vertex_circulator stop, iter;
	iter = stop = v->incident_halfedges();
	do {
		Vector2		iv(iter->source()->point(), iter->target()->point());
		iv.normalize();
		if ((mul * vec.dot(iv)) > SLIVER_PROTECTION) return false;
		++iter;
	} while (iter != stop);
	return true;
}

// Convert a polygon to a one-face pmwx.  Polygon must have no antennas and be simple.
static GISFace* PolyToPmwx(const Polygon2& inPoly, Pmwx& outMap, vector<GISVertex*> * outVertices)
{
	DebugAssert(outMap.empty()); 
	GISFace * face = outMap.insert_ring(outMap.unbounded_face(), inPoly);
	if (outVertices) 
	{
		outVertices->resize(inPoly.size());
		int i;
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = *(outMap.unbounded_face()->holes_begin());
		i = inPoly.size();
		do {
			--i;
			(*outVertices)[i] = circ->target();
			DebugAssert((*outVertices)[i]->point() == inPoly[i]);
			++circ;
		} while (circ != stop);
	}
	return face;
}

// Convert a one-face pmwx to a polygon.  Holes in the face are ignored.
static void PmwxToPoly(const Pmwx& inMap, Polygon2& outPoly)
{
	Pmwx::Ccb_halfedge_const_circulator circ, stop;
	int ctr = 0;

	Assert(inMap.unbounded_face()->holes_count() == 1);
	
	outPoly.clear();
	
	circ = stop = *(inMap.unbounded_face()->holes_begin());
	do {
		++ctr;
		++circ;
	} while (circ != stop); 

	outPoly.resize(ctr);

	circ = stop = *(inMap.unbounded_face()->holes_begin());
	do {
		--ctr;
		outPoly[ctr] = circ->target()->point();		
		++circ;
	} while (circ != stop); 
}

// Polygon Gap coverage
// 
// For a given polygon, try to fill in small "leaks" by connecting any external
// points that are visible to each other and smaller than 'dist'.
void	FillPolygonGaps(Polygon2& ioPolygon, double dist)
{
	vector<GISVertex *>	verts;
	Pmwx	pmwx;
	int i, j;
	Pmwx::Ccb_halfedge_circulator circ, stop;
	double dsqr = dist * dist;

	PolyToPmwx(ioPolygon, pmwx,&verts);

	// This isn't exactly intuitive.  We want to try every combination of vertices to see if we can make
	// a line out of them.  But...here's the rub: if we just try in random order, we'll make somes long
	// segments that go right past a nearby vertex.  This colocation problem is similar to slivering...
	// the map won't do the right thing and split the edge.  So...always build from shortest to longest.
	// Possibles is a queue of segments that meet the length requirement sorted by length.
//	multimap<double, pair<int, int> > 	possibles;

	for (i = 0; i < ioPolygon.size(); ++i)
	for (j = i+1; j < ioPolygon.size(); ++j)
	{
		Segment2	seg(ioPolygon.at(i), ioPolygon.at(j));
		Vector2		dir(seg.p1, seg.p2);
		dir.normalize();
		DebugAssert(seg.p1 != seg.p2);
		double len = seg.squared_length();
		if (len < dsqr)
		if (NoHalfedgeInDir(verts[i], dir, true))
		if (NoHalfedgeInDir(verts[j], dir, false))
		{
			Pmwx::Locate_type le, l2;
			GISHalfedge * h = verts[i]->halfedge();
			DebugAssert(h->target()->point() == seg.p1);
			le = Pmwx::locate_Vertex;
//			GISHalfedge * h = pmwx.locate_point(seg.p1, le);
//			Assert(le == Pmwx::locate_Vertex);
			Point2 pc;
			pmwx.ray_shoot(seg.p1, le, h, seg.p2, pc, l2);
			int sn = 0;
			if (pc == seg.p2 && l2 == Pmwx::locate_Vertex)
			{
#if DEV			
				pmwx.insert_edge(seg.p1, seg.p2, h, le, CheckSplit, &sn);
				DebugAssert(sn == 1);
#else
				pmwx.insert_edge(seg.p1, seg.p2, h, le, NULL, NULL);
#endif				
				
#if 0
				for (k = 0; k < ioPolygon.size(); ++k)
				{
					Pmwx::Locate_type t;
					pmwx.locate_point(ioPolygon[k], t);
					DebugAssert(t == Pmwx::locate_Vertex);
				}				
#endif					
			}
		}
	}
	
	PmwxToPoly(pmwx, ioPolygon);
}


// Make a polygon more convex.  Basically go around the 
// edge and if we can safely insert a ray that cuts off the
// concave edge, do so.  Keep doing this until we're stuck.
// Don't insert an edge that adds more than max_area.
// This is N^2, but for small polygons that may be okay.
void	SafeMakeMoreConvex(Polygon2& ioPolygon, double max_area)
{
	Pmwx	pmwx;
	
	PolyToPmwx(ioPolygon, pmwx, NULL);
	
	bool did_work;
	do {
		did_work = false;
		
		Pmwx::Ccb_halfedge_circulator stop, iter, next;
		stop = iter = *(pmwx.unbounded_face()->holes_begin());
		do {
			next = iter;
			++next;
			DebugAssert(iter != next);
			Vector2	v1(iter->source()->point(), iter->target()->point());
			Vector2	v2(next->source()->point(), next->target()->point());
			double area = v1.signed_area(v2);
			if (area > 0 && area < max_area)
			{
				Segment2	seg(iter->source()->point(),next->target()->point());
				Vector2		dir(seg.p1, seg.p2);
				dir.normalize();
				if (NoHalfedgeInDir(iter->source(), dir, true))
				if (NoHalfedgeInDir(next->target(), dir, false))
				{
					Pmwx::Locate_type l2, le = Pmwx::locate_Vertex;
					GISHalfedge * h = iter->twin();
					DebugAssert(h->target()->point() == seg.p1);
					Point2 pc;
					pmwx.ray_shoot(seg.p1, le, h, seg.p2, pc, l2);
					int sn = 0;
					if (pc == seg.p2 && l2 == Pmwx::locate_Vertex)
					{
		#if DEV			
						pmwx.insert_edge(seg.p1, seg.p2, h, le, CheckSplit, &sn);
						DebugAssert(sn == 1);
		#else
						pmwx.insert_edge(seg.p1, seg.p2, h, le, NULL, NULL);
		#endif				
						did_work = true;
						break;
					}
				}
			}
			++iter;
		} while (iter != stop);
	} while (did_work);
	
	PmwxToPoly(pmwx, ioPolygon);
}

struct sort_by_ymin {
	bool operator()(const Bbox2& lhs, const Bbox2& rhs) const { return lhs.ymin() < rhs.ymin(); }
};

// Simple self-intersection check.
// We build up a sorted map of our sides by their lower Y coordinate.
// Then for each side, check all sides above us until the sides are starting
// clearly above us.  (Why no need to check below?  Well, that's handled 
// when the other side is the 'i' - in other words, given two polys X and Y
// where X is below Y, we check XY, not YX).
bool	ValidatePolygonSimply(const Polygon2& ioPolygon)
{
		Point2	p;

	typedef multimap<Bbox2, Segment2, sort_by_ymin>	sort_map;
	sort_map	sides;
	for (int n = 0; n < ioPolygon.size(); ++n)
	{
		Segment2 side = ioPolygon.side(n);
		sides.insert(sort_map::value_type(Bbox2(side.p1, side.p2), side));
	}
	
	for (sort_map::iterator i = sides.begin(); i != sides.end(); ++i)
	{
		sort_map::iterator j = i;
		++j;
		for (; j != sides.end(); ++j)
		{
			if (i->first.overlap(j->first))
			if (i->second.intersect(j->second, p))
			if (p != i->second.p1 && p != i->second.p2 && p != i->first.p1 && p != i->first.p2)
				return false;

			// Quiick exit - if this poly starts above our top, we've gone through any possible overlaps.
			if (j->first.ymin() > i->first.ymax())
				break;
		}
	}
	return true;
}
