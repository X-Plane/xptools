/*
 *  WED_GISUtils.h
 *  SceneryTools
 *
 *  Created by bsupnik on 5/27/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef WED_GISUTILS_H
#define WED_GISUTILS_H

#include "MapDefs.h"
class IGISPointSequence;
class IGISEntity;
class IGISQuad;
class IGISPolygon;
class WED_Thing;

#include "Bezier.h"

/********************************************************************************************************************************************
 * BEZIER CURVE REPRESENTATION CONVERTERS
 ********************************************************************************************************************************************/


// This converts between the topologies of Bezier2 curves and the bounding end-points.
void	BezierToBezierPointStart (					  const Bezier2& next,BezierPoint2& out_pt);
void	BezierToBezierPointMiddle(const Bezier2& prev,const Bezier2& next,BezierPoint2& out_pt);	// Beziers must share a common point!
void	BezierToBezierPointEnd	 (const Bezier2& prev,					BezierPoint2& out_pt);
void	BezierPointToBezier(const BezierPoint2& p1,const BezierPoint2& p2, Bezier2& b);			

// These templates convert sequences of Bezier2 vs. BeizerPoint2.  Since these are SEQUENCES, a closed ring of BezierPoint2 will have a DUPLICATED
// common end point!

template <typename InputIterator, typename OutputIterator>
void	BezierToBezierPointSeq(InputIterator s, InputIterator e, OutputIterator oi);

template <typename InputIterator, typename OutputIterator>
void BezierPointToBezierSeq(InputIterator s, InputIterator e, OutputIterator oi);

// These tell us if our sequence is a closed ring.

template <typename InputIterator>
bool BezierPointSeqIsRing(InputIterator s, InputIterator e);

template <typename InputIterator>
bool BezierSeqIsRing(InputIterator s, InputIterator e);

// These convert between X-Plane's ridiculous "triple" notation where the lo and hi control handles MUST
// be mirors and our more sane independent notation.

template <typename InputIterator, typename OutputIterator>
void BezierPointSeqToTriple(InputIterator s, InputIterator e, OutputIterator o);

template <typename InputIterator, typename OutputIterator>
void BezierPointSeqFromTriple(InputIterator s, InputIterator e, OutputIterator o);

/********************************************************************************************************************************************
 * Polygon control
 ********************************************************************************************************************************************/

// These routines return UV mapped polygons for GIS entities.  They are ALWAYS exact, but FAIL on beziers.
// They return false if we get a bezier, true if we converted the all-side sequence.
bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2>& out_pol);			
bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment_2>& out_pol);			
bool	WED_PolygonForPointSequence(IGISPointSequence * in_seq, Polygon_2& out_pol, CGAL::Orientation);
bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, Polygon_with_holes_2& out_pol);

bool	WED_PolygonSetForEntity(IGISEntity * in_entity, Polygon_set_2& out_pgs);

// These routines create approximate polygons/polygon sets for GIS points/polygons.  Note that (1) we might have self intersections
// if the GIS data is junk.  And...we use an "epsi" to approximate this.  The intention of these routines is to get an _approximate_
// polygon that we can use for a UV map.  If the user has made a non-affine UV map, put it on a bezier, and clipped it, fer
// crying out loud, we are _not_ goin to get exact results, and we almost certainly don't care.
void	WED_ApproxPolygonForPointSequence(IGISPointSequence * in_seq, Polygon_2& out_pol, Polygon_2 * out_uv, double epsi);
void	WED_ApproxPolygonWithHolesForPolygon(IGISPolygon * in_poly, Polygon_with_holes_2& out_pol, Polygon_with_holes_2 * out_uv, double epsi);

// Same as above, but we create exact bezier polygons - yikes!  We blissfully ignore UV maps - we can re-establish those later!
void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2>& out_pol);			
void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier_curve_2>& out_pol);			
void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, Bezier_polygon_2& out_pol);				// requires closed ring
void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, Bezier_polygon_with_holes_2& out_pol);

template<typename __BezierSeqIter, class __NodeType>
void	WED_SetSequenceForIterator(__BezierSeqIter start, __BezierSeqIter end, WED_Thing * parent, bool is_ring);

/********************************************************************************************************************************************
 * UV Mapping
 ********************************************************************************************************************************************/

typedef vector<FastKernel::Triangle_2>		UVMap_t;

void	WED_MakeUVMap(
						const vector<Point_2>&	uv_map_ll,
						const vector<Point_2>&	uv_map_uv,
						UVMap_t&				out_map);

void	WED_MakeUVMap(
						IGISEntity *				in_quad,
						UVMap_t&				out_map);


void	WED_MapPoint(const UVMap_t&	in_map, const Point_2& ll, Point_2& uv);
void	WED_MapPolygon(const UVMap_t&	in_map, const Polygon_2& ll, Polygon_2& uv);
void	WED_MapPolygonWithHoles(const UVMap_t&	in_map, const Polygon_with_holes_2& ll, Polygon_with_holes_2& uv);

/********************************************************************************************************************************************
 * MORE COMPLEX IGIS-BASED OPERATIONS
 ********************************************************************************************************************************************/
 
// This will attempt to merge the above points, connecting any attached edges.
bool	WED_MergePoints(const vector<IGISEntity *>& in_points);

// This splits an edge at a given internal pt, forming two new edges.
bool	WED_SplitEdgeIfNeeded(WED_Thing * pt, const string& in_cross_name);





/********************************************************************************************************************************************
 * INLINES
 ********************************************************************************************************************************************/


// These templates convert sequences of Bezier2 vs. BeizerPoint2.  Since these are SEQUENCES, a closed ring of BezierPoint2 will have a DUPLICATED
// common end point!

template <typename InputIterator, typename OutputIterator>
void	BezierToBezierPointSeq(InputIterator s, InputIterator e, OutputIterator oi)
{
	if(s == e) return;
	BezierPoint2 b;
	BezierToBezierPointStart(*s, b);
	*oi = b;	
	while(s != e) {
		InputIterator n(s);
		++n;		
		if(n == e)		BezierToBezierPointEnd(*s, b);
		else			BezierToBezierPointMiddle(*s, *n, b);
		*oi = b;		
		s = n;		
	}
}

template <typename InputIterator, typename OutputIterator>
void BezierPointToBezierSeq(InputIterator s, InputIterator e, OutputIterator oi)
{
	if(s == e) return;
	InputIterator l(s);
	++s;
	if(s == e) return;

	while(s != e) {
		Bezier2 b;
		BezierPointToBezier(*l, *s, b);
		*oi = b;		
		l = s;
		++s;
	}
}

// These tell us if our sequence is a closed ring.

template <typename InputIterator>
bool BezierPointSeqIsRing(InputIterator s, InputIterator e)
{
	Point2 p1 = s->pt;
	Point2 p2 = s->pt;
	while(s != e)
	{
		p2 = s->pt;
		++s;
	}
	return p1 == p2;
}

template <typename InputIterator>
bool BezierSeqIsRing(InputIterator s, InputIterator e)
{
	Point2 p1 = s->p1;
	Point2 p2 = s->p2;
	while(s != e)
	{
		p2 = s->p2;
		++s;
	}
	return p1 == p2;
}

// These convert between X-Plane's ridiculous "triple" notation where the lo and hi control handles MUST
// be mirors and our more sane independent notation.

template <typename InputIterator, typename OutputIterator>
void BezierPointSeqToTriple(InputIterator s, InputIterator e, OutputIterator o)
{
	BezierPoint2 p;
	while(s != e)
	{
		if(s->has_lo() && s->has_hi() && !s->is_split())
			*o = *s;
		else
		{
			if(s->has_lo())
			{
				p.lo = s->lo;
				p.pt = s->pt;
				p.hi = p.pt + Vector2(p.lo,p.pt);
				*o = p;
			}
			p.lo = p.pt = p.hi = s->pt;
			*o = p;
			if(s->has_hi())
			{
				p.pt = s->pt;
				p.hi = s->hi;
				p.lo = p.pt + Vector2(p.hi,p.pt);
				*o = p;
			}
		}
		++s;
	}
}

template <typename InputIterator, typename OutputIterator>
void BezierPointSeqFromTriple(InputIterator s, InputIterator e, OutputIterator o)
{
	BezierPoint2	p;
	bool got_it = false;					// This is true if we have seen ANY part of this triple-point before.
	while(s != e)							// Basically the low control handle can only come first, so it must be set on lo, and
	{										// The high control handle can only be set later.  So lo, middle, hi encodes properly.
		if(got_it && s->pt != p.pt)
		{
			*o = p;
			got_it = false;
		}
		if(got_it && s->has_hi())			// Outgoing point with high handle?  use high
			p.hi = s->hi;
		else if (got_it)					// Center point...no high handle, clear high handle, means we are split
			p.hi = s->pt;
		else if (!got_it) {					// Low point - copy EVERYTHING, in case we aren't split.
			p = *s;
			got_it = true;					
		}
		++s;
	}
	if(got_it)
		*o = p;
}




#endif
