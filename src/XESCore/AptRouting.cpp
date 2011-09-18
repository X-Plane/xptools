/*
 * Copyright (c) 2009, Laminar Research.
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

#include "AptRouting.h"

#include "MapDefs.h"
#include "MapPolygon.h"
#include "MapTopology.h"
#include <CGAL/centroid.h>
#include "WED_Globals.h"
#include "WED_GISUtils.h"
#include "WED_ToolUtils.h"
#include "MapHelpers.h"
#include "ISelection.h"
#include "IGIS.h"
#include "WED_Runway.h"
#include "WED_RampPosition.h"
#include "GISUtils.h"
#include "AptAlgs.h"
#include "PerfUtils.h"
#include <CGAL/Gmpq.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>


#define USE_EXACT 0

#if USE_EXACT
typedef FastKernel SkelKernel;
#else
typedef CGAL::Exact_predicates_inexact_constructions_kernel	SkelKernel;
#endif

namespace CGAL {
	template<typename K>
	class Has_inexact_constructions { };

	template <>
	class Has_inexact_constructions<Exact_predicates_inexact_constructions_kernel> {
	public:
		typedef Tag_true	type;
	};

	template<>
	class Has_inexact_constructions<FastKernel> {
	public:
		typedef Tag_false	type;
	};

	template<>
	class Has_inexact_constructions<Simple_cartesian<Gmpq> > {
	public:
		typedef Tag_true	type;
	};
};




#include <CGAL/Sweep_line_2_algorithms.h>
#include <CGAL/Straight_skeleton_builder_2.h>
#include <CGAL/Straight_skeleton_converter_2.h>
#include "WED_AptIE.h"


typedef SkelKernel::Point_2	FPoint_2;
typedef CGAL::Polygon_2<SkelKernel>	FPolygon_2;

#if USE_EXACT
inline FPoint_2 to_fast(const Point_2& p) { return p; }
#else
inline Point2	cgal2ben(const FPoint_2& p) { return Point2(CGAL::to_double(p.x()),CGAL::to_double(p.y())); }
inline FPoint_2	to_fast(const Point_2& p) { return FPoint_2(CGAL::to_double(p.x()),CGAL::to_double(p.y())); }
#endif

typedef CGAL::Straight_skeleton_2<SkelKernel>	Straight_skeleton_2;
typedef CGAL::Straight_skeleton_builder_2<CGAL::Straight_skeleton_builder_traits_2<SkelKernel>, Straight_skeleton_2> Straight_skeleton_builder_2;
typedef CGAL::Straight_skeleton_2<FastKernel>	Slow_skeleton_2;

struct comp_vhandle {
	bool operator()(const Slow_skeleton_2::Vertex_handle& lhs, const Slow_skeleton_2::Vertex_handle& rhs) const { return &*lhs < &*rhs; }
};

struct comp_hehandle {
	bool operator()(const Slow_skeleton_2::Halfedge_handle& lhs, const Slow_skeleton_2::Halfedge_handle& rhs) const { return &*lhs < &*rhs; }
};

/***************************************************************************************************************************************
 * NETWORK ROUTING INTERNAL CLASSES AND UTILS
 ***************************************************************************************************************************************/
#pragma mark -

// Allow in-place djikstra...since these structs don't go out to WED, who cares if we jam some extra fields in place?

cgal_node_t * cgal_edge_t::other(cgal_node_t * who) const
{
	if(who==src) return dst;
	if(who==dst) return src;
	DebugAssert(!"Logic error");
	return NULL;
}

double cgal_edge_t::cost(void) const
{
	double c=1.0;
	if(src->yuck) c++;
	if(dst->yuck) c++;
	return c * sqrt(CGAL::to_double(Segment_2(src->loc,dst->loc).squared_length()));
}

void cgal_net_t::clear(void)
{
	for(vector<cgal_node_t *>::iterator n = nodes.begin(); n != nodes.end(); ++n) delete *n;
	for(vector<cgal_edge_t *>::iterator e = edges.begin(); e != edges.end(); ++e) delete *e;
	nodes.clear();
	edges.clear();
}

void cgal_net_t::kill_edge(cgal_edge_t * who)
{
	vector<cgal_edge_t *>::iterator i;
	i = find(who->src->edges.begin(),who->src->edges.end(), who);
	DebugAssert(i != who->src->edges.end());
	who->src->edges.erase(i);
	i = find(who->dst->edges.begin(),who->dst->edges.end(), who);
	DebugAssert(i != who->dst->edges.end());
	who->dst->edges.erase(i);
	i = find(edges.begin(), edges.end(), who);
	DebugAssert(i != edges.end());
	edges.erase(i);
	delete who;
}

void cgal_net_t::purge_unused_edges(void)
{
	for(vector<cgal_edge_t*>::iterator e = edges.begin(); e != edges.end(); /* intentional*/ )
	{
		if((*e)->used)
			++e;
		else {
			int n = distance(edges.begin(), e);
			kill_edge(*e);
			e = edges.begin();
			advance(e,n);
		}
	}
}

void cgal_net_t::purge_unused_nodes(void)
{
	for(vector<cgal_node_t*>::iterator n = nodes.begin(); n != nodes.end(); /* intentional */)
	if((*n)->edges.empty())
	{
		delete *n;
		n = nodes.erase(n);
	}
	else
		++n;
}

cgal_node_t * cgal_net_t::new_node_at(Point_2 p)
{
	cgal_node_t * n = new cgal_node_t;
	nodes.push_back(n);
	n->loc = p;
	return n;
}

cgal_edge_t * cgal_net_t::new_edge_between(cgal_node_t * v1, cgal_node_t * v2)
{
	cgal_edge_t * e = new cgal_edge_t;
	edges.push_back(e);
	e->src = v1;
	e->dst = v2;
	DebugAssert(v1 != v2);
	v1->edges.push_back(e);
	v2->edges.push_back(e);
	e->used = false;
	return e;
}

cgal_node_t * cgal_net_t::split_edge_at(cgal_edge_t * e, Point_2 where)
{
	cgal_node_t * n = new_node_at(where);
	cgal_node_t * v1 = e->src;
	cgal_node_t * v2 = e->dst;
	kill_edge(e);
	new_edge_between(v1,n);
	new_edge_between(v2,n);
	return n;
}


struct node_comp_cost {
bool operator()(cgal_node_t * lhs, cgal_node_t * rhs) const {
	return lhs->cost < rhs->cost;
}
};


static bool calc_one_routing(cgal_net_t& io_net, cgal_node_t * src, cgal_node_t * dst, int debug)
{
	DebugAssert(src != dst);
	for(vector<cgal_node_t *>::iterator n = io_net.nodes.begin(); n != io_net.nodes.end(); ++n)
	{
		(**n).cost = 0;
		(**n).done = false;
		(**n).prev = NULL;
	}

	vector<cgal_node_t *>	hot_nodes;

	src->cost = 0.0;
	hot_nodes.push_back(src);

	while(dst->prev == NULL)
	{
		if(hot_nodes.empty())
			return false;

		cgal_node_t * n = hot_nodes.front();
		hot_nodes.erase(hot_nodes.begin());
		n->done = true;

		for(vector<cgal_edge_t *>::iterator inc = n->edges.begin(); inc != n->edges.end(); ++inc)
		{
			cgal_edge_t * e = *inc;
			cgal_node_t * w = e->other(n);

			if(w->done)													// totally done - skip him...Djikstra alg guarantees that if he's done,
			{															// we can't improve his path anyway, because we've already gone on a LONGER trip than him!
				#if DEV
					DebugAssert(w->cost <= n->cost + e->cost());		// verify djikstra isn't hosed...rounding error could be a porblem?
				#endif
				continue;

			}
			double nc = n->cost + e->cost();

			// if we have never gotten ot this node, or it's cheaper to get there via us,
			// mark this guy.
			if(w->prev == NULL || nc < w->cost)
			{
				if(w->prev == NULL) hot_nodes.push_back(w);
				w->prev = e;
				w->cost = nc;
			}
		}

		// resort the PQ by cost.  Mutable PQ would work too.
		sort(hot_nodes.begin(),hot_nodes.end(), node_comp_cost());
	}

	// Now:
	cgal_node_t * i = dst;
	while(i)
	{
		if(i->prev)
		{
			#if DEV
			if(debug)
				debug_mesh_line(cgal2ben(i->prev->src->loc),cgal2ben(i->prev->dst->loc), 1,0,0,  1,1,0);
			#endif
			i->prev->used = true;
			DebugAssert(i != i->prev->other(i));
			i = i->prev->other(i);
		} else
			break;
	}
	return true;

}

static void find_all_routes(cgal_net_t& io_map)
{
	static int debug = 0;

	int n = 0;
	for(vector<cgal_node_t *>::iterator sn = io_map.nodes.begin(); sn != io_map.nodes.end(); ++sn)
	if(!(**sn).poi.empty())
	{
		vector<cgal_node_t *>::iterator next(sn);
		++next;
		for(vector<cgal_node_t *>::iterator dn = next; dn != io_map.nodes.end(); ++dn)
		if(!(**dn).poi.empty())
		{
			bool worked = calc_one_routing(io_map,*sn,*dn, false);//n == debug);
			++n;
			DebugAssert(worked);
		}
	}
	++debug;
}

/***************************************************************************************************************************************
 * SKELETON-BASED NETWORK INSTANTIATION
 ***************************************************************************************************************************************/
#pragma mark -

bool make_map_with_skeleton(
					const vector<Polygon2>&		in_pavement,
					const vector<Point2>&		in_poi,
					cgal_net_t&					out_route)
{
	Polygon_set_2 merged_area;

	apt_make_map_from_polygons(in_pavement, merged_area);

	#if DEV
		for(Pmwx::Edge_iterator e = merged_area.arrangement().edges_begin();
								e != merged_area.arrangement().edges_end(); ++e)
			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,1,1,  1,1,1);

	#endif

		for(Pmwx::Vertex_iterator v = merged_area.arrangement().vertices_begin(); v != merged_area.arrangement().vertices_end(); ++v)
		if(v->degree() != 2)
		{
			printf("ERROR: input is not strictly simple!\n");
			return false;
		}

	if(!merged_area.arrangement().is_valid())
	{
		printf("Basic arrangement setup failed.\n");
		return false;
	}

	Straight_skeleton_builder_2	ssb;

	vector<Polygon_with_holes_2>	polys;

	vector<FPolygon_2>	outer_contours, inner_contours;

	double shortest = 9.9e9;

	merged_area.polygons_with_holes(back_inserter(polys));
	{
		StElapsedTime timer("enter contours");
		for(vector<Polygon_with_holes_2>::iterator poly = polys.begin(); poly != polys.end(); ++poly)
		{
			FPolygon_2		pts;
			for(Polygon_2::Vertex_iterator v = poly->outer_boundary().vertices_begin(); v != poly->outer_boundary().vertices_end(); ++v)
				pts.push_back(to_fast(*v));
			if(!pts.is_simple())
			{
				printf("Failure: input contour is not simple after conversion.\n");
				return false;
			}
			for(int n = 0; n < pts.size(); ++n)
			{
				FPolygon_2::Segment_2 s=pts.edge(n);
				shortest = min(shortest,sqrt(s.squared_length()));
			}

			outer_contours.push_back(pts);
			ssb.enter_contour(pts.vertices_begin(),pts.vertices_end());

			for(Polygon_with_holes_2::Hole_iterator h = poly->holes_begin(); h != poly->holes_end(); ++h)
			{
				FPolygon_2	hole;
				for(Polygon_2::Vertex_iterator v = h->vertices_begin(); v != h->vertices_end(); ++v)
					hole.push_back(to_fast(*v));

				if(!hole.is_simple())
				{
					printf("Failure: input contour is not simple after conversion.\n");
					return false;
				}
				DebugAssert(pts.bounded_side(hole[0]) == CGAL::ON_BOUNDED_SIDE);

				ssb.enter_contour(hole.vertices_begin(),hole.vertices_end());
				inner_contours.push_back(hole);
			}
		}
	}

	printf("Shortest side is: %lf degs.\n", shortest);

	boost::shared_ptr<Straight_skeleton_2> ss;
	{
		StElapsedTime timer("skel");
		ss = ssb.construct_skeleton();
		if(!ss) {
			printf("Skeleton construction failed.\n");
			return false;
		}
	}

	boost::shared_ptr<Slow_skeleton_2>	sss;

	#if USE_EXACT
		sss = ss;
	#else
	{
		StElapsedTime timer("convert");
		sss = CGAL::convert_straight_skeleton_2<Slow_skeleton_2,Straight_skeleton_2>(*ss);
		if(sss == NULL)
		{
			printf("Skeleton conversion failed.\n");
			return false;
		}
	}
	#endif

	{
		vector<Curve_2>	edges_to_check;
		vector<Point_2>	pts_to_check;

		typedef map<Slow_skeleton_2::Vertex_handle, cgal_node_t *, comp_vhandle>	node_map_t;
		typedef set<Slow_skeleton_2::Halfedge_handle, comp_hehandle>				edge_set_t;
		node_map_t::iterator i;

		node_map_t	nodes;
		edge_set_t	edges;


		int num_degen = 0;

		for(Slow_skeleton_2::Halfedge_iterator e = sss->halfedges_begin(); e != sss->halfedges_end(); ++e)
		if(!e->is_inner_bisector())
		if(e->is_bisector())
		{
			#if DEV
			debug_mesh_line(cgal2ben(e->vertex()->point()),cgal2ben(e->opposite()->vertex()->point()),0,0,1,0,0,1);
			#endif
		}


		for(Slow_skeleton_2::Halfedge_iterator e = sss->halfedges_begin(); e != sss->halfedges_end(); ++e)
		if(e->is_inner_bisector())
		if(edges.count(e) == 0)
		{
			if(e->vertex() == e->opposite()->vertex())
			{
				printf("ERROR: degenerate edge.\n");
				return false;
			}
			if(e->vertex()->point() == e->opposite()->vertex()->point())
			{
				num_degen++;
				#if DEV
				debug_mesh_point(cgal2ben(e->vertex()->point()),0.2,0.4,1);
				#endif
			} else
				edges_to_check.push_back(Curve_2(Segment_2(e->vertex()->point(),e->opposite()->vertex()->point())));
			edges.insert(e);
			edges.insert(e->opposite());
			Slow_skeleton_2::Vertex_handle v1 = e->vertex();
			Slow_skeleton_2::Vertex_handle v2 = e->opposite()->vertex();

			cgal_node_t * n1, * n2;

			i = nodes.find(v1);
			if(i == nodes.end())
			{
				n1 = out_route.new_node_at(v1->point());
				nodes.insert(node_map_t::value_type(v1,n1));
				pts_to_check.push_back(v1->point());
			}
			else
				n1 = i->second;

			#if DEV
			debug_mesh_line(cgal2ben(v1->point()),cgal2ben(v2->point()), 1,0,0,  0,1,0);
			#endif
			i = nodes.find(v2);
			if(i == nodes.end())
			{
				n2 = out_route.new_node_at(v2->point());
				nodes.insert(node_map_t::value_type(v2,n2));
				pts_to_check.push_back(v2->point());
			}
			else
				n2 = i->second;

			out_route.new_edge_between(n1,n2);
		}

		for(Pmwx::Edge_iterator e = merged_area.arrangement().edges_begin();
								e != merged_area.arrangement().edges_end(); ++e)
			edges_to_check.push_back(e->curve());

		Traits_2			tr;
		vector<Point_2>		errs;
//		CGAL::compute_intersection_points(edges_to_check.begin(), edges_to_check.end(), back_inserter(errs), false, tr);
		if(!errs.empty())
		{
			printf("Found %zd errs.\n", errs.size());
			#if DEV
			for(vector<Point_2>::iterator i = errs.begin(); i != errs.end(); ++i)
				debug_mesh_point(cgal2ben(*i),1,0,0);
			#endif
		}

		list<pair<Point_2, CGAL::Object> > queries;
		CGAL::locate(merged_area.arrangement(), pts_to_check.begin(), pts_to_check.end(), back_inserter(queries));

		int bad_locs = 0;

		for(list<pair<Point_2,CGAL::Object> >::iterator q = queries.begin(); q != queries.end(); ++q)
		{
			Pmwx::Face_const_handle f;
			if(CGAL::assign(f, q->second))
			{
				if(!f->contained())
				{
					printf("Point outside polygon\n");
					#if DEV
					debug_mesh_point(cgal2ben(q->first),1,0,0);
					#endif
					FPoint_2	approx = to_fast(q->first);

					bool in_any = false;
					for(vector<FPolygon_2>::iterator c = outer_contours.begin(); c != outer_contours.end(); ++c)
					if(c->bounded_side(approx) == CGAL::ON_BOUNDED_SIDE)
					{
						in_any = true;
						break;
					}
					if(in_any) printf("But: the point is inside the approximate boundary.\n");

					++bad_locs;
				}
			}
			else {
				printf("Point on polygon edge.\n");
				#if DEV
				debug_mesh_point(cgal2ben(q->first),1,0,0);
				#endif
				++bad_locs;
			}
		}

		if(bad_locs > 0 || !errs.empty())
		{
			printf("This layout is invalid: %zd skeleton self-intersections and %d points outside the polygon area.  (Also, %d degen)\n",
				errs.size(), bad_locs, num_degen);
			return false;
		} else if (num_degen)
			printf("Layout okay but %d degenreate edges.\n", num_degen);
	}

	for(vector<Point2>::const_iterator p = in_poi.begin(); p != in_poi.end(); ++p)
	{
		Point_2 loc = ben2cgal(*p);

		cgal_node_t * best_node = NULL;
		cgal_edge_t * best_edge = NULL;
		NT	best_d;
		for(vector<cgal_node_t*>::iterator n = out_route.nodes.begin(); n != out_route.nodes.end(); ++n)
		{
			NT my_dist = CGAL::squared_distance(loc, (*n)->loc);
			if(best_node == NULL || my_dist < best_d)
			{
				best_d=my_dist;
				best_node=(*n);
			}
		}
		for(vector<cgal_edge_t*>::iterator e = out_route.edges.begin(); e != out_route.edges.end(); ++e)
		{
			Segment_2	s((*e)->src->loc,(*e)->dst->loc);
			NT my_dist = CGAL::squared_distance(s, loc);
			my_dist *= 1.1;
			if(my_dist < best_d)
			{
				best_node = NULL;
				best_edge = (*e);
				best_d = my_dist;
			}
		}

		if(best_node)
		{
			cgal_node_t * new_poi_node = out_route.new_node_at(loc);
			new_poi_node->poi.push_back(*p);
			out_route.new_edge_between(best_node,new_poi_node);
		}
		else if (best_edge)
		{
			Line_2	l(best_edge->src->loc,best_edge->dst->loc);
			Point_2 on_e = l.projection(loc);
			cgal_node_t * n = out_route.split_edge_at(best_edge, on_e);

			cgal_node_t * new_poi_node = out_route.new_node_at(loc);
			new_poi_node->poi.push_back(*p);
			out_route.new_edge_between(n,new_poi_node);
		}
		else
			DebugAssert(!"Logic error.");

	}
	return true;
}

/***************************************************************************************************************************************
 * PLANAR-MAP TOPOLOGY BASED ROUTING
 ***************************************************************************************************************************************/
#pragma mark -

#if 0

static void make_cgal_net_for_map(
						Pmwx&					in_map,
						cgal_net_t&				out_map,
						const vector<Point2>&	in_poi)
{
	typedef map<Face_const_handle, cgal_node_t *> mapping_t;
	mapping_t		face_mapping;		// This is how we converted each CGAL face into a node.

	// Step 1. Every contained face gets a node at its centroid, e.g. "how do we get to this face?"

	for(Pmwx::Face_iterator f = in_map.faces_begin(); f != in_map.faces_end(); ++f)
	if(f->contained())
	{
		cgal_node_t * n = new cgal_node_t;
		out_map.nodes.push_back(n);
		face_mapping.insert(mapping_t::value_type(f,n));
		vector<Point_2> pts;
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ=stop=f->outer_ccb();
		do {
			pts.push_back(circ->target()->point());
		} while (++circ != stop);
		n->loc = CGAL::centroid(pts.begin(),pts.end());
		n->yuck = false;
	}

	// Step 2.  Every face that touches another face gets exactly one edge going between the two.
	for(Pmwx::Face_iterator f = in_map.faces_begin(); f != in_map.faces_end(); ++f)
	if(f->contained())
	{
		set<Face_handle> neighbors;
		FindAdjacentFaces(f, neighbors);
		for(set<Face_handle>::iterator n = neighbors.begin(); n != neighbors.end(); ++n)
		{
			Face_handle nn = *n;
			DebugAssert(nn != f);
			if(!nn->contained())
			{
				mapping_t::iterator i = face_mapping.find(f);
				if(i != face_mapping.end())
					i->second->yuck = true;
			}
			if(nn->contained() && Face_handle(f) < nn)			// Why ordered compare?  To make sure we add each edge ONCE even though we VISIT the edge twice!
			{
				mapping_t::iterator i;
				cgal_edge_t * ne = new cgal_edge_t;
				out_map.edges.push_back(ne);
				ne->used = false;
				i = face_mapping.find(f);
				DebugAssert(i != face_mapping.end());
				ne->src = i->second;
				i = face_mapping.find(nn);
				DebugAssert(i != face_mapping.end());
				ne->dst = i->second;

				ne->src->edges.push_back(ne);
				ne->dst->edges.push_back(ne);
			}
		}
	}

	// Step 3: "Sink" POIs into the various nodes.
	#if !DEV
		#error the right thing to do would actually be to connect each POI to the nearest node by a single edge?
		#error or better connect to EVERY nearby node via an edge and let the alg clean the mess up?
	#endif

	Locator	loc(in_map);

	for(vector<Point2>::const_iterator poi = in_poi.begin(); poi != in_poi.end(); ++poi)
	{
		Point_2 p = ben2cgal(*poi);

		CGAL::Object	o;
		o = loc.locate(p);

		Pmwx::Face_const_handle		lf;
		Pmwx::Halfedge_const_handle le;
		Pmwx::Vertex_const_handle	lv;

		if(CGAL::assign(lf,o))
		{
			// trivial case: directh it.
		}
		else if(CGAL::assign(le,o))
		{
			// hit an edge - try either face, prefer containment.
			if(le->face()->contained())
				lf = le->face();
			else
				lf = le->twin()->face();
		}
		else if(CGAL::assign(lv,o))
		{
			// hit a vertex.  Try all faces adgacent to incident halfedges.
			// Note that we don't need the "twin" of an incident edge -
			// that face must be the same face as our next incident.
			// (Becasue c->twin->next == c+1 for circulators.)
			Pmwx::Halfedge_around_vertex_const_circulator circ, stop;
			circ=stop=lv->incident_halfedges();
			do {
				if(circ->face()->contained())
				{
					lf = circ->face();
					break;
				}
			} while (++circ != stop);
		}
		else
		{
			DebugAssert(!"Locate found NO valid object?!? code bug.");
		}

		// Do some validation!?

		#if !DEV
			# error need better err checking
		#endif
		DebugAssert(lf->contained());	// We have a slight problem: the destination is not actually ON the layout.  Uh oh!
		mapping_t::iterator i = face_mapping.find(lf);
		DebugAssert(i != face_mapping.end());
		cgal_node_t * on = i->second;
		on->poi.push_back(*poi);
	}
}

#endif
