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

#if 0
struct FaceIsWet { bool operator()(const Face_handle f) const { return f->data().IsWater(); } };

void	FindAdjacentWetFaces(Face_handle inFace, set<Face_handle>& outFaces)
{
	outFaces.clear();
	typedef CollectionVisitor<Pmwx,Face_handle, FaceIsWet>	Collector;
	Collector	 col(&outFaces);
	VisitAdjacentFaces<Pmwx,Collector>(inFace, col);	
}

#endif
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
#if 0


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
#endif

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

void trim_map(Pmwx& ioMap)
{
	for(Pmwx::Vertex_handle v = ioMap.vertices_begin(); v != ioMap.vertices_end(); ++v)
		v->data().trim();
	for(Pmwx::Halfedge_handle e = ioMap.halfedges_begin(); e != ioMap.halfedges_end(); ++e)
		e->data().trim();
	for(Pmwx::Face_handle f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
		f->data().trim();
}


int SimplifyMap(Pmwx& ioMap, bool inKillRivers, ProgressFunc func)
{
//	return 0;

// OPTIMIZE: it would be possible to utilize the inherent 'creation order' in the Pmwx
// to store the edges-to-die instead of a vector.  However this is probably NOT the major
// bottleneck in this routine.

// TODO: it would be nice to pass in a functor to evaluate edges.

	PROGRESS_START(func, 0, 2, "Finding unneeded half-edges...")
	int ctr = 0;
	int dead = 0;
	int tot = ioMap.number_of_halfedges();
	int check = tot / 100;

	for (Pmwx::Edge_iterator he = ioMap.edges_begin();
		he != ioMap.edges_end(); ctr += 2)
	{
		PROGRESS_CHECK(func,0,2,"Finding unneeded half-edges...",ctr,tot,check);
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
	PROGRESS_DONE(func, 0, 2, "Finding unneeded half-edges...");

	ctr = 0;
	tot = ioMap.number_of_vertices();
	check = tot / 100;
	PROGRESS_START(func,1,2, "Eliminating cuts in straight segments...");
	for(Pmwx::Vertex_iterator v = ioMap.vertices_begin(); v != ioMap.vertices_end(); ++ctr)
	{
		Pmwx::Vertex_handle k(v);
		++v;
		PROGRESS_CHECK(func,1,2, "Eliminating cuts in straight segments...",ctr,tot,check);		
		if(k->degree() == 2)
		{
			Halfedge_handle e1 = k->incident_halfedges();
			Halfedge_handle e2 = e1->next();
			DebugAssert(e1->target() == k);
			DebugAssert(e2->source() == k);
			if(e1->data() == e2->data() && CGAL::collinear(e1->source()->point(),e1->target()->point(),e2->target()->point()))
			{
				Curve_2	nc(Segment_2(e1->source()->point(),e2->target()->point()));
				ioMap.merge_edge(e1,e2,nc);
				++dead;			
			}	
		}
	}
	PROGRESS_DONE(func,1,2, "Eliminating cuts in straight segments...");

	return dead;
}
