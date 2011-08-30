/*
 * Copyright (c) 2010, Laminar Research.
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

#include "NetAlgs.h"
#include "NetHelpers.h"
#include "MapTopology.h"
#include "XESConstants.h"
#include "GISTool_Globals.h"
#include "DEMDefs.h"
#include "GISUtils.h"
#if OPENGL_MAP && DEV
	#include "RF_Selection.h"
#endif

#define	MIN_DIST_FOR_TYPE 0.005

int	KillTunnels(Pmwx& ioMap)
{
	int k = 0;
	for(Pmwx::Halfedge_iterator e = ioMap.halfedges_begin(); e != ioMap.halfedges_end(); ++e)
	for(GISNetworkSegmentVector::iterator s = e->data().mSegments.begin(); s != e->data().mSegments.end();)
	if(s->mSourceHeight < 0 || s->mTargetHeight < 0)
	{
		s = e->data().mSegments.erase(s);		
//		debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,0,0,1,1,0);
		++k;
	} else
		++s;
	return k;
}


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


/******************************************************************************************************************************************************
 *
 ******************************************************************************************************************************************************/

#define E_FLOT 160000			// Something ends by just floating in the air.
#define E_TERM	80000			// Dead-end highway when we could have a plug.
#define E_WATR	40000			// Bridge over water at level 0
#define E_RAMP	20000			// A bridge heads straight to zero right AFTER the bridge, which will cause an embankment to smooth things.
#define E_CRSH  10000			// Highway one-way is wrong, causes head-ons.

#define E_HPLG	1000			// Highway forms a plug junction, generally not desireable.

#define E_VERT	10				// Up & down action?
#define E_ROAD	1				// Flying city streets...not preferable.

int score_for_junction(Pmwx::Vertex_handle v)
{
	map<int,vector<Pmwx::Halfedge_handle> >	junc;
	int total = levelize_junction(v,junc);

	int score = 0;
	bool watr = false;
	bool flot = false;
	bool term = false;
	bool crsh = false;
	bool plug = false;
	bool ramp = false;
	bool road = false;
	double angle = 0;

	vector<Pmwx::Halfedge_handle>::iterator r;

	for(map<int,vector<Pmwx::Halfedge_handle> >::iterator l = junc.begin(); l != junc.end(); ++l)
	{
		if(l->first == 0)
		{
			for(r = l->second.begin(); r != l->second.end(); ++r)
			if(pmwx_categorize(*r) == pmwx_Wet)
				watr = true;
		}
		else
		{
			for(r = l->second.begin(); r != l->second.end(); ++r)
			if(get_he_road_use(*r) == use_Street)
				road = true;

			if(l->second.size() == 1)
				flot = true;
		}

		if(total > 1)
		if(l->second.size() == 1)
		if(is_level_highway(l->second))
			term = true;

		if(l->second.size() == 2)
		if(is_level_highway(l->second))
		{
			if(l->second[0]->target() != l->second[1]->source() &&
			   l->second[0]->source() != l->second[1]->target())
			   crsh = true;
			else
			{
				double dot = get_he_road_dot(l->second[0],l->second[1]);
				angle += acos(dot) * RAD_TO_DEG;
				if(dot < 0.5)			// SERIOUSLY sharp turn in a kinky highway?!?  Uh, might as well have a real plug, this is srsly not good.
					plug = true;
			}
		}

		if(l->second.size() > 2)
		if(is_level_highway(l->second))
		{
			if(!is_level_highway_y(v, l->second))
				plug = true;
			else
			{
				for(int i = 0; i < l->second.size(); ++i)
				for(int j = i+1; j < l->second.size(); ++j)
					angle += acos(get_he_road_dot(l->second[i],l->second[j])) * RAD_TO_DEG;
			}
		}
		if(is_level_mixed(l->second))
			plug = true;

		for(r = l->second.begin(); r != l->second.end(); ++r)
		if((*r)->data().mSegments.front().mSourceHeight != (*r)->data().mSegments.front().mTargetHeight)
		  score += E_VERT;


//		RAMP CHECK IS OFF.  Ramp check drives things crazy.  For example, we will tend to get a city street that goes over
//		a highway STAYING at bridge level and bridging over the entire road grid because there are no free nodes to let it 'drop'.
//		if(l->first == 0)
//		for(r = l->second.begin(); r != l->second.end(); ++r)
//		{
//			Pmwx::Vertex_handle o = ((*r)->source() == v) ? (*r)->target() : (*r)->source();
//			double level_at_o = get_he_level_at(*r, o);
//			if(level_at_o > 0.0)
//			{
//				map<int,vector<Pmwx::Halfedge_handle> >	them;
//				if(levelize_junction(o,them))
//				if(!them.empty())
//				if(them.begin()->first != level_at_o)
//					ramp = true;
//
//			}
//		}
	}

	if(watr) score += E_WATR;
	if(flot) score += E_FLOT;
	if(term) score += E_TERM;
	if(crsh) score += E_CRSH;
	if(ramp) score += E_RAMP;
	if(plug) score += E_HPLG;
	if(road) score += E_ROAD;

	score += angle;

	return score;
}

int	max_code(int num_hes, int num_levels)
{
	int r = 1;
	while(num_hes--)
		r *= num_levels;
	return r;
}

void	apply_levels(int num_hes, int num_levels, int code, vector<int>& levels)
{
	levels.resize(num_hes);
	for(int d = 0; d < num_hes; ++d)
	{
		levels[d] = code % num_levels;
		code /= num_levels;
	}
}

void	apply_combos_to_roads(Pmwx::Vertex_handle v, const vector<Pmwx::Halfedge_handle>& hes, const vector<int>& levels)
{
	DebugAssert(hes.size() == levels.size());
	for(int n = 0; n < hes.size(); ++n)
		set_he_level_at(hes[n],v,levels[n]);
}

int	optimize_one_junction(Pmwx::Vertex_handle v)
{
	int score = score_for_junction(v);
	if(score < E_HPLG) return score;
	vector<Pmwx::Halfedge_handle>	he_list;
	Pmwx::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = v->incident_halfedges();
	do {
		if(he_has_any_roads(circ))	he_list.push_back(circ);
	} while (++circ != stop);

	if(he_list.empty()) return score;
	if(he_list.size() > 8) return score;

	// now stuff all number combos, score, retry.
	vector<int>		best;
	int				best_score=0;
		
	for(int l = 1; l <= he_list.size(); ++l)
	{
		int mc = max_code(he_list.size(), l);
		for(int code = 0; code < mc; ++code)
		{
			vector<int>	trial;
			apply_levels(he_list.size(),l,code,trial);
			apply_combos_to_roads(v,he_list, trial);
			int this_score = score_for_junction(v);
			if(best.empty() || this_score < best_score)
			{
				best = trial;
				best_score = this_score;
				// Ben says: in theory we shold never want more levels than half the number of incoming roads plus one (for an odd one out that can dangle).
				// Verify this here, maybe we can cut our l loop down and save the most expensive iterations?  But we might always need two levels because
				// water FORCES us up one.
//				DebugAssert(l <= max(2UL,(he_list.size()+1)/2));
				// Wait - WRONG.  If our neighbors are WAY up high, WE might need to be too!  That minimizes up-down shift (E_VERT).
			}
		}
	}
	DebugAssert(!best.empty());
	apply_combos_to_roads(v,he_list, best);
	return best_score;
}


#if OPENGL_MAP  && DEV
void	debug_network(Pmwx& io_map)
{
	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
	#if OPENGL_MAP
	if(gEdgeSelection.empty() || gEdgeSelection.count(e)|| gEdgeSelection.count(e->twin()))
	#endif
	{
		if(e->data().HasGroundRoads() || e->twin()->data().HasGroundRoads())
		if(!e->face()->is_unbounded() && !e->twin()->face()->is_unbounded())
		if(e->face()->data().IsWater() && e->twin()->face()->data().IsWater())
			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),0,0,1,0,0,1);

		bool bad_type = false;

		map<int,int>	src,trg;
		GISNetworkSegmentVector::iterator r;
		for (r=e->data().mSegments.begin(); r != e->data().mSegments.end(); ++r)
		{
			if(gNetReps.count(r->mRepType) == 0 ||
			   gNetReps[r->mRepType].use_mode == use_None)
				bad_type=true;
			src[(int) r->mSourceHeight]++;
			trg[(int) r->mTargetHeight]++;
		}
		for (r=e->twin()->data().mSegments.begin(); r != e->twin()->data().mSegments.end(); ++r)
		{
			if(gNetReps.count(r->mRepType) == 0 ||
			   gNetReps[r->mRepType].use_mode == use_None)
				bad_type=true;
			trg[(int) r->mSourceHeight]++;
			src[(int) r->mTargetHeight]++;
		}

		bool doubled = false;
		map<int,int>::iterator i;
		for(i=src.begin(); i != src.end(); ++i)
			if(i->second > 1) doubled = true;
		for(i=trg.begin(); i != trg.end(); ++i)
			if(i->second > 1) doubled = true;

		if(bad_type)
			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,0,1,1,0,1);
		else if(doubled)
			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,0,0,1,0,0);
		else if(e->data().mSegments.size() + e->twin()->data().mSegments.size() > 1)
			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,1,0,1,1,0);


	}

	for(Pmwx::Vertex_iterator v = io_map.vertices_begin(); v != io_map.vertices_end(); ++v)
	#if OPENGL_MAP
	if(gVertexSelection.empty() || gVertexSelection.count(v))
	#endif
	{
		int score = score_for_junction(v);

			 if(score >= E_FLOT)		debug_mesh_point(cgal2ben(v->point()), 1,0,0);
		else if(score >= E_TERM)		debug_mesh_point(cgal2ben(v->point()), 1,1,0);
		else if(score >= E_CRSH)		debug_mesh_point(cgal2ben(v->point()), 1,0,1);
		else if(score >= E_WATR)		debug_mesh_point(cgal2ben(v->point()), 0,0,1);
		else if(score >= E_RAMP)		debug_mesh_point(cgal2ben(v->point()), 0,1,1);
		else if(score >= E_HPLG)		debug_mesh_point(cgal2ben(v->point()), 0,1,0);
	}
}
#endif

static void elevate_segments_to(GISNetworkSegmentVector& v, double h)
{
	for(GISNetworkSegmentVector::iterator i = v.begin(); i != v.end(); ++i)
	{
		i->mSourceHeight = max(i->mSourceHeight, h);
		i->mTargetHeight = max(i->mTargetHeight, h);
	}
}

static void strip_segments_to_one(GISNetworkSegmentVector& v)
{
	if(v.size() < 2) return;
	GISNetworkSegmentVector::iterator best = v.begin();
	for(GISNetworkSegmentVector::iterator i = v.begin(); i != v.end(); ++i)
		if(i->mFeatType < best->mFeatType)
			best = i;
	GISNetworkSegment_t keep = *best;
	v.clear();
	v.push_back(keep);
}

Pmwx::Vertex_handle apply_type_change_forward(Pmwx::Halfedge_handle he, int new_type, bool debug)
{
	if(get_he_rep_type(he) == new_type)
		return Pmwx::Vertex_handle();
	double wanted_dist = MIN_DIST_FOR_TYPE;
	while(wanted_dist > 0.0)
	{
		wanted_dist -= approx_he_len(he);
//		if(debug)
//		debug_mesh_line(cgal2ben(he->source()->point()),cgal2ben(he->target()->point()),1,1,1,0,1,0);
		set_he_rep_type(he,new_type);
		Pmwx::Halfedge_handle next = next_contig(he);
		if(next == Pmwx::Halfedge_handle())
			break;
		if(!matches_use(next,he))
			break;
		he = next;
	}
	return he->target();
}

Pmwx::Vertex_handle apply_type_change_reverse(Pmwx::Halfedge_handle he, int new_type, bool debug)
{
	if(get_he_rep_type(he) == new_type)
		return Pmwx::Vertex_handle();
	double wanted_dist = MIN_DIST_FOR_TYPE;
	while(wanted_dist > 0.0)
	{
		wanted_dist -= approx_he_len(he);
		set_he_rep_type(he,new_type);
//		if(debug)
//		debug_mesh_line(cgal2ben(he->source()->point()),cgal2ben(he->target()->point()),1,0,0,1,1,1);
		Pmwx::Halfedge_handle prev = prev_contig(he);
		if(prev == Pmwx::Halfedge_handle())
			break;
		if(!matches_use(prev,he))
			break;
		he = prev;
	}
	return he->source();
}


void apply_y_rules(Pmwx::Halfedge_handle trunk, Pmwx::Halfedge_handle left, Pmwx::Halfedge_handle right, set<Pmwx::Vertex_handle>& changed)
{
	int tr = trunk->data().mSegments.front().mRepType;
	int lr = left->data().mSegments.front().mRepType;
	int rr = right->data().mSegments.front().mRepType;

	Pmwx::Vertex_handle v;

	for(ForkRuleTable::iterator r = gForkRules.begin(); r != gForkRules.end(); ++r)
	if(r->trunk == tr && r->left == lr && r->right == rr)
	{
		if(trunk->target() == left->source())
		{
			DebugAssert(trunk->target() == right->source());
			// split
			v = apply_type_change_forward(left,r->new_left,false);	 if(v != Pmwx::Vertex_handle())	changed.insert(v);
			v = apply_type_change_forward(right,r->new_right,false); if(v != Pmwx::Vertex_handle())	changed.insert(v);
			v = apply_type_change_reverse(trunk,r->new_trunk,false); if(v != Pmwx::Vertex_handle())	changed.insert(v);
		}
		else
		{
			DebugAssert(left->target() == trunk->source());
			DebugAssert(right->target() == trunk->source());
			v = apply_type_change_reverse(left,r->new_left,false);		if(v != Pmwx::Vertex_handle())	changed.insert(v);
			v = apply_type_change_reverse(right,r->new_right,false);	if(v != Pmwx::Vertex_handle())	changed.insert(v);
			v = apply_type_change_forward(trunk,r->new_trunk,false);	if(v != Pmwx::Vertex_handle())	changed.insert(v);
		}
	}
}
//
//void apply_i_rules(Pmwx::Halfedge_handle prev, Pmwx::Halfedge_handle next)
//{
//	int pr = prev->data().mSegments.front().mRepType;
//	int nr = next->data().mSegments.front().mRepType;
//
//	for(ChangeRuleTable::iterator r = gChangeRules.begin(); r != gChangeRules.end(); ++r)
//	if(r->prev == pr && r->next == nr)
//	{
//		prev->data().mSegments.front().mRepType = r->new_prev;
//		next->data().mSegments.front().mRepType = r->new_next;
////		debug_mesh_line(cgal2ben(prev->source()->point()),cgal2ben(prev->target()->point()),1,0,0,1,0,0);
////		debug_mesh_line(cgal2ben(next->source()->point()),cgal2ben(next->target()->point()),0,1,0,0,1,0);
//	}
//}
//
void check_junction_highways(Pmwx::Vertex_handle v, set<Pmwx::Vertex_handle>& changed)
{
	map<int, vector<Pmwx::Halfedge_handle> > junc;
	int t = levelize_junction(v,junc);
	if(t >= 3)
	for(map<int, vector<Pmwx::Halfedge_handle> >::iterator l = junc.begin(); l != junc.end(); ++l)
	{
		if(is_level_highway_y(v,l->second) && is_level_highway(l->second))
		{
			for(int i = 0; i < 3; ++i)
			{
				int j = (i+1)%3;
				int k = (i+2)%3;
				if(l->second[i]->target() == v && l->second[j]->target() == v && l->second[k]->source() == v)
					apply_y_rules(l->second[k], l->second[j], l->second[i],changed);			// Merge
				else if(l->second[i]->target() == v && l->second[j]->source() == v && l->second[k]->source() == v)
					apply_y_rules(l->second[i], l->second[j], l->second[k],changed);			// Split
			}
		}
		else if (l->second.size() > 1 && is_level_mixed(l->second))
		{
			for(int i = 0; i < l->second.size(); ++i)
			if(l->second[i]->data().mSegments.back().mRepType == net_6City)
			{
				if(l->second[i]->source() == v)
				{
					Pmwx::Vertex_handle vv = apply_type_change_forward(l->second[i], net_4City,true);
					if(vv != Pmwx::Vertex_handle()) changed.insert(vv);
				}
				else
				{
					Pmwx::Vertex_handle vv = apply_type_change_reverse(l->second[i], net_4City,true);
					if(vv != Pmwx::Vertex_handle()) changed.insert(vv);
				}
			}
		}
	}
}

void repair_network(Pmwx& io_map)
{
	// FIRST: cut down any duplicate data.  Road code really needs to NOT have vertically stacked crap!

	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
	{
		strip_segments_to_one(e->data().mSegments);
		strip_segments_to_one(e->twin()->data().mSegments);
		if(!e->data().mSegments.empty() && !e->twin()->data().mSegments.empty())
		{
			if(e->data().mSegments.front().mFeatType < e->twin()->data().mSegments.front().mFeatType)
				e->twin()->data().mSegments.clear();
			else
				e->data().mSegments.clear();
		}
	}

	// NEXT: junction optimization...redo levels into junctions to minimize "weird shit".
	// We are going to save any junctions that had a crash...do not even TRY to fix them until
	// we have fixed all discontinuous junctions..otherwise we might not see the whole
	// contiguous on ramp.
	list<Pmwx::Vertex_handle> crsh_starts;

	for(Pmwx::Vertex_iterator v = io_map.vertices_begin(); v != io_map.vertices_end(); ++v)
	#if OPENGL_MAP && DEV
	if(gVertexSelection.empty() || gVertexSelection.count(v))
	#endif
	{
		int score = optimize_one_junction(v);

		#if OPENGL_MAP && DEV
		if(score >= E_RAMP)
			debug_mesh_point(cgal2ben(v->point()),1,0,0);
		#endif

		if(score >= E_CRSH && score < E_RAMP)
		{
			crsh_starts.push_back(v);
		}
	}

	multimap<double, list<Pmwx::Halfedge_handle> >	crshs;

	// Now: crash collection: run along each 'crash' vertex and collect the entire contiguous road
	// in both directions and stash it by length.

	for(list<Pmwx::Vertex_handle>::iterator v = crsh_starts.begin(); v != crsh_starts.end(); ++v)
	{
		set<Pmwx::Halfedge_handle>	processed;

		Pmwx::Halfedge_handle opp;
		Pmwx::Halfedge_around_vertex_circulator circ,stop;
		circ=stop=(*v)->incident_halfedges();
		do {
			if(!circ->data().mSegments.empty())
			if(processed.count(circ) == 0)
			{
				opp = next_contig(circ);
				if (opp != Pmwx::Halfedge_handle())
				{
					list<Pmwx::Halfedge_handle> k;
					processed.insert(circ);
					processed.insert(opp);
					double len = collect_contig<matches_any>(circ, k);
					crshs.insert(multimap<double,list<Pmwx::Halfedge_handle> >::value_type(len,k));
				}
			}
			if(!circ->twin()->data().mSegments.empty())
			if(processed.count(circ->twin()) == 0)
			{
				opp = prev_contig(circ->twin());
				if (opp != Pmwx::Halfedge_handle())
				{
					list<Pmwx::Halfedge_handle> k;
					processed.insert(circ->twin());
					processed.insert(opp);
					double len = collect_contig<matches_any>(circ->twin(), k);
					crshs.insert(multimap<double,list<Pmwx::Halfedge_handle> >::value_type(len,k));
				}
			}
		} while(++circ != stop);

		DebugAssert(!processed.empty());

	}

	// Now starting with the shortest crashed segments, try them in both uniform directions and set the whole
	// contiguous road to the direction that minimizes the fubar-ness of the end vertices.  Note that we have
	// to re-optimize the end vertices based on direction....we may have missed the best 'fit' since we were
	// going the wrong way.

	for(multimap<double,list<Pmwx::Halfedge_handle> >::iterator k = crshs.begin(); k != crshs.end(); ++k)
	{
//		for(list<Pmwx::Halfedge_handle>::iterator r = k->second.begin(); r != k->second.end(); ++r)
//			debug_mesh_line(cgal2ben((*r)->source()->point()),cgal2ben((*r)->target()->point()),1,0,0,0,1,0);
		set_forward(k->second);
		int score_f1 = optimize_one_junction(k->second.front()->source());
		int score_f2 = optimize_one_junction(k->second.back()->target());
		set_reverse(k->second);
		int score_r1 = optimize_one_junction(k->second.front()->source());
		int score_r2 = optimize_one_junction(k->second.back()->target());
//		printf("Fwd: %d,%d Rev: %d,%d\n",score_f1,score_f2,score_r1,score_r2);
		if(score_f1+score_f2 < score_r1+score_r2)
		{
			set_forward(k->second);
			optimize_one_junction(k->second.front()->source());
			optimize_one_junction(k->second.back()->target());
		}
	}

	// WONKY BRIDGE CHECK!!  We may have some roads that go from being OVER another road to ground in....zero time flat.
	// If the road is also draped, this is not so good.  So: fidn these edges and either lift the down side (making the
	// bridge longer, or if that won't work, split the edge).

	// We have THREE responses to a ground level junction connected by a single link to an overpass over a vector:
	// 1. if the ground level junction can be moved up one level (e.g. it isn't a real junction, just a pass-through and
	// there is nothing above) we can simply pull up the bridge a bit.
	// 2. we can subdivide the road and pull up half.
	// 3. we can do nothing.
	// We always take option 1 if we can, but we only subdivide when the ground-level junction is draped.  The assumption is
	// that if the road is graded at ground level, we can just run a bridge right out of the grading.
	list<Pmwx::Halfedge_handle>		need_split;

	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
	if(he_has_any_roads(e))
	{
		map<int,vector<Pmwx::Halfedge_handle> > over, gnd;

		double l1 = get_he_level_at(e,e->source());
		double l2 = get_he_level_at(e,e->target());
		if(l1 == 0.0 && l2 != 0.0)
		{
			if(levelize_junction(e->target(),over))
			if(over.begin()->first < l2)
			{
				levelize_junction(e->source(),gnd);
				DebugAssert(gnd.count(l1));
				bool gnd_draped = !is_level_graded(gnd[l1]);
				if(gnd[l1].size() != 2)
				{
					// "Red" case: the ground side of the junction we want to lift is a real junction (or the end of a
					// road).  We can't move it.  So we'll split.
					//debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),0.5,0,0,1,0,0);
					if(gnd_draped)
						need_split.push_back(get_he_with_roads(e));
				}
				else
				{
					if(gnd.count(l2))
					{
						// "Yellow" case: there is another road at our ground junction above us, so that if we 'lift' ourselves
						// we'll crash into something.
						//debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),0.5,0.5,0,1,1,0);
						if(gnd_draped)
							need_split.push_back(get_he_with_roads(e));
					}
					else
					{
						// "Green" case: on the ground it is just us going into our continuing road and nothing above us, so
						// we can lift our low end to fix the bridge.
						//debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),0,0.5,0,0,1,0);
						set_he_level_at(gnd[l1][0],e->source(),l2);
						set_he_level_at(gnd[l1][1],e->source(),l2);

					}
				}
			}
		}
		else if(l2 == 0.0 && l1 != 0.0)
		{
			if(levelize_junction(e->source(),over))
			if(over.begin()->first < l1)
			{
				levelize_junction(e->target(),gnd);
				DebugAssert(gnd.count(l2));
				bool gnd_draped = !is_level_graded(gnd[l2]);

				if(gnd[l2].size() != 2)
				{
					//debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,0,1,0.5,0,0.5);
					if(gnd_draped)
						need_split.push_back(get_he_with_roads(e));
				}
				else
				{
					if(gnd.count(l1))
					{
						//debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,0,1,0.5,0,0.5);
						if(gnd_draped)
							need_split.push_back(get_he_with_roads(e));
					}
					else
					{
						//debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),0,1,0,0,0.5,0);
						set_he_level_at(gnd[l2][0],e->target(),l1);
						set_he_level_at(gnd[l2][1],e->target(),l1);
					}
				}
			}
		}
	}
	for(list<Pmwx::Halfedge_handle>::iterator s = need_split.begin(); s != need_split.end(); ++s)
	{
		GISNetworkSegmentVector rv = (*s)->data().mSegments;
		DebugAssert(rv.size() == 1);
		DebugAssert((*s)->twin()->data().mSegments.empty());
		double hs = get_he_level_at(*s,(*s)->source());
		double ht = get_he_level_at(*s,(*s)->target());
		Point_2 ps = (*s)->source()->point();
		Point_2 pt = (*s)->target()->point();
		// TODO: better midpoint choice!
		Point_2 pm = CGAL::midpoint(ps,pt);
		X_monotone_curve_2 c1(Segment_2(ps,pm));
		X_monotone_curve_2 c2(Segment_2(pm,pt));
		Pmwx::Halfedge_handle h1 = io_map.split_edge(*s, c1, c2);
		Pmwx::Halfedge_handle h2 = h1->next();
		DebugAssert(h1->source()->point() == ps);
		DebugAssert(h1->target()->point() == pm);
		DebugAssert(h2->source()->point() == pm);
		DebugAssert(h2->target()->point() == pt);
		h1->data().mSegments = rv;
		h2->data().mSegments = rv;
		h1->data().mSegments.back().mTargetHeight = max(hs,ht);
		h2->data().mSegments.back().mSourceHeight = max(hs,ht);
	}



	// FORK CONTROL/CHANGE CONTROL.  First: run fork control over every vertex once.

	set<Pmwx::Vertex_handle>	trial, done, retry;

	for(Pmwx::Vertex_handle v = io_map.vertices_begin(); v != io_map.vertices_end(); ++v)
	{
		check_junction_highways(v,trial);
	}

	// Now, some set of additional vertices may need to be RE-evaluated.  We keep a 'done' queue to avoid
	// infinite loops, but we get at least two looks at every vertex.

	while(!trial.empty())
	{
		Vertex_handle v = *trial.begin();
		trial.erase(trial.begin());
		done.insert(v);
		check_junction_highways(v,retry);
		for(set<Pmwx::Vertex_handle>::iterator i = retry.begin(); i != retry.end(); ++i)
			if(done.count(*i) == 0)
			{
				trial.insert(*i);
//				debug_mesh_point(cgal2ben((*i)->point()),1,1,0);
			}
	}

/*
	for(Pmwx::Vertex_handle v = io_map.vertices_begin(); v != io_map.vertices_end(); ++v)
	{
		map<int, vector<Pmwx::Halfedge_handle> > junc;
		int t = levelize_junction(v,junc);
		if(t >= 2)
		for(map<int, vector<Pmwx::Halfedge_handle> >::iterator l = junc.begin(); l != junc.end(); ++l)
		if(l->second.size() == 2)
		{
			if(l->second[0]->target() == v && l->second[1]->source() == v)
				apply_i_rules(l->second[0],l->second[1]);
			else if(l->second[0]->source() == v && l->second[1]->target() == v)
				apply_i_rules(l->second[1],l->second[0]);
		}
	}
*/

	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
	{
		bool has_road = he_has_any_roads(e) && get_he_limited_access(e);
		e->data().mMark = e->twin()->data().mMark = has_road;
	}

	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
	if(e->data().mMark)
	{
		Pmwx::Halfedge_handle he = get_he_with_roads(e);
		list<Pmwx::Halfedge_handle>	strand;
		double len = collect_contig<matches_limited_access>(he, strand);
		vector<int>	rep_types;
		for(list<Pmwx::Halfedge_handle>::iterator r = strand.begin(); r != strand.end(); ++r)
		{
			(*r)->data().mMark = false;
			(*r)->twin()->data().mMark = false;
//			debug_mesh_line(cgal2ben((*r)->source()->point()),cgal2ben((*r)->target()->point()),0,0,1,1,0,0);

			int rt = get_he_rep_type(*r);
			if(rep_types.empty() || rep_types.back() != rt)	rep_types.push_back(rt);
		}
		if(strand.size() > 1)
		{
			for(int n = 1; n < rep_types.size(); ++n)
			for(ChangeRuleTable::iterator r = gChangeRules.begin(); r != gChangeRules.end(); ++r)
			if(rep_types[n-1] == r->prev && rep_types[n] == r->next)
			{
				rep_types.insert(rep_types.begin()+n,r->new_mid);
				++n;
				break;
			}

			DebugAssert(!rep_types.empty());
			int rm = rep_types.size() / 2;
			int rf = 0;
			int rb = rep_types.size()-1;
			double df = 0.0, db = 0.0;
			list<Pmwx::Halfedge_handle>::iterator	head = strand.begin(), tail = strand.end();

			while(head != tail)
			{
				set_he_rep_type(*head, rep_types[rf]);
				df += approx_he_len(*head);
				if(df > MIN_DIST_FOR_TYPE && rf < rm)
				{
					df = 0.0;
					++rf;
				}
				++head;
				if(head != tail)
				{
					--tail;
					set_he_rep_type(*tail,rep_types[rb]);
					db += approx_he_len(*tail);
					if(db > MIN_DIST_FOR_TYPE && rb > rm)
					{
						db = 0.0;
						--rb;
					}
				}
			}

			if(rb != rm || rf != rm || db == 0.0 || df == 0.0)
			for(list<Pmwx::Halfedge_handle>::iterator r = strand.begin(); r != strand.end(); ++r)
			{
//				debug_mesh_line(cgal2ben((*r)->source()->point()),cgal2ben((*r)->target()->point()),0,0,1,1,0,0);
			}
		}
	}

}
