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

void	FindAdjacentWetFaces(Face_handle inFace, set<Face_handle>& outFaces)
{
	outFaces.clear();
	set<Halfedge_handle> e;
	FindEdgesForFace<Pmwx>(inFace, e);
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
