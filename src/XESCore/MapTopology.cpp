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

#include "MapTopology.h"


/************************************************************************************************
 * FACE SETS AND EDGE SETS
 ************************************************************************************************/

void	FindEdgesForFace(Face_handle face, set<Halfedge_handle> &outEdges)
{
	Pmwx::Ccb_halfedge_circulator	circ, stop;
	if (!face->is_unbounded())
	{
		circ = stop = face->outer_ccb();
		do {
			outEdges.insert(circ);
			++circ;
		} while (circ != stop);
	}

	for (Pmwx::Hole_iterator hole = face->holes_begin();
		hole != face->holes_end(); ++hole)
	{
		circ = stop = *hole;
		do {
			outEdges.insert(circ);
			++circ;
		} while (circ != stop);
	}

}

void	FindFacesForEdgeSet(const set<Halfedge_handle>& inEdges, set<Face_handle>& outFaces)
{
	if (inEdges.empty()) return;
	outFaces.clear();

	set<Face_handle>	working;

	// Ben sez: the basic idea here is a "flood fill".  We keep adding more and more faces.
	// Each time we add a new face, we cross all edges that aren't in our hard boundary
	// (inEdges) and add that face too if we haven't seen it yet.
	//
	// But...there is no requirement that all faces surrounded by inEdges be contiguous!
	// So make sure to add ALL of the faces adjacent to inEdges immediately...otherwise
	// we might not flood-fill from one disjoint area to another.

	for (set<Halfedge_handle>::const_iterator e = inEdges.begin(); e != inEdges.end(); ++e)
		working.insert((*e)->face());

	while (!working.empty())
	{
		Face_handle	who = *working.begin();
		DebugAssert(!who->is_unbounded());
		outFaces.insert(who);
		working.erase(who);

		set<Halfedge_handle> edges;
		FindEdgesForFace(who, edges);

		for (set<Halfedge_handle>::iterator edge = edges.begin(); edge != edges.end(); ++edge)
		{
			if (inEdges.count(*edge) == 0)
			{
				Face_handle neighbor = (*edge)->twin()->face();
				if (!neighbor->is_unbounded())
				if (outFaces.count(neighbor) == 0)
					working.insert(neighbor);
			}
		}
	}
}

void	FindInternalEdgesForEdgeSet(const set<Halfedge_handle>& inEdges, set<Halfedge_handle>& outEdges)
{
	outEdges.clear();
	set<Face_handle>	to_visit;
	set<Face_handle>	visited;

	to_visit.insert((*inEdges.begin())->face());

	while(!to_visit.empty())
	{
		Face_handle who = *to_visit.begin();
		DebugAssert(!who->is_unbounded());
		to_visit.erase(who);
		visited.insert(who);

		set<Halfedge_handle>	who_edges;

		FindEdgesForFace(who,who_edges);

		for(set<Halfedge_handle>::iterator edge = who_edges.begin(); edge != who_edges.end(); ++edge)
		if(inEdges.count(*edge) == 0)
		{
			// For every half-edge of this face that is not in the outer boundary...

			// Figure out which face we need to "flood fill" over to.
			Face_handle neighbor = (*edge)->twin()->face();
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


void	FindEdgesForFaceSet(const set<Face_handle>& inFaces, set<Halfedge_handle>& outEdges)
{
	outEdges.clear();
	for (set<Face_handle>::const_iterator f = inFaces.begin(); f != inFaces.end(); ++f)
	{
		set<Halfedge_handle>	local;
		FindEdgesForFace(*f, local);
		for (set<Halfedge_handle>::iterator l = local.begin(); l != local.end(); ++l)
		{
			if (inFaces.count((*l)->twin()->face()) == 0)
				outEdges.insert(*l);
		}
	}
}

void	FindAdjacentFaces(Face_handle inFace, set<Face_handle>& outFaces)
{
	outFaces.clear();
	set<Halfedge_handle> e;
	FindEdgesForFace(inFace, e);
	for (set<Halfedge_handle>::iterator he = e.begin(); he != e.end(); ++he)
	if ((*he)->twin()->face() != inFace)
		outFaces.insert((*he)->twin()->face());
}

void	FindAdjacentWetFaces(Face_handle inFace, set<Face_handle>& outFaces)
{
	outFaces.clear();
	set<Halfedge_handle> e;
	FindEdgesForFace(inFace, e);
	for (set<Halfedge_handle>::iterator he = e.begin(); he != e.end(); ++he)
	if ((*he)->twin()->face() != inFace)
	if ((*he)->twin()->face()->data().IsWater())
		outFaces.insert((*he)->twin()->face());
}

bool		IsAdjacentWater(Face_const_handle in_face, bool unbounded_is_wet)
{
	DebugAssert(!in_face->is_unbounded());
	Pmwx::Ccb_halfedge_const_circulator circ, stop;
	circ = stop = in_face->outer_ccb();
	do {
		if(unbounded_is_wet || !circ->twin()->face()->is_unbounded())
		if(circ->twin()->face()->data().IsWater())
			return true;
	} while (++circ != stop);
	for(Pmwx::Hole_const_iterator h = in_face->holes_begin(); h != in_face->holes_end(); ++h)
	{
		circ = stop = *h;
		do {
			if(unbounded_is_wet || !circ->twin()->face()->is_unbounded())
			if(circ->twin()->face()->data().IsWater())
				return true;
		} while (++circ != stop);
	}
	return false;
}



void	FindConnectedWetFaces(Face_handle inFace, set<Face_handle>& outFaces)
{
	DebugAssert(inFace->data().IsWater());
	outFaces.clear();
	set<Face_handle>	working;
	working.insert(inFace);

	while (!working.empty())
	{
		Face_handle cur = *working.begin();
		working.erase(cur);
		outFaces.insert(cur);
		set<Face_handle> adjacent;
		FindAdjacentWetFaces(cur, adjacent);
		for (set<Face_handle>::iterator f = adjacent.begin(); f != adjacent.end(); ++f)
		if (!(*f)->is_unbounded())
		if (outFaces.count(*f) == 0)
			working.insert(*f);
	}
}


// Clean face - hrm - this is still written using topological delete - yuck!
void	CleanFace(
			Pmwx&					inMap,
			Pmwx::Face_handle		inFace)
{
// OPTIMIZE: this was written before we had face swapping and is done slowly using edge removals.
// Probably better would be:
//
// Pmwx	dump;
// SwapFace(inMap, dump, inFace, NULL);
//
// But we need to consider who "owns" the edges!
//
	set<Halfedge_handle> nuke;
	Assert(!inFace->is_unbounded());

	Pmwx::Ccb_halfedge_circulator stop, iter;
	stop = iter = inFace->outer_ccb();
	do {
		if (iter->face() == iter->twin()->face())
		if(nuke.count(iter->twin())==0)
			nuke.insert(iter);
		++iter;
	} while (iter != stop);

	for (set<Halfedge_handle>::iterator kill = nuke.begin(); kill != nuke.end(); ++kill)
	{
		inMap.remove_edge(*kill);
	}

	while (inFace->holes_begin() != inFace->holes_end())
	{
		inMap.remove_edge(*inFace->holes_begin());
	}
}



int RemoveUnboundedWater(Pmwx& ioMap)
{
// OPTIMIZE: see above comments about nuke-lists.  Also that while loop needes some examination!
	vector<Pmwx::Halfedge_handle>	deadList;

	int nuke = 0;

	for (Pmwx::Edge_iterator he = ioMap.edges_begin(); he != ioMap.edges_end(); ++he, ++he)
	{
		Pmwx::Halfedge_handle h = he;

		bool	iWet = h->face()->data().IsWater();
		bool	oWet = h->twin()->face()->data().IsWater();
		bool	road = !h->data().mSegments.empty() ||
					   !h->twin()->data().mSegments.empty();

		if (!road && iWet && oWet)
			deadList.push_back(he);
	}

	for (vector<Pmwx::Halfedge_handle>::iterator iter = deadList.begin(); iter != deadList.end(); ++iter)
	{
		ioMap.remove_edge(*iter);
		++nuke;
	}

	while (1)
	{
		deadList.clear();
		for (Pmwx::Halfedge_iterator he = ioMap.halfedges_begin(); he != ioMap.halfedges_end(); ++he, ++he)
		if (he->face()->is_unbounded() && he->twin()->face()->is_unbounded())
			deadList.push_back(he);
		if (deadList.empty()) break;

		for (vector<Pmwx::Halfedge_handle>::iterator iter = deadList.begin(); iter != deadList.end(); ++iter)
		{
			ioMap.remove_edge(*iter);
			++nuke;
		}
	}


	return nuke;
}


void ReduceToWaterBodies(Pmwx& ioMap)
{
// OPTIMIZE: it would be possible to utilize the inherent 'creation order' in the Pmwx
// to store the edges-to-die instead of a vector.  However this is probably NOT the major
// bottleneck in this routine.
	vector<Pmwx::Halfedge_handle>	deadList;

	for (Pmwx::Halfedge_iterator he = ioMap.halfedges_begin();
		he != ioMap.halfedges_end(); ++he, ++he)
	{
		bool iWet = he->face()->data().IsWater();
		bool oWet = he->twin()->face()->data().IsWater();
		if (iWet && oWet)
			deadList.push_back(he);
	}

	int i = 0;
	for (vector<Pmwx::Halfedge_handle>::iterator iter = deadList.begin();
		iter != deadList.end(); ++iter, ++i)
	{
		ioMap.remove_edge(*iter);
	}
}


int SimplifyMap(Pmwx& ioMap, bool inKillRivers, ProgressFunc func)
{
//	return 0;

// OPTIMIZE: it would be possible to utilize the inherent 'creation order' in the Pmwx
// to store the edges-to-die instead of a vector.  However this is probably NOT the major
// bottleneck in this routine.

// TODO: it would be nice to pass in a functor to evaluate edges.

	PROGRESS_START(func, 0, 1, "Finding unneeded half-edges...")
	int ctr = 0;
	int dead = 0;
	int tot = ioMap.number_of_halfedges();

	for (Pmwx::Edge_iterator he = ioMap.edges_begin();
		he != ioMap.edges_end(); ctr += 2)
	{
		PROGRESS_CHECK(func,0,1,"Finding unneeded half-edges...",ctr,tot,1000);
		Pmwx::Halfedge_handle h = he;

		bool	iWet = h->face()->data().IsWater();
		bool	oWet = h->twin()->face()->data().IsWater();
		bool	coastline = iWet != oWet;
		bool	border = h->face()->is_unbounded() != h->twin()->face()->is_unbounded();
		bool	lu_change = h->face()->data().mTerrainType != h->twin()->face()->data().mTerrainType;
		bool	road = !h->data().mSegments.empty() || !h->twin()->data().mSegments.empty();
		bool	stuff = h->face()->data().mAreaFeature.mFeatType != h->twin()->face()->data().mAreaFeature.mFeatType ||
						(h->face()->data().mAreaFeature.mFeatType != NO_VALUE &&
						h->face()->data().mAreaFeature.mParams != h->twin()->face()->data().mAreaFeature.mParams);
		bool	river = h->data().mParams.count(he_IsRiver) != 0 || h->twin()->data().mParams.count(he_IsRiver) != 0;
		if (river && (iWet || oWet)) river = false;	// Wipe out rivers that are inside water bodies or coastlines too.
		if (inKillRivers) river = false;
		bool must_burn = h->data().mParams.count(he_MustBurn) || h->twin()->data().mParams.count(he_MustBurn);

		if (!river && !stuff && !road && !coastline && !border && !lu_change && !must_burn)
		{
			Halfedge_handle k = he;
			++he;
			ioMap.remove_edge(k);
			dead += 2;
		}
		else
			++he;
	}
	PROGRESS_DONE(func, 0, 1, "Finding unneeded half-edges...");

	return dead;
}
