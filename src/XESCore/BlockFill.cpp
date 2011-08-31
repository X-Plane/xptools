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
 
 /*
 
	Things we can do to make this faster.
	
	The vast majority of time is spent doing the sweep of the initial lines used to create the block.  It is important that we keep this design; a lot of blocks
	will have a large number of edges and a small number of faces (think of a huge wildlife refuge with a crazy polygonal border).  In these cases, incremental 
	"zone"-based inserts have to iterate the whole block per step, and sweeps have to sweep the whole block.  Both are quite bad performance-wise.
	
	So to make it faster, we need to make the sweep faster, usually done via fewer segments.
	
	-	If we make the max allowable error metric on the Douglas Peuker larger contetually by what is on a side, whether we are erring in or out, and what type of
		zoning we are doing, we can have fewer segments, which gives us NlogN time improvement.
	-	If we eliminate adjacency by merging various blocks, we can also cut down on segments.  One way to do this would be to merge an edge's extensions and its 
		up-forthcoming cap.  (We CANNOT merge two adjacent edges into a contiguous zone because if the effective geometry of the two edges is self-intersecting,
		our rebuilding of the polygons will NOT handle this correctly!  We use a cross-side-toggle-polygon-fill scheme, so the self-intersecting inside will become 
		"outside".)
 
 */

#include "BlockFill.h"
#include "BlockAlgs.h"
#include "douglas_peuker.h"
#include "CompGeomDefs2.h"
#include "CompGeomUtils.h"
#include "MapPolygon.h"
#include "GISUtils.h"
#include "NetTables.h"
#include "Zoning.h"
#include "MeshDefs.h"
#include "DEMTables.h"
#include "MathUtils.h"
#include "MapRaster.h"
#include "STLUtils.h"
#include "PolyRasterUtils.h"
//#include "RF_Selection.h"
#include "MapHelpers.h"

#include "GISTool_Globals.h"

#include <CGAL/Arr_overlay_2.h>

#define FOREST_SUBDIVIDE_AREA	(10000)

#define BLOCK_ERR_MTR 1.0 
struct block_pt {
	bool operator==(const block_pt& rhs) const { return loc == rhs.loc; }
	Point2			loc;			// This is our original location on 
	Point2			offset_prev1;	// This is our offset location parallel to our PREVIOUS segment.
	Point2			offset_next1;	// This is our offset location parallel to our NEXT segment.
	Point2			offset_prev2;	// This is our offset location parallel to our PREVIOUS segment.
	Point2			offset_next2;	// This is our offset location parallel to our NEXT segment.
	Point2			offset_reflex1[3];// If we have a reflex vertex, up to 3 points form the "shape" around it.
	Point2			offset_reflex2[3];// If non-reflex but non-discon, this gives us one useful "mid" point.
	bool			locked;			// Is this point locked.  We lock any point that has high internal valence.
	bool			discon;			// Discontinuity in road or edge type - requires a "hard cap" between this segment and the next one.
	bool			reflex;
	int				edge_type;		// Edge type OUTGOING from this point.
	float			dot;
	Halfedge_handle	orig;
};

struct block_pt_locked {
	bool operator()(const block_pt& p) const { return p.locked; }
};
struct block_pt_err {
	double operator()(const block_pt& p1, const block_pt& p2, const block_pt& x) const {
		return Segment2(p1.loc,p2.loc).squared_distance(x.loc); 
	}
};



inline	void	MoveSegLeft(const Segment2& l1, double dist, Segment2& l2)
{
	Vector2 offset_vector;
	if(dist == 0.0)
	{
		l2 = l1;
		return;
	}
	DebugAssert(l1.p1 != l1.p2);
	offset_vector = Vector2(l1.source(), l1.target()).perpendicular_ccw();
	offset_vector.normalize();
	offset_vector *= dist;
	l2 = Segment2(l1.source() + offset_vector, l1.target() + offset_vector);	
	DebugAssert(l1.p1 != l2.p1);
	DebugAssert(l1.p2 != l2.p2);
	DebugAssert(l2.p1 != l2.p2);
}

static int	WidestRoadTypeForSegment(Pmwx::Halfedge_const_handle he)
{
	int	best_type = NO_VALUE;
	double best_width = 0.0;
	for (GISNetworkSegmentVector::const_iterator i = he->data().mSegments.begin(); i != he->data().mSegments.end(); ++i)
	{
		if (gNetReps[i->mRepType].width > best_width)
		{
			best_type = i->mRepType;
			best_width = gNetReps[i->mRepType].width;
		}
	}

	for (GISNetworkSegmentVector::const_iterator i = he->twin()->data().mSegments.begin(); i != he->twin()->data().mSegments.end(); ++i)
	{
		if (gNetReps[i->mRepType].width > best_width)
		{
			best_type = i->mRepType;
			best_width = gNetReps[i->mRepType].width;
		}
	}
	return best_type;
}

static float WidthForSegment(int seg_type)
{
	if(gNetReps.count(seg_type) == 0) return 0.0f;
	return gNetReps[seg_type].width * 0.5f;
}

static bool edges_match(Block_2::Halfedge_handle a, Block_2::Halfedge_handle b)
{
	return a->twin()->face()->data().usage == b->twin()->face()->data().usage &&
		   a->twin()->face()->data().feature== b->twin()->face()->data().feature;
}

static EdgeRule_t * edge_for_road(int road_type, int zoning)
{
	for(EdgeRuleTable::iterator e = gEdgeRules.begin(); e != gEdgeRules.end(); ++e)
	if(e->zoning == zoning && e->road_type == road_type)
		return &*e;
	return NULL;
}

static bool against_road(Block_2::Halfedge_const_handle h)
{
	return h->twin()->face()->data().usage == usage_Road;
}

inline void push_block_curve(vector<Block_2::X_monotone_curve_2>& curves, const Point2& p1, const Point2& p2, int idx1)
{
	DebugAssert(p1 != p2);
	curves.push_back(Block_2::X_monotone_curve_2(Segment_2(ben2cgal(p1),ben2cgal(p2)),idx1));
}

inline void push_block_curve(vector<Block_2::X_monotone_curve_2>& curves, const Point2& p1, const Point2& p2, int idx1, int idx2)
{
//	push_block_curve(curves,p1,p2,idx1);
//	push_block_curve(curves,p1,p2,idx2);
//	return;
	DebugAssert(idx1 != idx2);
	EdgeKey_container	keys;
	keys.insert(idx1);
	keys.insert(idx2);
	DebugAssert(p1 != p2);
	curves.push_back(Block_2::X_monotone_curve_2(Segment_2(ben2cgal(p1),ben2cgal(p2)),keys));
}

static void	init_road_ccb(int zoning, Pmwx::Ccb_halfedge_circulator he, CoordTranslator2& translator, 
							vector<BLOCK_face_data>& pieces, vector<Block_2::X_monotone_curve_2>& curves,
							int& cur_base, int span, int oob_idx, bool fill_edges)
{
	Pmwx::Ccb_halfedge_circulator circ, stop;
	circ = stop = he;
	do {
		if(WidestRoadTypeForSegment(circ->next()) != WidestRoadTypeForSegment(circ))
		{
			he = circ;
			break;
		}
		if(circ->source()->point().x() < he->source()->point().x())
			he = circ;
	} while(++circ != stop);

	vector<block_pt> pts;
	
	circ = stop = he;
	do {
		pts.push_back(block_pt());
		pts.back().loc = translator.Forward(cgal2ben(circ->source()->point()));
		pts.back().edge_type = WidestRoadTypeForSegment(circ);
		pts.back().locked = WidestRoadTypeForSegment(circ->prev()) != pts.back().edge_type;

		// We do NOT do dot products here!!  Calculate after we reduce unneeded points.

		if(!pts.back().locked)
		{
			int n = 0;
			Pmwx::Halfedge_around_vertex_circulator vcirc, vstop;
			vcirc = vstop = circ->source()->incident_halfedges();
			do {
				if(vcirc->face() == circ->face() ||
				   vcirc->twin()->face() == circ->face())
					++n;
			} while(++vcirc != vstop);
			
			pts.back().locked = n > 2;
		}
		
	} while(++circ != stop);

	pts.push_back(pts.front());	// this closes the "ring".

	{
		vector<block_pt> pts2;
		douglas_peuker(pts.begin(),pts.end()-1, back_inserter(pts2), BLOCK_ERR_MTR * BLOCK_ERR_MTR, block_pt_locked(), block_pt_err());
		pts.swap(pts2);
	}

	/********************************************************************************************************************************************
	 * CALCULATE GEOMETRY OF INSET ELEMENTS RELATIVE TO THE ROAD
	 ********************************************************************************************************************************************/

	int i, j, k;

	for(i = 0; i < pts.size(); ++i)
	{
		j = (i+1) % pts.size();
		k = (i+2) % pts.size();
		Vector2 v1(pts[i].loc,pts[j].loc);
		Vector2 v2(pts[j].loc,pts[k].loc);
		v1.normalize();
		v2.normalize();
		pts[j].dot = v1.dot(v2);
		pts[j].reflex = right_turn(pts[i].loc,pts[j].loc,pts[k].loc) || pts[i].loc == pts[k].loc;
		pts[j].discon = pts[j].edge_type != pts[i].edge_type || j == 0;

		if(pts[j].reflex && !pts[j].discon && pts[j].dot > 0.5)
		{
			pts[j].reflex = false;
		}
		if(pts[j].reflex) 
			pts[j].discon = true;

		Segment2	real_side(pts[i].loc, pts[j].loc);
		Segment2	offset;		
		MoveSegLeft(real_side, WidthForSegment(pts[i].edge_type), offset);
		pts[i].offset_next1 = offset.p1;
		pts[j].offset_prev1 = offset.p2;

		EdgeRule_t * er = fill_edges ? edge_for_road(pts[i].edge_type, zoning) : NULL;
		real_side = offset;
		MoveSegLeft(real_side, er ? er->width : 0.0, offset);

		pts[i].offset_next2 = offset.p1;
		pts[j].offset_prev2 = offset.p2;
	}
	
	for(i = 0; i < pts.size(); ++i)
	{
		j = (i+1) % pts.size();
		k = (i+2) % pts.size();
		if(pts[j].reflex)
		{
			// These calculations build the paths for the reflex "cap" for right turns.  We always do this, EVEN
			// if our edge types don't match.  why?  Imagine a local and secondary road at an L corner with an AG 
			// string around it - by putting the reflex corner in we capture (1) the junction and (2) the AG string fill
			// which lets us realize that it's really one big contiguous AG string.
		
			float w1_prev = WidthForSegment(pts[i].edge_type);
			float w1_next = WidthForSegment(pts[j].edge_type);
			EdgeRule_t * er_prev = fill_edges ? edge_for_road(pts[i].edge_type, zoning) : NULL;
			EdgeRule_t * er_next = fill_edges ? edge_for_road(pts[j].edge_type, zoning) : NULL;

			float w2_prev = er_prev ? er_prev->width : 0.0f;
			float w2_next = er_next ? er_next->width : 0.0f;
		
			if(pts[i].loc == pts[k].loc || pts[j].dot < 0)
			{
				Vector2 to_prev = Vector2(pts[i].loc,pts[j].loc);
				Vector2 to_next = Vector2(pts[k].loc,pts[j].loc);
				to_prev.normalize();
				to_next.normalize();
				pts[j].offset_reflex1[0] = pts[j].offset_prev1 + to_prev * w1_prev;
				pts[j].offset_reflex1[2] = pts[j].offset_next1 + to_next * w1_next;
				pts[j].offset_reflex2[0] = pts[j].offset_prev2 + to_prev * w2_prev;
				pts[j].offset_reflex2[2] = pts[j].offset_next2 + to_next * w2_next;
				
				// Make this better??!
				pts[j].offset_reflex1[1] = pts[j].offset_reflex1[0];
				pts[j].offset_reflex2[1] = pts[j].offset_reflex2[0];
				//debug_mesh_point(translator.Reverse(pts[j].offset_reflex1[1]),1,0,0);
			}
			else if(pts[j].dot > 0.99)
			{
				// Very slight corner - don't trust intersection.
				pts[j].offset_reflex1[0] = pts[j].offset_reflex1[1] = pts[j].offset_reflex1[2] = Segment2(pts[j].offset_prev1,pts[j].offset_next1).midpoint();
				pts[j].offset_reflex2[0] = pts[j].offset_reflex2[1] = pts[j].offset_reflex2[2] = Segment2(pts[j].offset_prev2,pts[j].offset_next2).midpoint();
				//debug_mesh_point(translator.Reverse(pts[j].offset_reflex1[1]),0,1,0);
			}
			else
			{
				// Sane intersection case.
				Line2 prev1(pts[i].offset_next1,pts[j].offset_prev1);
				Line2 prev2(pts[i].offset_next2,pts[j].offset_prev2);
				Line2 next1(pts[j].offset_next1,pts[k].offset_prev1);
				Line2 next2(pts[j].offset_next2,pts[k].offset_prev2);
				if(!prev1.intersect(next1,pts[j].offset_reflex1[2]))
					printf("WTF?\n");
				if(!prev2.intersect(next2,pts[j].offset_reflex2[2]))
					printf("WTF?\n");
				
				pts[j].offset_reflex1[0] = pts[j].offset_reflex1[1] = pts[j].offset_reflex1[2];
				pts[j].offset_reflex2[0] = pts[j].offset_reflex2[1] = pts[j].offset_reflex2[2];
				//debug_mesh_point(translator.Reverse(pts[j].offset_reflex1[1]),0,0,1);
			}
		} 
		else if (!pts[j].discon)
		{
			// These calculations try to build the path when we have contiguous left turns.  In this case, look at a few cases:
			if(pts[j].dot > 0.99)
			{
				// do not trust intersection, just run right through
				// Very slight corner - don't trust intersection.
				pts[j].offset_reflex1[0] = pts[j].offset_reflex1[1] = pts[j].offset_reflex1[2] = Segment2(pts[j].offset_prev1,pts[j].offset_next1).midpoint();
				pts[j].offset_reflex2[0] = pts[j].offset_reflex2[1] = pts[j].offset_reflex2[2] = Segment2(pts[j].offset_prev2,pts[j].offset_next2).midpoint();
				
			}
			else if(pts[j].dot >= 0.0)
			{
				Segment2	to_prev(pts[j].loc,pts[i].loc);
				Segment2	to_next(pts[j].loc,pts[k].loc);
				Point2 next_on_prev = to_prev.projection(pts[j].offset_next2);
				Point2 prev_on_next = to_prev.projection(pts[j].offset_prev2);
				
				if(!right_turn(pts[i].loc,pts[j].loc,pts[k].loc) && 
					(
						pts[j].loc.squared_distance(next_on_prev) > pts[j].loc.squared_distance(to_prev.midpoint()) ||
						pts[j].loc.squared_distance(prev_on_next) > pts[j].loc.squared_distance(to_next.midpoint())
					))
				{
					pts[j].discon = true;
					//debug_mesh_point(translator.Reverse(pts[j].offset_prev1),1,0,1);
					//debug_mesh_point(translator.Reverse(pts[j].offset_next1),1,0,1);
				}
				else
				{
					Line2 prev1(pts[i].offset_next1,pts[j].offset_prev1);
					Line2 prev2(pts[i].offset_next2,pts[j].offset_prev2);
					Line2 next1(pts[j].offset_next1,pts[k].offset_prev1);
					Line2 next2(pts[j].offset_next2,pts[k].offset_prev2);
					if(!prev1.intersect(next1,pts[j].offset_reflex1[2]))
						printf("WTF?\n");
					if(!prev2.intersect(next2,pts[j].offset_reflex2[2]))
						printf("WTF?\n");
					
					pts[j].offset_reflex1[0] = pts[j].offset_reflex1[1] = pts[j].offset_reflex1[2];
					pts[j].offset_reflex2[0] = pts[j].offset_reflex2[1] = pts[j].offset_reflex2[2];					
				}
				
			}
			else {
				// TIGHT left turn - don't even try, we can't figure out if we're going to have a crash.
				//debug_mesh_point(translator.Reverse(pts[j].loc),1,0,0);
				pts[j].discon = true;
			}
		}
		
	}
	

	/********************************************************************************************************************************************
	 * ATTACH GEOMETRY
	 ********************************************************************************************************************************************/
	
	for(i = 0; i < pts.size(); ++i)
	{
		j = (i+1) % pts.size();
		k = (i+2) % pts.size();

		EdgeRule_t * er = fill_edges ? edge_for_road(pts[i].edge_type, zoning) : NULL;
		float w = WidthForSegment(pts[i].edge_type);

		DebugAssert(er == NULL || w > 0.0);

		// PART 1 - Build geometry directly opposite the road.
		// This is the segment from I to J.

		if(er && w > 0.0)
		{		
			if(pts[i].discon && pts[j].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].loc,pts[i].offset_next1, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_prev1, cur_base + span, cur_base);

				push_block_curve(curves, pts[i].offset_next1,pts[i].offset_next2, cur_base);
				push_block_curve(curves, pts[j].offset_prev1,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_prev2, cur_base);
			}
			else if(pts[i].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].loc,pts[i].offset_next1, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_reflex1[0], cur_base + span, cur_base);

				push_block_curve(curves, pts[i].offset_next1,pts[i].offset_next2, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_reflex2[0], cur_base);
			}
			else if(pts[j].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_reflex1[0],pts[j].offset_prev1, cur_base + span, cur_base);

				push_block_curve(curves, pts[j].offset_prev1,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_prev2, cur_base);
			}
			else
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].offset_reflex1[0],pts[j].offset_reflex1[0], cur_base + span, cur_base);

				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_reflex2[0], cur_base);
			}
		}
		else if(er)
		{
			if(pts[i].discon && pts[j].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[i].loc,pts[i].offset_next2, cur_base);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_prev2, cur_base);
			}
			else if(pts[i].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[i].loc,pts[i].offset_next2, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_reflex2[0], cur_base);
			}
			else if(pts[j].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_prev2, cur_base);
			}
			else
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_reflex2[0], cur_base);
			}
		}
		else if (w > 0.0)
		{
			if(pts[i].discon && pts[j].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].loc,pts[i].offset_next1, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_prev1, cur_base + span);
			}
			else if(pts[i].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].loc,pts[i].offset_next1, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_reflex1[0], cur_base + span);
			}
			else if(pts[j].discon)
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_reflex1[0],pts[j].offset_prev1, cur_base + span);
			}
			else
			{
				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].offset_reflex1[0],pts[j].offset_reflex1[0], cur_base + span);
			}
		}
		else
		{
			push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx);
		}

		DebugAssert(pieces[cur_base + span].usage == usage_Empty);
		DebugAssert(pieces[cur_base		  ].usage == usage_Empty);
		
		if(pts[j].discon)
		{
			pieces[cur_base] = BLOCK_face_data(usage_Polygonal_Feature,er ? er->resource_id : NO_VALUE);
			pieces[cur_base + span] = BLOCK_face_data(usage_Road, pts[i].edge_type);
			
			++cur_base;
		}
		
		// Part 2 - reflex vertices.  These need a little "extra" around their edges.

		if(pts[j].reflex)
		{
			EdgeRule_t * er2 = fill_edges ? edge_for_road(pts[j].edge_type, zoning) : NULL;
			float w2 = WidthForSegment(pts[j].edge_type);
		
			if(!er || !er2 || er != er2)
				er2 = NULL;
			
			if(w == 0.0 || w2 == 0.0)
				w2 = 0.0;
		
			// NOTE: a reflex vertex can be semi-degenerate - that is, just enough for right_hand to return true in floating point, yet
			// some segments might be zero length.  Run a bunch of checks to eliminate degenerate infrastructure before passing back to CGAL!
			if(er2 && w2 > 0.0)
			{			
				if(pts[j].offset_next1 != pts[j].offset_prev1)
				{
					push_block_curve(curves, pts[j].loc,pts[j].offset_next1, cur_base + span);
					push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				}
				if(pts[j].offset_prev1 != pts[j].offset_reflex1[0])
					push_block_curve(curves, pts[j].offset_prev1, pts[j].offset_reflex1[0], cur_base + span, cur_base);
				if(pts[j].offset_reflex1[0] != pts[j].offset_reflex1[1])
					push_block_curve(curves, pts[j].offset_reflex1[0],pts[j].offset_reflex1[1], cur_base + span, cur_base);
				if(pts[j].offset_reflex1[1] != pts[j].offset_reflex1[2])
					push_block_curve(curves, pts[j].offset_reflex1[1],pts[j].offset_reflex1[2], cur_base + span, cur_base);
				if(curves, pts[j].offset_reflex1[2] != pts[j].offset_next1)
					push_block_curve(curves, pts[j].offset_reflex1[2], pts[j].offset_next1, cur_base + span, cur_base);

				if(pts[j].offset_next1 != pts[j].offset_prev1 || pts[j].offset_next2 != pts[j].offset_prev2)
				{
					push_block_curve(curves, pts[j].offset_next1,pts[j].offset_next2, cur_base);
					push_block_curve(curves, pts[j].offset_prev1,pts[j].offset_prev2, cur_base);
				}
				if(pts[j].offset_prev2 != pts[j].offset_reflex2[0])
					push_block_curve(curves, pts[j].offset_prev2, pts[j].offset_reflex2[0], cur_base);
				if(pts[j].offset_reflex2[0] != pts[j].offset_reflex2[1])
					push_block_curve(curves, pts[j].offset_reflex2[0],pts[j].offset_reflex2[1], cur_base);
				if(pts[j].offset_reflex2[1] != pts[j].offset_reflex2[2])
					push_block_curve(curves, pts[j].offset_reflex2[1],pts[j].offset_reflex2[2], cur_base);				
				if(pts[j].offset_reflex2[2] != pts[j].offset_next2)
					push_block_curve(curves, pts[j].offset_reflex2[2], pts[j].offset_next2, cur_base);

			}
			else if(er2)
			{
				if(pts[j].offset_next2 != pts[j].offset_prev2)
				{
					push_block_curve(curves, pts[j].loc,pts[j].offset_next2, cur_base);
					push_block_curve(curves, pts[j].loc,pts[j].offset_prev2, cur_base);
				}
				if(pts[j].offset_prev2 != pts[j].offset_reflex2[0])
					push_block_curve(curves, pts[j].offset_prev2, pts[j].offset_reflex2[0], cur_base);
				if(pts[j].offset_reflex2[0] != pts[j].offset_reflex2[1])
					push_block_curve(curves, pts[j].offset_reflex2[0],pts[j].offset_reflex2[1], cur_base);
				if(pts[j].offset_reflex2[1] != pts[j].offset_reflex2[2])
					push_block_curve(curves, pts[j].offset_reflex2[1],pts[j].offset_reflex2[2], cur_base);				
				if(pts[j].offset_reflex2[2] != pts[j].offset_next2)
					push_block_curve(curves, pts[j].offset_reflex2[2], pts[j].offset_next2, cur_base);
			}
			else if (w2 > 0.0)
			{
				if(pts[j].offset_next1 != pts[j].offset_prev1)
				{
					push_block_curve(curves, pts[j].loc,pts[j].offset_next1, cur_base + span);
					push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				}
				if(pts[j].offset_prev1 != pts[j].offset_reflex1[0])
					push_block_curve(curves, pts[j].offset_prev1, pts[j].offset_reflex1[0], cur_base + span);
				if(pts[j].offset_reflex1[0] != pts[j].offset_reflex1[1])
					push_block_curve(curves, pts[j].offset_reflex1[0],pts[j].offset_reflex1[1], cur_base + span);
				if(pts[j].offset_reflex1[1] != pts[j].offset_reflex1[2])
					push_block_curve(curves, pts[j].offset_reflex1[1],pts[j].offset_reflex1[2], cur_base + span);
				if(pts[j].offset_reflex1[2] != pts[j].offset_next1)
					push_block_curve(curves, pts[j].offset_reflex1[2], pts[j].offset_next1, cur_base + span);
			}
			else
			{
			}

			DebugAssert(pieces[cur_base + span].usage == usage_Empty);
			DebugAssert(pieces[cur_base		  ].usage == usage_Empty);
			
			pieces[cur_base] = BLOCK_face_data(usage_Polygonal_Feature,er ? er->resource_id : NO_VALUE);
			pieces[cur_base + span] = BLOCK_face_data(usage_Road, pts[i].edge_type);
			
			++cur_base;
		}



	} 	
}

enum face_cat {
	cat_urban = 0,
	cat_forest,
	cat_flat,
	cat_oob,
	cat_DIM
};

static face_cat categorize_vertex(CDT& mesh, CDT::Face_handle f, int n, Bbox2& bounds, float max_slope)
{
	if(mesh.is_infinite(f)) return cat_oob;
	Bbox2	cb;
	cb += cgal2ben(f->vertex(0)->point());
	cb += cgal2ben(f->vertex(1)->point());
	cb += cgal2ben(f->vertex(2)->point());
	if(!bounds.overlap(cb))
		return cat_oob;
	
	if(f->info().terrain == terrain_Water)
		return cat_flat;

	CDT::Vertex_handle v = f->vertex(n);
	
	int lu = f->info().terrain;
//	CDT::Face_circulator circ, stop;
//	circ = stop = v->incident_faces();
//	do 
//	{
//		if(!mesh.is_infinite(circ))
//		if(LowerPriorityNaturalTerrain(lu, circ->info().terrain))
//			lu = circ->info().terrain;
//	} while(++circ != stop);
	
	for(hash_map<int,float>::iterator b = v->info().border_blend.begin(); b != v->info().border_blend.end(); ++b)
	if(b->second > 0.5)
	if(f->info().terrain_border.count(b->first))
	if(LowerPriorityNaturalTerrain(lu, b->first))
		lu = b->first;
	
	if(gNaturalTerrainInfo[lu].autogen_mode == URBAN)	return (f->info().normal[2] < cos(max_slope * DEG_TO_RAD)) ? cat_forest : cat_urban;
	else if (gNaturalTerrainInfo[lu].autogen_mode == FOREST)	return cat_forest;
	else return cat_flat;
}


/*	
	if (f->info().normal[2] < cos(max_slope * DEG_TO_RAD)
			||	
			(need_lu && !gNaturalTerrainInfo[f->info().terrain].is_city))
	{
		if(gNaturalTerrainInfo[f->info().terrain].is_forest)
			return cat_forest;
		else
			return cat_flat;
	}
	else
		return cat_urban;
}
*/

static bool in_range(CDT::Face_handle f, CoordTranslator2& t)
{
	Bbox2	cb, fb;
	cb += cgal2ben(f->vertex(0)->point());
	cb += cgal2ben(f->vertex(1)->point());
	cb += cgal2ben(f->vertex(2)->point());
	
	fb += t.mSrcMin;
	fb += t.mSrcMax;
	
	return cb.overlap(fb);
}

static void push_curve(vector<Block_2::X_monotone_curve_2>& curves, const Point_2& p1, const Point_2& p2, int cat1, int cat2, int urban_idx, int oob_idx)
{
	DebugAssert(cat1 != cat2);
	if(cat1 != urban_idx && cat2 != urban_idx && cat1 != oob_idx && cat2 != oob_idx)
	{
		EdgeKey_container	keys;
		keys.insert(cat1);
		keys.insert(cat2);
		curves.push_back(Block_2::X_monotone_curve_2(Segment_2(p1, p2), keys));
	}
	else if(cat1 != urban_idx && cat1 != oob_idx)
	{
		curves.push_back(Block_2::X_monotone_curve_2(Segment_2(p1, p2), cat1));
	}
	else if(cat2 != urban_idx && cat2 != oob_idx)
	{
		curves.push_back(Block_2::X_monotone_curve_2(Segment_2(p1, p2), cat2));
	}	
}



static void	init_mesh(CDT& mesh, CoordTranslator2& translator, vector<Block_2::X_monotone_curve_2>& curves, int cat_table[cat_DIM],float max_slope, int need_lu)
{
	Point_2 start = Point_2(
						(translator.mSrcMin.x() + translator.mSrcMax.x()) * 0.5,
						(translator.mSrcMin.y() + translator.mSrcMax.y()) * 0.5);

	int n;
	CDT::Locate_type lt;
	CDT::Face_handle root = mesh.locate(start, lt, n);

	DebugAssert(lt != CDT::OUTSIDE_AFFINE_HULL);
	DebugAssert(lt != CDT::OUTSIDE_CONVEX_HULL);
	DebugAssert(in_range(root, translator));
	DebugAssert(!mesh.is_infinite(root));
	
	set<CDT::Face_handle>	visited;
	set<CDT::Face_handle>	to_visit;
	to_visit.insert(root);

	Bbox2	 lim;
	lim += translator.mSrcMin;
	lim += translator.mSrcMax;

	while(!to_visit.empty())
	{
		CDT::Face_handle f = *to_visit.begin();
		to_visit.erase(to_visit.begin());
		visited.insert(f);
			
		int cat[3] = {
			cat_table[categorize_vertex(mesh, f, 0, lim, max_slope)],
			cat_table[categorize_vertex(mesh, f, 1, lim, max_slope)],
			cat_table[categorize_vertex(mesh, f, 2, lim, max_slope)] };
		
		DebugAssert(cat[0] != cat_table[cat_oob]);
		DebugAssert(cat[1] != cat_table[cat_oob]);
		DebugAssert(cat[2] != cat_table[cat_oob]);
		
		if(cat[0] != cat[1] || cat[1] != cat[2])
		{
//			Point_2	p0 = ben2cgal(translator.Forward(cgal2ben(f->vertex(0)->point())));
//			Point_2	p1 = ben2cgal(translator.Forward(cgal2ben(f->vertex(0)->point())));
//			Point_2	p2 = ben2cgal(translator.Forward(cgal2ben(f->vertex(0)->point())));
			Point_2	p01 = ben2cgal(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(0)->point(),f->vertex(1)->point()))));
			Point_2	p12 = ben2cgal(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(1)->point(),f->vertex(2)->point()))));
			Point_2	p20 = ben2cgal(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(2)->point(),f->vertex(0)->point()))));
			Point_2 p012 = ben2cgal(translator.Forward(cgal2ben(CGAL::centroid(mesh.triangle(f)))));
			
//			if(p01 == p12 ||
//			   p01 == p20 ||
//			   p12 == p20 ||
//			   p012 == p01 ||
//			   p012 == p12 ||
//			   p012 == p20)
//			{
//				debug_mesh_point(cgal2ben(f->vertex(0)->point()),1,0,0);
//				debug_mesh_point(cgal2ben(f->vertex(1)->point()),1,0,0);
//				debug_mesh_point(cgal2ben(f->vertex(2)->point()),1,0,0);
//				DebugAssert(!"Degenerate tri");
//			}
			
			if(cat[0] == cat[1])
			{
				if(p12 != p20)
				push_curve(curves,p12, p20, cat[0], cat[2], cat_table[cat_urban], cat_table[cat_oob]);
			}
			else if(cat[1] == cat[2])
			{
				if(p20 != p01)
				push_curve(curves,p20, p01, cat[1], cat[0], cat_table[cat_urban], cat_table[cat_oob]);
			}
			else if(cat[2] == cat[0])
			{
				if(p01 != p12)
				push_curve(curves,p01, p12, cat[2], cat[1], cat_table[cat_urban], cat_table[cat_oob]);
			}
			else
			{
				if(p01 != p012)
				push_curve(curves,p01, p012, cat[0], cat[1], cat_table[cat_urban], cat_table[cat_oob]);
				if(p012 != p20)
				push_curve(curves,p012, p20, cat[0], cat[2], cat_table[cat_urban], cat_table[cat_oob]);
				if(p12 != p012)
				push_curve(curves,p12, p012, cat[1], cat[2], cat_table[cat_urban], cat_table[cat_oob]);
			}
		}
		
		for(int n = 0; n < 3; ++n)
		{
			CDT::Face_handle nf = f->neighbor(n);

			bool face_is_new = visited.count(nf) == 0;// && to_visit.count(nf) == 0;

			int l = cat[CDT::cw (n)];
			int r = cat[CDT::ccw(n)];
			
			int L = cat_table[categorize_vertex(mesh, nf, nf->index(f->vertex(CDT::cw (n))),lim, max_slope)];
			int R = cat_table[categorize_vertex(mesh, nf, nf->index(f->vertex(CDT::ccw(n))),lim, max_slope)];

			if(face_is_new)
			{
				Point_2	pl = ben2cgal(translator.Forward(cgal2ben(f->vertex(CDT::cw (n))->point())));
				Point_2	pr = ben2cgal(translator.Forward(cgal2ben(f->vertex(CDT::ccw(n))->point())));
				Point_2	pm = ben2cgal(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(CDT::cw (n))->point(),f->vertex(CDT::ccw(n))->point()))));

				if(l == r && L == R)
				{
					if(l != L)
						push_curve(curves,pr,pl,l,L,cat_table[cat_urban], cat_table[cat_oob]);
				}
				else
				{
					if(l != L)
						push_curve(curves,pm,pl,l,L,cat_table[cat_urban], cat_table[cat_oob]);
					if(r != R)
						push_curve(curves,pr,pm,r,R,cat_table[cat_urban], cat_table[cat_oob]);
				}
			}
			
			int nf_ok = (L != cat_table[cat_oob] || R != cat_table[cat_oob]);
			
			DebugAssert(!nf_ok || !mesh.is_infinite(nf));
			if(nf_ok && face_is_new)
			{
				to_visit.insert(nf);				
			}
		}	
	}

}


bool	init_block(
					CDT&					mesh,
					Pmwx::Face_handle		face,
					Block_2&				out_block,
					CoordTranslator2&		translator)
{
	DebugAssert(!face->is_unbounded());

	int zoning = face->data().GetZoning();
	ZoningInfoTable::iterator zi = gZoningInfo.find(zoning);
	ZoningInfo_t * info = (zi == gZoningInfo.end() ? NULL : &zi->second);



	Pmwx::Ccb_halfedge_circulator circ, stop;
	circ = stop = face->outer_ccb();
	Bbox2	bounds;
	do {
		bounds += cgal2ben(circ->source()->point());

	} while(++circ != stop);

	CreateTranslatorForBounds(bounds, translator);
	if(translator.mDstMax.x() < 1.0 || translator.mDstMax.y() < 1.0)
		return false;

	int num_he = count_circulator(face->outer_ccb());
	for(Pmwx::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h)
		num_he += count_circulator(*h);
		


	vector<BLOCK_face_data>				parts;
	vector<Block_2::X_monotone_curve_2> curves;

	num_he *= 2;
	parts.resize(2 * num_he + cat_DIM, BLOCK_face_data(usage_Empty, NO_VALUE));

	int cat_base = 2 * num_he;

	parts[cat_base + cat_oob] = BLOCK_face_data(usage_OOB, NO_VALUE);
	parts[num_he] = BLOCK_face_data(usage_Steep, NO_VALUE);
	parts[cat_base + cat_flat] = BLOCK_face_data(usage_OOB  , NO_VALUE);

	num_he += 2;
	
	int cat_table[cat_DIM];
	cat_table[cat_urban] = -1;
	cat_table[cat_oob] = cat_base + cat_oob;
	cat_table[cat_forest] = num_he;
	cat_table[cat_flat] = cat_base + cat_flat;
	
	if(info && (info->need_lu || info->max_slope))
		init_mesh(mesh, translator, curves, cat_table,info->max_slope,info->need_lu);

	int base_offset = 0;
	init_road_ccb(zoning, face->outer_ccb(), translator, parts, curves, base_offset, num_he+1, cat_base + cat_oob, info && info->fill_edge);
	for(Pmwx::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h)
		init_road_ccb(zoning, *h, translator, parts, curves, base_offset, num_he+1, cat_base + cat_oob, info && info->fill_edge);

//	printf("Num HE: %d.  Added: %d\n", num_he, base_offset);

#if 0
	for(vector<Block_2::X_monotone_curve_2>::iterator c = curves.begin(); c != curves.end(); ++c)
	{
		for(EdgeKey_iterator i = c->data().begin(); i != c->data().end(); ++i)
		if(*i == cat_base + cat_oob)
		{
			debug_mesh_line(
					translator.Reverse(cgal2ben(c->source())),
					translator.Reverse(cgal2ben(c->target())),
						0.5,0,0,
						1,0,0);
		}
		else if(*i == num_he)
		{
			debug_mesh_line(
					translator.Reverse(cgal2ben(c->source())),
					translator.Reverse(cgal2ben(c->target())),
						0,0.5,0,
						0,1,0);
		}
		else if(*i == cat_base + cat_flat)
		{
			debug_mesh_line(
					translator.Reverse(cgal2ben(c->source())),
					translator.Reverse(cgal2ben(c->target())),
						0,0,0.5,
						0,0,1);
		}
		else
		{
			debug_mesh_line(
					translator.Reverse(cgal2ben(c->source())),
					translator.Reverse(cgal2ben(c->target())),
						0.4,0.4,0.4,
						0.8,0.8,0.8);
		}
	}
#endif
	create_block(out_block,parts, curves, cat_base + cat_oob);	// First "parts" block is outside of CCB, marked as "out of bounds", so trapped areas are not marked empty.
	clean_block(out_block);
	return true;
}					

static double get_ccb_area(Block_2::Ccb_halfedge_circulator first)
{
	double t = 0.0;

	Block_2::Ccb_halfedge_circulator second(first);
	++second;
	
	DebugAssert(first != second);	// Topologically we cannot have a single halfedge!
	
	Block_2::Ccb_halfedge_circulator third(second);

	Point2 p1(cgal2ben(first->target()->point()));
	Point2 p2(cgal2ben(second->target()->point()));

	while(++third != first)
	{
		Point2 p3(cgal2ben(third->target()->point()));
		
		t += Vector2(p1,p2).signed_area(Vector2(p1,p3));
		
		p2 = p3;
	}
	return t;
}


class overlay_forests : public CGAL::_Arr_default_overlay_traits_base<Block_2, Pmwx, Block_2> {
public:
  virtual void create_face (
					Face_handle_A f1,
					Face_handle_B f2,
					Face_handle_R f) const
	{
		if(f1->data().usage == usage_Empty && f2->data().mTerrainType != NO_VALUE)
		{
			f->data().usage = usage_Polygonal_Feature;
			f->data().feature = f2->data().mTerrainType;
//			f->data().pre_placed = false;
		}
		else
			f->set_data(f1->data());
	}

};

bool	apply_fill_rules(
					int						zoning,
					Block_2&				block,
					CoordTranslator2&		translator)
{
	bool did_promote = false;
	if(gZoningInfo[zoning].fill_area)
	{
		for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
		if(!f->is_unbounded())
		if(f->data().usage == usage_Empty)
		{
			double area = get_ccb_area(f->outer_ccb());
			int has_hole = 0;
			for(Block_2::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
			{
				area -= get_ccb_area(*h);
				has_hole = 1;
			}
			
			int sides = 0;
			int has_roads = 0;
			Block_2::Ccb_halfedge_circulator circ(f->outer_ccb()), stop, next;
			stop = circ;
			do {
				++sides;
				if(circ->twin()->face()->data().usage == usage_Road)
					has_roads = true;
			} while (++circ != stop);
			
			for(FillRuleTable::iterator r = gFillRules.begin(); r != gFillRules.end(); ++r)
			if(has_roads)
			if(r->zoning == NO_VALUE || zoning == r->zoning)
			if(r->hole_ok || !has_hole)		
			if(0 == r->size_max || (r->size_min <= area && area <= r->size_max))
			if(0 == r->side_max || (r->side_min <= sides && sides <= r->side_max))
			{
				f->data().usage = usage_Polygonal_Feature;
				f->data().feature = r->resource_id;
//				f->data().pre_placed = false;
				break;
			}
		}
	}

	for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
	if(!f->is_unbounded())
	if(f->data().usage == usage_Empty)
	{
		Block_2::Ccb_halfedge_circulator circ(f->outer_ccb()), stop;
		stop = circ;
		if(stop->twin()->face()->data().usage == usage_Polygonal_Feature)
		{
			bool dif = false;
			do {
				if(circ->twin()->face()->data().usage != stop->twin()->face()->data().usage ||
				 circ->twin()->face()->data().feature != stop->twin()->face()->data().feature)
				{
					dif = true;
					break;
				}
			}
			while(++circ != stop);
			if(!dif)
			{
				f->data().feature = stop->twin()->face()->data().feature;
				f->data().usage = stop->twin()->face()->data().usage;				
				did_promote = true;
			}
		}
	}
	
	for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
	if(!f->is_unbounded())
	if(f->data().usage == usage_Steep)
		f->data().usage = usage_Empty;

	clean_block(block);
//	simplify_block(block, 0.75);
//	clean_block(block);
	
	if(gZoningInfo[zoning].fill_veg)
	{
		for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
		if(!f->is_unbounded())
		if(f->data().usage == usage_Empty)
		{

			f->data().usage = usage_Forest;
			f->data().feature = NO_VALUE;
		}	
	}
	return did_promote;
}


void	PolygonFromBlockCCB(Block_2::Ccb_halfedge_const_circulator circ, Polygon2& out_poly, CoordTranslator2 * translator)
{
	out_poly.clear();
	Block_2::Ccb_halfedge_const_circulator stop = circ;

	do {
		if(translator)
			out_poly.push_back(translator->Reverse(cgal2ben(circ->source()->point())));
		else
			out_poly.push_back(cgal2ben(circ->source()->point()));
	} while (stop != ++circ);
}

void extract_ccb(Block_2::Ccb_halfedge_const_circulator circ, Polygon2& out_poly, Polygon2& out_poly_mtr, CoordTranslator2& translator)
{
	out_poly.clear();
	out_poly_mtr.clear();
	Block_2::Ccb_halfedge_const_circulator stop = circ;

	do {
		out_poly_mtr.push_back(cgal2ben(circ->source()->point()));
		out_poly.push_back(translator.Reverse(out_poly_mtr.back()));
	} while (stop != ++circ);
}

void	PolygonFromBlock(Block_2::Face_const_handle in_face, vector<Polygon2>& out_ps, CoordTranslator2 * translator)
{
	DebugAssert(!in_face->is_unbounded());

	out_ps.push_back(Polygon2());
	PolygonFromBlockCCB(in_face->outer_ccb(), out_ps.back(), translator);
	DebugAssert(out_ps.back().size() > 2);
	
	for(Block_2::Hole_const_iterator h = in_face->holes_begin(); h != in_face->holes_end(); ++h)
	{
		out_ps.push_back(Polygon2());
		PolygonFromBlockCCB(*h, out_ps.back(), translator);
		DebugAssert(out_ps.back().size() > 2);
	}
	if (in_face->number_of_holes() >= 255)
	{
		printf("Face had %zd holes.\n",in_face->number_of_holes());
		throw "too many holes.";
	}
}

void	StringFromCCB(Block_2::Ccb_halfedge_const_circulator ccb, vector<Polygon2>& ps_use, vector<Polygon2>& ps_bad, CoordTranslator2& translator)
{
	Block_2::Ccb_halfedge_const_circulator circ, stop, prev;
	vector<Polygon2> * targ;
	circ = stop = ccb;
	bool has_gap = false;
	do {
		prev = circ;
		--prev;
		if(against_road(circ) != against_road(prev))
		{
			has_gap = true;
			break;
		}
	} while(++circ != stop);
	
	if(has_gap)
	{
		stop = circ;
		// This is the case where there is at least one discontinuity in the contour.  We are at a "start", that is, a seg whose prev is NOT like us.
		// For each of these "starts" we push the first and last, then we just push the last.
		do {
			prev = circ;
			--prev;
			
			bool rc = against_road(circ);
			bool rp = against_road(prev);
			bool want_first = rc != rp;

			targ = rc ? &ps_use : &ps_bad;
			
			if(want_first)
			{
				targ->push_back(Polygon2());
				targ->back().push_back(translator.Reverse(cgal2ben(circ->source()->point())));
			}
			targ->back().push_back(translator.Reverse(cgal2ben(circ->target()->point())));

		} while(++circ != stop);
	}
	else
	{	
		// This is the case where the whole ring is the same...figure out which one it is, push the whole ring, and dupe the end to form a 'string'.
		targ = against_road(circ) ? &ps_use : &ps_bad;
		targ->push_back(Polygon2());
		do {
			targ->back().push_back(translator.Reverse(cgal2ben(circ->source()->point())));			
		} while(++circ != stop);
		targ->back().push_back(targ->back().front());
	}
	

}


int	StringFromBlock(Block_2::Face_const_handle in_face, vector<Polygon2>& out_ps, CoordTranslator2& translator)
{
	DebugAssert(!in_face->is_unbounded());

	vector<Polygon2>	ps_use, ps_bad;

	StringFromCCB(in_face->outer_ccb(), ps_use,ps_bad, translator);
	for(Block_2::Hole_const_iterator h = in_face->holes_begin(); h != in_face->holes_end(); ++h)
		StringFromCCB(*h, ps_use,ps_bad, translator);
	out_ps.clear();
	out_ps.reserve(ps_use.size() + ps_bad.size());

	if((ps_use.size() + ps_bad.size()) > 255)
	{
		printf("ag string had %zd useful and %zd boundaries.\n", ps_use.size(), ps_bad.size());
		throw "too many contours";
	}
	out_ps.insert(out_ps.end(), ps_use.begin(),ps_use.end());
	out_ps.insert(out_ps.end(), ps_bad.begin(),ps_bad.end());
	DebugAssert(ps_use.size() <= 65535);	// 
	return ps_use.size();
}

void push_one_forest(vector<Polygon2>& bounds, const DEMGeo& dem, Pmwx::Face_handle dest_face)
{
	GISPolyObjPlacement_t o;
	o.mShape = bounds;
	o.mDerived = true;
	o.mParam = 255;
	// rep type?
	int lu_any = NO_VALUE;

	PolyRasterizer<double>	raster;
	for(vector<Polygon2>::const_iterator p = bounds.begin(); p != bounds.end(); ++p)
	for(int s = 0; s < p->size(); ++s)
	{
		Segment2 seg(p->side(s));
		raster.AddEdge(
			dem.lon_to_x(seg.p1.x()),
			dem.lat_to_y(seg.p1.y()),
			dem.lon_to_x(seg.p2.x()),
			dem.lat_to_y(seg.p2.y()));
		if(lu_any == NO_VALUE)
			lu_any = dem.xy_nearest(seg.p1.x(),seg.p1.y());
	}
	map<int,int> histo;
	
	raster.SortMasters();
	int y = 0, x1, x2, x;
	int total = 0;
	raster.StartScanline(y);
	while(y < dem.mHeight)
	{
		while(raster.GetRange(x1,x2))
		{
			for(x = x1; x < x2; ++x)
			{
				float sample = dem.get(x,y);
				if(sample != NO_VALUE)
					histo[sample]++;
				++total;
			}
		}
		++y;
		raster.AdvanceScanline(y);		
	}
	if(!histo.empty())
	{
		map<int,int>::iterator h = histo.end();
		--h;
		o.mRepType = h->first;
		dest_face->data().mPolyObjs.push_back(o);				
	}
	else if(lu_any != NO_VALUE)
	{
		o.mRepType = lu_any;
		dest_face->data().mPolyObjs.push_back(o);						
	}
	else
		printf("Lost forest: %d total points included.\n", total);
}	


void	extract_features(
					Block_2&				block,
					Pmwx::Face_handle		dest_face,
					CoordTranslator2&		translator,
					const DEMGeo&			forest_dem,
					ForestIndex&			forest_index)					
{
	try {
		for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
		if(!f->is_unbounded())
		{
			if(f->data().usage == usage_Polygonal_Feature)
		//	if(!f->data().pre_placed)
			{
				GISPolyObjPlacement_t o;
				
				o.mRepType = f->data().feature;
				o.mDerived = true;
				if(strstr(FetchTokenString(o.mRepType),".fac"))
					o.mParam = 10;
				else
					o.mParam = 255;

				if(strstr(FetchTokenString(o.mRepType),".ags"))
				{
					o.mParam = StringFromBlock(f,o.mShape,translator);
					DebugAssert(o.mShape.size() <= 255);
					dest_face->data().mPolyObjs.push_back(o);				
				}
				else
				{
					PolygonFromBlock(f,o.mShape, &translator);
					DebugAssert(o.mShape.size() <= 255);
					if(strstr(FetchTokenString(o.mRepType),".agb"))
					{
						int ls = o.mShape[0].longest_side();
						while(ls-- > 0)
						{
							o.mShape[0].push_back(o.mShape[0].front());
							o.mShape[0].erase(o.mShape[0].begin());
						}
					}
					dest_face->data().mPolyObjs.push_back(o);
				}
			}
			if(f->data().usage == usage_Forest)
			{
				vector<Polygon2>	forest(1);
				forest.reserve(1+f->number_of_holes());
				Polygon2	outer_m;
				extract_ccb(f->outer_ccb(),forest.back(),outer_m, translator);
				double area = outer_m.area();
				
				for(Block_2::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
				{
					Polygon2 hole_m;
					forest.push_back(Polygon2());
					extract_ccb(*h, forest.back(), hole_m, translator); 
					area -= hole_m.area();
				}
				
				if(area < FOREST_SUBDIVIDE_AREA)
				{
					push_one_forest(forest, forest_dem, dest_face);					
				} 
				else
				{
					Bbox2	total_forest_bounds;
					for(Polygon2::iterator p = forest.front().begin(); p != forest.front().end(); ++p)
						total_forest_bounds += *p;
					set<Face_handle>	forest_faces;
					forest_index.query_value(total_forest_bounds,set_inserter(forest_faces));
					
					vector<BLOCK_face_data>		block_types;
					map<int,int>				forest_type_idx;
					
					block_types.push_back(BLOCK_face_data());
					forest_type_idx[NO_VALUE] = 0;
					for(set<Face_handle>::iterator f = forest_faces.begin(); f != forest_faces.end(); ++f)
					{
						int ft = (*f)->data().mTerrainType;
						if(forest_type_idx.count(ft) == 0)
						{
							forest_type_idx[ft] = block_types.size();
							block_types.push_back(BLOCK_face_data(usage_Forest, ft));
						}
					}
					int oob_idx = block_types.size();
					block_types.push_back(BLOCK_face_data(usage_OOB, 0));
					vector<Block_2::X_monotone_curve_2>	curves;
					
					for(set<Face_handle>::iterator f = forest_faces.begin(); f != forest_faces.end(); ++f)
					{
						DebugAssert(forest_type_idx.count((*f)->data().mTerrainType));
						int ft = forest_type_idx[(*f)->data().mTerrainType];
						DebugAssert(!(*f)->is_unbounded());
						Pmwx::Ccb_halfedge_circulator circ, stop;
						{
							circ= stop = (*f)->outer_ccb();
							do {
								DebugAssert(circ->face() != circ->twin()->face());
								if(forest_faces.count(circ->twin()->face()) == 0 || he_is_same_direction(circ))
								{
									int ot = (forest_faces.count(circ->twin()->face()) == 0) ? -1 :
										forest_type_idx[circ->twin()->face()->data().mTerrainType];
									push_curve(curves,circ->source()->point(),circ->target()->point(),
										ft, ot, -1, -1);
								}
							}while(++circ != stop);
						}
						for(Pmwx::Hole_iterator h = (*f)->holes_begin(); h != (*f)->holes_end(); ++h)
						{
							circ= stop = *h;
							do {
								DebugAssert(circ->face() != circ->twin()->face());							
							if(forest_faces.count(circ->twin()->face()) == 0 || he_is_same_direction(circ))
							{
									int ot = (forest_faces.count(circ->twin()->face()) == 0) ? -1 :
										forest_type_idx[circ->twin()->face()->data().mTerrainType];
							
									push_curve(curves,circ->source()->point(),circ->target()->point(),
										ft, ot, -1, -1);
								}
							}while(++circ != stop);
						}
					}
					
					
					for(vector<Polygon2>::iterator w = forest.begin(); w != forest.end(); ++w)
					for(int s = 0; s < w->size(); ++s)
					{
						Segment2	seg(w->side(s));
						if(seg.p1 != seg.p2)
							push_curve(curves, ben2cgal(seg.p1),ben2cgal(seg.p2),oob_idx, -1, NO_VALUE, -1);
					}

					Block_2		divided_forest;

/*					
					for(vector<Block_2::X_monotone_curve_2>::iterator c = curves.begin(); c != curves.end(); ++c)					
					{
						if(c->data().size() > 1)
							debug_mesh_line(cgal2ben(c->source()),cgal2ben(c->target()),1,1,0,1,1,0);
						else
						for(EdgeKey_iterator i = c->data().begin(); i != c->data().end(); ++i)
						{
							if(*i == 0)
								debug_mesh_line(cgal2ben(c->source()),cgal2ben(c->target()),0.5,0.5,0.5, 0.5,0.5,0.5);
							else if(*i == oob_idx)
								debug_mesh_line(cgal2ben(c->source()),cgal2ben(c->target()),1,0,0, 1,0,0);
							else
								debug_mesh_line(cgal2ben(c->source()),cgal2ben(c->target()),0,0.5,0, 0,0.5,0);
							
						}
					}
*/
					
					create_block(divided_forest, block_types, curves, oob_idx);
					clean_block(divided_forest);
//					for(Block_2::Edge_iterator de= divided_forest.edges_begin(); de != divided_forest.edges_end(); ++de)
//						debug_mesh_line(cgal2ben(de->source()->point()),cgal2ben(de->target()->point()),1,1,1,1,1,1);
					for(Block_2::Face_iterator df = divided_forest.faces_begin(); df != divided_forest.faces_end(); ++df)
					if(!df->is_unbounded())
					if(df->data().usage == usage_Forest)
					{
						vector<Polygon2>	a_forest;
						PolygonFromBlock(df,a_forest, NULL);
						push_one_forest(a_forest, forest_dem, dest_face);					
					}
				}
			}
		}
	} catch (...) {
//		gFaceSelection.insert(dest_face);
	}
}

bool process_block(Pmwx::Face_handle f, CDT& mesh, const DEMGeo& forest_dem,ForestIndex&	forest_index)
{
	bool ret = false;
	int z = f->data().GetZoning();
	if(z == NO_VALUE || z == terrain_Natural)
		return false;
	DebugAssert(gZoningInfo.count(z) != 0);
	if (gZoningInfo.count(z) == 0)
		return false;
		
	CoordTranslator2	trans;
	Block_2 block;
	
	if(init_block(mesh, f, block, trans))
	{
//		simplify_block(block, 0.75);
		clean_block(block);

		if (apply_fill_rules(z, block, trans))
			ret = true;
		extract_features(block, f, trans, forest_dem, forest_index);
	}
	return ret;
}