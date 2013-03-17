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

#endif /* MapHelpers_H */
