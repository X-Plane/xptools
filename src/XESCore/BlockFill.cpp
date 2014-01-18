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
#include "XUtils.h"

#define	IGNORE_SHORT_AXIS	1

// If the particular cut lines on the top half and bottom half of a block are too close, they can be below the 
// floating point threshold and the block generation ends up with zero length segments.
// So: find any cuts closer than 0.1 meters nad merge them.  
// IF for some insane reason we intentiaanlly put a 0.1 meter feature into the block, we're going to blow up, 
// but if we have a 0.1 meter feature, we've gone really far off the reservation - it would imply the block 
// selected for zoning was grossly inappropriate AND the facade was made of tiny fragments.
#define SMALL_CUT 0.1

int num_block_processed = 0;
int num_blocks_with_split = 0;
int num_forest_split = 0;
int num_line_integ = 0;

#include <stdarg.h>

typedef UTL_interval<double>	time_region;

#include "GISTool_Globals.h"

#define DEBUG_BLOCK_CREATE_LINES 0

#include <CGAL/Arr_overlay_2.h>

#define TOO_SMALL_TO_CARE 300

// This controls how much we subdivide small forest stands.
#define FOREST_SUBDIVIDE_AREA		(1000000.0)

// Don't mess with this if you want to MAKE a DSF - larger than 255 and DSF encoder blows up.
#define MAX_FOREST_RINGS			255

#define BLOCK_ERR_MTR 0.5

#define map2block(X) (X)

#define TRACE_SUBDIVIDE if(0) printf

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

//------------------------------------------------------------------------------------------------------------------------------------------------
// CANDY BAR BLOCK CONSOLIDATOR UTILITY
//------------------------------------------------------------------------------------------------------------------------------------------------
//
//	Block generation is dominated by the integration of curves into the block map - that is, more lines = more slow.  So anything we can do to
//	reduce the number of lines we throw into the soup is a win.
//
//	The candy_bar class is a utility that is sort of like a hershey bar with squares of chocolate.  Client code specifies a grid and which grid
//	squares belong to which blocks; the candy bar then generates only the minimum cut lines to ensure block separation.  There are two 
//	optimizations:
//
//		Interior cuts within a block that would not help us are totally elimaanted.
//		Colinear sequential lines that separate the same faces are merged (removing vertices).
//
//
//	Since the facade subdivisions run at different 'schedules' for the top and bottom of the block, this kind of consolidation is really important!
//

class candy_bar {
public:

				candy_bar(int b_divs, double b_cuts[], double a_min, double a_max);

		int		insert_a_divide(double a);		// returns a-idx of block starting at a		
		void	insert_block(const BLOCK_face_data& d, int b_start, int b_end, double a_start, double a_end);	

		void	snap_round();

		int		build_curves(vector<BLOCK_face_data>& parts, const Vector2& va, const Vector2& vb, vector<Block_2::X_monotone_curve_2>& curves);	// returns block count

		void	dump();

private:

		pair<int,int>	get_lr(int a, int b);
		pair<int,int>	get_bt(int a, int b);
		
		void			emit_pair(vector<Block_2::X_monotone_curve_2>& curves, const Vector2& va, const Vector2& vb, int a1, int b1, int a2, int b2, int c1, int c2);

	vector<int>				m_part_idx;
	vector<BLOCK_face_data>	m_parts;
	int						m_a_divs;
	int						m_b_divs;
	vector<double>			m_a_cuts;
	vector<double>			m_b_cuts;
	
};	

candy_bar::candy_bar(int b_divs, double b_cuts[], double a_min, double a_max)
{
	m_a_divs = 1;
	m_b_divs = b_divs;
	m_part_idx.resize(m_a_divs * m_b_divs,-1);
	
	m_a_cuts.push_back(a_min);
	m_a_cuts.push_back(a_max);
	for(int i = 0; i <= b_divs; ++i)
		m_b_cuts.push_back(b_cuts[i]);
}

int	candy_bar::insert_a_divide(double a)
{
	DebugAssert(a >= m_a_cuts.front());		// Make sure the block really is being subdivided and 
	DebugAssert(a <= m_a_cuts.back());		// not GROWN - we do not support the GROW case.
	vector<double>::iterator ip = lower_bound(m_a_cuts.begin(),m_a_cuts.end(),a);
	if(ip == m_a_cuts.end() || *ip != a)
	{
		DebugAssert(ip != m_a_cuts.end());		// Off the end would mean growing the end
		DebugAssert(ip != m_a_cuts.begin());	// Having begin would mean growing the beginning if *ip != a
		
		// IP is ptr to the start of the NEXT block - find our actual index, and dupe the block.
		int split_block_idx = distance(m_a_cuts.begin(),ip)-1;
		
		// increment rows first - so that PAST rows get calced right.
		++m_a_divs;
		
		for(int b = 0; b < m_b_divs; ++b)
		{
			// Find our effective address and dupe it
			int bid = b * m_a_divs + split_block_idx;
			m_part_idx.insert(m_part_idx.begin()+bid,m_part_idx[bid]);
		}
		
		m_a_cuts.insert(ip,a);
		
		return split_block_idx+1;
	}
	else
	{
		DebugAssert(ip != m_a_cuts.end());
		// We are right on the nose of a cut - thus its index IS where the cut is.
		return distance(m_a_cuts.begin(),ip);
	}
}

void	candy_bar::insert_block(const BLOCK_face_data& d, int b_start, int b_end, double a_start, double a_end)
{
	DebugAssert(a_end > a_start);
	int sidx = insert_a_divide(a_start);
	int eidx = insert_a_divide(a_end);
	
	for(int b = b_start; b < b_end; ++b)
	for(int a = sidx; a < eidx; ++a)
		m_part_idx[a + b * m_a_divs] = m_parts.size();
	
	m_parts.push_back(d);
	
}

void	candy_bar::snap_round()
{
	int i;
	for(i = 1; i < m_a_cuts.size(); ++i)
	{
		if(m_a_cuts[i] - m_a_cuts[i-1] < SMALL_CUT)
			m_a_cuts[i] = m_a_cuts[i-1];
	}

	for(i = 1; i < m_b_cuts.size(); ++i)
	{
		if(m_b_cuts[i] - m_b_cuts[i-1] < SMALL_CUT)
			m_b_cuts[i] = m_b_cuts[i-1];
	}
}

int	candy_bar::build_curves(vector<BLOCK_face_data>& parts, const Vector2& va, const Vector2& vb, vector<Block_2::X_monotone_curve_2>& curves)
{
	int base = parts.size();
	parts.insert(parts.end(),m_parts.begin(),m_parts.end());
	
	int a, b, aa, bb;
	
	// HORIZONTAL LINES
	for(b = 0; b <= m_b_divs; ++b)
	{
		a = 0;
		while(a < m_a_divs)
		{
			pair<int,int> bp(get_bt(a,b));
			
			aa = a + 1;
			while(aa < m_a_divs && get_bt(aa,b) == bp)
				++aa;
				
			emit_pair(curves,va,vb,a,b,aa,b,bp.first,bp.second);
			
			a = aa;
		}
	}
	// VERTICAL LINES
	for(a = 0; a <= m_a_divs; ++a)
	{
		b = 0;
		while(b < m_b_divs)
		{
			pair<int,int> bp(get_lr(a,b));
			
			bb = b + 1;
			while(bb < m_b_divs && get_lr(a,bb) == bp)
				++bb;
				
			emit_pair(curves,va,vb,a,b,a,bb,bp.first,bp.second);
			
			b = bb;
		}
	}
	
	return m_parts.size();
	
}

void	candy_bar::dump()
{
	for(int b = m_b_divs - 1; b >= 0; --b)
	{
		for(int a = 0; a < m_a_divs; ++a)
		{
			printf("%d\t",m_part_idx[a + m_a_divs * b]);
		}
		printf("\n");
	}
}


void			candy_bar::emit_pair(vector<Block_2::X_monotone_curve_2>& curves, const Vector2& va, const Vector2& vb, int a1, int b1, int a2, int b2, int c1, int c2)
{	
	DebugAssert(a1 != a2 || b1 != b2);
	DebugAssert(a1 == a2 || b1 == b2);
	
	if(a1 == a2)
	{
		if(m_b_cuts[b1] == m_b_cuts[b2])	
			return;
	}

	if(b1 == b2)
	{
		if(m_a_cuts[a1] == m_a_cuts[a2])	
			return;
	}
	
	if(c1 != c2)
//	{
//		if (c1 != -1)
//			push_block_curve(curves,Point2() + va * m_a_cuts[a1] + vb * m_b_cuts[b1],Point2() + va * m_a_cuts[a2] + vb * m_b_cuts[b2],c1);
//	}
//	else
	{
		//printf("    %dx%d -> %dx%d  (%d/%d)\n", a1,b1,a2,b2,c1,c2);
	
		// Ensure that cuts were not stacked backward.
		DebugAssert(c1 != -1);
		if(c2 != -1)
		{
			push_block_curve(curves,Point2() + va * (m_a_cuts[a1]) + vb * m_b_cuts[b1],Point2() + va * (m_a_cuts[a2]) + vb * m_b_cuts[b2],c1,c2);

//			push_block_curve(curves,Point2() + va * m_a_cuts[a1] + vb * m_b_cuts[b1],Point2() + va * m_a_cuts[a2] + vb * m_b_cuts[b2],c1);
//			push_block_curve(curves,Point2() + va * m_a_cuts[a2] + vb * m_b_cuts[b2],Point2() + va * m_a_cuts[a1] + vb * m_b_cuts[b1],c2);
		}
		else
			push_block_curve(curves,Point2() + va * (m_a_cuts[a1]) + vb * m_b_cuts[b1],Point2() + va * (m_a_cuts[a2]) + vb * m_b_cuts[b2],c1);
	}
}

pair<int,int>	candy_bar::get_lr(int a, int b)
{
	DebugAssert(a >= 0);
	DebugAssert(a <= m_a_divs);
	DebugAssert(b >= 0);
	DebugAssert(b < m_b_divs);
	int l, r;
	if(a == 0)
		l = -1;
	else
		l = m_part_idx[a + b * m_a_divs - 1];
	
	if(a == m_a_divs)
		r = -1;
	else
		r = m_part_idx[a + b * m_a_divs    ];
		
	if(l != r && l == -1)
		swap(l,r);
	return make_pair(l,r);			
}

pair<int,int>	candy_bar::get_bt(int a, int b)
{
	DebugAssert(b >= 0);
	DebugAssert(b <= m_b_divs);
	DebugAssert(a >= 0);
	DebugAssert(a < m_a_divs);
	int bot, top;
	if(b == 0)
		bot = -1;
	else
		bot = m_part_idx[a + (b-1) * m_a_divs];
	
	if(b == m_b_divs)
		top = -1;
	else
		top = m_part_idx[a + b * m_a_divs    ];
		
	if(bot != top && bot == -1)
		swap(bot,top);
	return make_pair(bot,top);
}

	

//------------------------------------------------------------------------------------------------------------------------------------------------


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

static EdgeRule_t * edge_for_road(const pair<int,bool>& road_type, int zoning, int variant, float height)
{
	for(EdgeRuleTable::iterator e = gEdgeRules.begin(); e != gEdgeRules.end(); ++e)
	if(e->zoning == zoning && e->road_type == road_type.first)
	if(e->variant == -1 || e->variant == variant)
	if(e->height_min == e->height_max || (e->height_min <= height && height <= e->height_max))
		return &*e;
	return NULL;
}

static bool against_road(Block_2::Halfedge_const_handle h)
{
	return h->twin()->face()->data().usage == usage_Road &&
		   gNetReps[h->twin()->face()->data().feature].use_mode == use_Street;
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

// Given a range along the A axis and the path of one of the top or bottom edges of our block,
// This routine returns the length of how much the block drifts up or down along the interval.
// It is a measure of how much slop we'd need to put an AGB along 'range'.
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
		
		TRACE_SUBDIVIDE("B interval, segment from %lf,%lf to %lf,%lf, x range %lf, %lf\n",s->p1.x(),s->p1.y(),s->p2.x(),s->p2.y(), xmin, xmax);
		
		double y1 = s->y_at_x(xmin);
		double y2 = s->y_at_x(xmax);
		minv = min(min(y1,y2),minv);
		maxv = max(max(y1,y2),maxv);
	}
	
	return max(maxv-minv,0.0);
}

/*
double size_err(const FillRule_t * info, double width, const vector<double>& div_rats)
{
	double t = 0;
	for(int n = 0; n < div_rats.size(); ++n)
	{
		double w_mtr = div_rats[n] * width;
		double nearest = info->fac_min_width + round((w_mtr - info->fac_min_width) / info->fac_step) * info->fac_step;
		nearest = doblim(nearest,info->fac_min_width, info->fac_max_width);
		t += (w_mtr - nearest) * (w_mtr - nearest);		
	}
	return sqrt(t) / (double) div_rats.size();
}

static void make_one_subdiv_spelling(const FillRule_t * info, double width, vector<double>& div_rats)
{
	int approx_wanted = intmax2(1,width / ((info->fac_min_width + info->fac_max_width) * 0.5));

	int range = 1 + (info->fac_max_width - info->fac_min_width) / info->fac_step;
	
	double t = 0.0;
	for(int i = 0; i < approx_wanted; ++i)
	{
		div_rats.push_back(info->fac_max_width + info->fac_step * (rand() % range));
		t += div_rats.back();
	}
	DebugAssert(t > 0.0);
	t = 1.0 / t;
	for(int i = 0; i < div_rats.size(); ++i)
	{
		div_rats[i] *= t;
	}
}

static void make_fac_subdiv_spelling(const FillRule_t * info, double width, vector<double>& div_rats)
{
	typedef multimap<float,vector<float> >::const_iterator spelling_iter;
	spelling_iter rs = info->spellings.lower_bound(width-info->fac_step);
	spelling_iter re = info->spellings.upper_bound(width+info->fac_step);

	int num_choices = distance(rs,re);
	if(num_choices == 0)
	{	
		#if DEV
		if(rs == info->spellings.end())
			printf("WARNING: fill rule does not have enough length for %f\n",width);
		#endif	
		div_rats.push_back(1.0);
		return;
	}

	int our_pick = rand() % num_choices;
	
	advance(rs,our_pick);
	for(int n = 0; n < rs->second.size(); ++n)
		div_rats.push_back(rs->second[n] / rs->first);

//	printf("Best for %.1lf:", width);
//	for(int n = 0; n < div_rats.size(); ++n)
//		printf(" %.1lf", div_rats[n] * width);
//	printf("\n");
//	printf(" (Err = %lf\n", e_best);
}
*/
/*
static void dump_spelling(map<float,int>& histo)
{
	for(map<float,int>::iterator h = histo.begin(); h != histo.end(); ++h)
		printf("%f: x%d\n", h->first,h->second);
}

static void make_fac_subdiv_spelling(const FillRule_t * info, double width, vector<double>& div_rats)
{
	float			total = 0;
	map<float,int>	included;
	while(total < width)
	for(float w = info->fac_min_width; w <= info->fac_max_width; w += info->fac_step)
	{
		included[w] += 1;
		total += w;
	}
//	printf("Total start: %f for %f\n", total, width);
//	dump_spelling(included);
	while(total > width && !included.empty())
	{
		float over = total - width;
//		printf("over by %f\n",over);
		map<float,int>::iterator i = included.lower_bound(over);
		if(i != included.end() && i->first != over)
			++i;
		if(i == included.end())
			--i;
		total -= i->first;
//		printf("Will pull %f\n", i->first);
		if(i->second > 1)
			i->second -= 1;
		else
			included.erase(i);
//		dump_spelling(included);
	}
	
	for(map<float,int>::iterator h = included.begin(); h != included.end(); ++h)
	{
		while(h->second--)
			div_rats.push_back(h->first / total);
	}
//	printf("Best for %.1lf:", width);
//	for(int n = 0; n < div_rats.size(); ++n)
//		printf(" %.1lf", div_rats[n] * width);
}
*/


/*
	This routine creates the subdivision lines for AGBs and subdivided facades; it is applied
	when we have an AGB fill rule on the block.  The routine analyzes the block shape and decides
	where to cut up the block, etc.
 */
 
inline bool segment_crosses_y_within(const Segment2& s, double y, double x_min, double x_max)
{
	if((s.p1.y() > y && s.p2.y() < y) || (s.p1.y() < y && s.p2.y() > y))
	{
		double x = s.x_at_y(y);
		return x > x_min && x < x_max;
	}
	else
		return false;
}

	enum {
		reg_TERM = 0,
		reg_none,
		reg_junk,		// parking lots and other random junk.
		reg_slop,		// When the 'front' side is actually to the side, we just glom it on to our friend...as a side wall there aren't a lot of rules about its metrics.
		reg_fac,
		reg_agb,
	};

	#if DEV
		const char * reg_debug_str[] = { "term", "none", "junk", "slop", "fac", "agb" };
	#endif
	
	#define SLOPE_TO_SIDE 1.0		// walls sloping more than 45 degrees are deemed side-facing - this is the tangent of the angle from front.
	
	struct reg_info_t {
		int		bot_type;
		int		top_type;
		int		top_side;
		int		bot_side;
		double	top_slope;	// b over a - CANNOT be NaN because time range must have a-time > 0
		double	bot_slope;	// b over a - CANNOT be NaN because time range must have a-time > 0
		int		block_dir;	// Which way we have to go to widen.  If the block widens to the right, 1, to the left, 0, or -1 otherwise.
		
		bool	is_split;
		double	a_time;
	};
		
	
inline int find_first_bot_type(const vector<reg_info_t>& r, int t)
{
	for(int i = 0; i < r.size(); ++i)
	if(r[i].bot_type == t)
		return i;
	return -1;
}

inline int find_last_bot_type(const vector<reg_info_t>& r, int t)
{
	if(r.empty()) return -1;
	for(int i = r.size()-1; i >= 0; --i)
	if(r[i].bot_type == t)
		return i;
	return -1;
}

template <typename P>	void set_first(P& p, typename P::first_type v) {	p.first = v; }
template <typename P>	void set_second(P& p, typename P::second_type v){ p.second = v;}

typedef void (* pair_set_f)(pair<double, double>& p, double v);

const double UNDEF = -8192.0;

template <pair_set_f func>
void add_one_split(double x, double y, map<double,pair<double,double> >& times)
{	
	pair<map<double,pair<double,double> >::iterator,bool> it = times.insert(make_pair(x,make_pair(UNDEF, UNDEF)));
	func(it.first->second, y);
}


template <pair_set_f func>
void split_segment(const Segment2& s, double a_start, double a_stop, map<double,pair<double, double> >& times, const double * split_lines, int split_count)
{
	if(s.p1.x() >= a_start && s.p1.x() <= a_stop)
		add_one_split<func>(s.p1.x(),s.p1.y(),times);

	if(s.p2.x() >= a_start && s.p2.x() <= a_stop)
		add_one_split<func>(s.p2.x(),s.p2.y(),times);
		
	if((s.p1.x() < a_start && s.p2.x() > a_start) ||
	   (s.p2.x() < a_start && s.p1.x() > a_start))
	{	
		add_one_split<func>(a_start,s.y_at_x(a_start), times);		
	}

	if((s.p1.x() < a_stop && s.p2.x() > a_stop) ||
	   (s.p2.x() < a_stop && s.p1.x() > a_stop))
	{	
		add_one_split<func>(a_stop,s.y_at_x(a_stop), times);		
	}
	
	while(split_count-- > 0)
	{
		double ys = *split_lines++;
		if((s.p1.y() < ys && s.p2.y() > ys) ||
		   (s.p2.y() < ys && s.p1.y() > ys))
		{
			double xs = s.x_at_y(ys);
			if(xs > a_start && xs < a_stop)
				add_one_split<func>(xs,ys, times);
		}
	}		
}

struct dupe_bot_reg_type {
	bool operator()(const reg_info_t& lhs, const reg_info_t& rhs) const { 
		if(lhs.bot_type == reg_fac && lhs.bot_side != rhs.bot_side)
			return false;
		return lhs.bot_type == rhs.bot_type; 
	}
};

/*	There's a nasty edge case in the code: if a segment is VERY close but not quite veritcal, the division code wants to intersect the rising section with
 *	the various cut lines (center, top safe, bottom safe) to then break the line into horizontal regions, which would then become slop, building, slop 
*	over building, etc.  
*
*	Buuuuuuut....the math is done in raw double, so if the dx of the segment is REALLY tiny, the intersections of the horizointal lines with the segment will
*	erroneously be calculated outside the horizontal span of the segment, and the 'slicer' code will throw them out.  
*
*	Once we don't have valid intersections at the division line, the block processing algo goes haywire and we die.
*
*	Buuuuuuuut....this case is silly - the line is almost vertical and produces no useful effect by being horizontal.  At best we will consolidate its length
*	into the next region, at worst we screw up and make a sliver.
*
*	So, we simply 'square' the block, moving the corners to make the side truly vertical, and go home happy.
*/
static void fix_near_vertical(Polygon2& p)
{
	for(int i = 0; i < p.size(); ++i)
	{
		Segment2 s(p.side(i));
		double dx = fabs(s.p1.x()-s.p2.x());
		double dy = fabs(s.p1.y()-s.p2.y());
		
		if(dx < 0.01)
		{
			DebugAssert(dy > 1.0);
			p[i].x_ = s.p2.x();
		}		
	}
}
 
static int	init_subdivisions(
							Pmwx::Face_handle f,
							const FillRule_t * info, CoordTranslator2& translator, 
							vector<block_pt>& outer_ccb_pts, 		
							vector<BLOCK_face_data>& parts, 
							vector<Block_2::X_monotone_curve_2>& curves)
{
	int i;
	/***********************************************************************************************
	 * OVERVIEW
	 ***********************************************************************************************/
	 
	//	This function creates the curves to fill a block with a mix of facades and possibly an AGB
	//	to cover its interior.
	//
	//	We are going to work in a local coordinate system aligned to the major axis of the block.
	//	The "A" axis runs along this axis, and the "B" axis is CCW perpendicular to it.  If our
	//	block is longer east-west than north-south (think NYC) then the A axis would run eastward
	//	along the south side and the B axis would run northward along the west side.

	DebugAssert(info->fac_id != NO_VALUE || info->agb_id != NO_VALUE);

	vector<pair<Pmwx::Halfedge_handle,Pmwx::Halfedge_handle> >	sides;
	Polygon2													mbounds;		// The block in metric bounds
	Polygon2													abounds;		// The block in axis-aligned bounds
	
	if(!build_convex_polygon(f->outer_ccb(),sides,translator,mbounds, 2.0, 2.0))
		return 0;
	
	Vector2	va, vb;
	int mj = pick_major_axis(sides,mbounds,va,vb);
	DebugAssert(mj >= 0);
	
	// Bounds is our axis-aligned bounding box.  Note that while the origin of the block might
	// be the start of the side defining the major axis, the axis-aligned lower left could be 
	// well into negative space.  Bottom line is, we don't ever assume the origin has meaning.
	
	double bounds[4];			// bounds in AB space.

	bounds[0] = bounds[2] = va.dot(Vector2(mbounds[0]));
	bounds[1] = bounds[3] = vb.dot(Vector2(mbounds[0]));
	
	int left = 0, right = 0;					// Index number of extreme left & right points in A axis.
	
	for(i = 0; i < mbounds.size(); ++i)
	{
		double ca = va.dot(Vector2(mbounds[i]));
		double cb = vb.dot(Vector2(mbounds[i]));
		
		abounds.push_back(Point2(ca,cb));
	}
	
	fix_near_vertical(abounds);
	
	for(i = 0; i < abounds.size(); ++i)
	{
		double ca = abounds[i].x();
		double cb = abounds[i].y();
		
		if(ca < abounds[left].x())
			left = i;

		if(ca > abounds[right].x())
			right = i;
		
		bounds[0] = min(bounds[0],ca);
		bounds[1] = min(bounds[1],cb);
		bounds[2] = max(bounds[2],ca);
		bounds[3] = max(bounds[3],cb);	
	}
	DebugAssert(left != right);
	
	double sbounds[4] = {		// bounds inset in slop, AB space.
		bounds[0] + info->agb_slop_width,
		bounds[1] + info->agb_slop_depth,
		bounds[2] - info->agb_slop_width,
		bounds[3] - info->agb_slop_depth };
	
	DebugAssert(bounds[1] > UNDEF);	// 8 km south of the block should be a safe flag.

	TRACE_SUBDIVIDE("Block is from %lf,%lf to %lf,%lf\n", bounds[0],bounds[1],bounds[2],bounds[3]);
	
	/***********************************************************************************************
	 * BASIC TIME REGION DIAGNOSTICS
	 ***********************************************************************************************/

	
//
//	 
//	 
//	todo - we need a subdivision algo that can find 'zones' of autogen broken at interesting times,
//	like when we go from these depths vertically:
//	
//	- scrap (too thin for 1 row)
//	- half-size (1 row of buildings)
//	- full size (2 rows of buildings)
//	- AGB safe (2 rows and within the slop.)
//	
//	we need to synthesize the regions to not have any be too small, sometimes, 
	 
	// We are going to isolate the region along the a axis that can have an AGB on it.  Since the
	// building splits run 'along' the A axis we can think of regions along the A axis that will have
	// either facades or AGBs; the cuts between them are always perpendicular ot A.
	
//	time_region		agb_friendly(bounds[0],bounds[2]);		// Start with the entire block.

	double fac_extra_len = info->fac_extra;					// This is the worst-case width of the widest
//	for(int i = 0; i < outer_ccb_pts.size(); ++i)			// road in the block.  It is used as a fudge 
//	{														// factor on fac width so that the road eating
//		if(outer_ccb_pts[i].edge_type.first != NO_VALUE)	// into the fac doens't cause the fac to be too small.
//			fac_extra_len = dobmax2(fac_extra_len,gNetReps[outer_ccb_pts[i].edge_type.first].width());
//	}
//	TRACE_SUBDIVIDE("Facade overhang is: %lf\n",fac_extra_len);
	
	// facade depth splits!
	int fds = info->fac_depth_split;
	
	DebugAssert(outer_ccb_pts.size() >= 3);

	// These are the corners of the subset of the polygon that runs along the true top and bottom of the
	// polygon.  For degenerate cases like trapezoids, one edge will be pulled AAAAAALL the way to the other!

	// If we are a 30-60 block with 2 subdivisions and 30max facades, then: fac-depth max is 30, fac depth min is 15, and at worst
	// we need two maxed out facades to cover our area.
	// As we step the block depth, we notice that:
	// - If BOTH edges are inside the 'safe' zone then by definition a single facade can cross.
	// - If at least ONE edge is out side the safe zone, then we can always subdivide (because we have 
	//   1.5x max, which means two facades at 75% of max, which is totally legit.
	
	double fac_depth_max = info->max_side_minor / (float) fds;
	double fac_depth_min = fac_depth_max * 0.5;
	
	int prev_bottom = left, prev_top = left;
	int bottom = abounds.next(prev_bottom);
	int top = abounds.prev(prev_top);
	DebugAssert(bottom != right || top != right);
	
	double b_split = (bounds[1] + bounds[3]) * 0.5;		// This is where we will split a 2-layer block.
	double top_safe = b_split + fac_depth_min;			// This is how far PAST the line the region must be go to safely
	double bot_safe = b_split - fac_depth_min;			// spawn top or bottom...

	double all_splits[] = { b_split, top_safe, bot_safe };
	const int num_splits = sizeof(all_splits) / sizeof(all_splits[0]);

//	if(fds > 1)
//		printf("Splits are: %lf, %lf, %lf\n", bot_safe, b_split, top_safe);
//	printf("Safe bounds are: %lf, %lf, %lf, %lf\n", sbounds[0],sbounds[1],sbounds[2],sbounds[3]);
	
//	printf("Block trace from %lf to %lf\n",bounds[0],bounds[2]);
//	debug_mesh_line(
//			translator.Reverse(Point2() + va * bounds[0] + vb * bot_safe),
//			translator.Reverse(Point2() + va * bounds[2] + vb * bot_safe),
//			0,1,0,0,1,0);

//	debug_mesh_line(
//			translator.Reverse(Point2() + va * bounds[0] + vb * top_safe),
//			translator.Reverse(Point2() + va * bounds[2] + vb * top_safe),
//			0,0,1,0,0,1);

	vector<reg_info_t>	regions;

	while(1)
	{
		// EMIT BLOCK
		double a_start = max(abounds[prev_bottom].x(),abounds[prev_top].x());
		double a_stop = min(abounds[bottom].x(),abounds[top].x());
		DebugAssert(a_start <= a_stop);
		
		// Sanity check...a_start == a_stop when we have VERTICAL sides...
		if(a_start < a_stop)
		{
		
			Segment2	bottom_seg = abounds.side(prev_bottom);
			Segment2	top_seg = abounds.side(top);
			
			// If we have to consider whether a block 'transitions' the safety zones on the top or bottom...
			// and subdivide accordingly.
			
			map<double, pair<double, double> >	bound_times;
			
			split_segment<set_first>(bottom_seg, a_start, a_stop, bound_times, all_splits, num_splits);
			split_segment<set_second>(top_seg, a_start, a_stop, bound_times, all_splits, num_splits);
			
			for(map<double,pair<double,double> >::iterator i = bound_times.begin(); i != bound_times.end(); ++i)
			{
				if(i->second.first == UNDEF)
					i->second.first = bottom_seg.y_at_x(i->first);

				if(i->second.second == UNDEF)
					i->second.second = top_seg.y_at_x(i->first);
			}
			
			DebugAssert(a_start != a_stop);
			DebugAssert(bound_times.size() > 1);
			DebugAssert(bound_times.begin()->first == a_start);
			DebugAssert(nth_from(bound_times.end(),-1)->first == a_stop);
			
			map<double,pair<double,double> >::iterator p1,p2;
			p1 = bound_times.begin();
			p2 = p1;
			++p2;
			
			while(p2 != bound_times.end())
			{	
				double b1 = p1->second.first;
				double b2 = p2->second.first;
				double t1 = p1->second.second;
				double t2 = p2->second.second;
				
				double min_depth = min(t1-b1,t2-b2);
										
				
				reg_info_t reg;
				
				reg.top_slope = fabs(t2-t1) / (p2->first - p1->first);
				reg.bot_slope = fabs(b2-b1) / (p2->first - p1->first);
				reg.top_side = top;
				reg.bot_side = prev_bottom;			
				
				if(reg.top_slope > reg.bot_slope)
					reg.block_dir = (t2 == t1) ? 0 : (t1 < t2 ? 1 : -1);
				else			
					reg.block_dir = (b2 == b1) ? 0 : (b2 < b1 ? 1 : -1);			
				
				reg.a_time = p1->first;
				if(b1 <= sbounds[1] && b2 <= sbounds[1] &&
				   t1 >= sbounds[3] && t2 >= sbounds[3])
				{
					reg.bot_type = reg_agb;
					reg.top_type = reg_agb;
				}
				else if(fds > 1)
				{
					reg.is_split = true;
					
					// double split calculations -- calculate bottom
					if(b1 >= b_split && b2 >= b_split)
					{
						// Bottom is empty
						reg.bot_type = reg_none;
						if(reg.bot_slope > SLOPE_TO_SIDE || reg.top_slope > SLOPE_TO_SIDE)
							reg.top_type = reg_slop;
						else
							reg.top_type = ((t1 >= top_safe && t2 >= top_safe) &&		// This logic controls the case where the block comes to a sharpened
											 (b1 <= b_split || b2 <= b_split))			// point.  The bottom has nothing and the top is very sharp.  The top
											 ? reg_fac : reg_junk;						// condition makes sure the top edge is not tapering down and the bottom 
					}																	// condition controls how wide we must be to go from parking lot to bldg.
					else if(t1 <= b_split && t2 <= b_split)								// Change the || to && to force parking lot all the way to the subdivision point.
					{
						// top is empty
						reg.top_type = reg_none;
						if(reg.bot_slope > SLOPE_TO_SIDE || reg.top_slope > SLOPE_TO_SIDE)
							reg.bot_type = reg_slop;
						else
							reg.bot_type = 
									((b1 <= bot_safe && b2 <= bot_safe) &&
									 (t1 >= b_split || t2 >= b_split))
									   ? reg_fac : reg_junk;
					}
					else
					{
						DebugAssert(b1 <= b_split);
						DebugAssert(b2 <= b_split);
						DebugAssert(t1 >= b_split);
						DebugAssert(t2 >= b_split);
						
						// truly split
						
						if(reg.top_slope > SLOPE_TO_SIDE)
							reg.top_type = reg_slop;
						else
							reg.top_type = (t1 >= top_safe && t2 >= top_safe) ? reg_fac : reg_junk;
						
						if(reg.bot_slope > SLOPE_TO_SIDE)
							reg.bot_type = reg_slop;
						else
							reg.bot_type = (b1 <= bot_safe && b2 <= bot_safe) ? reg_fac : reg_junk;
					}
				}
				else 
				{
					if(reg.top_slope > SLOPE_TO_SIDE || reg.bot_slope > SLOPE_TO_SIDE)
						reg.bot_type = reg_slop;
					else
						reg.bot_type = (min_depth >= fac_depth_min) ? reg_fac : reg_junk;
					reg.top_type = reg_none;
					reg.is_split = false;
				}

	//			printf("Region from: %lf (%lf,%lf) to %lf (%lf,%lf) / %lf,%lf d=%d [min: %lf max: %lf]\n",
	//				p1->first,p1->second.first,p1->second.second,
	//				p2->first,p2->second.first,p2->second.second,
	//				reg.bot_slope,reg.top_slope,
	//				reg.block_dir,
	//				p1->second.second - p1->second.first,
	//				p2->second.second - p2->second.first);

				regions.push_back(reg);
				
				++p1;
				++p2;
			}
			
		} // safety check for VERTICAL sides.
		else
		{
			// just some debug checking that we failed for the reason we thought we would....
			
			Segment2	bottom_seg = abounds.side(prev_bottom);
			Segment2	top_seg = abounds.side(top);
			
			DebugAssert(bottom_seg.is_vertical() || top_seg.is_vertical());
			
		}
	
		if(bottom == right && top == right)
			break;
	
		// This phase of the loop tries to walk _one_ segment forward to find the next 'region'
		// of the block in A-space.
	
		if(bottom != right && top != right)
		{
			if(abounds[bottom].x() == abounds[top].x())
			{
				prev_bottom = bottom;
				bottom = abounds.next(bottom);		
				prev_top = top;
				top = abounds.prev(top);						
			}
			else if(abounds[bottom].x() < abounds[top].x())
			{
				prev_bottom = bottom;
				bottom = abounds.next(bottom);		
			}
			else
			{
				prev_top = top;
				top = abounds.prev(top);						
			}
		} 
		else if(bottom != right)
		{
			prev_bottom = bottom;
			bottom = abounds.next(bottom);		
		}
		else
		{
			prev_top = top;
			top = abounds.prev(top);						
		}
	
	}	

	reg_info_t cap;
	cap.bot_type =reg_TERM;
	cap.top_type =reg_TERM;
	cap.bot_slope=0.0;
	cap.top_slope=0.0;
	cap.bot_side=-1;
	cap.top_side=-1;
	cap.block_dir=0;
	cap.is_split = 1;	// so that the top term is copied ot the bottom in the zipper code.
	cap.a_time = bounds[2];
	regions.push_back(cap);
	
//	printf("---- pre consolidation ---\n");
//	for(i = 0; i < regions.size(); ++i)
//	if(regions[i].is_split)
//		printf("   %d(%d/%d): %f (%s/%s) @ %lf/%lf\n", i, regions[i].bot_side, regions[i].top_side, regions[i].a_time, reg_debug_str[regions[i].bot_type], reg_debug_str[regions[i].top_type], regions[i].bot_slope,regions[i].top_slope);
//	else
//		printf("   %d(%d): %f (%s) @ %lf/%lf\n", i, regions[i].bot_side, regions[i].a_time, reg_debug_str[regions[i].bot_type], regions[i].bot_slope,regions[i].top_slope);
	
	// 
	/***********************************************************************************************
	 * REGION GROUPING AND CONSOLIDATION
	 ***********************************************************************************************/
	
	// Rule 1: up to X meters of slop to the left and right of an AGB get merged into an AGB
	{
		int first_agb = find_first_bot_type(regions,reg_agb);
		if(first_agb != -1)
		{
			if(regions[first_agb].a_time <= sbounds[0])
			{
				regions[first_agb].a_time = bounds[0];
				if(first_agb > 0)
					regions.erase(regions.begin(),regions.begin() + first_agb);
			}
		}
		
		int last_agb = find_last_bot_type(regions,reg_agb);
		if(last_agb != -1)
		{
			if(regions[last_agb+1].a_time >= sbounds[2])
			{	
				// Confirm we can leave our AGB and the terminator and still delete at least one
				// region
//				if(last_agb + 1 < regions.size() - 1)
				if(last_agb + 2 < regions.size())		// avoid size() - x which can sign wrap
					regions.erase(regions.begin()+last_agb+1,regions.end()-1);
			}
		}		
	}
	
	// Rule 2: merge any adjacent AGBs (if there are any) to ensure correct book keeping.
	i = 0;
	while(i < regions.size())
	{
		if(regions[i].bot_type == reg_agb &&
			regions[i+1].bot_type == reg_agb)		// safe because we ALWAYS have a 'term' end!
		{
			// Important: take the FLATTEST AGB of the two - that way the single region representing
			// The AGB will contain sides that are a good proxy for the AGB.  We are often consolidating
			// smal amounts of 'slop' and if we get the junk's side, our AGB will be based on a side of
			// the block and not top/bottom.  
			
			// (If a side slop makes it ALL the way from the TOP of the block to the BOTTOM in a single side
			// then its top/bottom will meet the AGB extrema rules - we'll carve off a facade later when
			// we discover that the AGB is not square enough on its ends.)
			
			// To get facades to merge with their neighbors properly, it is key that the AGB be represented
			// by the flat sides that dominate it.

			double slope_i   = max(regions[i  ].bot_slope,regions[i  ].top_slope);
			double slope_ipp = max(regions[i+1].bot_slope,regions[i+1].top_slope);
		
			if(slope_i > slope_ipp)
				regions.erase(regions.begin()+i  );			
			else		
				regions.erase(regions.begin()+i+1);			
		}
		else
			++i;			
	}
	
	// Rule 3: if there is any remaining, um, anything next to the AGB that is below min-fac-width, borrow
	// some AGB-space to widen it.  Otherwise some slant blocks will be nothing but a 5m wide slop slice that
	// just barely didn't make the AGB
	
	if(info->fac_depth_split > 0.0)
	for(i = 0; i < regions.size(); ++i)
	{
		if(regions[i].bot_type == reg_agb)
		{
			// TWO reasons why we might need to chew a little AGB:
			// 1. Our next 'thing is' really short or
			// 2. Our next thing is really sloped to the side (and is thus slop - it needs to join SOMETHING.
			
			if(i > 0)
			if(((regions[i].a_time - regions[i-1].a_time) < info->fac_extra) ||
				regions[i-1].bot_slope > SLOPE_TO_SIDE || 
				regions[i-1].top_slope > SLOPE_TO_SIDE)
			{
				regions.insert(regions.begin()+i,regions[i]);				
				regions[i].top_type = reg_fac;
				regions[i].bot_type = reg_fac;
				regions[i+1].a_time += info->fac_extra;
				++i;
			}		
			
			if(regions[i+1].bot_type != reg_TERM)
			if(((regions[i+2].a_time - regions[i+1].a_time) < info->fac_extra) ||
				regions[i+1].bot_slope > SLOPE_TO_SIDE ||
				regions[i+1].top_slope > SLOPE_TO_SIDE)
			{
				regions.insert(regions.begin()+i,regions[i]);
				regions[i+1].a_time   = regions[i+2].a_time - info->fac_extra;
				regions[i+1].top_type = reg_fac;
				regions[i+1].bot_type = reg_fac;				
				++i;
			}
		}
	}
	
	// Rule 4: If the AGBs are too small to be, like, AGBs, we facade-ize them.
	for(i = 0; i < regions.size(); ++i)
	if(regions[i].bot_type == reg_agb)
	if((regions[i+1].a_time-regions[i].a_time) < info->agb_min_width)
	{
		if(fds > 1)
		{
			regions[i].bot_type = reg_fac;
			regions[i].top_type = reg_fac;
			regions[i].is_split = true;
		}
		else
		{
			regions[i].bot_type = reg_fac;
			regions[i].top_type = reg_none;
			regions[i].is_split = false;
		}
	}
	
	// Rule 5: AGB-only blocks with no AGBs are a fail - early exit.

	if(info->fac_depth_split == 0.0)
	{
		DebugAssert(regions.size() > 1);
		for(i = 1; i < regions.size(); ++i)
		if(regions[-1].bot_type != reg_agb)
			return 0;		
	}
	
	// Rule 6: we split the top and bottom.

	vector<reg_info_t>	regions_top(regions), regions_bot;
	regions.swap(regions_bot);
	
	vector<reg_info_t> * regions_all[2] = { &regions_bot, &regions_top };
	
	for(i = 0; i < regions_top.size(); ++i)
	{
		if(regions_top[i].is_split)
		{
			regions_top[i].bot_type = regions_top[i].top_type;
			regions_top[i].bot_side = regions_top[i].top_side;
		} else {
			regions_top[i].bot_type = reg_none;			
		}
	}

	// Rule 7: merge same type areas - sometimes we have to break at a side change.
	for(i = 0; i < 2; ++i)
		regions_all[i]->erase(unique(regions_all[i]->begin(),regions_all[i]->end(),dupe_bot_reg_type()),regions_all[i]->end());
	
//	printf("---- post consolidation ---\n");
//	for(i = 0; i < regions_bot.size()-1; ++i)
//		printf("   %d: %f (%s %d) %.1lfm\n", i, regions_bot[i].a_time, reg_debug_str[regions_bot[i].bot_type],regions_bot[i].bot_side,regions_bot[i+1].a_time-regions_bot[i].a_time);
//	printf("          -----   \n");
//	for(i = 0; i < regions_top.size()-1; ++i)
//		printf("   %d: %f (%s %d) %.1lfm\n", i, regions_top[i].a_time, reg_debug_str[regions_top[i].bot_type],regions_top[i].bot_side,regions_top[i+1].a_time-regions_top[i].a_time);
	
	// Rule 8: time used for junk and slop become part of the adjacent wider/deeper block.
	
	for(i = 0; i < 2; ++i)
	{
		vector<reg_info_t>::iterator r = regions_all[i]->begin(); 
		while(r != regions_all[i]->end())
		{
			if(r->bot_type == reg_none || r->bot_type == reg_slop)
			{
				if(r->block_dir < 0)
				{
					r = regions_all[i]->erase(r);
				}
				else				
				{
					vector<reg_info_t>::iterator n(r);
					++n;
					n->a_time = r->a_time;
					r = regions_all[i]->erase(r);
				}
			}
			else
				++r;
		}
	}
	
//	printf("---- with slop removal ---\n");
//	for(i = 0; i < regions_bot.size()-1; ++i)
//		printf("   %d: %f (%s %d) %.1lfm\n", i, regions_bot[i].a_time, reg_debug_str[regions_bot[i].bot_type],regions_bot[i].bot_side,regions_bot[i+1].a_time-regions_bot[i].a_time);
//	printf("          -----   \n");
//	for(i = 0; i < regions_top.size()-1; ++i)
//		printf("   %d: %f (%s %d) %.1lfm\n", i, regions_top[i].a_time, reg_debug_str[regions_top[i].bot_type],regions_top[i].bot_side,regions_top[i+1].a_time-regions_top[i].a_time);
	
	int part_base = parts.size();

	for(i = 0; i < (fds > 1 ? 2 : 1); ++i)
	{
		regions_all[i]->front().a_time -= 1.0;
		regions_all[i]->back().a_time += 1.0;
	}
	
	double	unified[2] = { bounds[1] - 1.0, bounds[3] + 1.0 };
	double	divided[3] = { bounds[1] - 1.0, b_split, bounds[3] + 1.0 };
	
	double snickers_front = regions_bot.front().a_time;
	double snickers_back = regions_bot.back().a_time;
	if(fds > 1)
	{
		snickers_front = min(snickers_front, regions_top.front().a_time);
		snickers_back = max(snickers_back,regions_top.back().a_time);
	}
	
	candy_bar	snickers(fds > 1 ? 2 : 1, fds > 1 ? divided : unified, snickers_front, snickers_back);
	
	int ctr = part_base;
	for(i = 0; i < (fds > 1 ? 2 : 1); ++i)
	{
		double b_bot = (fds > 1 && i == 1) ? b_split : bounds[1]-1.0;
		double b_top = (fds > 1 && i == 0) ? b_split : bounds[3]+1.0;
	
		for(vector<reg_info_t>::iterator r = regions_all[i]->begin(); r != regions_all[i]->end(); ++r)
		if(r->bot_type != reg_TERM)
		{
			vector<reg_info_t>::iterator n(r);
			++n;
			
			double rgb[3] = { 0 };
			switch(r->bot_type) {
			case reg_agb:		rgb[0] = 1; break;
			case reg_fac:		rgb[2] = 1; break;
			case reg_junk:		rgb[0] = 1; rgb[1] = 1; break;
			}
			
			if(r->bot_type == reg_agb && i == 1)
				continue;
			
			int b1 = i;
			int b2 = i+1;
			if(fds > 1 && r->bot_type == reg_agb) b2 = 2;
							
			if(r->bot_type == reg_agb)
			{
				b_bot = bounds[1]-1.0;
				b_top = bounds[3]+1.0;
			
				BLOCK_face_data bd(usage_Polygonal_Feature, info->agb_id);
				bd.major_axis = va;
				bd.height = 0;
				bd.simplify_id = ctr++;
				
				snickers.insert_block(bd, b1, b2, r->a_time,n->a_time);
			}
			else if(r->bot_type == reg_junk)
			{
				BLOCK_face_data bd(usage_Polygonal_Feature, info->fil_id);
				bd.major_axis = va;
				bd.height = 0;
				bd.simplify_id = ctr++;				
				snickers.insert_block(bd, b1, b2, r->a_time,n->a_time);
			}
			else
			{
				double width = n->a_time - r->a_time;				
				float max_height = f->data().GetParam(af_HeightObjs,0.0);
				FacadeSpelling_t * fac_rule = GetFacadeRule(info->zoning, info->variant, width, max_height, (bounds[3]-bounds[1]) / fds);
				if(fac_rule == NULL)
				{
					DebugAssert(!"Fac fail!!?!?");
					return 0;
				}
				//printf("Tried to fill %.1lf, got %.1f (%.1lf - %.1lf)\n",width,fac_rule->width_real, fac_rule->width_min, fac_rule->width_max);

				double accum = 0.0;
				for(int fn = 0; fn < fac_rule->facs.size(); ++fn)
				{
					//printf("     %s (%.1f\n", FetchTokenString(fac_rule->facs[fn].fac_id_front), fac_rule->facs[fn].width);
					BLOCK_face_data bf(usage_Polygonal_Feature,fac_rule->facs[fn].fac_id_front);
					bf.major_axis = va;
					if(max_height > 0 && max_height < fac_rule->facs[fn].height_min)
					{
						printf("ERROR: block height: %f, zone %f..%f\n", max_height,fac_rule->height_min,fac_rule->height_max);
						for(int k = 0; k < fac_rule->facs.size(); ++k)
							printf("%d:  %f..%f  %s/%s\n", k, fac_rule->facs[k].height_min,fac_rule->facs[k].height_max,
								FetchTokenString(fac_rule->facs[k].fac_id_front),
								FetchTokenString(fac_rule->facs[k].fac_id_back));
					}
					Assert(max_height == 0.0f || max_height >= fac_rule->facs[fn].height_min);
					if(max_height > 0.0)
						bf.height = RandRange(fac_rule->facs[fn].height_min,fltmin2(fac_rule->facs[fn].height_max,max_height));
					else
						bf.height = RandRange(fac_rule->facs[fn].height_min,fac_rule->facs[fn].height_max);
					bf.simplify_id = ctr++;
					
					double a_start = double_interp(0,r->a_time,fac_rule->width_real,n->a_time,accum);
					if(fn == 0) a_start = r->a_time;
					accum += fac_rule->facs[fn].width;
					double a_end = double_interp(0,r->a_time,fac_rule->width_real,n->a_time,accum);
					if(fn == (fac_rule->facs.size()-1))
						a_end = n->a_time;
					
					snickers.insert_block(bf,b1,b2,a_start,a_end);
				}		
			
			}

//			Point2 p1 = Point2() + va * r->a_time + vb * b_bot;
//			Point2 p2 = Point2() + va * r->a_time + vb * b_top;
//			Point2 p3 = Point2() + va * n->a_time + vb * b_top;
//			Point2 p4 = Point2() + va * n->a_time + vb * b_bot;
			
//			push_block_curve(curves,p1,p2,parts.size());
//			push_block_curve(curves,p2,p3,parts.size());
//			push_block_curve(curves,p3,p4,parts.size());
//			push_block_curve(curves,p4,p1,parts.size());
//						
//			parts.push_back(BLOCK_face_data());
//			
//			parts.back().usage = usage_Polygonal_Feature;
//			parts.back().feature = info->agb_id;
//			parts.back().height = 0.0;
//			parts.back().major_axis = va;
//			parts.back().simplify_id = parts.size()-1;
			
//			debug_mesh_line(translator.Reverse(p1),translator.Reverse(p2),rgb[0],rgb[1],rgb[2],rgb[0],rgb[1],rgb[2]);
//			debug_mesh_line(translator.Reverse(p2),translator.Reverse(p3),rgb[0],rgb[1],rgb[2],rgb[0],rgb[1],rgb[2]);
//			debug_mesh_line(translator.Reverse(p3),translator.Reverse(p4),rgb[0],rgb[1],rgb[2],rgb[0],rgb[1],rgb[2]);
//			debug_mesh_line(translator.Reverse(p4),translator.Reverse(p1),rgb[0],rgb[1],rgb[2],rgb[0],rgb[1],rgb[2]);
		}
	}
	
	snickers.snap_round();
	snickers.build_curves(parts, va, vb, curves);
	
/*	
	int left_top = left, right_top = right, left_bottom = left, right_bottom = right;
	
	while(left_top != right && abounds[left_top].y() < sbounds[3])
		left_top = abounds.prev(left_top);

	while(left_bottom != right && abounds[left_bottom].y() > sbounds[1])
		left_bottom = abounds.next(left_bottom);
		
	while(right_top != left && abounds[right_top].y() < sbounds[3])
		right_top = abounds.next(right_top);

	while(right_bottom != left && abounds[right_bottom].y() > sbounds[1])
		right_bottom = abounds.prev(right_bottom);
		
		
	// Now we can see: if the edges of our 'flat top' zones are too far from the edge they SHOULD be at, then
	// we have a corner cut off, or a crazy-angled side or some other such nuttiness, and we need to NOT have
	// an AGB there!
	
	if(abounds[left_top].x() > sbounds[0])
		agb_friendly -= time_region::interval(bounds[0],abounds[left_top].x());
		
	if(abounds[left_bottom].x() > sbounds[0])
		agb_friendly -= time_region::interval(bounds[0],abounds[left_bottom].x());
		
	if(abounds[right_top].x() < sbounds[2])
		agb_friendly -= time_region::interval(abounds[right_top].x(),bounds[2]);
		
	if(abounds[right_bottom].x() < sbounds[2])
		agb_friendly -= time_region::interval(abounds[right_bottom].x(),bounds[2]);
		
	
	// One last sanity check: for now we assume that whomever the hell called us doesn't have "divets" cut out
	// of our top and bottom.  

	// If this code blows up, we can find and black-list these angular regions from the AGB.
	
	#if DEV
	for(i = right_top; i != left_top; i = abounds.next(i))
		DebugAssert(abounds[i].y() >= sbounds[3]);


	for(i = left_bottom; i != right_bottom; i = abounds.next(i))
		DebugAssert(abounds[i].y() <= sbounds[1]);
	#endif

	// If we don't have the option of making facades at all, exit if we already can tell that our time region isn't
	// goin to hit the AGB-only case.
	
	if(info->fac_depth_split == 0.0)
	{
		if(!agb_friendly.is_simple() ||						// Hrm...AGB block has a facade hole in the middle
			agb_friendly.empty() ||							// Whole area is facade
			agb_friendly.get_min() > bounds[0] ||			// Block starts with facade
			agb_friendly.get_max() < bounds[2] ||			// Block ends with facade
			(bounds[2] - bounds[0]) < info->agb_min_width)	// Block is TOO SMALL - will become a facade!!
			
			return 0;
	}
	
	// This goes in and erodes the AGB friendly regions to ensure that at a minimum a fac-min length chunk of
	// major road side is covered by the facade.  This ensures that, no matter how totally fubar the sloppy ends
	// might be, we have a place to put the store front of the facade.
	
	time_region		must_be_fac_too;
	for(time_region::const_iterator agbs = agb_friendly.begin(); agbs != agb_friendly.end(); ++agbs)
	{
		double width = agbs->second - agbs->first;
//		if(width < real_min_agb)
//		{
//			TRACE_SUBDIVIDE("Time region %lf to %lf rejected - too small.\n", agbs->first, agbs->second);
//			DebugAssert(info->fac_id != NO_VALUE);
//			must_be_fac_too += *agbs;			
//		}
//		else
		{
			double must_have = 0.0;
			if(agbs->first != bounds[0])
				must_have += fac_extra_len;
			if(agbs->second != bounds[2])
				must_have += fac_extra_len;
			if(width > (must_have + info->agb_min_width))
			{
				if(agbs->first != bounds[0])
					must_be_fac_too += time_region::interval(agbs->first,agbs->first + fac_extra_len);
				if(agbs->second != bounds[2])
					must_be_fac_too += time_region::interval(agbs->second - fac_extra_len, agbs->second);
			} else {
				must_be_fac_too += *agbs;			
			}
		}		
	}

	DebugAssert(must_be_fac_too.empty() || info->fac_depth_split);

	agb_friendly -= must_be_fac_too;

//	agb_friendly ^= time_region::interval(bounds[0],bounds[2]);
*/
	/***********************************************************************************************
	 * SLICE AND DICE
	 ***********************************************************************************************/

	// Now we have enough information to slice up our area.  We're going to wal the positive and 
	// negative space of the AGB region and produce a series of cut intervals and primitive types.
	// When we hit a facade region, we'll use a formula to subdivide it - it might be quite large.
/*
	vector<pair<double, bool> >	a_cuts;
	vector<pair<int, float>	>	a_features, b_features;

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
			
			#if !DEV
				#error THIS IS BUGGY.   Make sure we consider the role of the division on the width.
			#endif
			
			FacadeSpelling_t * fac_rule = GetFacadeRule(info->zoning, info->variant, width, f->data().GetParam(af_HeightObjs,0.0), (bounds[3]-bounds[1]) / fds);
			if(fac_rule == NULL)
				return 0;

			float accum = 0.0;
			for(int n = 0; n < fac_rule->facs.size(); ++n)
			{
				a_cuts.push_back(pair<double,bool>(
					double_interp(
							0,fac.first,fac_rule->width_real,fac.second,accum + RandRange(-2,2)),false));
				a_features.push_back(pair<int,float>(fac_rule->facs[n].fac_id_front,RandRange(fac_rule->facs[n].height_min,fac_rule->facs[n].height_max)));
				b_features.push_back(pair<int,float>(fac_rule->facs[n].fac_id_back,RandRange(fac_rule->facs[n].height_min,fac_rule->facs[n].height_max)));
				a_features.back().second = min(a_features.back().second,max(f->data().GetParam(af_HeightObjs,0.0),16.0f));
				b_features.back().second = min(b_features.back().second,max(f->data().GetParam(af_HeightObjs,0.0),16.0f));

				accum += fac_rule->facs[n].width;
			}		
		}
		if(agb.first != agb.second)
		{
			// This calculation figures out how much the roads in the major axis direction
			// are drifting, and subdivides the AGB to 'stair step' as needed.		
//			double top_span = find_b_interval(agb,top_agbs, bounds);
//			TRACE_SUBDIVIDE("top span: %lf\n", top_span);
//			double bot_span = find_b_interval(agb,bot_agbs, bounds);
//			TRACE_SUBDIVIDE("bot span: %lf\n", bot_span);
//			double worst_span = max(top_span,bot_span);
//			double subdiv = dobmax2(1.0,floor(worst_span / info->agb_slop_depth));
//			TRACE_SUBDIVIDE("worst span: %lf\n", worst_span);
//			TRACE_SUBDIVIDE("Subdiving agb from %lf to %lf ito %lf pieces.\n", agb.first,agb.second,subdiv);
			
			double subdiv = 1.0;
			
			for(double s = 0.0; s < subdiv; s += 1.0)
			{
				a_cuts.push_back(pair<double,bool>(double_interp(0,agb.first,subdiv,agb.second,s),true));
				a_features.push_back(pair<int,bool>(info->agb_id,0.0));
				b_features.push_back(pair<int,bool>(info->agb_id,0.0));
			}		
		}
				
		if(agb_iter == agb_friendly.end())
			break;
		
		last = agb_iter->second;		
		++agb_iter;
	}

	
	a_cuts.push_back(pair<double,bool>(bounds[2]+1,false));
	a_features.push_back(pair<int,float>(NO_VALUE,0.0));
	b_features.push_back(pair<int,float>(NO_VALUE,0.0));
	a_cuts.front().first -= 1.0;
*/
	/***********************************************************************************************
	 * GEOMETRY GENERATION
	 ***********************************************************************************************/


#if 0
	//	Finally, given the cuts, we can make the polygon curve shapes and stash them.
	int part_base = parts.size();
	DebugAssert(a_cuts.size() >= 2);
	int a_blocks = a_cuts.size()-1;
	parts.resize(part_base + fds * a_blocks);
	
	for(int a = 0; a < a_blocks; ++a)
	for(int f = 0; f < fds; ++f)
	{
		int pid = part_base + a * fds + f;
		parts[pid].usage = usage_Polygonal_Feature;
//		parts[pid].feature = a_cuts[a].second ? info->agb_id : info->fac_id;
		if(f == 0)
		{
			parts[pid].feature = a_features[a].first;
			parts[pid].height = a_features[a].second;
		} 
		else
		{
			parts[pid].feature = b_features[a].first;
			parts[pid].height = b_features[a].second;
		}
		parts[pid].major_axis = va;
		DebugAssert(parts[pid].feature != NO_VALUE);
		parts[pid].simplify_id = part_base + f;
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

#endif

	
	// This puts down the outer roadways and boundaries. Because AGB filling and edge filling are NOT used together,
	// the edge-based road fill will not otherwise run!

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


static bool mismatched_edge_types(const pair<int,bool>& e1, const pair<int,bool>& e2, int zoning, int variant, float height, bool fill_edges)
{
	float w1 = WidthForSegment(e1);
	float w2 = WidthForSegment(e2);
	if(w1 != w2) return true;
	if(!fill_edges)	return false;

	EdgeRule_t * r1 = edge_for_road(e1, zoning, variant, height);
	EdgeRule_t * r2 = edge_for_road(e1, zoning, variant, height);
	
	if (r1 == NULL && r2 == NULL) return false;
	if (r1 == NULL || r2 == NULL) return true;
	
	if(r1->width != r2->width || r1->resource_id != r2->resource_id) return true;
	return false;
}

static void	init_road_ccb(int zoning, int variant,float height,  Pmwx::Ccb_halfedge_circulator he, CoordTranslator2& translator, 
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
		pts[j].discon = j == 0 || mismatched_edge_types(pts[j].edge_type,pts[i].edge_type, zoning, variant, height, fill_edges);
		
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

		EdgeRule_t * er = (fill_edges && edge_ok_for_ag(pts[i].loc,pts[j].loc, translator,approx_ag)) ? edge_for_road(pts[i].edge_type, zoning, variant, height) : NULL;
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
							int offset, int zoning,
							bool	want_feature)
{
	PointRule_t * rule; 

	int idx = 0;
	for(GISPointFeatureVector::const_iterator f = feats.begin(); f != feats.end(); ++f, ++idx)
	if((rule = GetPointRuleForFeature(zoning, *f)) != NULL)
	{
		Point2	fl(trans.Forward(cgal2ben(f->mLocation)));

		int best = -1;
		double best_dist = 0;
		
		if(rule->fac_id_ant)
		{
			for(int i = 0; i < pts.size(); ++i)
			{
				int j = (i + 1) % pts.size();
				int k = (i + pts.size() - 1) % pts.size();
				if(pts[j].loc == pts[k].loc)
				if(ground_road_access_for_he(pts[i].orig))
				{
					double my_dist = pts[i].loc.squared_distance(fl);				
					if(best == -1 || my_dist < best_dist)
					{	
						best = i;
						best_dist = my_dist;
					}
				}
			}
		}
		
		if(best >= 0 && rule->fac_id_free && best_dist > sqr(max(rule->x_width_ant,rule->x_depth_ant)))
			best = -1;
		
		if(best >= 0)
		{
			Point2		anchor(pts[best].loc);
			int j = (best + pts.size() - 1) % pts.size();
			Vector2		depth(pts[j].loc,pts[best].loc);
			depth.normalize();
			Vector2	width(depth.perpendicular_cw());

			float w = WidthForSegment(pts[best].edge_type);
			if(w > 1.0)
				anchor += (depth * (w+0.5));

			Vector2	xwidth(width);
			Vector2	xdepth(depth);

			if(want_feature)
			{
				width *= rule->width_ant * 0.5;
				depth *= rule->depth_ant;
			} else {
				anchor -= depth * 0.5 * (rule->x_depth_ant - rule->depth_ant);					
				width *= rule->x_width_ant * 0.5;	
				depth *= rule->x_depth_ant;
			}
			if(!want_feature && rule->x_width_ant == 0.0) continue;
						
			push_block_curve(curves, anchor - width, anchor + width, offset + idx);
			push_block_curve(curves, anchor + width, anchor + width + depth, offset + idx);
			push_block_curve(curves, anchor + width + depth, anchor - width + depth, offset + idx);
			push_block_curve(curves, anchor - width + depth, anchor - width, offset + idx);
			
			parts[offset + idx].usage = want_feature ? usage_Polygonal_Feature : usage_OOB;
			parts[offset + idx].feature = rule->fac_id_ant;
			parts[offset + idx].height = f->mParams.count(pf_Height) ? f->mParams.find(pf_Height)->second : 0.0f;
			parts[offset + idx].simplify_id = offset + idx;
			

		
		}
		else
		{
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
			if(best >= 0 && rule->fac_id_free && best_dist > sqr(max(rule->x_width_rd,rule->x_depth_rd)))
				best = -1;
			
			if(best >= 0)
			{
				Segment2	wall(pts[best].loc,pts[(best+1)%pts.size()].loc);
				Assert(wall.p1 != wall.p2);
				Point2		anchor(wall.midpoint());
				Vector2		width(wall.p1,wall.p2);
				width.normalize();
				Vector2	depth(width.perpendicular_ccw());
				
				float w = WidthForSegment(pts[best].edge_type);
				if(w > 1.0)
					anchor += (depth * (w-1.0));
				
				if(want_feature)
				{												
					width *= rule->width_rd * 0.5;
					depth *= rule->depth_rd;
				}
				else {
					anchor -= depth * 0.5 * (rule->x_depth_rd - rule->depth_rd);					
					width *= rule->x_width_rd * 0.5;
					depth *= rule->x_depth_rd;
				}

				if(!want_feature && rule->x_width_rd == 0.0) continue;
				
				push_block_curve(curves, anchor - width, anchor + width, offset + idx);
				push_block_curve(curves, anchor + width, anchor + width + depth, offset + idx);
				push_block_curve(curves, anchor + width + depth, anchor - width + depth, offset + idx);
				push_block_curve(curves, anchor - width + depth, anchor - width, offset + idx);
				
				parts[offset + idx].usage = want_feature ? usage_Polygonal_Feature : usage_OOB;
				parts[offset + idx].feature = rule->fac_id_rd;
				parts[offset + idx].height = f->mParams.count(pf_Height) ? f->mParams.find(pf_Height)->second : 0.0f;
				parts[offset + idx].simplify_id = offset + idx;
			}
			else
			{
				
				Point2		anchor(fl);
				Vector2		width(1,0);
				width.normalize();
				Vector2	depth(width.perpendicular_ccw());
				
				if(want_feature)
				{								
					width *= rule->width_free * 0.5;
					depth *= rule->depth_free;
				}
				else
				{
					anchor -= depth * 0.5 * (rule->x_depth_free - rule->depth_free);					
					width *= rule->x_width_free * 0.5;
					depth *= rule->x_depth_free;
				}

				if(!want_feature && rule->x_width_free == 0.0) continue;
				
				push_block_curve(curves, anchor - width, anchor + width, offset + idx);
				push_block_curve(curves, anchor + width, anchor + width + depth, offset + idx);
				push_block_curve(curves, anchor + width + depth, anchor - width + depth, offset + idx);
				push_block_curve(curves, anchor - width + depth, anchor - width, offset + idx);
				
				parts[offset + idx].usage = want_feature ? usage_Polygonal_Feature : usage_OOB;
				parts[offset + idx].feature = rule->fac_id_free;
				parts[offset + idx].height = f->mParams.count(pf_Height) ? f->mParams.find(pf_Height)->second : 0.0f;
				parts[offset + idx].simplify_id = offset + idx;
			
			}
		}
		
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

inline bool too_damn_small(Block_2::Ccb_halfedge_circulator circ, double len)
{
	len *= len;
	Block_2::Ccb_halfedge_circulator stop(circ);
	do {
		if(Segment2(cgal2ben(circ->source()->point()),cgal2ben(circ->target()->point())).squared_length() > len)
			return false;
	}while(++circ != stop);
	return true;
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
	float height = face->data().GetParam(af_HeightObjs,0);
	ZoningInfoTable::iterator zi = gZoningInfo.find(zoning);
	ZoningInfo_t * info = (zi == gZoningInfo.end() ? NULL : &zi->second);
	int median = face->data().GetParam(af_Median,0);
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

//#if OPENGL_MAP && DEV
//	if(has_road != face->data().GetParam(af_RoadEdge,-1))
//	{
//		printf("Failed.  Thought we had %f, actually had %d.  Zoning %s.\n", 
//				face->data().GetParam(af_RoadEdge,-1),
//				has_road,
//				FetchTokenString(zoning));
//		gFaceSelection.insert(face);
//	}
//#endif
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
	
	// THIS IS THE AUTOGEN BLOCK CASE - WE RUN DOWN THE BLOCK AND DRAW A NICE GRID, GO HOME HAPPY.
	
	if(info && info->fill_area && !median)
	{
		// For now we use our first X road halfedges ...
		DebugAssert(!info->fill_edge);
		FillRule_t * r = GetFillRuleForBlock(face);
		if(r && (r->agb_id != NO_VALUE))
			block_feature_count = init_subdivisions(face, r, translator, outer_ccb_pts, parts, curves);
		if(block_feature_count)
			oob_idx = parts.size() - 1;
	}
	
	// THIS IS THE AD-HOC CASE.  WE PUT IN POINT FEATURE FACADES, AGS, AND ALL SORTS OF OTHER CRAP!
	
	if(block_feature_count == 0)
	{	
		num_he *= 2;	// We need up to two parts (edge, corner) for each road.
	
		int num_extras = 1;
	
		if(info->fill_points)
		num_extras += face->data().mPointFeatures.size() * 2;
	
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
		if(!face->data().mPointFeatures.empty() && !median)
		{
			// First pass: antenna zones around point features...
			init_point_features(face->data().mPointFeatures, curves, parts, outer_ccb_pts, translator, num_he + 1, zoning, false);
			// Second pass: the point features themselves.
			init_point_features(face->data().mPointFeatures, curves, parts, outer_ccb_pts, translator, num_he + 1 + face->data().mPointFeatures.size(), zoning, true);
		}
		int base_offset = block_feature_count;
		{
			init_road_ccb(zoning, variant, height, face->outer_ccb(), translator, parts, curves, outer_ccb_pts, base_offset, num_he+num_extras, cat_base + cat_oob, info && info->fill_edge && !median, ag_ok_approx_dem);
			for(Pmwx::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h)
			{
				vector<block_pt>	hole_pts;
				block_pts_from_ccb(*h,translator, hole_pts,BLOCK_ERR_MTR,true);		
				init_road_ccb(zoning, variant, height, *h, translator, parts, curves, hole_pts, base_offset, num_he+num_extras, cat_base + cat_oob, info && info->fill_edge && !median, ag_ok_approx_dem);
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
//	debug_show_block(out_block,translator);
	
	/* Funky post processing step... */

/*	
	list<Block_2::Halfedge_handle>	splits_we_do_not_want;
	for(Block_2::Edge_iterator e = out_block.edges_begin(); e != out_block.edges_end(); ++e)
	if(!e->face()->is_unbounded() && !e->twin()->face()->is_unbounded())
	{
		Block_2::Face_handle f1(e->face());
		Block_2::Face_handle f2(e->twin()->face());

		if(f1->data().usage == f2->data().usage &&
			f1->data().usage == usage_Polygonal_Feature)
		if(f1->data().simplify_id == f2->data().simplify_id)			
		if(strstr(FetchTokenString(f1->data().feature),".fac") &&
		   strstr(FetchTokenString(f2->data().feature),".fac"))
		if(count_circulator(f1->outer_ccb()) < 4 ||
			count_circulator(f2->outer_ccb()) < 4 ||
			too_damn_small(f1->outer_ccb(),7.5) ||
			too_damn_small(f2->outer_ccb(),7.5))
		{
//			printf("Small side elim: %s\n", FetchTokenString(f1->data().feature));
			splits_we_do_not_want.push_back(e);
//			debug_mesh_line(
//						translator.Reverse(cgal2ben(e->source()->point())),
//						translator.Reverse(cgal2ben(e->target()->point())),
//						1,0,0,1,0,0);
		}
	}
	for(list<Block_2::Halfedge_handle>::iterator k =	splits_we_do_not_want.begin(); k != splits_we_do_not_want.end(); ++k)
		out_block.remove_edge(*k);
	if(!splits_we_do_not_want.empty())
		clean_block(out_block);
*/	
		
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
	if(orig_face->data().GetParam(af_Median,0) == 0.0)
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

	
	// Go find AGS that have been orphaned -- no point to having them.
	for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
	if(!f->is_unbounded())
	if(f->data().usage == usage_Polygonal_Feature)
	{
		if(strstr(FetchTokenString(f->data().feature),".ags"))
		{
			Block_2::Ccb_halfedge_circulator circ, stop;
			bool found_road = false;
			circ = stop = f->outer_ccb();
			Polygon2 bounds;
			do {
				bounds.push_back(cgal2ben(circ->source()->point()));
				if(circ->twin()->face()->data().usage == usage_Road ||
					circ->twin()->face()->data().usage == usage_Road_End)
				{
					found_road = true;
				}
			} while (++circ != stop);
			if(!found_road || bounds.area() < 900)
			{
				f->data().usage = usage_Empty;
			}
		}
		
	}
	
	
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

	// consolidate new forest against old....
	clean_block(block);
	
	return did_promote;
}

Block_2::Halfedge_handle best_side_for_facade(Block_2::Ccb_halfedge_circulator circ, const Vector2& major)
{
	Block_2::Halfedge_handle good = Block_2::Halfedge_handle();
	Block_2::Halfedge_handle best = Block_2::Halfedge_handle();
	
	Block_2::Ccb_halfedge_circulator stop(circ);
	do {
		if(against_road(circ))
		{
			if(good == Block_2::Halfedge_handle() || 
				CGAL::squared_distance(circ->source()->point(),circ->target()->point()) > 
				CGAL::squared_distance(good->source()->point(),good->target()->point()))
				good = circ;			
				
			Vector2	this_side(cgal2ben(circ->source()->point()),cgal2ben(circ->target()->point()));
			this_side.normalize();
			double my_dot = fabs(this_side.dot(major));
			if(my_dot > 0.9)
			{
				if(best == Block_2::Halfedge_handle() || 
					CGAL::squared_distance(circ->source()->point(),circ->target()->point()) > 
					CGAL::squared_distance(best->source()->point(),best->target()->point()))
					best = circ;			
			}
		}
	} while(++circ != stop);

	if(best != Block_2::Halfedge_handle())
		return best;
	else
		return good;
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
	if(bounds.size() > MAX_FOREST_RINGS)
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
		o.mRepType = highest_key(histo);
		if(o.mRepType != NO_VALUE && o.mRepType != DEM_NO_DATA)
			dest_face->data().mPolyObjs.push_back(o);				
	}
	else if(lu_any != NO_VALUE && lu_any != DEM_NO_DATA)
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
					Block_2::Halfedge_handle start = best_side_for_facade(f->outer_ccb(), f->data().major_axis);
					bool fail_start = start == Block_2::Halfedge_handle();
					if(fail_start)
						start = f->outer_ccb();
					o.mParam = intmax2(8,block_height);
					
					PolygonFromBlock(f,start, o.mShape, &translator, 0.0,false);
					double len = sqrt(Segment2(cgal2ben(start->source()->point()),cgal2ben(start->target()->point())).squared_length());
					if(f->data().simplify_id > 0)
					{
						o.mParam = f->data().height;
					}
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
					{
						#if !IGNORE_SHORT_AXIS
							fail_extraction(dest_face,o.mShape,&translator,"SHORT AXIS FAIL: %f\n", dest_face->data().GetParam(af_ShortAxisLength,0.0));
						#endif
					}
					else
					{
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
				if(area > TOO_SMALL_TO_CARE)
				{
					if(f->number_of_holes() < MAX_FOREST_RINGS && area < FOREST_SUBDIVIDE_AREA)
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
								
								{
									data_preserver_t<Block_2>	info(divided_forest);
									CGAL::insert(divided_forest, c);
								}
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
	if(f->data().GetParam(af_Median,0) > 1)
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

// This counts the cost in vertices of polygonal autogen.
	
//	int total = 0;
//	for(GISPolyObjPlacementVector::iterator p = f->data().mPolyObjs.begin(); p != f->data().mPolyObjs.end(); ++p)
//	{
//		for(vector<Polygon2>::iterator r = p->mShape.begin(); r != p->mShape.end(); ++r)
//			total += r->size();
//	}
//	printf("Face had %d vertices.\n", total);
	return ret;
}
