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
#include "NetPlacement.h"
#include "NetTables.h"
#include "GISUtils.h"
#include "ParamDefs.h"
#include "XESConstants.h"
#include "CompGeomDefs3.h"
#include "MeshAlgs.h"
#include "MapDefs.h"

bool	HalfedgeIsSeparated(Pmwx::Halfedge_handle he)
{
	if (!he->mDominant) he = he->twin();
	for (GISNetworkSegmentVector::iterator seg = he->mSegments.begin(); seg != he->mSegments.end(); ++seg)
	{
		if (IsSeparatedHighway(seg->mFeatType)) return true;
	}
	return false;
}

void	MakeHalfedgeOneway(Pmwx::Halfedge_handle he)
{
	if (!he->mDominant) he = he->twin();
	for (GISNetworkSegmentVector::iterator seg = he->mSegments.begin(); seg != he->mSegments.end(); ++seg)
	{
		seg->mFeatType = SeparatedToOneway(seg->mFeatType);
	}
}

void	CalcRoadTypes(Pmwx& ioMap, const DEMGeo& inElevation, const DEMGeo& inUrbanDensity, ProgressFunc inProg)
{
	double	total = ioMap.number_of_faces() + ioMap.number_of_halfedges();
	int		ctr = 0;
	
	if (inProg) inProg(0, 1, "Calculating Road Types", 0.0);
	
	for (Pmwx::Face_iterator face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->IsWater())
	{
		if (inProg && (ctr % 1000) == 0) inProg(0, 1, "Calculating Road Types", (double) ctr / total);

		Pmwx::Ccb_halfedge_circulator iter, stop, last;
		// This isn't totally trivial...there are basically two cases we want to look at:
		// For the outer CCB of this face, if there is at least two discontinuities in
		// separation, that means we have separated highways.  One discontinuity isn't quite 
		// enough because it can be one chain.
		
		int		has_unsep = 0;
		int 	xons = 0;
		bool	needs_processing = false;

		xons = 0;
		last = iter = stop = face->outer_ccb();
		do {
			last = iter;
			++iter;			
			if (HalfedgeIsSeparated(iter) != HalfedgeIsSeparated(last))
				++xons;
		} while (iter != stop);
		if (xons >= 4) needs_processing = true;
		if (xons > 0) has_unsep++;
		
		for (Pmwx::Holes_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
		{
			if (needs_processing) break;
			last = iter = stop = *hole;
			xons = 0;
			do {
				last = iter;
				++iter;			
				if (HalfedgeIsSeparated(iter) != HalfedgeIsSeparated(last))
					++xons;
			} while (iter != stop);
			if (xons >= 4) needs_processing = true;
			if (xons > 0) has_unsep++;
			if (has_unsep > 1) needs_processing = true;
		}
		
		if (needs_processing)
		{
			iter = stop = face->outer_ccb();
			do {
				MakeHalfedgeOneway(iter);
				++iter;
			} while (iter != stop);
			for (Pmwx::Holes_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
			{
				iter = stop = *hole;
				do {
					MakeHalfedgeOneway(iter);
					++iter;
				} while (iter != stop);
			}
		
		}
	}
	
	for (Pmwx::Halfedge_iterator edge = ioMap.halfedges_begin(); edge != ioMap.halfedges_end(); ++edge, ++ctr)
	if (edge->mDominant)
	{
		if (inProg && (ctr % 1000) == 0) inProg(0, 1, "Calculating Road Types", (double) ctr / total);
	
		double	x1 =edge->source()->point().x;
		double	y1 =edge->source()->point().y;
		double	x2 =edge->target()->point().x;
		double	y2 =edge->target()->point().y;
		double	startE = inElevation.value_linear(x1,y1);
		double	endE = inElevation.value_linear(x2, y2);
		double	urbanS = inUrbanDensity.value_linear(x1, y1);
		double	urbanE = inUrbanDensity.value_linear(x2, y2);
		
		double	dist = LonLatDistMeters(x1,y1,x2,y2);
		if (dist <= 0.0) continue;
		
		double gradient = fabs(startE - endE) / dist;
		double urban = (urbanS + urbanE) * 0.5;
		
		for (GISNetworkSegmentVector::iterator seg = edge->mSegments.begin(); seg != edge->mSegments.end(); ++seg)
		{
			// GRADIENT BRIDGES ARE TURNED OFF!!  We do NOT have the calculations 
			// to do these right. :-(
//			bool bridge = false; // gradient > gNetFeatures[seg->mFeatType].max_gradient;
//			if (edge->face()->IsWater() && edge->twin()->face()->IsWater())
//				bridge = true;
			seg->mRepType = NO_VALUE;
			for (Road2NetInfoTable::iterator p = gRoad2Net.begin(); p != gRoad2Net.end(); ++p)
			{
				if (p->first == seg->mFeatType && 
//					bridge == p->second.bridge &&
					urban >= p->second.min_density &&
					urban <= p->second.max_density)
				{
					seg->mRepType = p->second.entity_type;
				}
			}			
		}
	}

	if (inProg) inProg(0, 1, "Calculating Road Types", 1.0);	
}

/******************************************************************************************
 * FORMING NETWORK TOPOLOGY FROM GT-POLYGONS
 ******************************************************************************************/
#pragma mark -

void	Net_ChainInfo_t::reverse(void)
{
	swap(start_junction, end_junction);
	for (int n = 0; n < shape.size() / 2; ++n)
	{
		swap(shape[n], shape[shape.size()-n-1]);
		swap(ground[n], ground[shape.size()-n-1]);
	}
}

int				Net_ChainInfo_t::pt_count(void)
{
	return	shape.size() + 2;
}

int				Net_ChainInfo_t::seg_count(void)
{
	return shape.size() + 1;
}

Point3		Net_ChainInfo_t::nth_pt(int n)
{
	if (n == 0) return start_junction->location;
	if (n > shape.size()) return end_junction->location;
	return shape[n-1];
}

double		Net_ChainInfo_t::nth_ground(int n)
{
	if (n == 0) return start_junction->ground;
	if (n > shape.size()) return end_junction->ground;
	return ground[n-1];
}

Segment3		Net_ChainInfo_t::nth_seg(int n)
{
	return Segment3(nth_pt(n), nth_pt(n+1));
}

Vector3		Net_ChainInfo_t::vector_to_junc(Net_JunctionInfo_t * junc)
{
	if (junc == start_junction)
		return Vector3(nth_pt(1), nth_pt(0));
	else
		return Vector3(nth_pt(pt_count()-2),nth_pt(pt_count()-1));
}

Vector2		Net_ChainInfo_t::vector_to_junc_flat(Net_JunctionInfo_t * junc)
{
	Vector3 foo = vector_to_junc(junc);
	return Vector2(foo.dx, foo.dy);
}

Net_JunctionInfo_t *	Net_ChainInfo_t::other_junc(Net_JunctionInfo_t * junc)
{
	return (junc == start_junction) ? end_junction : start_junction;
}


// This routine takes a network and combines chains that are contiguous through a junction, reducing
// junctions and forming longer chains.  This routien will not remove a junction that is colocated
// with another jucntion unless compact_colocated is true.
void	OptimizeNetwork(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& outChains)	//, bool compact_colocated)
{
	int	total_merged = 0;
	int total_removed = 0;
	
	// First go through and consider every jucntion...if it has 2 items and they're the same and not a 
	// loop, we can coalesce.  Look out for a colocated junctions.
	for (Net_JunctionInfoSet::iterator junc = ioJunctions.begin(); junc != ioJunctions.end(); ++junc)
	{
		Net_JunctionInfo_t * me = (*junc);
		if (me->chains.size() == 2)
//		if (compact_colocated || me->colocated.empty())
		{
			Net_ChainInfoSet::iterator i = me->chains.begin();
			Net_ChainInfo_t * sc = *i;
			++i;
			Net_ChainInfo_t * ec = *i;
			
			if (sc != ec && sc->entity_type == ec->entity_type && 
				sc->start_junction != sc->end_junction &&
				ec->start_junction != ec->end_junction &&
				sc->other_junc(me) != ec->other_junc(me))
			{
				// Organize so start chain feeds into end chain directionally, with
				// us as the middle junction.
				if (sc->end_junction != me)		sc->reverse();
				if (ec->start_junction != me)	ec->reverse();
				if (sc->end_junction != me)		
					printf("Topology error.\n");
				if (ec->start_junction != me)
					printf("Topology error.\n");				

				// These junctions cap the new complete chain.
				Net_JunctionInfo_t * sj = sc->start_junction;
				Net_JunctionInfo_t * ej = ec->end_junction;		
				if (sj == me)
					printf("Topology error.\n");		
				if (ej == me)
					printf("Topology error.\n");		
				// Accumulate all shape points.
				sc->shape.push_back(me->location);
				sc->ground.push_back(me->ground);
				sc->shape.insert(sc->shape.end(), ec->shape.begin(),ec->shape.end());
				sc->ground.insert(sc->ground.end(), ec->ground.begin(),ec->ground.end());
				// We no longer have points.
				me->chains.clear();
				// End junction now has first chain instead of second.
				ej->chains.erase(ec);
				ej->chains.insert(sc);
				// Start chain now ends at end junction, not us.
				sc->end_junction = ej;
				// Nuke second chain, it's useless.
				delete ec;
				outChains.erase(ec);
				++total_merged;
			}
		}
	}
	
	// Now go through and take out the trash...schedule for deletion every 0-valence
	// junction and also undo the colocation loops.
	Net_JunctionInfoSet	deadJuncs;
	for (Net_JunctionInfoSet::iterator junc = ioJunctions.begin(); junc != ioJunctions.end(); ++junc)
	{
		if ((*junc)->chains.empty())
		{
//			for (Net_JunctionInfoSet::iterator colo = (*junc)->colocated.begin(); colo != (*junc)->colocated.end(); ++colo)
//				(*colo)->colocated.erase(*junc);
			deadJuncs.insert(*junc);
			++total_removed;
		}
	}

	// Finally do the actual deletion.
	for (Net_JunctionInfoSet::iterator junc = deadJuncs.begin(); junc != deadJuncs.end(); ++junc)
	{
		ioJunctions.erase(*junc);
		delete (*junc);
	}	
	printf("Optimize: %d merged, %d removed.\n", total_merged, total_removed);
}

#define REDUCE_SHAPE_ANGLE 	0.984807753012208	// 0.9961946980917455
#define MAX_CUT_DIST		2000

int	NukeStraightShapePoints(Net_ChainInfoSet& ioChains)
{
	int reduces = 0;
	for (Net_ChainInfoSet::iterator i = ioChains.begin(); i != ioChains.end(); ++i)
	{
		Net_ChainInfo_t * c = *i;
		for (int v = 0; v < c->shape.size();)
		{
			Point3 * p1, * p2, * p3;
			p2 = &c->shape[v];
			if (v == 0)
				p1 = &c->start_junction->location;
			else
				p1 = &c->shape[v-1];
			if (v == (c->shape.size()-1))
				p3 = &c->end_junction->location;
			else
				p3 = &c->shape[v+1];
			Vector3 v1(*p1, *p2);
			Vector3 v2(*p2, *p3);
			
			v1.dx *= (DEG_TO_MTR_LAT * cos(p2->y * DEG_TO_RAD));
			v2.dx *= (DEG_TO_MTR_LAT * cos(p2->y * DEG_TO_RAD));
			v1.dy *= (DEG_TO_MTR_LAT);
			v2.dy *= (DEG_TO_MTR_LAT);
			double	d1 = sqrt(v1.dot(v1));
			double	d2 = sqrt(v2.dot(v2));
			v1.normalize();
			v2.normalize();
			double ang = v1.dot(v2);
			if (ang > REDUCE_SHAPE_ANGLE && d1 < MAX_CUT_DIST && d2 < MAX_CUT_DIST)
			{
				c->shape.erase(c->shape.begin()+v);
				c->ground.erase(c->ground.begin()+v);
				++reduces;
			} else
				++v;
		}
	}
	return reduces;
}

void	BuildNetworkTopology(Pmwx& inMap, Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains)
{
	outJunctions.clear();
	outChains.clear();

	typedef	map<Pmwx::Vertex_handle, Net_JunctionInfo_t*>		JunctionTableType;
	typedef	map<Pmwx::Halfedge_handle, Net_ChainInfo_t*>		ChainTableType;
	JunctionTableType												junctionTable;

	/************ STEP 1 - BUILD THE BASIC NETWORK ************/
	for (Pmwx::Vertex_iterator v = inMap.vertices_begin(); v != inMap.vertices_end(); ++v)
	{
		Net_JunctionInfo_t * junc = new Net_JunctionInfo_t;
		junc->location.x = v->point().x;
		junc->location.y = v->point().y;
		junc->location.z = 0.0;
//		if (junc->location.x < -119.0 || junc->location.x > -116.0 || junc->location.y < 33.0 || junc->location.y > 36.0)
//			printf("Bad vertex!\n");
		junctionTable.insert(JunctionTableType::value_type(v,junc));
		outJunctions.insert(junc);
	}
	for (Pmwx::Halfedge_iterator e = inMap.halfedges_begin(); e != inMap.halfedges_end(); ++e)
	if (e->mDominant)
	for (GISNetworkSegmentVector::iterator seg = e->mSegments.begin(); seg != e->mSegments.end(); ++seg)
	if (seg->mRepType != NO_VALUE)
	{
		Net_ChainInfo_t *	chain = new Net_ChainInfo_t;
		chain->entity_type = seg->mRepType;
		chain->over_water = e->face()->IsWater() || e->twin()->face()->IsWater();
		chain->start_junction = junctionTable[e->source()];
		chain->end_junction = junctionTable[e->target()];
		chain->start_junction->chains.insert(chain);
		chain->end_junction->chains.insert(chain);
		outChains.insert(chain);
	}

	// Do one initial compaction - odds are we have a lot of chains that need to be built up!

	OptimizeNetwork(outJunctions, outChains);	//, true);
	
	CountNetwork(outJunctions, outChains);
	int nukes = NukeStraightShapePoints(outChains);
	CountNetwork(outJunctions, outChains);
	printf("Nuked %d shape points.\n", nukes);
	
	// Now we need to start sorting out the big mess we have - we have a flat topology and in reality
	// highways don't work that way!	
}

void	CleanupNetworkTopology(Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains)
{
	for (Net_JunctionInfoSet::iterator i = outJunctions.begin(); i != outJunctions.end(); ++i)
		delete (*i);

	for (Net_ChainInfoSet::iterator j = outChains.begin(); j != outChains.end(); ++j)
		delete (*j);
}

void	ValidateNetworkTopology(Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains)
{
	Net_JunctionInfoSet::iterator ji;
	Net_ChainInfoSet::iterator ci;
	for (ji = outJunctions.begin(); ji != outJunctions.end(); ++ji)
	{
		for (ci = (*ji)->chains.begin(); ci != (*ji)->chains.end(); ++ci)
		{
			if ((*ci)->start_junction != (*ji) && (*ci)->end_junction != (*ji))
				printf("VALIDATION ERR - junction has a chain not pointing back at the junction.\n");				
		}
	}
	for (ci = outChains.begin(); ci != outChains.end(); ++ci)
	{
		if ((*ci)->start_junction->chains.find(*ci) == (*ci)->start_junction->chains.end())
			printf("VALIDATION ERROR - chain refers to a junction not pointing back at the chain.\n");
		if ((*ci)->end_junction->chains.find(*ci) == (*ci)->end_junction->chains.end())
			printf("VALIDATION ERROR - chain refers to a junction not pointing back at the chain.\n");
	}
}

void	CountNetwork(const Net_JunctionInfoSet& inJunctions, const Net_ChainInfoSet& inChains)
{
	int juncs = inJunctions.size();
	int chains = 0;
	int segs = 0;
	for (Net_ChainInfoSet::const_iterator i = inChains.begin(); i != inChains.end(); ++i)
	{
		segs += (1+(*i)->shape.size());
		chains++;
	}
	printf("Junctions: %d, Chains: %d, Segs: %d\n", juncs, chains, segs);
}

#if 0
void	MakeHappyBridges(Net_ChainInfoSet& outChains)
{
	return;
	for (Net_ChainInfoSet::iterator happyBridge = outChains.begin(); happyBridge != outChains.end(); ++happyBridge)
	{
		Net_ChainInfo_t * chain = *happyBridge;
		if (gBridgeTypes.count(chain->entity_type))
		if (!chain->shape.empty())
		{
			double	len = 0;
			int n;
			for (n = 0; n <= chain->shape.size(); ++n)
			{
				Point3 p1 = chain->nth_pt(n);
				Point3 p2 = chain->nth_pt(n+1);
				len += LonLatDistMeters(p1.x, p1.y, p2.x, p2.y);
			}
			
			double	max_height = 80.0;
			double	hlen = 0.5 * len;

//			printf("Bridge len is %lf, max height is %lf, half len is %lf\n",				len, max_height, hlen);			
			
			double	height;
			double	i = 0;
			for (n = 0; n < chain->shape.size(); ++n)
			{
				Point3 p1 = chain->nth_pt(n);
				Point3 p2 = chain->nth_pt(n+1);
				i += LonLatDistMeters(p1.x, p1.y, p2.x, p2.y);
				if (i < hlen)
				{
					height = i / 14.0;					
				} else {
					height = (len - i) / 14.0;
				}
				if (height > max_height) height = max_height;
//				printf("Setting point %d (%lf along) to %lf\n", n, i, height);
				chain->shape[n].z += height;
			}
		}
	}
}
#endif

void	DrapeRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, CDT& inMesh)
{
	int total = 0;
	int added = 0;
	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	{
		Net_ChainInfo_t *	chain = *chainIter;
		
		vector<Point3>	all_pts, these_pts;
		
		CDT_MarchOverTerrain_t	info;
		
		for (int n = 0; n < chain->pt_count(); ++n)
		{
			++total;
			Point3 pt(chain->nth_pt(n));
			if (n == 0)
			{
				MarchHeightStart(inMesh, CDT::Point(pt.x, pt.y), info);
			}
			else 
			{
				MarchHeightGo(inMesh, CDT::Point(pt.x, pt.y), info, these_pts);
				total += (these_pts.size()-1);
				added += (these_pts.size()-2);
				DebugAssert(!these_pts.empty());
				if (all_pts.empty())	all_pts.swap(these_pts);
				else					all_pts.insert(all_pts.end(), ++these_pts.begin(), these_pts.end());
			}
		}
		chain->start_junction->location = all_pts.front();
		chain->end_junction->location = all_pts.back();
		chain->shape.clear();
		all_pts.erase(all_pts.begin());
		all_pts.pop_back();
		chain->shape.swap(all_pts);
		
		chain->start_junction->ground = chain->start_junction->location.z;
		chain->end_junction->ground = chain->end_junction->location.z;
		
		chain->ground.resize(chain->shape.size());
		for (int m = 0; m < chain->shape.size(); ++m)
			chain->ground[m] = chain->shape[m].z;
	}
	printf("Total of %d road pts, %d were added for mesh.\n", total, added);
}

bool	CanMerge(Net_ChainInfo_t * road1, Net_ChainInfo_t * road2, double angle)
{
	if (angle > 0.7) return true;
	if (gNetEntities[road1->entity_type].limited_access) return false;
	if (gNetEntities[road2->entity_type].limited_access) return false;
	return true;
}

void	VerticalPartitionRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	Net_JunctionInfoSet	new_juncs;
	for (Net_JunctionInfoSet::iterator juncIter = ioJunctions.begin(); juncIter != ioJunctions.end(); ++juncIter)
	if ((*juncIter)->chains.size() > 2)
	{
		Net_JunctionInfo_t * junc = *juncIter;
		
		vector<Net_ChainInfoSet>	layers;
		
		for (Net_ChainInfoSet::iterator chainIter = junc->chains.begin(); chainIter != junc->chains.end(); ++chainIter)
		{
			int 	best_layer = layers.size();
			double	best_angle = 0.0;
			
			Vector2	this_vec = (*chainIter)->vector_to_junc_flat(junc);
			this_vec.normalize();
			
			for (int l = 0; l < layers.size(); ++l)
			for (Net_ChainInfoSet::iterator posMerge = layers[l].begin(); posMerge != layers[l].end(); ++posMerge)
			{
				Vector2	merge_vec = (*posMerge)->vector_to_junc_flat(junc);
				merge_vec.normalize();
				double	our_angle = fabs(this_vec.dot(merge_vec));
				if (CanMerge(*chainIter, *posMerge, our_angle))
				if (our_angle > best_angle)
				{
					best_angle = our_angle;
					best_layer = l;
				}
			}
			
			if (best_layer == layers.size())
				layers.push_back(Net_ChainInfoSet());

			layers[best_layer].insert(*chainIter);
		}
		
		for (int new_layer = 1; new_layer < layers.size(); ++new_layer)
		{
			Net_JunctionInfo_t * new_junc = new Net_JunctionInfo_t;
			new_juncs.insert(new_junc);
			new_junc->location = junc->location;
			new_junc->ground = junc->ground;
			new_junc->location.z += (5.0 * new_layer);
			for (Net_ChainInfoSet::iterator migrate = layers[new_layer].begin(); migrate != layers[new_layer].end(); ++migrate)
			{
				new_junc->chains.insert(*migrate);
				junc->chains.erase(*migrate);
				if ((*migrate)->start_junction == junc) (*migrate)->start_junction = new_junc;
				if ((*migrate)->end_junction == junc) (*migrate)->end_junction = new_junc;
			}
		}
	}
	
	ioJunctions.insert(new_juncs.begin(),new_juncs.end());	
}
