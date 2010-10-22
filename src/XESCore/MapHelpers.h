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
#include "ProgressUtils.h"

#define DEBUG_SIMPLIFY 1

// move this

template <class Point>
class spatial_index {
public:
	spatial_index(const Point2& minc, const Point2& maxc, int idim)
	{
		dim = idim;
		min_corner = minc; max_corner = maxc; buckets.resize(dim*dim);
	}
	
	void	insert(const Point& p)
	{
		Point2 pp = cgal2ben(p);
		int x_bucket = double_interp(min_corner.x(),0,max_corner.x(),dim-1,pp.x());
		int y_bucket = double_interp(min_corner.y(),0,max_corner.y(),dim-1,pp.y());
		buckets[x_bucket + y_bucket * dim].push_back(p);
	}
	
	bool	pts_in_tri_no_corners(const Point& a, const Point& b, const Point& c)
	{
		Point2 aa(cgal2ben(a));
		Point2 bb(cgal2ben(b));
		Point2 cc(cgal2ben(c));
		
		int x1_bucket = intlim(double_interp(min_corner.x(),0,max_corner.x(),dim-1,min(min(aa.x(),bb.x()),cc.x()))-1,0,dim-1);
		int y1_bucket = intlim(double_interp(min_corner.y(),0,max_corner.y(),dim-1,min(min(aa.y(),bb.y()),cc.y()))-1,0,dim-1);
		int x2_bucket = intlim(double_interp(min_corner.x(),0,max_corner.x(),dim-1,max(max(aa.x(),bb.x()),cc.x()))+1,0,dim-1);
		int y2_bucket = intlim(double_interp(min_corner.y(),0,max_corner.y(),dim-1,max(max(aa.y(),bb.y()),cc.y()))+1,0,dim-1);
		
		Triangle_2 tri(a,b,c);
		for(int y = y1_bucket; y <= y2_bucket; ++y)
		for(int x = x1_bucket; x <= x2_bucket; ++x)
		for(typename list<Point>::const_iterator i = buckets[x+y*dim].begin(); i != buckets[x+y*dim].end(); ++i)
		if(tri.bounded_side(*i) != CGAL::ON_UNBOUNDED_SIDE)
		if (*i != a && 
			*i != b &&
			*i != c)		
			return true;
		return false;
	}
	
	int						dim;
	Point2					min_corner;
	Point2					max_corner;
	vector<list<Point> >	buckets;
};





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

template <typename Arr>
class check_split_zone_visitor {
public:
  typedef std::pair<typename Arr::Halfedge_handle, bool>            Result;

	X_monotone_curve_2		cv;
	typename Arr::Vertex_handle	v1;
	typename Arr::Vertex_handle	v2;
	bool					has_overlap;
	bool					has_split;
	bool					has_complete;
	typename Arr::Face_handle		found_face;

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
		if(				partial.left() != cv.left() ||
						partial.right() != cv.right())
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

template <class Arr>
bool			can_possibly_merge(
							Arr&							p,
							typename Arr::Halfedge_handle	he)
{
	if(he->target()->degree() != 2)	return false;
	
	typename Arr::Vertex_handle v1 = he->source();
	typename Arr::Vertex_handle v2 = he->next()->target();

	if(v1->point() == v2->point()) return false;
	if(he->direction() != he->next()->direction())
		return false;

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

template <class Arr>
bool			possible_interference_with_merge(
							Arr&							p,
							typename Arr::Halfedge_handle	he,
							spatial_index<Point_2>&			idx)
{
	DebugAssert(he->next() != he->twin());
	// Colinear points?  We are trying to merge truly colinear edges, which generally is fine.  can't do triangle query into
	// all vertices soo...
	if(CGAL::collinear(he->source()->point(),he->target()->point(),he->next()->target()->point()))
	{
		if(CGAL::collinear_are_ordered_along_line(he->source()->point(),he->target()->point(),he->next()->target()->point()))
			return false;
		else
		{
			DebugAssert("!How did we get here?");
			return true;
		}
	}

	Point_2 a(he->source()->point());
	Point_2 b(he->target()->point());
	Point_2 c(he->next()->target()->point());
	DebugAssert(a != b);
	DebugAssert(b != c);
	DebugAssert(a != c);
	if(idx.pts_in_tri_no_corners(a,b,c))
		return true;
	return false;
}							

template <class Arr>
bool			can_merge(
							Arr&							p,
							typename Arr::Halfedge_handle	he)
{
	// Ben says: degree isn't 2?  This can can happen - for example,
	//	*
	//	|\
	//	*-*
	//	|\|\
	//	*-*-*
	// Using GPS this is one outer triangle and one inner hole.  But as a planar map,
	// outside junctions have degree 4.  So...if our degree isn't 2, it probably means
	// our outer CCB touches our hole at a noded point.  No biggie, but we're not going
	// to remove that node, ever!	
	if(he->target()->degree() != 2)	return false;
	
	typename Arr::Vertex_handle v1 = he->source();
	typename Arr::Vertex_handle v2 = he->next()->target();

	if(v1->point() == v2->point()) return false;
	if(he->direction() != he->next()->direction())
		return false;

	// Ben says: we cannot test the collinear case (a truly unneeded vertex because it doesn't turn anything) because
	// we cannot tell what an "overlap" result from the test insertion REALLY means.  It could mean we are collinear, so the
	// test insertion of the "bypass" edge covers both originals perfectly.  It could also mean that there is already a bypass
	// edge in place that we are going to overlap.  In this second case, the merge will create an invalid pmwx.  So just
	// test collinear here, rather than trying to make the overlap detector wicked smart.
	if(CGAL::collinear(he->source()->point(),he->target()->point(),he->next()->target()->point()))
	if(CGAL::collinear_are_ordered_along_line(he->source()->point(),he->target()->point(),he->next()->target()->point()))
		return true;
	typedef	CGAL::Arrangement_zone_2<Arr,check_split_zone_visitor<Arr> >	zone_type;

	check_split_zone_visitor<Arr> v;
	zone_type		zone(p, &v);
	v.cv = X_monotone_curve_2(Segment_2(v1->point(),v2->point()),0);
	v.v1 = v1;
	v.v2 = v2;

	typename Arr::Vertex_const_handle lv = (v.cv.left() == v1->point()) ? v1 : v2;

	zone.init_with_hint(v.cv,CGAL::make_object(lv));
	zone.compute_zone();
	if (v.has_overlap || v.has_split || !v.has_complete) return false;

	// Topology check: if we have a non-convex quad, merging the edges might put them in a different face - but
	// CGAL's merge op does NOT handle topology changes.  So be sure the face isn't going to change!
	if(CGAL::left_turn(he->source()->point(),he->target()->point(),he->next()->target()->point()))
		return v.found_face == he->face();
	else
		return v.found_face == he->twin()->face();
}

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

template <class Arr>
struct default_lock_traits {
	bool is_locked(typename Arr::Vertex_handle v) const { return false; }
	void remove(typename Arr::Vertex_handle v) const { }
};

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
	

	Point2	minc = cgal2ben(io_block.vertices_begin()->point());
	Point2	maxc = minc;
	for(typename Arr::Vertex_iterator v = io_block.vertices_begin(); v != io_block.vertices_end(); ++v)
	{
		Point2	p = cgal2ben(v->point());
		minc.x_ = min(minc.x(),p.x());
		minc.y_ = min(minc.y(),p.y());
		maxc.x_ = max(maxc.x(),p.x());
		maxc.y_ = max(maxc.y(),p.y());
	}
		
	spatial_index<Point_2>	vertex_index(minc, maxc, 1024);
	for(typename Arr::Vertex_iterator v = io_block.vertices_begin(); v != io_block.vertices_end(); ++v)
	{
		vertex_index.insert(v->point());
	}
	
	int tot = 0, qwik=0;
	
	while(!q.empty())
	{
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
		if(!tr.is_locked(v))
		{
			if(can_possibly_merge(io_block,h1))
			if(!possible_interference_with_merge(io_block,h1,vertex_index) || can_merge(io_block, h1))
			{
				#if DEV && DEBUG_SIMPLIFY
				++tot;
				if (!possible_interference_with_merge(io_block,h1,vertex_index))
				{
					++qwik;					
					DebugAssert(can_merge(io_block, h1));
				}
//				printf(" Eliminate: 0x%08x\n",&*v);
				#endif
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
				
				typename Arr::Halfedge_handle next = h1->next();
				typename Arr::Halfedge_handle remain = io_block.merge_edge(h1,next,Curve_2(Segment_2(h1->source()->point(),next->target()->point())));
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
	#if DEV && DEBUG_SIMPLIFY
	printf("Quick checks: %d, total checks: %d\n",qwik,tot);
	#endif
	PROGRESS_DONE(func,0,1,"Simplifying...")
	
}


#endif /* MapHelpers_H */
