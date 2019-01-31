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

struct Segment2uv : public Segment2 {
	Segment2		uv;
	
	Segment2uv() { }
	Segment2uv(const Point2& ip1, const Point2& ip2, const Point2& uv1, const Point2& uv2) : Segment2(ip1, ip2), uv(uv1,uv2) { }
	Segment2uv(const Segment2& rhs, Segment2 u) : Segment2(rhs), uv(u) { }
	Segment2uv(const Segment2uv& rhs) : Segment2(rhs), uv(rhs.uv) { }
	
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

struct Bezier2uv : public Bezier2 {
	Bezier2		uv;
	
	Bezier2uv() { }
	Bezier2uv(const Point2& ip1, const Point2& ic1, const Point2& ic2, const Point2& ip2, const Point2& uvp1, const Point2& uvc1, const Point2& uvc2, const Point2& uvp2) : Bezier2(ip1,ic1,ic2,ip2), uv(uvp1,uvc1,uvc2,uvp2) { }
	Bezier2uv(const Bezier2& x, const Bezier2& u) : Bezier2(x), uv(u) { }
	Bezier2uv(const Segment2& x, const Segment2& u) : Bezier2(x), uv(u) { }
	Bezier2uv(const Bezier2uv& x) : Bezier2(x), uv(x.uv) { }
	Bezier2uv(const Segment2uv& x) : Bezier2(x), uv(x.uv) { }
	
	void subcurve(Bezier2uv& sub, double t1, double t2) const { Bezier2::subcurve(sub,t1,t2); uv.subcurve(sub.uv,t1,t2); }
	Segment2uv	as_segment(void) const { return Segment2uv(p1,p2, uv.p1, uv.p2); }
	
};

struct BezierPoint2p : public BezierPoint2 {
	int		param;	
};

struct BezierPoint2uv : public BezierPoint2 {
	BezierPoint2	uv;
};


struct BezierPolygon2p : public vector<Bezier2p> {
};

struct Polygon2p : public vector<Segment2p> {
};


struct BezierPolygon2uv : public vector<Bezier2uv> {
};

struct Polygon2uv : public vector<Segment2uv> {
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

void	BezierToBezierPointStart (					    const Bezier2uv& next, BezierPoint2uv& out_pt);
void	BezierToBezierPointMiddle(const Bezier2uv& prev,const Bezier2uv& next, BezierPoint2uv& out_pt);	// Beziers must share a common point!
void	BezierToBezierPointEnd	 (const Bezier2uv& prev,					   BezierPoint2uv& out_pt);


void	BezierPointToBezier(const BezierPoint2&  p1,const BezierPoint2&  p2, Bezier2&  b);			
void	BezierPointToBezier(const BezierPoint2p& p1,const BezierPoint2p& p2, Bezier2p& b);			
void	BezierPointToBezier(const BezierPoint2uv& p1,const BezierPoint2uv& p2, Bezier2uv& b);

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
bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2uv>& out_pol);

bool	WED_PolygonForPointSequence(IGISPointSequence * in_seq, Polygon2& out_pol, int orientation);
bool	WED_PolygonForPointSequence(IGISPointSequence * in_seq, Polygon2p& out_pol, int orientation);
bool	WED_PolygonForPointSequence(IGISPointSequence * in_seq, Polygon2uv& out_pol, int orientation);

bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2>& out_pol);
bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2p>& out_pol);
bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2uv>& out_pol);

// These routines return bezier polygons for the given point sequence.  Since we never have to worry about "oh we got a curve"
// and we do not check orientation, no return codes are needed.
void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2>& out_pol);			
void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2p>& out_pol);			
void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2uv>& out_pol);

void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2& out_pol, int orientation);				// requires closed ring
void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2p& out_pol, int orientation);				// requires closed ring
void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2uv& out_pol, int orientation);				// requires closed ring

void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2>& out_pol);
void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2p>& out_pol);
void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2uv>& out_pol);

template<typename __BezierSeqIter, class __NodeType>
void	WED_SetSequenceForIterator(__BezierSeqIter start, __BezierSeqIter end, WED_Thing * parent, bool is_ring);


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

template <typename T>
struct BezierConversionTraits {
	T conform_hi_from_lo(const T& p) const { T r(p); r.hi = r.pt + Vector2(r.lo,r.pt); return r; }
	T conform_lo_from_hi(const T& p) const { T r(p); r.lo = r.pt + Vector2(r.hi,r.pt); return r; }
	T conform_no_handles(const T& p) const { T r(p); r.lo = r.pt; r.hi = r.pt; return r; }

	void extract_hi(const T& i, T& o) const { o.hi = i.hi; }
	void extract_pt(const T& i, T& o) const { o.hi = i.pt; }
	void extract_all(const T& i, T& o) const { o = i; }
};

template <>
struct BezierConversionTraits<BezierPoint2uv> {
	BezierPoint2uv conform_hi_from_lo(const BezierPoint2uv& p) const { BezierPoint2uv r(p); r.hi = r.pt + Vector2(r.lo,r.pt); r.uv.hi = r.uv.pt + Vector2(r.uv.lo,r.uv.pt); return r; }
	BezierPoint2uv conform_lo_from_hi(const BezierPoint2uv& p) const { BezierPoint2uv r(p); r.lo = r.pt + Vector2(r.hi,r.pt); r.uv.lo = r.uv.pt + Vector2(r.uv.hi,r.uv.pt); return r; }
	BezierPoint2uv conform_no_handles(const BezierPoint2uv& p) const { BezierPoint2uv r(p); r.lo = r.hi = r.pt; r.uv.lo = r.uv.hi = r.uv.pt; return r; }

	void extract_hi(const BezierPoint2uv& i, BezierPoint2uv& o) const { o.hi = i.hi; o.uv.hi = i.uv.hi; }
	void extract_pt(const BezierPoint2uv& i, BezierPoint2uv& o) const { o.hi = i.pt; o.uv.hi = i.uv.pt; }
	void extract_all(const BezierPoint2uv& i, BezierPoint2uv& o) const { o = i; }
};


template <typename InputIterator, typename OutputIterator>
void BezierPointSeqToTriple(InputIterator s, InputIterator e, OutputIterator o)
{
	typename OutputIterator::container_type::value_type p;
	BezierConversionTraits<typename OutputIterator::container_type::value_type> traits;
	
	while(s != e)
	{
		if(s->has_lo() && s->has_hi() && !s->is_split())
			*o = *s;
		else
		{
			if(s->has_lo())
			{
				*o = traits.conform_hi_from_lo(*s);
			}
			*o = traits.conform_no_handles(*s);
			if(s->has_hi())
			{
				*o = traits.conform_lo_from_hi(*s);
			}
		}
		++s;
	}
}

template <typename InputIterator, typename OutputIterator>
void BezierPointSeqFromTriple(InputIterator s, InputIterator e, OutputIterator o)
{
	typename OutputIterator::container_type::value_type p;
	BezierConversionTraits<typename OutputIterator::container_type::value_type> traits;

	bool got_it = false;					// This is true if we have seen ANY part of this triple-point before.
	while(s != e)							// Basically the low control handle can only come first, so it must be set on lo, and
	{										// The high control handle can only be set later.  So lo, middle, hi encodes properly.
		if(got_it && s->pt != p.pt)
		{
			*o = p;
			got_it = false;
		}
		if(got_it && s->has_hi())			// Outgoing point with high handle?  use high
			traits.extract_hi(*s, p);
		else if (got_it)					// Center point...no high handle, clear high handle, means we are split
			traits.extract_pt(*s, p);
		else if (!got_it) {					// Low point - copy EVERYTHING, in case we aren't split.
			traits.extract_all(*s, p);
			got_it = true;
		}
		++s;
	}
	if(got_it)
		*o = p;
}




#endif
