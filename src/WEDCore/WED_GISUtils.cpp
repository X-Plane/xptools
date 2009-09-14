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
#include "WED_Globals.h"
/************************************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************************************/

#if !DEV
	#error we need an option to get approximate pt sequenes from beziers!
	#error we need to handle degunking self-intersecting and backward polygons!
#endif

bool WED_PolygonForPointSequence(IGISPointSequence * ps, Polygon_2& p, Polygon_2 * uv)
{
	DebugAssert(ps->IsClosed());
	DebugAssert(!uv || ps->HasLayer(gis_UV));
	int ss = ps->GetNumSides();
	for(int s = 0; s < ss; ++s)
	{
		Segment2 seg;
		Bezier2 bez;
		if(ps->GetSide(gis_Geo,s, seg, bez))
			return false;
	}
	int nn = ps->GetNumPoints();
	for(int n = 0; n < nn; ++n)
	{
		Point2	pt;
		ps->GetNthPoint(n)->GetLocation(gis_Geo,pt);
		p.push_back(ben2cgal(pt));
		if(uv)
		{
			Point2 st;
			ps->GetNthPoint(n)->GetLocation(gis_UV,st);
			uv->push_back(ben2cgal(st));
		}
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
			if(!WED_PolygonForPointSequence(p->GetOuterRing(), pring, NULL))
				return false;
			if (pring.orientation() == CGAL::CLOCKWISE)
				pring.reverse_orientation();
			
			Polygon_with_holes_2 pwh(pring);
			int nn = p->GetNumHoles();
			for(int n = 0; n < nn; ++n)
			{
				pring.clear();
				if(!WED_PolygonForPointSequence(p->GetNthHole(n), pring, NULL))
					return false;
				if(!pring.is_simple())
				{
					vector<Point_2>	errs;
					Traits_2			tr;					
					vector<Curve_2>	sides;
					for(Polygon_2::Edge_const_iterator e = pring.edges_begin(); e != pring.edges_end(); ++e)
					{
						sides.push_back(Curve_2(*e,0));
						#if DEV
						debug_mesh_line(cgal2ben(e->source()),cgal2ben(e->target()),1,0,0,  0,1,0  );
						#endif
					}
					CGAL::compute_intersection_points(sides.begin(),sides.end(), back_inserter(errs), false, tr);
					for(int n = 0; n < errs.size(); ++n)
					{
						#if DEV
						debug_mesh_point(cgal2ben(errs[n]),1,0,0);
						#endif
					}
					return false;
				}	
					
				if (pring.orientation() == CGAL::CLOCKWISE)
					pring.reverse_orientation();
				
				pring.reverse_orientation();
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
						const Polygon_2&		uv_map_ll,
						const Polygon_2&		uv_map_uv,
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
		out_map.push_back(Triangle_2(
					f->vertex(0)->info(),
					f->vertex(1)->info(),
					f->vertex(2)->info()));
	}
}

void	WED_MakeUVMap(
						IGISQuad *				in_quad,
						UVMap_t&				out_map)
{
	Point2	corners[4];
	Point2		ST[4];
	in_quad->GetCorners (gis_Geo,corners);
	in_quad->GetCorners(gis_UV,ST);
	Polygon_2 map_ll, map_uv;
	for(int n = 0; n < 4; ++n)
	{
		map_ll.push_back(ben2cgal(corners[n]));
		map_uv.push_back(ben2cgal(ST	   [n]));
	}
	WED_MakeUVMap(map_ll,map_uv, out_map);
}

void	WED_MakeUVMap(
						IGISPolygon *			in_quad,
						UVMap_t&				out_map)
{
	
}


bool	WED_MapPoint(const UVMap_t&	in_map, const Point_2& ll, Point_2& uv)
{
	for(int n = 0; n < in_map.size(); n += 2)
	if(in_map[n].bounded_side(ll) != CGAL::ON_UNBOUNDED_SIDE)
	{
		FastKernel::FT	total = in_map[n].area();
		
		FastKernel::FT a0 = Triangle_2(in_map[n].vertex(1), in_map[n].vertex(2), ll).area() / total;
		FastKernel::FT a1 = Triangle_2(in_map[n].vertex(2), in_map[n].vertex(0), ll).area() / total;
		FastKernel::FT a2 = Triangle_2(in_map[n].vertex(0), in_map[n].vertex(1), ll).area() / total;

		uv = CGAL::ORIGIN +
			Vector_2(CGAL::ORIGIN, in_map[n+1].vertex(0)) * a0 +
			Vector_2(CGAL::ORIGIN, in_map[n+1].vertex(1)) * a1 +
			Vector_2(CGAL::ORIGIN, in_map[n+1].vertex(2)) * a2;

		return true;
	}
	return false;	
}

bool	WED_MapPolygon(const UVMap_t&	in_map, const Polygon_2& ll, Polygon_2& uv)
{
	uv.clear();
	for(Polygon_2::Vertex_const_iterator v = ll.vertices_begin(); v != ll.vertices_end(); ++v)
	{
		Point_2	uvp;
		if (!WED_MapPoint(in_map, *v, uvp))
			return false;
		uv.push_back(uvp);
	}
	return true;
}

bool	WED_MapPolygonWithHoles(const UVMap_t&	in_map, const Polygon_with_holes_2& ll, Polygon_with_holes_2& uv)
{
	uv.clear();
	if(!ll.is_unbounded())
	{
		Polygon_2	outer_ring;
		if (!WED_MapPolygon(in_map,ll.outer_boundary(), outer_ring))
			return false;
		uv = Polygon_with_holes_2(outer_ring);
	}

	for(Polygon_with_holes_2::Hole_const_iterator h = ll.holes_begin(); h != ll.holes_end(); ++h)
	{
		Polygon_2	hole_uv;
		if (!WED_MapPolygon(in_map, *h, hole_uv))
			return false;
		uv.add_hole(hole_uv);
	}
	return true;
}

