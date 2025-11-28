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
#include "MathUtils.h"

#if APL
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#define N_LAT 9
#define N_LON 18

static int8_t deviation_table[N_LON + 1][N_LAT] = {
	{ -4,  0,  1,  4,  8, 10, 13, 23, 48},   // deviations at longitude -180 from north to south
	{  6, 10, 11, 10,  9,  9, 14, 23, 45},
	{ 12, 19, 18, 14, 10,  9, 14, 23, 42},
	{  4, 10, 11, 13,  9,  8, 12, 22, 39},
	{-27,  0,  5,  5,  4,  6, 11, 21, 33},
	{-44,-26,-16, -9, -6, -3,  2, 12, 23},
	{-40,-31,-23,-16,-15,-16,-15, -5, 10},
	{-29,-24,-19,-12,-14,-20,-24,-17, -1},
	{-15,-13,-10, -5, -6,-12,-23,-21,-10},
	{  0, -1,  0,  1,  0, -4,-15,-22,-20},
	{ 15, 11,  8,  5,  3,  1, -9,-31,-35},
	{ 30, 22, 15,  6,  3,  0,-15,-42,-50},
	{ 42, 28, 18,  6,  1, -4,-18,-46,-63},
	{ 48, 25, 13,  4,  0, -3,-12,-39,-72},
	{ 35,  4, -1, -2, -1,  0, -3,-21,-76},
	{ -9,-16,-14, -8, -4,  0,  1, -3,-62},
	{-20,-18,-15, -9, -3,  2,  6, 10, 28},
	{-15,-10, -8, -3,  3,  7, 11, 19, 48},
	{ 80, 70, 60, 40, 20,  0,-20,-40,-60},   // NOT the data for +180, but rather the lattitudes
};

float MagneticDeviation(float lon, float lat)
{
	if ((lat < deviation_table[N_LON][0]) && lat > deviation_table[N_LON][N_LAT - 1])
		for (int x = 1; x < N_LAT; x++)
			if (lat > deviation_table[N_LON][x])
			{
				const float lon_step = 360.0f / N_LON;

				int	y0 = (lon + 180.0f) / lon_step;
				int	y1 = y0 + 1;
				if (y1 >= N_LON)	y1 = 0;

				auto west = -180.0f + y0 * lon_step;
				auto east = west + lon_step;

				auto north_dev = interp(west, deviation_table[y0][x - 1],
					east, deviation_table[y1][x - 1], lon);
				auto south_dev = interp(west, deviation_table[y0][x],
					east, deviation_table[y1][x], lon);

				return interp(deviation_table[N_LON][x - 1], north_dev,
					deviation_table[N_LON][x], south_dev, lat);
			}

	return 0.0;
}

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


static void PointSequenceToPolygon2(IGISPointSequence * ps, Polygon2& pol, double max_err)
{
	int n = ps->GetNumSides();
	
	for (int i = 0; i < n; ++i)
	{
		Bezier2		b;
		if (ps->GetSide(gis_Geo,i,b))
		{
#if 1
			double size_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
							  sqrt(Vector2(b.c1,b.c2).squared_length()) +
							  sqrt(Vector2(b.c2,b.p2).squared_length());
			int point_count = min(max((int) (size_approx / max_err), 3), 30);
			
			pol.reserve(pol.size() + point_count);
			
			for (int k = 0; k < point_count; ++k)
				pol.push_back(b.midpoint((float) k / (float) point_count));
#else
//			https://raphlinus.github.io/graphics/curves/2019/12/23/flatten-quadbez.html

			for (int k = 0; k < point_count; ++k)
				pol.push_back(b.midpoint((float) k / (float) point_count));
#endif
			if (i == n-1 && !ps->IsClosed())
				pol.push_back(b.p2);
		}
		else
		{
			pol.push_back(b.p1);
			if (i == n-1 && !ps->IsClosed())
				pol.push_back(b.p2);
		}
	}
}


void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, vector<Polygon2>& out_pol)
{
	int nn = in_poly->GetNumHoles();
//	out_pol.clear();
	out_pol.push_back(Polygon2());
	
// break up any bezier segments into plain vertices, allow certain max error only
//	WED_BezierPolygonForPointSequence(in_poly->GetOuterRing(),out_pol.back(), COUNTERCLOCKWISE);
	PointSequenceToPolygon2(in_poly->GetOuterRing(), out_pol.back(), 3e-4);

	for(int n = 0; n < nn; ++n)
	{
		out_pol.push_back(Polygon2());
//		WED_BezierPolygonForPointSequence(in_poly->GetNthHole(n),out_pol.back(), CLOCKWISE);
		PointSequenceToPolygon2(in_poly->GetNthHole(n), out_pol.back(), 3e-4);
	}
}
