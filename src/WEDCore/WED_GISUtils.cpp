/*
 *  WED_GISUtils.cpp
 *  SceneryTools
 *
 *  Created by bsupnik on 5/27/09.
 *  Copyright 2009 Laminar Research. All rights reserved.
 *
 */
 
#define USE_CGAL_POLYGONS 1

#include "WED_GISUtils.h"
#include "IGIS.h"
#if !defined(__i386__) && defined(IBM)
#define __i386__
#define __i386__defined 1
#endif
#include <CGAL/Sweep_line_2_algorithms.h>
#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>
#if __i386__defined
#undef __i386__
#undef __i386__defined
#endif

#include "WED_GISEdge.h"
#include "WED_Thing.h"


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
		Segment2	s;
		if(in_seq->GetSide(gis_Geo, n, s, b))
			return false;

		out_pol.push_back(s);
	}
	return true;
}

bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment2p>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		Segment2	s;
		Segment2	sp;
		if(in_seq->GetSide(gis_Geo, n, s, b))
			return false;
		if(in_seq->GetSide(gis_Param, n, sp, b))
			return false;

		out_pol.push_back(Segment2p(s,sp.p1.x()));
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


#if USE_CGAL_POLYGONS

bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment_2>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		Segment2	s;
		if(in_seq->GetSide(gis_Geo, n, s, b))
			return false;

		out_pol.push_back(Segment_2(ben2cgal<Point_2>(s.p1),ben2cgal<Point_2>(s.p2)));
	}
	return true;
}


bool WED_PolygonForPointSequence(IGISPointSequence * ps, Polygon_2& p, CGAL::Orientation wanted_orientation)
{
	DebugAssert(ps->IsClosed());
	int ss = ps->GetNumSides();
	DebugAssert(ss >= 3);
	for(int s = 0; s < ss; ++s)
	{
		Segment2 seg;
		Bezier2 bez;
		if(ps->GetSide(gis_Geo,s, seg, bez))
			return false;
	}
	int nn = ps->GetNumPoints();
	Point2 last;
	ps->GetNthPoint(nn-1)->GetLocation(gis_Geo,last);
	for(int n = 0; n < nn; ++n)
	{
		Point2	pt;
		ps->GetNthPoint(n)->GetLocation(gis_Geo,pt);
		if(pt != last)
			p.push_back(ben2cgal<Point_2>(pt));
		last = pt;
	}
	DebugAssert(p.size() >= 3);
	if(wanted_orientation != CGAL::ZERO)
	{
		if(!p.is_simple())
			return false;
		if(p.orientation() != wanted_orientation)
			p.reverse_orientation();
	}

// Debug code if we have to sort out non-simple polygons...
//					vector<Point_2>	errs;
//					Traits_2			tr;
//					vector<Curve_2>	sides;
//					for(Polygon_2::Edge_const_iterator e = pring.edges_begin(); e != pring.edges_end(); ++e)
//					{
//						sides.push_back(Curve_2(*e,0));
//						#if DEV
//						debug_mesh_line(cgal2ben(e->source()),cgal2ben(e->target()),1,0,0,  0,1,0  );
//						#endif
//					}
//					CGAL::compute_intersection_points(sides.begin(),sides.end(), back_inserter(errs), false, tr);

	return true;
}

bool	WED_PolygonWithHolesForPolygon(IGISPolygon * in_poly, Polygon_with_holes_2& out_pol)
{
	if (!WED_PolygonForPointSequence(in_poly->GetOuterRing(), out_pol.outer_boundary(), CGAL::COUNTERCLOCKWISE))
		return false;
	int nn = in_poly->GetNumHoles();
	for(int n = 0; n < nn; ++n)
	{
		Polygon_2 hole;
		if (!WED_PolygonForPointSequence(in_poly->GetNthHole(n), hole, CGAL::CLOCKWISE))
			return false;
		out_pol.add_hole(hole);
	}

	Traits_2 tr;
	bool ok = CGAL::is_valid_unknown_polygon(out_pol,tr);

//	bool ok = tr.is_valid_2_object()(out_pol);
	return ok;
}
#endif


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

#if USE_CGAL_POLYGONS

bool	WED_PolygonSetForEntity(IGISEntity * in_entity, Polygon_set_2& out_pgs)
{
	IGISQuad *			q;
	IGISBoundingBox *	b;
	IGISPolygon *		p;
	IGISComposite *		c;

	out_pgs.clear();
	switch(in_entity->GetGISClass()) {

	case gis_Polygon:
		if((p = SAFE_CAST(IGISPolygon, in_entity)) != NULL)
		{
			Polygon_2	pring;
			if(!WED_PolygonForPointSequence(p->GetOuterRing(), pring, CGAL::COUNTERCLOCKWISE))
				return false;

			Polygon_with_holes_2 pwh(pring);
			int nn = p->GetNumHoles();
			for(int n = 0; n < nn; ++n)
			{
				pring.clear();
				if(!WED_PolygonForPointSequence(p->GetNthHole(n), pring, CGAL::CLOCKWISE))
					return false;
				pwh.add_hole(pring);
			}
			out_pgs.join(pwh);
		}
		break;
	case gis_BoundingBox:
		if((b = SAFE_CAST(IGISBoundingBox, in_entity)) != NULL)
		{
			Point2	pmin, pmax;
			b->GetMin()->GetLocation(gis_Geo,pmin);
			b->GetMax()->GetLocation(gis_Geo,pmax);

			Polygon_2	bounds;
			bounds.push_back(Point_2(pmin.x(),pmin.y()));
			bounds.push_back(Point_2(pmax.x(),pmin.y()));
			bounds.push_back(Point_2(pmax.x(),pmax.y()));
			bounds.push_back(Point_2(pmin.x(),pmax.y()));
			DebugAssert(bounds.orientation() != CGAL::CLOCKWISE);
			out_pgs = bounds;

		}
		break;
	case gis_Composite:
		if((c = SAFE_CAST(IGISComposite, in_entity)) != NULL)
		{
			int nn = c->GetNumEntities();
			for(int n = 0; n < nn; ++n)
			{
				Polygon_set_2	pgs;
				if(!WED_PolygonSetForEntity(c->GetNthEntity(n), pgs))
					return false;
				out_pgs.join(pgs);
			}
		}
		break;
	default:
		if((q = SAFE_CAST(IGISQuad, in_entity)) != NULL)
		{
			Point2	crn[4];
			q->GetCorners(gis_Geo,crn);
			Polygon_2	bounds;
			bounds.push_back(ben2cgal<Point_2>(crn[3]));
			bounds.push_back(ben2cgal<Point_2>(crn[2]));
			bounds.push_back(ben2cgal<Point_2>(crn[1]));
			bounds.push_back(ben2cgal<Point_2>(crn[0]));
			DebugAssert(bounds.orientation() != CGAL::CLOCKWISE);
			out_pgs = bounds;
		}
		break;
	}
	return true;
}

void WED_ApproxPolygonForPointSequence(IGISPointSequence * ps, Polygon_2& p, Polygon_2 * uv, double epsi)
{
	DebugAssert(ps->IsClosed());
	DebugAssert(!uv || ps->HasLayer(gis_UV));
	int ss = ps->GetNumSides();
	for(int s = 0; s < ss; ++s)
	{
		Segment2 g_seg, uv_seg;
		Bezier2 g_bez, uv_bez;
		if (uv)
			ps->GetSide(gis_UV,s, uv_seg,uv_bez);

		if(ps->GetSide(gis_Geo,s, g_seg, g_bez))
		{
			vector<Point2>	pts_ben, uv_ben;
			if(uv)
				approximate_bezier_epsi_2(g_bez, uv_bez, epsi, back_inserter(pts_ben),back_inserter(uv_ben));
			else
				approximate_bezier_epsi(g_bez, epsi, back_inserter(pts_ben));
			for(int n = 0; n < pts_ben.size(); ++n)
			{
				p.push_back(ben2cgal<Point_2>(pts_ben[n]));
				if(uv)
					uv->push_back(ben2cgal<Point_2>(uv_ben[n]));
			}
		} else {
			p.push_back(ben2cgal<Point_2>(g_seg.p1));
			if(uv)
				uv->push_back(ben2cgal<Point_2>(uv_seg.p1));
		}
	}
}

void	WED_ApproxPolygonWithHolesForPolygon(IGISPolygon * in_poly, Polygon_with_holes_2& out_pol, Polygon_with_holes_2 * out_uv, double epsi)
{
	WED_ApproxPolygonForPointSequence(in_poly->GetOuterRing(), out_pol.outer_boundary(), out_uv ? &out_uv->outer_boundary() : NULL, epsi);

	int nn = in_poly->GetNumHoles();
	for(int n = 0; n < nn; ++n)
	{
		Polygon_2	h, huv;
		WED_ApproxPolygonForPointSequence(in_poly->GetNthHole(n), h, out_uv ? &huv : NULL, epsi);
		out_pol.add_hole(h);
		if(out_uv)
			out_uv->add_hole(huv);
	}
}

#endif

void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier2>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		Segment2	s;
		if(!in_seq->GetSide(gis_Geo, n, s, b))
		{
			b.p1 = b.c1 = s.p1;
			b.p2 = b.c2 = s.p2;
		}
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
		Segment2	s;
		if(!in_seq->GetSide(gis_Geo, n, s, b))
		{
			b.p1 = b.c1 = s.p1;
			b.p2 = b.c2 = s.p2;
		}
		if(!in_seq->GetSide(gis_Param, n, s, bp))
		{
			bp.p1 = bp.c1 = s.p1;
			bp.p2 = bp.c2 = s.p2;
		}
		out_pol.push_back(Bezier2p(b,bp.p1.x()));
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


#if !NO_CGAL_BEZIER

void	WED_BezierVectorForPointSequence(IGISPointSequence * in_seq, vector<Bezier_curve_2>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		Segment2	s;
		if(!in_seq->GetSide(gis_Geo, n, s, b))
		{
			b.p1 = b.c1 = s.p1;
			b.p2 = b.c2 = s.p2;
		}
		out_pol.push_back(ben2cgal(b));
	}
}

void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, Bezier_polygon_2& out_pol, CGAL::Orientation orientation)
{
	vector<Bezier_curve_2>	crvs;
	DebugAssert(in_seq->IsClosed());
	WED_BezierVectorForPointSequence(in_seq, crvs);

	Bezier_traits_base_ traits;
	Bezier_traits_base_::Make_x_monotone_2	monifier(traits.make_x_monotone_2_object());

	vector<CGAL::Object>	mono_curves;

	for(vector<Bezier_curve_2>::iterator c = crvs.begin(); c != crvs.end(); ++c)
		monifier(*c, back_inserter(mono_curves));

	out_pol.clear();

	for(vector<CGAL::Object>::iterator o = mono_curves.begin(); o != mono_curves.end(); ++o)
	{
		Bezier_polygon_2::X_monotone_curve_2	c;
		if(CGAL::assign(c,*o))
			out_pol.push_back(c);
		else
			Assert(!"No cast.");
	}

	if(orientation != CGAL::ZERO)
	{
		// Ben says: we should really check for self-intersecting polygons here...but...
		// GPS doesn't seem to care for orientation, nor does it know about them, so defer until later.
		// Once we get our orientation's right in WED_BezierPolygonWithHolesForPolygon we can validate the whole thing.
//		if(!out_pol.is_simple())
//			return false;
		if(orientation != out_pol.orientation())
			out_pol.reverse_orientation();
	}
}

void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, Bezier_polygon_with_holes_2& out_pol)
{
	WED_BezierPolygonForPointSequence(in_poly->GetOuterRing(), out_pol.outer_boundary(), CGAL::COUNTERCLOCKWISE);
	int nn = in_poly->GetNumHoles();
	for(int n = 0; n < nn; ++n)
	{
		Bezier_polygon_2 hole;
		WED_BezierPolygonForPointSequence(in_poly->GetNthHole(n), hole, CGAL::CLOCKWISE);
		out_pol.add_hole(hole);
	}
//	Bezier_traits_2 tr;
////	bool ok = tr.is_valid_2_object()(out_pol);
//	bool ok = CGAL::is_valid_unknown_polygon(out_pol,tr);
//	return ok;

}
#endif

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




/************************************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************************************/

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel SloppyKernel;
typedef SloppyKernel SloppyTraits_2;
typedef SloppyKernel::Point_2 Sloppy_point_2;

#ifndef CGALDefs_H
template <typename T>
T ben2cgal(const Point2& p) { return T(p.x(),p.y()); }

template <typename T>
Point2 cgal2ben(const T& p) { return Point2(p.x(),p.y()); }
#endif

#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_data_structure_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

typedef	CGAL::Triangulation_vertex_base_with_info_2<Point2, SloppyTraits_2>		tVb;
typedef CGAL::Triangulation_face_base_2<SloppyTraits_2>		tFb;
typedef	CGAL::Triangulation_data_structure_2<tVb, tFb>						tTDS;

typedef	CGAL::Triangulation_2<SloppyKernel, tTDS>							UV_Mesh;

void	WED_MakeUVMap(
						const vector<Point2>&	uv_map_ll,
						const vector<Point2>&	uv_map_uv,
						UVMap_t&				out_map)
{
	UV_Mesh	mesh;
	for(int n = 0; n < uv_map_ll.size(); ++n)
	{
		UV_Mesh::Vertex_handle v = mesh.insert(ben2cgal<Sloppy_point_2>(uv_map_ll[n]));
		v->info() = uv_map_uv[n];
	}
	out_map.clear();
	for(UV_Mesh::Face_iterator f = mesh.finite_faces_begin(); f != mesh.finite_faces_end(); ++f)
	{
		Triangle2 t(cgal2ben(f->vertex(0)->point()),
					cgal2ben(f->vertex(1)->point()),
					cgal2ben(f->vertex(2)->point()));
		if(t.is_ccw())
		{
			DebugAssert(t.signed_area() > 0.0);
			out_map.push_back(t);
			out_map.push_back(Triangle2(
						f->vertex(0)->info(),
						f->vertex(1)->info(),
						f->vertex(2)->info()));
		}
	}
}

static void	WED_MakeUVMapInternal(IGISEntity * entity, vector<Point2>& pts_ll, vector<Point2>& pts_uv)
{
	if(!entity->HasLayer(gis_UV))
		return;

	Point2	g,u;

	int n,nn;
	IGISPoint * pt;
	IGISPoint_Bezier * bez;
	IGISPointSequence * seq;
	IGISPolygon * poly;
	IGISBoundingBox * bbox;
	IGISComposite * comp;
	switch(entity->GetGISClass()) {
	case gis_Point:
	case gis_Point_Heading:
	case gis_Point_HeadingWidthLength:
		if((pt = dynamic_cast<IGISPoint *>(entity)) != NULL)
		{
			pt->GetLocation(gis_Geo,g);
			pt->GetLocation(gis_UV ,u);

			pts_ll.push_back(g);
			pts_uv.push_back(u);
		}
		break;
	case gis_Point_Bezier:
		if((bez = dynamic_cast<IGISPoint_Bezier *>(entity)) != NULL)
		{
			bez->GetLocation(gis_Geo,g);
			bez->GetLocation(gis_UV ,u);
			pts_ll.push_back(g);
			pts_uv.push_back(u);

			if(bez->GetControlHandleLo(gis_Geo,g))
			{
				bez->GetControlHandleLo(gis_UV,u);
				pts_ll.push_back(g);
				pts_uv.push_back(u);
			}

			if(bez->GetControlHandleHi(gis_Geo,g))
			{
				bez->GetControlHandleHi(gis_UV,u);
				pts_ll.push_back(g);
				pts_uv.push_back(u);
			}
		}
		break;
	case gis_PointSequence:
	case gis_Line:
	case gis_Line_Width:
	case gis_Edge:
	case gis_Ring:
	case gis_Chain:
		if((seq = dynamic_cast<IGISPointSequence *>(entity)) != NULL)
		{
			nn = seq->GetNumPoints();
			for(n = 0; n < nn; ++n)
				WED_MakeUVMapInternal(seq->GetNthPoint(n),pts_ll,pts_uv);
		}
		break;
//	case gis_Area:
	case gis_Polygon:
		if((poly = dynamic_cast<IGISPolygon *>(entity)) != NULL)
		{
			WED_MakeUVMapInternal(poly->GetOuterRing(),pts_ll,pts_uv);
			nn = poly->GetNumHoles();
			for(n = 0; n < nn; ++n)
				WED_MakeUVMapInternal(poly->GetNthHole(n),pts_ll,pts_uv);
		}
		break;
	case gis_BoundingBox:
		if((bbox = dynamic_cast<IGISBoundingBox *>(entity)) != NULL)
		{
			WED_MakeUVMapInternal(bbox->GetMin(),pts_ll,pts_uv);
			WED_MakeUVMapInternal(bbox->GetMax(),pts_ll,pts_uv);
		}
		break;
	case gis_Composite:
		if((comp = dynamic_cast<IGISComposite *>(entity)) != NULL)
		{
			nn = comp->GetNumEntities();
			for(n = 0; n < nn; ++n)
				WED_MakeUVMapInternal(comp->GetNthEntity(n),pts_ll,pts_uv);
		}
		break;
	}
}

void	WED_MakeUVMap(
						IGISEntity *			in_quad,
						UVMap_t&				out_map)
{
	vector<Point2> map_ll;
	vector<Point2> map_uv;
	WED_MakeUVMapInternal(in_quad, map_ll, map_uv);
	WED_MakeUVMap(map_ll,map_uv, out_map);
}

void WED_MapPoint(const UVMap_t&	in_map, const Point2& ll, Point2& uv)
{
	// If we find a point INSIDE a triangle, we take the bathymetric coordinates and we're done.
	// If we are outside a triangle, we track the CLOSEST outside point, trying closer ones.
	// We do this so that if the point is SLIGHTLY outside the mesh due to rounding error, we take the nearest
	// triangle's three points to establish our bathymetric coordinate system.  This will produce basically
	// perfect results for points just outside the mesh.
	double									best_dist;
	bool									found_one = false;

	for(int n = 0; n < in_map.size(); n += 2)
	{
		DebugAssert(in_map[n].is_ccw());
		
		bool is_in = in_map[n].inside_ccw(ll);
		bool want_it = is_in;
		if(!is_in)
		{
			if(found_one)
			{
				double my_dist =  	  in_map[n].squared_distance_ccw(ll);
				if(my_dist < best_dist)
				{
					want_it = true;
					best_dist = my_dist;
				}
			}
			else
			{
				want_it = true;
				found_one = true;
				best_dist = in_map[n].squared_distance_ccw(ll);
			}
		}

		if(want_it)
		{
			double a0, a1, a2;
			if(in_map[n].bathymetric_interp(ll,a0, a1, a2))
			{
				uv = Point2(0.0,0.0) + 
							Vector2(in_map[n+1].p1) * a0 +
							Vector2(in_map[n+1].p2) * a1 +
							Vector2(in_map[n+1].p3) * a2;
				if(is_in)
					return;
			}
			else
			{
				DebugAssert(!"Bathymetric failure.");
			}
		}
	}
}

void	WED_MapPolygon(const UVMap_t&	in_map, const Polygon2& ll, Polygon2& uv)
{
	uv.clear();
	for(Polygon2::const_iterator v = ll.begin(); v != ll.end(); ++v)
	{
		Point2	uvp;
		WED_MapPoint(in_map, *v, uvp);
		uv.push_back(uvp);
	}
}

void	WED_MapPolygonWithHoles(const UVMap_t&	in_map, const vector<Polygon2>& ll, vector<Polygon2>& uv)
{
	uv.clear();
	for(vector<Polygon2>::const_iterator h = ll.begin(); h != ll.end(); ++h)
	{
		Polygon2	hole_uv;
		WED_MapPolygon(in_map, *h, hole_uv);
		uv.push_back(hole_uv);
	}
}

bool	WED_MergePoints(const vector<IGISEntity *>& in_points)
{
	if(in_points.size() < 2) return false;
	vector<WED_Thing *>	things;

	Point2	avg(0.0,0.0);
	IGISPoint * keeper;

	for(int n = 0; n < in_points.size(); ++n)
	{
		WED_Thing * w = dynamic_cast<WED_Thing *>(in_points[n]);
		IGISPoint * p = dynamic_cast<IGISPoint *>(in_points[n]);
		if(w == NULL || p == NULL) return false;
		if(n == 0) keeper = p;
		things.push_back(w);
		Point2	t;
		p->GetLocation(gis_Geo,t);
		avg += Vector2(t);
	}
	avg.x_ *= (1.0 / (double) in_points.size());
	avg.y_ *= (1.0 / (double) in_points.size());
	keeper->SetLocation(gis_Geo, avg);

	for(int d = 1; d < things.size(); ++d)
	{
		set<WED_Thing *>	viewers;
		things[d]->GetAllViewers(viewers);
		for(set<WED_Thing *>::iterator v = viewers.begin(); v != viewers.end(); ++v)
		{
			(*v)->ReplaceSource(things[d], things[0]);
		}
		things[d]->SetParent(NULL, 0);
		things[d]->Delete();
	}
	return true;
}

bool WED_SplitEdgeIfNeeded(WED_Thing * pt, const string& in_cross_name)
{
	WED_Thing * par = pt->GetParent();
	WED_GISEdge * edge = dynamic_cast<WED_GISEdge *>(pt->GetParent());

	if(edge)
	{
		int split_idx = pt->GetMyPosition();

		pt->SetParent(par->GetParent(),par->GetMyPosition()+1);
		string pname;
		par->GetName(pname);
		pt->SetName(pname+"_"+in_cross_name);

		WED_Thing * s = edge->GetNthSource(0);
		WED_Thing * d = edge->GetNthSource(1);

		WED_GISEdge * edge2 = dynamic_cast<WED_GISEdge*>(edge->Clone());
		edge2->SetParent(edge->GetParent(), edge->GetMyPosition()+1);

		edge->RemoveSource(d);
		edge2->RemoveSource(s);
		edge->AddSource(pt,1);
		edge2->AddSource(pt,0);

		while(edge->CountChildren() > split_idx)
		{
			WED_Thing * k = edge->GetNthChild(split_idx);
			k->SetParent(NULL,0);
			k->Delete();
		}

		while(split_idx--)
		{
			WED_Thing * k = edge2->GetNthChild(0);
			k->SetParent(NULL,0);
			k->Delete();
		}
		return true;
	}
	return false;
}
