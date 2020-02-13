/*
 *  WED_GISUtils.cpp
 *  SceneryTools
 *
 *  Created by bsupnik on 5/27/09.
 *  Copyright 2009 Laminar Research. All rights reserved.
 *
 */
 
#include "WED_GISUtils.h"
#include "IGIS.h"

#include "WED_GISEdge.h"
#include "WED_Thing.h"

#if APL
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

void	BezierToBezierPointStart (const Bezier2& next,BezierPoint2& out_pt)
{
	out_pt.lo = out_pt.pt = next.p1;
	out_pt.hi = next.c1;
}

void	BezierToBezierPointMiddle(const Bezier2& prev,const Bezier2& next,BezierPoint2& out_pt)
{
	// CGAL gives imprecise matching...need to investigate.
//	DebugAssert(prev.p2 == next.p1);
	out_pt.lo = prev.c2;
	out_pt.hi = next.c1;
	out_pt.pt = next.p1;
}

void	BezierToBezierPointEnd	 (const Bezier2& prev,					BezierPoint2& out_pt)
{
	out_pt.lo = prev.c2;
	out_pt.hi = out_pt.pt = prev.p2;
}

void	BezierPointToBezier(const BezierPoint2& p1,const BezierPoint2& p2, Bezier2& b)
{
	b.p1 = p1.pt;
	b.c1 = p1.hi;
	b.c2 = p2.lo;
	b.p2 = p2.pt;
}



void	BezierToBezierPointStart (const Bezier2p& next,BezierPoint2p& out_pt)
{
	out_pt.lo = out_pt.pt = next.p1;
	out_pt.hi = next.c1;
	out_pt.param = next.param;
}

void	BezierToBezierPointMiddle(const Bezier2p& prev,const Bezier2p& next,BezierPoint2p& out_pt)
{
	// CGAL gives imprecise matching...need to investigate.
//	DebugAssert(prev.p2 == next.p1);
	out_pt.lo = prev.c2;
	out_pt.hi = next.c1;
	out_pt.pt = next.p1;
	out_pt.param = next.param;
}

void	BezierToBezierPointEnd	 (const Bezier2p& prev,					BezierPoint2p& out_pt)
{
	out_pt.lo = prev.c2;
	out_pt.hi = out_pt.pt = prev.p2;
	out_pt.param = prev.param;
}

void	BezierPointToBezier(const BezierPoint2p& p1,const BezierPoint2p& p2, Bezier2p& b)
{
	b.p1 = p1.pt;
	b.c1 = p1.hi;
	b.c2 = p2.lo;
	b.p2 = p2.pt;
	b.param = p1.param;
}






void	BezierToBezierPointStart (const Bezier2uv& next,BezierPoint2uv& out_pt)
{
	out_pt.lo = out_pt.pt = next.p1;
	out_pt.hi = next.c1;
	out_pt.uv.lo = out_pt.uv.pt = next.uv.p1;
	out_pt.uv.hi = next.uv.c1;
}

void	BezierToBezierPointMiddle(const Bezier2uv& prev,const Bezier2uv& next,BezierPoint2uv& out_pt)
{
	// CGAL gives imprecise matching...need to investigate.
//	DebugAssert(prev.p2 == next.p1);
	out_pt.lo = prev.c2;
	out_pt.hi = next.c1;
	out_pt.pt = next.p1;

	out_pt.uv.lo = prev.uv.c2;
	out_pt.uv.hi = next.uv.c1;
	out_pt.uv.pt = next.uv.p1;
}

void	BezierToBezierPointEnd	 (const Bezier2uv& prev,					BezierPoint2uv& out_pt)
{
	out_pt.lo = prev.c2;
	out_pt.hi = out_pt.pt = prev.p2;

	out_pt.uv.lo = prev.uv.c2;
	out_pt.uv.hi = out_pt.uv.pt = prev.uv.p2;
}

void	BezierPointToBezier(const BezierPoint2uv& p1,const BezierPoint2uv& p2, Bezier2uv& b)
{
	b.p1 = p1.pt;
	b.c1 = p1.hi;
	b.c2 = p2.lo;
	b.p2 = p2.pt;

	b.uv.p1 = p1.uv.pt;
	b.uv.c1 = p1.uv.hi;
	b.uv.c2 = p2.uv.lo;
	b.uv.p2 = p2.uv.pt;
}






/************************************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************************************/


int WED_HasBezierSeq(IGISPointSequence * ring)
{
	int np = ring->GetNumPoints();
	IGISPoint_Bezier * b;
	Point2 d;
	for(int n = 0; n < np; ++n)
	if(ring->GetNthPoint(n)->GetGISClass() == gis_Point_Bezier)
	if((b =dynamic_cast<IGISPoint_Bezier *>(ring->GetNthPoint(n))) != NULL)
	if(b->GetControlHandleHi(gis_Geo,d) || b->GetControlHandleLo(gis_Geo,d))
		return 1;
	return 0;
}

int WED_HasBezierPol(IGISPolygon * pol)
{
	if(WED_HasBezierSeq(pol->GetOuterRing())) return 1;
	int hc = pol->GetNumHoles();
	for(int h = 0; h < hc; ++h)
	if(WED_HasBezierSeq(pol->GetNthHole(h))) return 1;
	return 0;
}

// TODO:
//	#error we need an option to get approximate pt sequenes from beziers!
//	#error we need to handle degunking self-intersecting and backward polygons!

bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		if(in_seq->GetSide(gis_Geo, n, b))
			return false;

		out_pol.push_back(b.as_segment());
	}
	return true;
}

bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2p>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b, sp;
		if(in_seq->GetSide(gis_Geo, n, b))
			return false;
		if(in_seq->GetSide(gis_Param, n, sp))
			return false;

		out_pol.push_back(Segment2p(b.p1,b.p2,sp.p1.x()));
	}
	return true;
}

bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2uv>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b, uv;
		if(in_seq->GetSide(gis_Geo, n, b))
			return false;
		if(in_seq->GetSide(gis_UV, n, uv))
			return false;

		out_pol.push_back(Segment2uv(b.p1,b.p2,uv.p1, uv.p2));
	}
	return true;
}



bool WED_PolygonForPointSequence(IGISPointSequence * ps, Polygon2& p, int wanted_orientation)
{
	vector<Segment2> pp;
	if(!WED_VectorForPointSequence(ps,pp))
		return false;
	for(int i = 0; i < pp.size(); ++i)
		p.push_back(pp[i].p1);
		
	if ((wanted_orientation == CLOCKWISE && p.is_ccw()) ||
		(wanted_orientation == COUNTERCLOCKWISE && !p.is_ccw()))
	{
		reverse(p.begin(),p.end());
	}

	return true;
}

bool WED_PolygonForPointSequence(IGISPointSequence * ps, Polygon2p& p, int wanted_orientation)
{
	if(!WED_VectorForPointSequence(ps,p))
		return false;
	if ((wanted_orientation == CLOCKWISE		&&  is_ccw_polygon_seg(p.begin(),p.end())) ||
		(wanted_orientation == COUNTERCLOCKWISE && !is_ccw_polygon_seg(p.begin(),p.end())))
	{
		reverse(p.begin(),p.end());
		for(Polygon2p::iterator i = p.begin(); i != p.end(); ++i)
			swap(i->p1,i->p2);
	}
	return true;		
}

bool WED_PolygonForPointSequence(IGISPointSequence * ps, Polygon2uv& p, int wanted_orientation)
{
	if(!WED_VectorForPointSequence(ps,p))
		return false;
	if ((wanted_orientation == CLOCKWISE		&&  is_ccw_polygon_seg(p.begin(),p.end())) ||
		(wanted_orientation == COUNTERCLOCKWISE && !is_ccw_polygon_seg(p.begin(),p.end())))
	{
		reverse(p.begin(),p.end());
		for(Polygon2uv::iterator i = p.begin(); i != p.end(); ++i)
			swap(i->p1,i->p2);
	}
	return true;		
}





bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2>& out_pol)
{
	out_pol.clear();
	int nn = in_poly->GetNumHoles();
	out_pol.reserve(nn+1);
	out_pol.push_back(Polygon2());
	if (!WED_PolygonForPointSequence(in_poly->GetOuterRing(), out_pol.back(), COUNTERCLOCKWISE))
		return false;
	for(int n = 0; n < nn; ++n)
	{
		out_pol.push_back(Polygon2());
		if (!WED_PolygonForPointSequence(in_poly->GetNthHole(n), out_pol.back(), CLOCKWISE))
			return false;
	}
	return true;
}

bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2p>& out_pol)
{
	out_pol.clear();
	int nn = in_poly->GetNumHoles();
	out_pol.reserve(nn+1);
	out_pol.push_back(Polygon2p());
	if (!WED_PolygonForPointSequence(in_poly->GetOuterRing(), out_pol.back(), COUNTERCLOCKWISE))
		return false;
	for(int n = 0; n < nn; ++n)
	{
		out_pol.push_back(Polygon2p());
		if (!WED_PolygonForPointSequence(in_poly->GetNthHole(n), out_pol.back(), CLOCKWISE))
			return false;
	}
	return true;
}

bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2uv>& out_pol)
{
	out_pol.clear();
	int nn = in_poly->GetNumHoles();
	out_pol.reserve(nn+1);
	out_pol.push_back(Polygon2uv());
	if (!WED_PolygonForPointSequence(in_poly->GetOuterRing(), out_pol.back(), COUNTERCLOCKWISE))
		return false;
	for(int n = 0; n < nn; ++n)
	{
		out_pol.push_back(Polygon2uv());
		if (!WED_PolygonForPointSequence(in_poly->GetNthHole(n), out_pol.back(), CLOCKWISE))
			return false;
	}
	return true;
}






void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		in_seq->GetSide(gis_Geo, n, b);
		out_pol.push_back(b);
	}
}

void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2p>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		Bezier2		bp;
		in_seq->GetSide(gis_Geo, n, b);
		in_seq->GetSide(gis_Param, n, bp);
		out_pol.push_back(Bezier2p(b,bp.p1.x()));
	}
}

void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2uv>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		Bezier2		uv;
		in_seq->GetSide(gis_Geo, n, b);
		in_seq->GetSide(gis_UV, n, uv);
		out_pol.push_back(Bezier2uv(b,uv));
	}
}



void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2& out_pol, int orientation)
{
	WED_BezierVectorForPointSequence(in_seq, out_pol);
	
	if ((orientation == CLOCKWISE		 &&  is_ccw_polygon_seg(out_pol.begin(),out_pol.end())) ||
		(orientation == COUNTERCLOCKWISE && !is_ccw_polygon_seg(out_pol.begin(),out_pol.end())))
	{
		reverse(out_pol.begin(),out_pol.end());
		for(BezierPolygon2::iterator i = out_pol.begin(); i != out_pol.end(); ++i)
		{
			swap(i->p1,i->p2);
			swap(i->c1,i->c2);
		}
	}
}

void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2p& out_pol, int orientation)
{
	WED_BezierVectorForPointSequence(in_seq, out_pol);

	if ((orientation == CLOCKWISE		 &&  is_ccw_polygon_seg(out_pol.begin(),out_pol.end())) ||
		(orientation == COUNTERCLOCKWISE && !is_ccw_polygon_seg(out_pol.begin(),out_pol.end())))
	{
		reverse(out_pol.begin(),out_pol.end());
		for(BezierPolygon2p::iterator i = out_pol.begin(); i != out_pol.end(); ++i)
		{
			swap(i->p1,i->p2);
			swap(i->c1,i->c2);
		}
	}
}

void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, BezierPolygon2uv& out_pol, int orientation)
{
	WED_BezierVectorForPointSequence(in_seq, out_pol);

	if ((orientation == CLOCKWISE		 &&  is_ccw_polygon_seg(out_pol.begin(),out_pol.end())) ||
		(orientation == COUNTERCLOCKWISE && !is_ccw_polygon_seg(out_pol.begin(),out_pol.end())))
	{
		reverse(out_pol.begin(),out_pol.end());
		for(BezierPolygon2uv::iterator i = out_pol.begin(); i != out_pol.end(); ++i)
		{
			swap(i->p1,i->p2);
			swap(i->c1,i->c2);
		}
	}
	
}



void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2>& out_pol)
{
	int nn = in_poly->GetNumHoles();
	out_pol.clear();
	out_pol.reserve(nn+1);
	out_pol.push_back(BezierPolygon2());
	WED_BezierPolygonForPointSequence(in_poly->GetOuterRing(),out_pol.back(), COUNTERCLOCKWISE);
	for(int n = 0; n < nn; ++n)
	{
		out_pol.push_back(BezierPolygon2());
		WED_BezierPolygonForPointSequence(in_poly->GetNthHole(n),out_pol.back(), CLOCKWISE);
	}
}

void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2p>& out_pol)
{
	int nn = in_poly->GetNumHoles();
	out_pol.clear();
	out_pol.reserve(nn+1);
	out_pol.push_back(BezierPolygon2p());
	WED_BezierPolygonForPointSequence(in_poly->GetOuterRing(),out_pol.back(), COUNTERCLOCKWISE);
	for(int n = 0; n < nn; ++n)
	{
		out_pol.push_back(BezierPolygon2p());
		WED_BezierPolygonForPointSequence(in_poly->GetNthHole(n),out_pol.back(), CLOCKWISE);
	}
}

void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<BezierPolygon2uv>& out_pol)
{
	int nn = in_poly->GetNumHoles();
	out_pol.clear();
	out_pol.reserve(nn+1);
	out_pol.push_back(BezierPolygon2uv());
	WED_BezierPolygonForPointSequence(in_poly->GetOuterRing(),out_pol.back(), COUNTERCLOCKWISE);
	for(int n = 0; n < nn; ++n)
	{
		out_pol.push_back(BezierPolygon2uv());
		WED_BezierPolygonForPointSequence(in_poly->GetNthHole(n),out_pol.back(), CLOCKWISE);
	}
}

