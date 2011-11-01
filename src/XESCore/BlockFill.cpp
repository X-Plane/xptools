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
#include "RF_Selection.h"
#include "MapHelpers.h"
#include "NetHelpers.h"
#include "UTL_interval.h"

int num_block_processed = 0;
int num_blocks_with_split = 0;
int num_forest_split = 0;
int num_line_integ = 0;

#include <stdarg.h>

typedef UTL_interval<double>	time_region;

#include "GISTool_Globals.h"

#define DEBUG_BLOCK_CREATE_LINES 0

#include <CGAL/Arr_overlay_2.h>

#define FOREST_SUBDIVIDE_AREA	(1000000.0)

#define BLOCK_ERR_MTR 0.5

#define map2block(X) (X)

struct block_pt_locked {
	bool operator()(const block_pt& p) const { return p.locked; }
};
struct block_pt_err {
	double operator()(const block_pt& p1, const block_pt& p2, const block_pt& x) const {
		DebugAssert(p1.loc != p2.loc);
		return Segment2(p1.loc,p2.loc).squared_distance(x.loc); 
	}
};

struct block_pt_compare {
	bool operator()(const block_pt& p1, const block_pt& p2) const {
		return p1.loc == p2.loc;
	}
};

template<typename __InputIterator>
struct ring_node
{
	typedef ring_node<__InputIterator>							self_type;
	typename multimap<double,self_type *>::iterator				self;
	
	self_type *		orig_prev;
	self_type *		orig_next;
	self_type *		prev;
	self_type *		next;
	__InputIterator	pt;
	
};

template <typename __InputIterator, typename __MeasureFunctor>
double measure_ring_node(ring_node<__InputIterator> * node, const __MeasureFunctor& measure)
{
	double err = measure(*node->prev->pt, *node->next->pt, *node->pt);

	ring_node<__InputIterator> * iter = node->orig_prev;
	while(iter != node->prev)
	{
		err = max(err,measure(*node->prev->pt, *node->next->pt, *iter->pt));
		iter = iter->orig_prev;
	}

	iter = node->orig_next;
	while(iter != node->next)
	{
		err = max(err,measure(*node->prev->pt, *node->next->pt, *iter->pt));
		iter = iter->orig_next;
	}
	return err;
}

template <typename __InputIterator, typename __OutputIterator, typename __LockFunctor, typename __MeasureFunctor, typename __CompareFunctor>
void ring_simplify(
			__InputIterator			start,
			__InputIterator			end,
			__OutputIterator		out,
			double					epsi,
			const __LockFunctor&	lock_check,
			const __MeasureFunctor& measure,
			const __CompareFunctor& compare)
{
	typedef ring_node<__InputIterator>	node;
	typedef multimap<double, node *>	queue_t;
	
	queue_t	q;
	node * head = NULL;
	node * prev = NULL;
	
	int nc = 0;
	for(__InputIterator p(start); p != end; ++p)
	{
		node * n = new node;
		//printf("%d: %p locked = %s\n", nc, n, lock_check(*p) ? "yes" : "no");
		if(head == NULL) head = n;
		n->next = n->orig_next = NULL;
		if(prev)
		{
			prev->next = n;
			prev->orig_next = n;
			n->prev = prev;
			n->orig_prev = prev;
		} 
		else
		{
			n->prev = NULL;
			n->orig_prev = NULL;
		}
		n->pt = p;
		n->self = q.end();		
		++nc;
		prev = n;
	}
	prev->next = head;
	prev->orig_next = head;
	head->prev = prev;
	head->orig_prev = prev;
	
	node * iter = head;
	
	do
	{
		if(!lock_check(*iter->pt))
		if(!compare(*iter->prev->pt, *iter->next->pt))
		{
			double err = measure_ring_node(iter,measure);
			if(err < epsi)
				iter->self = q.insert(typename queue_t::value_type(err, iter));
		}
		iter = iter->orig_next;
	} while(iter != head);
	
	while(!q.empty() && nc > 3)
	{
		iter = q.begin()->second;
		q.erase(q.begin());
		iter->self = q.end();

		--nc;
		iter->prev->next = iter->next;
		iter->next->prev = iter->prev;
		
		if(iter == head)
			head = iter->next;
		
		if(iter->prev->self != q.end())
		{
			q.erase(iter->prev->self);
			if(!compare(*iter->prev->prev->pt, *iter->prev->next->pt))
			{
				double err = measure_ring_node(iter->prev,measure);
				if(err < epsi)
					iter->prev->self = q.insert(typename queue_t::value_type(err, iter->prev));
				else
					iter->prev->self = q.end();			
			}
			else
				iter->prev->self = q.end();						
		}
		if(iter->next->self != q.end())
		{
			q.erase(iter->next->self);
			if(!compare(*iter->next->prev->pt, *iter->next->next->pt))
			{
				double err = measure_ring_node(iter->next,measure);
				if(err < epsi)
					iter->next->self = q.insert(typename queue_t::value_type(err, iter->next));
				else
					iter->next->self = q.end();			
			} else
				iter->next->self = q.end();			
		}
	}
	
	iter = head;
	int ne = 0;
	do 
	{
		//printf("%d: %p (%lf,%lf)\n", ne, iter, iter->pt->loc.x(), iter->pt->loc.y());
		++ne;
	
		*out++ = *iter->pt;

		iter = iter->next;
	} while(iter != head);
	
	iter = head;
	do
	{
		node * k = iter;
		iter = iter->orig_next;
		delete k;
	} while(iter != head);
}

struct simple_pt_locked {
	bool operator()(const Point2& p) const { return false; }
};
struct simple_pt_err {
	double operator()(const Point2& p1, const Point2& p2, const Point2& x) const {
		return Segment2(p1,p2).squared_distance(x); 
	}
};

struct simple_pt_compare {
	double operator()(const Point2& p1, const Point2& p2) const {
		return p1 == p2;
	}
};

void ring_simplify_polygon(Polygon2& io_poly, double err)
{
	Polygon2	reduced;
	ring_simplify(io_poly.begin(),io_poly.end(),back_inserter(reduced), err, simple_pt_locked(),simple_pt_err(), simple_pt_compare());
	io_poly.swap(reduced);
}

#if OPENGL_MAP && DEV
static void	debug_show_block(Block_2& io_block, CoordTranslator2& t)
{
	for(Block_2::Halfedge_iterator e = io_block.halfedges_begin(); e != io_block.halfedges_end(); ++e)
	{
		Segment2 s(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()));
		Vector2 v(s.p1,s.p2);
		v.normalize();
		v = v.perpendicular_ccw();
		v *= 0.1;
		s.p1 += v;
		s.p2 += v;
		s.p1 = t.Reverse(s.p1);
		s.p2 = t.Reverse(s.p2);
		
		if(e->face()->is_unbounded())
		{
			debug_mesh_line(s.p1,s.p2,1,0,1,1,0,1);
		} else switch(e->face()->data().usage) {
		case usage_Road:			
		case usage_Road_End:					debug_mesh_line(s.p1,s.p2,0.7,0.7,0.7,0.7,0.7,0.7);		break;
		case usage_Empty:						debug_mesh_line(s.p1,s.p2,0.8,0.8,0,0.8,0.8,0);			break;
		case usage_Steep:						debug_mesh_line(s.p1,s.p2,0,0.5,0,0,0.5,0);	break;

		case usage_Point_Feature:
		case usage_Polygonal_Feature:			debug_mesh_line(s.p1,s.p2,0,0,1,0,0,1);	break;
		case usage_Forest:						debug_mesh_line(s.p1,s.p2,0,1,0,0,1,0);	break;
		
		case usage_OOB:							debug_mesh_line(s.p1,s.p2,1,0,0,1,0,0);
		}
	}
}
#endif

inline void		encode_ag_height(unsigned short& param, float h)
{
	h *= 0.25;
	unsigned int hlim = intlim(h,1,255);
	param |= (hlim << 8);
}

double my_turn_degs(const Point2& p1, const Point2& p2, const Point2& p3)
{
	if(p1 == p3)	return 180.0;
	Vector2	v1(p1,p2);
	Vector2 v2(p2,p3);
	v1.normalize();
	v2.normalize();
	double dot = doblim(v1.dot(v2),-1.0,1.0);
	double ang = doblim(acos(dot) * RAD_TO_DEG,-180.0,180.0);
	if(left_turn(p1,p2,p3))
		return ang;
	else
		return -ang;
	
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

static pair<int,bool>	WidestRoadTypeForSegment(Pmwx::Halfedge_const_handle he)
{
	int	best_type = NO_VALUE;
	bool flip = false;
	double best_width = 0.0;
	for (GISNetworkSegmentVector::const_iterator i = he->data().mSegments.begin(); i != he->data().mSegments.end(); ++i)
	{
		DebugAssert(i->mRepType != NO_VALUE);
		if (gNetReps[i->mRepType].width() > best_width)
		{
			best_type = i->mRepType;
			best_width = gNetReps[i->mRepType].width();
		}
	}

	for (GISNetworkSegmentVector::const_iterator i = he->twin()->data().mSegments.begin(); i != he->twin()->data().mSegments.end(); ++i)
	{
		if(i->mRepType == NO_VALUE)	return pair<int,bool>(i->mFeatType,true);
		if (gNetReps[i->mRepType].width() > best_width)
		{
			best_type = i->mRepType;
			flip = true;
			best_width = gNetReps[i->mRepType].width();
		}
	}
	return pair<int,bool>(best_type, flip);
}

float WidthForSegment(const pair<int,bool>& seg_type)
{
	if(gNetReps.count(seg_type.first) == 0) return 0.0f;
	if(seg_type.second)
		return gNetReps[seg_type.first].semi_r;
	else
		return gNetReps[seg_type.first].semi_l;
}

static bool edges_match(Block_2::Halfedge_handle a, Block_2::Halfedge_handle b)
{
	return a->twin()->face()->data().usage == b->twin()->face()->data().usage &&
		   a->twin()->face()->data().feature== b->twin()->face()->data().feature;
}

static EdgeRule_t * edge_for_road(const pair<int,bool>& road_type, int zoning, int variant)
{
	for(EdgeRuleTable::iterator e = gEdgeRules.begin(); e != gEdgeRules.end(); ++e)
	if(e->zoning == zoning && e->road_type == road_type.first)
	if(e->variant == -1 || e->variant == variant)
		return &*e;
	return NULL;
}

static bool against_road(Block_2::Halfedge_const_handle h)
{
	return h->twin()->face()->data().usage == usage_Road &&
		   gNetReps[h->twin()->face()->data().feature].use_mode == use_Street;
}

inline void push_block_curve(vector<Block_2::X_monotone_curve_2>& curves, const Point2& p1, const Point2& p2, int idx1)
{
	DebugAssert(idx1 >= 0);
	DebugAssert(p1 != p2);
	curves.push_back(Block_2::X_monotone_curve_2(BSegment_2(ben2cgal<BPoint_2>(p1),ben2cgal<BPoint_2>(p2)),idx1));
}

inline void push_block_curve(vector<Block_2::X_monotone_curve_2>& curves, const Point2& p1, const Point2& p2, int idx1, int idx2)
{
	DebugAssert(idx1 >= 0);
	DebugAssert(idx2 >= 0);
//	push_block_curve(curves,p1,p2,idx1);
//	push_block_curve(curves,p1,p2,idx2);
//	return;
	DebugAssert(idx1 != idx2);
	EdgeKey_container	keys;
	keys.insert(idx1);
	keys.insert(idx2);
	DebugAssert(p1 != p2);
	curves.push_back(Block_2::X_monotone_curve_2(BSegment_2(ben2cgal<BPoint_2>(p1),ben2cgal<BPoint_2>(p2)),keys));
}

int find_most_locked_pt(vector<block_pt>& pts, bool skip_first)
{
	int r = -1;
	double most_area = 0.0;
	for(int i = 0; i < pts.size(); ++i)
	{
		int j = (i + 1) % pts.size();
		int k = (i + 2) % pts.size();
	
		if(j == 0 && skip_first)
			continue;
	
		if(pts[j].locked)
			return j;
				
		Vector2 v1(pts[i].loc,pts[j].loc);
		Vector2 v2(pts[k].loc,pts[j].loc);
		
		float area = fabs(v1.signed_area(v2));
		if(r == -1 || area > most_area)
		{
			r = j;
			most_area = area;
		}		
	}
	DebugAssert(r >= 0);
	return r;
}

void	rotate_to_corner(vector<block_pt>& outer_ccb_pts)
{
	DebugAssert(outer_ccb_pts.size() > 2);
	int c = 0;
	double dot = 1.0;
	for(int i = 0; i < outer_ccb_pts.size(); ++i)
	{
		int j = (i+1 ) % outer_ccb_pts.size();
		int k = (i+2 ) % outer_ccb_pts.size();
		Vector2	v1(outer_ccb_pts[i].loc,outer_ccb_pts[j].loc);
		Vector2	v2(outer_ccb_pts[j].loc,outer_ccb_pts[k].loc);
		v1.normalize();
		v2.normalize();
		double my_dot = v1.dot(v2);
		if(my_dot < dot)
		{
			c = j;
			dot = my_dot;
		}
	}
	if(c != 0)
		rotate(outer_ccb_pts.begin(),outer_ccb_pts.begin()+c,outer_ccb_pts.end());
}

bool block_pts_from_ccb(
			Pmwx::Ccb_halfedge_circulator	he, 
			CoordTranslator2&				translator, 
			vector<block_pt>&				pts,
			double							dp_err_mtr,
			bool							is_hole)
{
	Pmwx::Ccb_halfedge_circulator circ, stop;
	circ = stop = he;
	do {
		pts.push_back(block_pt());
		pts.back().loc = translator.Forward(cgal2ben(circ->source()->point()));
		pts.back().edge_type = WidestRoadTypeForSegment(circ);
		pts.back().locked = WidestRoadTypeForSegment(circ->prev()) != pts.back().edge_type;
		pts.back().antenna = circ->face() == circ->twin()->face();
		pts.back().reflex = false;
		pts.back().orig = circ;
		
		// We do NOT do dot products here!!  Calculate after we reduce unneeded points.

		int n = 0;
		Pmwx::Halfedge_around_vertex_circulator vcirc, vstop;
		vcirc = vstop = circ->source()->incident_halfedges();
		do {
			// We are counting the INTERNAL degree of the vertex - that is, how many edges come out of this vertex that
			// WE care about.  the corner of a square block has internal degree 2 but total deree 4 - we don't CARE about the
			// roads that aren't part of us.
			if(vcirc->face() == circ->face())
				++n;
			if(vcirc->twin()->face() == circ->face())
				++n;
			if(vcirc->face() == circ->face())
			if(vcirc->twin()->face() == circ->face())
				pts.back().locked = true;
		} while(++vcirc != vstop);
		
		if(n > 2)
			pts.back().locked = true;
		
	} while(++circ != stop);

	for(int n = 1; n < pts.size(); ++n)
	{
		// If the side BEFORE n and the side AFTER n don't have the same 
		// antenna status, LOCK the point.  I don't know WHY we wouldn't already
		// be locked, but we can't have DP changing our antenna status!!
		if(pts[n-1].antenna != pts[n].antenna)
		{
			//if(!pts[n].locked)
			//	debug_mesh_point(translator.Reverse(pts[n].loc),1,0,1);
			DebugAssert(pts[n].locked);
			pts[n].locked = true;
		}
	}

	int r = find_most_locked_pt(pts,false);
	if(r != 0)
		rotate(pts.begin(),pts.begin()+r,pts.end());

	DebugAssert(pts.size() >= (is_hole ? 2 :3));

	for(int k = 1; k < pts.size(); ++k)
	if(pts[k-1].loc == pts[k].loc)
	{
		pts.erase(pts.begin()+k);
		--k;
	}
	while(pts.size() > 1 && pts.front().loc == pts.back().loc)
		pts.pop_back();
	
	if(pts.size() < (is_hole ? 2 : 3))
		return false;
	
#if DEV
	for(int k = 0; k < pts.size(); ++k)
		DebugAssert(pts[k].loc != pts[(k+1)%pts.size()].loc);
	DebugAssert(pts.size() >= (is_hole ? 2 :3));
#endif

	if(dp_err_mtr > 0.0 && pts.size() > 3)
	{
//		r = find_most_locked_pt(pts,true);
//		pts[r].reflex =true;
//		pts.push_back(pts.front());	// this closes the "ring".

		vector<block_pt> pts2;
	
//		DebugAssert(r != 0);
//		DebugAssert(r != (pts.size()-1));
//		douglas_peuker(pts.begin(),pts.begin() + r,back_inserter(pts2), BLOCK_ERR_MTR * BLOCK_ERR_MTR, block_pt_locked(), block_pt_err());
//		douglas_peuker(pts.begin() + r,pts.end()-1,back_inserter(pts2), BLOCK_ERR_MTR * BLOCK_ERR_MTR, block_pt_locked(), block_pt_err()); 
		ring_simplify(pts.begin(),pts.end(), back_inserter(pts2),dp_err_mtr * dp_err_mtr, block_pt_locked(), block_pt_err(), block_pt_compare());
		DebugAssert(pts2.size() >= (is_hole ? 2 :3));

		#if DEV
			for(int k = 0; k < pts2.size(); ++k)
				DebugAssert(pts2[k].loc != pts2[(k+1)%pts2.size()].loc);
		#endif

		pts.swap(pts2);

	}
	return true;
}

double find_b_interval(time_region::interval& range, const vector<Segment2>& segs, double b_range[4])
{
	double minv = b_range[3];
	double maxv = b_range[1];
	for(vector<Segment2>::const_iterator s = segs.begin(); s != segs.end(); ++s)
	{
		DebugAssert(s->p1.x() < s->p2.x());
		if(range.first > s->p2.x())
			continue;
		if(range.second < s->p1.x())
			continue;
		
		double xmin = max(s->p1.x(),range.first);
		double xmax = min(s->p2.x(),range.second);
		DebugAssert(xmin <= xmax);
		DebugAssert(xmin >= range.first);
		DebugAssert(xmax <= range.second);
		DebugAssert(xmin >= s->p1.x());
		DebugAssert(xmax <= s->p2.x());
		
		double y1 = s->y_at_x(xmin);
		double y2 = s->y_at_x(xmax);
		minv = min(min(y1,y2),minv);
		maxv = max(max(y1,y2),maxv);
	}
	
	return max(maxv-minv,0.0);
}

static int	init_subdivisions(
							Pmwx::Face_handle f,
							const FillRule_t * info, CoordTranslator2& translator, 
							vector<block_pt>& outer_ccb_pts, 		
							vector<BLOCK_face_data>& parts, 
							vector<Block_2::X_monotone_curve_2>& curves)
{
	DebugAssert(info->fac_id != NO_VALUE || info->agb_id != NO_VALUE);

	vector<pair<Pmwx::Halfedge_handle,Pmwx::Halfedge_handle> >	sides;
	Polygon2													mbounds;
	
	if(!build_convex_polygon(f->outer_ccb(),sides,translator,mbounds, 2.0, 2.0))
		return 0;
	
	Vector2	va, vb;
	int mj = pick_major_axis(sides,mbounds,va,vb);
	DebugAssert(mj >= 0);
	
	double bounds[4];

	bounds[0] = bounds[2] = va.dot(Vector2(mbounds[0]));
	bounds[1] = bounds[3] = vb.dot(Vector2(mbounds[0]));
	for(int i = 1; i < mbounds.size(); ++i)
	{
		double ca = va.dot(Vector2(mbounds[i]));
		double cb = vb.dot(Vector2(mbounds[i]));
		bounds[0] = min(bounds[0],ca);
		bounds[1] = min(bounds[1],cb);
		bounds[2] = max(bounds[2],ca);
		bounds[3] = max(bounds[3],cb);	
	}

	
	time_region		agb_friendly(bounds[0],bounds[2]);
	
	#define AGB_SUBDIV_LEN	(info->agb_min_width)
	#define FACADE_A_SLOP (info->agb_slop_width)
	#define FACADE_B_SLOP (info->agb_slop_depth)
	
	#define MIN_AGB_SIZE	(info->agb_min_width)
	#define MIN_FAC_EXTRA_ADD (info->fac_extra)
	#define FAC_SUBDIV_WIDTH (info->fac_width)
	#define FAC_DEPTH_SPLIT (info->fac_depth ? info->fac_depth : 100000.0)

	
	double fac_extra_len = MIN_FAC_EXTRA_ADD;
	for(int i = 0; i < outer_ccb_pts.size(); ++i)
	{
		if(outer_ccb_pts[i].edge_type.first != NO_VALUE)
			fac_extra_len = dobmax2(fac_extra_len,gNetReps[outer_ccb_pts[i].edge_type.first].width());
	}
	double real_min_agb = fac_extra_len * 2.0 + MIN_AGB_SIZE;

	
	// facade depth splits!
	int fds = intlim(floor(fabs(bounds[3]-bounds[1]) / FAC_DEPTH_SPLIT), 1, 2);	// NEVER split more than twice.  Middle facades are STRANDED!
	
	DebugAssert(outer_ccb_pts.size() >= 3);

	vector<Segment2>	top_agbs, bot_agbs;
	
	for(int i = 0; i < outer_ccb_pts.size(); ++i)
	{
		int j = (i + 1) % outer_ccb_pts.size();
		
		Point2	pi(va.dot(Vector2(outer_ccb_pts[i].loc)),vb.dot(Vector2(outer_ccb_pts[i].loc)));
		Point2	pj(va.dot(Vector2(outer_ccb_pts[j].loc)),vb.dot(Vector2(outer_ccb_pts[j].loc)));
		Vector2	v_side(pi,pj);
		bool top = v_side.dx < 0.0;
		v_side.dx = fabs(v_side.dx);
		v_side.dy = fabs(v_side.dy);
		DebugAssert(AGB_SUBDIV_LEN > 0.0);
		double max_facade_subdivs = dobmax2(1,floor(v_side.dx / AGB_SUBDIV_LEN));
		// Test for too much north-south travel is tempered by the distance - basically we can subdivide
		// horizontally to fit AGBs in with some block drift.
		if(v_side.dy > (FACADE_B_SLOP * max_facade_subdivs))
		{		
			// This side is sloping too much north-south to be an AGB.  But it MIGHT be the west or easet side
			// of the block.
			
			// Since we don't subdivide vertically, we measure for absolute error on the easet and west side.
			if(v_side.dx > FACADE_A_SLOP)
			{
				// okay - we have a slanty side - mark this east-west interval as NOT an AGB!
				DebugAssert(pi.x() != pj.x());
				agb_friendly -= time_region::interval(min(pi.x(),pj.x()),max(pi.x(),pj.x()));
			}
		} 
		else
		{
			v_side.normalize();
			if(v_side.dy > v_side.dx)
			{
				// This is an AGB - because it's the top or bottom - save its path for later use in subdividing AGbB.
				if(top)
				{
					DebugAssert(pj.x() < pi.x());
					top_agbs.push_back(Segment2(pj,pi));
				}
				else
				{
					DebugAssert(pi.x() < pj.x());
					bot_agbs.push_back(Segment2(pi,pj));
				}
			}
		}
	}
	
	if(info->fac_id == NO_VALUE)
	{
		if(!agb_friendly.is_simple() ||					// Hrm...AGB block has a facade hole in the middle
			agb_friendly.empty() ||						// Whole area is facade
			agb_friendly.get_min() > bounds[0] ||		// Block starts with facade
			agb_friendly.get_max() < bounds[2] ||		// Block ends with facade
			(bounds[2] - bounds[0]) < real_min_agb)		// Block is TOO SMALL - will become a facade!!
			
			return 0;
	}
	
	time_region		must_be_fac_too;
	for(time_region::const_iterator agbs = agb_friendly.begin(); agbs != agb_friendly.end(); ++agbs)
	{
		double width = agbs->second - agbs->first;
		if(width < real_min_agb)
		{
			DebugAssert(info->fac_id != NO_VALUE);
			must_be_fac_too += *agbs;			
		}
		else
		{
			double must_have = 0.0;
			if(agbs->first != bounds[0])
				must_have += fac_extra_len;
			if(agbs->second != bounds[2])
				must_have += fac_extra_len;
			if(width > must_have)
			{
				if(agbs->first != bounds[0])
					must_be_fac_too += time_region::interval(agbs->first,agbs->first + fac_extra_len);
				if(agbs->second != bounds[2])
					must_be_fac_too += time_region::interval(agbs->second - fac_extra_len, agbs->second);
			} else 
				must_be_fac_too += *agbs;			
		}		
	}
	
	agb_friendly -= must_be_fac_too;

//	agb_friendly ^= time_region::interval(bounds[0],bounds[2]);

	vector<pair<double, bool> >	a_cuts;

	double last = bounds[0];

	time_region::const_iterator agb_iter = agb_friendly.begin();
	while(1)
	{
		// output fac BEFORE agb, and agb.
		// if agb == end, output last fac AFTER AGB.  outer FACs AM not exist.
		time_region::interval agb,fac;
		
		// Last hanging fac, but it doens't exist.  Bail.
		if(agb_iter == agb_friendly.end() && (!agb_friendly.empty() && agb_friendly.get_max() == bounds[2]))
			break;
			
		if(agb_iter == agb_friendly.end())
		{
			fac = time_region::interval(agb_friendly.empty() ? bounds[0] : agb_friendly.get_max(), bounds[2]);
			agb = time_region::interval(bounds[2],bounds[2]);
		} 
		else
		{
			fac = time_region::interval(last,agb_iter->first);
			agb = *agb_iter;
		}
		
		if(fac.first != fac.second)
		{
			double width = fac.second - fac.first;
			DebugAssert(FAC_SUBDIV_WIDTH > 0.0);
			double subdiv = dobmax2(1,floor(width / FAC_SUBDIV_WIDTH));
			DebugAssert(subdiv < 500);
			for(double s = 0.0; s < subdiv; s += 1.0)
			{
				a_cuts.push_back(pair<double,bool>(double_interp(0,fac.first,subdiv,fac.second,s),false));
			}		
		}
		if(agb.first != agb.second)
		{
			double top_span = find_b_interval(agb,top_agbs, bounds);
			double bot_span = find_b_interval(agb,bot_agbs, bounds);
			double worst_span = max(top_span,bot_span);
			double subdiv = dobmax2(1.0,floor(worst_span / FACADE_B_SLOP));
			
			for(double s = 0.0; s < subdiv; s += 1.0)
			{
				a_cuts.push_back(pair<double,bool>(double_interp(0,agb.first,subdiv,agb.second,s),true));
			}		
		}
				
		if(agb_iter == agb_friendly.end())
			break;
		
		last = agb_iter->second;		
		++agb_iter;
	}

	
	a_cuts.push_back(pair<double,bool>(bounds[2]+1,false));
	a_cuts.front().first -= 1.0;

	int part_base = parts.size();
	DebugAssert(a_cuts.size() >= 2);
	int a_blocks = a_cuts.size()-1;
	parts.resize(part_base + fds * a_blocks);
	
	for(int a = 0; a < a_blocks; ++a)
	for(int f = 0; f < fds; ++f)
	{
		int pid = part_base + a * fds + f;
		parts[pid].usage = usage_Polygonal_Feature;
		parts[pid].feature = a_cuts[a].second ? info->agb_id : info->fac_id;
		DebugAssert(parts[pid].feature);
		parts[pid].major_axis = va;
		DebugAssert(parts[pid].feature != NO_VALUE);
		parts[pid].can_simplify = false;
	}

	for(int a = 0; a < a_blocks; ++a)
	{		
		DebugAssert(a_cuts[a].first != a_cuts[a+1].first);
		
		bool fac_left = (a > 0 && !a_cuts[a-1].second) || !a_cuts[a].second;
		bool fac_right = !a_cuts[a].second || (a < a_blocks-1 && !a_cuts[a+1].second);
		bool fac = !a_cuts[a].second;
		for(int f = 0; f < fds; ++f)
		{	
			int pid = part_base + a * fds;
			if(!a_cuts[a].second)	pid += f;
			int pid_left = -1;
			int pid_right = -1;
			if(a > 0)
			{
				pid_left = part_base + (a-1)*fds;
				if(!a_cuts[a-1].second) pid_left += f;
			}
			if(a < a_blocks-1)
			{
				pid_right = part_base + (a+1)*fds;
				if(!a_cuts[a+1].second) pid_right += f;
			}
			
//			printf("Tile %d, %d: %d/%d/%d facleft=%s,facright=%s\n", a,f,pid_left,pid,pid_right,fac_left ? "yes":"no",fac_right ? "yes":"no");
			
			double bot_b = double_interp(0,bounds[1],fds,bounds[3],f  );
			double top_b = double_interp(0,bounds[1],fds,bounds[3],f+1);
			if(f == 0) bot_b = bounds[1]-1.0;
			if((f+1)==fds) top_b = bounds[3]+1.0;
			
			// This is the INNER rect - if we have FULL subdivision.
			Point2 bl = Point2(0,0) + va * a_cuts[a  ].first + vb * bot_b;
			Point2 tl = Point2(0,0) + va * a_cuts[a  ].first + vb * top_b;
			Point2 br = Point2(0,0) + va * a_cuts[a+1].first + vb * bot_b;
			Point2 tr = Point2(0,0) + va * a_cuts[a+1].first + vb * top_b;

			// This is the OUTER rect - if we don't
			Point2 Bl = Point2(0,0) + va * a_cuts[a  ].first + vb * (bounds[1]-1.0);
			Point2 Tl = Point2(0,0) + va * a_cuts[a  ].first + vb * (bounds[3]+1.0);
			Point2 Br = Point2(0,0) + va * a_cuts[a+1].first + vb * (bounds[1]-1.0);
			Point2 Tr = Point2(0,0) + va * a_cuts[a+1].first + vb * (bounds[3]+1.0);

			// BOTTOM edge.
			if(f == 0)
				push_block_curve(curves,bl,br,pid);
			else if(fac)
			{
				push_block_curve(curves,bl,br,pid,pid-1);
//				push_block_curve(curves,br,bl,pid-1);
//				push_block_curve(curves,bl,br,pid);
			}
			if(f == fds-1)
				push_block_curve(curves,tr,tl,pid);
			
			// LEFT edge
			if(fac_left)
			{
				if(a > 0)
				{
					push_block_curve(curves,tl,bl,pid,pid_left);
//					push_block_curve(curves,bl,tl,pid_left);
//					push_block_curve(curves,tl,bl,pid);
				}
				else
					push_block_curve(curves,tl,bl,pid		  );
			}
			else if(f == 0)
			{
				if(a > 0)
				{
					push_block_curve(curves,Tl,Bl,pid,pid_left);
//					push_block_curve(curves,Bl,Tl,pid_left);
//					push_block_curve(curves,Tl,Bl,pid);
				}	
				else
					push_block_curve(curves,Tl,Bl,pid		  );
			}	

			// RIGHT edge
			if(fac_right)
			{
				if(a < a_blocks-1)
				{
					push_block_curve(curves,br,tr,pid,pid_right);
//					push_block_curve(curves,br,tr,pid);
//					push_block_curve(curves,tr,br,pid_right);
				} else
					push_block_curve(curves,br,tr,pid		  );
			}
			else if(f == 0)
			{
				if(a < a_blocks-1)
				{
					push_block_curve(curves,Br,Tr,pid, pid_right);
//					push_block_curve(curves,Br,Tr,pid);
//					push_block_curve(curves,Tr,Br,pid_right);
				} else
					push_block_curve(curves,Br,Tr,pid		  );
			}	
	

		}
	}
	
	int ring_base = parts.size();
	
	for(int n = 0; n < mbounds.size(); ++n)
	if(ground_road_access_for_he(sides[n].first))
		parts.push_back(BLOCK_face_data(usage_Road,get_he_rep_type(sides[n].first)));
	else
		parts.push_back(BLOCK_face_data(usage_OOB,0));

	for(int n = 0; n < mbounds.size(); ++n)
	{
		Segment2	side(mbounds.side(n));
		
		double w = width_for_he(sides[n].first);
		if(w && ground_road_access_for_he(sides[n].first))
		{
			push_block_curve(curves,side.p1,side.p2,ring_base+n);
			
			Vector2	v_side(side.p1,side.p2);
			v_side.normalize();
			v_side = v_side.perpendicular_cw();
			v_side *= (50.0 + w);
			Segment2 rside(side.p1 + v_side,side.p2 + v_side);
			push_block_curve(curves,side.p1,rside.p1,ring_base+n,parts.size());
			push_block_curve(curves,side.p2,rside.p2,ring_base+n,parts.size());
			push_block_curve(curves,rside.p1,rside.p2,ring_base+n,parts.size());
			
		}
		else {
			push_block_curve(curves,side.p1,side.p2,parts.size());
		}
	}
	
	parts.push_back(BLOCK_face_data(usage_OOB,0));
	
	return parts.size() - part_base;
	
}

bool edge_ok_for_ag(const Point2& i, const Point2& j, CoordTranslator2& c, const DEMGeo& idx)
{
	Bbox2	bounds;
	bounds += c.Reverse(i);
	bounds += c.Reverse(j);

	int x1 = idx.x_lower(bounds.xmin());
	int x2 = idx.x_upper(bounds.xmax());

	int y1 = idx.y_lower(bounds.ymin());
	int y2 = idx.y_upper(bounds.ymax());
	
	for(int y = y1; y <= y2; ++y)
	for(int x = x1; x <= x2; ++x)
	if(idx.get(x,y) > 0.0)
		return true;
	return false;

	
}


static bool mismatched_edge_types(const pair<int,bool>& e1, const pair<int,bool>& e2, int zoning, int variant, bool fill_edges)
{
	float w1 = WidthForSegment(e1);
	float w2 = WidthForSegment(e2);
	if(w1 != w2) return true;
	if(!fill_edges)	return false;

	EdgeRule_t * r1 = edge_for_road(e1, zoning, variant);
	EdgeRule_t * r2 = edge_for_road(e1, zoning, variant);
	
	if (r1 == NULL && r2 == NULL) return false;
	if (r1 == NULL || r2 == NULL) return true;
	
	if(r1->width != r2->width || r1->resource_id != r2->resource_id) return true;
	return false;
}

static void	init_road_ccb(int zoning, int variant, Pmwx::Ccb_halfedge_circulator he, CoordTranslator2& translator, 
							vector<BLOCK_face_data>& pieces, 
							vector<Block_2::X_monotone_curve_2>& curves,
							vector<block_pt>&	pts,
							int& cur_base, int span, int oob_idx, bool fill_edges,
							const DEMGeo& approx_ag)
{

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
		// Any change in edge type is a "discontinuity" - we can't optimize out the sides of our road.
		// And...point 0 is ALWAYS discontinuous - um, something goes wrong if that isn't true.
		pts[j].discon = j == 0 || mismatched_edge_types(pts[j].edge_type,pts[i].edge_type, zoning, variant, fill_edges);
		
//		if(j == 0)
//			debug_mesh_point(translator.Reverse(pts[j].loc),1,1,0);

		// Special case: shallow reflext points without a discontinuity?  Treat as NON-reflex..
		// the non-reflex continuous case actually handles this with grace.
		if(pts[j].reflex && !pts[j].discon && pts[j].dot > 0.5)
		{
			pts[j].reflex = false;
		}
		// And - all TRUE reflex points ARE discontinuous...we have a crapload of overlapping stuff.
		if(pts[j].reflex) 
			pts[j].discon = true;

		Segment2	real_side(pts[i].loc, pts[j].loc);
		Segment2	offset;		
		MoveSegLeft(real_side, WidthForSegment(pts[i].edge_type), offset);
		pts[i].offset_next1 = offset.p1;
		pts[j].offset_prev1 = offset.p2;

		EdgeRule_t * er = (fill_edges && edge_ok_for_ag(pts[i].loc,pts[j].loc, translator,approx_ag)) ? edge_for_road(pts[i].edge_type, zoning, variant) : NULL;
		real_side = offset;
		MoveSegLeft(real_side, er ? er->width : 0.0, offset);

		pts[i].offset_next2 = offset.p1;
		pts[j].offset_prev2 = offset.p2;
		pts[i].edge_rule = er;
	}


	// Now if we have a corner, we are going to build that corner connecting geometry - and we MIGHT 
	// discover that we need to reclasify a bit too!
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
			EdgeRule_t * er_prev = pts[i].edge_rule;
			EdgeRule_t * er_next = pts[j].edge_rule;

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
			else if(pts[j].dot >= -0.03)
			{
				// We have a continuous shallow left turn.  We need to either: pull in the contours so that 
				// we form a nice smooth left turn, OR declare it discontinuous, if we can't handle it.
				// Note that we don't test for positive dot because, due to slight rounding, about half of the 90
				// degree real-world turns come out as "tight" (dot less than 0).  In practice our "reverser" test
				// can handle a case that is nearly 90 degrees, so this saves us from having to create a discontinuity
				// in about 50% of the square blocks out there.
				Point2	xon1, xon2;
				Line2 prev1(pts[i].offset_next1,pts[j].offset_prev1);
				Line2 prev2(pts[i].offset_next2,pts[j].offset_prev2);
				Line2 next1(pts[j].offset_next1,pts[k].offset_prev1);
				Line2 next2(pts[j].offset_next2,pts[k].offset_prev2);
				if(!prev1.intersect(next1,xon1))
					printf("WTF?\n");
				if(!prev2.intersect(next2,xon2))
					printf("WTF?\n");
				
				// There are basically TWO ways this can go to hell.
				// 1. If the segment is REALLY short, the intersection will be off of the segment's span.
				// 2. If the turn is really tight, the NEXT intersection happens behind the last one.
				// In both cases the net result is the same: we end up with a contour that is going...backward!
				// Detect this and declare that we can't flow through.
				
				Vector2	dir_prev_core(pts[i].loc,pts[j].loc);
				Vector2	dir_prev_off1(pts[i].offset_next1,xon1);
				Vector2	dir_prev_off2(pts[i].offset_next2,xon2);
				Vector2	dir_next_core(pts[j].loc,pts[k].loc);
				Vector2	dir_next_off1(xon1,pts[k].offset_prev1);
				Vector2	dir_next_off2(xon2,pts[k].offset_prev2);
				
				if (dir_prev_core.dot(dir_prev_off1) < 0.0 ||
					dir_prev_core.dot(dir_prev_off2) < 0.0 ||
					dir_next_core.dot(dir_next_off1) < 0.0 ||
					dir_next_core.dot(dir_next_off2) < 0.0)
				{
					pts[j].discon = true;
					//debug_mesh_point(translator.Reverse(pts[j].offset_prev1),1,0,1);
					//debug_mesh_point(translator.Reverse(pts[j].offset_next1),1,0,1);				
				}
				else
				{
					pts[j].offset_reflex1[0] = pts[j].offset_reflex1[1] = pts[j].offset_reflex1[2] = xon1;
					pts[j].offset_reflex2[0] = pts[j].offset_reflex2[1] = pts[j].offset_reflex2[2] = xon2;	
					// the reflex curve points are used to build a continuous curve, but ... edit the prev/next
					// pts too.  That way the NEXT contour will recognize that we are pulled "forward", and if it is
					// pull back, we recognize the reverse.  (If we don't do this, each end of the offset curve can be 
					// pulled PAST the other and we will not notice.
					pts[j].offset_prev1 = pts[j].offset_next1 = xon1;
					pts[j].offset_prev2 = pts[j].offset_next2 = xon2;
				}
			}
			else {
				// TIGHT left turn - don't even try, we can't figure out if we're going to have a crash.
				// Just make discontinous.  The math to figure out whether we crash is too hard.  PUNT!
				// It's useful later to know that all continuous turns are < 90 degrees.
				//debug_mesh_point(translator.Reverse(pts[j].loc),1,0,0);
				pts[j].discon = true;
			}
		}
		
	}
	
	// Finally: prevent a full loop on a SINGLE continuous curve.  This is a problem because by looping we could easily overlap.
	// So, a stupid test: when we have about reversed direction from our previous discontinuity, induce another one.  Note that if the 
	// continuity was allowed, prev and next are "tucked" so we end up with overlapped curves...doesn't really matter, we are still capped,
	// and that's what costs us WRT CGAL performance.

	DebugAssert(pts[0].discon);
	Vector2	start_of_run;
	double	h_accum = 0;
	for(i = 0; i < pts.size(); ++i)
	{
		j = (i+1) % pts.size();
		k = (i+pts.size()-1) % pts.size();
		Vector2	v(pts[i].loc,pts[j].loc);
		v.normalize();	
		
		// Continuous left turn that is now going about opposite?  Invoke discontinuity.
		// Exact angle isn't that important because big win is when we have a WINDY rode (and the heading
		// stays pretty constant) we can run FOREVER with a series of shallow left/right turns.
		if(!pts[i].discon)
		{
			h_accum += my_turn_degs(pts[k].loc,pts[i].loc,pts[j].loc);
			
			if(h_accum >= 180.0 || h_accum < -180.0)
			{
				pts[i].discon = true;
				//debug_mesh_point(translator.Reverse(pts[i].loc),0,1,1);
			}
		}

		if(pts[i].discon)
		{
			start_of_run = v;
			h_accum = 0;
		}
	}
	

	/********************************************************************************************************************************************
	 * ATTACH GEOMETRY
	 ********************************************************************************************************************************************/
	
	for(i = 0; i < pts.size(); ++i)
	{
		j = (i+1) % pts.size();
//		k = (i+2) % pts.size();

		EdgeRule_t * er = pts[i].edge_rule;
		float w = WidthForSegment(pts[i].edge_type);

		DebugAssert(er == NULL || w > 0.0);

		// PART 1 - Build geometry directly opposite the road.
		// This is the segment from I to J.

		if(er && w > 0.0)
		{		
			if(pts[i].discon && pts[j].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1, pts[i].loc,cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_prev1, cur_base + span, cur_base);

				push_block_curve(curves, pts[i].offset_next2, pts[i].offset_next1, cur_base);
				push_block_curve(curves, pts[j].offset_prev1,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_prev2, cur_base);
			}
			else if(pts[i].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1, pts[i].loc, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_reflex1[0], cur_base + span, cur_base);

				push_block_curve(curves, pts[i].offset_next2,pts[i].offset_next1, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_reflex2[0], cur_base);
			}
			else if(pts[j].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_reflex1[0],pts[j].offset_prev1, cur_base + span, cur_base);

				push_block_curve(curves, pts[j].offset_prev1,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_prev2, cur_base);
			}
			else
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].offset_reflex1[0],pts[j].offset_reflex1[0], cur_base + span, cur_base);

				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_reflex2[0], cur_base);
			}
		}
		else if(er)
		{
			if(pts[i].discon && pts[j].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[i].offset_next2, pts[i].loc,cur_base);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_prev2, cur_base);
			}
			else if(pts[i].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[i].offset_next2, pts[i].loc, cur_base);
				push_block_curve(curves, pts[i].offset_next2,pts[j].offset_reflex2[0], cur_base);
			}
			else if(pts[j].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev2, cur_base);
				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_prev2, cur_base);
			}
			else
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base);
				push_block_curve(curves, pts[i].offset_reflex2[0],pts[j].offset_reflex2[0], cur_base);
			}
		}
		else if (w > 0.0)
		{
			if(pts[i].discon && pts[j].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1, pts[i].loc,cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_prev1, cur_base + span);
			}
			else if(pts[i].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1, pts[i].loc, cur_base + span);
				push_block_curve(curves, pts[i].offset_next1,pts[j].offset_reflex1[0], cur_base + span);
			}
			else if(pts[j].discon)
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
				push_block_curve(curves, pts[j].loc,pts[j].offset_prev1, cur_base + span);
				push_block_curve(curves, pts[i].offset_reflex1[0],pts[j].offset_prev1, cur_base + span);
			}
			else
			{
				if(pts[i].antenna)	push_block_curve(curves, pts[i].loc,pts[j].loc,			 cur_base + span);
				else				push_block_curve(curves, pts[i].loc,pts[j].loc, oob_idx, cur_base + span);
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
			pieces[cur_base] = BLOCK_face_data(er ? usage_Polygonal_Feature : usage_Empty,er ? er->resource_id : NO_VALUE);
			pieces[cur_base + span] = BLOCK_face_data(usage_Road, pts[i].edge_type.first);
			
			++cur_base;
		}
		
		// Part 2 - reflex vertices.  These need a little "extra" around their edges.

		if(pts[j].reflex)
		{
			EdgeRule_t * er2 = pts[j].edge_rule;
			float w2 = WidthForSegment(pts[j].edge_type);
		
			if(!er || !er2 || er->width != er2->width || er->resource_id != er2->resource_id)
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
					push_block_curve(curves, pts[j].offset_prev1, pts[j].loc, cur_base + span);
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
					push_block_curve(curves, pts[j].offset_prev2,pts[j].offset_prev1, cur_base);
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
					push_block_curve(curves, pts[j].offset_prev2, pts[j].loc, cur_base);
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
					push_block_curve(curves, pts[j].offset_prev1, pts[j].loc,cur_base + span);
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
			
			pieces[cur_base] = BLOCK_face_data(er ? usage_Polygonal_Feature : usage_Empty,er ? er->resource_id : NO_VALUE);
			pieces[cur_base + span] = BLOCK_face_data(usage_Road, pts[i].edge_type.first);
			
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
	
	if(gNaturalTerrainInfo[lu].autogen_mode == URBAN)			return (f->info().normal[2] < cos(max_slope * DEG_TO_RAD)) ? cat_forest : cat_urban;
	else if (gNaturalTerrainInfo[lu].autogen_mode == FOREST)	return cat_forest;
	else														return cat_flat;
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

static void push_curve(vector<Block_2::X_monotone_curve_2>& curves, const BPoint_2& p1, const BPoint_2& p2, int cat1, int cat2, int urban_idx, int oob_idx)
{
	DebugAssert(cat1 != cat2);
	if(cat1 != urban_idx && cat2 != urban_idx && cat1 != oob_idx && cat2 != oob_idx)
	{
		EdgeKey_container	keys;
		keys.insert(cat1);
		keys.insert(cat2);
		curves.push_back(Block_2::X_monotone_curve_2(BSegment_2(p1, p2), keys));
	}
	else if(cat1 != urban_idx && cat1 != oob_idx)
	{
		curves.push_back(Block_2::X_monotone_curve_2(BSegment_2(p1, p2), cat1));
	}
	else if(cat2 != urban_idx && cat2 != oob_idx)
	{
		curves.push_back(Block_2::X_monotone_curve_2(BSegment_2(p1, p2), cat2));
	}	
}


static void	init_point_features(const GISPointFeatureVector& feats, 
							vector<Block_2::X_monotone_curve_2>& curves, 
							vector<BLOCK_face_data>& parts, 
							vector<block_pt>&	pts,
							CoordTranslator2&	trans,
							int offset, int zoning)
{
	PointRule_t * rule; 

	int idx = 0;
	for(GISPointFeatureVector::const_iterator f = feats.begin(); f != feats.end(); ++f, ++idx)
	if((rule = GetPointRuleForFeature(zoning, *f)) != NULL)
	{
		Point2	fl(trans.Forward(cgal2ben(f->mLocation)));
		int best = -1;
		double best_dist = 0;
		for(int i = 0; i < pts.size(); ++i)
		{
			int j = (i + 1) % pts.size();
			DebugAssert(pts[i].loc != pts[j].loc);
			if(ground_road_access_for_he(pts[i].orig))
			if(pts[i].loc != pts[j].loc)
			{
				double my_dist = Segment2(pts[i].loc,pts[j].loc).squared_distance(fl);				
				if(best == -1 || my_dist < best_dist)
				{	
					best = i;
					best_dist = my_dist;
				}
			}
		}
		if(best == -1)
			best = 0;
		
		Segment2	wall(pts[best].loc,pts[(best+1)%pts.size()].loc);
		Assert(wall.p1 != wall.p2);
		Point2		anchor(wall.midpoint());
		Vector2		width(wall.p1,wall.p2);
		width.normalize();
		Vector2	depth(width.perpendicular_ccw());
		
		float w = WidthForSegment(pts[best].edge_type);
		if(w > 1.0)
			anchor += (depth * (w-1.0));
		
		width *= rule->width * 0.5;
		depth *= rule->depth;
		
		push_block_curve(curves, anchor - width, anchor + width, offset + idx);
		push_block_curve(curves, anchor + width, anchor + width + depth, offset + idx);
		push_block_curve(curves, anchor + width + depth, anchor - width + depth, offset + idx);
		push_block_curve(curves, anchor - width + depth, anchor - width, offset + idx);
		
		parts[offset + idx].usage = usage_Polygonal_Feature;
		parts[offset + idx].feature = rule->fac_id;
		
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
			BPoint_2	p01 = ben2cgal<BPoint_2>(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(0)->point(),f->vertex(1)->point()))));
			BPoint_2	p12 = ben2cgal<BPoint_2>(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(1)->point(),f->vertex(2)->point()))));
			BPoint_2	p20 = ben2cgal<BPoint_2>(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(2)->point(),f->vertex(0)->point()))));
			BPoint_2 p012 = ben2cgal<BPoint_2>(translator.Forward(cgal2ben(CGAL::centroid(mesh.triangle(f)))));
			
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
				BPoint_2	pl = ben2cgal<BPoint_2>(translator.Forward(cgal2ben(f->vertex(CDT::cw (n))->point())));
				BPoint_2	pr = ben2cgal<BPoint_2>(translator.Forward(cgal2ben(f->vertex(CDT::ccw(n))->point())));
				BPoint_2	pm = ben2cgal<BPoint_2>(translator.Forward(cgal2ben(CGAL::midpoint(f->vertex(CDT::cw (n))->point(),f->vertex(CDT::ccw(n))->point()))));

				if(l == r && L == R)
				{
					if(l != L && pl != pr)
						push_curve(curves,pr,pl,l,L,cat_table[cat_urban], cat_table[cat_oob]);
				}
				else
				{
					if(l != L && pm != pl)
						push_curve(curves,pm,pl,l,L,cat_table[cat_urban], cat_table[cat_oob]);
					if(r != R && pr != pm)
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
					CoordTranslator2&		translator,
					const DEMGeo&			ag_ok_approx_dem)
{
	DebugAssert(!face->is_unbounded());

	int zoning = face->data().GetZoning();
	int variant = face->data().GetParam(af_Variant,-1);
	ZoningInfoTable::iterator zi = gZoningInfo.find(zoning);
	ZoningInfo_t * info = (zi == gZoningInfo.end() ? NULL : &zi->second);
	if(info == NULL) return false;

	int has_road = 0;
	int has_non_road = 0;

	Pmwx::Ccb_halfedge_circulator circ, stop;
	circ = stop = face->outer_ccb();
	Bbox2	bounds;
	do {
		bounds += cgal2ben(circ->source()->point());
		if(he_has_any_roads(circ) && get_he_street(circ) && (circ->data().HasGroundRoads() || circ->twin()->data().HasGroundRoads()))
			has_road = 1;
		else
			has_non_road = 1;
	} while(++circ != stop);
	if(has_road && !has_non_road)	has_road = 2;

#if OPENGL_MAP && DEV
	if(has_road != face->data().GetParam(af_RoadEdge,-1))
	{
		printf("Failed.  Thought we had %f, actually had %d.  Zoning %s.\n", 
				face->data().GetParam(af_RoadEdge,-1),
				has_road,
				FetchTokenString(zoning));
		gFaceSelection.insert(face);
	}
#endif
	CreateTranslatorForBounds(bounds, translator);
	if(translator.mDstMax.x() < 1.0 || translator.mDstMax.y() < 1.0)
		return false;

	int num_he = count_circulator(face->outer_ccb());
	for(Pmwx::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h)
		num_he += count_circulator(*h);
		
	vector<block_pt>	outer_ccb_pts;
	
	block_pts_from_ccb(face->outer_ccb(), translator, outer_ccb_pts,BLOCK_ERR_MTR,false);
	
	// We can't have a continuous edge around the first point.  If the first point is a flat edge,
	// we end up inducing a discontinuity.  This can make a micro-break in the autogen.
	rotate_to_corner(outer_ccb_pts);


	vector<BLOCK_face_data>				parts;
	vector<Block_2::X_monotone_curve_2> curves;

	// What IS the layout of our block table?  Basically each possible burn-in feature gets an index number into the parts vector,
	// which provides the meta data for that part, with the following rules:
	// 1. Overlapping parts MUST have unique IDs, because self-intersections are NOT handled.  The code is meant to splat a large number
	//	  of atomic convex shapes.
	// 2. Higher ID parts over-write smaller ones to own area.
	// 3. ONE part, the "OOB" part, is inverted, e.g. OUTSIDE the contour starts as included. We can uset his to specify negatively that all space
	//	  OUTSIDE our polygon is not for use.  This keeps things from "leaking" all over the place.
	// 
	// We then allocate two entries per half-edge plus four:
	// m	block features, var length
	// n	road-spawned autogen - that is, the autogen attached directly to road segments.
	// 1	"steep" - area zoned as too steep for autogen, but possibly useful for vegetation.
	// n	actual road surface area.  This area is strictly out of bounds to ALL autogen to avoid overlaps.
	// 1	(unused - if we had a high priority forest it would go here, but forest goes in the steep bucket before, to avoid overwriting roads.)
	// 1	"flat" - area is zoned as flat but inhospitable to ALL autogen, e.g. a sand dune.
	// 1	OOB - the out of bonuds area that will be reversed, to remove negative space.

	int block_feature_count = 0;
	int oob_idx = 0;
	if(info && info->fill_area)
	{
		// For now we use our first X road halfedges ...
		DebugAssert(!info->fill_edge);
		FillRule_t * r = GetFillRuleForBlock(face);
		if(r && (r->agb_id != NO_VALUE))
			block_feature_count = init_subdivisions(face, r, translator, outer_ccb_pts, parts, curves);
		if(block_feature_count)
			oob_idx = parts.size() - 1;
	}
	
	if(block_feature_count == 0)
	{	
		num_he *= 2;	// We need up to two parts (edge, corner) for each road.
	
		int num_extras = 1;
	
		if(info->fill_points)
		num_extras += face->data().mPointFeatures.size();
	
		DebugAssert(parts.size() == block_feature_count);
		parts.resize(block_feature_count + num_extras + 2 * num_he + cat_DIM, BLOCK_face_data(usage_Empty, NO_VALUE));
		int cat_base = 2 * num_he + block_feature_count + num_extras;

		parts[block_feature_count + num_he] = BLOCK_face_data(usage_Steep, NO_VALUE);
		
		parts[cat_base + cat_flat] = BLOCK_face_data(usage_OOB  , NO_VALUE);
		parts[cat_base + cat_oob] = BLOCK_face_data(usage_OOB, NO_VALUE);

		
		int cat_table[cat_DIM];
		cat_table[cat_urban] = -1;
		cat_table[cat_oob] = cat_base + cat_oob;
		cat_table[cat_forest] = num_he + block_feature_count;
		cat_table[cat_flat] = cat_base + cat_flat;
		
		if(info && (info->need_lu || info->max_slope))
			init_mesh(mesh, translator, curves, cat_table,info->max_slope,info->need_lu);

		if(info->fill_points)		
		if(!face->data().mPointFeatures.empty())
			init_point_features(face->data().mPointFeatures, curves, parts, outer_ccb_pts, translator, num_he + 1, zoning);

		int base_offset = block_feature_count;
		{
			init_road_ccb(zoning, variant, face->outer_ccb(), translator, parts, curves, outer_ccb_pts, base_offset, num_he+num_extras, cat_base + cat_oob, info && info->fill_edge, ag_ok_approx_dem);
			for(Pmwx::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h)
			{
				vector<block_pt>	hole_pts;
				block_pts_from_ccb(*h,translator, hole_pts,BLOCK_ERR_MTR,true);		
				init_road_ccb(zoning, variant, *h, translator, parts, curves, hole_pts, base_offset, num_he+num_extras, cat_base + cat_oob, info && info->fill_edge, ag_ok_approx_dem);
			}
		}
		oob_idx = cat_base + cat_oob;
	//	printf("Num HE: %d.  Added: %d\n", num_he, base_offset);
	}

#if DEBUG_BLOCK_CREATE_LINES
	for(vector<Block_2::X_monotone_curve_2>::iterator c = curves.begin(); c != curves.end(); ++c)
	{
		Segment2	s(cgal2ben(c->source()),cgal2ben(c->target()));
		Vector2	v(s.p1,s.p2);
		v = v.perpendicular_ccw();
		v.normalize();
		v *= 0.25;
		s.p1+=v;
		s.p2+=v;
		s.p1 = translator.Reverse(s.p1);
		s.p2 = translator.Reverse(s.p2);
		for(EdgeKey_iterator i = c->data().begin(); i != c->data().end(); ++i)
		if(*i == oob_idx)
		{
			debug_mesh_line(
						s.p1,s.p2,					
						0.5,0,0,
						1,0,0);
		}
//		else if(*i == num_he+block_feature_count)
//		{
//			debug_mesh_line(
//						s.p1,s.p2,					
//						0,0.5,0,
//						0,1,0);
//		}
//		else if(*i == cat_base + cat_flat)
//		{
//			debug_mesh_line(
//						s.p1,s.p2,					
//						0,0,0.5,
//						0,0,1);
//		}
		else
		{
			int feat = parts[*i].feature;
			if(strstr(FetchTokenString(feat),".agb"))		debug_mesh_line(s.p1,s.p2,	0.4,0.0,0.0,	0.8,0.0,0.0);
			else if(strstr(FetchTokenString(feat),".ags"))	debug_mesh_line(s.p1,s.p2,	0.0,0.0,0.4,	0.0,0.0,0.8);
			else if(strstr(FetchTokenString(feat),".fac"))	debug_mesh_line(s.p1,s.p2,	0.4,0.4,0.0,	0.8,0.8,0.0);
			else											debug_mesh_line(s.p1,s.p2,	0.0,0.4,0.0,	0.0,0.8,0.0);
		}
	}
#endif
//	for(int n = 0; n < parts.size(); ++n)
//		printf("%d: %d %s\n", n, parts[n].usage, FetchTokenString(parts[n].feature));
	create_block(out_block,parts, curves, oob_idx);	// First "parts" block is outside of CCB, marked as "out of bounds", so trapped areas are not marked empty.
	num_line_integ += curves.size();
//	debug_show_block(out_block,translator);
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

bool is_road_adjacent(Block_2::Face_handle f)
{
	Block_2::Ccb_halfedge_circulator circ, stop;
	circ = stop = f->outer_ccb();
	do
	{
		if(against_road(circ))
			return true;
	} while(++circ != stop);
	for(Block_2::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
	{
		circ = stop = *h;
		do
		{
			if(against_road(circ))
				return true;
		} while(++circ != stop);
	}
	return false;
}

bool	apply_fill_rules(
					int						zoning,
					Pmwx::Face_handle		orig_face,
					Block_2&				block,
					CoordTranslator2&		translator)
{
	bool did_promote = false;
	if(gZoningInfo[zoning].fill_area)
	{
		FillRule_t * r = GetFillRuleForBlock(orig_face);
		if(r != NULL && (r->agb_id == NO_VALUE))
		for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
		if(!f->is_unbounded())
		if(f->data().usage == usage_Empty)
		if(is_road_adjacent(f))
		{
			
			f->data().usage = usage_Polygonal_Feature;
			if(r->fac_id != NO_VALUE)
				f->data().feature = r->fac_id;
			else
				f->data().feature = r->ags_id;
			DebugAssert(f->data().feature != NO_VALUE);
		}
	}

	// If an empty face is surrounded by a monolithic usage, we will connect them all.
	// This "fills in" an AGS that has wasted space.
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

Block_2::Halfedge_handle best_side_for_facade(Block_2::Ccb_halfedge_circulator circ)
{
	Block_2::Halfedge_handle best = Block_2::Halfedge_handle();
	Block_2::Ccb_halfedge_circulator stop(circ);
	do {
		if(against_road(circ))
		{
			if(best == Block_2::Halfedge_handle() || 
				CGAL::squared_distance(circ->source()->point(),circ->target()->point()) > 
				CGAL::squared_distance(best->source()->point(),best->target()->point()))
				best = circ;			
		}
	} while(++circ != stop);
	
	return best;	
}

Block_2::Halfedge_handle best_side_for_agb(Block_2::Ccb_halfedge_circulator circ, const Vector2& major, unsigned short& param)
{
	Block_2::Halfedge_handle best = Block_2::Halfedge_handle();
	Block_2::Ccb_halfedge_circulator stop(circ);
	
	double best_dot = 0.0f;
	Vector2	best_vec;
	do {
		if(against_road(circ))
		{
			Vector2	this_side(cgal2ben(circ->source()->point()),cgal2ben(circ->target()->point()));
			this_side.normalize();
			double my_dot = fabs(this_side.dot(major));
			if(my_dot > best_dot)
			{
				best = circ;
				best_dot = my_dot;
				best_vec = this_side;
			}
		}
	} while(++circ != stop);
	
	if(best == Block_2::Halfedge_handle())
		return best;
	//	+-----1-------+	
	//	2             0
	//	+--- ROOT --->+	
	int has_road_on[3] = { 0 };
	
	do {
		if(circ != best)
		if(against_road(circ))
		{
			Vector2	this_side(cgal2ben(circ->source()->point()),cgal2ben(circ->target()->point()));
			this_side.normalize();
			double my_dot = fabs(this_side.dot(best_vec));
			if(my_dot > 0.7)
				has_road_on[1] = 1;
			else
			{
				if(best_vec.left_turn(this_side))
					has_road_on[0] = 1;
				else
					has_road_on[2] = 1;
			}
		}
	} while(++circ != stop);
	
	param = 1 * has_road_on[0] +
			2 * has_road_on[1] +
			4 * has_road_on[2];
	return best;
}



void	PolygonFromBlockCCB(Block_2::Halfedge_const_handle circ, Polygon2& out_poly, CoordTranslator2 * translator, double err)
{
	out_poly.clear();
	Block_2::Halfedge_const_handle stop = circ;

	do {
			out_poly.push_back(cgal2ben(circ->source()->point()));
			circ = circ->next();
	} while (stop != circ);
	if(err)
		ring_simplify_polygon(out_poly,err*err);
	if(translator)
	for(int n = 0; n < out_poly.size(); ++n)
		out_poly[n] = translator->Reverse(out_poly[n]);
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

void	PolygonFromBlock(Block_2::Face_const_handle in_face, Block_2::Halfedge_const_handle first_side, vector<Polygon2>& out_ps, CoordTranslator2 * translator, double err, bool will_split)
{
	DebugAssert(!in_face->is_unbounded());

	out_ps.push_back(Polygon2());
	PolygonFromBlockCCB(first_side, out_ps.back(), translator, err);
	DebugAssert(out_ps.back().size() > 2);
	
	for(Block_2::Hole_const_iterator h = in_face->holes_begin(); h != in_face->holes_end(); ++h)
	{
		out_ps.push_back(Polygon2());
		PolygonFromBlockCCB(*h, out_ps.back(), translator, err);
		DebugAssert(out_ps.back().size() > 2);
	}
	if(!will_split)
	if(out_ps.size() > 255)
	{
		printf("Face %s had %zd holes.\n",FetchTokenString(in_face->data().feature),in_face->number_of_holes());
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
	if(bounds.size() > 255)
	{
		printf("Face had %zd holes.\n",bounds.size()-1);
		throw "too many holes.";
	}

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

void	fail_extraction(Pmwx::Face_handle dest_face, vector<Polygon2>& shape, CoordTranslator2 * trans, const char * fmt, ...)
{
	va_list	arg;
	va_start(arg, fmt);
	#if DEV && OPENGL_MAP
		for(int s = 0; s < shape.size(); ++s)
		for(int ss = 0; ss < shape[s].size(); ++ss)
		{
			Segment2	si(shape[s].side(ss));
			if(trans) {
				si.p1 = trans->Reverse(si.p1);
				si.p2 = trans->Reverse(si.p2); }
			debug_mesh_line(si.p1,si.p2,1,0,0,1,0,0);
		}
		gFaceSelection.insert(dest_face);
		vprintf(fmt,arg);
	#else
		AssertPrintfv(fmt,arg);
	#endif
}

void PushSideIn(Polygon2& poly, int side, double dist)
{
	Segment2	s(poly.side(side));
	Vector2		v(s.p1,s.p2);
	v.normalize();
	v = v.perpendicular_ccw();
	v *= dist;
	poly[side] += v;
	poly[(side+1) % poly.size()] += v;
}

void	extract_features(
					Block_2&				block,
					Pmwx::Face_handle		dest_face,
					CoordTranslator2&		translator,
					const DEMGeo&			forest_dem,
					ForestIndex&			forest_index)					
{
	double	block_height = dest_face->data().GetParam(af_HeightObjs,8.0);

	bool did_split = false;
#if DEV && OPENGL_MAP
	try
#endif
	{
		for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
		if(!f->is_unbounded())
		{
			if(f->data().usage == usage_Polygonal_Feature)
		//	if(!f->data().pre_placed)
			{
				GISPolyObjPlacement_t o;
				
				o.mRepType = f->data().feature;
				Assert(o.mRepType != NO_VALUE);
				o.mDerived = true;
				//
				// AGS CASE
				//
				// AGS: we need a special extractor just to figure out what's going on.  This builds the (byzantine)
				// line mess that is an AGS.
				if(strstr(FetchTokenString(o.mRepType),".ags"))
				{
					o.mParam = StringFromBlock(f,o.mShape,translator);
					encode_ag_height(o.mParam,block_height);					
					DebugAssert(o.mShape.size() <= 255);
					dest_face->data().mPolyObjs.push_back(o);				
				}
				else if(strstr(FetchTokenString(o.mRepType),".fac"))
				{		
					Block_2::Halfedge_handle start = best_side_for_facade(f->outer_ccb());
					bool fail_start = start == Block_2::Halfedge_handle();
					if(fail_start)
						start = f->outer_ccb();
						o.mParam = block_height;
					PolygonFromBlock(f,start, o.mShape, &translator, 0.0,false);
//					if(fail_start)
//						fail_extraction(dest_face,o.mShape,NULL,"NO ANCHOR SIDE ON FAC.");										
					DebugAssert(o.mShape.size() <= 255);
					dest_face->data().mPolyObjs.push_back(o);
				}
				else
				{
					DebugAssert(strstr(FetchTokenString(o.mRepType),".agb"));
					Block_2::Halfedge_handle start = best_side_for_agb(f->outer_ccb(), f->data().major_axis, o.mParam);
					bool fail_start = start == Block_2::Halfedge_handle();
					if(fail_start)
						start = f->outer_ccb();
					// Simplify mandatory to fix colinear manifold pt split.
					PolygonFromBlock(f,start, o.mShape, NULL, 3.0,false);
					DebugAssert(o.mShape.size() <= 255);
					if(fail_start)
					{
						fail_extraction(dest_face,o.mShape,&translator,"NO ANCHOR SIDE ON AGB.");
					}


						if(o.mShape[0].size() != 4)
							fail_extraction(dest_face,o.mShape,&translator,"SHORT AXIS FAIL: %f\n", dest_face->data().GetParam(af_ShortAxisLength,0.0));
			
														PushSideIn(o.mShape[0],0,-3.0);
					if(o.mParam & 1)					PushSideIn(o.mShape[0],1,-3.0);
					if(o.mParam & 2)					PushSideIn(o.mShape[0],2,-3.0);
					if(o.mParam & 4)					PushSideIn(o.mShape[0],3,-3.0);
					
					encode_ag_height(o.mParam,block_height);
					
					for(int n = 0; n < o.mShape[0].size(); ++n)
						o.mShape[0][n] = translator.Reverse(o.mShape[0][n]);

					dest_face->data().mPolyObjs.push_back(o);
				}
			}
			if(f->data().usage == usage_Forest)
			{
				vector<Polygon2>	forest(1);
				double area;
				forest.reserve(1+f->number_of_holes());
				Polygon2	outer_m;
				extract_ccb(f->outer_ccb(),forest.back(),outer_m, translator);
				area = outer_m.area();

				for(Block_2::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
				{
					Polygon2 hole_m;
					forest.push_back(Polygon2());
					extract_ccb(*h, forest.back(), hole_m, translator); 
					area -= hole_m.area();
				}
				if(f->number_of_holes() < 255 && area < FOREST_SUBDIVIDE_AREA)
				{
					push_one_forest(forest, forest_dem, dest_face);					
				} 
				else
				{
					did_split = true;
					++num_forest_split;
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
									push_curve(curves,
											map2block(circ->source()->point()),
											map2block(circ->target()->point()),
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
							
									push_curve(curves,
											map2block(circ->source()->point()),
											map2block(circ->target()->point()),
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
							push_curve(curves, ben2cgal<BPoint_2>(seg.p1),ben2cgal<BPoint_2>(seg.p2),oob_idx, -1, NO_VALUE, -1);
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

					{
retry:					
						bool all_ok = true;
						for(Block_2::Face_iterator df = divided_forest.faces_begin(); df != divided_forest.faces_end(); ++df)
						if(!df->is_unbounded())
						if(df->data().usage == usage_Forest)
						if(df->number_of_holes() >= 255)
						{
							Block_2::Hole_iterator h = df->holes_begin();
							Block_2::Point_2 p1 = (*h)->source()->point();
							++h;
							Block_2::Point_2 p2 = (*h)->source()->point();							
							Block_2::X_monotone_curve_2 c (BSegment_2(p1, p2), 0);							
							CGAL::insert(divided_forest, c);
							goto retry;
						}
					}

					for(Block_2::Face_iterator df = divided_forest.faces_begin(); df != divided_forest.faces_end(); ++df)
					if(!df->is_unbounded())
					if(df->data().usage == usage_Forest)
					{
						vector<Polygon2>	a_forest;
						PolygonFromBlock(df,df->outer_ccb(),a_forest, NULL,0.0,false);
						push_one_forest(a_forest, forest_dem, dest_face);					
					}
				}
			}
		}			
	} 
#if DEV && OPENGL_MAP
	catch (...) 
	{
		gFaceSelection.insert(dest_face);
	}
#endif	
	if(did_split)
		num_blocks_with_split++;
}

bool process_block(Pmwx::Face_handle f, CDT& mesh, const DEMGeo& ag_ok_approx_dem, const DEMGeo& forest_dem,ForestIndex&	forest_index)
{
	++num_block_processed;
	bool ret = false;
	int z = f->data().GetZoning();
	if(z == NO_VALUE || z == terrain_Natural)
		return false;
	DebugAssert(gZoningInfo.count(z) != 0);
	if (gZoningInfo.count(z) == 0)
		return false;
		
	CoordTranslator2	trans;
	Block_2 block;
	
	if(init_block(mesh, f, block, trans, ag_ok_approx_dem))
	{
//		simplify_block(block, 0.75);
//		clean_block(block);

		if (apply_fill_rules(z, f, block, trans))
			ret = true;
		extract_features(block, f, trans, forest_dem, forest_index);
	}
	return ret;
}
