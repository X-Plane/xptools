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
#include <CGAL/Sweep_line_2_algorithms.h>
//#include "WED_Globals.h"
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




/************************************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************************************/

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


bool	WED_VectorForPointSequence(IGISPointSequence * in_seq, vector<Segment_2>& out_pol)
{
	int ns = in_seq->GetNumSides();
	for(int n = 0; n < ns; ++n)
	{
		Bezier2		b;
		Segment2	s;
		if(in_seq->GetSide(gis_Geo, n, s, b))
			return false;

		out_pol.push_back(Segment_2(ben2cgal(s.p1),ben2cgal(s.p2)));
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
			p.push_back(ben2cgal(pt));
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
	return true;
}

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
			bounds.push_back(ben2cgal(crn[3]));
			bounds.push_back(ben2cgal(crn[2]));
			bounds.push_back(ben2cgal(crn[1]));
			bounds.push_back(ben2cgal(crn[0]));
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
				p.push_back(ben2cgal(pts_ben[n]));
				if(uv)
					uv->push_back(ben2cgal(uv_ben[n]));
			}
		} else {
			p.push_back(ben2cgal(g_seg.p1));
			if(uv)
				uv->push_back(ben2cgal(uv_seg.p1));			
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

void	WED_BezierPolygonForPointSequence(IGISPointSequence * in_seq, Bezier_polygon_2& out_pol)
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
}

void	WED_BezierPolygonWithHolesForPolygon(IGISPolygon * in_poly, Bezier_polygon_with_holes_2& out_pol)
{
	WED_BezierPolygonForPointSequence(in_poly->GetOuterRing(), out_pol.outer_boundary());
	int nn = in_poly->GetNumHoles();
	for(int n = 0; n < nn; ++n)
	{
		Bezier_polygon_2 hole;
		WED_BezierPolygonForPointSequence(in_poly->GetNthHole(n), hole);
		out_pol.add_hole(hole);
	}
}



/************************************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************************************/

//#include <CGAL/Origin.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_data_structure_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

typedef	CGAL::Triangulation_vertex_base_with_info_2<Point_2, Traits_2>		tVb;
typedef CGAL::Triangulation_face_base_with_info_2<Point_2, Traits_2>		tFb;
typedef	CGAL::Triangulation_data_structure_2<tVb, tFb>						tTDS;

typedef	CGAL::Triangulation_2<FastKernel, tTDS>								UV_Mesh;

void	WED_MakeUVMap(
						const vector<Point_2>&	uv_map_ll,
						const vector<Point_2>&	uv_map_uv,
						UVMap_t&				out_map)
{
	UV_Mesh	mesh;
	for(int n = 0; n < uv_map_ll.size(); ++n)
	{
		UV_Mesh::Vertex_handle v = mesh.insert(uv_map_ll[n]);
		v->info() = uv_map_uv[n];
	}
	out_map.clear();
	for(UV_Mesh::Face_iterator f = mesh.finite_faces_begin(); f != mesh.finite_faces_end(); ++f)
	{
		out_map.push_back(Triangle_2(
					f->vertex(0)->point(),
					f->vertex(1)->point(),
					f->vertex(2)->point()));

//		debug_mesh_point(cgal2ben(f->vertex(0)->point()),1,1,1);
//		debug_mesh_point(cgal2ben(f->vertex(1)->point()),1,1,1);
//		debug_mesh_point(cgal2ben(f->vertex(2)->point()),1,1,1);
//		debug_mesh_line (cgal2ben(f->vertex(0)->point()),cgal2ben(f->vertex(1)->point()),0,0,1,0,0,1);
//		debug_mesh_line (cgal2ben(f->vertex(1)->point()),cgal2ben(f->vertex(2)->point()),0,0,1,0,0,1);
//		debug_mesh_line (cgal2ben(f->vertex(2)->point()),cgal2ben(f->vertex(0)->point()),0,0,1,0,0,1);
//
//		debug_mesh_point(cgal2ben(f->vertex(0)->info()),1,1,1);
//		debug_mesh_point(cgal2ben(f->vertex(1)->info()),1,1,1);
//		debug_mesh_point(cgal2ben(f->vertex(2)->info()),1,1,1);
//		debug_mesh_line (cgal2ben(f->vertex(0)->info()),cgal2ben(f->vertex(1)->info()),0,0,1,0,0,1);
//		debug_mesh_line (cgal2ben(f->vertex(1)->info()),cgal2ben(f->vertex(2)->info()),0,0,1,0,0,1);
//		debug_mesh_line (cgal2ben(f->vertex(2)->info()),cgal2ben(f->vertex(0)->info()),0,0,1,0,0,1);					
					
		out_map.push_back(Triangle_2(
					f->vertex(0)->info(),
					f->vertex(1)->info(),
					f->vertex(2)->info()));
	}
}

static void	WED_MakeUVMapInternal(IGISEntity * entity, vector<Point_2>& pts_ll, vector<Point_2>& pts_uv)
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
			
			pts_ll.push_back(ben2cgal(g));
			pts_uv.push_back(ben2cgal(u));
		}
		break;
	case gis_Point_Bezier:
		if((bez = dynamic_cast<IGISPoint_Bezier *>(entity)) != NULL)
		{
			bez->GetLocation(gis_Geo,g);
			bez->GetLocation(gis_UV ,u);
			pts_ll.push_back(ben2cgal(g));
			pts_uv.push_back(ben2cgal(u));
			
			if(bez->GetControlHandleLo(gis_Geo,g))
			{
				bez->GetControlHandleLo(gis_UV,u);
				pts_ll.push_back(ben2cgal(g));
				pts_uv.push_back(ben2cgal(u));
			}

			if(bez->GetControlHandleHi(gis_Geo,g))
			{
				bez->GetControlHandleHi(gis_UV,u);
				pts_ll.push_back(ben2cgal(g));
				pts_uv.push_back(ben2cgal(u));
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
	vector<Point_2> map_ll, map_uv;
	WED_MakeUVMapInternal(in_quad, map_ll, map_uv);
	WED_MakeUVMap(map_ll,map_uv, out_map);
}

void WED_MapPoint(const UVMap_t&	in_map, const Point_2& ll, Point_2& uv)
{
	// If we find a point INSIDE a triangle, we take the bathymetric coordinates and we're done.
	// If we are outside a triangle, we track the CLOSEST outside point, trying closer ones.
	// We do this so that if the point is SLIGHTLY outside the mesh due to rounding error, we take the nearest
	// triangle's three points to establish our bathymetric coordinate system.  This will produce basically
	// perfect results for points just outside the mesh.
	FastKernel::FT							best_dist;
	bool									found_one = false;

	for(int n = 0; n < in_map.size(); n += 2)
	{
		bool is_in = in_map[n].bounded_side(ll) != CGAL::ON_UNBOUNDED_SIDE;
		bool want_it = is_in;
		if(!is_in)
		{
			if(found_one)
			{
				FastKernel::FT my_dist =  	  squared_distance(in_map[n], ll);
				if(CGAL::compare(my_dist, best_dist) == CGAL::SMALLER)
				{
					want_it = true;
					best_dist = my_dist;
				}
			}
			else
			{
				FastKernel::FT my_dist =  	  squared_distance(in_map[n], ll);
				want_it = true;
				found_one = true;
				best_dist = my_dist;
			}
		}
		
		if(want_it)
		{
			FastKernel::FT	total = in_map[n].area();
			
			FastKernel::FT a0 = Triangle_2(in_map[n].vertex(1), in_map[n].vertex(2), ll).area() / total;
			FastKernel::FT a1 = Triangle_2(in_map[n].vertex(2), in_map[n].vertex(0), ll).area() / total;
			FastKernel::FT a2 = Triangle_2(in_map[n].vertex(0), in_map[n].vertex(1), ll).area() / total;

			uv = CGAL::ORIGIN +
				Vector_2(CGAL::ORIGIN, in_map[n+1].vertex(0)) * a0 +
				Vector_2(CGAL::ORIGIN, in_map[n+1].vertex(1)) * a1 +
				Vector_2(CGAL::ORIGIN, in_map[n+1].vertex(2)) * a2;

			if(is_in) 
				return;
		}
	}
}

void	WED_MapPolygon(const UVMap_t&	in_map, const Polygon_2& ll, Polygon_2& uv)
{
	uv.clear();
	for(Polygon_2::Vertex_const_iterator v = ll.vertices_begin(); v != ll.vertices_end(); ++v)
	{
		Point_2	uvp;
		WED_MapPoint(in_map, *v, uvp);
		uv.push_back(uvp);
	}
}

void	WED_MapPolygonWithHoles(const UVMap_t&	in_map, const Polygon_with_holes_2& ll, Polygon_with_holes_2& uv)
{
	uv.clear();
	if(!ll.is_unbounded())
	{
		Polygon_2	outer_ring;
		WED_MapPolygon(in_map,ll.outer_boundary(), outer_ring);
		uv = Polygon_with_holes_2(outer_ring);
	}

	for(Polygon_with_holes_2::Hole_const_iterator h = ll.holes_begin(); h != ll.holes_end(); ++h)
	{
		Polygon_2	hole_uv;
		WED_MapPolygon(in_map, *h, hole_uv);
		uv.add_hole(hole_uv);
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