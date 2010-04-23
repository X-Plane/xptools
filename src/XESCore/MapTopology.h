/*
 * Copyright (c) 2008, Laminar Research.
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

#ifndef MapTopology_H
#define MapTopology_H

#include "MapDefs.h"
#include "ProgressUtils.h"

/************************************************************************************************
 * FACE SETS AND EDGE SETS
 ************************************************************************************************
 *
 * It is often useful to treat a group of faces or a group of edges as a unit...two examples:
 *
 * - A body of water might be made up of multiple faces becaue bridges split the body of water.
 *   So we use a face set to represent the entire body of water.
 *
 * - When merging or copying from one map to another, a single face or single edge in one map
 *   may be represented by many edges or faces in the new one!
 *
 * Generally when we have an edge set we mean a set of halfedges that form one or more rings.
 * (But the edge set is not ordered!)  When we have a face set, the area of the faces is not
 * necessarily continuous and may contain holes, islands, or whatever!
 *
 */

/*
 * FindEdgesForFace
 *
 * Given a face, return all halfedges that have the face on its left.
 * THIS DOES NOT CLEAR THE SET, FOR YOUR CONVENIENCE!!
 *
 */
template <typename Arr>
void	FindEdgesForFace(typename Arr::Face_handle face, set<typename Arr::Halfedge_handle>& outEdges);

/*
 * FindFacesForEdgeSet
 *
 * Given a bounded edge set, return all faces within that edge set.
 * THIS DOES CLEAR THE FACE SET!
 *
 */
template <typename Arr>
void	FindFacesForEdgeSet(const set<typename Arr::Halfedge_handle>& inEdges, set<typename Arr::Face_handle>& outFaces);

/*
 * FindEdgesForFaceSet
 *
 * Given a set of faces, finds the halfedges that bound the set of faces.  Halfedges facing the
 * inside of the face set are returned.
 *
 */
template <typename Arr>
void	FindEdgesForFaceSet(const set<typename Arr::Face_handle>& inFaces, set<typename Arr::Halfedge_handle>& outEdges);

/*
 * FindInternalEdgesForFaceSet
 *
 * Given a set of edges that rings an area, return any edges inside.  Edges returned are guaranteed to not
 * be twins, but no guarantee about which edge we get!
 *
 */
template <typename Arr>
void	FindInternalEdgesForEdgeSet(const set<typename Arr::Halfedge_handle>& inEdges, set<typename Arr::Halfedge_handle>& outEdges);

/*
 * FindAdjacentFaces
 *
 * Given a face, returns all faces that are touching this face.
 *
 */
template <typename Arr>
void	FindAdjacentFaces(typename Arr::Face_handle inFace, set<typename Arr::Face_handle>& outFaces);

/*
 * FindAdjacentWetFaces
 *
 * Given a face, returns all faces that are touching this face.
 *
 */
void	FindAdjacentWetFaces(Face_handle inFace, set<Face_handle>& outFaces);

/*
 * IsAdjacentWater
 *
 * Return true if adjacent to any water poly.
 *
 */
bool		IsAdjacentWater(Face_const_handle in_face, bool unbounded_is_wet);

/*
 * FindConnectedWetFaces
 *
 * Given a water face, return all connected waterways.  This gives you a truly enclosed water
 * body, taking into account things like bridges.
 *
 */
void	FindConnectedWetFaces(Face_handle inFace, set<Face_handle>& outFaces);

/*
 * CleanFace
 *
 * Given a face on a map, this routine 'cleans' its interior, removing any
 * antennas, holes, or anything else in its interior.
 *
 * (This is a convenient high level swap.)
 *
 */
void	CleanFace(
			Pmwx&				inMap,
			Face_handle	inFace);

/*
 * RemoveUnboundedWater
 *
 * Similar to the routines above, this routine removes all vectors with water on both sides
 * and no transportation links.  The result is (among other things) a removal of unbounded water
 * from a map.  This can be useful for reducing a map to dry land and embedded lakes.
 *
 * This also removes antennas into the unbounded face.
 *
 * Performance: O(N*M) where N = number of removed halfedges, and M = average number
 * of halfedges in a CCB.
 *
 */
int RemoveUnboundedWater(Pmwx& ioMap);

/*
 * ReduceToWaterBodies
 *
 * This routine removes edges until the map consists only of faces that represent contiguous
 * water bodies, and the infinite face.  Please note that implicit in this is that edges
 * be constructed to form discrete water bodies up to the edge of the map's useful area.
 *
 * Performance: O(N*M) where N = number of removed halfedges, and M = average number
 * of halfedges in a CCB.
 *
 */
void ReduceToWaterBodies(Pmwx& ioMap);

/*
 * SimplifyMap
 *
 * SimplifyMap removes all edges that do not separate distinct land uses, form the outer
 * CCB, or contain rivers/transportation.  This can include lines from unused data, like
 * geopolitical boundaries, or lines that are artifacts of the import.
 *
 * Performance: O(N*M) where N = number of removed halfedges, and M = average number
 * of halfedges in a CCB.
 *
 */
int SimplifyMap(Pmwx& ioMap, bool inKillRivers, ProgressFunc func);



template <typename Properties, typename Arr=Pmwx>
class	MapBFSVisitor {
public:

	typedef pair<typename Arr::Face_handle, Properties>				prop_pair;
	typedef list<prop_pair>											face_queue;

	virtual	void	initialize_properties(Properties& io_properties)=0;
	virtual	void	adjust_properties(typename Arr::Halfedge_handle edge, Properties& io_properties)=0;
	virtual	void	mark_face(const Properties& in_properties, typename Arr::Face_handle face)=0;
	
	void	Visit(Arr * targ)
	{
		face_queue	q;
		targ->unbounded_face()->set_visited(true);
		Properties p;
		initialize_properties(p);
		q.push_back(prop_pair(targ->unbounded_face(), p));			
		bfs_scan(q);		
		for(typename Arr::Face_iterator f = targ->faces_begin(); f != targ->faces_end(); ++f)
			f->set_visited(false);
	}
	
private:

	void bfs_process_ccb(typename Arr::Ccb_halfedge_circulator ccb, const Properties& props, face_queue& q)
	{
		typename Arr::Ccb_halfedge_circulator circ(ccb), stop(ccb);
		do {
			if(!circ->twin()->face()->visited())
			{
				typename Arr::Face_handle f(circ->twin()->face());
				f->set_visited(true);
				Properties	twin_props(props);
				
				adjust_properties(circ, twin_props);
				
				// go through cuve, twiddle twin-props
				q.push_back(prop_pair(f,twin_props));
			}
		} while (++circ != stop);
	}

	void bfs_scan(face_queue& q)
	{
		while(!q.empty())
		{
			Properties	props(q.front().second);
			typename Arr::Face_handle	f(q.front().first);
			q.pop_front();
			
			mark_face(props,f);
			
			if(!f->is_unbounded())
				bfs_process_ccb(f->outer_ccb(), props, q);
			
			for(typename Arr::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
				bfs_process_ccb(*h, props, q);
		}
	}

};

/************************************************************************************************************************
 * TEMPLATE IMPLS
 ************************************************************************************************************************/

template <typename Arr>
inline void	FindEdgesForFace(typename Arr::Face_handle face, set<typename Arr::Halfedge_handle> &outEdges)
{
	typename Arr::Ccb_halfedge_circulator	circ, stop;
	if (!face->is_unbounded())
	{
		circ = stop = face->outer_ccb();
		do {
			outEdges.insert(circ);
			++circ;
		} while (circ != stop);
	}

	for (typename Arr::Hole_iterator hole = face->holes_begin();
		hole != face->holes_end(); ++hole)
	{
		circ = stop = *hole;
		do {
			outEdges.insert(circ);
			++circ;
		} while (circ != stop);
	}

}

template <typename Arr>
inline void	FindFacesForEdgeSet(const set<typename Arr::Halfedge_handle>& inEdges, set<typename Arr::Face_handle>& outFaces)
{
	if (inEdges.empty()) return;
	outFaces.clear();

	set<typename Arr::Face_handle>	working;

	// Ben sez: the basic idea here is a "flood fill".  We keep adding more and more faces.
	// Each time we add a new face, we cross all edges that aren't in our hard boundary
	// (inEdges) and add that face too if we haven't seen it yet.
	//
	// But...there is no requirement that all faces surrounded by inEdges be contiguous!
	// So make sure to add ALL of the faces adjacent to inEdges immediately...otherwise
	// we might not flood-fill from one disjoint area to another.

	for (typename set<typename Arr::Halfedge_handle>::const_iterator e = inEdges.begin(); e != inEdges.end(); ++e)
		working.insert((*e)->face());

	while (!working.empty())
	{
		typename Arr::Face_handle	who = *working.begin();
		DebugAssert(!who->is_unbounded());
		outFaces.insert(who);
		working.erase(who);

		set<typename Arr::Halfedge_handle> edges;
		FindEdgesForFace<Arr>(who, edges);

		for (typename set<typename Arr::Halfedge_handle>::iterator edge = edges.begin(); edge != edges.end(); ++edge)
		{
			if (inEdges.count(*edge) == 0)
			{
				typename Arr::Face_handle neighbor = (*edge)->twin()->face();
				if (!neighbor->is_unbounded())
				if (outFaces.count(neighbor) == 0)
					working.insert(neighbor);
			}
		}
	}
}

template <typename Arr>
inline void	FindInternalEdgesForEdgeSet(const set<typename Arr::Halfedge_handle>& inEdges, set<typename Arr::Halfedge_handle>& outEdges)
{
	outEdges.clear();
	set<typename Arr::Face_handle>	to_visit;
	set<typename Arr::Face_handle>	visited;

	to_visit.insert((*inEdges.begin())->face());

	while(!to_visit.empty())
	{
		typename Arr::Face_handle who = *to_visit.begin();
		DebugAssert(!who->is_unbounded());
		to_visit.erase(who);
		visited.insert(who);

		set<typename Arr::Halfedge_handle>	who_edges;

		FindEdgesForFace<Arr>(who,who_edges);

		for(typename set<typename Arr::Halfedge_handle>::iterator edge = who_edges.begin(); edge != who_edges.end(); ++edge)
		if(inEdges.count(*edge) == 0)
		{
			// For every half-edge of this face that is not in the outer boundary...

			// Figure out which face we need to "flood fill" over to.
			typename Arr::Face_handle neighbor = (*edge)->twin()->face();
			if (!neighbor->is_unbounded())
			if (visited.count(neighbor) == 0)
				to_visit.insert(neighbor);

			// We only want to take each edge once.  Check relative to curve direction...if we are
			// not "with out curve", we get inserted by the other side.
			if(he_is_same_direction(*edge))
				outEdges.insert(*edge);
		}
	}
}

template <typename Arr>
inline void	FindEdgesForFaceSet(const set<typename Arr::Face_handle>& inFaces, set<typename Arr::Halfedge_handle>& outEdges)
{
	outEdges.clear();
	for (typename set<typename Arr::Face_handle>::const_iterator f = inFaces.begin(); f != inFaces.end(); ++f)
	{
		set<typename Arr::Halfedge_handle>	local;
		FindEdgesForFace<Pmwx>(*f, local);
		for (typename set<typename Arr::Halfedge_handle>::iterator l = local.begin(); l != local.end(); ++l)
		{
			if (inFaces.count((*l)->twin()->face()) == 0)
				outEdges.insert(*l);
		}
	}
}

template <typename Arr>
inline void	FindAdjacentFaces(typename Arr::Face_handle inFace, set<typename Arr::Face_handle>& outFaces)
{
	outFaces.clear();
	set<typename Arr::Halfedge_handle> e;
	FindEdgesForFace(inFace, e);
	for (typename set<typename Arr::Halfedge_handle>::iterator he = e.begin(); he != e.end(); ++he)
	if ((*he)->twin()->face() != inFace)
		outFaces.insert((*he)->twin()->face());
}



#endif /* MapTopology_H */
