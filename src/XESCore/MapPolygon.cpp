/*
 * Copyright (c) 2008, Laminar Research.  All rights reserved.
 *
 */

#include "hl_types.h"
#include "MapPolygon.h"
#include "STLUtils.h"

void	PolygonFromCCB(Pmwx::Ccb_halfedge_const_circulator circ, Polygon_2& out_poly, RingInset_t * out_inset, Inset_f func,Bbox_2 * extent)
{
	out_poly.clear();
	Pmwx::Ccb_halfedge_const_circulator stop = circ;
	if(extent)
		*extent=circ->source()->point().bbox();
	do {
		out_poly.push_back(circ->source()->point());
		if(out_inset && func) out_inset->push_back(func(circ));
		if(extent) *extent += circ->source()->point().bbox();
	} while (stop != ++circ);
}

void	PolygonFromFace(Pmwx::Face_const_handle in_face, Polygon_with_holes_2& out_ps, PolyInset_t * out_inset, Inset_f func,Bbox_2 * extent)
{
	// Ben says: EASY way would of course be to simply mark only that face as "included" and
	// rebuild the PS off of the arrangement.  But this would involve copying the entire map and
	// then the polygon set would have to delete 99.9% of it.  Too slow!

	// So instead, we rebuild the poly the old fashion way...we can use a PS with holes because we know
	// a single mesh face is a single contiguous area!  By using the tighter data type we pass this assumption
	// on to the client code.

	Polygon_2			outer_ccb;
	vector<Polygon_2>	holes;

	if(out_inset)
		out_inset->push_back(RingInset_t());

	if(!in_face->is_unbounded())
		PolygonFromCCB(in_face->outer_ccb(), outer_ccb, out_inset ? &out_inset->back() : NULL, func, extent);

	for(Pmwx::Hole_const_iterator h = in_face->holes_begin(); h != in_face->holes_end(); ++h)
	{
		if(out_inset)
			out_inset->push_back(RingInset_t());

		holes.push_back(Polygon_2());

		PolygonFromCCB(*h, holes.back(), out_inset ? &out_inset->back() : NULL, func, extent);
	}
	out_ps = Polygon_with_holes_2(outer_ccb,holes.begin(),holes.end());
}

void	MapPolygonForward(Polygon_2& io_poly, const CoordTranslator_2& translator)
{
	for(Polygon_2::Vertex_iterator v = io_poly.vertices_begin(); v != io_poly.vertices_end(); ++v)
		*v = translator.Forward(*v);
}

void	MapPolygonForward(Polygon_with_holes_2& io_poly, const CoordTranslator_2& translator)
{
	if(!io_poly.is_unbounded())	MapPolygonForward(io_poly.outer_boundary(), translator);
	for(Polygon_with_holes_2::Hole_iterator h = io_poly.holes_begin(); h != io_poly.holes_end(); ++h)
		MapPolygonForward(*h, translator);
}

void	MapPolygonReverse(Polygon_2& io_poly, const CoordTranslator_2& translator)
{
	for(Polygon_2::Vertex_iterator v = io_poly.vertices_begin(); v != io_poly.vertices_end(); ++v)
		*v = translator.Reverse(*v);
}

void	MapPolygonReverse(Polygon_with_holes_2& io_poly, const CoordTranslator_2& translator)
{
	if(!io_poly.is_unbounded())	MapPolygonReverse(io_poly.outer_boundary(), translator);
	for(Polygon_with_holes_2::Hole_iterator h = io_poly.holes_begin(); h != io_poly.holes_end(); ++h)
		MapPolygonReverse(*h, translator);
}

void	cgal2ben(const Polygon_2& cgal, Polygon2& ben)
{
	ben.resize(cgal.size());
	for(int n = 0; n < cgal.size(); ++n)
		ben[n] = cgal2ben(cgal.vertex(n));
}

void	cgal2ben(const Polygon_with_holes_2& cgal, vector<Polygon2>& ben)
{
	ben.push_back(Polygon2());
	if(!cgal.is_unbounded())	cgal2ben(cgal.outer_boundary(),ben.back());
	for(Polygon_with_holes_2::Hole_const_iterator h = cgal.holes_begin(); h != cgal.holes_end(); ++h)
	{
		ben.push_back(Polygon2());
		cgal2ben(*h,ben.back());
	}
}

void	ben2cgal(const Polygon2& ben, Polygon_2& cgal)
{
	cgal.clear();
	for(int n = 0; n < ben.size(); ++n)
		cgal.push_back(ben2cgal(ben[n]));
}

void	ben2cgal(const vector<Polygon2>& ben, Polygon_with_holes_2& cgal)
{
	vector<Polygon_2>	holes(ben.size());
	for(int n = 0; n < ben.size(); ++n)
		ben2cgal(ben[n],holes[n]);
	cgal = Polygon_with_holes_2(holes[0],holes.begin()+1,holes.end());
}




class check_split_zone_visitor {
public:
  typedef std::pair<Halfedge_handle, bool>            Result;

	X_monotone_curve_2	cv;
	Vertex_handle		v1;
	Vertex_handle		v2;
	bool				has_overlap;
	bool				has_split;
	bool				has_complete;
	Face_handle			found_face;

  void init (Arrangement_2 *arr)
  {
	has_overlap = false;
	has_split = false;
	has_complete = false;
  }

  Result found_subcurve (const X_monotone_curve_2& partial,
                         Face_handle face,
                         Vertex_handle left_v, Halfedge_handle left_he,
                         Vertex_handle right_v, Halfedge_handle right_he)
	{
		if(				partial.left() != cv.left() ||
						partial.right() != cv.right())
			has_split=true;
		else
			has_complete=true;
			found_face=face;
	return Result(Halfedge_handle(), true);
	}

  Result found_overlap (const X_monotone_curve_2& cv,
                        Halfedge_handle he,
                        Vertex_handle left_v, Vertex_handle right_v)
{
	has_overlap = true;
	return Result(he,true);
}

};

static bool			can_insert(
							Pmwx&					p,
							Vertex_handle			v1,
							Vertex_handle			v2)
{
	if(v1->point() == v2->point()) return false;
	typedef	CGAL::Arrangement_zone_2<Pmwx,check_split_zone_visitor>	zone_type;

	check_split_zone_visitor v;
	zone_type		zone(p, &v);
	v.cv = X_monotone_curve_2(Segment_2(v1->point(),v2->point()),0);
	v.v1 = v1;
	v.v2 = v2;

	Vertex_const_handle lv = (v.cv.left() == v1->point()) ? v1 : v2;

	zone.init_with_hint(v.cv,CGAL::make_object(lv));
	zone.compute_zone();
	return !v.has_overlap && !v.has_split && v.has_complete;
}

static bool			can_merge(
							Pmwx&					p,
							Halfedge_handle			he)
{
	DebugAssert(he->target()->degree() == 2);
	Vertex_handle v1 = he->source();
	Vertex_handle v2 = he->next()->target();

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
	typedef	CGAL::Arrangement_zone_2<Pmwx,check_split_zone_visitor>	zone_type;

	check_split_zone_visitor v;
	zone_type		zone(p, &v);
	v.cv = X_monotone_curve_2(Segment_2(v1->point(),v2->point()),0);
	v.v1 = v1;
	v.v2 = v2;

	Vertex_const_handle lv = (v.cv.left() == v1->point()) ? v1 : v2;

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


static Face_handle	face_for_curve(
							Vertex_handle			v,
							Segment_2				s)
{
	typedef	CGAL::Arr_traits_basic_adaptor_2<Traits_2>		Traits_adapter_2;
	Traits_adapter_2	traits;

	X_monotone_curve_2	cv(s);
	Traits_adapter_2::Is_between_cw_2	is_between_cw = traits.is_between_cw_2_object();


	bool	to_left = cv.left() == v->point();
	Pmwx::Halfedge_around_vertex_circulator curr = v->incident_halfedges();
	Pmwx::Halfedge_around_vertex_circulator next = curr;
	Pmwx::Halfedge_around_vertex_circulator first = curr;
	++next;
	bool	eq_curr, eq_next;

	if (curr == next)
		return curr->face();

	while (! is_between_cw (cv, to_left,
						  curr->curve(), (curr->direction() == CGAL::LARGER),
						  next->curve(), (next->direction() == CGAL::LARGER),
						  v->point(), eq_curr, eq_next))
  {
    if (eq_curr || eq_next)
      return Face_handle();

    // Move to the next pair of incident halfedges.
    curr = next++;

    // If we completed a full traversal around v without locating the
    // place for cv, it follows that cv overlaps and existing curve.
    if (curr == first)
      return Face_handle();
  }

	return curr->face();
}

// Polygon Gap coverage
//
// For a given polygon, try to fill in small "leaks" by connecting any external
// points that are visible to each other and smaller than 'dist'.?      =

void	FillPolygonGaps(Polygon_set_2& ioPolygon, double dist)
{
	double dsqr = dist * dist;

	Arrangement_2 pmwx(ioPolygon.arrangement());

	vector<Vertex_handle>	vertices;
	for(Pmwx::Hole_iterator h = pmwx.unbounded_face()->holes_begin(); h != pmwx.unbounded_face()->holes_end(); ++h)
	{
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = *h;
		do {
			vertices.push_back(circ->target());
		}
		while (stop != ++circ);
	}

	typedef multimap<double, pair<Vertex_handle,Vertex_handle > >		seg_queue_t;
	seg_queue_t															seg_queue;

	int ctr=0;
	for(vector<Vertex_handle>::iterator v1 = vertices.begin(); v1 != vertices.end(); ++v1)
	for(vector<Vertex_handle>::iterator v2 = v1; v2 != vertices.end(); ++v2)
	if(v1 != v2)
	{
		Segment_2 	s((*v1)->point(),(*v2)->point());
		double len = CGAL::to_double(s.squared_length());
		if(len < dsqr)
		{
//			CGAL::insert_curve(pmwx,Curve_2(possible_curve));
//			debug_mesh_line(cgal2ben(possible_curve.source()),cgal2ben(possible_curve.target()),1,0,0,  0,1,0);
			seg_queue.insert(seg_queue_t::value_type(len,pair<Vertex_handle, Vertex_handle>(*v1,*v2)));
		}
	}

	for(seg_queue_t::reverse_iterator pseg = seg_queue.rbegin(); pseg != seg_queue.rend(); ++pseg)
	{
		Segment_2	s(pseg->second.first->point(),pseg->second.second->point());
		if(face_for_curve(pseg->second.first ,s) == pmwx.unbounded_face())
		if(face_for_curve(pseg->second.second,s) == pmwx.unbounded_face())
		if(can_insert(pmwx,pseg->second.first,pseg->second.second))
		{
			++ctr;
//			debug_mesh_line(cgal2ben(s.source()),cgal2ben(s.target()),1,0,0,0,1,0);
//			CGAL::insert_curve(pmwx,Curve_2(s,0));
			pmwx.insert_at_vertices(X_monotone_curve_2(s,0),pseg->second.first,pseg->second.second);

		}
	}


//	printf("%d vertices in polygon, added %d.\n", vertices.size(), ctr);

	for(Pmwx::Face_iterator f = pmwx.faces_begin(); f != pmwx.faces_end(); ++f)
	if(!f->is_unbounded())
		f->set_contained(true);

	for(Pmwx::Edge_iterator eit = pmwx.edges_begin(); eit != pmwx.edges_end(); )
	{
		if(eit->face()->contained() == eit->twin()->face()->contained())
		{
			Pmwx::Edge_iterator k(eit);
			++eit;
			pmwx.remove_edge(k);
		}
		else
		++eit;
	}

	ioPolygon = Polygon_set_2(pmwx);
}

// Make a polygon more convex.  Basically go around the
// edge and if we can safely insert a ray that cuts off the
// concave edge, do so.  Keep doing this until we're stuck.
// Don't insert an edge that adds more than max_area.
// This is N^2, but for small polygons that may be okay.
void	SafeMakeMoreConvex(Polygon_set_2& ioPolygon, double max_area)
{
	Pmwx	pmwx(ioPolygon.arrangement());

	int ctr = 0;
	bool did_work;
	DebugAssert(pmwx.unbounded_face()->holes_begin() != pmwx.unbounded_face()->holes_end());

	do {
		did_work = false;

		Pmwx::Ccb_halfedge_circulator stop, iter, next, third;
		stop = iter = *(pmwx.unbounded_face()->holes_begin());
		do {
			next = iter;
			++next;
			DebugAssert(iter != next);

			third=next;
			++third;
			Triangle_2 tri(iter->source()->point(),iter->target()->point(),next->target()->point());

			if(third->target() != iter->source() && third != iter && CGAL::to_double(tri.area()) < max_area && CGAL::left_turn(iter->source()->point(),iter->target()->point(),next->target()->point()))
			{
//				bool ok = true;
//				for(Pmwx::Vertex_iterator v = pmwx.vertices_begin(); v != pmwx.vertices_end(); ++v)
//				if(v != iter->source())
//				if(v != iter->target())
//				if(v != next->target())
//				{
//					if(tri.bounded_side(v->point()) == CGAL::ON_BOUNDED_SIDE)
//					{
//						ok = false;
//						break;
//					}
//				}
//				if(ok)
				if(can_insert(pmwx,iter->source(),next->target()))

				{
//					debug_mesh_line(cgal2ben(iter->source()->point()),cgal2ben(next->target()->point()),1,0,0,0,1,0);
//					CGAL::insert_curve(pmwx,Curve_2(Segment_2(iter->source()->point(),next->target()->point())));
					pmwx.insert_at_vertices(X_monotone_curve_2(Segment_2(iter->source()->point(),next->target()->point())),iter->source(),next->target());
					++ctr;
					did_work=true;
					break;		// MUST break here - insert line changes topology, which can cause stop and circ to not match.  BAd!
				}
			}
			++iter;
		} while (iter != stop);
	} while (did_work);

	for(Pmwx::Face_iterator f = pmwx.faces_begin(); f != pmwx.faces_end(); ++f)
	if(!f->is_unbounded())
		f->set_contained(true);

//	printf("Inserted %d.\n", ctr);
	ioPolygon = Polygon_set_2(pmwx);
}

static double calc_err(Halfedge_handle h, float max_err)
{
	Segment_2	seg(h->source()->point(),h->next()->target()->point());

	if(seg.source() == seg.target())
		return max_err;

	if(h->next() == h->prev()) return max_err;

	Line_2	nl(seg);

	Point_2	proj(nl.projection(h->target()->point()));

	double err_me = sqrt(CGAL::to_double(CGAL::squared_distance(h->target()->point(),proj)));

	return err_me + h->data().mInset + h->next()->data().mInset;
}

void	SimplifyPolygonMaxMove(Polygon_set_2& ioPolygon, double max_err)
{
	Pmwx pmwx(ioPolygon.arrangement());

	Pmwx::Ccb_halfedge_circulator circ,stop;
	Pmwx::Hole_iterator h;

	pqueue<double, Halfedge_handle>		queue;


	for(h = pmwx.unbounded_face()->holes_begin(); h !=  pmwx.unbounded_face()->holes_end(); ++h)
	{
		circ = stop = *h;
		do {
//			debug_mesh_point(
//						cgal2ben(circ->source()->point()),
//						1,1,1);
			circ->data().mInset = 0;
			circ->next()->data().mInset = 0;

			if(circ->next() != circ && circ->prev() != circ->next())
//			if(CGAL::right_turn(circ->source()->point(), circ->target()->point(),circ->next()->target()->point()))
			{
				double err = calc_err(circ, max_err);
				if (err < max_err)
				{
					circ->next()->data().mInset = 0;
					queue.insert(err, Halfedge_handle(circ));
				}
			}
		} while (stop != ++circ);
	}

	int ctr = 0;
	int total = queue.size();
	int orig = pmwx.number_of_vertices();

	while(!queue.empty())
	{
		double err = queue.front_priority();
		DebugAssert(err < max_err);

		Halfedge_handle me = queue.front_value();
		queue.pop_front();

		Halfedge_handle	next = me->next();

		Halfedge_handle prev = me->prev();
		DebugAssert(prev != me);
		DebugAssert(prev != next);
		DebugAssert(next != me);

		if(queue.count(next))
			queue.erase(next);

		Halfedge_handle old_me(me);

		if(can_merge(pmwx,me))
		{
			double old_err = max(me->data().mInset,me->next()->data().mInset);
			++ctr;
			DebugAssert(pmwx.is_valid());
			me = pmwx.merge_edge(me,next,Curve_2(Segment_2(me->source()->point(),next->target()->point()),0));
			DebugAssert(pmwx.is_valid());
			me->data().mInset = err + old_err;

			DebugAssert(queue.count(me) == 0);

			err = calc_err(me, max_err);
			if(err < max_err)
				queue.insert(err,me);

			err = calc_err(prev, max_err);
			if(me->next() == prev)
			{
				queue.erase(me->next());
				queue.erase(prev);
			}
			else
			{
				if(err < max_err)	queue.insert(err, prev);
				else				queue.erase(prev);
			}
		}
	}

	for(Pmwx::Face_iterator f = pmwx.faces_begin(); f != pmwx.faces_end(); ++f)
	if(!f->is_unbounded())
		f->set_contained(true);

	ioPolygon = Polygon_set_2(pmwx);

//	printf("Originally: %d.  Max possible remove: %d.  Actual remove: %d.\n", orig, total, ctr);

}

// Make a polygon simpel as follows:
// Insert all edges into an arrangement.  Whatever area is bounded is considered "in" the new polygon.
void MakePolygonSimple(const Polygon_2& inPolygon, vector<Polygon_2>& out_simple_polygons)
{
	Pmwx	pmap;
	vector<Curve_2>	curves;
	curves.reserve(inPolygon.size());
	for(int n = 0; n < inPolygon.size(); ++n)
	if(inPolygon.edge(n).source() != inPolygon.edge(n).target())
		curves.push_back(Curve_2(inPolygon.edge(n),0));
	curves.insert(curves.end(), curves.begin(),curves.end());
	CGAL::insert_curves(pmap, curves.begin(), curves.end());
	Pmwx::Face_iterator f;
	for(f = pmap.faces_begin(); f != pmap.faces_end(); ++f)
		f->set_contained(!f->is_unbounded());

	// Ben says: ugliness: we cannot simply use the output iterator because
	// the polygons set depends on the curve insertions being based on polygon windings.  Since we have 
	// thrown in self-intersecting curves, some of the curve directions will be whacked out.
	// So...how to get our curves.  CCB of the holes in the unbounded face is not a simple polygon in the
	// figure-8 case.  Instead, iterate and export all faces.  The fact that we have filled freaking 
	// everything is what assures that bounded face outer ccb's are simple.

	Polygon_set_2	pset(pmap);

/*
	Can't do this!  See note above.
	
	vector<Polygon_with_holes_2>	all;
	pset.polygons_with_holes(back_inserter(all));
	
	for(vector<Polygon_with_holes_2>::iterator a = all.begin(); a != all.end(); ++a)
	{
		out_simple_polygons.push_back(a->outer_boundary());
		DebugAssert(out_simple_polygons.back().is_simple());
		DebugAssert(a->holes_begin() == a->holes_end());
	}
*/	
	for(f = pset.arrangement().faces_begin(); f != pset.arrangement().faces_end(); ++f)
	if(f->contained())
	{
		out_simple_polygons.push_back(Polygon_2());
		Pmwx::Ccb_halfedge_circulator circ = f->outer_ccb(), stop = f->outer_ccb();
		do {
			out_simple_polygons.back().push_back(circ->target()->point());
		} while (++circ != stop);

		DebugAssert(out_simple_polygons.back().is_simple());
	}
}
