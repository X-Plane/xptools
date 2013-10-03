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

#include "CompGeomDefs2.h"

class IGISPointSequence;
class IGISEntity;
class IGISQuad;
class IGISPolygon;
class WED_Thing;

struct Segment2p : public Segment2 {
	int		param;
	
	Segment2p() : param(0) { }
	Segment2p(const Point2& ip1, const Point2& ip2, int p) : Segment2(ip1, ip2), param(p) { }
	Segment2p(const Segment2& rhs, int p) : Segment2(rhs), param(p) { }
	Segment2p(const Segment2p& rhs) : Segment2(rhs), param(rhs.param) { }
	
};

struct Bezier2p : public Bezier2 {
	int		param;
	
	Bezier2p() : param(0) { }
	Bezier2p(const Point2& ip1, const Point2& ic1, const Point2& ic2, const Point2& ip2, int p) : Bezier2(ip1,ic1,ic2,ip2), param(p) { }
	Bezier2p(const Bezier2& x, int p) : Bezier2(x), param(p) { }
	Bezier2p(const Segment2& x, int p) : Bezier2(x), param(p) { }
	Bezier2p(const Bezier2p& x) : Bezier2(x), param(x.param) { }
	Bezier2p(const Segment2p& x) : Bezier2(x), param(x.param) { }
	
	void subcurve(Bezier2p& sub, double t1, double t2) const { Bezier2::subcurve(sub,t1,t2); sub.param = param; }
	Segment2p	as_segment(void) const { return Segment2p(p1,p2, param); }
	
};

struct BezierPoint2p : public BezierPoint2 {
	int		param;	
};

struct BezierPolygon2p : public vector<Bezier2p> {
};

struct Polygon2p : public vector<Segment2p> {
};

/********************************************************************************************************************************************
 * BEZIER CURVE REPRESENTATION CONVERTERS
 ********************************************************************************************************************************************/


// This converts between the topologies of Bezier2 curves and the bounding end-points.
void	BezierToBezierPointStart (					  const Bezier2& next,BezierPoint2& out_pt);
void	BezierToBezierPointMiddle(const Bezier2& prev,const Bezier2& next,BezierPoint2& out_pt);	// Beziers must share a common point!
void	BezierToBezierPointEnd	 (const Bezier2& prev,					  BezierPoint2& out_pt);

void	BezierToBezierPointStart (					   const Bezier2p& next,BezierPoint2p& out_pt);
void	BezierToBezierPointMiddle(const Bezier2p& prev,const Bezier2p& next,BezierPoint2p& out_pt);	// Beziers must share a common point!
void	BezierToBezierPointEnd	 (const Bezier2p& prev,					    BezierPoint2p& out_pt);


void	BezierPointToBezier(const BezierPoint2&  p1,const BezierPoint2&  p2, Bezier2&  b);			
void	BezierPointToBezier(const BezierPoint2p& p1,const BezierPoint2p& p2, Bezier2p& b);			

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

// These return true if there are any beziers in the GIS entities...
int WED_HasBezierSeq(IGISPointSequence * ring);
int WED_HasBezierPol(IGISPolygon * pol);

// These routines return polygons for GIS entities.  They are ALWAYS exact, but FAIL on beziers.
// They return false if we get a bezier, true if we converted the all-side sequence.
//
// The CGAL variants will also fail if the polygon is invalid.
//
// For polygons, if the wanted orientation is CCW and CW (and not ZERO) then the polygon is reversed
// as needed to get the desired orientation.

bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2>& out_pol);			
bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2p>& out_pol);			

bool	WED_PolygonForPointSequence(IGISPointSequence * in_seq, Polygon2& out_pol, int orientation);
bool	WED_PolygonForPointSequence(IGISPointSequence * in_seq, Polygon2p& out_pol, int orientation);

bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2>& out_pol);
bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2p>& out_pol);

// These routines return bezier polygons for the given point sequence.  Since we never have to worry about "oh we got a curve"
// and we do not check orientation, no return codes are needed.
void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2>& out_pol);			
void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2p>& out_pol);			

void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2& out_pol, int orientation);				// requires closed ring
void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2p& out_pol, int orientation);				// requires closed ring

void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2>& out_pol);
void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2p>& out_pol);

template<typename __BezierSeqIter, class __NodeType>
void	WED_SetSequenceForIterator(__BezierSeqIter start, __BezierSeqIter end, WED_Thing * parent, bool is_ring);

/********************************************************************************************************************************************
 * UV Mapping
 ********************************************************************************************************************************************/

typedef vector<Triangle2>		UVMap_t;

bool	WED_MakeUVMap(
						IGISPolygon *			in_poly,
						UVMap_t&				out_map);


void	WED_MapPoint(const UVMap_t&	in_map, const Point2& ll, Point2& uv);
void	WED_MapPolygon(const UVMap_t&	in_map, const Polygon2& ll, Polygon2& uv);
void	WED_MapPolygonWithHoles(const UVMap_t&	in_map, const vector<Polygon2>& ll, vector<Polygon2>& uv);

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
	typename OutputIterator::container_type::value_type b;
//	BezierPoint2 b;
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
//		Bezier2 b;
		typename OutputIterator::container_type::value_type b;
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
	typename OutputIterator::container_type::value_type p;
	//BezierPoint2 p;
	while(s != e)
	{
		if(s->has_lo() && s->has_hi() && !s->is_split())
			*o = *s;
		else
		{
			if(s->has_lo())
			{
				p = *s;
				p.hi = p.pt + Vector2(p.lo,p.pt);
				*o = p;
			}
			p = *s;
			p.lo = p.hi = s->pt;
			*o = p;
			if(s->has_hi())
			{
				p = *s;
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
//	BezierPoint2	p;
	typename OutputIterator::container_type::value_type p;
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
