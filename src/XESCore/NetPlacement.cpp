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

#if !DEV
clean this out
#endif
#include "WED_Globals.h"

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

/*
bool	CanMerge(Net_ChainInfo_t * road1, Net_ChainInfo_t * road2, double angle)
{
	if (angle > 0.85) return true;
	if (gNetEntities[road1->entity_type].use_mode == use_Limited) return false;
	if (gNetEntities[road2->entity_type].use_mode == use_Limited) return false;
	return true;
}

// This struct contains all the chains that will go in one layer.  We also cache whether
// anyone is limited access and the most north-south angle of any of them in dy form.
struct	HighwayLayer_t {
	Net_ChainInfoSet	chains;
	bool				lim_access;
	bool				water_bridge;
	double				abs_y;
};

// This sorts - prioritize bridges, then highways on top, and NS over EW.
struct SortLayersByHighway {
	bool	operator()(const HighwayLayer_t& lhs, const HighwayLayer_t& rhs) const {
		if (lhs.water_bridge != rhs.water_bridge)
			return rhs.water_bridge;
		if (lhs.lim_access != rhs.lim_access)
			return rhs.lim_access;
		return lhs.abs_y < rhs.abs_y;
	}
};
*/

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
	gMeshPoints.clear();
	
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
			gMeshPoints.push_back(Point2(junc->location.x, junc->location.y));
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
					int ramp_level = occupied.GetGoodSlot(use_Limited, false);
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

void	VerticalBuildBridges(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	for (Net_ChainInfoSet::iterator chain = ioChains.begin(); chain != ioChains.end(); ++chain)	
	if ((*chain)->over_water)
	{
		double total_len = (*chain)->meter_length(0, (*chain)->pt_count()-1);

		double	agl1 = -1.0;
		double	agl2 = -1.0;
		if ((*chain)->start_junction->vertical_locked)	agl1 = (*chain)->start_junction->agl;
		if ((*chain)->end_junction->vertical_locked)	agl2 = (*chain)->end_junction->agl;
		
		int	bridge_rule = FindBridgeRule((*chain)->entity_type, total_len, agl1, agl2);
		if (bridge_rule == -1.0)
		{
			AssertPrintf("Bridge rule failed for type=%s, len=%lf, agl1=%lf, agl2=%lf", FetchTokenString((*chain)->entity_type), total_len, agl1, agl2);
		} else {
		
			if (!(*chain)->start_junction->vertical_locked)
			{
				(*chain)->start_junction->vertical_locked = true;
				(*chain)->start_junction->agl = max(5.0f, gBridgeInfo[bridge_rule].min_height);
				(*chain)->start_junction->location.z += max(5.0f, gBridgeInfo[bridge_rule].min_height);
			}
			if (!(*chain)->end_junction->vertical_locked)
			{
				(*chain)->end_junction->vertical_locked = true;
				(*chain)->end_junction->agl = max(5.0f, gBridgeInfo[bridge_rule].min_height);
				(*chain)->end_junction->location.z += max(5.0f, gBridgeInfo[bridge_rule].min_height);
			}
				
			(*chain)->export_type = gBridgeInfo[bridge_rule].export_type;
			
			// TODO - build actual bridge arch with split!
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
			DebugAssert((*chain)->over_water == false);
			
			if ((*chain)->other_junc(*junc)->vertical_locked)
			{
				shortest_len = min(shortest_len, (*chain)->meter_length(0, (*chain)->seg_count()));
				nearby_locked = true;							
			}
		}
		
		if (nearby_locked)
			to_process.insert(q_type::value_type(shortest_len, *junc));
	}// else
	//	(*junc)->lock_dist_m = 0.0;
	
//	FILE * fi = fopen("road.txt", "a");
//	fprintf(fi, "Processing...\n");
	
	while (!to_process.empty())
	{
		Net_JunctionInfo_t * current = to_process.begin()->second;
		double		current_lock_dist = to_process.begin()->first;
//		current->lock_dist_m = to_process.begin()->first;
		to_process.erase(to_process.begin());
		
		if (!current->vertical_locked)
		{
			double	min_msl = current->location.z		  ;
			double	max_msl = current->location.z + 1000.0;
			double	gnd_msl = current->location.z		  ;

//			fprintf(fi,"Processing %lf, %lf: min=%lf, max=%lf, gnd=%lf, prio = %lf\n",
//					current->location.x, current->location.y, min_msl, max_msl, gnd_msl, current->lock_dist_m);
			
			for (chain = current->chains.begin(); chain != current->chains.end(); ++chain)
			{
				Net_JunctionInfo_t * other = (*chain)->other_junc(current);
				if (other->vertical_locked)
				{
					double seg_len = (*chain)->meter_length(0, (*chain)->seg_count());
													  
					double var = seg_len * gNetEntities[(*chain)->entity_type].max_slope;
					
//					fprintf(fi,"     var=%lf, location.z = %lf, old range = %lf - %lf ", var, other->location.z, min_msl, max_msl);
					min_msl = max(min_msl, other->location.z - var);
					max_msl = min(max_msl, other->location.z + var);
//					fprintf(fi," new range = %lf - %lf\n", min_msl, max_msl);
				} else {
					double len = current_lock_dist + (*chain)->meter_length(0, (*chain)->seg_count());
					to_process.insert(q_type::value_type(len, other));
				}
			}
/*			
			for (chain = current->chains.begin(); chain != current->chains.end(); ++chain)
			{
				Net_JunctionInfo_t * other = (*chain)->other_junc(current);
				if (other->vertical_locked)
				{
					double seg_len = (*chain)->meter_length(0, (*chain)->seg_count());
					double var = seg_len * gNetEntities[(*chain)->entity_type].max_slope;
					fprintf(fi,"  Seg len = %lf, var = %lf, type = %s, Y=%lf goes to %lf,%lf\n", 
						seg_len, var, FetchTokenString((*chain)->entity_type), other->location.z, other->location.x, other->location.y);
				} else {
					double seg_len = (*chain)->meter_length(0, (*chain)->seg_count());
					fprintf(fi,"  Seg len = %lf, unlocked, type = %s, goes to %lf, %lf\n", 
						seg_len, FetchTokenString((*chain)->entity_type), other->location.x, other->location.y);
				}				
			}
*/		
			
			if (min_msl > max_msl)			min_msl = ((min_msl + max_msl) * 0.5);			
			if (min_msl < gnd_msl)			min_msl = gnd_msl;
			
//			fprintf(fi,"    final height: min=%lf,max=%lf,gnd=%lf\n", min_msl, max_msl, gnd_msl);
			
			current->location.z = min_msl;
			current->agl = min_msl - gnd_msl;
			current->vertical_locked = true;
		}	
		
	}
//	fclose(fi);
	
	// TODO - this code sucks!  Ideally we'd consider the sloep factor as we smooth out each junction and
	// potentially chop up segments that defy slope-bridging constraints!
	// (Also we need to examine code above, where slope constraints can do whacky things in a hilly area.
	// I think we need to break up any segments that defy slope constraints, so we can whack them all later.)
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

void	SpacePowerlines(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, double ideal_dist_m, double max_dip)
{
	int		total_pts = 0;
	int		total_xos = 0;
	int		kill_pts = 0;
	int		kill_xos = 0;
	int		end_crossings = 0;
	int		true_crossings = 0;

	gMeshPoints.clear();

	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	if (gNetEntities[(*chainIter)->entity_type].use_mode == use_Power)
	for (int n = 0; n < (*chainIter)->seg_count(); ++n)
	{
		Segment3 seg((*chainIter)->nth_seg(n));
		do {
			(*chainIter)->split_seg(n, 0.5); 
		}	
		while (LonLatDistMeters(seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y) > ideal_dist_m);
		++n;
	}	
	
	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	if (gNetEntities[(*chainIter)->entity_type].use_mode == use_Power)
	for (int n = 0; n < (*chainIter)->power_crossing.size(); ++n)
	if ((*chainIter)->power_crossing[n])
	{
		++true_crossings;
//		gMeshPoints.push_back(Point2((*chainIter)->shape[n].x,(*chainIter)->shape[n].y));
	}
//	for (Net_JunctionInfoSet::iterator juncIter = ioJunctions.begin(); juncIter != ioJunctions.end(); ++juncIter)
//	if ((*juncIter)->power_crossing)
//	{
//		++true_crossings;
//		gMeshPoints.push_back(Point2((*juncIter)->location.x,(*juncIter)->location.y));
//	}
	
	printf("True power crossing count after stuff...shape only=%d\n", true_crossings);
	
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

				Vector2	v1(Point2(p1.x, p1.y), Point2(p2.x, p2.y));
				Vector2	v2(Point2(p2.x, p2.y), Point2(p3.x, p3.y));
				v1.normalize();
				v2.normalize();
			
				double len1 = chain->meter_length(i  , i+1);	// Counting off by one since i is in shape pts, not total pts!
				double len2 = chain->meter_length(i+1, i+2);
				
				double	msl1 = chain->nth_pt(i  ).z;
				double	msl  = chain->nth_pt(i+1).z;
				double	msl2 = chain->nth_pt(i+2).z;
				
				double	rat = len1 / (len1 + len2);

				double	mid_msl = msl1 * (1.0 - rat) + msl2 * rat;

				Point2	p_mid = Segment2(Point2(p1.x,p1.y), Point2(p3.x, p3.y)).midpoint(rat);
				
				double	displacement = LonLatDistMeters(p2.x, p2.y, p_mid.x, p_mid.y);
				
				double dot = v1.dot(v2);

				if (first_time && chain->power_crossing[i])
				{
					bool len_ok = (len1 + len2) < ideal_dist_m;
					bool dip_ok = (mid_msl + max_dip) > msl;
					bool peak_ok = (msl <= msl1 || msl <= msl2);
//					bool dot_ok = dot > 0.999;
					bool dot_ok = displacement < 5.0;

					printf("Len=%s Dip=%s Peak=%s dot=%s    len1=%lf len2=%lf msl1=%lf msl=%lf msl2=%lf msl_mid=%lf disp=%lf\n", 
						len_ok ? "y" : "n",
						dip_ok ? "y" : "n",
						peak_ok ? "y" : "n",
						dot_ok ? "y" : "n",
						len1, len2,
						msl1, msl, msl2, mid_msl,
						displacement);
				}
				
			
				if ((len1 + len2) < ideal_dist_m &&		// We're short enough that we can wipe the middle and
					(mid_msl + max_dip) > msl &&		// removing this wouldn't cause the segment to dip too deep and
					(msl <= msl1 || msl <= msl2) &&		// This isn't a local high-point
					displacement < 5.0)					// We don't deviate laterally too much!
				
				if (!first_time || chain->power_crossing[i])
				{
					if (chain->power_crossing[i]){ ++kill_xos;
					gMeshPoints.push_back(Point2(chain->shape[i].x, chain->shape[i].y));

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
	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	if (gNetEntities[(*chainIter)->entity_type].use_mode == use_Power)
	for (int n = 0; n < (*chainIter)->seg_count(); ++n)
	{
		Segment3 seg = (*chainIter)->nth_seg(n);
		printf("  Seg %d: dist = %lf\n", n, LonLatDistMeters(seg.p1.x,seg.p1.y,seg.p2.x,seg.p2.y));
	}
}
