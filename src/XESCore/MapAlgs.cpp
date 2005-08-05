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
#include <CGAL/polygon_2_algorithms.h>
#include "AssertUtils.h"
#include "CompGeomUtils.h"
#include "PolyRasterUtils.h"
#include "DEMDefs.h"

// Show all ideal insert lines for an inset!
#define DEBUG_SHOW_INSET 1

#if DEBUG_SHOW_INSET
#include "WED_Globals.h"
#endif
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
	
	// Ben sez: the set of halfedges may be disjoint or have a pinch that stops flood-fill;
	// add ALL faces to the working set up front!
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

struct	CropProgInfo_t {
	vector<Point2>			pts;
	vector<GISHalfedge *>	edges;
};

void	ProgNotifier(GISHalfedge * o, GISHalfedge * n, void * r)
{
	CropProgInfo_t * ref = (CropProgInfo_t *) r;
	GISHalfedge * e = (n == NULL) ? o : n;
	
	if (o == NULL || n == NULL)
	if (o != NULL || n != NULL)
	{
		DebugAssert(e != NULL);
		ref->edges.push_back(e);
		ref->pts.push_back(e->target()->point());
	}
	
}



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
			
			
			
void	CropMap(
			Pmwx&					ioMap,
			Pmwx&					ioCutout,
			const vector<Point2>&	ring,			
			ProgressFunc			inFunc)
{
	CropProgInfo_t	info;
	CropProgInfo_t * ref = &info;

	for (int n = 0; n < ring.size(); ++n)
	{
		int m = (n+1)%ring.size();
		if (inFunc) inFunc(0,2,"Cutting map...", (float) n / (float) ring.size());
		
		ioMap.insert_edge(ring[n], ring[m], ProgNotifier, &info);
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

static	void	RemoveAntennasFromFace(Pmwx& inPmwx, GISFace * inFace)
{
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

struct	InduceCCBInfo_t {
	GISHalfedge *			master;
	vector<GISHalfedge *>	slave_halfedges;
	vector<Point2>			break_pts;
};	

void	InduceCCBNotifier(GISHalfedge * o, GISHalfedge * n, void * r)
{
	InduceCCBInfo_t *	ref = (InduceCCBInfo_t *) r;
	if (o && n)
	{
		
	} else {
		GISHalfedge * e = o ? o : n;
		ref->slave_halfedges.push_back(e);
		if (e->source()->point() != ref->master->source()->point())
			ref->break_pts.push_back(e->source()->point());
	}
}

static	void	InduceCCB(
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
		InduceCCBInfo_t	info;
		info.master = iter;
		if (last_slave)
			last_slave = inSlave.insert_edge(iter->source()->point(), iter->target()->point(), last_slave, Pmwx::locate_Vertex, InduceCCBNotifier, &info);
		else
			last_slave = inSlave.insert_edge(iter->source()->point(), iter->target()->point(), InduceCCBNotifier, &info);

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

void	SwapFace(
			Pmwx&			inMaster,
			Pmwx&			inSlave,
			GISFace *		inFace,
			ProgressFunc	inFunc)
{
	RemoveAntennasFromFace(inMaster, inFace);

	vector<GISHalfedge *>	ringMaster, ringSlave;
	InduceCCB(inMaster, inSlave, inFace->outer_ccb(), ringMaster, ringSlave, true);
	DebugAssert(ringMaster.size() == ringSlave.size());
	SwapMaps(inMaster, inSlave, ringMaster, ringSlave);
	
	for (Pmwx::Holes_iterator hole = inFace->holes_begin(); hole != inFace->holes_end(); ++hole)
	{
		ringMaster.clear();
		ringSlave.clear();
		InduceCCB(inSlave, inMaster, *hole, ringMaster, ringSlave, false);
		DebugAssert(ringMaster.size() == ringSlave.size());
		SwapMaps(inSlave, inMaster, ringMaster, ringSlave);		
	}
}


struct OverlayInfo_t {
	vector<GISHalfedge *>	dstRing;
	vector<GISHalfedge *>	srcRing;
	GISHalfedge *			curSrc;
	Pmwx *					curSrcMap;
};

void OverlayNotifier(GISHalfedge * o, GISHalfedge * n, void * r)
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
			inDst.insert_edge((*riter)->source()->point(), (*riter)->target()->point(), OverlayNotifier, &info);
		}

#if DEV
		DebugAssert(inSrc.is_valid());
		DebugAssert(inDst.is_valid());
#endif

//		printf("%d pts\n", info.dstRing.size());
//		for (int n = 0; n < info.dstRing.size(); ++n)
//		{
//			printf("%d Src: %lf,%lf->%lf,%lf   Dst: %lf,%lf->%lf,%lf\n", n,
//				info.srcRing[n]->source()->point().x,info.srcRing[n]->source()->point().y,
//				info.srcRing[n]->target()->point().x,info.srcRing[n]->target()->point().y,
//				info.dstRing[n]->source()->point().x,info.dstRing[n]->source()->point().y,
//				info.dstRing[n]->target()->point().x,info.dstRing[n]->target()->point().y);
//		}
//
//		if (ctr == 18) return;
		SwapMaps(inDst, inSrc, info.dstRing, info.srcRing);
	}
}



void	CleanFace(
			Pmwx&					inMap,
			Pmwx::Face_handle		inFace)
{
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

struct MergeMaps_EdgeNotifier_t {
	multimap<GISHalfedge *, GISHalfedge *> *		edgeMap;
	GISHalfedge *									srcEdge;
	map<GISVertex *, GISVertex *> *					vertMap;
};

static void MergeMaps_EdgeNotifier(GISHalfedge * he_old, GISHalfedge * he_new, void * ref)
{
	MergeMaps_EdgeNotifier_t * info = (MergeMaps_EdgeNotifier_t *) ref;

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

void MergeMaps(Pmwx& ioDstMap, Pmwx& ioSrcMap, bool inForceProps, set<GISFace *> * outFaces, bool pre_integrated)
{
//	DebugAssert(ioDstMap.is_valid());
	DebugAssert(ioSrcMap.is_valid());
	// Step 1 - we need to copy all halfedges from ioSrcMap to ioDstMap.
	// We'll remember the mappign and also copy attributes.

		multimap<GISHalfedge *, GISHalfedge *>		edgeMap;
		map<GISVertex *, GISVertex *> 				vertMap;
	MergeMaps_EdgeNotifier_t info;
	info.edgeMap = &edgeMap;
	info.vertMap = &vertMap;
	
	int ctr = 0;

	if (pre_integrated)
	{
		map<Point2, GISVertex *, lesser_y_then_x>			pt_index;
		
		for (Pmwx::Vertex_iterator v = ioDstMap.vertices_begin(); v != ioDstMap.vertices_end(); ++v)
			pt_index[v->point()] = v;

		for (Pmwx::Halfedge_iterator iter = ioSrcMap.halfedges_begin(); iter != ioSrcMap.halfedges_end(); ++iter, ++ctr)
		if (iter->mDominant)
		{
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

		int fast = 0, slow = 0;
		for (Pmwx::Halfedge_iterator iter = ioSrcMap.halfedges_begin(); iter != ioSrcMap.halfedges_end(); ++iter, ++ctr)
		if (iter->mDominant)
		{
			info.srcEdge = iter;
			map<GISVertex *, GISVertex *>::iterator hint = vertMap.find(iter->source());
			if (hint != vertMap.end()) 
			{
				++fast;
				ioDstMap.insert_edge(iter->source()->point(), iter->target()->point(), hint->second->halfedge(), Pmwx::locate_Vertex, MergeMaps_EdgeNotifier, &info);
			} else {
				++slow;
				ioDstMap.insert_edge(iter->source()->point(), iter->target()->point(), MergeMaps_EdgeNotifier, &info);
			}
		}
	}
	
	ctr = 0;

	for (Pmwx::Face_iterator fiter = ioSrcMap.faces_begin(); fiter != ioSrcMap.faces_end(); ++fiter)
	if (!fiter->is_unbounded())
	if (fiter->mAreaFeature.mFeatType != NO_VALUE || fiter->mTerrainType != terrain_Natural)
	{
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


#if 0
		if (ctr == 498) 
		{
			printf("Old borders:\n");
			for (set<GISHalfedge *>::iterator e = borders_old.begin(); e != borders_old.end(); ++e)
			{
				printf("%f,%f->%f,%f\n", (*e)->source()->point().x,(*e)->source()->point().y,(*e)->target()->point().x,(*e)->target()->point().y);
			}
			printf("New borders:\n");
			for (set<GISHalfedge *>::iterator e = borders_new.begin(); e != borders_new.end(); ++e)
			{
				printf("%f,%f->%f,%f\n", (*e)->source()->point().x,(*e)->source()->point().y,(*e)->target()->point().x,(*e)->target()->point().y);
			}
		}
#if DEV
		for (set<GISHalfedge *>::iterator e = borders_new.begin(); e != borders_new.end(); ++e)
		{
			bool	 ok = false;
			bool	 linked = false;
			for (set<GISHalfedge *>::iterator ee = borders_new.begin(); ee != borders_new.end(); ++ee)
			{
				if ((*e)->target() == (*ee)->source())
				{
					ok = true;
					break;
				}
				if ((*e)->target()->point() == (*ee)->source()->point())
				{
					linked = true;
				}
			}
			if (!ok)
			{
				printf("not ok - linked: %s\n", linked ? "yes" : "no");
				DebugAssert(!ok);
			}
			
		}
#endif	
#endif	
		
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
		++ctr;
	}
}

struct hash_vertex {
	typedef GISVertex * KeyType;
	size_t operator()(const KeyType& key) const { return (size_t) key; }
};


void	SwapMaps(	Pmwx& 							ioMapA, 
					Pmwx& 							ioMapB, 
					const vector<GISHalfedge *>&	inBoundsA,
					const vector<GISHalfedge *>&	inBoundsB)
{
	DebugAssert(inBoundsA.size() == inBoundsB.size());
	
	set<GISFace *>		moveFaceFromA, moveFaceFromB;
	set<GISHalfedge *>	moveEdgeFromA, moveEdgeFromB;
	set<GISVertex * >	moveVertFromA, moveVertFromB;
	hash_map<GISVertex *, GISVertex *, hash_vertex>	keepVertFromA, keepVertFromB;
	hash_map<GISVertex *, GISVertex *, hash_vertex>::iterator findVert;
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
		keepVertFromA.insert(hash_map<GISVertex *, GISVertex *, hash_vertex>::value_type(inBoundsA[n]->target(), inBoundsB[n]->target()));
		keepVertFromB.insert(hash_map<GISVertex *, GISVertex *, hash_vertex>::value_type(inBoundsB[n]->target(), inBoundsA[n]->target()));
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

#if DEV
//	BEN SEZ: this assert is not safe - if a vertex has two CCB halfedges pointing
//	to it this 1:1 thing gets f--cked.
//	for (n = 0; n < inBoundsA.size(); ++n)
//	{
//		DebugAssert(inBoundsA[n]->target()->halfedge() == inBoundsA[n]);
//		DebugAssert(inBoundsB[n]->target()->halfedge() == inBoundsB[n]);
//	}
#endif
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



void ReduceToWaterBodies(Pmwx& ioMap)
{
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
//		if (i == 491)
//			printf("About to go loco!\n");
		ioMap.remove_edge(*iter);
	}	
}

int SimplifyMap(Pmwx& ioMap)
{
	vector<Pmwx::Halfedge_handle>	deadList;
	
	for (Pmwx::Halfedge_iterator he = ioMap.halfedges_begin();
		he != ioMap.halfedges_end(); ++he, ++he)
	{
		Pmwx::Halfedge_handle h = he;
		if (!h->mDominant) h = h->twin();
		
		bool	iWet = h->face()->IsWater();
		bool	oWet = h->twin()->face()->IsWater();
		bool	border = h->face()->is_unbounded() || h->twin()->face()->is_unbounded();
		bool	coastline = iWet != oWet;
		bool	lu_change = h->face()->mTerrainType != h->twin()->face()->mTerrainType;
		bool	road = !h->mSegments.empty();
//		bool	stuff = h->face()->mAreaFeature.mFeatType != NO_VALUE || h->twin()->face()->mAreaFeature.mFeatType != NO_VALUE;
		bool	stuff = h->face()->mAreaFeature.mFeatType != h->twin()->face()->mAreaFeature.mFeatType ||
					(h->face()->mAreaFeature.mFeatType != NO_VALUE &&
						h->face()->mAreaFeature.mParams != h->twin()->face()->mAreaFeature.mParams);
		
		bool	river = h->mParams.find(he_IsRiver) != h->mParams.end();
		if (river && (iWet || oWet)) river = false;	// Wipe out rivers that are inside water bodies or coastlines too.

		if (!river && !stuff && !road && !coastline && !border && !lu_change)
			deadList.push_back(he);
	}
	
	int i = 0;
	for (vector<Pmwx::Halfedge_handle>::iterator iter = deadList.begin();
		iter != deadList.end(); ++iter, ++i)	
	{
		ioMap.remove_edge(*iter);
	}	
	return deadList.size();
}

int RemoveUnboundedWater(Pmwx& ioMap)
{
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

struct	sort_by_bbox_base {
	bool operator()(const Bbox2& lhs, const Bbox2& rhs) const { return lhs.ymin() < rhs.ymin(); }
};

static bool epsi_intersect(const Segment2& segA, const Segment2& segB, double epsi, Point2& cross)
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
#define	SMALL_SEG_CUTOFF	0.01
#define BBOX_SLOP			0.00001
#define NEAR_SLIVER_COLINEAR 7.7e-12

inline bool	near_colinear(const Segment2& seg, Point2& p)
{
//	printf("Colinear check: %lf, %lf->%lf, %lf with %lf,%lf\n", seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y, p.x, p.y);
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
void TopoIntegrateMaps(Pmwx * mapA, Pmwx * mapB)
{
	typedef multimap<Bbox2, GISHalfedge *, sort_by_bbox_base>		HalfedgeMap;
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
//		printf("Swapping A/B maps.\n");
		swap(mapA, mapB);
	} else {
//		printf("Not swapping maps.\n");
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
					if (near_colinear(segA, segB.p1))
					{
						did_colinear = true;
						splitA.insert(SplitMap::value_type(iterA, segB.p1));
						setA.insert(iterA);
					}
					if (near_colinear(segA, segB.p2))
					{
						did_colinear = true;
						splitA.insert(SplitMap::value_type(iterA, segB.p2));
						setA.insert(iterA);
					}
					if (near_colinear(segB, segA.p1))
					{
						did_colinear = true;
						splitB.insert(SplitMap::value_type(he_box->second, segA.p1));
						setB.insert(he_box->second);
					}
					if (near_colinear(segB, segA.p2))
					{
						did_colinear = true;
						splitB.insert(SplitMap::value_type(he_box->second, segA.p2));
						setB.insert(he_box->second);
					}
				}
				
				if (!did_colinear && segA.intersect(segB, p))	//epsi_intersect(segA, segB, NEAR_INTERSECT_DIST, p))
				{
					if (p != segA.p1 && p != segA.p2)
					{
						splitA.insert(SplitMap::value_type(iterA, p));
						setA.insert(iterA);
					}
					if (p != segB.p1 && p != segB.p2)
					{
						splitB.insert(SplitMap::value_type(he_box->second, p));
						setB.insert(he_box->second);
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
					if (near_colinear(segA, segB.p1))
					{
						did_colinear = true;				
						splitA.insert(SplitMap::value_type(iterA, segB.p1));
						setA.insert(iterA);
					}
					if (near_colinear(segA, segB.p2))
					{
						did_colinear = true;				
						splitA.insert(SplitMap::value_type(iterA, segB.p2));
						setA.insert(iterA);
					}
					if (near_colinear(segB, segA.p1))
					{
						did_colinear = true;				
						splitB.insert(SplitMap::value_type(he_box->second, segA.p1));
						setB.insert(he_box->second);
					}
					if (near_colinear(segB, segA.p2))
					{
						did_colinear = true;				
						splitB.insert(SplitMap::value_type(he_box->second, segA.p2));
						setB.insert(he_box->second);
					}
				}
				
				if (!did_colinear && segA.intersect(segB, p))
				{
					if (p != segA.p1 && p != segA.p2)
					{
						splitA.insert(SplitMap::value_type(iterA, p));
						setA.insert(iterA);
					}
					if (p != segB.p1 && p != segB.p2)
					{
						splitB.insert(SplitMap::value_type(he_box->second, p));
						setB.insert(he_box->second);
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
//			printf("Splitting A map with %lf, %lf\n", spi->second.x, spi->second.y);
#if DEV
			if (origSegA.is_vertical())
				DebugAssert(origSegA.p1.x == spi->second.x);
			if (origSegA.is_horizontal())
				DebugAssert(origSegA.p1.y == spi->second.y);
#endif				
			mapA->split_edge(subdiv, spi->second);
			subdiv = subdiv->next();
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
//			printf("Splitting B map with %lf, %lf\n", spi->second.x, spi->second.y);
#if DEV
			if (origSegB.is_vertical())
				DebugAssert(origSegB.p1.x == spi->second.x);
			if (origSegB.is_horizontal())
				DebugAssert(origSegB.p1.y == spi->second.y);
#endif				
			mapB->split_edge(subdiv, spi->second);
			subdiv = subdiv->next();
		}
	}	
}

GISFace * SafeInsertRing(Pmwx * inPmwx, GISFace * parent, const vector<Point2>& inPoints)
{
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
	
	GISHalfedge * he;	
	for (n = 0; n < inPoints.size(); ++n)
	{
		he = inPmwx->insert_edge(inPoints[n], inPoints[(n+1)%inPoints.size()], NULL, NULL);
	}
	return he->face();
}

#pragma mark -

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
				if (e != NO_DATA)
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
			if (e != NO_DATA)
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
		return NO_DATA;	
}

int	GetParamHistogram(const Pmwx::Face_handle f, const DEMGeo& dem, map<float, int>& outHistogram)
{
	PolyRasterizer	rast;
	int count = 0;
	int e;
	int y = SetupRasterizerForDEM(f, dem, rast);
//	outHistogram.clear();
	rast.StartScanline(y);
	while (!rast.DoneScan())
	{
		int x1, x2;
		while (rast.GetRange(x1, x2))
		{
			for (int x = x1; x < x2; ++x)
			{
				e = dem.get(x,y);
				if (e != NO_DATA)
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
			if (e != NO_DATA)
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
				
				if (inSrcDEM.get(x,y) != NO_DATA)
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

#pragma mark -

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

#pragma mark -

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

#pragma mark -

static void Inset_CollectNotifier(GISHalfedge * he_old, GISHalfedge * he_new, void * ref)
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

static void AntennaFunc(int n, void * ref)
{
	vector<bool> *	is_new = (vector<bool>*) ref;
	is_new->insert(is_new->begin()+n, true);
}

static bool crunch_edge(Pmwx& ioMap, GISHalfedge * e, GISHalfedge * s)
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

void	InsetPmwx(Pmwx& inMap, GISFace * inFace)
{
	/********************************************************
	 * PRE-SANITIZE
	 ********************************************************/
	// Before we try to work with this polygon, we need to clean it up a bit!
	// Keep trying until we fix no pinches.
	
	Pmwx::Ccb_halfedge_circulator circ, stop;
	
	bool did_work;
	do {
		DebugAssert(inMap.is_valid());
		did_work = false;

		// First go around and merge any pointless splits.  This keeps us from going
		// berzerk later due to false breaks in a polygon.
		for (Pmwx::Holes_iterator hole = inFace->holes_begin(); ; ++hole)
		{
			stop = circ = ((hole == inFace->holes_end()) ? inFace->outer_ccb() : *hole);
			do {
				if (crunch_edge(inMap,circ, circ))
				{
					did_work = true;
					goto retry;
				}
				++circ;
			} while (circ != stop);
			
			if (hole == inFace->holes_end())
				break;
		}

		// We are going to go over all vectors looking for tight left turns - "pinches".
		// We're going to "fix" them by just cutting them off.  In our terms, the 
		// incoming segment is the first part of the 'V' of the pinch we hit as we go
		// around our CCW border, and the outgoing is the second.  Prev is before the V
		// and Next is after.

		for (Pmwx::Holes_iterator hole = inFace->holes_begin(); ; ++hole)
		{
			stop = circ = ((hole == inFace->holes_end()) ? inFace->outer_ccb() : *hole);
			do {
				
				GISHalfedge * prev = circ;
				GISHalfedge * inc = prev->next();
				GISHalfedge * out = inc->next();
				GISHalfedge * next = out->next();
				
				// If we hit a triangle, bail out now.
				if (out->next() == inc)
				{
					DebugAssert(inc->twin() == out);
					DebugAssert(hole != inFace->holes_end());
					break;
				}
				
				// Only evaluate turns that are left and more than 90 degrees
				Vector2	incv(inc->source()->point(), inc->target()->point());
				Vector2	outv(out->source()->point(), out->target()->point());
				if (inc->twin() != out && incv.left_turn(outv) && incv.dot(outv) < 0.0)
				{
					// Calculate how much space we need for the inset, and how 
					// much we actually have.
					incv.normalize();
					outv.normalize();
					bool pinch = false;
					Segment2	outs(out->source()->point(), out->target()->point());
					Segment2	incs(inc->target()->point(), inc->source()->point());
					DebugAssert(incs.p1 == outs.p1);
					double	margin_for_long ;
					double	needed_for_long ;
					double	needed_for_short;
					if (incs.squared_length() > outs.squared_length())
					{
						margin_for_long = incs.squared_distance(outs.p2);
						needed_for_long = (double) inc->mTransition / (DEG_TO_NM_LAT * NM_TO_MTR);
						needed_for_short = (double) out->mTransition / (DEG_TO_NM_LAT * NM_TO_MTR);
						
					} else {

						margin_for_long = outs.squared_distance(incs.p2);
						needed_for_long = (double) out->mTransition / (DEG_TO_NM_LAT * NM_TO_MTR);
						needed_for_short = (double) inc->mTransition / (DEG_TO_NM_LAT * NM_TO_MTR);
					}
					needed_for_long += needed_for_short;
					needed_for_long *= needed_for_long;
					if (margin_for_long < needed_for_long)
					{
						// We have a pinch!  If inc and out are left in place, the intersection of the
						// two will be off in the middle of the poly - it'll make a mess.
						if (prev == next)
						{
							// Check closed triangle - we better be an outer CCB!
							// Just kill the face and bail.
							DebugAssert(hole == inFace->holes_end());
							inFace->mTerrainType = terrain_Water;
							return;
						}
						
						// Fixing the pinch - first we need to find the two incoming/outgoing sides.
						// if they aren't going to intersect nicely, we'll just use normals to the 
						// pinched vertices.
						
						Vector2	next_v(next->source()->point(), next->target()->point());
						Vector2	prev_v(prev->source()->point(), prev->target()->point());
						
						// Some protection: if our previous segment forms a pinch in the other
						// direction or is forming a convex angle, it's not appropriate to try
						// to use it to chop off the V - just use something perpendicular to the V.
						if (prev_v.dot(incv) < 0.0 || prev_v.left_turn(incv))
							prev_v = incv.perpendicular_ccw();
						if (next_v.dot(outv) < 0.0 || outv.left_turn(outv))
							next_v = outv.perpendicular_cw();
						
						bool	inc_ant = prev->twin() == inc;
						bool	out_ant = next->twin() == out;
						bool	inc_thin = inc->face() == inc->twin()->face();
						bool	out_thin = out->face() == out->twin()->face();
						
						// Hit computation: intersect the prev and next segments against the V.
						// This is like "if the previous road kept going, how would it form a more
						// shallow V".  The idea here is that if we have a deep V inside a shallow
						// V, preserve the shallow V; if we just cut things off, we tend to get rounding
						// which under-utilizes the pinched real-estate.  Of course if we fail to extend
						// prev and next we will just punt and cut off the V.
						
						Point2	i_hit;
						Point2	o_hit;
						Point2	mid_p;
						bool	i_on = false;		// Does next extend to the incoming segment?
						bool	o_on = false;		// Does prev extend to the outgoing segment?
						bool	has_m = false;		// Do both lines cross somewhere?
																							
						if (Line2(incs.p2, prev_v).intersect(Line2(outs), o_hit))
							o_on = outs.collinear_has_on(o_hit) && o_hit != outs.p1 && o_hit != outs.p2;
						if (Line2(outs.p2, next_v).intersect(Line2(incs), i_hit))
							i_on = incs.collinear_has_on(i_hit) && i_hit != incs.p1 && i_hit != incs.p2;
						if (i_on && o_on && Line2(incs.p2, prev_v).intersect(Line2(outs.p2, next_v), mid_p))
							has_m = true;
						
						set<GISHalfedge *>	new_e_prev, new_e_next;
						GISHalfedge * outk = out_ant ? NULL : out;
						GISHalfedge * inck = inc_ant ? NULL : inc;
						
						// We are now going to handle the four crossing cases:
						
						if (i_on && o_on && has_m)
						{
							/* CASE 1 */
							// Double intersect case.  Extending prev and next hits at some midpoint.
							// Build a smaller V into the bigger V.
							// (Note that because we require i_on and o_on, we _know_ that has_m
							// is somewhere in the pinched V - otherwise both prev and next wouldn't
							// hit the pinched V.
							inMap.insert_edge(outs.p2, mid_p, Inset_CollectNotifier, &new_e_prev);
							inMap.insert_edge(mid_p, incs.p2, Inset_CollectNotifier, &new_e_next);						
						} 
						else if (i_on && !o_on)
						{
							/* CASE 2 */
							// Next projects back into the incoming side of the V but not vice versa.  We are
							// going to insert one line from the outgoing point of V (shared with next) to
							// halfway throuhg the incoming side.  This line is colinear with next, simplifying
							// the V.
							
							// Split check - If our hit point is not incident with a vertex, pre-split.  insert_edge
							// does NOT correctly handle an insert onto a halfedge!!
							if (i_hit != incs.p1 && i_hit != incs.p2)
								inck = inMap.split_edge(inc, i_hit)->next();
	
							// Reversal check!  Next was projected to the incoming side but we do NOT know if it points
							// toward or away from the incoming side.  We have to know this - if we insert this edge from
							// the wrong side of next, we're going to get a line along next but extending beyond it -
							// another case the Pmwx will vommit all over.  So check now.
							if (Vector2(outs.p2, i_hit).dot(next_v) > 0.0)
								inMap.insert_edge(next->target()->point(), i_hit, Inset_CollectNotifier, &new_e_next);
							else
								inMap.insert_edge(outs.p2, i_hit, Inset_CollectNotifier, &new_e_next);
						} 
						else if (o_on && !i_on)
						{
							/* CASE 3 */
							// Prev projects into the outgoing side, but not vice versa.  This case exactly
							// mirrors the one above.
							if (o_hit != outs.p1 && o_hit != outs.p2)
								outk = inMap.split_edge(out, o_hit);

							if (Vector2(o_hit, incs.p2).dot(prev_v) > 0.0)
								inMap.insert_edge(o_hit, prev->source()->point(), Inset_CollectNotifier, &new_e_prev);						
							else							
								inMap.insert_edge(o_hit, incs.p2, Inset_CollectNotifier, &new_e_prev);						
						}
						else
						{
							/* CASE 4 */
							// Neither next nor prev intersect with each other.  This will happen if, for example,
							// we have a tight pinch and equal-length sides of the V, and prev and next don't point
							// any where useful.  In this case we punt and just chop the whole sucker off.
							inMap.insert_edge(outs.p2, incs.p2, Inset_CollectNotifier, &new_e_prev);
						}
						
						// Water mark - set all of our added segments to water.  We track who came from prev and
						// next to preserve variable-width insets to that the inset border is continuous.						
						for (set<GISHalfedge *>::iterator ee = new_e_prev.begin(); ee != new_e_prev.end(); ++ee)
						{
							(*ee)->twin()->mTransition = prev->mTransition;
							(*ee)->face()->mTerrainType = terrain_Water;
						}
						for (set<GISHalfedge *>::iterator ee = new_e_next.begin(); ee != new_e_next.end(); ++ee)
						{
							(*ee)->twin()->mTransition = next->mTransition;
							(*ee)->face()->mTerrainType = terrain_Water;
						}
						
						// Delete unused sides - we may need to wipe out part of the V.  This is important because
						// we want to simplify the polygon once the V has been removed.
						if (outk && !out_thin)
						{
							if (outk->twin()->face() == inFace)
								outk = outk->twin();
							DebugAssert(outk->twin()->face() != inFace);
							inMap.remove_edge(outk);
						}
						if (inck && !inc_thin)
						{
							if (inck->twin()->face() == inFace)
								inck = inck->twin();
							DebugAssert(inck->twin()->face() != inFace);
							inMap.remove_edge(inck);
						}
						
						// Now that the V is potetially gone, remove colinear sides.
						if (!inc_ant) crunch_edge(inMap,prev, next);
						if (!out_ant) crunch_edge(inMap,next->twin(), next->twin());
						
						// At this point get the hell out....we've done a lot of work - odds are our iterators are totally useless.
						did_work = true;
						DebugAssert(inMap.is_valid());
						goto retry;						
					}					
				}
				
				++circ;
			} while (circ != stop);
			
			if (hole == inFace->holes_end())
				break;
		}

retry:
		;
		
	} while (did_work);

	DebugAssert(inMap.is_valid());
	
	/********************************************************
	 * INSET CALCULATION
	 ********************************************************/
	// First we must figure out what we're going to insert.  If we insert while
	// we iterate, we'll destroy the face structure as we explore it - not good.
	
	vector<Polygon2>	  rings;	// These will be a series of inset rings.
	vector<Polygon2>	  insets;	// These will be a series of inset rings.
	vector<vector<bool> > is_news;
	
	for (Pmwx::Holes_iterator hole = inFace->holes_begin(); ; ++hole)
	{
		stop = circ = ((hole == inFace->holes_end()) ? inFace->outer_ccb() : *hole);
		rings.push_back(Polygon2());
		vector<double>	ratios;
		do {
			rings.back().push_back(circ->source()->point());
			ratios.push_back((double) circ->mTransition / (DEG_TO_NM_LAT * NM_TO_MTR));
			++circ;
		} while (circ != stop);
		
		insets.push_back(Polygon2());
		is_news.push_back(vector<bool>(rings.back().size(), false));
		InsetPolygon2(rings.back(), &*ratios.begin(), 1.0, true, insets.back(), AntennaFunc, &is_news.back());
		if (hole == inFace->holes_end())
			break;
	}
	
	set<GISHalfedge *> edges;

	// Now go through and insert all of the edges.  We'll remember the last one as a hint because 99%
	// of the time we're in a ring.  This saves us time spent doing locate_point in the pmwx.  We still
	// suffer a bit because we have to do ray shoots - these are edges that WILL intersect!

#if DEV
	try {
#endif
			
		GISHalfedge * hint = NULL;

#if DEBUG_SHOW_INSET				
		gMeshLines.clear();
#endif		
		int ctr = 0;
		int fast = 0, slow = 0;
		for (int p = 0; p < insets.size(); ++p)
		{
			int on = 0;
			for (int n = 0; n < insets[p].size(); ++n)
			{
				Segment2	inset = insets[p].side(n);
#if DEBUG_SHOW_INSET				
				gMeshLines.push_back(inset.p1);
				gMeshLines.push_back(inset.p2);
#endif
//				printf("GOING TO INSERT (S): %lf, %lf (%016llx, %016llx)\n", inset.p1.x, inset.p1.y, inset.p1.x, inset.p1.y);
//				printf("GOING TO INSERT (T): %lf, %lf (%016llx, %016llx)\n", inset.p2.x, inset.p2.y, inset.p2.x, inset.p2.y);

#if 1 || !DEBUG_SHOW_INSET
				if (hint && hint->target()->point() == inset.p1)
				{
					++fast;
					hint = inMap.insert_edge(inset.p1, inset.p2, hint, Pmwx::locate_Vertex, Inset_CollectNotifier, &edges);
				} else {
					hint = inMap.insert_edge(inset.p1, inset.p2, Inset_CollectNotifier, &edges);
					++slow;
				}
#endif				
				++ctr;			
			}
		}
//		printf("Fast: %d, Slow = %d\n", fast, slow);
		
#if DEV		
	} catch (...) {
//		gMap = inMap;
		throw;
	}	
#endif	

	// Mark the sides of the edges as wet - this voids out the space outside the inset polygon.	
	for (set<GISHalfedge *>::iterator e = edges.begin(); e != edges.end(); ++e)
		(*e)->twin()->face()->mTerrainType = terrain_Water;
		
	DebugAssert(inMap.is_valid());
}
		
