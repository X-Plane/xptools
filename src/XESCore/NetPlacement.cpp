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
#include "MapDefs.h"
#include "NetPlacement.h"
#include "NetTables.h"
#include "GISUtils.h"
#include "ParamDefs.h"
#include "XESConstants.h"
#include "CompGeomDefs3.h"
#include "MeshAlgs.h"
#include "STLUtils.h"
#include "MathUtils.h"

// Move a bridge N meters to simlify it
#define	BRIDGE_TURN_SIMPLIFY	20

#if 0
bool	HalfedgeIsSeparated(Pmwx::Halfedge_handle he)
{
	for (GISNetworkSegmentVector::iterator seg = he->data().mSegments.begin(); seg != he->data().mSegments.end(); ++seg)
	{
		if (IsSeparatedHighway(seg->mFeatType)) return true;
	}
	for (GISNetworkSegmentVector::iterator seg = he->twin()->data().mSegments.begin(); seg != he->twin()->data().mSegments.end(); ++seg)
	{
		if (IsSeparatedHighway(seg->mFeatType)) return true;
	}
	return false;
}

void	MakeHalfedgeOneway(Pmwx::Halfedge_handle he)
{
	for (GISNetworkSegmentVector::iterator seg = he->data().mSegments.begin(); seg != he->data().mSegments.end(); ++seg)
	{
		seg->mFeatType = SeparatedToOneway(seg->mFeatType);
	}
}
#endif

void	CalcRoadTypes(Pmwx& ioMap, const DEMGeo& inElevation, const DEMGeo& inUrbanDensity, const DEMGeo& inTemp, const DEMGeo& inRain, ProgressFunc inProg)
{
	int kill = 0;
	for (Pmwx::Halfedge_iterator edge = ioMap.halfedges_begin(); edge != ioMap.halfedges_end(); ++edge)
	for (int i = 0    ; i < edge->data().mSegments.size(); ++i)
	for (int j = i + 1; j < edge->data().mSegments.size(); ++j)
	{
		if(edge->data().mSegments[i] == edge->data().mSegments[j])
		{
			edge->data().mSegments.erase(edge->data().mSegments.begin() + j);
			--j;
			++kill;
		}	
	}
	if(kill > 0)
		printf("Killed %d duplicate roads.\n", kill);

	double	total = ioMap.number_of_faces() + ioMap.number_of_halfedges();
	int		ctr = 0;

	if (inProg) inProg(0, 1, "Calculating Road Types", 0.0);

#if 0
	for (Pmwx::Face_iterator face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if (!face->data().IsWater())
	{
		if (inProg && total && (ctr % 1000) == 0) inProg(0, 1, "Calculating Road Types", (double) ctr / total);

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

		for (Pmwx::Hole_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
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
			for (Pmwx::Hole_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
			{
				iter = stop = *hole;
				do {
					MakeHalfedgeOneway(iter);
					++iter;
				} while (iter != stop);
			}

		}
	}
#endif	

	for (Pmwx::Halfedge_iterator edge = ioMap.halfedges_begin(); edge != ioMap.halfedges_end(); ++edge, ++ctr)
	{
		if (inProg && total && (ctr % 1000) == 0) inProg(0, 1, "Calculating Road Types", (double) ctr / total);

		double	x1 = CGAL::to_double(edge->source()->point().x());
		double	y1 = CGAL::to_double(edge->source()->point().y());
		double	x2 = CGAL::to_double(edge->target()->point().x());
		double	y2 = CGAL::to_double(edge->target()->point().y());
		double	startE = inElevation.value_linear(x1,y1);
		double	endE = inElevation.value_linear(x2, y2);
		double	urbanS = inUrbanDensity.value_linear(x1, y1);
		double	urbanE = inUrbanDensity.value_linear(x2, y2);
		double	rainS = inRain.value_linear(x1,y1);
		double	rainE = inRain.value_linear(x2,y2);
		double	tempS = inRain.value_linear(x1,y1);
		double	tempE = inRain.value_linear(x2,y2);


		double	dist = LonLatDistMeters(x1,y1,x2,y2);
		if (dist <= 0.0) continue;

		double gradient = fabs(startE - endE) / dist;
		double urban = (urbanS + urbanE) * 0.5;
		double rain = (rainS + rainE) * 0.5;
		double temp = (tempS + tempE) * 0.5;

		int zl = edge->face()->data().GetZoning();
		int zr = edge->twin()->face()->data().GetZoning();

		for (GISNetworkSegmentVector::iterator seg = edge->data().mSegments.begin(); seg != edge->data().mSegments.end(); ++seg)
		{
			// GRADIENT BRIDGES ARE TURNED OFF!!  We do NOT have the calculations
			// to do these right. :-(
//			bool bridge = false; // gradient > gNetFeatures[seg->mFeatType].max_gradient;
//			if (edge->face()->IsWater() && edge->twin()->face()->IsWater())
//				bridge = true;


			seg->mRepType = NO_VALUE;
			for (Feature2RepInfoTable::iterator p = gFeature2Rep.begin(); p != gFeature2Rep.end(); ++p)
			{
				Feature2RepInfo& r(p->second);
				if(p->first == seg->mFeatType)
				if(r.min_density == r.max_density || (r.min_density <= urban && urban <= r.max_density))
				if(r.rain_min == r.rain_max || (r.rain_min <= rain && rain <= r.rain_max))
				if(r.temp_min == r.temp_max || (r.temp_min <= temp && temp <= r.temp_max))
				if(r.zoning_left == NO_VALUE || r.zoning_left == zl)
				if(r.zoning_right == NO_VALUE || r.zoning_right == zr)				
				{
					seg->mRepType = p->second.rep_type;
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

//double		Net_JunctionInfo_t::GetMatchingAngle(Net_ChainInfo_t * chain1, Net_ChainInfo_t * chain2)
//{
//	Vector3	vec1 = chain1->vector_to_junc(this);
//	Vector3	vec2 = chain2->vector_to_junc(this);
//	vec1.normalize();
//	vec2.normalize();
//	double dot = -(vec1.dot(vec2));
//	if (dot >= 0.0)
//	if (chain1->rep_type == chain2->rep_type)
//		dot += 1.0;
//	return dot;
//}

int			Net_JunctionInfo_t::GetLayerForChain(Net_ChainInfo_t * me)
{
	if(me->start_junction == this) return me->start_layer;
	if(me->end_junction == this) return me->end_layer;	
		Assert(!"Not found");
	return 0;
}

void		Net_JunctionInfo_t::SetLayerForChain(Net_ChainInfo_t * me, int l)
{
	if(me->start_junction == this) me->start_layer = l;
	else if(me->end_junction == this) me->end_layer = l;
	else	Assert(!"Not found");
}

void	Net_ChainInfo_t::reverse(void)
{
	swap(start_junction, end_junction);
	swap(start_layer, end_layer);
	for (int n = 0; n < shape.size() / 2; ++n)
	{
		swap(shape[n], shape[shape.size()-n-1]);
//		swap(agl[n], agl[shape.size()-n-1]);
//		swap(power_crossing[n], power_crossing[shape.size()-n-1]);
	}
}

//int				Net_ChainInfo_t::pt_count(void)
//{
//	return	shape.size() + 2;
//}
//
//int				Net_ChainInfo_t::seg_count(void)
//{
//	return shape.size() + 1;
//}
//
//Point3		Net_ChainInfo_t::nth_pt(int n)
//{
//	if (n == 0) return start_junction->location;
//	if (n > shape.size()) return end_junction->location;
//	return shape[n-1];
//}

//double		Net_ChainInfo_t::nth_agl(int n)
//{
//	if (n == 0) return start_junction->agl;
//	if (n > shape.size()) return end_junction->agl;
//	return agl[n-1];
//}

//Segment3		Net_ChainInfo_t::nth_seg(int n)
//{
//	return Segment3(nth_pt(n), nth_pt(n+1));
//}

//Vector3		Net_ChainInfo_t::vector_to_junc(Net_JunctionInfo_t * junc)
//{
//	if (junc == start_junction)
//		return Vector3(nth_pt(1), nth_pt(0));
//	else
//		return Vector3(nth_pt(pt_count()-2),nth_pt(pt_count()-1));
//}
//
//Vector2		Net_ChainInfo_t::vector_to_junc_flat(Net_JunctionInfo_t * junc)
//{
//	Vector3 foo = vector_to_junc(junc);
//	return Vector2(foo.dx, foo.dy);
//}

Net_JunctionInfo_t *	Net_ChainInfo_t::other_junc(Net_JunctionInfo_t * junc)
{
	return (junc == start_junction) ? end_junction : start_junction;
}

//double		Net_ChainInfo_t::meter_length(int pt_start, int pt_stop)
//{
//	// 0, pts-1 gives total length, 0 1 gives first seg len
//	double total = 0.0;
//
//	for (int n = pt_start; n < pt_stop; ++n)
//	{
//		Point3 p1 = nth_pt(n  );
//		Point3 p2 = nth_pt(n+1);
//		total += LonLatDistMeters(p1.x, p1.y, p2.x, p2.y);
//	}
//	return total;
//}
//
//double		Net_ChainInfo_t::dot_angle(int ctr_pt)
//{
//	Point3	p1(nth_pt(ctr_pt-1));
//	Point3	p2(nth_pt(ctr_pt  ));
//	Point3	p3(nth_pt(ctr_pt+1));
//
//	Vector2	v1(Point2(p1.x, p1.y), Point2(p2.x, p2.y));
//	Vector2	v2(Point2(p2.x, p2.y), Point2(p3.x, p3.y));
//
//	v1.normalize();
//	v2.normalize();
//	return v1.dot(v2);
//}
//
//void					Net_ChainInfo_t::split_seg(int n, double rat)
//{
//	Segment3	seg;
//	seg.p1 = nth_pt(n  );
//	seg.p2 = nth_pt(n+1);
////	double	g1 = nth_agl(n  );
////	double	g2 = nth_agl(n+1);
//
//	shape.insert(shape.begin()+n,seg.midpoint(rat));
////	agl.insert(agl.begin()+n, g1 * (1.0-rat) + g2 * rat);
////	power_crossing.insert(power_crossing.begin()+n, false);
//}

#pragma mark -

#define REDUCE_SHAPE_ANGLE 	0.984807753012208	// 0.9961946980917455
#define MAX_CUT_DIST		2000

// This routine removes shape points that form less than a certain cosine dot angle, as long
// as the two edges forming the angle are < 2 km.  We use this to reduce the number of nodes...
// in particular, shape points that were due to the original crossing of a road with some
// land use change or other arbitrary vector feature will get reduced if they are no longer
// doing any good.
int	NukeStraightShapePoints(Net_ChainInfoSet& ioChains)
{
	int reduces = 0;
	for (Net_ChainInfoSet::iterator i = ioChains.begin(); i != ioChains.end(); ++i)
	{
		Net_ChainInfo_t * c = *i;
		for (int v = 0; v < c->shape.size();)
//		if (!c->power_crossing[v])							// Do not nuke power crossings - we flag them on purpsoe and need to preserve them in the export!!
		{
			Point2 * p1, * p2, * p3;
			p2 = &c->shape[v];
			if (v == 0)
				p1 = &c->start_junction->location;
			else
				p1 = &c->shape[v-1];
			if (v == (c->shape.size()-1))
				p3 = &c->end_junction->location;
			else
				p3 = &c->shape[v+1];
			Vector2 v1(*p1, *p2);
			Vector2 v2(*p2, *p3);

			v1.dx *= (DEG_TO_MTR_LAT * cos(p2->y() * DEG_TO_RAD));
			v2.dx *= (DEG_TO_MTR_LAT * cos(p2->y() * DEG_TO_RAD));
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
//				c->agl.erase(c->agl.begin()+v);
//				c->power_crossing.erase(c->power_crossing.begin()+v);
				++reduces;
			} else
				++v;
		}
	}
	return reduces;
}


// This routine takes a network and combines chains that are contiguous through a junction, reducing
// junctions and forming longer chains.
void	OptimizeNetwork(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& outChains, bool water_only)
{
	int	total_merged = 0;
	int total_removed = 0;

	// First go through and consider every jucntion...if it has 2 items and they're the same and not a
	// loop, we can coalesce.
	for (Net_JunctionInfoSet::iterator junc = ioJunctions.begin(); junc != ioJunctions.end(); ++junc)
	{
		Net_JunctionInfo_t * me = (*junc);
		if (me->chains.size() == 2)
		{
			Net_ChainInfoSet::iterator i = me->chains.begin();
			Net_ChainInfo_t * sc = *i++;
			Net_ChainInfo_t * ec = *i++;

			if (sc->over_water || !water_only)
			if (sc != ec && sc->rep_type == ec->rep_type &&					// Ignore layer?  YES!  If we only have 2 roads coming together, assume that differing layer does 
						    sc->export_type == ec->export_type &&			// NOT mean that they really are hanging off.
						    sc->over_water == ec->over_water &&
//							sc->draped == ec->draped &&
				sc->start_junction != sc->end_junction &&
				ec->start_junction != ec->end_junction &&
				sc->other_junc(me) != ec->other_junc(me))
			{
				// Organize so start chain feeds into end chain directionally, with
				// us as the middle junction.
				
				if(sc->start_junction == me && ec->end_junction == me)
					swap(sc,ec);

				if(!IsOneway(sc->rep_type) && sc->end_junction != me)	sc->reverse();
				if(!IsOneway(ec->rep_type) && ec->start_junction != me)	ec->reverse();

				if(sc->end_junction == me && ec->start_junction == me)
				{
					// These junctions cap the new complete chain.
					Net_JunctionInfo_t * sj = sc->start_junction;
					Net_JunctionInfo_t * ej = ec->end_junction;
					Assert (sj != me);
					Assert (ej != me);
					// Accumulate all shape points.
					sc->shape.push_back(me->location);
//					sc->agl.push_back(me->agl);
//					sc->power_crossing.push_back(me->power_crossing);
					sc->shape.insert(sc->shape.end(), ec->shape.begin(),ec->shape.end());
//					sc->agl.insert(sc->agl.end(), ec->agl.begin(),ec->agl.end());
//					sc->power_crossing.insert(sc->power_crossing.end(), ec->power_crossing.begin(), ec->power_crossing.end());
					// We no longer have points.
					me->chains.clear();
					// End junction now has first chain instead of second.
					ej->chains.erase(ec);
					ej->chains.insert(sc);
					// Start chain now ends at end junction, not us.
					sc->end_junction = ej;
					sc->end_layer = ec->end_layer;
					// Nuke second chain, it's useless.
					delete ec;
					outChains.erase(ec);
					++total_merged;
				}
			}
		}
	}

	// Now go through and take out the trash...schedule for deletion every 0-valence
	// junction.
	Net_JunctionInfoSet	deadJuncs;
	for (Net_JunctionInfoSet::iterator junc = ioJunctions.begin(); junc != ioJunctions.end(); ++junc)
	{
		if ((*junc)->chains.empty())
		{
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
	int s = NukeStraightShapePoints(outChains);
	printf("Optimize: %d merged, %d removed, %d straight.\n", total_merged, total_removed, s);

}

typedef pair<Net_JunctionInfo_t *,Net_JunctionInfo_t *>	JuncPair;

struct sort_by_sqr_dist {
	bool operator()(const JuncPair& lhs, const JuncPair& rhs) const
	{
		double d1 = lhs.first->location.squared_distance(lhs.second->location);
		double d2 = rhs.first->location.squared_distance(rhs.second->location);
		return d1 < d2;
	}
};

struct sort_by_y {
	bool operator()(Net_JunctionInfo_t * const & p1, Net_JunctionInfo_t * const & p2) const 
	{
		return p1->location.y() < p2->location.y(); 
	}
};

inline bool within_box(const Point2& p1, const Point2& p2, double d)
{
	if(fabs(p1.x() - p2.x()) <= d)
	if(fabs(p1.y() - p2.y()) <= d)
		return true;
	return false;
}

typedef multiset<Net_JunctionInfo_t *, sort_by_y>	y_sorted_set;

void	MergeNearJunctions(Net_JunctionInfoSet& juncs, Net_ChainInfoSet& chains, double dist)
{
//		ValidateNetworkTopology(juncs,chains);

	printf("Before merge: %d juncs, %d chains.\n", juncs.size(), chains.size());

	while(1)
	{

		bool did_work = false;
		y_sorted_set	sorted_juncs;
		
		copy(juncs.begin(),juncs.end(),set_inserter(sorted_juncs));

		vector<JuncPair> kill;
		for(y_sorted_set::iterator i = sorted_juncs.begin(); i != sorted_juncs.end(); ++i)
		{
			y_sorted_set::iterator j(i);
			++j;
			while(j != sorted_juncs.end() && (((*j)->location.y() - (*i)->location.y()) < (2.0*dist)))
			{
//				printf("Measuring: 0x%08x, 0x%08x\n", (*i), (*j));
			
				if(within_box((*i)->location,(*j)->location,dist))
				{
					kill.push_back(JuncPair(*i,*j));
				}
				++j;
			}
		}
		sort(kill.begin(),kill.end(), sort_by_sqr_dist());

		for(vector<JuncPair>::iterator jp = kill.begin(); jp != kill.end(); ++jp)
		{
//			printf("Considering: 0x%08x, 0x%08x\n", jp->first, jp->second);			
			if(juncs.count(jp->first) && juncs.count(jp->second))
			if(within_box(jp->first->location,jp->second->location,dist))
			{
//				printf("Killing: 0x%08x, 0x%08x\n", jp->first, jp->second);			
				list<Net_ChainInfo_t *>	dead;
				for(Net_ChainInfoSet::iterator c = jp->first->chains.begin(); c != jp->first->chains.end(); ++c)
				if((*c)->other_junc(jp->first) == jp->second)
					dead.push_back(*c);

				copy(dead.begin(),dead.end(),set_eraser(chains));
				copy(dead.begin(),dead.end(),set_eraser(jp->first->chains));
				
				jp->first->location = Point2(
								(jp->first->location.x() + jp->second->location.x()) * 0.5,
								(jp->first->location.y() + jp->second->location.y()) * 0.5);
				copy(jp->second->chains.begin(),jp->second->chains.end(), set_inserter(jp->first->chains));
				
				for(Net_ChainInfoSet::iterator c = jp->second->chains.begin(); c != jp->second->chains.end(); ++c)
				{
					if((*c)->start_junction == jp->second) (*c)->start_junction = jp->first;
					if((*c)->end_junction == jp->second) (*c)->end_junction = jp->first;
				}
				
				DebugAssert(juncs.count(jp->second));
				juncs.erase(jp->second);
				delete jp->second;
				did_work = true;
				// This was serious paranoia in inital implementation, but don't even have in dev, makes alg very slow.
		//		ValidateNetworkTopology(juncs,chains);		
			}
		}	
	#if DEV
		ValidateNetworkTopology(juncs,chains);
	#endif
		if(!did_work)	
			break;
		else
			printf("After merge: %d juncs, %d chains.\n", juncs.size(), chains.size());
	}
	
	Net_JunctionInfo_t * bad_a = NULL, * bad_b = NULL;

#if DEV	
	for(Net_ChainInfoSet::iterator c = chains.begin(); c != chains.end(); ++c)
	{
		if(within_box((*c)->start_junction->location, (*c)->end_junction->location, dist))
		{
			bad_a = (*c)->start_junction; bad_b = (*c)->end_junction;
			printf("%p: %f,%f to %f, %f\n", (*c), 
						(*c)->start_junction->location.x(),
						(*c)->start_junction->location.y(),
						(*c)->end_junction->location.x(),
						(*c)->end_junction->location.y());
			printf("ERROR: junctions too close together (%p, %p).\n", 
						(*c)->start_junction,(*c)->end_junction);
		}
	}
#endif

/*	
	{
		y_sorted_set	sorted_juncs;
		
		copy(juncs.begin(),juncs.end(),set_inserter(sorted_juncs));

		printf("Bad a: %d\n", sorted_juncs.count(bad_a));
		printf("Bad a: %d\n", sorted_juncs.count(bad_b));

		vector<JuncPair> kill;
		for(y_sorted_set::iterator i = sorted_juncs.begin(); i != sorted_juncs.end(); ++i)
		{
			y_sorted_set::iterator j(i);
			++j;
			while(j != sorted_juncs.end() && (((*j)->location.y() - (*i)->location.y()) < (2.0*dist)))
			{
				if(*i == bad_a || *j == bad_a ||
				   *i == bad_b || *j == bad_b)
				printf("Measuring: 0x%08x, 0x%08x\n", (*i), (*j));
			
				if(within_box((*i)->location,(*j)->location,dist))
				{
					if(*i == bad_a || *j == bad_a ||
					   *i == bad_b || *j == bad_b)
						printf("Killing: 0x%08x, 0x%08x\n", (*i), (*j));
					kill.push_back(JuncPair(*i,*j));
				}
				++j;
			}
		}
	}
*/	
}




// This routine forms an original topology level 1 network from our planar map.
void	BuildNetworkTopology(Pmwx& inMap, CDT& /*inMesh*/, Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains)
{
	outJunctions.clear();
	outChains.clear();

	typedef	map<Pmwx::Vertex *, Net_JunctionInfo_t*>		JunctionTableType;
	typedef	map<Pmwx::Halfedge_handle, Net_ChainInfo_t*>	ChainTableType;
	JunctionTableType										junctionTable;

	CDT::Face_handle	f;
	CDT::Locate_type	lt;
	int					li;

	/************ STEP 1 - BUILD THE BASIC NETWORK ************/
	for (Pmwx::Vertex_iterator v = inMap.vertices_begin(); v != inMap.vertices_end(); ++v)
	{
		bool has_any = false;
		Pmwx::Halfedge_around_vertex_circulator circ, stop;
		circ = stop = v->incident_halfedges();
		do {
			for (GISNetworkSegmentVector::iterator seg = circ->data().mSegments.begin(); seg != circ->data().mSegments.end(); ++seg)
			if (seg->mRepType != NO_VALUE)
			{
				has_any = true;
				goto hack;
			}

			for (GISNetworkSegmentVector::iterator seg = circ->twin()->data().mSegments.begin(); seg != circ->twin()->data().mSegments.end(); ++seg)
			if (seg->mRepType != NO_VALUE)
			{
				has_any = true;
				goto hack;
			}

		} while (++circ != stop);
		
	hack:
		if(!has_any)
			continue;
		
		Net_JunctionInfo_t * junc = new Net_JunctionInfo_t;
//		junc->vertical_locked = false;
		junc->location.x_ = CGAL::to_double(v->point().x());
		junc->location.y_ = CGAL::to_double(v->point().y());
//		f = inMesh.locate(v->point(), lt, li, f);
//		junc->location.z = HeightWithinTri(inMesh, f, v->point());
		
//		junc->power_crossing = false;
		junctionTable.insert(JunctionTableType::value_type(&*v,junc));
		outJunctions.insert(junc);
		//printf("+");
	}
	for (Pmwx::Halfedge_iterator e = inMap.halfedges_begin(); e != inMap.halfedges_end(); ++e)
	for (GISNetworkSegmentVector::iterator seg = e->data().mSegments.begin(); seg != e->data().mSegments.end(); ++seg)
	if (seg->mRepType != NO_VALUE)
	{
		Net_ChainInfo_t *	chain = new Net_ChainInfo_t;
		chain->rep_type = seg->mRepType;
		chain->export_type = NO_VALUE;
//		chain->draped = true;
		chain->over_water = e->face()->data().IsWater() && e->twin()->face()->data().IsWater();
		chain->start_junction = junctionTable[&*e->source()];
		chain->end_junction = junctionTable[&*e->target()];
		chain->start_junction->chains.insert(chain);
		chain->end_junction->chains.insert(chain);
		chain->start_layer = seg->mSourceHeight;
		chain->end_layer = seg->mTargetHeight;
		outChains.insert(chain);
		//printf("|");
	}

	

//	CountNetwork(outJunctions, outChains);
//	int nukes = NukeStraightShapePoints(outChains);
//	CountNetwork(outJunctions, outChains);
//	printf("Nuked %d shape points.\n", nukes);

	// Now we need to start sorting out the big mess we have - we have a flat topology and in reality
	// highways don't work that way!
}

// This routine purges the memory used for a network set.
void	CleanupNetworkTopology(Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains)
{
	for (Net_JunctionInfoSet::iterator i = outJunctions.begin(); i != outJunctions.end(); ++i)
		delete (*i);

	for (Net_ChainInfoSet::iterator j = outChains.begin(); j != outChains.end(); ++j)
		delete (*j);
}

// This routine checks the connectivity of the network for linkage and ptr errors.
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

// This routine prints the counts for all junctions, chains and shape points.
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
void	DrapeRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, CDT& inMesh, bool only_power_lines)
{
	int total = 0;
	int added = 0;
	vector<Point3>	all_pts,	these_pts;

	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	if(!only_power_lines || gNetReps[(*chainIter)->rep_type].use_mode == use_Power)
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
				MarchHeightStart(inMesh, CDT::Point(CGAL::to_double(pt.x), CGAL::to_double(pt.y)), info);
			}
			else
			{
				MarchHeightGo(inMesh, CDT::Point(CGAL::to_double(pt.x), CGAL::to_double(pt.y)), info, these_pts);
				#if 0 && DEV
					Point3 prev(chain->nth_pt(n-1));
					Bbox2	lim(prev.x,prev.y,pt.x,pt.y);
					for(int i = 0; i < these_pts.size(); ++i)
					{
						DebugAssert(lim.contains(Point2(these_pts[i].x,these_pts[i].y)));
					}
				#endif
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
	printf("Promote: before: %llu juncs, %llu chains\n", (unsigned long long)ioJunctions.size(), (unsigned long long)ioChains.size());
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
			new_chain->start_layer = chain->end_layer;
			new_chain->end_junction = chain->end_junction;
			new_chain->end_layer = chain->end_layer;
			new_chain->rep_type = chain->rep_type;
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
	printf("Promote: after: %llu juncs, %llu chains\n", (unsigned long long)ioJunctions.size(), (unsigned long long)ioChains.size());
	ValidateNetworkTopology(ioJunctions, ioChains);

}

Net_JunctionInfo_t * CloneJunction (Net_JunctionInfo_t * j)
{
	Net_JunctionInfo_t * n = new Net_JunctionInfo_t;
	n->location = j->location;
	n->agl = j->agl;
	n->power_crossing = j->power_crossing;
	n->vertical_locked = j->vertical_locked;
	return n;
}

void	MigrateChain(Net_ChainInfo_t * chain, Net_JunctionInfo_t * old_j, Net_JunctionInfo_t * new_j)
{
	new_j->chains.insert(chain);
	old_j->chains.erase(chain);
	DebugAssert(chain->start_junction == old_j || chain->end_junction == old_j);
	if (chain->start_junction == old_j) chain->start_junction = new_j;
	if (chain->end_junction == old_j) chain->end_junction = new_j;
}

// This is like the original vertical partition, but...we only pull out power lines...we do this for the new
// draping format, which REQUIRES shared junctions for stacks of bridges, etc.
void	VerticalPartitionOnlyPower(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	int	total_power_crossings = 0;

	vector<Net_JunctionInfo_t *>		new_juncs;		// This is any new junctions created during this partitioning phase.

	// Iterate across all junctions - since we separate power from non-power...
	for (Net_JunctionInfoSet::iterator juncIter = ioJunctions.begin(); juncIter != ioJunctions.end(); ++juncIter)
	{
		Net_JunctionInfo_t * junc = *juncIter;

		// STEP 1 - PREP
		// We go through and find out who is around and pull out various useful chains.  When we're done we'll know some
		// of the main groups we must make and also roughly how the highways relate.

			vector<Net_ChainInfo_t *>						power_lines;									// All of the segments...
			bool											has_power = false;
			bool											has_nonpower = false;

		for (Net_ChainInfoSet::iterator chain = junc->chains.begin(); chain != junc->chains.end(); ++chain)
		{
			if (gNetReps[(*chain)->rep_type].use_mode == use_Power)
			{
				has_power = true;
				power_lines.push_back(*chain);
			}
			else
				has_nonpower = true;
		}

		if (has_power && has_nonpower)
		{
			junc->power_crossing = true;
			Net_JunctionInfo_t * only_power = CloneJunction(junc);
			new_juncs.push_back(only_power);		
			
			for (vector<Net_ChainInfo_t *>::iterator pl = power_lines.begin(); pl != power_lines.end(); ++pl)
			{
				MigrateChain(*pl, junc, only_power);
			}
			
			++total_power_crossings;
		}
		
		map<int, int>	layer_map;
		int best = 0;
		for(Net_ChainInfoSet::iterator chain = junc->chains.begin(); chain != junc->chains.end(); ++chain)
		{
			int l = junc->GetLayerForChain(*chain);
			int c = ++layer_map[l];
			if (c >= 2 && l > best)
				best = l;
		}
		
		
		
		for(Net_ChainInfoSet::iterator chain = junc->chains.begin(); chain != junc->chains.end(); ++chain)		
		{
			int l = junc->GetLayerForChain(*chain);
			if(layer_map[l] < 2)
				junc->SetLayerForChain(*chain, best);
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

			double	displacement = LonLatDistMeters(p2.x, p2.y, p_mid.x(), p_mid.y());

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
//		if ((*chain)->start_junction->vertical_locked)	agl1 = (*chain)->start_junction->agl;
//		if ((*chain)->end_junction->vertical_locked)	agl2 = (*chain)->end_junction->agl;

		int	bridge_rule = FindBridgeRule((*chain)->rep_type, total_len, smallest_len, biggest_len, (*chain)->seg_count(), sharpest_turn, agl1, agl2);
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
			(*chain)->draped = false;
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
			DebugAssert(((*chain))->over_water == false || ((*chain))->export_type == -1);	// If it's water, it better be hidden or locked down

			if (((*chain))->other_junc(*junc)->vertical_locked)
			{
				shortest_len = min(shortest_len, ((*chain))->meter_length(0, ((*chain))->seg_count()));
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
				Net_JunctionInfo_t * other = ((*chain))->other_junc(current);
				if (other->vertical_locked)
				{
					double seg_len = ((*chain))->meter_length(0, ((*chain))->seg_count());

					double var = seg_len * gNetReps[((*chain))->rep_type].max_slope;

					min_msl = max(min_msl, other->location.z - var);
					max_msl = min(max_msl, other->location.z + var);
					max_agl = max(max_agl,  other->agl);
				} else {
					double len = current_lock_dist + ((*chain))->meter_length(0, ((*chain))->seg_count());
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
	if (gNetReps[(*chain)->rep_type].use_mode != use_Power)
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

#endif

void	AssignExportTypes(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains)
{
	for (Net_ChainInfoSet::iterator chain = ioChains.begin(); chain != ioChains.end(); ++chain)
	if ((*chain)->export_type == 0)
	{
		/*
		bool above_ground = false;
		for (int n = 0; n < (*chain)->pt_count(); ++n)
		{
			if ((*chain)->nth_agl(n) > 0.0)
			{
				above_ground = true;
				break;
			}
		}
		
		(*chain)->export_type = above_ground ?  gNetReps[(*chain)->rep_type].export_type_overpass : gNetReps[(*chain)->rep_type].export_type_normal;
		*/
		(*chain)->export_type = gNetReps[(*chain)->rep_type].export_type_draped;
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

#if 0

void	SpacePowerlines(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, double ideal_dist_m, double max_dip)
{
	int		total_pts = 0;
	int		total_xos = 0;
	int		kill_pts = 0;
	int		kill_xos = 0;
	int		end_crossings = 0;

//	gMeshPoints.clear();

	for (Net_ChainInfoSet::iterator chainIter = ioChains.begin(); chainIter != ioChains.end(); ++chainIter)
	if (gNetReps[(*chainIter)->rep_type].use_mode == use_Power)
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

				double	displacement = LonLatDistMeters(p2.x, p2.y, p_mid.x(), p_mid.y());

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

#endif
