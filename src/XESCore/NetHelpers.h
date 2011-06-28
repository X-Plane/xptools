/* 
 * Copyright (c) 2010, Laminar Research.
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

#ifndef NetHelpers_H
#define NetHelpers_H

/*

	NetHelpers - THEORY OF OPERATION
	
	These routines help treat the planar map like a connected road network.  This means that they do a certain amount of analysis on what would
	be otherwise trivial operations (e.g. "Find the next road segment") because the routines have to (1) skip through non-road planar map 
	segments and (2) treat roads at differing levels as not intersecting.

	NOTE: by convention unless otherwise noted, a road is noted by the halfedge that goes in the _same_ direction as the underlying road.
	(For two-way roads, the road will still be stored on one half-edge or the other.)
	
	Contiguous strand direction: contiguous strands are defined by the topology of the road grid: if you treat the road grid as intersecting
	only at a level, strands are defined by the longest runs before you hit a level of degree != 3.  Thus strands are not actually concerned
	with the direction of traffic!
	
	But: since the strand contains no exits or entries on its interior, if the strand has a section where the traffic direction changes (and
	the road type is not two-way) then the strand contains a "crash" and traffic won't function.  In this case we can at least fix the
	strand itself by making all segments point in the same direction.  Note that we don't necessarily know what that right direction is, but
	we can sometimes guess by optimizing the junctions at our ends.  (E.g. if we are a ramp between two highways and the highways have good
	topology and good direction, the on-ramp will have a much better junction score in the right direction than the wrong one.)
	
	Also note that the strand has 'direction' even if the road types are technically two-way...the two-way case is one where (1) it'd be
	hard to see the error in X-Plane and (2) we don't _have_ to fix things.

*/

#include "MapDefs.h"
#include "NetTables.h"
#include "MathUtils.h"

// These routines can accept either a halfedge OR its twin.

// Are there any roads on either this halfedge or its twin?
inline bool he_has_any_roads(Pmwx::Halfedge_handle he);
// Return the halfedge that has the roads.
inline Pmwx::Halfedge_handle get_he_with_roads(Pmwx::Halfedge_handle he);
// Setting the height of a road in terms of an explicit vertex.
inline double get_he_level_at(Pmwx::Halfedge_handle he, Pmwx::Vertex_handle v);
inline double set_he_level_at(Pmwx::Halfedge_handle he, Pmwx::Vertex_handle v, double h);
// Approximate normalized lat-lon vector direction of the road on the edge.
inline Vector2 get_he_road_dir(Pmwx::Halfedge_handle he);
// Approximate dot product of two roads.
inline double get_he_road_dot(Pmwx::Halfedge_handle h1,Pmwx::Halfedge_handle h2);
// Actual rep type
inline int get_he_rep_type(Pmwx::Halfedge_handle he);
inline void set_he_rep_type(Pmwx::Halfedge_handle he, int t);
// Use category for road (e.g. use_Limited)
inline int get_he_road_use(Pmwx::Halfedge_handle he);
// Are we a highway or ramp?
inline int get_he_limited_access(Pmwx::Halfedge_handle he);
// Approximate length in degrees lat/lon.
inline double approx_he_len(Pmwx::Halfedge_handle he);

// A few predicates:
inline bool matches_any(Pmwx::Halfedge_handle he1, Pmwx::Halfedge_handle he2) { return true; }
inline bool matches_rep_type(Pmwx::Halfedge_handle he1, Pmwx::Halfedge_handle he2);
inline bool matches_use(Pmwx::Halfedge_handle he1, Pmwx::Halfedge_handle he2);
inline bool matches_limited_access(Pmwx::Halfedge_handle he1, Pmwx::Halfedge_handle he2);

// Typically we store a "level" of a junction as a vector of half-edges in clock-wise rotation, where the half-edge must be the one that supports
// the directed road.  We store the entire junction as a map of integer level numbers to vector levels.  This is a "levelized" junction.

// Decompose a junction in to levels
inline int levelize_junction(Pmwx::Vertex_handle v, map<int,vector<Pmwx::Halfedge_handle> >& out_junc);
// Is this a highway level (e.g. only highways and ramps?)
inline bool	is_level_highway     (const vector<Pmwx::Halfedge_handle>& l);
// Does this level contain highways and non-highways mixed together?
inline bool	is_level_mixed     (const vector<Pmwx::Halfedge_handle>& l);
// Will this level be graded (if on the ground)?
inline bool is_level_graded		(const vector<Pmwx::Halfedge_handle>& l);
// If this is a highway, is it a valid Y split or merge?
inline bool	is_level_highway_y   (Pmwx::Vertex_handle j, const vector<Pmwx::Halfedge_handle>& l);

// These routines work on a "contiguous strand."  A contiguous strand is a list of half-edges such that:
// - They all point in the same direction (e.g. the target of one is the osurce source of the next) and
// - They all have roads and
// - They share a common level at each junction and
// - At each junction, no OTHER roads share that level.
// In other words, the contiguous strand is a section of road with no real interchanges.  The stand
// _may_ have changes in its feature or rep road types (E.g. it's not all uniform road like in X-Plane.)

typedef bool (* match_pred_f)(Pmwx::Halfedge_handle h1, Pmwx::Halfedge_handle h2);

// These collect the next/previous in the strand or a null handle if none.
inline Pmwx::Halfedge_handle next_contig(Pmwx::Halfedge_handle me);
inline Pmwx::Halfedge_handle prev_contig(Pmwx::Halfedge_handle me);
// This collects the entire strand that root is within.  The strand will be in the direction of root.
// The strand will have at least one halfedge (root).  Error if root has no roads!
template<match_pred_f P>
inline double collect_contig(Pmwx::Halfedge_handle root, list<Pmwx::Halfedge_handle>& flow);
// This changes the direction of all roads on the strand to go "forward" (with the direction of the 
// halfedges selected) or "reversed".
inline void set_forward(list<Pmwx::Halfedge_handle>& flow);
inline void set_reverse(list<Pmwx::Halfedge_handle>& flow);	










inline bool he_has_any_roads(Pmwx::Halfedge_handle he)
{
	return he->data().HasRoads() || he->twin()->data().HasRoads();	
}

inline Pmwx::Halfedge_handle get_he_with_roads(Pmwx::Halfedge_handle he)
{
	if(he->data().mSegments.empty())
		return he->twin();
	else
		return he;
}

inline double get_he_level_at(Pmwx::Halfedge_handle he, Pmwx::Vertex_handle v)
{
	DebugAssert(he->data().mSegments.size() + he->twin()->data().mSegments.size() >= 1);
	DebugAssert(he->source() == v || he->target() == v);
	if(!he->data().mSegments.empty())
	{
		if(he->source() == v)	return he->data().mSegments.back().mSourceHeight;
		else					return he->data().mSegments.back().mTargetHeight;
	}
	else
	{
		if(he->source() == v)	return he->twin()->data().mSegments.back().mTargetHeight;
		else					return he->twin()->data().mSegments.back().mSourceHeight;
	}
}

inline double set_he_level_at(Pmwx::Halfedge_handle he, Pmwx::Vertex_handle v, double h)
{
	DebugAssert(he->data().mSegments.size() + he->twin()->data().mSegments.size() == 1);
	DebugAssert(he->source() == v || he->target() == v);
	if(!he->data().mSegments.empty())
	{
		if(he->source() == v)	he->data().mSegments.back().mSourceHeight=h;
		else					he->data().mSegments.back().mTargetHeight=h;
	}
	else
	{
		if(he->source() == v)	he->twin()->data().mSegments.back().mTargetHeight=h;
		else					he->twin()->data().mSegments.back().mSourceHeight=h;
	}
}

inline Vector2 get_he_road_dir(Pmwx::Halfedge_handle he)
{
	Vector2 v;
	DebugAssert(he->data().mSegments.size() + he->twin()->data().mSegments.size() >= 1);
	if(!he->data().mSegments.empty())
		v = Vector2(cgal2ben(he->source()->point()),cgal2ben(he->target()->point()));
	else
		v = Vector2(cgal2ben(he->target()->point()),cgal2ben(he->source()->point()));
	v.normalize();
	return v;
}

inline double get_he_road_dot(Pmwx::Halfedge_handle h1,Pmwx::Halfedge_handle h2)
{
	Vector2 v1(get_he_road_dir(h1));
	Vector2 v2(get_he_road_dir(h2));
	return doblim(v1.dot(v2),-1.0,1.0);
}

inline int get_he_rep_type(Pmwx::Halfedge_handle he)
{
	int rep = NO_VALUE;
	DebugAssert(he->data().mSegments.size() + he->twin()->data().mSegments.size() >= 1);
	if(!he->data().mSegments.empty())
		rep = he->data().mSegments.back().mRepType;
	else
		rep = he->twin()->data().mSegments.back().mRepType;
	return rep;
}

inline void set_he_rep_type(Pmwx::Halfedge_handle he, int t)
{
	DebugAssert(he->data().mSegments.size() + he->twin()->data().mSegments.size() >= 1);
	if(!he->data().mSegments.empty())
		he->data().mSegments.back().mRepType = t;
	else
		he->twin()->data().mSegments.back().mRepType = t;
}

inline int get_he_road_use(Pmwx::Halfedge_handle he)
{
	int rep = get_he_rep_type(he);
	if (gNetReps.count(rep) == 0) return use_None;
	return gNetReps[rep].use_mode;
}

inline int get_he_limited_access(Pmwx::Halfedge_handle he)
{
	int use = get_he_road_use(he);
	return use == use_Limited || use == use_Ramp;
}

inline bool	is_level_highway     (const vector<Pmwx::Halfedge_handle>& l)
{
	for(vector<Pmwx::Halfedge_handle>::const_iterator r = l.begin(); r != l.end(); ++r)
	{
		int use = get_he_road_use(*r);
		if(use == use_Limited) return true;			// ANY highway?  Limited access!
		if(use != use_Ramp) return false;			// Non-ramp...must be a city street.
	}
	return true;		// Got here, was all ramps, treat as highway
}

inline bool	is_level_mixed     (const vector<Pmwx::Halfedge_handle>& l)
{
	bool h = false;
	bool s = false;
	for(vector<Pmwx::Halfedge_handle>::const_iterator r = l.begin(); r != l.end(); ++r)
	{
		int use = get_he_road_use(*r);
		if(use == use_Limited) h = true;
		if(use == use_Street) s = true;
	}
	return s && h;
}

inline bool is_level_graded		(const vector<Pmwx::Halfedge_handle>& l)
{
	bool m = false;
	bool s = false;
	for(vector<Pmwx::Halfedge_handle>::const_iterator r = l.begin(); r != l.end(); ++r)
	{
		int use = get_he_road_use(*r);
		if(use == use_Limited) return true;			// ANY highway?  Limited access!
		if(use == use_Ramp) m = true;
		if(use == use_Street) s = true;
	}
	return m && !s;
}


inline bool	is_level_highway_y   (Pmwx::Vertex_handle j, const vector<Pmwx::Halfedge_handle>& l)
{
	if(l.size() != 3) return false;
	int in_count = 0;
	int out_count = 0;
	for(int i = 0; i < 3; ++i)
	{
		if(l[i]->target() == j)	++in_count;
		if(l[i]->source() == j)	++out_count;
		for(int j = i+1; j < 3; ++j)
		if(get_he_road_dot(l[i],l[j]) < 0.5)	// Ben says: fairly relaxed concept of kink ... sometimes data thinning + lat/lon skew gives us borderline cases!
			return false;
	}
	return in_count == 1 || out_count == 1;
}

inline Pmwx::Halfedge_handle next_contig(Pmwx::Halfedge_handle me)
{	
	Pmwx::Halfedge_around_vertex_circulator circ, stop;
	circ=stop=me->target()->incident_halfedges();

	double my_level = get_he_level_at(me, me->target());
	
	Pmwx::Halfedge_handle choice;
	do {
		if(circ != me && he_has_any_roads(circ) && get_he_level_at(circ, me->target()) == my_level)
		{
			if(choice == Pmwx::Halfedge_handle())
				choice = circ->twin();
			else
				return Pmwx::Halfedge_handle();
		}
	} while(++circ != stop);
	return choice;
}

inline Pmwx::Halfedge_handle prev_contig(Pmwx::Halfedge_handle me)
{
	Pmwx::Halfedge_around_vertex_circulator circ, stop;
	circ=stop=me->source()->incident_halfedges();

	double my_level = get_he_level_at(me, me->source());

	Pmwx::Halfedge_handle choice;
	do {
		if(circ->twin() != me && he_has_any_roads(circ) && get_he_level_at(circ,me->source()) == my_level)
		{
			if(choice == Pmwx::Halfedge_handle())
				choice = circ;
			else
				return Pmwx::Halfedge_handle();
		}
	} while(++circ != stop);
	return choice;	
}



inline double approx_he_len(Pmwx::Halfedge_handle he)
{
	return sqrt(cgal2ben(he->source()->point()).squared_distance(cgal2ben(he->target()->point())));
}

inline bool matches_rep_type(Pmwx::Halfedge_handle he1, Pmwx::Halfedge_handle he2)
{
	return get_he_rep_type(he1) == get_he_rep_type(he2);
}

inline bool matches_use(Pmwx::Halfedge_handle he1, Pmwx::Halfedge_handle he2)
{
	return get_he_road_use(he1) == get_he_road_use(he2);
}

inline bool matches_limited_access(Pmwx::Halfedge_handle he1, Pmwx::Halfedge_handle he2)
{
	return get_he_limited_access(he1) == get_he_limited_access(he2);
}


template<match_pred_f P>
inline double collect_contig(Pmwx::Halfedge_handle root, list<Pmwx::Halfedge_handle>& flow)
{
	flow.clear();
	flow.push_back(root);
	Pmwx::Halfedge_handle iter = prev_contig(root);
	double len = approx_he_len(root);
	while(iter != Pmwx::Halfedge_handle() && iter != root)
	{
		if(!P(iter,flow.front()))
			break;
		flow.push_front(iter);
		len += approx_he_len(iter);
		iter = prev_contig(iter);
	}
	iter = next_contig(root);
	while(iter != Pmwx::Halfedge_handle() && iter != root)
	{
		if(!P(iter,flow.back()))
			break;
		flow.push_back(iter);
		len += approx_he_len(iter);
		iter = next_contig(iter);
	}
	return len;
}

inline void set_forward(list<Pmwx::Halfedge_handle>& flow)
{
	for(list<Pmwx::Halfedge_handle>::iterator l = flow.begin(); l != flow.end(); ++l)
	if(!(*l)->twin()->data().mSegments.empty())
	{
		swap((*l)->twin()->data().mSegments.front().mSourceHeight,
			 (*l)->twin()->data().mSegments.front().mTargetHeight);
		(*l)->twin()->data().mSegments.swap((*l)->data().mSegments);
	}
}

inline void set_reverse(list<Pmwx::Halfedge_handle>& flow)
{
	for(list<Pmwx::Halfedge_handle>::iterator l = flow.begin(); l != flow.end(); ++l)
	if(!(*l)->data().mSegments.empty())
	{
		swap((*l)->data().mSegments.front().mSourceHeight,
			 (*l)->data().mSegments.front().mTargetHeight);
		(*l)->twin()->data().mSegments.swap((*l)->data().mSegments);
	}	
}
	
inline int levelize_junction(Pmwx::Vertex_handle v, map<int,vector<Pmwx::Halfedge_handle> >& out_junc)
{
	int ret = 0;
	out_junc.clear();
	if(v->is_isolated()) return 0;
	Pmwx::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = v->incident_halfedges();
	do {
		GISNetworkSegmentVector::iterator r;
		for(r = circ->data().mSegments.begin(); r != circ->data().mSegments.end(); ++r)
		{
			out_junc[(int) r->mTargetHeight].push_back(circ);
			++ret;
		}
		for(r = circ->twin()->data().mSegments.begin(); r != circ->twin()->data().mSegments.end(); ++r)
		{
			out_junc[(int) r->mSourceHeight].push_back(circ->twin());
			++ret;
		}
	} while(++circ != stop);
	return ret;
}


#endif /* NetHelpers_H */
