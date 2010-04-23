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

#include "BlockFill.h"
#include "BlockAlgs.h"
#include "CompGeomDefs2.h"
#include "MapPolygon.h"
#include "GISUtils.h"
#include "NetTables.h"

#define BLOCK_ERR_MTR 1.0 
struct block_pt {
	bool operator==(const block_pt& rhs) const { return loc == rhs.loc; }
	Point2			loc;
	Point2			offset_prev;	// This is our offset location parallel to our PREVIOUS segment.
	Point2			offset_next;	// This is our offset location parallel to our NEXT segment.
	bool			locked;			// Is this point locked.  We lock any point that has high internal valence.
	int				edge_type;		// Edge type OUTGOING from this point.
	float			dot;
	Halfedge_handle	orig;
};

struct block_pt_locked {
	bool operator()(const block_pt& p) const { return p.locked; }
};
struct block_pt_err {
	bool operator()(const block_pt& p1, const block_pt& p2, const block_pt& x) const {
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



inline	void	MoveSegLeft(const Segment2& l1, double dist, Segment2& l2, Vector2& offset_vector)
{
	DebugAssert(l1.p1 != l1.p2);
	DebugAssert(dist > 0.0);
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
	if(gNetReps.count(seg_type) == 0) return 1.0f;
	return max(1.0f, gNetReps[seg_type].width);
}

static void	init_road_ccb(Pmwx::Ccb_halfedge_circulator he, CoordTranslator_2& translator, vector<block_create_t>& pieces)
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
		pts.back().loc = cgal2ben(translator.Forward(circ->source()->point()));
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
		Vector2		vv;
		MoveSegLeft(real_side, WidthForSegment(pts[i].edge_type), offset, vv);

		pts[i].offset_next = offset.p1;
		pts[j].offset_prev = offset.p2;
	}
	
	for(i = 0; i < pts.size(); ++i)
	{
		j = (i+1) % pts.size();
		k = (i+2) % pts.size();

		pieces.push_back(block_create_t());
		pieces.back().bounds.push_back(ben2cgal(pts[i].loc));
		pieces.back().bounds.push_back(ben2cgal(pts[j].loc));
		pieces.back().bounds.push_back(ben2cgal(pts[j].offset_prev));
		pieces.back().bounds.push_back(ben2cgal(pts[i].offset_next));
		pieces.back().data.usage = usage_Road;
		pieces.back().data.feature = pts[i].edge_type;
		
	} 
	
}

void	init_block(
					Pmwx::Face_handle		face,
					Block_2&				out_block,
					CoordTranslator_2&		translator)
{
	DebugAssert(!face->is_unbounded());

	Pmwx::Ccb_halfedge_circulator circ, stop;
	circ = stop = face->outer_ccb();
	Point_2	minp(circ->source()->point());
	Point_2 maxp(minp);
	
	do {
		if(circ->source()->point().x() < minp.x())
			minp = Point_2(circ->source()->point().x(), minp.y());
		if(circ->source()->point().y() < minp.y())
			minp = Point_2(minp.x(),circ->source()->point().y());
		if(circ->source()->point().x() > maxp.x())
			maxp = Point_2(circ->source()->point().x(), maxp.y());
		if(circ->source()->point().y() > maxp.y())
			maxp = Point_2(maxp.x(),circ->source()->point().y());

	} while(++circ != stop);
	
	CreateTranslatorForBounds(minp, maxp, translator);

	vector<block_create_t>	parts;
	
	init_road_ccb(face->outer_ccb(), translator, parts);
	for(Pmwx::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h)
		init_road_ccb(*h, translator, parts);
			
	
	create_block(out_block,parts);

}					

void	apply_fill_rules(
					Block_2&				block,
					CoordTranslator_2&		translator)
{
	for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
	if(!f->is_unbounded())
	if(f->data().usage == usage_Empty)
	{
		f->data().usage = usage_Polygonal_Feature;
		f->data().feature = feat_ForestPark;
		f->data().pre_placed = false;
	}
}


void	PolygonFromBlockCCB(Block_2::Ccb_halfedge_const_circulator circ, Polygon_2& out_poly, CoordTranslator_2& translator)
{
	out_poly.clear();
	Block_2::Ccb_halfedge_const_circulator stop = circ;

	do {
		out_poly.push_back(translator.Reverse(circ->source()->point()));
	} while (stop != ++circ);
}

void	PolygonFromBlock(Block_2::Face_const_handle in_face, Polygon_with_holes_2& out_ps, CoordTranslator_2& translator)
{
	Polygon_2			outer_ccb;
	vector<Polygon_2>	holes;

	if(!in_face->is_unbounded())
		PolygonFromBlockCCB(in_face->outer_ccb(), outer_ccb, translator);

	for(Block_2::Hole_const_iterator h = in_face->holes_begin(); h != in_face->holes_end(); ++h)
	{
		holes.push_back(Polygon_2());
		PolygonFromBlockCCB(*h, holes.back(), translator);
	}
	out_ps = Polygon_with_holes_2(outer_ccb,holes.begin(),holes.end());
}



void	extract_features(
					Block_2&				block,
					Pmwx::Face_handle		dest_face,
					CoordTranslator_2&		translator)
{
	for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
	if(!f->is_unbounded())
	if(f->data().usage == usage_Polygonal_Feature)
	if(!f->data().pre_placed)
	{
		GISPolyObjPlacement_t o;
		
		o.mRepType = f->data().feature;
		o.mHeight = 10.0;
		o.mDerived = true;
		PolygonFromBlock(f,o.mShape, translator);
		dest_face->data().mPolyObjs.push_back(o);
	}
}
