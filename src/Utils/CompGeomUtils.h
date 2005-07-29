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
#ifndef COMPGEOMUTILS_H
#define COMPGEOMUTILS_H

#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

struct	CoordTranslator {
	Point2	mSrcMin;
	Point2	mSrcMax;
	Point2	mDstMin;
	Point2	mDstMax;
	
	Point2	Forward(Point2 input);
	Point2	Reverse(Point2 input);
};
	

/* 
 * InsetPolygon
 *
 * Given a CCW polygon (or a segment chain), this routine insets
 * by a ratio for each segment multiplied by an inset.  For CW polygon,
 * use a negative ratio.  If the inset ratios are too large, the polygon
 * will become non-simple.
 *
 * This routine will handle antennas into the polygon (but not out of the
 * polygon - cut these off) if they are of equal length, and will do its best
 * to cope with very sharp angles in and out and near-colinear sides.
 * 
 * If you pass in an "antenna function", it will be called any time an extra
 * vertex must be added (introducing a new side).  The newly inserted point's 
 * index is passed in, relative to the changing polygon.  (Meaning on the second
 * call, the index is +1 due to the first call.)
 *
 */
void	InsetPolygon2(
				const Polygon2&				inChain,
				const double *				inRatios,
				double						inInset,
				bool						inIsRing,
				Polygon2&					outChain,
				void						(* antennaFunc)(int n, void * ref)=NULL,
				void *						ref = NULL);
				
				
#if 0
Does anyone use this?
void	InsetPolygon3(
				const Polygon3&				inChain,
				const double *				inRatios,
				double						inInset,
				bool						inIsRing,
				const Vector3&				inUp,
				Polygon3&					outChain);
#endif

/*
 * CalcMaxInset
 *
 * Given a CCW polygon and a vector of distances to inset the
 * polygon, this routine will calculate the maximum multiplyer
 * for those distances that the polygon may be inset before 
 * the inset polygon becomes non-simple.  At this inset point,
 * at least two adjacent points will be colocated.
 *
 * inIsRing is true for a polygon, false for a connected chain, but
 * see warnings below.
 *
 * WARNING: THIS ROUTINE IS NOT CORRECT!
 *
 * In the following case it will not work: if you create a 
 * concave simple polygon (for example, a thick letter H)
 * where concave offshoots would degenerate to triangles that 
 * would be islands in the degeneration, this routine will not
 * catch the islands, and instead return a non-simple polygon.
 *
 * Therefore a precondition of correct insetting is that 
 * the maximum simple inset not contain islands!
 *
 * When calculating a ring, collision of the beginning and end
 * of the ring are not detected; for this reason, set inIsRing
 * to true even for non-ring collisions.  This is not perfect but
 * will produce strange results less often.
 *
 */
double	CalcMaxInset(
				const Polygon2&				inChain,
				const double *				inRatios,
				bool						inIsRing);

/*
 * ExtendBoundingSphereToPt
 *
 * Given a bounding sphere and a point, extend the sphere as
 * necessary.
 *
 */
void ExtendBoundingSphereToPt(const Point3& p, Sphere3& ioSphere);


/* 
 * ExtendBoundingSphereToSphere
 *
 * Given a sphere, include another one in it.
 *
 */ 
void ExtendBoundingSphereToSphere(const Sphere3& newSphere, Sphere3& ioSphere);

/*
 * FastBoundingSphere
 *
 * Given an unsorted list of points this routine calculates an 
 * approximate bounding sphere that contains all of them.  It does not
 * calculate the absolutely smallest bounding sphere (which is very slow)
 * but calculates a very close approximation.
 *
 */ 
void	FastBoundingSphere(
				const vector<Point3>&		inPoints,
				Sphere3&					outSphere);

/*
 * PointInPolygon3
 *
 * Given a reasonably coplanar convex polygon and a point, this function returns
 * true if the point is within the infinitely extended prism defined by the 
 * polygon.  This includes points on the polygon.
 *
 */
bool	PointInPolygon3(
				const Polygon3&				inPolygon,
				const Point3&				inPoint);

/*
 * BezierCurve
 *
 * Given two points and up to 2 control points, create a chain of points that
 * represents this bezier curve.  inStart and inEnda re always the first and last
 * points in the vector.  inNumSegments is how many segments to create.  There will
 * be inNumSegments+1 points in the vector _unless_ there is not a start or end
 * curve.  (In this case, the straight line is created with two end points and no
 * intermediate points.)
 *
 * inProtectStart and inProtectEnd are minimum lengths for the first and last 
 * segment in the curve.  This can be crucial when turning a bezier curve
 * into a quad chain - see comments in the implementation for a description
 * of the "pinching" problem.
 *
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
				vector<Point3>&				outPoints);

/*
 * ChainToQuadStrip
 *
 * Given a series of 3-d points defining a line through space and
 * a series of up-vectors and widths, this routine extrudes the 
 * results into a quad strip.
 *
 */
void	ChainToQuadStrip(
				const vector<Point3>&		inChain,
				const vector<Vector3>&		inUp,
				const vector<double>&		inWidth,
				vector<Point3>&				outQuadStrip);

void	ReverseQuadStrip(
				vector<Point3>&				ioQuadStrip);

/*
 * RemoveEndsFromQuadStrip
 *
 * This routine removes the first N units of quad strip from
 * the beginning of this strip.  The removed quad stripping
 * always forms vaguely a parallelagram because the routine
 * moves down each side.  The new quad strip's end points are
 * the old one's start points when done.  
 *
 * If (because the quad strip's end is angled, but its 
 * midsections are straight) the quad strip would be inside
 * out after the cut, vertices are dropped to make the quad 
 * strip regular.
 *
 */
void	RemoveFromQuadStripFront(
				vector<Point3>&				ioChain,
				double						inRemoveFromStart,
				vector<Point3>&				outFront,
				bool						inTrimOffBevel);

void	RemoveFromQuadStripBack(
				vector<Point3>&				ioChain,
				double						inRemoveFromEnd,
				vector<Point3>&				outBack,
				bool						inTrimOffBevel);

/*
 * LongerSideOfQuad
 *
 * Given a quad strip, this routine figures out which side of the beginning
 * of the first quad is longer.  If the right side is longer, the difference
 * in length is returned as a positive number; if the left a negative.
 *
 */
double	LongerSideOfQuad(
				const vector<Point3>&		inChain);

/*
 * ClipToHalfPlane3
 *
 * Remove things on the side of the plane pointed to by the normal.
 *
 */
void	ClipToHalfPlane3(
				const Polygon3&				inPolygon,
				const Plane3&				inPlane,
				Polygon3&					outPolygon);


/*
 * IntersectLinesAroundJunction
 *
 * Given two lines that represent the edges of a roadway,
 * and the point that is the true center of intersection of the
 * roadway, this routine finds a point that is on or above or below
 * where those lines projections would intersect if looked on from
 * straight above, based on the ground plane at the exact intersection
 * of the two lines.
 *
 */
bool	IntersectLinesAroundJunction(
				const Line3&				inLine1,
				const Line3&				inLine2,
				const Point3&				inJunctionPt,
				Point3&						outIntersection);



// Given three vectors, if they originated from the same point, and you 
// rotated counterclockwise from 1, would you hit 2 and then 3?
bool	Is_CCW_Between(const Vector2& v1, const Vector2& v2, const Vector2& v3);

// Given a polygon, try to reduce the number of sides based on some tollerance.
// void	ReducePolygon(Polygon2& ioPolygon, double tolerance, double angle, double min_len, double cut_len);

// Given a polygon, simplify its boundary.  The new polygon will not have
// a corresponding segment farther than "max_err" from an original point.
// Only original points will be used.
//
// WARNING: THERE ARE HACKS IN THIS FUNCTION FOR INTEGRAL POINTS!
void	SimplifyPolygonMaxMove(Polygon2& ioPolygon, double max_err, bool allow_inset, bool allow_expand);

// Simple algorithm: this algorithm simply takes the midpoints to simply a polygon.
void	MidpointSimplifyPolygon(Polygon2& ioPolygon);

// Given a polygon, insert somothing turns for any turn more than max_turn_deg.
// Turns start smooth_radius units from each vertex that needs smoothing.  Each
// new introduced angle is approximately max_turn_deg in theory.
//
// WARNING: THERE ARE HACKS IN THIS FUNCTION FOR INTEGRAL POINTS!
void	SmoothPolygon(Polygon2& ioPolygon, double smooth_radius, double max_turn_deg);



// Calculate the convex hull of a polygon. 
// NOTE: needs to be rewritten! - BAS
//			Why?!? - BAS
void	MakePolygonConvex(Polygon2& ioPolygon);

// Fill in bridges outside the polygon where two points are less than 'dist' units
// apart.  (WARNING: O(N^2) right now!)
// (For example, this is used to bridge across "inlets" in an airport.)
void	FillPolygonGaps(Polygon2& ioPolygon, double dist);

// Given a polygon, changes its bounds to make it "more convex" (e.g. each step increases area and
// decreases inset angles), with no addition being greater than max_area.  Runs until no more adds
// can be made or the convex hull is reached.
// (For example, this is used to simplify an airport by annexing surrounding land.)
void	SafeMakeMoreConvex(Polygon2& ioPolygon, double max_area);


// Return true if a polygon really is simple by checking for self-intersections.
bool	ValidatePolygonSimply(const Polygon2& ioPolygon);


#endif
