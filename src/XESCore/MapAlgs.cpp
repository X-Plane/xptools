/* 
 * Copyright (c) 2004, Laminar Research.
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
#include "MapAlgs.h"
#include "ParamDefs.h"
#include "GISUtils.h"
#include <iomanip>
#include "AssertUtils.h"
#include "CompGeomUtils.h"
#include "PolyRasterUtils.h"
#include "DEMDefs.h"

// NOTE: by convention all of the static helper routines and structs have the __ prefix..this is intended 
// for the sole purpose of making it easy to read the function list popup in the IDE...


// Show all ideal insert lines for an inset!
#define DEBUG_SHOW_INSET 0
#define TRACE_TOPO_INTEGRATE 0

#if DEBUG_SHOW_INSET
#include "WED_Globals.h"
#endif

/************************************************************************************************
 * FACE SETS AND EDGE SETS
 ************************************************************************************************/

void	FindEdgesForFace(GISFace * face, set<GISHalfedge *>& outEdges)
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
	
	for (Pmwx::Holes_iterator hole = face->holes_begin();
		hole != face->holes_end(); ++hole)
	{
		circ = stop = *hole;
		do {
			outEdges.insert(circ);
			++circ;
		} while (circ != stop);		
	}
	
}

void	FindFacesForEdgeSet(const set<GISHalfedge *>& inEdges, set<GISFace *>& outFaces)
{
	if (inEdges.empty()) return;
	outFaces.clear();
	
	set<GISFace *>	working;
	
	// Ben sez: the basic idea here is a "flood fill".  We keep adding more and more faces.
	// Each time we add a new face, we cross all edges that aren't in our hard boundary 
	// (inEdges) and add that face too if we haven't seen it yet.
	//
	// But...there is no requirement that all faces surrounded by inEdges be contiguous!
	// So make sure to add ALL of the faces adjacent to inEdges immediately...otherwise
	// we might not flood-fill from one disjoint area to another.
	
	for (set<GISHalfedge *>::const_iterator e = inEdges.begin(); e != inEdges.end(); ++e)	
		working.insert((*e)->face());
	
	while (!working.empty())
	{
		GISFace *	who = *working.begin();
		DebugAssert(!who->is_unbounded());
		outFaces.insert(who);
		working.erase(who);
		
		set<GISHalfedge*> edges;
		FindEdgesForFace(who, edges);
		
		for (set<GISHalfedge*>::iterator edge = edges.begin(); edge != edges.end(); ++edge)
		{
			if (inEdges.count(*edge) == 0)
			{
				GISFace * neighbor = (*edge)->twin()->face();
				if (!neighbor->is_unbounded())
				if (outFaces.count(neighbor) == 0)
					working.insert(neighbor);
			}
		}
	}
}

void	FindEdgesForFaceSet(const set<GISFace *>& inFaces, set<GISHalfedge *>& outEdges)
{
	outEdges.clear();
	for (set<GISFace *>::const_iterator f = inFaces.begin(); f != inFaces.end(); ++f)
	{
		set<GISHalfedge *>	local;
		FindEdgesForFace(*f, local);
		for (set<GISHalfedge *>::iterator l = local.begin(); l != local.end(); ++l)
		{
			if (inFaces.count((*l)->twin()->face()) == 0)
				outEdges.insert(*l);
		}
	}
}

void	FindAdjacentFaces(GISFace * inFace, set<GISFace *>& outFaces)
{
	outFaces.clear();
	set<GISHalfedge *> e;
	FindEdgesForFace(inFace, e);
	for (set<GISHalfedge*>::iterator he = e.begin(); he != e.end(); ++he)
	if ((*he)->twin()->face() != inFace)
		outFaces.insert((*he)->twin()->face());
}

void	FindAdjacentWetFaces(GISFace * inFace, set<GISFace *>& outFaces)
{
	outFaces.clear();
	set<GISHalfedge *> e;
	FindEdgesForFace(inFace, e);
	for (set<GISHalfedge*>::iterator he = e.begin(); he != e.end(); ++he)
	if ((*he)->twin()->face() != inFace)
	if ((*he)->twin()->face()->IsWater())	
		outFaces.insert((*he)->twin()->face());
}


void	FindConnectedWetFaces(GISFace * inFace, set<GISFace *>& outFaces)
{
	DebugAssert(inFace->IsWater());
	outFaces.clear();
	set<GISFace *>	working;
	working.insert(inFace);
	
	while (!working.empty())
	{
		GISFace * cur = *working.begin();
		working.erase(cur);
		outFaces.insert(cur);
		set<GISFace *> adjacent;
		FindAdjacentWetFaces(cur, adjacent);
		for (set<GISFace *>::iterator f = adjacent.begin(); f != adjacent.end(); ++f)
		if (!(*f)->is_unbounded())
		if (outFaces.count(*f) == 0)
			working.insert(*f);		
	}
}

void	CCBToPolygon(const GISHalfedge * ccb, Polygon2& outPolygon, vector<double> * road_types, double (* weight_func)(const GISHalfedge * edge), Bbox2 * outBounds)
{
	if (road_types != NULL) DebugAssert(weight_func != NULL);
	if (road_types == NULL) DebugAssert(weight_func == NULL);
	
	outPolygon.clear();
	if (road_types) road_types->clear();
	
	const GISHalfedge * iter = ccb, * stop = ccb;
	
	if (outBounds)	(*outBounds) = iter->source()->point();
	
	do {
		outPolygon.push_back(iter->source()->point());
		if (outBounds) (*outBounds) += iter->source()->point();
		if (road_types)
			road_types->push_back(weight_func(iter));		
		iter = iter->next();
	} while (iter != stop);
}

void	FaceToComplexPolygon(const GISFace * face, vector<Polygon2>& outPolygon, vector<vector<double> > * road_types, double (* weight_func)(const GISHalfedge * edge), Bbox2 * outBounds)
{
	outPolygon.clear();
	if (road_types)	road_types->clear();
	
	if (!face->is_unbounded())
	{
		outPolygon.push_back(Polygon2());
		if (road_types) road_types->push_back(vector<double>());
		CCBToPolygon(face->outer_ccb(), outPolygon.back(), road_types ? &road_types->back() : NULL, weight_func, outBounds);
	}
	
	for (Pmwx::Holes_const_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)	
	{
		outPolygon.push_back(Polygon2());
		if (road_types) road_types->push_back(vector<double>());
		CCBToPolygon(*hole, outPolygon.back(), road_types ? &road_types->back() : NULL, weight_func, NULL);		
	}
}

GISFace *	ComplexPolygonToPmwx(const vector<Polygon2>& inPolygons, Pmwx& outPmwx, int inTerrain, int outTerrain)
{
	GISFace * outer = NULL;
	outPmwx.clear();
	outPmwx.unbounded_face()->mTerrainType = outTerrain;
	for (vector<Polygon2>::const_iterator poly = inPolygons.begin(); poly != inPolygons.end(); ++poly)
	{
		GISFace * parent = (poly == inPolygons.begin()) ? outPmwx.unbounded_face() : outer;
		if (poly == inPolygons.begin())
		{
			GISFace * new_f = SafeInsertRing(&outPmwx, parent, *poly);
			outer = new_f;
			new_f->mTerrainType = inTerrain;
		
		}
		else
		{
			Polygon2	rev(*poly);
			reverse(rev.begin(), rev.end());
			GISFace * new_f = SafeInsertRing(&outPmwx, parent, rev);
			new_f->mTerrainType = outTerrain;
		}
	}
	return outer;
}



/************************************************************************************************
 * MAP EDITING
 ************************************************************************************************/
#pragma mark -

// This notifier and accompanying struct accumlates all halfedges that are inserted or found
// doing an insert-with intersections as well as all target points in order.
struct	__CropProgInfo_t {
	vector<Point2>			pts;
	vector<GISHalfedge *>	edges;
};

static void	__ProgNotifier(GISHalfedge * o, GISHalfedge * n, void * r)
{
	__CropProgInfo_t * ref = (__CropProgInfo_t *) r;
	GISHalfedge * e = (n == NULL) ? o : n;
	
	if (o == NULL || n == NULL)
	if (o != NULL || n != NULL)
	{
		DebugAssert(e != NULL);
		ref->edges.push_back(e);
		ref->pts.push_back(e->target()->point());
	}
}

// Basic crop map.  Build a ring from our bounds and call advanced crop-map.
void	CropMap(
			Pmwx&			ioMap,
			double			inWest,
			double			inSouth,
			double			inEast,
			double			inNorth,
			bool			inKeepOutside,
			ProgressFunc	inFunc)
{
	Pmwx	cutout;
	
	vector<Point2>	pts;
	pts.push_back(Point2(inWest, inNorth));
	pts.push_back(Point2(inWest, inSouth));
	pts.push_back(Point2(inEast, inSouth));
	pts.push_back(Point2(inEast, inNorth));
	CropMap(ioMap, cutout, pts, inFunc);
	if (!inKeepOutside)
		ioMap.swap(cutout);
}
			
			
// Advanced crop map.  Insert the crop ring into the main map with intersections
// and into the cutout empty map using the fast "insert_ring" primitive.
// Then swap to get the insides into the cutout ring.
void	CropMap(
			Pmwx&					ioMap,
			Pmwx&					ioCutout,
			const vector<Point2>&	ring,			
			ProgressFunc			inFunc)
{
	__CropProgInfo_t	info;
	__CropProgInfo_t * ref = &info;

	for (int n = 0; n < ring.size(); ++n)
	{
		int m = (n+1)%ring.size();
		if (inFunc) inFunc(0,2,"Cutting map...", (float) n / (float) ring.size());
		
		ioMap.insert_edge(ring[n], ring[m], __ProgNotifier, &info);
	}

	if (inFunc) inFunc(0, 2, "Cutting map...", 1.0);
	
	ioCutout.clear();
	GISFace * area;
	area = ioCutout.insert_ring(ioCutout.unbounded_face(), info.pts);
	vector<GISHalfedge *>	inner_ring;
	
	Pmwx::Ccb_halfedge_circulator stop, iter;
	stop = iter = area->outer_ccb();
	do {
		inner_ring.push_back(iter);
		++iter;		
	} while (iter != stop);
	
	SwapMaps(ioMap, ioCutout, info.edges, inner_ring);
#if DEV
	DebugAssert(ioMap.is_valid());
	DebugAssert(ioCutout.is_valid());
#endif	

}

// Utility routine - delete antennas from a face.
static	void	__RemoveAntennasFromFace(Pmwx& inPmwx, GISFace * inFace)
{
// OPTIMIZE: remove second set?
	set<GISHalfedge *>	edges;
	set<GISHalfedge *>	nuke;
	
	FindEdgesForFace(inFace, edges);
	for (set<GISHalfedge *>::iterator i = edges.begin(); i != edges.end(); ++i)
	if ((*i)->mDominant)
	if ((*i)->twin()->face() == inFace)
		nuke.insert(*i);

	for (set<GISHalfedge *>::iterator j = nuke.begin(); j != nuke.end(); ++j)
	{
		inPmwx.remove_edge(*j);
	}
}

// Utility: given a face CCB with no antennas, force the exact edges in
// two maps and record the vectors in order.  This uses a notifier to
// capture a series of inserts-with-intersections.
struct	__InduceCCBInfo_t {
	GISHalfedge *			master;
	vector<GISHalfedge *>	slave_halfedges;
	vector<Point2>			break_pts;
};	

void	__InduceCCBNotifier(GISHalfedge * o, GISHalfedge * n, void * r)
{
	__InduceCCBInfo_t *	ref = (__InduceCCBInfo_t *) r;
	if (o && n)
	{
		
	} else {
		GISHalfedge * e = o ? o : n;
		ref->slave_halfedges.push_back(e);
		if (e->source()->point() != ref->master->source()->point())
			ref->break_pts.push_back(e->source()->point());
	}
}

static	void	__InduceCCB(
					Pmwx& 					inMaster, 
					Pmwx& 					inSlave, 
					GISHalfedge * 			ccb, 
					vector<GISHalfedge *>& 	masterE, 
					vector<GISHalfedge *>& 	slaveE, 
					bool 					outer)
{
	GISHalfedge * iter = ccb;
	GISHalfedge * last_slave = NULL;
	do {
		__InduceCCBInfo_t	info;
		info.master = iter;
		if (last_slave)
			last_slave = inSlave.insert_edge(iter->source()->point(), iter->target()->point(), last_slave, Pmwx::locate_Vertex, __InduceCCBNotifier, &info);
		else
			last_slave = inSlave.insert_edge(iter->source()->point(), iter->target()->point(), __InduceCCBNotifier, &info);

		slaveE.insert(slaveE.end(), info.slave_halfedges.begin(), info.slave_halfedges.end());
		
		masterE.push_back(info.master);
		for (vector<Point2>::iterator pt = info.break_pts.begin(); pt != info.break_pts.end(); ++pt)
		{
			info.master = inMaster.split_edge(info.master, *pt)->next();			
			masterE.push_back(info.master);
			iter = info.master;
		}

		iter = iter->next();
	} while (iter != ccb);
	
	if (!outer)
	{
		vector<GISHalfedge *>	masterR, slaveR;
		vector<GISHalfedge *>::reverse_iterator riter;
		for (riter = masterE.rbegin(); riter != masterE.rend(); ++riter)
			masterR.push_back((*riter)->twin());
		for (riter = slaveE.rbegin(); riter != slaveE.rend(); ++riter)
			slaveR.push_back((*riter)->twin());
		
		masterE.swap(masterR);
		slaveE.swap(slaveR);
	}
}

// Face swap - clean antenna (a requirement) and induce the face into the other map.  Then we can use
// our swap operator.
void	SwapFace(
			Pmwx&			inMaster,
			Pmwx&			inSlave,
			GISFace *		inFace,
			ProgressFunc	inFunc)
{
	__RemoveAntennasFromFace(inMaster, inFace);

	vector<GISHalfedge *>	ringMaster, ringSlave;
	__InduceCCB(inMaster, inSlave, inFace->outer_ccb(), ringMaster, ringSlave, true);
	DebugAssert(ringMaster.size() == ringSlave.size());
	SwapMaps(inMaster, inSlave, ringMaster, ringSlave);
	
	for (Pmwx::Holes_iterator hole = inFace->holes_begin(); hole != inFace->holes_end(); ++hole)
	{
		ringMaster.clear();
		ringSlave.clear();
		__InduceCCB(inSlave, inMaster, *hole, ringMaster, ringSlave, false);
		DebugAssert(ringMaster.size() == ringSlave.size());
		SwapMaps(inSlave, inMaster, ringMaster, ringSlave);		
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
	set<GISHalfedge *> nuke;
	Assert(!inFace->is_unbounded());
	
	Pmwx::Ccb_halfedge_circulator stop, iter;	
	stop = iter = inFace->outer_ccb();
	do {
		if (iter->mDominant)
		if (iter->face() == iter->twin()->face())
			nuke.insert(iter);
		++iter;
	} while (iter != stop);

	for (set<GISHalfedge*>::iterator kill = nuke.begin(); kill != nuke.end(); ++kill)
	{
		inMap.remove_edge(*kill);
	}
	
	while (inFace->holes_begin() != inFace->holes_end())
	{
		inMap.remove_edge(*inFace->holes_begin());
	}
}

// Overlay notification: this notifier tracks insertion into one map and back-induces splits into
// the source map!
struct OverlayInfo_t {
	vector<GISHalfedge *>	dstRing;
	vector<GISHalfedge *>	srcRing;
	GISHalfedge *			curSrc;
	Pmwx *					curSrcMap;
};

static void __OverlayNotifier(GISHalfedge * o, GISHalfedge * n, void * r)
{
	OverlayInfo_t * ref = (OverlayInfo_t *) r;
	if (o != NULL && n != NULL) return;
	GISHalfedge * e = (n == NULL) ? o : n;
	
	if (e->target()->point() == ref->curSrc->target()->point())
	{
		// this is the last edge to be added - easy!
		ref->dstRing.push_back(e);
		ref->srcRing.push_back(ref->curSrc);
	} else {
		// the edge took multiple pieces in the other map.
		ref->curSrc = ref->curSrcMap->split_edge(ref->curSrc, e->target()->point());
		ref->dstRing.push_back(e);
		ref->srcRing.push_back(ref->curSrc);		
		ref->curSrc = ref->curSrc->next();
	}
}

// Overlay - please note that this is NOT a merge, it is an "overwrite"...stuff under "inSrc" inside "inDst" goes away.
// Strategy: for each contiguous blob in inSrc (an area that will overwrite inDst) we induce this ring in the dest map,
// swap and the stuff we overwrite ends up in inSrc.

// TODO: it would be nice to explicitly handle antennas in inSrc's outer islands.
void OverlayMap(
			Pmwx& 	inDst, 
			Pmwx& 	inSrc)
{
	int ctr = 0;
	for (Pmwx::Holes_iterator hole = inSrc.unbounded_face()->holes_begin(); hole != inSrc.unbounded_face()->holes_end(); ++hole, ++ctr)
	{
		OverlayInfo_t	info;
		info.curSrcMap = &inSrc;
		Pmwx::Ccb_halfedge_circulator iter, stop;
		iter = stop = *hole;
		vector<GISHalfedge *>	edges;
		do {
			DebugAssert(iter->twin()->face() != inSrc.unbounded_face());
			edges.push_back(iter->twin());
			++iter;
		} while (iter != stop);


		for (vector<GISHalfedge *>::reverse_iterator riter = edges.rbegin(); riter != edges.rend(); ++riter)
		{
			info.curSrc = *riter;
			inDst.insert_edge((*riter)->source()->point(), (*riter)->target()->point(), __OverlayNotifier, &info);
		}

#if DEV
		DebugAssert(inSrc.is_valid());
		DebugAssert(inDst.is_valid());
#endif

		SwapMaps(inDst, inSrc, info.dstRing, info.srcRing);
	}
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
		bool iWet = he->face()->IsWater();
		bool oWet = he->twin()->face()->IsWater();
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
// OPTIMIZE: it would be possible to utilize the inherent 'creation order' in the Pmwx
// to store the edges-to-die instead of a vector.  However this is probably NOT the major
// bottleneck in this routine.

// TODO: it would be nice to pass in a functor to evaluate edges.
	vector<Pmwx::Halfedge_handle>	deadList;

	PROGRESS_START(func, 0, 2, "Finding unneeded half-edges...")
	int ctr = 0;
	int tot = ioMap.number_of_halfedges();
	
	for (Pmwx::Halfedge_iterator he = ioMap.halfedges_begin();
		he != ioMap.halfedges_end(); ++he, ++he, ++ctr, ++ctr)
	{
		PROGRESS_CHECK(func,0,2,"Finding unneeded half-edges...",ctr,tot,1000);
		Pmwx::Halfedge_handle h = he;
		if (!h->mDominant) h = h->twin();
		
		bool	iWet = h->face()->IsWater();
		bool	oWet = h->twin()->face()->IsWater();
		bool	border = h->face()->is_unbounded() != h->twin()->face()->is_unbounded();
		bool	coastline = iWet != oWet;
		bool	lu_change = h->face()->mTerrainType != h->twin()->face()->mTerrainType;
		bool	road = !h->mSegments.empty();
		bool	stuff = h->face()->mAreaFeature.mFeatType != h->twin()->face()->mAreaFeature.mFeatType ||
					(h->face()->mAreaFeature.mFeatType != NO_VALUE &&
						h->face()->mAreaFeature.mParams != h->twin()->face()->mAreaFeature.mParams);
		bool	river = h->mParams.find(he_IsRiver) != h->mParams.end();
		if (river && (iWet || oWet)) river = false;	// Wipe out rivers that are inside water bodies or coastlines too.
		if (inKillRivers) river = false;
		bool must_burn = h->mParams.count(he_MustBurn);

		if (!river && !stuff && !road && !coastline && !border && !lu_change && !must_burn)
			deadList.push_back(he);
	}
	PROGRESS_DONE(func, 0, 2, "Finding unneeded half-edges...");
	
	tot = deadList.size();
	ctr = 0;
	
	PROGRESS_START(func, 1,2,"Deleting halfedges...");
	for (vector<Pmwx::Halfedge_handle>::iterator iter = deadList.begin();
		iter != deadList.end(); ++iter, ++ctr)	
	{
		PROGRESS_CHECK(func,1,2,"Deleting halfedges...",ctr,tot,1000);
		ioMap.remove_edge(*iter);
	}	
	PROGRESS_DONE(func,1,2,"Deleting halfedges...");
	return deadList.size();
}

int RemoveUnboundedWater(Pmwx& ioMap)
{
// OPTIMIZE: see above comments about nuke-lists.  Also that while loop needes some examination!
	vector<Pmwx::Halfedge_handle>	deadList;
	
	int nuke = 0;
	
	for (Pmwx::Halfedge_iterator he = ioMap.halfedges_begin(); he != ioMap.halfedges_end(); ++he, ++he)
	{
		Pmwx::Halfedge_handle h = he;
		if (!h->mDominant) h = h->twin();
		
		bool	iWet = h->face()->IsWater();
		bool	oWet = h->twin()->face()->IsWater();
		bool	road = !h->mSegments.empty();

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


// Utility: this notifier establishes the mapping from the src to the dst as we insert!
struct __MergeMaps_EdgeNotifier_t {
	multimap<GISHalfedge *, GISHalfedge *> *		edgeMap;
	GISHalfedge *									srcEdge;
	map<GISVertex *, GISVertex *> *					vertMap;
};

static void __MergeMaps_EdgeNotifier(GISHalfedge * he_old, GISHalfedge * he_new, void * ref)
{
	__MergeMaps_EdgeNotifier_t * info = (__MergeMaps_EdgeNotifier_t *) ref;

	if (he_old && he_new)
	{
		// This is a huge stinking mess...we have split an edge in the new map as we introduce lines.  But this means that
		// our mapping from old to new is no longer right.  Fix it!
		// (This case is not needed for topologically integrated maps but IS needed for not-meant-to-be-friends maps, like
		// a forest pattern + a road grid.
		
		vector<pair<GISHalfedge*, GISHalfedge*> >	new_mappings;
		for (multimap<GISHalfedge*, GISHalfedge*>::iterator pairing = info->edgeMap->begin(); pairing != info->edgeMap->end(); ++pairing)
		{
			if (pairing->second == he_old)
				AssertPrintf("WARNING: splits in the map merge.  This case is foolishly NOT handled yet!");			
		}

	} else {
		// This is a new edge introduced...whether it is a real edge or a dupe of the old one, we need to copy params
		// and establish a linking.
		GISHalfedge * he = (he_old ? he_old : he_new);
		
		// Remember the mapping from old to new
		info->edgeMap->insert(multimap<GISHalfedge *, GISHalfedge *>::value_type(info->srcEdge, he));
		info->edgeMap->insert(multimap<GISHalfedge *, GISHalfedge *>::value_type(info->srcEdge->twin(), he->twin()));
		
		if (he->source()->point() == info->srcEdge->source()->point())
			(*info->vertMap)[info->srcEdge->source()] = he->source();

		if (he->target()->point() == info->srcEdge->source()->point())
			(*info->vertMap)[info->srcEdge->source()] = he->target();

		if (he->source()->point() == info->srcEdge->target()->point())
			(*info->vertMap)[info->srcEdge->target()] = he->source();

		if (he->target()->point() == info->srcEdge->target()->point())
			(*info->vertMap)[info->srcEdge->target()] = he->target();
		
		if (!he->mDominant) he = he->twin();
		
		// Copy the halfedge
		he->mSegments.insert(he->mSegments.end(), info->srcEdge->mSegments.begin(),info->srcEdge->mSegments.end());		
		he->mParams.insert(info->srcEdge->mParams.begin(),info->srcEdge->mParams.end());
	}
}

void MergeMaps(Pmwx& ioDstMap, Pmwx& ioSrcMap, bool inForceProps, set<GISFace *> * outFaces, bool pre_integrated, ProgressFunc func)
{
// OPTIMIZE: now that maps have a point index, we do not need to maintain a prviate one!
	DebugAssert(ioSrcMap.is_valid());

	// Step 1 - we need to copy all halfedges from ioSrcMap to ioDstMap.
	// We'll remember the mappign and also copy attributes.

		multimap<GISHalfedge *, GISHalfedge *>		edgeMap;
		map<GISVertex *, GISVertex *> 				vertMap;
	__MergeMaps_EdgeNotifier_t info;
	info.edgeMap = &edgeMap;
	info.vertMap = &vertMap;
	
	int ctr = 0;
	int total = ioSrcMap.number_of_halfedges();
	int step = total / 200;

	PROGRESS_START(func, 0, 2, "Merging edges into map...")

	if (pre_integrated)
	{
		// Pre-integrated edge merge case - we know there are no intersections, so find points and use the fast insertion routines.
	
		map<Point2, GISVertex *, lesser_y_then_x>			pt_index;
		
		for (Pmwx::Vertex_iterator v = ioDstMap.vertices_begin(); v != ioDstMap.vertices_end(); ++v)
			pt_index[v->point()] = v;

		for (Pmwx::Halfedge_iterator iter = ioSrcMap.halfedges_begin(); iter != ioSrcMap.halfedges_end(); ++iter, ++ctr)
		if (iter->mDominant)
		{
			PROGRESS_CHECK(func, 0, 2, "Merging edges into map...", ctr, total, step)
		
			map<Point2, GISVertex *, lesser_y_then_x>::iterator i1, i2;
			GISHalfedge * nh;
			
			i1 = pt_index.find(iter->source()->point());
			i2 = pt_index.find(iter->target()->point());
			if (i1 != pt_index.end())
			{
				if (i2 != pt_index.end())
				{
					/* CASE 1 - Both points already in. */
					nh = ioDstMap.nox_insert_edge_between_vertices(i1->second, i2->second);
				} 
				else
				{
					/* Case 2 - Point 1 in, point 2 new. */
					nh = ioDstMap.nox_insert_edge_from_vertex(i1->second, iter->target()->point());
					pt_index[iter->target()->point()] = nh->target();
				}
			} 
			else
			{
				if (i2 != pt_index.end())
				{
					/* Case 3 - Point 1 new, point 2 in. */
					nh = ioDstMap.nox_insert_edge_from_vertex(i2->second, iter->source()->point())->twin();
					pt_index[iter->source()->point()] = nh->source();
				} 
				else
				{
					/* Case 4 - both points new. */
					nh = ioDstMap.nox_insert_edge_in_hole(iter->source()->point(), iter->target()->point());
					pt_index[iter->source()->point()] = nh->source();
					pt_index[iter->target()->point()] = nh->target();
				}
			}
			
			edgeMap.insert(multimap<GISHalfedge *, GISHalfedge *>::value_type(iter, nh));
			edgeMap.insert(multimap<GISHalfedge *, GISHalfedge *>::value_type(iter->twin(), nh->twin()));
			if (!nh->mDominant) nh = nh->twin();
			nh->mSegments.insert(nh->mSegments.end(), iter->mSegments.begin(),iter->mSegments.end());		
			nh->mParams.insert(iter->mParams.begin(),iter->mParams.end());
			
		}
		
		
	} else {
	
		// Slow case - use inserts with intersections.

		int fast = 0, slow = 0;
		for (Pmwx::Halfedge_iterator iter = ioSrcMap.halfedges_begin(); iter != ioSrcMap.halfedges_end(); ++iter, ++ctr)
		if (iter->mDominant)
		{
			PROGRESS_CHECK(func, 0, 2, "Merging edges into map...", ctr, total, step)
		
			info.srcEdge = iter;
			map<GISVertex *, GISVertex *>::iterator hint = vertMap.find(iter->source());
			if (hint != vertMap.end()) 
			{
				++fast;
				ioDstMap.insert_edge(iter->source()->point(), iter->target()->point(), hint->second->halfedge(), Pmwx::locate_Vertex, __MergeMaps_EdgeNotifier, &info);
			} else {
				++slow;
				ioDstMap.insert_edge(iter->source()->point(), iter->target()->point(), __MergeMaps_EdgeNotifier, &info);
			}
		}
	}

	PROGRESS_DONE(func, 0, 2, "Merging edges into map...")

	PROGRESS_START(func, 1, 2, "Copying face metadata...")
	
	// STEP 2 - map a faces boundary from src to dst.  Flood fill to find the corresponding faces and copy 
	// what's needed.
	
	ctr = 0;
	total = ioSrcMap.number_of_faces();
	step = total / 200;
	
	for (Pmwx::Face_iterator fiter = ioSrcMap.faces_begin(); fiter != ioSrcMap.faces_end(); ++fiter, ++ctr)
	if (!fiter->is_unbounded())
	// Fast eval - if the source face is uninteresting, skip this whole loop!
	if (fiter->mAreaFeature.mFeatType != NO_VALUE || fiter->mTerrainType != terrain_Natural)
	{
		PROGRESS_CHECK(func, 1, 2, "Copying face metadata...", ctr, total, step)
	

		set<GISHalfedge *>	borders_old, borders_new;

		FindEdgesForFace(fiter, borders_old);
		
		set<GISFace *>	sanity;
		FindFacesForEdgeSet(borders_old, sanity);
		DebugAssert(sanity.size() == 1);
		DebugAssert(*sanity.begin() == &*fiter);
		
		DebugAssert(!borders_old.empty());
		
		// go through CCB - add all dests in edgemap from each CCB to borders
		for (set<GISHalfedge *>::iterator e = borders_old.begin(); e != borders_old.end(); ++e)
		{
			pair<multimap<GISHalfedge *, GISHalfedge *>::iterator, multimap<GISHalfedge *, GISHalfedge *>::iterator>	range;
			range = edgeMap.equal_range(*e);			
			DebugAssert(range.first != range.second);
			
			for (multimap<GISHalfedge *, GISHalfedge *>::iterator i = range.first; i != range.second; ++i)
			{
				borders_new.insert(i->second);
			}
		}		
		DebugAssert(!borders_new.empty());
		
		// Next find facse for the edge set
		set<GISFace *>		faces;
		
		FindFacesForEdgeSet(borders_new, faces);		
		DebugAssert(!faces.empty());

		// Finally, mark all items in faces.
		
		for (set<GISFace *>::iterator face = faces.begin(); face != faces.end(); ++face)
		{
			if (outFaces)	outFaces->insert(*face);
			if (fiter->mTerrainType != terrain_Natural)	
			if (inForceProps || (*face)->mTerrainType == terrain_Natural)
				(*face)->mTerrainType = fiter->mTerrainType;

			if (fiter->mAreaFeature.mFeatType != NO_VALUE)		
			if (inForceProps || (*face)->mAreaFeature.mFeatType == NO_VALUE)
				(*face)->mAreaFeature.mFeatType = fiter->mAreaFeature.mFeatType;
			
		}
	}
	PROGRESS_DONE(func, 1, 2, "Copying face metadata...")
	
}

// Vertex hasher - to be honest I forget why this was necessary, but without it the 
// STL couldn't build the hash table.  Foo.
#if !MSC
struct hash_vertex {
	typedef GISVertex * KeyType;
	size_t operator()(const KeyType& key) const { return (size_t) key; }
};
#endif

// Map swap - Basically we build up mapping between the two maps and flood fill to
// find the insides.
void	SwapMaps(	Pmwx& 							ioMapA, 
					Pmwx& 							ioMapB, 
					const vector<GISHalfedge *>&	inBoundsA,
					const vector<GISHalfedge *>&	inBoundsB)
{
	DebugAssert(inBoundsA.size() == inBoundsB.size());
	
	set<GISFace *>		moveFaceFromA, moveFaceFromB;
	set<GISHalfedge *>	moveEdgeFromA, moveEdgeFromB;
	set<GISVertex * >	moveVertFromA, moveVertFromB;
#if MSC
	hash_map<GISVertex *, GISVertex *>	keepVertFromA, keepVertFromB;
	hash_map<GISVertex *, GISVertex *>::iterator findVert;
#else
	hash_map<GISVertex *, GISVertex *, hash_vertex>	keepVertFromA, keepVertFromB;
	hash_map<GISVertex *, GISVertex *, hash_vertex>::iterator findVert;
#endif
	set<GISFace *>::iterator		faceIter;
	set<GISHalfedge *>::iterator	edgeIter;	
	set<GISVertex *>::iterator		vertIter;	
	int n;	

#if DEV
	for (n = 0; n < inBoundsA.size(); ++n)
	{
		DebugAssert(inBoundsA[n]->target()->point() == inBoundsB[n]->target()->point());
		DebugAssert(inBoundsA[n]->source()->point() == inBoundsB[n]->source()->point());
		DebugAssert(inBoundsA[n]->face() != inBoundsA[n]->twin()->face());
		DebugAssert(inBoundsB[n]->face() != inBoundsB[n]->twin()->face());
	}
#endif
	
	/********************************************************************************
	 * PREPERATION - FIGURE OUT WHO IS MOVING WHERE.
	 ********************************************************************************/
	
	// All of the inner bounds have to move.
	moveEdgeFromA.insert(inBoundsA.begin(), inBoundsA.end());
	moveEdgeFromB.insert(inBoundsB.begin(), inBoundsB.end());
	
	// Inner bounds dictate contained faces, all of which move.
	FindFacesForEdgeSet(moveEdgeFromA, moveFaceFromA);
	FindFacesForEdgeSet(moveEdgeFromB, moveFaceFromB);
	
	// Accumulate total set of edges that must move from faces (CCBs and holes.)
	for (faceIter = moveFaceFromA.begin(); faceIter != moveFaceFromA.end(); ++faceIter)
		FindEdgesForFace(*faceIter, moveEdgeFromA);

	for (faceIter = moveFaceFromB.begin(); faceIter != moveFaceFromB.end(); ++faceIter)
		FindEdgesForFace(*faceIter, moveEdgeFromB);
	
	for (n = 0; n < inBoundsA.size(); ++n)
	{
		// We want stuff on the crop edge to go to the inside.  This is because 99% of the time
		// the crop inside is used.  So make sure it is dominant.  It's okay to f--- with dominance
		// because the edge gets resorted later when we do the move of the halfedge.
		if (!inBoundsA[n]->mDominant)		inBoundsA[n]->SwapDominance();
		if (!inBoundsB[n]->mDominant)		inBoundsB[n]->SwapDominance();	

		// Vertices on the CCB do NOT move since they are used by exterior stuff.
		// We need to know the correspondence for later!  So build a map
		// of how they relate for quick access.
#if MSC
		keepVertFromA.insert(hash_map<GISVertex *, GISVertex *>::value_type(inBoundsA[n]->target(), inBoundsB[n]->target()));
		keepVertFromB.insert(hash_map<GISVertex *, GISVertex *>::value_type(inBoundsB[n]->target(), inBoundsA[n]->target()));
#else
		keepVertFromA.insert(hash_map<GISVertex *, GISVertex *, hash_vertex>::value_type(inBoundsA[n]->target(), inBoundsB[n]->target()));
		keepVertFromB.insert(hash_map<GISVertex *, GISVertex *, hash_vertex>::value_type(inBoundsB[n]->target(), inBoundsA[n]->target()));
#endif
	}
	
	// Accume all non-saved vertices as moving.
	for (edgeIter = moveEdgeFromA.begin(); edgeIter != moveEdgeFromA.end(); ++edgeIter)
		if (keepVertFromA.count((*edgeIter)->target()) == 0)
			moveVertFromA.insert((*edgeIter)->target());

	for (edgeIter = moveEdgeFromB.begin(); edgeIter != moveEdgeFromB.end(); ++edgeIter)
		if (keepVertFromB.count((*edgeIter)->target()) == 0)
			moveVertFromB.insert((*edgeIter)->target());
	

	/********************************************************************************
	 * SANITY CHECK!
	 ********************************************************************************/

#if DEV
	// We don't move the unbounded face!!
	for (faceIter = moveFaceFromA.begin(); faceIter != moveFaceFromA.end(); ++faceIter)
		DebugAssert(!(*faceIter)->is_unbounded());

	for (faceIter = moveFaceFromB.begin(); faceIter != moveFaceFromB.end(); ++faceIter)
		DebugAssert(!(*faceIter)->is_unbounded());
		
	// Edges all ref a face that's moving too
	for (edgeIter = moveEdgeFromA.begin(); edgeIter != moveEdgeFromA.end(); ++edgeIter)
	{
		DebugAssert(!(*edgeIter)->face()->is_unbounded());
		DebugAssert(moveFaceFromA.count((*edgeIter)->face()) > 0);
	}

	for (edgeIter = moveEdgeFromB.begin(); edgeIter != moveEdgeFromB.end(); ++edgeIter)
	{
		DebugAssert(!(*edgeIter)->face()->is_unbounded());
		DebugAssert(moveFaceFromB.count((*edgeIter)->face()) > 0);
	}
	// TODO: confirm all bounds of all faces are in the edge set?
#endif

	

	/********************************************************************************
	 * FIX REFS TO NON-MOVING VERTICES
	 ********************************************************************************/
	
	// For each vertex we are keeping, if it references a CCB halfedge, better swap it
	// over to the new CCB.	 We take the cheap way out and always force the vertex ptr!
	for (n = 0; n < inBoundsA.size(); ++n)
	{
		inBoundsA[n]->target()->set_halfedge(inBoundsB[n]);
		inBoundsB[n]->target()->set_halfedge(inBoundsA[n]);
	}
	 
	 // For each halfedge we are moving, we need to see if it references a non-moving 
	 // vertex.  If so, we need to make sure it now references the new one.	 
	for (edgeIter = moveEdgeFromA.begin(); edgeIter != moveEdgeFromA.end(); ++edgeIter)
	{
		findVert = keepVertFromA.find((*edgeIter)->target());
		if (findVert != keepVertFromA.end())
			(*edgeIter)->set_target(findVert->second);
	}

	for (edgeIter = moveEdgeFromB.begin(); edgeIter != moveEdgeFromB.end(); ++edgeIter)
	{
		findVert = keepVertFromB.find((*edgeIter)->target());
		if (findVert != keepVertFromB.end())
			(*edgeIter)->set_target(findVert->second);
	}

	/********************************************************************************
	 * SWAP THE CCB
	 ********************************************************************************/

	// Wait this is really easy!  Faces aren't moving.  The "next" of each ring is
	// preserved.  We've already adjusted all vertices.  So twin swapping is all
	// it takes!

	for (n = 0; n < inBoundsA.size(); ++n)
	{
		GISHalfedge * a_twin = inBoundsA[n]->twin();
		GISHalfedge * b_twin = inBoundsB[n]->twin();

		a_twin->set_twin(inBoundsB[n]);
		b_twin->set_twin(inBoundsA[n]);
		
		inBoundsA[n]->set_twin(b_twin);
		inBoundsB[n]->set_twin(a_twin);
		
		// Make sure dominance is on inside.
		DebugAssert(inBoundsA[n]->mDominant);
		DebugAssert(inBoundsB[n]->mDominant);
	}
		

	/********************************************************************************
	 * MIGRATE ALL ENTITIES
	 ********************************************************************************/
	
	// Finally we just need to swap each entity into a new list.

	for (faceIter = moveFaceFromA.begin(); faceIter != moveFaceFromA.end(); ++faceIter)
		ioMapB.MoveFaceToMe(&ioMapA, *faceIter);
	for (faceIter = moveFaceFromB.begin(); faceIter != moveFaceFromB.end(); ++faceIter)
		ioMapA.MoveFaceToMe(&ioMapB, *faceIter);

	for (vertIter = moveVertFromA.begin(); vertIter != moveVertFromA.end(); ++vertIter)
		ioMapA.UnindexVertex(*vertIter);
	for (vertIter = moveVertFromB.begin(); vertIter != moveVertFromB.end(); ++vertIter)
		ioMapB.UnindexVertex(*vertIter);
	for (vertIter = moveVertFromA.begin(); vertIter != moveVertFromA.end(); ++vertIter)
		ioMapB.MoveVertexToMe(&ioMapA, *vertIter);
	for (vertIter = moveVertFromB.begin(); vertIter != moveVertFromB.end(); ++vertIter)
		ioMapA.MoveVertexToMe(&ioMapB, *vertIter);
	for (vertIter = moveVertFromA.begin(); vertIter != moveVertFromA.end(); ++vertIter)
		ioMapB.ReindexVertex(*vertIter);
	for (vertIter = moveVertFromB.begin(); vertIter != moveVertFromB.end(); ++vertIter)
		ioMapA.ReindexVertex(*vertIter);

	for (int n = 0; n < inBoundsA.size(); ++n)
	{
		ioMapB.MoveHalfedgeToMe(&ioMapA, inBoundsA[n]);
		ioMapA.MoveHalfedgeToMe(&ioMapB, inBoundsB[n]);
		moveEdgeFromA.erase(inBoundsA[n]);
		moveEdgeFromB.erase(inBoundsB[n]);
	}

	for (edgeIter = moveEdgeFromA.begin(); edgeIter != moveEdgeFromA.end(); ++edgeIter)
	if ((*edgeIter)->mDominant)
		ioMapB.MoveEdgeToMe(&ioMapA, *edgeIter);
	for (edgeIter = moveEdgeFromB.begin(); edgeIter != moveEdgeFromB.end(); ++edgeIter)
	if ((*edgeIter)->mDominant)
		ioMapA.MoveEdgeToMe(&ioMapB, *edgeIter);
		
}

// Sort op to sort a bbox bby its lower bounds.
struct	__sort_by_bbox_base {
	bool operator()(const Bbox2& lhs, const Bbox2& rhs) const { return lhs.ymin() < rhs.ymin(); }
};

// I think we didn't end up needing this...the idea was to avoid tiny slivers.
static bool __epsi_intersect(const Segment2& segA, const Segment2& segB, double epsi, Point2& cross)
{
	if (segA.is_horizontal() || segA.is_vertical() ||
		segB.is_horizontal() || segB.is_vertical())
	{
		return segA.intersect(segB, cross);
	}
	
	if (!Line2(segA).intersect(Line2(segB), cross)) return false;
	
//	epsi = epsi * epsi;
	
	double	da1 = Vector2(segA.p1, cross).squared_length();
	double	da2 = Vector2(segA.p2, cross).squared_length();
	double	db1 = Vector2(segB.p1, cross).squared_length();
	double	db2 = Vector2(segB.p2, cross).squared_length();
	
	if (da1 < epsi || da2 < epsi || db1 < epsi || db2 < epsi)
	{	
		double da = (da1 < da2) ? da1 : da2;
		Point2 pa = (da1 < da2) ? segA.p1 : segA.p2;
		double db = (db1 < db2) ? db1 : db2;
		Point2 pb = (db1 < db2) ? segB.p1 : segB.p2;
		
		if (da < epsi && db < epsi)
		{
			cross = (da < db) ? pa : pb;
			return true;
		} else if (da < epsi) {
			cross = pa;
			return true;
		} else {
			DebugAssert(db < epsi);
			cross = pb;
			return true;
		}		
	}
	return segA.collinear_has_on(cross) && segB.collinear_has_on(cross);
}

/*
 * TopoIntegrateMaps
 *
 * The idea here is pretty simple: we are going to insert points into both A and B so that anywhere edges cross
 * in A and B there is a pre-inserted vertex, and more importantly that vertex has the EXACT same value in both
 * pmwxs.  That means that when we go to merge, there will not be any line-line intersections and the resulting
 * map will not have slivering problems and will be topologically correct.
 *
 * Example: point P of an overlay map is almost colinear with halfedge H in the main map.  But P has two halfedges
 * Pa and Pb on opposite sides of H.  When we insert Pa and Pb, we will get a judgement that each segment may or may
 * not cross H (and if it does) at a crossing point Ca and Cb that will be different, because the intersection algorithm
 * "jitters" based on the slope of Pa and Pb which are different.  In the worst case, P is taken to be on the same side
 * of H as Pa and Pb, resulting in topologically different locations for P and a double-point insertion.
 *
 * By pre-breaking Pa and/or Pb at a single crossing point C that is then inserted into H, we guarantee consistent
 * topological judgements for P (it must be on exactly one side of C by the topological relationships of the overlay
 * map since the overlay is modified to have C inserted as a split edge in only one of Pa ir Pb) and we don't get a
 * double insert of P.
 *
 */
 
 // Colinear check - this tells us if two points are very close to each other but not truly colinear.
#define	SMALL_SEG_CUTOFF	0.01
#define BBOX_SLOP			0.00001
#define NEAR_SLIVER_COLINEAR 7.7e-12

inline bool	__near_colinear(const Segment2& seg, Point2& p)
{
	// Don't return true for the degenerate case - isn't useful!
	if (seg.p1 == p) return false;
	if (seg.p2 == p) return false;
	// Special case h and v.
	if (seg.is_horizontal())
		return p.y == seg.p1.y && ((seg.p1.x < p.x && p.x < seg.p2.x) || (seg.p2.x < p.x && p.x < seg.p1.x));
	if (seg.is_vertical())
		return p.x == seg.p1.x && ((seg.p1.y < p.y && p.y < seg.p2.y) || (seg.p2.y < p.y && p.y < seg.p1.y));
	
	Point2	pp = seg.projection(p);
	
	if (p == pp || Vector2(p, pp).squared_length() < NEAR_SLIVER_COLINEAR)
	{
		return (Vector2(seg.p1, seg.p2).dot(Vector2(seg.p1, p)) > 0.0) &&
			   (Vector2(seg.p2, seg.p1).dot(Vector2(seg.p2, p)) > 0.0);
	}
	return false;
}


// Topo integration.  Basically for any two colinear segments, insert each other's 
// end poitns into the map, and for any crossing segments, insert the crossing point.
// The hope is that:
// (1) there will be no crossings between segs in map A and B - only segments ending
// in common vertices (who are perfectly equal), and
// (2) where segments overlap, they are broken into two perfectly overlapping segments
// and non-overlapping segments.
//
// Please note that where a point P is near-colinear to an edge E, that edge E ends up
// CHANGING to E1 and E2 to meet point P.  Basically we'd rather ruin our map than get 
// a sliver.
//
// We spatially index the bounding box of all edges to speed up intersection finding!
// Also please note that we split all indexed boxe sinto "big and small" by some
// arbitrary size...the idea is: we have to look at big items a lot so we put the small
// pieces in a separate container that can be searched with less "slop".
//
// WARNING: SMALL_SEG_CUTOFF is hard-coded!
void TopoIntegrateMaps(Pmwx * mapA, Pmwx * mapB)
{
// OPTIMIZE: to be explored - would a merge sort (after sorting both sets) be time-faster
// than what we have now?  How well does the double-partition work?

	typedef multimap<Bbox2, GISHalfedge *, __sort_by_bbox_base>		HalfedgeMap;
	typedef multimap<GISHalfedge *, Point2>		SplitMap;
	typedef	set<GISHalfedge *>					SplitSet;
	typedef map<double, Point2>					SortedPoints;
		
		HalfedgeMap				map_small;
		HalfedgeMap				map_big;

		double					yrange_small = 0.0;
		double					yrange_big = 0.0;
			
		SplitMap				splitA, splitB;
		SplitSet				setA, setB;	
		Pmwx::Halfedge_iterator iterA;
		Pmwx::Halfedge_iterator iterB;
		Point2					p;
		double					dist;
		pair<HalfedgeMap::iterator, HalfedgeMap::iterator>	possibles;
		HalfedgeMap::iterator								he_box;
		
		pair<SplitMap::iterator, SplitMap::iterator>		range;
		SplitSet::iterator 									he;
		SplitMap::iterator									splitIter;
		SortedPoints::iterator								spi;
		
		SortedPoints 										splits;
		GISHalfedge *										subdiv;
		Bbox2												key_lo, key_hi;

	// Note: we want to index the larger pmwx.  This way we can rule out by spatial sorting a lot more checks.
	if (mapA->number_of_halfedges() > mapB->number_of_halfedges())
	{
#if TRACE_TOPO_INTEGRATE
		printf("Swapwping maps for speed.\n");
#endif
	
		swap(mapA, mapB);
	}
	
	// We are going to store the halfedges of B in two maps, sorted by their bbox's ymin.
	// The idea here is to have a set of very small segments...we can be real precise in the
	// range we scan here.  The big map will have all the huge exceptions, which we'll have to
	// totally iterate, but that wil be a much smaller set.
	for (iterB = mapB->halfedges_begin(); iterB != mapB->halfedges_end(); ++iterB)
	if (iterB->mDominant)
	{
		Bbox2	bounds(iterB->source()->point(),iterB->target()->point());
		bounds.expand(BBOX_SLOP);
		
		dist = bounds.ymax() - bounds.ymin();
		if (dist > SMALL_SEG_CUTOFF)
		{
			yrange_big = max(yrange_big, dist);
			map_big.insert(HalfedgeMap::value_type(bounds, iterB));
		} else {
			yrange_small = max(yrange_small, dist);
			map_small.insert(HalfedgeMap::value_type(bounds, iterB));
		}
		
	}

	// Go through and search both the small and big set, looking for halfedges.
	for (iterA = mapA->halfedges_begin(); iterA != mapA->halfedges_end(); ++iterA)
	if (iterA->mDominant)		
	{
		Segment2	segA(iterA->source()->point(), iterA->target()->point());
		Bbox2		boxA(iterA->source()->point(), iterA->target()->point());
		boxA.expand(BBOX_SLOP);

		bool did_colinear;
		key_lo = Point2(0.0, boxA.ymin() - yrange_big);
		key_hi = Point2(0.0, boxA.ymax());
		possibles.first = map_big.lower_bound(key_lo);
		possibles.second = map_big.upper_bound(key_hi);
		for (he_box = possibles.first; he_box != possibles.second; ++he_box)
		{
			if (boxA.overlap(he_box->first))
			{
				Segment2	segB(he_box->second->source()->point(), he_box->second->target()->point());
				
				// IMPORTANT: we need to check for the almost-colinear case first.  Why?  Well, it turns out that
				// in a near-colinear case, sometimes the intersect route will return an intersection.  But we do 
				// not want to insert some bogus point P into two lines if really they overlap!  P might be epsi-
				// equal to an end point and then we're f--ed.  So do colinear first and skip the intersection
				// if we find one.
				
				did_colinear = false;
				if (segA.is_horizontal() == segB.is_horizontal() && segA.is_vertical() == segB.is_vertical())
				{
					if (__near_colinear(segA, segB.p1))
					{
						did_colinear = true;
						splitA.insert(SplitMap::value_type(iterA, segB.p1));
						setA.insert(iterA);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split A %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y,segB.p1.x,segB.p1.y);
#endif						
					}
					if (__near_colinear(segA, segB.p2))
					{
						did_colinear = true;
						splitA.insert(SplitMap::value_type(iterA, segB.p2));
						setA.insert(iterA);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split A %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y,segB.p2.x,segB.p2.y);
#endif						
					}
					if (__near_colinear(segB, segA.p1))
					{
						did_colinear = true;
						splitB.insert(SplitMap::value_type(he_box->second, segA.p1));
						setB.insert(he_box->second);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split B %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y,segA.p1.x,segA.p1.y);
#endif						
					}
					if (__near_colinear(segB, segA.p2))
					{
						did_colinear = true;
						splitB.insert(SplitMap::value_type(he_box->second, segA.p2));
						setB.insert(he_box->second);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split B %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y,segA.p2.x,segA.p2.y);
#endif						
					}
				}
				
				if (!did_colinear && segA.intersect(segB, p))	//__epsi_intersect(segA, segB, NEAR_INTERSECT_DIST, p))
				{
					if (p != segA.p1 && p != segA.p2)
					{
						splitA.insert(SplitMap::value_type(iterA, p));
						setA.insert(iterA);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split A %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: intersect from %.15lf,%.15lf->%.15lf,%.15lf.\n",
								segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y,  p.x,p.y,  segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y);
#endif						
						
					}
					if (p != segB.p1 && p != segB.p2)
					{
						splitB.insert(SplitMap::value_type(he_box->second, p));
						setB.insert(he_box->second);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split B %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: intersect from %.15lf,%.15lf->%.15lf,%.15lf.\n",
								segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y,  p.x,p.y,  segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y);
#endif						
					}
				}
			}
		}
		
		key_lo = Point2(0.0, boxA.ymin() - yrange_small);
		key_hi = Point2(0.0, boxA.ymax());
		possibles.first = map_small.lower_bound(key_lo);
		possibles.second = map_small.upper_bound(key_hi);
		for (he_box = possibles.first; he_box != possibles.second; ++he_box)
		{
			if (boxA.overlap(he_box->first))
			{
				Segment2	segB(he_box->second->source()->point(), he_box->second->target()->point());
				did_colinear = false;				
				
				if (segA.is_horizontal() == segB.is_horizontal() && segA.is_vertical() == segB.is_vertical())
				{
					if (__near_colinear(segA, segB.p1))
					{
						did_colinear = true;				
						splitA.insert(SplitMap::value_type(iterA, segB.p1));
						setA.insert(iterA);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split A %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y,segB.p1.x,segB.p1.y);
#endif						
					}
					if (__near_colinear(segA, segB.p2))
					{
						did_colinear = true;				
						splitA.insert(SplitMap::value_type(iterA, segB.p2));
						setA.insert(iterA);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split A %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y,segB.p2.x,segB.p2.y);
#endif						
					}
					if (__near_colinear(segB, segA.p1))
					{
						did_colinear = true;				
						splitB.insert(SplitMap::value_type(he_box->second, segA.p1));
						setB.insert(he_box->second);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split B %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y,segA.p1.x,segA.p1.y);
#endif						
					}
					if (__near_colinear(segB, segA.p2))
					{
						did_colinear = true;				
						splitB.insert(SplitMap::value_type(he_box->second, segA.p2));
						setB.insert(he_box->second);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split B %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: near collinear.\n",
								segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y,segA.p2.x,segA.p2.y);
#endif						
					}
				}
				
				if (!did_colinear && segA.intersect(segB, p))
				{
					if (p != segA.p1 && p != segA.p2)
					{
						splitA.insert(SplitMap::value_type(iterA, p));
						setA.insert(iterA);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split A %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: intersect from %.15lf,%.15lf->%.15lf,%.15lf.\n",
								segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y,  p.x,p.y,  segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y);
#endif						
					}
					if (p != segB.p1 && p != segB.p2)
					{
						splitB.insert(SplitMap::value_type(he_box->second, p));
						setB.insert(he_box->second);
#if TRACE_TOPO_INTEGRATE	
						printf("Will split B %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf: intersect from %.15lf,%.15lf->%.15lf,%.15lf.\n",
								segB.p1.x,segB.p1.y,segB.p2.x,segB.p2.y,  p.x,p.y,  segA.p1.x,segA.p1.y,segA.p2.x,segA.p2.y);
#endif						
					}
				}
			}
		}
	}

	// Now go through and actually make the splits.
	for (he = setA.begin(); he != setA.end(); ++he)
	{
		range = splitA.equal_range(*he);
		splits.clear();

		Segment2 origSegA((*he)->source()->point(), (*he)->target()->point());
		for (splitIter = range.first; splitIter != range.second; ++splitIter)
		{
			dist = Segment2((*he)->source()->point(), splitIter->second).squared_length();
			splits.insert(SortedPoints::value_type(dist, splitIter->second));
		}
		
		subdiv = *he;
		for (spi = splits.begin(); spi != splits.end(); ++spi)
		{
#if DEV
			if (origSegA.is_vertical())
				DebugAssert(origSegA.p1.x == spi->second.x);
			if (origSegA.is_horizontal())
				DebugAssert(origSegA.p1.y == spi->second.y);
#endif
#if TRACE_TOPO_INTEGRATE
			printf("Splitting A: %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf\n", 
				subdiv->source()->point().x,subdiv->source()->point().y,
				subdiv->target()->point().x,subdiv->target()->point().y,
				spi->second.x, spi->second.y);
#endif
		// BEN SAYS: HOLY CRAP!!! SCARY LAST MINUTE CHANGE!  Try the Dr it hurts strategy when a bad intersect gives us a vertex we already have.  Hope that in
		// the actual render the fact that the locate WILL succeed will keep things from falling appart.
//			if (mapA->locate_vertex(spi->second) == NULL)
			{
				mapA->split_edge(subdiv, spi->second);
				subdiv = subdiv->next();
			}
		}
	}

	for (he = setB.begin(); he != setB.end(); ++he)
	{
		range = splitB.equal_range(*he);
		splits.clear();

		Segment2	origSegB((*he)->source()->point(), (*he)->target()->point());
		for (splitIter = range.first; splitIter != range.second; ++splitIter)
		{
			dist = Segment2((*he)->source()->point(), splitIter->second).squared_length();
			splits.insert(SortedPoints::value_type(dist, splitIter->second));
		}
		
		subdiv = *he;
		for (spi = splits.begin(); spi != splits.end(); ++spi)
		{
#if DEV
			if (origSegB.is_vertical())
				DebugAssert(origSegB.p1.x == spi->second.x);
			if (origSegB.is_horizontal())
				DebugAssert(origSegB.p1.y == spi->second.y);
#endif				
#if TRACE_TOPO_INTEGRATE
			printf("Splitting B: %.15lf,%.15lf->%.15lf,%.15lf at %.15lf,%.15lf\n", 
				subdiv->source()->point().x,subdiv->source()->point().y,
				subdiv->target()->point().x,subdiv->target()->point().y,
				spi->second.x, spi->second.y);
#endif
//			if (mapB->locate_vertex(spi->second) == NULL)
			{
				mapB->split_edge(subdiv, spi->second);
				subdiv = subdiv->next();
			}
		}
	}	
}


// Safe ring insert - basically check to see if the vertices exist and do the right thing.
GISFace * SafeInsertRing(Pmwx * inPmwx, GISFace * parent, const vector<Point2>& inPoints)
{
// OPTIMIZE: re-examine this now that we have vertex indexing...can we use the built in index?
// SPECULATION: even with point indexing, insert_ring is STILL faster - insert_edge, even when
// both points are known still must find the sorting point around the vertices and worse yet 
// potentially has to split faces and move holes...all slow!  The ring does NONE of this.

	bool	needs_slow = false;
	set<Point2, lesser_x_then_y>	pts;
	int n;
	for (n = 0; n < inPoints.size(); ++n)
	{
		if (inPmwx->locate_vertex(inPoints[n]) != NULL)
		{
			needs_slow = true;
			break;
		}
		if (pts.count(inPoints[n]) > 0)
		{
			needs_slow = true;
			break;
		}
		pts.insert(inPoints[n]);
	}

	if (!needs_slow)
		return inPmwx->insert_ring(parent, inPoints);
	
	GISHalfedge * he = NULL;	
	for (n = 0; n < inPoints.size(); ++n)
	{
		if(inPoints[n] != inPoints[(n+1)%inPoints.size()])
			he = inPmwx->insert_edge(inPoints[n], inPoints[(n+1)%inPoints.size()], NULL, NULL);
	}
	Assert(he);
	return he->face();
}

/************************************************************************************************
 * MAP ANALYSIS AND RASTERIZATION/ANALYSIS
 ************************************************************************************************/
#pragma mark -

// Dominance: for conveninence I mark half my edges with a flag...this is a rapid way to 
// guarantee we never list an edge and its twin even when the edge set we get is not an in-order
// iteration.  Cheesy but it works.
bool	ValidateMapDominance(const Pmwx& inMap)
{
	int doubles = 0;
	int zeros = 0;
	int ctr = 0;
	int wrong = 0;
	for (Pmwx::Halfedge_const_iterator i = inMap.halfedges_begin(); i != inMap.halfedges_end(); ++i, ++ctr)
	{
		bool ideal_dom = ((ctr % 2) == 0);
		if (i->mDominant != ideal_dom)
		{
#if DEV
			++wrong;
#else
			return false;
#endif
		}
		if (i->mDominant && i->twin()->mDominant)
		{
#if DEV
			++doubles;
			printf("Double on edge: %lf,%lf -> %lf,%lf\n", 
				i->source()->point().x,i->source()->point().y,
				i->target()->point().x,i->target()->point().y);
#else
			return false;
#endif			
		}
		if (!i->mDominant && !i->twin()->mDominant)
		{
#if DEV
			++zeros;
			printf("Zero on edge: %lf,%lf -> %lf,%lf\n", 
				i->source()->point().x,i->source()->point().y,
				i->target()->point().x,i->target()->point().y);
#else
			return false;
#endif			
		}
	}
#if DEV
	if (doubles > 0 || zeros > 0 || wrong > 0)
		printf("Validation : %d double-dominant halfedges, and %d zero-dominant halfedges.  %d wrong\n", doubles, zeros, wrong);
#endif	
	return doubles == 0 && zeros == 0 && wrong == 0;
}


void	CalcBoundingBox(
			const Pmwx&		inMap,
			Point2&			sw,
			Point2&			ne)
{
	bool		inited = false;
	Bbox2		box;
	
	for (Pmwx::Holes_const_iterator holes = inMap.unbounded_face()->holes_begin();
		holes != inMap.unbounded_face()->holes_end(); ++holes)
	{
		Pmwx::Ccb_halfedge_const_circulator	cur, last;
		cur = last = *holes;
		do {

			if (!inited)
			{
				box = Bbox2(cur->source()->point());
				inited = true;
			}
			
			box += cur->source()->point();
			box += cur->target()->point();
		
			++cur;
		} while (cur != last);		
	}
	sw = box.p1;
	ne = box.p2;
}			

double	GetMapFaceAreaMeters(const Pmwx::Face_handle f)
{
	if (f->is_unbounded()) return -1.0;
	Polygon2	outer;
	Pmwx::Ccb_halfedge_circulator	circ = f->outer_ccb();
	Pmwx::Ccb_halfedge_circulator	start = circ;
	do {
			outer.push_back(Point2(circ->source()->point().x,circ->source()->point().y));
		++circ;
	} while (circ != start);
	
	CoordTranslator trans;
	
	CreateTranslatorForPolygon(outer, trans);
	
	for (int n = 0; n < outer.size(); ++n)
	{
		outer[n] = trans.Forward(outer[n]);
	}
	
	double me = outer.area();

	for (Pmwx::Holes_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
	{
		Polygon2	ib;
		Pmwx::Ccb_halfedge_circulator	circ(*h);
		Pmwx::Ccb_halfedge_circulator	start = circ;
		do {
				ib.push_back(Point2(circ->source()->point().x,circ->source()->point().y));
			++circ;
		} while (circ != start);
		
		for (int n = 0; n < ib.size(); ++n)
			ib[n] = trans.Forward(ib[n]);
		
		me += ib.area();
	}
	return me;
}

double	GetMapEdgeLengthMeters(const Pmwx::Halfedge_handle e)
{
	return LonLatDistMeters(
				e->source()->point().x,
				e->source()->point().y,
				e->target()->point().x,
				e->target()->point().y);
}

float	GetParamAverage(const Pmwx::Face_handle f, const DEMGeo& dem, float * outMin, float * outMax)
{
	PolyRasterizer	rast;
	int count = 0;
	float e;
	float avg = 0.0;
	int y = SetupRasterizerForDEM(f, dem, rast);
	rast.StartScanline(y);
	while (!rast.DoneScan())
	{
		int x1, x2;
		while (rast.GetRange(x1, x2))
		{
			for (int x = x1; x < x2; ++x)
			{
				e = dem.get(x,y);
				if (e != DEM_NO_DATA)
				{
					if (count == 0) 
					{
						if (outMin) *outMin = e;
						if (outMax) *outMax = e;
					} else {
						if (outMin) *outMin = min(*outMin,e);
						if (outMax) *outMax = max(*outMax,e);
					}
					avg += e;
					count++;
				}
			}
		}
		++y;
		if (y >= dem.mHeight) break;
		rast.AdvanceScanline(y);		
	}
	if (count == 0)
	{
		Pmwx::Ccb_halfedge_const_circulator stop, i;
		i = stop = f->outer_ccb();
		do {
			e = dem.value_linear(i->source()->point().x,i->source()->point().y);
			if (e != DEM_NO_DATA)
			{
				if (count == 0) 
				{
					if (outMin) *outMin = e;
					if (outMax) *outMax = e;
				} else {
					if (outMin) *outMin = min(*outMin,e);
					if (outMax) *outMax = max(*outMax,e);
				}
				avg += e;
				count++;
			}
			++i;
		} while (i != stop);
	}
	if (count > 0)
		return avg / (float) count;
	else
		return DEM_NO_DATA;	
}

int	GetParamHistogram(const Pmwx::Face_handle f, const DEMGeo& dem, map<float, int>& outHistogram)
{
	PolyRasterizer	rast;
	int count = 0;
	int e;
	int y = SetupRasterizerForDEM(f, dem, rast);
	rast.StartScanline(y);
	while (!rast.DoneScan())
	{
		int x1, x2;
		while (rast.GetRange(x1, x2))
		{
			for (int x = x1; x < x2; ++x)
			{
				e = dem.get(x,y);
				if (e != DEM_NO_DATA)
					count++, outHistogram[e]++;
			}
		}
		++y;
		if (y >= dem.mHeight) break;
		rast.AdvanceScanline(y);		
	}
	if (count == 0)
	{
		Pmwx::Ccb_halfedge_const_circulator stop, i;
		i = stop = f->outer_ccb();
		do {
			e = dem.xy_nearest(i->source()->point().x,i->source()->point().y);
			if (e != DEM_NO_DATA)
				count++, outHistogram[e]++;
			++i;
		} while (i != stop);
	}
	return count;
}

bool	ClipDEMToFaceSet(const set<GISFace *>& inFaces, const DEMGeo& inSrcDEM, DEMGeo& inDstDEM, int& outX1, int& outY1, int& outX2, int& outY2)
{
	set<GISHalfedge *>		allEdges, localEdges;
	for (set<GISFace *>::const_iterator f = inFaces.begin(); f != inFaces.end(); ++f)
	{
		localEdges.clear();
		FindEdgesForFace(*f, localEdges);
		for (set<GISHalfedge *>::iterator e = localEdges.begin(); e != localEdges.end(); ++e)
		{
			if (inFaces.count((*e)->face()) == 0 || inFaces.count((*e)->twin()->face()) == 0)
				allEdges.insert(*e);
		}
	}
	
	PolyRasterizer	rast;
	int x, y;
	y = SetupRasterizerForDEM(allEdges, inSrcDEM, rast);

	outX1 = inSrcDEM.mWidth;
	outX2 = 0;
	outY1 = inSrcDEM.mHeight;
	outY2 = 0;
	bool ok = false;
	
	rast.StartScanline(y);
	while (!rast.DoneScan())
	{
		int x1, x2;
		while (rast.GetRange(x1, x2))
		{
			for (x = x1; x < x2; ++x)
			{
				outX1 = min(outX1, x);
				outX2 = max(outX2, x+1);
				outY1 = min(outY1, y);
				outY2 = max(outY2, y+1);
				ok = true;
				
				if (inSrcDEM.get(x,y) != DEM_NO_DATA)
					inDstDEM(x,y) = inSrcDEM(x,y);
			}
		}
		++y;
		if (y >= inSrcDEM.mHeight) break;
		rast.AdvanceScanline(y);		
	}
	if (outX1 < 0)	outX1 = 0;
	if (outY1 < 0)	outY1 = 0;
	if (outX2 > inDstDEM.mWidth) outX2 = inDstDEM.mWidth;
	if (outY2 > inDstDEM.mHeight) outY2 = inDstDEM.mHeight;
	return ok;
}

int		SetupRasterizerForDEM(const Pmwx::Face_handle f, const DEMGeo& dem, PolyRasterizer& rasterizer)
{
	set<GISHalfedge *>	all, useful;
	FindEdgesForFace(f, all);
	for (set<GISHalfedge *>::iterator e = all.begin(); e != all.end(); ++e)
	{
		if ((*e)->face() != (*e)->twin()->face())
			useful.insert(*e);
	}
	return SetupRasterizerForDEM(useful, dem, rasterizer);
}

int		SetupRasterizerForDEM(const set<GISHalfedge *>& inEdges, const DEMGeo& dem, PolyRasterizer& rasterizer)
{
	for (set<GISHalfedge *>::const_iterator e = inEdges.begin(); e != inEdges.end(); ++e)
	{
		double x1 = dem.lon_to_x((*e)->source()->point().x);
		double y1 = dem.lat_to_y((*e)->source()->point().y);
		double x2 = dem.lon_to_x((*e)->target()->point().x);
		double y2 = dem.lat_to_y((*e)->target()->point().y);

		if (y1 != y2)
		{
			if (y1 < y2)
				rasterizer.masters.push_back(PolyRasterSeg_t(x1,y1,x2,y2));
			else
				rasterizer.masters.push_back(PolyRasterSeg_t(x2,y2,x1,y1));
		}
	}					

	rasterizer.SortMasters();
	
	if (rasterizer.masters.empty())
		 return 0;
	return floor(rasterizer.masters.front().y1);
}

/************************************************************************************************
 * POLYGON TRUNCATING AND EDITING
 ************************************************************************************************/
#pragma mark -

/*
 * WARNING WARNING WARNING: This routine's implementation is INCORRECT and UNRELIABLE.
 * The SK_Skeleton APIs are designed to provide correct straight-skeleton-based inset
 * calculations; InsetPmwx is an old attempt to write an inset routine without proper
 * skeleton treatmant and should not be used!
 */

static void __Inset_CollectNotifier(GISHalfedge * he_old, GISHalfedge * he_new, void * ref)
{
	set<GISHalfedge *> * edges = (set<GISHalfedge *> *) ref;
	if (he_old && he_new)
	{
		if (edges->count(he_old) > 0) edges->insert(he_new);
		if (edges->count(he_old->twin()) > 0) edges->insert(he_new->twin());
	} else {
		if (he_old) edges->insert(he_old);
		if (he_new) edges->insert(he_new);
	}
}

static void __AntennaFunc(int n, void * ref)
{
	vector<bool> *	is_new = (vector<bool>*) ref;
	is_new->insert(is_new->begin()+n, true);
}

static bool __crunch_edge(Pmwx& ioMap, GISHalfedge * e, GISHalfedge * s)
{
	bool nuke = false;
	while (1)
	{
		GISHalfedge * v = e->next();
		if (v != e->twin() &&					// Can't be the end - I'd nuke myself
			v->twin()->next() == e->twin())	// Back side must be direct too - means we're the only ones here
		{
			if (v == s) return nuke;
			Vector2	v1(e->source()->point(), e->target()->point());
			Vector2	v2(v->source()->point(), v->target()->point());
			v1.normalize();
			v2.normalize();
			if (v1.dot(v2) > 0.9999)
			{
				ioMap.merge_edges(e, v);
				nuke = true;
			} else
				return nuke;
		} else
			return nuke;			
	}
}

// This is mapped FROM the vertices of the face...
// but defined NOT by a vertex as a key but by TWO halfedges!
// where one follows the other.  We key by the original incoming
// edge!
struct	he_buffer_point {
	GISHalfedge *		prev;
	GISHalfedge *		next;	
	GISVertex *			orig;		// 
	int					cap;
	int					split;
	Point2				p1;
	Point2				p2;
	GISHalfedge *		v1;
	GISHalfedge *		v2;
	GISHalfedge *		span;
};

struct	he_buffer_struct {
	he_buffer_struct(he_buffer_point * iv1, he_buffer_point * iv2,GISHalfedge * iorig) : v1(iv1), v2(iv2), orig(iorig), p(NULL) { }
	he_buffer_point *	v1;
	he_buffer_point *	v2;
	GISHalfedge	*		orig;
	GISHalfedge *		p;
	int					flipped;
};

typedef multimap<GISHalfedge *,GISHalfedge *>		HalfedgeSplitMap;
typedef	vector<GISHalfedge *>						InsertVector;
typedef	pair<HalfedgeSplitMap*,InsertVector*>		SplitNotifInfo;

static void split_notif(GISHalfedge * e1, GISHalfedge * e2, void * r)
{
	SplitNotifInfo * m = (SplitNotifInfo *) r;
	if (e1 && e2)
	{
		m->first->insert(HalfedgeSplitMap::value_type(e1,e2));
		m->first->insert(HalfedgeSplitMap::value_type(e1->twin(),e2->twin()));
	} else if (e1)
		m->second->push_back(e1);
	else if (e2)
		m->second->push_back(e2);
}

GISHalfedge * insert_into_map_proxy(Pmwx& io_map, const Point2& p1, const Point2& p2, HalfedgeSplitMap * mapping)
{
	SplitNotifInfo s;
	InsertVector v;
	s.first = mapping;
	s.second = &v;
	io_map.insert_edge(p1,p2,split_notif,&s);
	for (int n = 1; n < v.size(); ++n)
	{
		mapping->insert(HalfedgeSplitMap::value_type(v[0],v[n]));
		mapping->insert(HalfedgeSplitMap::value_type(v[0]->twin(),v[n]->twin()));
	}
	return v[0];
}

void	calc_vertex_ext(he_buffer_point * hbp, double dist)
{
	Segment2	s1(hbp->prev->source()->point(),hbp->prev->target()->point());
	Segment2	s2(hbp->next->source()->point(),hbp->next->target()->point());
	Vector2		v1(s1.p1,s1.p2);
	Vector2		v2(s2.p1,s2.p2);
	
	if (v1.right_turn(v2) && v1.dot(v2) < 0.8)	hbp->cap = true;

	if (hbp->cap)
	{
		hbp->split = true;
		v1.normalize();
		v2.normalize();
		Vector2		vc(v1-v2);
		vc.normalize();
		vc *= dist;
		v1 = v1.perpendicular_ccw();
		v2 = v2.perpendicular_ccw();
		v1 *= dist;
		v2 *= dist;
		
		s1.p1 += v1;
		s1.p2 += v1;
		s2.p1 += v2;
		s2.p2 += v2;

		Point2 me(hbp->prev->target()->point());

		
		Line2	line1(s1);
		Line2	line2(s2);
		Line2	cross_bar(me+vc,vc.perpendicular_ccw());

		if (!line1.intersect(cross_bar,hbp->p1))
			DebugAssert(!"Mitre 1 failed.");
		
		if (!line2.intersect(cross_bar,hbp->p2))
			DebugAssert(!"Mitre 2 failed.");
	}
	else
	{
		v1 = v1.perpendicular_ccw();
		v2 = v2.perpendicular_ccw();
		v1.normalize();
		v2.normalize();
		
		if (v1.dot(v2) > 0.99)
		{
			Vector2	va(v1+v2);
			va.normalize();
			va *= dist;
			hbp->p1 = hbp->p2 = s1.p2 + va;
		} 
		else 
		{			
			v1 *= dist;
			v2 *= dist;
			
			s1.p1 += v1;
			s1.p2 += v1;
			s2.p1 += v2;
			s2.p2 += v2;
			
			if (s1.intersect(s2,hbp->p1))
			{
				hbp->split = false;
				hbp->p2 = hbp->p1;
			} else
			{
				hbp->split = true;
				hbp->p1 = s1.p2;
				hbp->p2 = s2.p1;
			}
		}
	}
}

void	extend_edge_set(set<GISHalfedge *>& in_set, set<GISHalfedge *>& out_set, HalfedgeSplitMap& mapping)
{
	out_set.clear();
	pair<HalfedgeSplitMap::iterator,HalfedgeSplitMap::iterator> range;
	
	while(!in_set.empty())
	{
		GISHalfedge * k = *in_set.begin();
		in_set.erase(k);
		out_set.insert(k);
		range = mapping.equal_range(k);
		for (HalfedgeSplitMap::iterator i = range.first; i != range.second; ++i)
		{
			DebugAssert(out_set.count(i->second)==0);
			in_set.insert(i->second);
		}
	}
}


void	InsetPmwx(GISFace * inFace, Pmwx& outMap, double dist)
{
	outMap = *inFace;
	map<GISHalfedge *,he_buffer_point>		vert_table;
	vector<he_buffer_struct>				edge_table;
	HalfedgeSplitMap						splits;	
	
	GISFace * face = (*outMap.unbounded_face()->holes_begin())->twin()->face();

	DebugAssert(!face->is_unbounded());
	outMap.unbounded_face()->mTerrainType=0;
	face->mTerrainType = 1;
	
	Pmwx::Ccb_halfedge_circulator circ, stop;
	Pmwx::Holes_iterator h;
	// FIRST process over all vertices...compute the locations of the extended vertices.
	circ = stop = face->outer_ccb();
	do {
		he_buffer_point * hbp = &vert_table[circ];
		hbp->prev = circ;
		hbp->next = circ->next();
		hbp->orig = circ->target();
		hbp->cap = circ->twin() == circ->next();
		calc_vertex_ext(hbp, dist);
		++circ;
	} while (circ != stop);
	for (h = face->holes_begin(); h != face->holes_end(); ++h)
	{
		circ = stop = *h;
		do {
			he_buffer_point * hbp = &vert_table[circ];
			hbp->orig = circ->target();
			hbp->prev = circ;
			hbp->next = circ->next();
			hbp->cap = circ->twin() == circ->next();
			calc_vertex_ext(hbp, dist);
			++circ;
		} while (circ != stop);
	}
	
	// SECOND - go over edges - make the buffer spans
	circ = stop = face->outer_ccb();
	do {
		edge_table.push_back(he_buffer_struct(
					&vert_table[circ],
					&vert_table[circ->next()],
					circ->next()));
		Vector2	v1(circ->next()->source()->point(),circ->next()->target()->point());
		Vector2	v2(edge_table.back().v1->p2,edge_table.back().v2->p1);
		edge_table.back().flipped = (v1.dot(v2) < 0);
		++circ;
	} while (circ != stop);
	for (h = face->holes_begin(); h != face->holes_end(); ++h)
	{
		circ = stop = *h;
		do {
			edge_table.push_back(he_buffer_struct(
						&vert_table[circ],
						&vert_table[circ->next()],
						circ->next()));
		Vector2	v1(circ->next()->source()->point(),circ->next()->target()->point());
		Vector2	v2(edge_table.back().v1->p2,edge_table.back().v2->p1);
		edge_table.back().flipped = (v1.dot(v2) < 0);
			++circ;
		} while (circ != stop);
	}
	
	// THIRD insert all the vertex elements
	
	for (map<GISHalfedge *,he_buffer_point>::iterator v = vert_table.begin(); v != vert_table.end(); ++v)
	{
		he_buffer_point * hbp = &v->second;
		hbp->v1 = insert_into_map_proxy(outMap,
							hbp->orig->point(),
							hbp->p1,
							&splits);
		if (hbp->split)
		hbp->v2 = insert_into_map_proxy(outMap,
							hbp->orig->point(),
							hbp->p2,
							&splits);
		else 
			hbp->v2 = hbp->v1;
	}

	// FOUR - build the cross-caps

	for (vector<he_buffer_struct>::iterator hbs = edge_table.begin(); hbs != edge_table.end(); ++hbs)
	if (!hbs->flipped)
	{
		hbs->p = insert_into_map_proxy(outMap,
							hbs->v1->p2,
							hbs->v2->p1,
							&splits);		
	}
	for (map<GISHalfedge *,he_buffer_point>::iterator v = vert_table.begin(); v != vert_table.end(); ++v)
	if (v->second.cap)
	{
		he_buffer_point * hbp = &v->second;
		hbp->span = insert_into_map_proxy(outMap,
							hbp->p1, hbp->p2,
							&splits);		
	}
	
	// FIFTH flood fill!
//	for(Pmwx::Face_iterator fi = outMap.faces_begin(); fi != outMap.faces_end(); ++fi)
//		fi->mTerrainType = 0;

	for (vector<he_buffer_struct>::iterator hbs = edge_table.begin(); hbs != edge_table.end(); ++hbs)
	if (!hbs->flipped)
	{
		set<GISHalfedge *>	reg, ext_reg;
		set<GISFace *>		faces;
		reg.insert(hbs->orig);
		reg.insert(hbs->p->twin());
		reg.insert(hbs->v1->v2->twin());
		reg.insert(hbs->v2->v1);
		
		extend_edge_set(reg,ext_reg,splits);
		FindFacesForEdgeSet(ext_reg,faces);
		for (set<GISFace *>::iterator fi = faces.begin(); fi != faces.end(); ++fi)
			(*fi)->mTerrainType = 0;
	}
	
	for (vector<he_buffer_struct>::iterator hbs = edge_table.begin(); hbs != edge_table.end(); ++hbs)
	if (hbs->flipped)
	{
		hbs->orig->face()->mTerrainType = 0;
	}	

	for (map<GISHalfedge *,he_buffer_point>::iterator v = vert_table.begin(); v != vert_table.end(); ++v)
	if (v->second.cap)
	{
		he_buffer_point * hbp = &v->second;
		set<GISHalfedge *>	reg, ext_reg;
		set<GISFace *>		faces;
		reg.insert(hbp->v1->twin());
		reg.insert(hbp->v2);
		reg.insert(hbp->span->twin());
		
		extend_edge_set(reg,ext_reg,splits);
		FindFacesForEdgeSet(ext_reg,faces);
		for (set<GISFace *>::iterator fi = faces.begin(); fi != faces.end(); ++fi)
			(*fi)->mTerrainType = 0;
	}

	// SIXTH - kill unneeded edges!
	set<GISHalfedge *>	dead_list;
	for (Pmwx::Halfedge_iterator he = outMap.halfedges_begin(); he != outMap.halfedges_end(); ++he)
	if (he->mDominant)
	if (he->face()->mTerrainType == he->twin()->face()->mTerrainType)
		dead_list.insert(he);
		
	for (set<GISHalfedge *>::iterator d=dead_list.begin(); d != dead_list.end(); ++d)
		outMap.remove_edge(*d);
	
}

inline bool Near(double x, double y)
{
	return fabs(x-y) < 0.00001;
}

inline bool IsOnBox(const Point2& p1, const Point2& p2, const Bbox2& box)
{
	if (Near(p1.x , p2.x ) && Near(p1.x , box.p1.x)) return true;
	if (Near(p1.x , p2.x ) && Near(p1.x , box.p2.x)) return true;

	if (Near(p1.y , p2.y ) && Near(p1.y , box.p1.y)) return true;
	if (Near(p1.y , p2.y ) && Near(p1.y , box.p2.y)) return true;
	return false;
}
		
void UnmangleBorder(Pmwx& ioMap)
{
/*	for (Pmwx::Holes_iterator hole = ioMap.unbounded_face()->holes_begin(); hole != ioMap.unbounded_face()->holes_end(); ++hole)
	{
		printf("Hole:\n");
		Pmwx::Ccb_halfedge_circulator iter, stop;
		iter = stop = *hole;
		do {
			printf("  %lf,%lf  valence = %d\n", iter->target()->point().x,iter->target()->point().y, iter->target()->degree());
			++iter;
		} while (iter != stop);
	} */
	
	set <GISHalfedge *>	dead;
	
	Bbox2	box;
	CalcBoundingBox(ioMap, box.p1, box.p2);
	
	for (Pmwx::Halfedge_iterator he = ioMap.halfedges_begin(); he != ioMap.halfedges_end(); ++he)
	if (he->mDominant)
	if (he->face() != ioMap.unbounded_face())
	if (he->twin()->face() != ioMap.unbounded_face())
	if (IsOnBox(he->source()->point(), he->target()->point(), box))
		dead.insert(he);

	printf("Will kill %d segments.\n", dead.size());
	for (set<GISHalfedge *>::iterator d = dead.begin(); d != dead.end(); ++d)
	{
		// Ben says: not sure why this assert was here...well, I am actually, it's to try to make sure we're not
		// changing the fundamental topology of the import...but there are zero length segments that DO change the 
		// import. :-(  WTF is wrong with VMAP0?
//		DebugAssert((*d)->face()->IsWater() == (*d)->twin()->face()->IsWater());
		ioMap.remove_edge(*d);
	}

}