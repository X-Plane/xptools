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

#include <CGAL/Arr_overlay_2.h>

#define BLOCK_ERR_MTR 1.0 
struct block_pt {
	bool operator==(const block_pt& rhs) const { return loc == rhs.loc; }
	Point2			loc;			// This is our original location on 
	Point2			offset_prev1;	// This is our offset location parallel to our PREVIOUS segment.
	Point2			offset_next1;	// This is our offset location parallel to our NEXT segment.
	Point2			offset_prev2;	// This is our offset location parallel to our PREVIOUS segment.
	Point2			offset_next2;	// This is our offset location parallel to our NEXT segment.
	Point2			offset_reflex1[3];// If we have a reflex vertex, up to 3 points form the "shape" around it.
	Point2			offset_reflex2[3];// If we have a reflex vertex, up to 3 points form the "shape" around it.
	bool			locked;			// Is this point locked.  We lock any point that has high internal valence.
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



template <typename __InputIterator, typename __OutputIterator, typename __LockFunctor, typename __MeasureFunctor>
void douglas_peuker(
			__InputIterator			start,
			__InputIterator			stop,
			__OutputIterator		out,
			double					epsi_2,
			const __LockFunctor&	lock_check,
			const __MeasureFunctor& measure)
{
	if(start == stop)
		return;
	if(*start == *stop)
	{
		int n = stop - start;
		if (n == 1) 
			return;
		
		__InputIterator p(start);
		++p;
		while(p < stop)
		{
			if(lock_check(*p))
			{
				douglas_peuker(start,p,out,epsi_2,lock_check,measure);
				douglas_peuker(p,stop,out,epsi_2,lock_check,measure);
				return;
			}
			++p;
		}
			
		__InputIterator mid(start);
		advance(mid,n/2);
		douglas_peuker(start,mid,out,epsi_2,lock_check,measure);
		douglas_peuker(mid,stop,out,epsi_2,lock_check,measure);
		return;		
	}
	
	
	__InputIterator p(start);
	++p;
	double max_d=0.0;
	__InputIterator worst(stop);
	while(p < stop)
	{
		if(lock_check(*p))
		{
			douglas_peuker(start,p,out,epsi_2,lock_check,measure);
			douglas_peuker(p,stop,out,epsi_2,lock_check,measure);
			return;
		}
		double d = measure(*start, *stop, *p);
		if(d > max_d)
		{
			max_d = d;
			worst = p;
		}
		++p;
	}
	if(max_d >= epsi_2)
	{
		douglas_peuker(start,worst,out,epsi_2,lock_check,measure);
		douglas_peuker(worst,stop,out,epsi_2,lock_check,measure);
	}
	else
	{
		*out++ = *start;
	}
}



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
		if(right_turn(pts[i].loc,pts[j].loc,pts[k].loc) || pts[i].loc == pts[k].loc)
		{
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
			}
			else if(pts[j].dot > 0.99)
			{
				// Very slight corner - don't trust intersection.
				pts[j].offset_reflex1[0] = pts[j].offset_reflex1[1] = pts[j].offset_reflex1[2] = Segment2(pts[j].offset_prev1,pts[j].offset_next1).midpoint();
				pts[j].offset_reflex2[0] = pts[j].offset_reflex2[1] = pts[j].offset_reflex2[2] = Segment2(pts[j].offset_prev2,pts[j].offset_next2).midpoint();
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

		if(er && w > 0.0)
		{
			push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);

			push_block_curve(curves, pts[i].loc,pts[i].offset_next1, cur_base + span);
			push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
			push_block_curve(curves, pts[i].offset_next1,pts[j].offset_prev1, cur_base + span, cur_base);

			push_block_curve(curves, pts[i].offset_next1,pts[i].offset_next2, cur_base);
			push_block_curve(curves, pts[j].offset_prev1,pts[j].offset_prev2, cur_base);
			push_block_curve(curves, pts[i].offset_next2,pts[j].offset_prev2, cur_base);

		}
		else if(er)
		{
			push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
			push_block_curve(curves, pts[i].loc,pts[i].offset_next2, cur_base);
			push_block_curve(curves, pts[j].loc,pts[j].offset_prev2, cur_base);
			push_block_curve(curves, pts[i].offset_next2,pts[j].offset_prev2, cur_base);
		}
		else if (w > 0.0)
		{
			push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
			push_block_curve(curves, pts[i].loc,pts[i].offset_next1, cur_base + span);
			push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
			push_block_curve(curves, pts[i].offset_next1,pts[j].offset_prev1, cur_base + span);
		}
		else
		{
			push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx);
		}

		DebugAssert(pieces[cur_base + span].usage == usage_Empty);
		DebugAssert(pieces[cur_base		  ].usage == usage_Empty);
		
		pieces[cur_base] = BLOCK_face_data(usage_Polygonal_Feature,er ? er->resource_id : NO_VALUE);
		pieces[cur_base + span] = BLOCK_face_data(usage_Road, pts[i].edge_type);
		
		++cur_base;

		// Part 2 - reflex vertices.  These need a little "extra" around their edges.

		if(right_turn(pts[i].loc,pts[j].loc,pts[k].loc) || pts[i].loc == pts[k].loc)
		{
			EdgeRule_t * er2 = fill_edges ? edge_for_road(pts[j].edge_type, zoning) : NULL;
			float w2 = WidthForSegment(pts[j].edge_type);
		
			if(!er || !er2 || er != er2)
				er2 = NULL;
			
			if(w == 0.0 || w2 == 0.0)
				w2 = 0.0;
		
			if(er2 && w2 > 0.0)
			{			
				push_block_curve(curves, pts[j].loc,pts[j].offset_next1, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
//				push_block_curve(curves, pts[j].offset_next1,pts[j].offset_prev1, cur_base + span, cur_base);
				push_block_curve(curves, pts[j].offset_prev1, pts[j].offset_reflex1[0], cur_base + span, cur_base);
				if(pts[j].offset_reflex1[0] != pts[j].offset_reflex1[1])
					push_block_curve(curves, pts[j].offset_reflex1[0],pts[j].offset_reflex1[1], cur_base + span, cur_base);
				if(pts[j].offset_reflex1[1] != pts[j].offset_reflex1[2])
					push_block_curve(curves, pts[j].offset_reflex1[1],pts[j].offset_reflex1[2], cur_base + span, cur_base);
				push_block_curve(curves, pts[j].offset_reflex1[2], pts[j].offset_next1, cur_base + span, cur_base);

				push_block_curve(curves, pts[j].offset_next1,pts[j].offset_next2, cur_base);
				push_block_curve(curves, pts[j].offset_prev1,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[j].offset_prev2, pts[j].offset_reflex2[0], cur_base);
				if(pts[j].offset_reflex2[0] != pts[j].offset_reflex2[1])
					push_block_curve(curves, pts[j].offset_reflex2[0],pts[j].offset_reflex2[1], cur_base);
				if(pts[j].offset_reflex2[1] != pts[j].offset_reflex2[2])
					push_block_curve(curves, pts[j].offset_reflex2[1],pts[j].offset_reflex2[2], cur_base);				
				push_block_curve(curves, pts[j].offset_reflex2[2], pts[j].offset_next2, cur_base);

			}
			else if(er2)
			{
				push_block_curve(curves, pts[j].loc,pts[j].offset_next2, cur_base);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev2, cur_base);
//				push_block_curve(curves, pts[j].offset_next2,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[j].offset_prev2, pts[j].offset_reflex2[0], cur_base);
				if(pts[j].offset_reflex2[0] != pts[j].offset_reflex2[1])
					push_block_curve(curves, pts[j].offset_reflex2[0],pts[j].offset_reflex2[1], cur_base);
				if(pts[j].offset_reflex2[1] != pts[j].offset_reflex2[2])
					push_block_curve(curves, pts[j].offset_reflex2[1],pts[j].offset_reflex2[2], cur_base);				
				push_block_curve(curves, pts[j].offset_reflex2[2], pts[j].offset_next2, cur_base);
			}
			else if (w2 > 0.0)
			{
				push_block_curve(curves, pts[j].loc,pts[j].offset_next1, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
//				push_block_curve(curves, pts[j].offset_next1,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[j].offset_prev1, pts[j].offset_reflex1[0], cur_base + span);
				if(pts[j].offset_reflex1[0] != pts[j].offset_reflex1[1])
					push_block_curve(curves, pts[j].offset_reflex1[0],pts[j].offset_reflex1[1], cur_base + span);
				if(pts[j].offset_reflex1[1] != pts[j].offset_reflex1[2])
					push_block_curve(curves, pts[j].offset_reflex1[1],pts[j].offset_reflex1[2], cur_base + span);
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

static bool want_face(CDT::Face_handle f,float max_slope,int need_lu)
{
	return (f->info().normal[2] < cos(max_slope * DEG_TO_RAD))
			||	
			(need_lu && !gNaturalTerrainInfo[f->info().terrain].is_city);
}

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

static void	init_mesh(CDT& mesh, CoordTranslator2& translator, vector<Block_2::X_monotone_curve_2>& curves, int oob_idx,float max_slope, int need_lu)
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

	while(!to_visit.empty())
	{
		CDT::Face_handle f = *to_visit.begin();
		to_visit.erase(to_visit.begin());
		visited.insert(f);

		int want_this_face = want_face(f,max_slope,need_lu);

		for(int n = 0; n < 3; ++n)
		{
			CDT::Face_handle nf = f->neighbor(n);

			int nf_ok = !mesh.is_infinite(nf) && in_range(nf,translator);
			int want_nf = nf_ok && want_face(nf,max_slope,need_lu);
			
			if(want_this_face && !want_nf)
			{
				curves.push_back(Block_2::X_monotone_curve_2(Segment_2(
					ben2cgal(translator.Forward(cgal2ben(f->vertex(CDT::ccw(n))->point()))),
					ben2cgal(translator.Forward(cgal2ben(f->vertex(CDT::cw (n))->point())))), oob_idx));
			}
				
			if(nf_ok)
			if(visited.count(nf) == 0 && to_visit.count(nf) == 0)
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
	parts.resize(2 * num_he + 2, BLOCK_face_data(usage_Empty, NO_VALUE));

	int idx_oob = num_he + 1;
	int idx_mesh = num_he;

	parts[idx_mesh] = BLOCK_face_data(usage_Steep, NO_VALUE);
	parts[idx_oob ] = BLOCK_face_data(usage_OOB  , NO_VALUE);
	
	num_he += 2;
	
	if(info && (info->need_lu || info->max_slope))
		init_mesh(mesh, translator, curves, idx_mesh,info->max_slope,info->need_lu);

	int base_offset = 0;
	init_road_ccb(zoning, face->outer_ccb(), translator, parts, curves, base_offset, num_he, idx_oob, info && info->fill_edge);
	for(Pmwx::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h)
		init_road_ccb(zoning, *h, translator, parts, curves, base_offset, num_he, idx_oob, info && info->fill_edge);

//	printf("Num HE: %d.  Added: %d\n", num_he, base_offset);

	create_block(out_block,parts, curves, idx_oob);	// First "parts" block is outside of CCB, marked as "out of bounds", so trapped areas are not marked empty.
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
					CoordTranslator2&		translator,
					DEMGeo&					forest_dem)
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
	
	if(gZoningInfo[zoning].fill_veg)
	{
//		int x1 = intmax2(floor(forest_dem.lon_to_x(translator.mSrcMin.x())),0);
//		int y1 = intmax2(floor(forest_dem.lat_to_y(translator.mSrcMin.y())),0);
//		int x2 = intmin2(ceil (forest_dem.lon_to_x(translator.mSrcMax.x())),forest_dem.mWidth );
//		int y2 = intmin2(ceil (forest_dem.lat_to_y(translator.mSrcMax.y())),forest_dem.mHeight);
//
//		Pmwx forest_areas;
//		
//		MapFromDEM(forest_dem,x1,y1,x2,y2,NO_VALUE, forest_areas, &translator);
//
//		Block_2	orig(block);
//		overlay_forests traits;
//		CGAL::overlay(orig, forest_areas, block, traits);
//
//		clean_block(block);

		for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
		if(!f->is_unbounded())
		if(f->data().usage == usage_Empty)
		{
			Block_2::Ccb_halfedge_circulator circ(f->outer_ccb()), stop;
			stop = circ;
			do {
				Point2 sample = translator.Reverse(cgal2ben(circ->source()->point()));				
				float fe = forest_dem.xy_nearest(sample.x(),sample.y());
				if(fe != NO_VALUE)
				{
					f->data().usage = usage_Polygonal_Feature;
					f->data().feature = fe;
//					f->data().pre_placed = false;
					break;
				}
			} while (++circ !=  stop);		
		}	
	}
	return did_promote;
}


void	PolygonFromBlockCCB(Block_2::Ccb_halfedge_const_circulator circ, Polygon2& out_poly, CoordTranslator2& translator)
{
	out_poly.clear();
	Block_2::Ccb_halfedge_const_circulator stop = circ;

	do {
		out_poly.push_back(translator.Reverse(cgal2ben(circ->source()->point())));
	} while (stop != ++circ);
}

void	PolygonFromBlock(Block_2::Face_const_handle in_face, vector<Polygon2>& out_ps, CoordTranslator2& translator)
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
	out_ps.insert(out_ps.end(), ps_use.begin(),ps_use.end());
	out_ps.insert(out_ps.end(), ps_bad.begin(),ps_bad.end());
	DebugAssert(ps_use.size() <= 65535);	// 
	return ps_use.size();
}


void	extract_features(
					Block_2&				block,
					Pmwx::Face_handle		dest_face,
					CoordTranslator2&		translator)
{
	for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
	if(!f->is_unbounded())
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
			dest_face->data().mPolyObjs.push_back(o);				
		}
		else
		{
			PolygonFromBlock(f,o.mShape, translator);
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
}

bool process_block(Pmwx::Face_handle f, CDT& mesh, DEMGeo& forest_dem)
{
	bool ret = false;
	int z = f->data().GetZoning();
	if(z == NO_VALUE || z == terrain_Natural || gZoningInfo.count(z) == 0)
		return false;
		
	CoordTranslator2	trans;
	Block_2 block;
	
	if(init_block(mesh, f, block, trans))
	{
		simplify_block(block, 0.75);
		clean_block(block);

		if (apply_fill_rules(z, block, trans, forest_dem))
			ret = true;
		extract_features(block, f, trans);
	}
	return ret;
}