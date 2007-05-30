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

// Move a bridge N meters to simlify it
#define	BRIDGE_TURN_SIMPLIFY	20

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

Net_ChainInfo_t * Net_JunctionInfo_t::get_other(Net_ChainInfo_t * me)
{
	DebugAssert(chains.size() == 2);
	Net_ChainInfoSet::iterator iter = chains.begin();
	
	Net_ChainInfo_t * one = *iter++;
	Net_ChainInfo_t * two = *iter++;
	if (me == one) return two;
	if (me == two) return one;	
	DebugAssert(!"Not found!");
	return me;
}

double		Net_JunctionInfo_t::GetMatchingAngle(Net_ChainInfo_t * chain1, Net_ChainInfo_t * chain2)
{
	Vector3	vec1 = chain1->vector_to_junc(this);
	Vector3	vec2 = chain2->vector_to_junc(this);
	vec1.normalize();
	vec2.normalize();
	double dot = -(vec1.dot(vec2));
	if (dot >= 0.0)
	if (chain1->entity_type == chain2->entity_type)
		dot += 1.0;
	return dot;
}

void	Net_ChainInfo_t::reverse(void)
{
	swap(start_junction, end_junction);
	for (int n = 0; n < shape.size() / 2; ++n)
	{
		swap(shape[n], shape[shape.size()-n-1]);
		swap(agl[n], agl[shape.size()-n-1]);
		swap(power_crossing[n], power_crossing[shape.size()-n-1]);
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

double		Net_ChainInfo_t::nth_agl(int n)
{
	if (n == 0) return start_junction->agl;
	if (n > shape.size()) return end_junction->agl;
	return agl[n-1];
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

double		Net_ChainInfo_t::meter_length(int pt_start, int pt_stop)
{
	// 0, pts-1 gives total length, 0 1 gives first seg len
	double total = 0.0;
	
	for (int n = pt_start; n < pt_stop; ++n)
	{
		Point3 p1 = nth_pt(n  );
		Point3 p2 = nth_pt(n+1);
		total += LonLatDistMeters(p1.x, p1.y, p2.x, p2.y);
	}
	return total;
}

double		Net_ChainInfo_t::dot_angle(int ctr_pt)
{
	Point3	p1(nth_pt(ctr_pt-1));
	Point3	p2(nth_pt(ctr_pt  ));
	Point3	p3(nth_pt(ctr_pt+1));
	
	Vector2	v1(Point2(p1.x, p1.y), Point2(p2.x, p2.y));
	Vector2	v2(Point2(p2.x, p2.y), Point2(p3.x, p3.y));
	
	v1.normalize();
	v2.normalize();
	return v1.dot(v2);
}

void					Net_ChainInfo_t::split_seg(int n, double rat)
{
	Segment3	seg;
	seg.p1 = nth_pt(n  );
	seg.p2 = nth_pt(n+1);
	double	g1 = nth_agl(n  );
	double	g2 = nth_agl(n+1);
	
	shape.insert(shape.begin()+n,seg.midpoint(rat));	
	agl.insert(agl.begin()+n, g1 * (1.0-rat) + g2 * rat);
	power_crossing.insert(power_crossing.begin()+n, false);
}

#pragma mark -

// This routine takes a network and combines chains that are contiguous through a junction, reducing
// junctions and forming longer chains.  This routien will not remove a junction that is colocated
// with another jucntion unless compact_colocated is true.
void	OptimizeNetwork(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& outChains, bool water_only)
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
			
			if (sc->over_water || !water_only)
			if (sc != ec && sc->entity_type == ec->entity_type && 
						    sc->export_type == ec->export_type &&
						    sc->over_water == ec->over_water &&
				sc->start_junction != sc->end_junction &&
				ec->start_junction != ec->end_junction &&
				sc->other_junc(me) != ec->other_junc(me))
			{
				// Organize so start chain feeds into end chain directionally, with
				// us as the middle junction.
				if (sc->end_junction != me)		sc->reverse();
				if (ec->start_junction != me)	ec->reverse();
				Assert (sc->end_junction == me);	
				Assert(ec->start_junction == me);

				// These junctions cap the new complete chain.
				Net_JunctionInfo_t * sj = sc->start_junction;
				Net_JunctionInfo_t * ej = ec->end_junction;		
				Assert (sj != me);
				Assert (ej != me);
				// Accumulate all shape points.
				sc->shape.push_back(me->location);
				sc->agl.push_back(me->agl);
				sc->power_crossing.push_back(me->power_crossing);
				sc->shape.insert(sc->shape.end(), ec->shape.begin(),ec->shape.end());
				sc->agl.insert(sc->agl.end(), ec->agl.begin(),ec->agl.end());
				sc->power_crossing.insert(sc->power_crossing.end(), ec->power_crossing.begin(), ec->power_crossing.end());
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
				c->agl.erase(c->agl.begin()+v);
				c->power_crossing.erase(c->power_crossing.begin()+v);
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
		junc->vertical_locked = false;
		junc->location.x = v->point().x;
		junc->location.y = v->point().y;
		junc->location.z = 0.0;
		junc->power_crossing = false;
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
		chain->export_type = NO_VALUE;
		chain->over_water = e->face()->IsWater() && e->twin()->face()->IsWater();
		chain->start_junction = junctionTable[e->source()];
		chain->end_junction = junctionTable[e->target()];
		chain->start_junction->chains.insert(chain);
		chain->end_junction->chains.insert(chain);
		outChains.insert(chain);
	}

	// Do one initial compaction - odds are we have a lot of chains that need to be built up!

	OptimizeNetwork(outJunctions, outChains, true);
	
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
				AssertPrintf("VALIDATION ERR - junction has a chain not pointing back at the junction.\n");				
		}
	}
	for (ci = outChains.begin(); ci != outChains.end(); ++ci)
	{
		if ((*ci)->start_junction->chains.find(*ci) == (*ci)->start_junction->chains.end())
			AssertPrintf("VALIDATION ERROR - chain refers to a junction not pointing back at the chain.\n");
		if ((*ci)->end_junction->chains.find(*ci) == (*ci)->end_junction->chains.end())
			AssertPrintf("VALIDATION ERROR - chain refers to a junction not pointing back at the chain.\n");
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

void	DrapeRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, CDT& inMesh)
{
	int total = 0;
	int added = 0;
	vector<Point3>	all_pts, these_pts;
		
	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	{
		all_pts.clear();
		these_pts.clear();
		Net_ChainInfo_t *	chain = *chainIter;
		
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
		
		chain->start_junction->agl = 0.0;
		chain->end_junction->agl = 0.0;
		
		chain->agl.resize(chain->shape.size());
		chain->power_crossing.resize(chain->shape.size());
		for (int m = 0; m < chain->shape.size(); ++m)
		{
			chain->agl[m] = 0.0;
			chain->power_crossing[m] = false;
		}
	}
	printf("Total of %d road pts, %d were added for mesh.\n", total, added);
}

void	PromoteShapePoints(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	ValidateNetworkTopology(ioJunctions, ioChains);
	printf("Promote: before: %d juncs, %d chains\n", ioJunctions.size(), ioChains.size());
	Net_ChainInfoSet	new_chains;
	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	if (!(*chainIter)->over_water)
	{
		Net_ChainInfo_t * chain = *chainIter;
		while (!chain->shape.empty())
		{
			Net_ChainInfo_t * new_chain = new Net_ChainInfo_t;
			Net_JunctionInfo_t * new_junc = new Net_JunctionInfo_t;
			
			new_chains.insert(new_chain);
			ioJunctions.insert(new_junc);
			
			new_junc->location = chain->shape.back();
			new_junc->agl = chain->agl.back();
			new_junc->power_crossing = chain->power_crossing.back();
			new_junc->chains.insert(new_chain);
			new_junc->chains.insert(chain);
			new_junc->vertical_locked = false;
			
			new_chain->start_junction = new_junc;
			new_chain->end_junction = chain->end_junction;
			new_chain->entity_type = chain->entity_type;
			new_chain->export_type = chain->export_type;
			new_chain->over_water = chain->over_water;
			
			chain->shape.pop_back();
			chain->agl.pop_back();
			chain->power_crossing.pop_back();
			
			chain->end_junction->chains.erase(chain);
			chain->end_junction->chains.insert(new_chain);
			
			chain->end_junction = new_junc;
		}
	}
	ioChains.insert(new_chains.begin(), new_chains.end());
	printf("Promote: after: %d juncs, %d chains\n", ioJunctions.size(), ioChains.size());
	ValidateNetworkTopology(ioJunctions, ioChains);

}

// RoadLayers - THEORY OF OPERATION
// We need to keep track of what has been put at what level of our intersection...we use a vector of ints
// where each int is a type of road use.  The first slot (0) represents ground level and each slot then stacks
// up a layer.  We can ask for the next slot compatible with a given road type.  This code takes care of the 
// logic that says things like...roads and city streets can cross on ground but not mid-air...put the highway
// in the first unused slot...etc.  wants_ground tells us if we search the ground slot first....it's possible that
// a number of road layers connect to water-bridges and must start above ground!
class	RoadLayers {
public:
	int		GetGoodSlot(int road_type, int wants_ground);
	void	UseSlot(int road_type, int slot);
private:
	bool	Compatible(int base_type, int new_type, int on_ground);
	vector<int>	slots;
};

int RoadLayers::GetGoodSlot(int road_type, int wants_ground)
{
	int start = wants_ground ? 0 : 1;
	int n;
	for (n = start; n < slots.size(); ++n)
		if (Compatible(slots[n], road_type, n == 0))	return n;
	return n;
}

void RoadLayers::UseSlot(int road_type, int slot)
{
	while (slot >= slots.size())	slots.push_back(use_None);
	slots[slot] = road_type;
}

bool	RoadLayers::Compatible(int base_type, int new_type, int on_ground)
{
	if (base_type == use_None) return true;
	if (base_type == use_Limited || new_type == use_Limited) return false;
	if (base_type == new_type) return true;
	if (base_type == use_Street && new_type == use_Rail && on_ground)	return true;
	if (base_type == use_Rail && new_type == use_Street && on_ground)	return true;
	return false;
}

Net_JunctionInfo_t *	CloneJunction(Net_JunctionInfo_t * junc)
{
	Net_JunctionInfo_t * new_junc = new Net_JunctionInfo_t;
	new_junc->location = junc->location;
	new_junc->agl = junc->agl;
	new_junc->power_crossing = false;
	new_junc->vertical_locked = true;
	return new_junc;
}

void	MigrateChain(Net_ChainInfo_t * chain, Net_JunctionInfo_t * old_j, Net_JunctionInfo_t * new_j)
{
	new_j->chains.insert(chain);
	old_j->chains.erase(chain);
	DebugAssert(chain->start_junction == old_j || chain->end_junction == old_j);
	if (chain->start_junction == old_j) chain->start_junction = new_j;
	if (chain->end_junction == old_j) chain->end_junction = new_j;	
}

// VerticalPartitionRoads
// 
// When we get road data it's often in "flat" form - if a highway goes over a street via a bridge, 
// the data has _four_ segments meeting at a point wehre the highway is on top of the street, even
// though no car could go directly from the highway to the street.
//
// This routine attempts to 'unflatten' the data:
// - It breaks up junctions with many segments into separate junctions based on where there is
//	 real connectivity.
// - It positions the junctions vertically where they need spatial separation.  This vertical
//   separation tries to take into account water bridges: 

void	VerticalPartitionRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	int	total_power_crossings = 0;
	
	typedef	pair<Net_ChainInfo_t *, Net_ChainInfo_t *> 		HighwayPair;
	typedef	multimap<double, HighwayPair, greater<double> >	HighwayAngleMap;

	Net_JunctionInfoSet		new_juncs;		// This is any new junctions created during this partitioning phase.

	// Iterate across all junctions that have more than 2 things going into them!  If 2 things connect to each other
	// we just assume they blend nicely.  (Note that this can be stupid...a road that ends at the start of a powerline...
	// but we don't try to sort out that pathological case.)
	for (Net_JunctionInfoSet::iterator juncIter = ioJunctions.begin(); juncIter != ioJunctions.end(); ++juncIter)
	if ((*juncIter)->chains.size() > 2)
	{
		Net_JunctionInfo_t * junc = *juncIter;
		
		// STEP 1 - PREP
		// We go through and find out who is around and pull out various useful chains.  When we're done we'll know some
		// of the main groups we must make and also roughly how the highways relate.
		
			vector<Net_ChainInfo_t *>						trains, streets, ramps;							// All of the segments...
			HighwayAngleMap									highways;										// A map of all POSSIBLE highway matches
			map<Net_ChainInfo_t *, Net_JunctionInfo_t *>	used_highways;									// For any highway that's been placed, who did we connect it to?
			Net_ChainInfoSet								unused_highways;								// All of the highways
			bool											train_bridge = false, street_bridge = false;
			RoadLayers										occupied;										// Tracking which layer is where.
			vector<Net_JunctionInfo_t *>					cloned_junctions;
			bool											has_power = false;
			bool											has_nonpower = false;
		
		for (Net_ChainInfoSet::iterator chain = junc->chains.begin(); chain != junc->chains.end(); ++chain)
		{
				double	angle;
				
			switch(gNetEntities[(*chain)->entity_type].use_mode) {
			case use_Limited:
				for (Net_ChainInfoSet::iterator other = junc->chains.begin(); other != junc->chains.end(); ++other)
				if (*chain != *other)
				if (gNetEntities[(*other)->entity_type].use_mode == use_Limited)
				{
					angle = junc->GetMatchingAngle(*chain, *other);
					highways.insert(HighwayAngleMap::value_type(angle, HighwayPair(*chain, *other)));
				}
				unused_highways.insert(*chain);
				break;
			case use_Street:	streets.push_back(*chain); if ((*chain)->over_water) street_bridge = true; 	break;
			case use_Ramp:		ramps.push_back(*chain); 													break;
			case use_Rail:		trains.push_back(*chain); if ((*chain)->over_water) train_bridge = true; 	break;
			case use_Power:																					break;
			default: AssertPrintf("Unknown use!");															break;
			}
			
			if (gNetEntities[(*chain)->entity_type].use_mode == use_Power)
				has_power = true;
			else
				has_nonpower = true;
		}
		
		if (has_power && has_nonpower)
		{
			junc->power_crossing = true;
			++total_power_crossings;
		}

			Net_JunctionInfo_t * 			street_junc = NULL;
			Net_JunctionInfo_t * 			ramp_junc = NULL;
			Net_JunctionInfo_t * 			train_junc = NULL;
			vector<Net_JunctionInfo_t *>	highway_juncs;
		
		// STEP 2 - STREETS
		// Build a new layer with all city streets...this is pretty easy - they all just go together.
		if (!streets.empty())
		{
			street_junc = CloneJunction(junc);
			cloned_junctions.push_back(street_junc);
			new_juncs.insert(street_junc);
			int street_level = occupied.GetGoodSlot(use_Street, !street_bridge);
			occupied.UseSlot(use_Street, street_level);
			street_junc->location.z += (5.0 * street_level);
			street_junc->agl = 5.0 * street_level;
			
			for (vector<Net_ChainInfo_t *>::iterator street = streets.begin(); street != streets.end(); ++street)
			{
				MigrateChain(*street, junc, street_junc);
			}
		}

		// STEP 3 - TRAINS
		// Build a new layer with all trains - similar to city streets.
		if (!trains.empty())
		{
			train_junc = CloneJunction(junc);
			cloned_junctions.push_back(train_junc);
			new_juncs.insert(train_junc);
			int train_level = occupied.GetGoodSlot(use_Rail, !train_bridge);
			occupied.UseSlot(use_Rail, train_level);
			train_junc->location.z += (5.0 * train_level);
			train_junc->agl = 5.0 * train_level;
			
			for (vector<Net_ChainInfo_t *>::iterator train = trains.begin(); train != trains.end(); ++train)
			{
				MigrateChain(*train, junc, train_junc);
			}
		}
		
		// STEP 4 - HIGHWAYS
		// Go through each highway pair.  For each highway pair, if neither segment is used, build a new
		// continuous highway on its own layer.  This tries to build the most continuous, least crossing,
		// least twisty set of highway layers we can come up with.
		for (HighwayAngleMap::iterator highway = highways.begin(); highway != highways.end(); ++highway)
		if (unused_highways.count(highway->second.first) > 0)
		if (unused_highways.count(highway->second.second) > 0)
		{
			bool	highway_bridge = highway->second.first->over_water || highway->second.second->over_water;
			Net_JunctionInfo_t * highway_junc = CloneJunction(junc);
			cloned_junctions.push_back(highway_junc);
			highway_juncs.push_back(highway_junc);
			new_juncs.insert(highway_junc);
			int highway_level = occupied.GetGoodSlot(use_Limited, !highway_bridge);
			occupied.UseSlot(use_Limited, highway_level);
			highway_junc->location.z += (5.0 * highway_level);
			highway_junc->agl = 5.0 * highway_level;
			
			MigrateChain(highway->second.first, junc, highway_junc);
			MigrateChain(highway->second.second, junc, highway_junc);
			
			unused_highways.erase(highway->second.first);
			unused_highways.erase(highway->second.second);
			used_highways[highway->second.first] = highway_junc;
			used_highways[highway->second.second] = highway_junc;			
		}

		// Odd highway case - we have a number of highways but some unpaired segment...this happens when for
		// example we have a Y-split...two of the three parts  of the Y were merged previously...now we have
		// one extra piece.  We jam it into the junction it would fit best with.
		for (HighwayAngleMap::iterator highway = highways.begin(); highway != highways.end(); ++highway)
		if (unused_highways.count(highway->second.first) > 0)
		{
			bool	highway_bridge = highway->second.first->over_water;
			DebugAssert(used_highways.count(highway->second.second) > 0);
			Net_JunctionInfo_t * highway_junc = used_highways[highway->second.second];
			// Horrendous case: what if the piece we are merging with is on the ground but we
			// are a bridge?!?  Rip the layer out and put it on top of the stack.  This is 
			// yucky and leaves the bottom layer open but we don't think this will happen much and
			// also frankly we don't care.
			if (highway_junc->agl == 0.0 && highway_bridge)
			{
				int highway_level = occupied.GetGoodSlot(use_Limited, !highway_bridge);
				occupied.UseSlot(use_Limited, highway_level);
				highway_junc->location.z = junc->location.z + (5.0 * highway_level);
				highway_junc->agl = 5.0 * highway_level;
			}
						
			MigrateChain(highway->second.first, junc, highway_junc);
			
			unused_highways.erase(highway->second.first);
			used_highways[highway->second.first] = highway_junc;
		}

		// Straggler case - we may have one more highway segment laying around...for example if this junction ends in a highway turning
		// into a city street or just ending.  Make sure that (1) we never have TWO stragglers - they should have been jammed together into
		// a continuous highway and (2) if we have a straggler, that we didn't have highway pairs.
		DebugAssert(unused_highways.size() < 2);
		DebugAssert(highways.empty() || unused_highways.empty());
		for (Net_ChainInfoSet::iterator straggler = unused_highways.begin(); straggler != unused_highways.end(); ++straggler)
		{
			// Either make a new junction or recycle the street junction.  Only use the street junction if:
			// (1) there is a street junction
			// (2) there are NO exit ramps here
			// (3) the street's elevation matches the bridging we need for this bridge.
			bool	highway_bridge = (*straggler)->over_water;
			Net_JunctionInfo_t * highway_junc = NULL;
			if (street_junc != NULL && (highway_bridge == (street_junc->agl != 0.0)))
			{
				highway_junc = street_junc;
			} else {			
				highway_junc = CloneJunction(junc);
				cloned_junctions.push_back(highway_junc);
				highway_juncs.push_back(highway_junc);
				new_juncs.insert(highway_junc);
				int highway_level = occupied.GetGoodSlot(use_Limited, !highway_bridge);
				occupied.UseSlot(use_Limited, highway_level);
				highway_junc->location.z += (5.0 * highway_level);
				highway_junc->agl = 5.0 * highway_level;
			}
			
			MigrateChain(*straggler, junc, highway_junc);
		}

		// STEP 5 - RAMPS
		// Ramps then just have to be attached to, well, whatever we can find.  First we'll try all highways with a nice
		// merge angle, then city streets, then any highway we have.  If all that fails, we'll have to make up a junction.

		for (vector<Net_ChainInfo_t *>::iterator ramp = ramps.begin(); ramp != ramps.end(); ++ramp)
		{
			double 				 best_angle = 0.0;
			Net_JunctionInfo_t * best_junc = NULL;
			Vector2				 ramp_vec = (*ramp)->vector_to_junc_flat(junc);
			ramp_vec.normalize();
			for (vector<Net_JunctionInfo_t *>::iterator h_junc = highway_juncs.begin(); h_junc != highway_juncs.end(); ++h_junc)
			{
				for (Net_ChainInfoSet::iterator j_chain = (*h_junc)->chains.begin(); j_chain != (*h_junc)->chains.end(); ++j_chain)
				{
					Vector2	chain_vec = (*j_chain)->vector_to_junc_flat(junc);
					chain_vec.normalize();
					double dot = fabs(chain_vec.dot(ramp_vec));
					if (dot >= best_angle)
					{
						best_junc = *h_junc;
						best_angle = dot;
					}
				}
			}
			
			if (best_junc == NULL && street_junc == NULL)
			{
				if (ramp_junc == NULL)
				{
					ramp_junc = CloneJunction(junc);
					cloned_junctions.push_back(ramp_junc);
					new_juncs.insert(ramp_junc);
					int ramp_level = occupied.GetGoodSlot(use_Limited, true);
					occupied.UseSlot(use_Limited, ramp_level);
					ramp_junc->location.z += (5.0 * ramp_level);
					ramp_junc->agl = 5.0 * ramp_level;
				}
				MigrateChain(*ramp, junc, ramp_junc);
			} else {
			
				if (best_angle >= 0.85 || street_junc == NULL)
				{
					MigrateChain(*ramp, junc, best_junc);
				} else {
					MigrateChain(*ramp, junc, street_junc);
				}		
			}
		}		

		// STEP 6 - CLEANUP
		// Finally we will check to see whether we really have a vertically-controlled situation, e.g. bridges over each other.
		// If not we can vertically unlock our junctions, which will allow the road smoother to later do a better job of bringing
		// highways gently up and down.

		int total_juncs = 0;
		for (vector<Net_JunctionInfo_t *>::iterator all_juncs = cloned_junctions.begin(); all_juncs != cloned_junctions.end(); ++all_juncs)
		{
			if (!(*all_juncs)->chains.empty())	++total_juncs;
		}
		
		if (total_juncs < 2)
		for (vector<Net_JunctionInfo_t *>::iterator all_juncs = cloned_junctions.begin(); all_juncs != cloned_junctions.end(); ++all_juncs)
		{
			(*all_juncs)->vertical_locked = false;
		}
	}
	
	ioJunctions.insert(new_juncs.begin(),new_juncs.end());	
	printf("total_power_crossings=%d\n",total_power_crossings);
}

// Given a chain, go from origin along chain at least scan_dist or until we hit a locked or otherwise screwy
// vertex.  
double find_highest_ground_along_chain(Net_ChainInfo_t * chain, Net_JunctionInfo_t * origin, double scan_dist)
{
	Net_JunctionInfo_t * junc = chain->other_junc(origin);
	double msl = max(origin->location.z - origin->agl,
					 junc->location.z - junc->agl);
	scan_dist -= chain->meter_length(0, chain->seg_count());
	
	while (scan_dist > 0.0 && !junc->vertical_locked && junc->chains.size() == 2)
	{
		chain = junc->get_other(chain);
		junc = chain->other_junc(junc);
		msl = max(msl,
						 junc->location.z - junc->agl);
	
		scan_dist -= chain->meter_length(0, chain->seg_count());
		
	}
	
	return msl;
}

void	VerticalBuildBridges(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
		int		n;
		
	for (Net_ChainInfoSet::iterator chain = ioChains.begin(); chain != ioChains.end(); ++chain)	
	if ((*chain)->over_water)
	{
		/* STEP 1 - SIMPLIFICATION */
		
		// Go through this bridge and remove shaping points as long as we don't displace the bridge more than
		// a few meters at most (BRIDGE_TURN_SIMPLIFY).  Why?  There  are a lot of spurious shape points in
		// roads, particularly due to crossing of mesh triangles.  We want the longest segs so we have the 
		// most options in terms of breaking up and building the bridge.  (If we have shape points, our bridge
		// options are more limited.)
	
		for (int i = 0; i < (*chain)->shape.size(); ++i)
		{
			Point3	p1 = (*chain)->nth_pt(i  );
			Point3	p2 = (*chain)->nth_pt(i+1);
			Point3	p3 = (*chain)->nth_pt(i+2);

			Point2	p_mid = Segment2(Point2(p1.x,p1.y), Point2(p3.x, p3.y)).projection(Point2(p2.x,p2.y));
			
			double	displacement = LonLatDistMeters(p2.x, p2.y, p_mid.x, p_mid.y);
			
			if (displacement < BRIDGE_TURN_SIMPLIFY)
			{
				(*chain)->shape.erase((*chain)->shape.begin()+i);
				(*chain)->agl.erase((*chain)->agl.begin()+i);
				(*chain)->power_crossing.erase((*chain)->power_crossing.begin()+i);
				--i;
			}
		}

		/* STEP 2 - BRIDGE CHOICE */
		
		// Pick a bridge, any bridge.  Go to the spreadsheet to look up bridge choices.
	
		double	total_len = 0.0;
		double	biggest_len = 0.0;
		double	smallest_len = 9.9e9;
		double	sharpest_turn = 1.0;	// 1.0 == straight!
		for (n = 0; n < (*chain)->seg_count(); ++n)
		{
			double local_len = (*chain)->meter_length(n, n+1);
			total_len += local_len;
			biggest_len = max(biggest_len, local_len);
			smallest_len = min(smallest_len, local_len);
			
			if (n > 0) sharpest_turn = min(sharpest_turn,(*chain)->dot_angle(n));
		}

		double	agl1 = -1.0;
		double	agl2 = -1.0;
		if ((*chain)->start_junction->vertical_locked)	agl1 = (*chain)->start_junction->agl;
		if ((*chain)->end_junction->vertical_locked)	agl2 = (*chain)->end_junction->agl;
		
		int	bridge_rule = FindBridgeRule((*chain)->entity_type, total_len, smallest_len, biggest_len, (*chain)->seg_count(), sharpest_turn, agl1, agl2);
		if (bridge_rule == -1)
		{
			(*chain)->export_type = -1;

		} else {
		
			/* STEP 3 - PICK STARTING POINTS */
			
			// Now we have to decide how high up the bridge starts!  Our rule is:
			// Look out for search_dist to see how high the ground is (or a locked junction is).  Call that our uniform height.
			// If preferred is higher up, use preferred.  Clamp to min and max.

			if (agl1 == -1)
			{
				if ((*chain)->start_junction->chains.size() == 2)
				{
					agl1 = max(find_highest_ground_along_chain(
									(*chain)->start_junction->get_other(*chain),
									(*chain)->start_junction,
									gBridgeInfo[bridge_rule].search_dist) - (*chain)->start_junction->location.z,
								(double) gBridgeInfo[bridge_rule].pref_start_agl);
					
				} else
					agl1 = gBridgeInfo[bridge_rule].pref_start_agl;

				agl1 = max((double) gBridgeInfo[bridge_rule].min_start_agl, agl1);
				agl1 = min((double) gBridgeInfo[bridge_rule].max_start_agl, agl1);
			}

			if (agl2 == -1)
			{
				if ((*chain)->end_junction->chains.size() == 2)
				{
					agl2 = max(find_highest_ground_along_chain(
									(*chain)->end_junction->get_other(*chain),
									(*chain)->end_junction,
									gBridgeInfo[bridge_rule].search_dist) - (*chain)->end_junction->location.z,
								(double) gBridgeInfo[bridge_rule].pref_start_agl);
					
				} else
					agl2 = gBridgeInfo[bridge_rule].pref_start_agl;

				agl2 = max((double) gBridgeInfo[bridge_rule].min_start_agl, agl2);
				agl2 = min((double) gBridgeInfo[bridge_rule].max_start_agl, agl2);
			}
			
			/* STEP 4 - PICK BRIDGE GEOMETRY */
			
			// Now we have to pick the bridge height at max clearnace.  First guess: total-length based.
			// Then limit by how high we can get			
			double	total_length = (*chain)->meter_length(0, (*chain)->seg_count());
			double	center_height = total_length * gBridgeInfo[bridge_rule].height_ratio;
			// Make sure it's not below either of our ends!
			center_height = max(center_height, agl1);
			center_height = max(center_height, agl2);			
			// Limit by the amount we can climb do to slope constraints.
			center_height = min(center_height, agl1 + total_length * 0.5 * gBridgeInfo[bridge_rule].road_slope);
			center_height = min(center_height, agl2 + total_length * 0.5 * gBridgeInfo[bridge_rule].road_slope);
			// Limit by explicit rules to keep within a sane range.
			center_height = min(center_height, (double) gBridgeInfo[bridge_rule].max_center_agl);
			center_height = max(center_height, (double) gBridgeInfo[bridge_rule].min_center_agl);

			// This is our guestimate for how long the ramps have to be, roughly.
			double	ramp1 = (center_height - agl1) / gBridgeInfo[bridge_rule].road_slope;
			double	ramp2 = (center_height - agl2) / gBridgeInfo[bridge_rule].road_slope;
			double	ramp_rat1 = (				ramp1) / total_length;
			double	ramp_rat2 = (total_length - ramp2) / total_length;

			if (ramp1 + ramp2 > total_length)
			{
				ramp_rat1 = ramp_rat2 = (ramp1) / (ramp1 + ramp2);
				ramp1 = total_length * (	  ramp_rat1);
				ramp2 = total_length * (1.0 - ramp_rat2);
			}
			
			/* STEP 5 - POINT SPLITTING */
			
			// The spreadsheet contains three rules on how to split up the roadway: by count (split everything by N),
			// by length (split if the length is more than 2N), and by arch, which will try to make sure we have split points
			// where we need them.
						
			if (gBridgeInfo[bridge_rule].split_count > 1)
			for (n = 0; n < (*chain)->seg_count(); ++n)
			{
				int ctr = gBridgeInfo[bridge_rule].split_count;
				while (ctr > 1)
				{
					(*chain)->split_seg(n, 1.0 / (float) ctr);
					--ctr;
					++n;
				}
			}

			if (gBridgeInfo[bridge_rule].split_length > 0.0)
			for (n = 0; n < (*chain)->seg_count(); ++n)
			{
				double local_len = (*chain)->meter_length(n,n+1);
				double reps = local_len / gBridgeInfo[bridge_rule].split_length;
				int	   reps_i = reps;
				
				while (reps_i > 1)
				{
					(*chain)->split_seg(n, 1.0 / (float) reps_i);
					++n;
					--reps_i;
				}
			}
			
			if (gBridgeInfo[bridge_rule].split_arch)
			{
				double 	cume_len = 0.0;				// How far along the chain we've gone
				double	len_local; 					// Length of this one segment in meters
				double	scaled_start, scaled_end;	// Start and end of this segment as a ratio of the whole chain
				double	ramp_rat1_local;			// position of the ramp cut points relative to the segment.  Values 
				double	ramp_rat2_local;			// negative or greater than one indicate we're off the edge of the seg!
				
				for (n = 0; n < (*chain)->seg_count(); ++n)
				{
					len_local = (*chain)->meter_length(n,n+1);
					scaled_start = cume_len / total_length;
					cume_len += len_local;
					scaled_end = cume_len / total_length;
					
					ramp_rat1_local = (ramp_rat1 - scaled_start) / (scaled_end - scaled_start);
					if (ramp_rat1_local > 0.1 && ramp_rat1_local < 0.9)
					{
						(*chain)->split_seg(n, ramp_rat1_local);
						break;
					}
					if (ramp_rat1_local < 0.0)
						break;					
				}

				cume_len = 0.0;
				for (n = 0; n < (*chain)->seg_count(); ++n)
				{
					len_local = (*chain)->meter_length(n,n+1);
					scaled_start = cume_len / total_length;
					cume_len += len_local;
					scaled_end = cume_len / total_length;
					
					ramp_rat2_local = (ramp_rat2 - scaled_start) / (scaled_end - scaled_start);
					if (ramp_rat2_local > 0.1 && ramp_rat2_local < 0.9)
					{
						(*chain)->split_seg(n, ramp_rat2_local);
						break;
					}
					if (ramp_rat2_local < 0.0)
						break;
				}
			}
			
			/* STEP 6 - ARCH BRIDGE */
			
			double	cur_height;
			double	length_start = 0.0;
			double	length_end;
			double	shape_rat;
			
			for (n = 0; n < (*chain)->shape.size(); ++n)
			{
				length_start += (*chain)->meter_length(n,n+1);
				length_end = total_length - length_start;
				shape_rat = length_start / total_length;
					 if (shape_rat <= ramp_rat1)cur_height = agl1 + (length_start * gBridgeInfo[bridge_rule].road_slope);
				else if (shape_rat >= ramp_rat2)cur_height = agl2 + (length_end   * gBridgeInfo[bridge_rule].road_slope);
				else							cur_height = center_height;					
				if (cur_height > center_height)	cur_height = center_height;
				
				(*chain)->agl[n] = cur_height;
				(*chain)->shape[n].z += cur_height;
			}
		
			if (!(*chain)->start_junction->vertical_locked)
			{
				(*chain)->start_junction->vertical_locked = true;
				(*chain)->start_junction->agl = agl1;
				(*chain)->start_junction->location.z += agl1;
			}
			if (!(*chain)->end_junction->vertical_locked)
			{
				(*chain)->end_junction->vertical_locked = true;
				(*chain)->end_junction->agl = agl2;
				(*chain)->end_junction->location.z += agl2;
			}
				
			(*chain)->export_type = gBridgeInfo[bridge_rule].export_type;			
		}
	}
}

void	InterpolateRoadHeights(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	typedef	multimap<double, Net_JunctionInfo_t *>	q_type;
	q_type							to_process;
	Net_JunctionInfoSet::iterator	junc;
	Net_ChainInfoSet::iterator		chain;
	
	int	pre_locked = 0;
	int was_locked = 0;
	
	for (junc = ioJunctions.begin(); junc != ioJunctions.end(); ++junc)
	if (!(*junc)->vertical_locked)
	{
		double	shortest_len = 9.9e9;
		bool	nearby_locked = false;
		for (chain = (*junc)->chains.begin(); chain != (*junc)->chains.end(); ++chain)
		{
			DebugAssert((*chain)->over_water == false || (*chain)->export_type == -1);	// If it's water, it better be hidden or locked down
			
			if ((*chain)->other_junc(*junc)->vertical_locked)
			{
				shortest_len = min(shortest_len, (*chain)->meter_length(0, (*chain)->seg_count()));
				nearby_locked = true;							
			}
		}
		
		if (nearby_locked)
			to_process.insert(q_type::value_type(shortest_len, *junc));
	}
	
	while (!to_process.empty())
	{
		Net_JunctionInfo_t * current = to_process.begin()->second;
		double		current_lock_dist = to_process.begin()->first;
		to_process.erase(to_process.begin());
		
		if (!current->vertical_locked)
		{
			double	min_msl = current->location.z		  ;
			double	max_msl = current->location.z + 1000.0;
			double	gnd_msl = current->location.z		  ;
			double	max_agl = 0.0						  ;

			for (chain = current->chains.begin(); chain != current->chains.end(); ++chain)
			{
				Net_JunctionInfo_t * other = (*chain)->other_junc(current);
				if (other->vertical_locked)
				{
					double seg_len = (*chain)->meter_length(0, (*chain)->seg_count());
													  
					double var = seg_len * gNetEntities[(*chain)->entity_type].max_slope;
					
					min_msl = max(min_msl, other->location.z - var);
					max_msl = min(max_msl, other->location.z + var);
					max_agl = max(max_agl, other->agl);
				} else {
					double len = current_lock_dist + (*chain)->meter_length(0, (*chain)->seg_count());
					to_process.insert(q_type::value_type(len, other));
				}
			}
			
			if (min_msl > max_msl)				min_msl = ((min_msl + max_msl) * 0.5);			
			if (min_msl < gnd_msl)				min_msl = gnd_msl;			
			if (min_msl > (gnd_msl + max_agl))	min_msl = gnd_msl + max_agl;
			
			current->location.z = min_msl;
			current->agl = min_msl - gnd_msl;
			current->vertical_locked = true;
		}	
		
	}

/*	
	// TODO - this code sucks!  Ideally we'd consider the sloep factor as we smooth out each junction and
	// potentially chop up segments that defy slope-bridging constraints!
	// (Also we need to examine code above, where slope constraints can do whacky things in a hilly area.
	// I think we need to break up any segments that defy slope constraints, so we can whack them all later.)
	
	It is also disabled - we don't use shape points for on-land anymore, we use a ton of junctions!
	
	for (chain = ioChains.begin(); chain != ioChains.end(); ++chain)
	if (gNetEntities[(*chain)->entity_type].use_mode != use_Power)
	{	
		Net_JunctionInfo_t * start = (*chain)->start_junction;
		Net_JunctionInfo_t * end = (*chain)->end_junction;
		// Either we're both locked or we're both not!  Shouldn't be any half-way!  We would both be unlocked
		// if we are a self-contained graph of fully unlocked chains nodes.
		DebugAssert((start->vertical_locked && end->vertical_locked) || (!start->vertical_locked && !end->vertical_locked));
		
		if (start->vertical_locked && end->vertical_locked)
		if (start->agl > 0.0 || end->agl > 0.0)
		{
			double total_len = (*chain)->meter_length(0, (*chain)->seg_count());
			double cur_len = 0.0;
			for (int n = 0; n < ((*chain)->seg_count()-1); ++n)
			{
				cur_len += (*chain)->meter_length(n,n+1);
				double ratio = cur_len / total_len;
				
				double cur_msl = (1.0-ratio) * start->location.z + ratio * end->location.z;
				double cur_gnd = (*chain)->shape[n].z;
				if (cur_msl < cur_gnd) cur_msl = cur_gnd;
				double cur_agl = cur_msl - cur_gnd;
				
				(*chain)->shape[n].z = cur_msl;
				(*chain)->agl[n] = cur_agl;
			}
		}
	}
*/	
}

void	AssignExportTypes(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	for (Net_ChainInfoSet::iterator chain = ioChains.begin(); chain != ioChains.end(); ++chain)
	if ((*chain)->export_type == 0)
	{
		bool above_ground = false;
		for (int n = 0; n < (*chain)->pt_count(); ++n)
		{
			if ((*chain)->nth_agl(n) > 0.0)
			{
				above_ground = true;
				break;
			}
		}
		
		(*chain)->export_type = above_ground ?  gNetEntities[(*chain)->entity_type].export_type_overpass : gNetEntities[(*chain)->entity_type].export_type_normal;
	}
}

void DeleteBlankChains(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	for (Net_ChainInfoSet::iterator chain = ioChains.begin(); chain != ioChains.end();)
	{
		if ((*chain)->export_type == -1)
		{
			(*chain)->start_junction->chains.erase(*chain);
			(*chain)->end_junction->chains.erase(*chain);
			delete *chain;
			ioChains.erase(chain++);
		} else {
			++chain;
		}
	}
}

void	SpacePowerlines(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, double ideal_dist_m, double max_dip)
{
	int		total_pts = 0;
	int		total_xos = 0;
	int		kill_pts = 0;
	int		kill_xos = 0;
	int		end_crossings = 0;

//	gMeshPoints.clear();
	
	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	if (gNetEntities[(*chainIter)->entity_type].use_mode == use_Power)
	{
		Net_ChainInfo_t * chain = *chainIter;
		
		if (chain->start_junction->power_crossing)
			++end_crossings;
		if (chain->end_junction->power_crossing)
			++end_crossings;
		
		bool did_work;
		bool first_time = true;
		do {
			did_work = first_time ? true : false;
		
			DebugAssert(chain->shape.size() == chain->power_crossing.size());
		
			for (int i = 0; i < chain->shape.size(); ++i)
			{
				if (first_time							  )	++total_pts;
				if (first_time && chain->power_crossing[i]) ++total_xos;
				
				Point3	p1 = chain->nth_pt(i  );
				Point3	p2 = chain->nth_pt(i+1);
				Point3	p3 = chain->nth_pt(i+2);

				double len1 = chain->meter_length(i  , i+1);	// Counting off by one since i is in shape pts, not total pts!
				double len2 = chain->meter_length(i+1, i+2);
				
				double	msl1 = chain->nth_pt(i  ).z;
				double	msl  = chain->nth_pt(i+1).z;
				double	msl2 = chain->nth_pt(i+2).z;
				
				double	rat = len1 / (len1 + len2);

				double	mid_msl = msl1 * (1.0 - rat) + msl2 * rat;

				Point2	p_mid = Segment2(Point2(p1.x,p1.y), Point2(p3.x, p3.y)).projection(Point2(p2.x,p2.y));
				
				double	displacement = LonLatDistMeters(p2.x, p2.y, p_mid.x, p_mid.y);
				
				if ((len1 + len2) < ideal_dist_m &&		// We're short enough that we can wipe the middle and
					(mid_msl + max_dip) > msl &&		// removing this wouldn't cause the segment to dip too deep and
					(msl <= msl1 || msl <= msl2) &&		// This isn't a local high-point
					displacement < 5.0)					// We don't deviate laterally too much!
				
				if (!first_time || chain->power_crossing[i])
				{
					if (chain->power_crossing[i]){ ++kill_xos;
//					gMeshPoints.push_back(Point2(chain->shape[i].x, chain->shape[i].y));

					}
												  ++kill_pts;
					chain->shape.erase(chain->shape.begin()+i);
					chain->agl.erase(chain->agl.begin()+i);
					chain->power_crossing.erase(chain->power_crossing.begin()+i);
					--i;
					did_work = true;
				}
			}
		
			first_time =  false;
		} while (did_work);		
	}
	printf("Total pts=%d total xos=%d, kill pts=%d, kill xos=%d, end xos=%d\n", total_pts, total_xos, kill_pts, kill_xos, end_crossings);
}
