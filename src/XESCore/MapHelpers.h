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

#ifndef MapHelpers_H
#define MapHelpers_H

#include "MapDefs.h"
#include "STLUtils.h"	// for pqueue
#include "MathUtils.h"
#include "point_index.h"
#include "ProgressUtils.h"


#define DEBUG_SIMPLIFY 0

/************************************************************************************************************************************************
 * SUB-PMWX FOLLOWING
 ************************************************************************************************************************************************/
// These routines help identify connected half-edges that all meet some 'predicate'.  This is a way to work on a virtual meta-arrangement (where
// one meta-face is many faces) within an arrangement based on a run-time selectable parameter.  An example might be to look at the DSF map and
// select for terrain changes but not roads.  

// Given a half-edge that meets a predicate, if there is exactly one predicate going out from this predicate's target that ALSO meets the predicate,
// return it, otherwise return a null halfedge.  This lets us follow "poly-lines" of the predicate until we hit a node whose degree (with respect
// to the predicate) is not two.

template <class Arr, bool(* Pred)(typename Arr::Halfedge_handle)>
typename Arr::Halfedge_handle next_he_pred(typename Arr::Halfedge_handle he)
{
	DebugAssert(Pred(he));
	typename Arr::Halfedge_around_vertex_circulator circ, stop;
	typename Arr::Halfedge_handle ret, cand;
	circ = stop = he->target()->incident_halfedges();
	do
	{
		cand = circ->twin();
		if(circ != he)
		if(Pred(cand))
		{
			if (ret == typename Arr::Halfedge_handle())
				ret = cand;
			else
				return typename Arr::Halfedge_handle();
		}
	} while(++circ != stop);
	return ret;
}

// Given two vertices, this finds the left-hand face to a poly-line made up entirely of edges that 
// meet the predicate and travel from v1 to v2.  The poly-line must be connected only by nodes of
// degree 2 (with respect to the predicate).  
// Note that it is an error to call this if there exist several such paths (an ambiguous poly-line).
// If no such poly-line exists, return a null face handle.
// "wrong ways" are vertices that we must NOT go through.  This allows us to disambiguate going
// the wrong way around a loop.
template <typename Arr, bool(* Pred)(typename Arr::Halfedge_handle)>
typename Arr::Face_handle face_for_vertices(typename Arr::Vertex_handle v1, typename Arr::Vertex_handle v2, const set<typename Arr::Vertex_handle>& wrong_ways)
{
	typename Arr::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = v1->incident_halfedges();
	typename Arr::Face_handle ret;
	do
	{
		typename Arr::Halfedge_handle h = circ->twin();
		if(Pred(h))
		{
			while(h != Halfedge_handle() && h->target() != v2 && wrong_ways.count(h->target()) == 0)
				h = next_he_pred<Arr,Pred>(h);
			if(h != typename Arr::Halfedge_handle() && wrong_ways.count(h->target()) == 0)
			{
				DebugAssert(h->target() == v2);
				#if !DEV
					return circ->twin()->face();
				#endif
				// Debug mode runs ALL paths and checks to make sure
				// we don't have an ambiguity.
				if (ret != typename Arr::Face_handle())
					return typename Arr::Face_handle();
				DebugAssert(ret == typename Arr::Face_handle());
				ret = circ->twin()->face();
			}
		}
	} while(++circ != stop);
	return ret;
}

// Given two vertices, this finds the left-hand face to a poly-line made up entirely of edges that 
// meet the predicate and travel from v1 to v2.  The poly-line must be connected only by nodes of
// degree 2 (with respect to the predicate).  
// Note that it is an error to call this if there exist several such paths (an ambiguous poly-line).
// If no such poly-line exists, return a null face handle.
// "wrong ways" are vertices that we must NOT go through.  This allows us to disambiguate going
// the wrong way around a loop.
template <typename Arr, bool(* Pred)(typename Arr::Halfedge_handle)>
typename Arr::Halfedge_handle halfedge_for_vertices(typename Arr::Vertex_handle v1, typename Arr::Vertex_handle v2, const set<typename Arr::Vertex_handle>& wrong_ways)
{
	typename Arr::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = v1->incident_halfedges();
	typename Arr::Halfedge_handle ret;
	do
	{
		typename Arr::Halfedge_handle h = circ->twin();
		if(Pred(h))
		{
			while(h != Halfedge_handle() && h->target() != v2 && wrong_ways.count(h->target()) == 0)
				h = next_he_pred<Arr,Pred>(h);
			if(h != typename Arr::Halfedge_handle() && wrong_ways.count(h->target()) == 0)
			{
				DebugAssert(h->target() == v2);
				#if !DEV
					return circ->twin();
				#endif
				// Debug mode runs ALL paths and checks to make sure
				// we don't have an ambiguity.
				if (ret != typename Arr::Halfedge_handle())
					return typename Arr::Halfedge_handle();
				DebugAssert(ret == typename Arr::Halfedge_handle());
				ret = circ->twin();
			}
		}
	} while(++circ != stop);
	return ret;
}


// Given a vertex, how many of the incoming half-edges meet the predicate?
template <typename Arr, bool(* Pred)(typename Arr::Halfedge_handle)>
int degree_with_predicate(typename Arr::Vertex_handle v)
{
	if (v->is_isolated()) return 0;
	typename Arr::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = v->incident_halfedges();
	int t = 0;
	do {
		if(Pred(circ))
			++t;
	} while(++circ != stop);
	return t;
}

// given two vertices, how many unambiguous paths are there out of v1 to v2
// that use only the predicate?
template <typename Arr, bool(* Pred)(typename Arr::Halfedge_handle)>
int count_paths_to(typename Arr::Vertex_handle v1, typename Arr::Vertex_handle v2)
{
	typename Arr::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = v1->incident_halfedges();
	int ret = 0;
	do
	{
		typename Arr::Halfedge_handle h = circ->twin();
		if(Pred(h))
		{
			// This 'spoke' off of v1 via 'h' is in the predicate set.
			// Run unambiguously until we hit something - v2, or a non-deg-2 node.
			while(h != Halfedge_handle() && h->target() != v2)
				h = next_he_pred<Arr,Pred>(h);
			// Now if we stopped at a vert and not a dead end (non-deg-2 node) we fonud a path.
			if(h != typename Arr::Halfedge_handle())
			{				
				DebugAssert(h->target() == v2);
				++ret;
			}
		}
	} while(++circ != stop);
	return ret;
}




/************************************************************************************************************************************************
 * CURVE DIRECTION UTILS
 ************************************************************************************************************************************************/


template<class He>
inline bool	he_is_same_direction(He he)
{
	return (he->curve().is_directed_right() == (he->direction() == CGAL::ARR_LEFT_TO_RIGHT));
}

template<class He>
inline He he_get_same_direction(He he)
{
	return he_is_same_direction(he) ? he : he->twin();
}

template<class He>
inline bool he_is_same_direction_as(He he, const Curve_2& c)
{
	return CGAL::angle(
		he->source()->point(),
		he->target()->point(),
		he->target()->point() + Vector_2(c.source(),c.target())) == CGAL::OBTUSE;
}

/************************************************************************************************************************************************
 * ARRANGEMENT OBSERVERS
 ************************************************************************************************************************************************/



template <class Arr>
class	data_preserver_t : public CGAL::Arr_observer<Arr> {
public:
	typedef	typename CGAL::Arr_observer<Arr> base;
	data_preserver_t() : base() { }
	data_preserver_t(Arr& a) : base(a) { }

	virtual void after_split_face (typename Arr::Face_handle f,
                                 typename Arr::Face_handle new_f,
                                 bool is_hole)
	{
		new_f->set_data(f->data());
		new_f->set_contained(f->contained());
	}

	virtual void after_split_edge(typename Arr::Halfedge_handle e1, typename Arr::Halfedge_handle e2)
	{
		e2->set_data(e1->data());
		e2->twin()->set_data(e1->twin()->data());
	}
};

template <class Arr>
class edge_collector_t : public data_preserver_t<Arr> {
public:

	typedef	data_preserver_t<Arr> base;
	edge_collector_t() : base() { }
	edge_collector_t(Arr& a) : base(a) { }


	typename Arr::X_monotone_curve_2	input;
	set<typename Arr::Halfedge_handle>	results;
	int ctr;

	// A new edge is created.  CGAL always inserts a sub-curve of the original curve, so the half-edge we
	// want is the one going in the same directoin as its underlying cuvre.  This is the "fast" case because
	// we don't have to do any geometry compares - CGAL caches all the information needed to detect this case
	// in the DCEL.
	virtual void after_create_edge (typename Arr::Halfedge_handle e)
	{
		results.insert(he_get_same_direction<typename Arr::Halfedge_handle>(e));
		++ctr;
		DebugAssert(he_is_same_direction_as<typename Arr::Halfedge_handle>(he_get_same_direction<typename Arr::Halfedge_handle>(e), input));		// Debug validation that we are okay.
	}

	// Modify edge.  If the edge overlaps an existing one, we get this message.  In this case, we need to compare our
	// ideal curve to the one we got - CGAL will not reverse the underlying curve, just modify the tagged data.  So we
	// look at the original for orientation - slightly slower, requires a predicate.
	virtual void after_modify_edge (typename Arr::Halfedge_handle e)
	{
		if(he_is_same_direction_as(e, input))
			results.insert(e);
		else
			results.insert(e->twin());
		++ctr;
	}

	virtual void after_split_edge(typename Arr::Halfedge_handle e1, typename Arr::Halfedge_handle e2)
	{
		DebugAssert(results.count(e1) == 0);
		DebugAssert(results.count(e1->twin()) == 0);

		data_preserver_t<Arr>::after_split_edge(e1,e2);

		if(e1->source()->point() == input.source() &&
		   e1->target()->point() == input.target())				{ results.insert(e1); ++ctr; }

		if(e2->source()->point() == input.source() &&
		   e2->target()->point() == input.target())				{ results.insert(e2); ++ctr; }

		if(e1->target()->point() == input.source() &&
		   e1->source()->point() == input.target())				{ results.insert(e1->twin()); ++ctr; }

		if(e2->target()->point() == input.source() &&
		   e2->source()->point() == input.target())				{ results.insert(e2->twin()); ++ctr; }
	}


};

/************************************************************************************************************************************************
 * TOPOLOGY TESTING
 ************************************************************************************************************************************************/

// This visitor class is used to check whether an insert line would crash into, um, stuff.
template <typename Arr>
class check_split_zone_visitor {
public:
	typedef std::pair<typename Arr::Halfedge_handle, bool>            Result;

	X_monotone_curve_2			cv;
	typename Arr::Vertex_handle	v1;
	typename Arr::Vertex_handle	v2;
	bool						has_overlap;
	bool						has_split;
	bool						has_complete;
	typename Arr::Face_handle	found_face;

	void init (Arr *arr)
	{
		has_overlap = false;
		has_split = false;
		has_complete = false;
	}

	Result found_subcurve (const X_monotone_curve_2& partial,
							typename Arr::Face_handle face,
							typename Arr::Vertex_handle left_v, typename Arr::Halfedge_handle left_he,
							typename Arr::Vertex_handle right_v, typename Arr::Halfedge_handle right_he)
	{
		if(partial.left() != cv.left() || partial.right() != cv.right())
			has_split=true;
		else
			has_complete=true;
		found_face=face;
		return Result(typename Arr::Halfedge_handle(), true);
	}

	Result found_overlap (const X_monotone_curve_2& cv,
							typename Arr::Halfedge_handle he,
							typename Arr::Vertex_handle left_v, typename Arr::Vertex_handle right_v)
	{
		has_overlap = true;
		return Result(he,true);
	}
};

// This tells us if we can insert an edge between v1 and v2 without hitting any other edges or end-point nodes.
// it does NOT tell us if this would change the topology in a substantial way - just whether we can insert without
// intersection.
template<class Arr>
bool			can_insert(
							Arr&								p,
							typename Arr::Vertex_handle			v1,
							typename Arr::Vertex_handle			v2)
{
	if(v1->point() == v2->point()) return false;
	typedef	CGAL::Arrangement_zone_2<Arr,check_split_zone_visitor<Arr> >	zone_type;

	check_split_zone_visitor<Arr> v;
	zone_type		zone(p, &v);
	v.cv = X_monotone_curve_2(Segment_2(v1->point(),v2->point()),0);
	v.v1 = v1;
	v.v2 = v2;

	typename Arr::Vertex_const_handle lv = (v.cv.left() == v1->point()) ? v1 : v2;

	zone.init_with_hint(v.cv,CGAL::make_object(lv));
	zone.compute_zone();
	return !v.has_overlap && !v.has_split && v.has_complete;
}

/************************************************************************************************************************************************
 * SIMPLIFICATION
 ************************************************************************************************************************************************/

template <class Arr>
struct default_lock_traits {
	bool is_locked(typename Arr::Vertex_handle v) const { return false; }
	void remove(typename Arr::Vertex_handle v) const { }
};

// Arrangement Simplifier
// Given an arrangement, this simplifies it by merging adjacent edges and removing degree-two nodes.  It does so without changing the
// topology of the arrangement.  
// 
// The implementation uses a spatial point index to improve topology check speed.
// Besides a maximum error metric, you pass a functor that tells if a vertex should be locked (kept no matter what) and
// is notified when a vertex is removed.  Vertices whose degree are not 2 are never removed.

template <class Arr, class Traits=default_lock_traits<Arr> >
class	arrangement_simplifier {
public:
	
	// Traits has a lock_it method that is called to see if a vertex cannot be removed for arbitrary reasons.
	void simplify(Arr& io_block, double max_err, const Traits& tr=Traits(), ProgressFunc=NULL);

private:
	typedef	map<typename Arr::Halfedge_handle, list<Point2> >		Error_map;
	typedef pqueue<double, typename Arr::Vertex_handle>				Remove_Queue;

	// Given a vertex, how much error do we pick up by removing it?  Also, return the two half-edges pointing to the vertex.
	double simple_vertex_error(typename Arr::Vertex_handle v, typename Arr::Halfedge_handle * he1, typename Arr::Halfedge_handle * he2);
	
	// Return whether the error from removing 'v' is within max-err.  We evaluate ALL past points that were removed, to make sure cumulative error does not
	// get out of control.
	bool total_error_ok(typename Arr::Vertex_handle v, typename Arr::Halfedge_handle h1, typename Arr::Halfedge_handle h2, const list<Point2>& l1, const list<Point2>& l2, double max_err);
	
	// Given two HEs, queue up their end points into our curve map if needed.
	void queue_incident_edges_if_needed(typename Arr::Halfedge_iterator he1, typename Arr::Halfedge_iterator he2, Error_map& err_checks);

};


#pragma mark -

/************************************************************************************************************************************************
 * INLINE FUNCTIONS AND IMPLEMENTATION
 ************************************************************************************************************************************************/

// Given a halfedg, this tells if we can possibly merge it with its next halfedge.  The merge is impossible if:
// - The edge's target degree is not two (ambiguous who to merge with or no one to merge with) or
// - Our next edge's destination links to our source (we can't merge a triangle down to a line) or
// - We don't have the same X direction as our next edge.  This is a CGAL API limitation - since a merge is assumed
//   to replace one monotone curve with another, CGAL won't re-evalutate left-right caching flags.
template <class Arr>
bool			can_possibly_merge(
							Arr&							p,
							typename Arr::Halfedge_handle	he)
{
	if(he->target()->degree() != 2)	return false;		// Complex junction, not shape point? No merge!
	
	typename Arr::Vertex_handle v1 = he->source();
	typename Arr::Vertex_handle v2 = he->next()->target();

	if(v1->point() == v2->point()) return false;		// Same side of antenna?  no merge!
	
	// Ben says: by being very careful about the order of merge we can work around this limitation.
//	if(he->direction() != he->next()->direction())		// This is a CGAL bug/sadness: can't change the monotone direction of the underlying curve
//		return false;									// because it is cached. :-(

	typename Arr::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = v1->incident_halfedges();
	do 
	{
		if(circ->source() == v2)
		{
			return false;					// V1 and V2 already connected?  Ruh oh!
		}
	} while(++circ != stop);

	return true;
}

// A collection-visitor that checks for points within the triangle ABC - used to find 'squatters'.
struct visit_pt_in_tri {
	visit_pt_in_tri(const Point_2& a, const Point_2& b, const Point_2& c) : tri(a,b,c)
	{
		if(tri.orientation() == CGAL::CLOCKWISE)	
			tri = tri.opposite();
		count = 0;
	}
	Triangle_2	tri;
	bool operator()(const Point_2& p)
	{
		for(int n = 0; n < 3; ++n)
			if(tri.vertex(n) == p)
				return false;
		if(tri.has_on_unbounded_side(p))
			return false;
		++count;
		return true;
	}
	int count;
};

// Given a triangle ABC, this calculates the minimal search radius circle for our spatial index.
template <class Traits>
void	circle_to_search(const typename Traits::Point_2& a,const typename Traits::Point_2& b,const typename Traits::Point_2& c,typename Traits::Circle_2& circ)
{	
	// If any circle from two points contains the third, use that - otherwise, we'll build a cirlce aronud all 3. For nearly colinear points,
	// A cirlce touching all three is a lot bigger than one touching just two.
	if(CGAL::side_of_bounded_circle(a,b,c) != CGAL::ON_UNBOUNDED_SIDE)
		circ = typename Traits::Circle_2(a,b,CGAL::COUNTERCLOCKWISE);
	else if(CGAL::side_of_bounded_circle(b,c,a) != CGAL::ON_UNBOUNDED_SIDE)
		circ = typename Traits::Circle_2(b,c,CGAL::COUNTERCLOCKWISE);
	else if(CGAL::side_of_bounded_circle(c,a,b) != CGAL::ON_UNBOUNDED_SIDE)
		circ = typename Traits::Circle_2(c,a,CGAL::COUNTERCLOCKWISE);
	else
		circ = typename Traits::Circle_2(a,b,c);
}

// Given a pair of ordered points ABC, this returns true if the spatial index has any points in the interior of ABC or between A and C (not including B).
// We use this to check for squatters.
template <class Arr>
bool			squatters_in_area(
							const typename Arr::Point_2&						a,
							const typename Arr::Point_2&						b,
							const typename Arr::Point_2&						c,
							spatial_index_2<typename Arr::Geometry_traits_2>&	idx)
{
	CGAL::Orientation o = CGAL::orientation(a,b,c);

	if(o == CGAL::COLLINEAR)
	{
		if(CGAL::collinear_are_ordered_along_line(a,b,c))
			return false;
		else
		{
			DebugAssert("!How did we get here?");
			return true;
		}
	}

	DebugAssert(a != b);
	DebugAssert(b != c);
	DebugAssert(a != c);
	
	if(o == CGAL::LEFT_TURN)
	{
		visit_pt_in_tri visitor(a,b,c);		
		typename Arr::Geometry_traits_2::Circle_2 where;
		circle_to_search<typename Arr::Geometry_traits_2>(a,b,c,where);
		
		idx.search(where,visitor);
		return visitor.count > 0;
	}
	else
	{
		visit_pt_in_tri visitor(c,b,a);		

		Point_2 ctr = CGAL::circumcenter(a,b,c);
		typename Arr::Geometry_traits_2::Circle_2 where;
		circle_to_search<typename Arr::Geometry_traits_2>(c,b,a,where);

		idx.search(where,visitor);
		return visitor.count > 0;
	}
}

// Given a halfedge that can be merged with its next topology wise, are
// any squatters in the spatial index stopping us?
template <class Arr>
bool			squatters_stopping_merge(
							Arr&												p,
							typename Arr::Halfedge_handle						he,
							spatial_index_2<typename Arr::Geometry_traits_2>&	idx)
{
	DebugAssert(he->next() != he->twin());
	// Colinear points?  We are trying to merge truly colinear edges, which generally is fine.  can't do triangle query into
	// all vertices soo...

	typename Arr::Point_2 a(he->source()->point());
	typename Arr::Point_2 b(he->target()->point());
	typename Arr::Point_2 c(he->next()->target()->point());
	return squatters_in_area<Arr>(a,b,c,idx);
}							

// Utility adapter to dump points into an index directly from an arrangement.
template <class Arr>
struct arr_vertex_pt_extractor {
	typename Arr::Point_2 operator()(typename Arr::Vertex_handle v) const { return v->point(); }
};

template <class Arr, class Traits>
double arrangement_simplifier<Arr,Traits>::simple_vertex_error(typename Arr::Vertex_handle v, typename Arr::Halfedge_handle * he1, typename Arr::Halfedge_handle * he2)
{
	DebugAssert(v->degree() == 2);
	typename Arr::Halfedge_around_vertex_circulator circ = v->incident_halfedges();
	typename Arr::Halfedge_handle h1 = circ;
	++circ;
	typename Arr::Halfedge_handle h2 = circ;
	DebugAssert(h1->target() == v);
	DebugAssert(h2->target() == v);
	if(he1) *he1 = h1;
	if(he2) *he2 = h2;
	
	DebugAssert(h1->source()->point() != h2->source()->point());
	FastKernel::Line_2 sl(h1->source()->point(),h2->source()->point());
	return sqrt(CGAL::to_double(CGAL::squared_distance(sl, v->point())));
}

template <class Arr, class Traits>
bool arrangement_simplifier<Arr,Traits>::total_error_ok(typename Arr::Vertex_handle v, typename Arr::Halfedge_handle h1, typename Arr::Halfedge_handle h2, const list<Point2>& l1, const list<Point2>& l2, double max_err2)
{
	DebugAssert(v->degree() == 2);
	Line2 sl(cgal2ben(h1->source()->point()),cgal2ben(h2->source()->point()));
	list<Point2>::const_iterator p;
	for(p = l1.begin(); p != l1.end(); ++p)
	if(sl.squared_distance(*p) > max_err2)
		return false;
	for(p = l2.begin(); p != l2.end(); ++p)
	if(sl.squared_distance(*p) > max_err2)
		return false;
	return true;
}

template <class Arr, class Traits>
void arrangement_simplifier<Arr,Traits>::queue_incident_edges_if_needed(typename Arr::Halfedge_iterator he1, typename Arr::Halfedge_iterator he2, Error_map& err_checks)
{
	he1 = he_get_same_direction(he1);
	he2 = he_get_same_direction(he2);
	typename Error_map::iterator i1 = err_checks.find(he1);
	typename Error_map::iterator i2 = err_checks.find(he2);
	if(i1 == err_checks.end())
	{
		i1 = err_checks.insert(typename Error_map::value_type(he1, list<Point2>())).first;
		i1->second.push_back(cgal2ben(he1->source()->point()));
		i1->second.push_back(cgal2ben(he1->target()->point()));
	}
	if(i2 == err_checks.end())
	{
		i2 = err_checks.insert(typename Error_map::value_type(he2, list<Point2>())).first;
		i2->second.push_back(cgal2ben(he2->source()->point()));
		i2->second.push_back(cgal2ben(he2->target()->point()));
	}
}


template <class Arr, class Traits>
void arrangement_simplifier<Arr,Traits>::simplify(Arr& io_block, double max_err, const Traits& tr, ProgressFunc func)
{	
	if(io_block.is_empty())	return;
	DebugAssert(io_block.vertices_begin() != io_block.vertices_end());
	double err;
	typename Arr::Halfedge_handle he1, he2;
	double max_err2 = max_err*max_err;

	Remove_Queue	q;
	Error_map		err_checks;
	for(typename Arr::Vertex_iterator v = io_block.vertices_begin(); v != io_block.vertices_end(); ++v)
	if(v->degree() == 2)
	if(!tr.is_locked(v))
	if((err=simple_vertex_error(v, &he1, &he2)) <= max_err)
	{
		queue_incident_edges_if_needed(he1,he2,err_checks);
		q.insert(err, v);
	}
	
	int total = q.size();
	int check = max(total / 100, 1);
	PROGRESS_START(func,0,1,"Simplifying...")
	
//
//	Point2	minc = cgal2ben(io_block.vertices_begin()->point());
//	Point2	maxc = minc;
//	for(typename Arr::Vertex_iterator v = io_block.vertices_begin(); v != io_block.vertices_end(); ++v)
//	{
//		Point2	p = cgal2ben(v->point());
//		minc.x_ = min(minc.x(),p.x());
//		minc.y_ = min(minc.y(),p.y());
//		maxc.x_ = max(maxc.x(),p.x());
//		maxc.y_ = max(maxc.y(),p.y());
//	}
		
	spatial_index_2<typename Arr::Geometry_traits_2>	vertex_index;
	
	vertex_index.insert(io_block.vertices_begin(), io_block.vertices_end(), arr_vertex_pt_extractor<Arr>());
	
	
	int tot = 0, qwik=0;
	
//	int ctr = 1;
	while(!q.empty())
	{
//		if(ctr <= 0) 
//			break;
		PROGRESS_CHECK(func,0,1,"Simplifying...", (total-q.size()),total, check)
		typename Arr::Vertex_handle v = q.front_value();
//		printf("Q contains %d items, trying: 0x%08x\n", q.size(), &*v);
		q.pop_front();
		
		typename Arr::Halfedge_handle h1 =   v->incident_halfedges();
		typename Arr::Halfedge_handle h2 = ++v->incident_halfedges();

		typename Arr::Halfedge_handle k1 = he_get_same_direction(h1);
		typename Arr::Halfedge_handle k2 = he_get_same_direction(h2);
		typename Error_map::iterator i1 = err_checks.find(k1);
		typename Error_map::iterator i2 = err_checks.find(k2);
		DebugAssert(i1 != err_checks.end());
		DebugAssert(i2 != err_checks.end());
		DebugAssert(i1->second.size() >= 2);
		DebugAssert(i1->second.size() >= 2);

		if(total_error_ok(v, h1, h2, i1->second, i2->second, max_err2))
		{
			if(!tr.is_locked(v))
			{
				if(can_possibly_merge(io_block,h1))
				{
	//				if(!CGAL::collinear(h1->source()->point(),h1->target()->point(),h1	->next()->target()->point()))
	//				{
	//					debug_mesh_point(cgal2ben(h1->target()->point()),1,1,0);
	//					--ctr;
	//				}
					if(!squatters_stopping_merge(io_block,h1,vertex_index))
					{
						list<Point2>	ml;
						if(i1->second.front() == i2->second.front())
						{
							ml.swap(i1->second);
							ml.reverse();
							ml.pop_back();
							ml.splice(ml.end(), i2->second);
						}
						else if(i1->second.front() == i2->second.back())
						{
							ml.swap(i2->second);
							ml.pop_back();
							ml.splice(ml.end(),i1->second);
						}
						else if(i1->second.back() == i2->second.front())
						{
							ml.swap(i1->second);
							ml.pop_back();
							ml.splice(ml.end(),i2->second);
						}
						else if(i1->second.back() == i2->second.back())
						{
							ml.swap(i1->second);
							ml.pop_back();
							i2->second.reverse();
							ml.splice(ml.end(), i2->second);
						}
						else
						{
							DebugAssert(!"NO COMMON END VERTEX FOUND.");
						}
						DebugAssert(i1->second.empty());
						DebugAssert(i2->second.empty());
						err_checks.erase(i1);
						err_checks.erase(i2);
						
						q.erase(h1->source());
						q.erase(h2->source());
						
						tr.remove(h1->target());

						vertex_index.remove(h1->target()->point());
						
						// CGAL caches the x directions of half-edges. (Since all curves are
						// x-monotone the direction must be left or right.)  The problem is that
						// the x-direction of the merged edges might not match the x-direction of
						// the edge CGAL keeps.  So we have to carefully check what we're doing.
						
						// CGAL, as of this writing, will preserve the first of two half-edges if
						// you pass them to merge like this:
						//
						//   --h1-->o--h2-->
						//
						// Now there are two cases: if h1 and h2 have the SAME x direction, their
						// sum will have the same x direction.  That is, the sum of two x+ or x-
						// vectors is x+ or x-.
						//
						// If the two halfedges go in opposite directions, then the curve must match
						// one of them.
						// Case 1: the curve matches h1 - we just pass h1->h2.
						// Case 2: the curve matches h2.  Therefore the curve's opposite will 
						// match h2's twin.  we pass the curve's opposite and h2'->h1'.
						
						typename Arr::Halfedge_handle next = h1->next();
						Curve_2 nc(Segment_2(h1->source()->point(),next->target()->point()));

						typename Arr::Halfedge_handle remain;						
						if(nc.is_directed_right() == (h1->direction() == CGAL::ARR_LEFT_TO_RIGHT))
						{
							remain = io_block.merge_edge(h1,next,nc);
						}
						else 
						{
							Curve_2 nco(Segment_2(next->target()->point(), h1->source()->point()));
							DebugAssert(nco.is_directed_right() == (next->twin()->direction() == CGAL::ARR_LEFT_TO_RIGHT));
							remain = io_block.merge_edge(next->twin(),h1->twin(),nc)->twin();
						}
					
						typename Arr::Halfedge_handle rk = he_get_same_direction(remain);
						DebugAssert(err_checks.count(rk) == 0);
						err_checks.insert(typename Error_map::value_type(rk, ml));
						
						DebugAssert(q.count(remain->source()) == 0);
						DebugAssert(q.count(remain->target()) == 0);
						if(remain->source()->degree() == 2)
						if(!tr.is_locked(remain->source()))
						{
							typename Arr::Halfedge_handle ne1, ne2;
							double e = simple_vertex_error(remain->source(), &ne1, &ne2);
							if(e <= max_err)
							{
								// Why would this be neeed?  Because the max vertex error may have been too great before we moved
								// the side before but not now.  This is partly becuase simple vertex error is the wrong metric, but
								// we can fine tune this later.
								queue_incident_edges_if_needed(ne1,ne2,err_checks);
								q.insert(e,remain->source());
		//						printf(" Q prev 0x%08x\n",&*remain->source());
							}
						}
						if(remain->target()->degree() == 2)
						if(!tr.is_locked(remain->target()))
						{
							typename Arr::Halfedge_handle ne1, ne2;
							double e = simple_vertex_error(remain->target(), &ne1, &ne2);
							if(e <= max_err)
							{
								queue_incident_edges_if_needed(ne1,ne2,err_checks);
								q.insert(e,remain->target());
		//						printf(" Q targ 0x%08x\n",&*remain->target());
							}
						}
					}
				}
			} 
		}
	}
	#if DEV && DEBUG_SIMPLIFY
	printf("Quick checks: %d, total checks: %d\n",qwik,tot);
	#endif
	PROGRESS_DONE(func,0,1,"Simplifying...")
	
}


#endif /* MapHelpers_H */
