/* 
 * Copyright (c) 2013, Laminar Research.
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
#ifndef WED_CLIPPING_H
#define WED_CLIPPING_H

#include "CompGeomDefs2.h"
#include "AssertUtils.h"

#include "STLUtils.h"
#include "WED_GISUtils.h"

/*
	These routines clip polygons to an AABB - the only clipping shape we support.
	The routines aim to produce correct output for well-formed input that isn't at the
	margin of floating point, not explode when something goes wrong, and in some cases 
	indicate algorithmic failure.
	
	These routiens are _not_ at all designed for performance.  They do clipping in
	four half-plane passes, they generate all split and intermediate segments even
	when we know the results will be thrown out, etc.  The algorithm is designed for
	simplicity in the hope of getting correctness right.
	
	Segment clipping always succeeds; polygon clipping may fail.  In particular, it
	will fail if the polygon is not well-formed, e.g. if the winding order is wrong or
	it is self-intersecting.
	
	GENERAL POLYGONS VS POLYGONS
	
	When a polygon is made of line segments, we can store it as a sequence of vertices, 
	with the topology implied.
	
	When our sides are bezier curves and not segments, we store a sequence of sides; 
	in this case, it is a _semantic_ requirement that the end of each curve be the
	start of the next.  If this is not the case, we can say that the general polygon
	(a generalization of the segment polygon for any curve) is invalid because it is
	disconnected.
	
	We store pure polygons as point sequences, but we store polygons with side parameters
	and all bezier polygons (with or without side parameters) as general polygons of sides.
	
	POLYGONS WITH HOLES
	
	By convention a polygon with holes is stored as a sequence of polygons - the first is
	the outer boundary and the rest are holes.
	
	WINDING
	
	We expect our outer boundaries to be CCW and our inner holes to be CW.
	
	ORDER STABILITY
	
	The code returns a polygon unmodified if it doesn't need to be clipped.  When clipped,
	the "first" side of the polygon can (and often does) become rotated).
	
	LINE SEGMENT CUTTING
	
	Poly-lines (line segments) are represented by sequences of line segments or bezier curves.
	In this case, breaks in the segment are considered intentional - that is, a sequence of
	segments represents a "list of poly-lines".
	
	The clipping code simply goes through and chops up the segments, producing even more gaps.
	If there were silly gaps (e.g. poly-line ABC is ordered A,B,C) then it is clipped that way.
	
	In other words, the poly-line clipper is really a segment clipper that clips a list of 
	segments.

*/

void	clip_segments(vector<Segment2>& out_segs, const Bbox2& box);
void	clip_segments(vector<Segment2p>& out_segs, const Bbox2& box);
void	clip_segments(vector<Bezier2>& out_segs, const Bbox2& box);
void	clip_segments(vector<Bezier2p>& out_segs, const Bbox2& box);

// Returns true if clip succeeeded - returns false if something went wrong - typical failures are induced
// by polygons with the wrong winding and self-intersecting or degenerate polygons.  On failure, contents
// of out_pwh_list are undefined and should not be uesd.
bool	clip_polygon(const vector<Polygon2>& in_pwh, vector<vector<Polygon2> >& out_pwh_list, const Bbox2& box);
bool	clip_polygon(const vector<Polygon2p>& in_pwh, vector<vector<Polygon2p> >& out_pwh_list, const Bbox2& box);
bool	clip_polygon(const vector<Polygon2uv>& in_pwh, vector<vector<Polygon2uv> >& out_pwh_list, const Bbox2& box);

bool	clip_polygon(const vector<BezierPolygon2>& in_pwh, vector<vector<BezierPolygon2> >& out_pwh_list, const Bbox2& box);
bool	clip_polygon(const vector<BezierPolygon2p>& in_pwh, vector<vector<BezierPolygon2p> >& out_pwh_list, const Bbox2& box);
bool	clip_polygon(const vector<BezierPolygon2uv>& in_pwh, vector<vector<BezierPolygon2uv> >& out_pwh_list, const Bbox2& box);

#endif
