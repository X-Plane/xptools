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

struct uv_vert {
	uv_vert() { }
	uv_vert(const Point2& ll, const Point2& st) : xy(ll), uv(st) { }
	Point2	xy;
	Point2	uv;
};

static UVMap_t *			out_map = NULL;
static vector<const uv_vert *>	verts;
static GLenum				vert_mode = GL_NONE;

#if !IBM
#define CALLBACK
#endif

static void CALLBACK uv_begin(GLenum mode)
{
	DebugAssert(verts.empty());
	DebugAssert(mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN);
	vert_mode = mode;
}

static void CALLBACK uv_vertex(const GLfloat * v)
{
	const uv_vert * vv = (const uv_vert *) v;	
	verts.push_back(vv);
}

static void uv_tri(const uv_vert * a,const uv_vert * b,const uv_vert * c)
{
	out_map->push_back(Triangle2(a->xy,b->xy,c->xy));
	out_map->push_back(Triangle2(a->uv,b->uv,c->uv));
}

static void CALLBACK uv_end()
{
	DebugAssert(vert_mode == GL_TRIANGLES || vert_mode == GL_TRIANGLE_STRIP || vert_mode == GL_TRIANGLE_FAN);
	DebugAssert(out_map != NULL);
	
	int i;
	int nv = verts.size();
	switch(vert_mode) {
	case GL_TRIANGLES:
		for(i = 2; i < nv; i += 3)
			uv_tri(verts[i-2],verts[i-1],verts[i]);
		break;
	case GL_TRIANGLE_STRIP:
		for(i = 2; i < nv; ++i)
			if(i % 2)
				uv_tri(verts[i-1],verts[i-2],verts[i]);
			else
				uv_tri(verts[i-2],verts[i-1],verts[i]);
		break;
	case GL_TRIANGLE_FAN:
		for(i = 2; i < nv; ++i)
			uv_tri(verts[0],verts[i-1],verts[i]);
		break;
	}
	
	vert_mode = GL_NONE;	
	verts.clear();
}

static void	WED_MakeUVMap(const vector<vector<uv_vert> >& poly, UVMap_t& uvmap)
{
	uvmap.clear();
	out_map = &uvmap;

	GLUtriangulatorObj * tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN,	(void (CALLBACK *)(void))uv_begin);
	gluTessCallback(tess, GLU_TESS_END,		(void (CALLBACK *)(void))uv_end);
	gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))uv_vertex);

	gluBeginPolygon(tess);
	
	for(vector<vector<uv_vert> >::const_iterator w = poly.begin(); w != poly.end(); ++w)
	{
		if(w != poly.begin())
			gluNextContour(tess, GLU_INTERIOR);
		
		for(vector<uv_vert>::const_iterator v = w->begin(); v != w->end(); ++v)
		{
			Point2 p = v->xy;
			double vv[3] = { 
					p.x(),
					p.y(),
					0 };
			gluTessVertex(tess, vv, (GLvoid*) &*v);
		}		
	}
	
	
	gluEndPolygon(tess);
	gluDeleteTess(tess);	
	out_map = NULL;
}

static bool WED_TO_UV_ps(IGISPointSequence * ps, vector<uv_vert>& out_vert)
{
	DebugAssert(ps->IsClosed());
	
	int n, ns = ps->GetNumSides();
	for(n = 0; n < ns; ++n)
	{
		Bezier2		llb, uvb;
		Segment2	lls, uvs;
		
		if(!ps->HasLayer(gis_UV))
			return false;
		
		bool bez_ll = ps->GetSide(gis_Geo, n, lls, llb);
		bool bez_uv = ps->GetSide(gis_UV, n, uvs, uvb);
		
		if(bez_ll != bez_uv)
			return false;
			
		if(bez_ll)
		{			
			out_vert.push_back(uv_vert(llb.p1,uvb.p1));
			if(out_vert.back().xy != llb.c1)
				out_vert.push_back(uv_vert(llb.c1,uvb.c1));
			if(out_vert.back().xy != llb.c2 &&
				llb.c2 != llb.p2)
			out_vert.push_back(uv_vert(llb.c2,uvb.c2));
		}
		else
		{
			out_vert.push_back(uv_vert(lls.p1,uvs.p1));
		}
	}
	return true;
}

static bool WED_TO_UV_poly(IGISPolygon * pp, vector<vector<uv_vert> >& out_vert)
{
	out_vert.push_back(vector<uv_vert>());
	if(!WED_TO_UV_ps(pp->GetOuterRing(), out_vert.back()))
		return false;
	
	int h, hc = pp->GetNumHoles();
	for(h = 0; h < hc; ++h)
	{
		if(!WED_TO_UV_ps(pp->GetNthHole(h), out_vert.back()))
			return false;
	}
	return true;
}

bool	WED_MakeUVMap(
						IGISPolygon *		poly, UVMap_t&	out_map)
{
	vector<vector<uv_vert> > verts;
	
	if(!WED_TO_UV_poly(poly,verts))
		return false;
	
	WED_MakeUVMap(verts,out_map);
	
	return true;
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
		// Special case: most points come from the source geo and need no interpolation - only
		// points from DSF tile cuts need interp.  But the bathymetric area process is full of
		// rounding error and borks our UV map.  So if we detect a direct hit on a corner, just
		// take it verbatim...this cleans the UV map a bit.
		if(in_map[n].p1 == ll)
		{
			uv = in_map[n+1].p1;
			return;
		}
		if(in_map[n].p2 == ll)
		{
			uv = in_map[n+1].p2;
			return;
		}
		if(in_map[n].p3 == ll)
		{
			uv = in_map[n+1].p3;
			return;
		}
	
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
	IGISPoint * keeper = NULL;

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
