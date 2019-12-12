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

#include "BlockAlgs.h"
#include "MapTopology.h"
#include "MapHelpers.h"
#include "STLUtils.h"
#include "NetTables.h"
#include "NetHelpers.h"
#include "CompGeomUtils.h"
#include "GISUtils.h"

#define PTS_LIM 30UL

class check_block_visitor {
public:
	typedef std::pair<Block_2::Halfedge_handle, bool>            Result;

	Block_2::X_monotone_curve_2	cv;
	bool				ok;

	void init (Block_2 *arr)
	{
		ok = true;
	}

	// This is called when we find that some or all of our side goes through the INTERIOR of our face, possibly bounded by some, um, stuff.
	// "face" is the face we go through, and the various v/he is the bounding edge/face if there is one.  In this case, we are going through
	// a face and so the question is: is the face empty?  If not we have crashed through the MIDDLE of a building!
	Result found_subcurve (const Block_2::X_monotone_curve_2& partial,
                         Block_2::Face_handle face,
                         Block_2::Vertex_handle left_v, Block_2::Halfedge_handle left_he,
                         Block_2::Vertex_handle right_v, Block_2::Halfedge_handle right_he)
	{
		if(face->data().usage != usage_Empty)		
			ok = false;
		
		return Result(Block_2::Halfedge_handle(), !ok);		// We halt if we are NOT ok.
	}

	// This is called when we partyl or fully overlap an EXISTING edge.  In this case, we have to figure out which side of the edge is OUR interior
	// and then check THAT face.  We might be EXACTLY shadowing a building, or we might be PERFECTLY next to it.
	Result found_overlap (const Block_2::X_monotone_curve_2& cv,
							Block_2::Halfedge_handle he,
							Block_2::Vertex_handle left_v, Block_2::Vertex_handle right_v)
	{
		DebugAssert(he->direction() == CGAL::ARR_LEFT_TO_RIGHT);
		Block_2::Face_const_handle f = cv.is_directed_right() ? he->face() : he->twin()->face();

		if(f->data().usage != usage_Empty)
		ok = false;
	
		return Result(Block_2::Halfedge_handle(), !ok);
	}

};

class apply_properties_visitor : public MapBFSVisitor<set<int>, Block_2 > {
public:

	typedef	set<int>				Prop_t;
	const vector<BLOCK_face_data> *	feature_map;
	Prop_t							initial;
	
	virtual	void	initialize_properties(Prop_t& io_properties)
	{
		io_properties = initial;
	}
	
	virtual	void	adjust_properties(Block_2::Halfedge_handle edge, Prop_t& io_properties)
	{
		for(EdgeKey_iterator k = edge->curve().data().begin(); k != edge->curve().data().end(); ++k)
		{
			int key = *k;
			if(key >= 0 && key < feature_map->size())
			{
				if(io_properties.count(key))	io_properties.erase(key);
				else							io_properties.insert(key);
			}
		}
	}
	
	virtual	void	mark_face(const Prop_t& in_properties, Block_2::Face_handle face)
	{
		if(!in_properties.empty())
		{
//			printf("Tagging face with %d props.\n", in_properties.size());
//			for(Prop_t::const_iterator pp = in_properties.begin(); pp != in_properties.end(); ++pp)
//				printf("     %d (%d/%s)\n",  *pp, (*feature_map)[*pp].usage,FetchTokenString((*feature_map)[*pp].feature));
			face->set_data((*feature_map)[*(--in_properties.end())]);
		} 
		else
		{
			face->data().usage = usage_Empty;
			face->data().feature = NO_VALUE;
		}	
	}
};

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_data_structure_2.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Filtered_kernel.h>

typedef CGAL::Filtered_kernel< CGAL::Simple_cartesian<double> >	HackKernel;
//typedef FastKernel HackKernel;
typedef CGAL::Triangulation_data_structure_2 <
						CGAL::Triangulation_vertex_base_2<HackKernel>,
						CGAL::Constrained_triangulation_face_base_2<HackKernel> >	FastCDTDS;
typedef CGAL::Constrained_Delaunay_triangulation_2<
	HackKernel,
	FastCDTDS,
	CGAL::Exact_predicates_tag
	>		
FastCDT;

inline HackKernel::Point_2 C(const FastKernel::Point_2& p)
{
	return HackKernel::Point_2(CGAL::to_double(p.x()),CGAL::to_double(p.y()));
}

// We build a block from scratch with a bunch of polygons.  Note that where polygons overlap, the earliest one in the block will "win" - that is, unneeded halfedges
// WILL exist, but the "tags" will match the lower prio blokc data.
void	create_block(
					Block_2&									block,
					const vector<BLOCK_face_data>&				in_data,
					const vector<Block_2::X_monotone_curve_2>&	in_bounds,
					int											unbounded_idx)
{
	block.clear();
	
	#if DEV
		for(int n = 0; n < in_data.size(); ++n)
			if(in_data[n].usage == usage_Polygonal_Feature)
				DebugAssert(in_data[n].feature != 0);
	#endif
	
	// First we are going to build up a curve list and bulk insert them all.  Each curve has the polygon number as its data.
	// This should be faster than doing a series of piece-wise inserts.

#if 0
	vector<Block_2::X_monotone_curve_2>	keep;
	set<pair<Point_2,Point_2> >	we_have;
	for(int n = 0; n < in_bounds.size(); ++n)
	{
		pair<Point_2,Point_2>	s(in_bounds[n].source(),in_bounds[n].target());
		if(s.first < s.second) swap(s.first,s.second);
		if(we_have.count(s) == 0)
		{
			keep.push_back(in_bounds[n]);
			we_have.insert(s);
		}
	}

	CGAL::insert(block, keep.begin(), keep.end());
#endif	

	CGAL::insert(block, in_bounds.begin(), in_bounds.end());

#if 0
	FastCDT	cdt;
	FastCDT::Face_handle hint;
	vector<pair<FastCDT::Vertex_handle, FastCDT::Vertex_handle> >	vv;
	
	set<pair<FastCDT::Vertex_handle, FastCDT::Vertex_handle> > we_have;
	
	for(int n = 0; n < in_bounds.size(); ++n)
	{
		pair<FastCDT::Vertex_handle,FastCDT::Vertex_handle>	r;
		r.first = cdt.insert(C(in_bounds[n].source()),hint);
		hint = r.first->face();
		r.second = cdt.insert(C(in_bounds[n].target()),hint);
		hint = r.second->face();
		if(r.first < r.second)	swap(r.first,r.second);
		if(we_have.count(r) == 0)
		{
			we_have.insert(r);
			vv.push_back(r);
		}	
	}
	for(int n = 0; n < vv.size(); ++n)
	{
		DebugAssert(vv[n].first != vv[n].second);
		cdt.insert_constraint(vv[n].first,vv[n].second);
	}
#endif	

	// Now we go back and do a search from the outside in, toggling our "membership" each time we cross a bounding edge, to keep track of
	// which face we are in.

	for(Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
			f->set_visited(false);

	apply_properties_visitor	visitor;
	visitor.feature_map = &in_data;
	// What is this?  The FIRST contour is considered the contour that tags the unbounded polygon.  So we "start" with this polygon in effect.
	if(unbounded_idx != -1)
		visitor.initial.insert(unbounded_idx);
	
	visitor.Visit(&block);

	#if DEV
	for (Block_2::Face_iterator f = block.faces_begin(); f != block.faces_end(); ++f)
	if(f->data().usage == usage_Polygonal_Feature)
				DebugAssert(f->data().feature != 0);
	#endif

}

#if 0

// This routine checks whether a poylgon can be inserted into a block such that the interior of the polygon does not at all overlap with used space
// in the block.  We COULD do this using boolean operations (E.g. is the intersection of the used block space and our polygon empty?) but we do NOT want
// to.  Why?  Time to do a boolean op is a function of BOTH polygons, and in a lot of cases the block will be HUGELY complex (imagine a giant forest expanse
// with lots of crap already inserted into it) and our test block will be TINY (a quad).  In this case, we get better performance by doing a piece-wise per-side
// test.  
// Side note: using the bulk locator totally violates this principle...we will probably need to replace it with a walk location strategy.
// So we use the zone utility to see what we'll hit as we go along our edges.  
bool	can_insert_into_block(
					Block_2&						block,
					const Polygon_2&				bounds)
{
	// Run a bulk location on the entire block...TBD: there may be blocks where sweeping is worse than marching.
	// Anyway, we get a bunch of pairs of the locate point and the actual part of the arrangement we hit.
	vector<pair<Point_2, CGAL::Object> >	pts;
	CGAL::locate(block, bounds.vertices_begin(), bounds.vertices_end(), back_inserter(pts));
	
	// Quick check: if ANY of our points are inside a non empty face, we are by definition hosed.
	// We can see this now just from the location data.
	Block_2::Face_const_handle ff;
	for(int n = 0; n < pts.size(); ++n)
	if(CGAL::assign(ff,pts[n].second))
	if(ff->data().usage != usage_Empty)
		return false;

	for(int n = 0; n < pts.size(); ++n)
	{
		// We are now going to go through each side and do a zone test.  This will check whether it is 
		// either running along a full area or crashing through it.
		int m = (n+1)%pts.size();
		check_block_visitor	v;
		v.cv = Block_2::X_monotone_curve_2(Segment_2(pts[n].first, pts[m].first));

		CGAL::Arrangement_zone_2<Block_2,check_block_visitor>	zone(block, &v);

		if(v.cv.is_directed_right())
			zone.init_with_hint(v.cv,pts[n].second);
		else
			zone.init_with_hint(v.cv,pts[m].second);

		zone.compute_zone();
		
		if(!v.ok)
			return false;		
	}
	return true;
}

static void	CollectEdges(Block_2& io_dst, edge_collector_t<Block_2> * collector, const Polygon_2& src, Block_locator * loc)
{
	DebugAssert(src.size() >= 3);
	DebugAssert(src.is_simple());
	for(int n = 0; n < src.size(); ++n)
	{
		collector->input = Pmwx::X_monotone_curve_2(src.edge(n),0);
		collector->ctr = 0;
		DebugAssert(collector->input.source() != collector->input.target());
		if(loc)			CGAL::insert(io_dst, collector->input,*loc);
		else			CGAL::insert(io_dst, collector->input);
		DebugAssert(collector->ctr > 0);
	}
}


void	do_insert_into_block(
					Block_2&						block,
					const Polygon_2&				bounds,
					const BLOCK_face_data&			data)
{
	edge_collector_t<Block_2>	collector;
	collector.attach(block);

	CollectEdges(block, &collector, bounds, NULL);

	set<Block_2::Face_handle>	faces;
	FindFacesForEdgeSet<Block_2>(collector.results, faces);

	for(set<Block_2::Face_handle>::iterator f = faces.begin(); f != faces.end(); ++f)
		(*f)->set_data(data);
}					

#endif

void clean_block(Block_2& block, bool merge_faces, bool merge_colinear)
{
	vector<Block_2::Halfedge_handle> kill;
	if(merge_faces)
	for(Block_2::Edge_iterator eig = block.edges_begin(); eig != block.edges_end(); ++eig)
		if(eig->face()->data().usage == eig->twin()->face()->data().usage &&
		   eig->face()->data().feature == eig->twin()->face()->data().feature &&
		   eig->face()->data().simplify_id >= 0 &&
		   eig->face()->data().simplify_id == eig->twin()->face()->data().simplify_id)
	{
			kill.push_back(eig);
	}
//	printf("Before:\n");
//	for(Block_2::Face_handle f = block.faces_begin(); f != block.faces_end(); ++f)
//		printf("%d/%s\n",f->data().usage, FetchTokenString(f->data().feature));

	for(vector<Block_2::Halfedge_handle>::iterator k = kill.begin(); k != kill.end(); ++k)
		block.remove_edge(*k);

	if(merge_colinear)
	for(Block_2::Vertex_iterator v = block.vertices_begin(); v != block.vertices_end();)
	{
		Block_2::Vertex_handle k(v);
		++v;
		if(k->degree() == 2)
		{
			Block_2::Halfedge_handle e1 = k->incident_halfedges();
			Block_2::Halfedge_handle e2 = e1->next();
			DebugAssert(e1->target() == k);
			DebugAssert(e2->source() == k);
			
			if(e1->data() == e2->data() && CGAL::collinear(e1->source()->point(),e1->target()->point(),e2->target()->point()))
			{
				Block_2::X_monotone_curve_2	nc(Block_traits_2::Segment_2(e1->source()->point(),e2->target()->point()));
				block.merge_edge(e1,e2,nc);
			}
		}
	}
	

//	printf("After:\n");
//	for(Block_2::Face_handle f = block.faces_begin(); f != block.faces_end(); ++f)
//		printf("%d/%s\n",f->data().usage, FetchTokenString(f->data().feature));
}

/********************************************************************************************************************************************
 *
 ********************************************************************************************************************************************/

struct traits {
	void remove(Block_2::Vertex_handle v) const {} 
	bool is_locked(Block_2::Vertex_handle v) const
	{
		DebugAssert(v->degree() == 2);
		Block_2::Halfedge_handle he = v->incident_halfedges();
		return he->face()->is_unbounded() || he->twin()->face()->is_unbounded();
	}
};



void simplify_block(Block_2& io_block, double max_err)
{
	arrangement_simplifier<Block_2, traits> simplifier;
	traits tr;
	simplifier.simplify(io_block, max_err, tr);
}


// This routine tries to find the 'grain' of a block by a weighting of dot 
// products relative to all sides.  A few notes:
// 1. It tries to first run wiht a handy-cap to take only street-level roads.
//    It then tries again if the filter is too harsh.
// 2. It does an intentionally bad job when there are a huge number of sides
//    since such high-count polygons aren't sane AGB candidates.
// 3. It ALWAYS returns some answer, no matter how goofy the polygon.
void find_major_axis(vector<block_pt>&	pts,
				Segment2 *			out_segment,
				Vector2 *			out_major,
				Vector2 *			out_minor,
				double *			bounds)
{
	double best_v = -1.0;
	Vector2	temp_a, temp_b;
	double bounds_temp[4];
	
	if(out_major == NULL) out_major = &temp_a;
	if(out_minor == NULL) out_minor = &temp_b;
	if(bounds == NULL) bounds = bounds_temp;

	bool elev_ok = false;
		
	for(int tries = 0; tries < 2; ++tries)
	{	
		
		for(int i = 0; i < pts.size(); ++i)
		if(elev_ok || ground_road_access_for_he(pts[i].orig))
		{
			int j = (i + 1) % pts.size();
			Vector2	v_a(pts[i].loc,pts[j].loc);
			v_a.normalize();
			Vector2 v_b(v_a.perpendicular_ccw());
			
			double total = 0.0;
			int max_k = min(pts.size(),PTS_LIM);
			for(int k = 0; k < max_k; ++k)
			{
				int l = (k + 1) % pts.size();
				Vector2	s(pts[k].loc,pts[l].loc);
				
				total += max(fabs(v_a.dot(s)),fabs(v_b.dot(s)));			
			}
			
			if(total >= best_v)
			{
				best_v = total;
				if(out_segment) *out_segment = Segment2(pts[i].loc,pts[j].loc);
				*out_major = v_a;
				*out_minor = v_b;			
			}	
		}
		
		if(best_v != -1)
			break;
		elev_ok = true;
	}
	
	
	{
		int longest = -1;
		double corr_len = -1;
		for(int i = 0; i < pts.size(); ++i)
		if(/*elev_ok ||*/ ground_road_access_for_he(pts[i].orig))
		{
			int j = (i + 1) % pts.size();
			Vector2 this_side(pts[i].loc,pts[j].loc);
			double len = this_side.normalize();
			double my_corr = fltmax2(fabs(this_side.dot(*out_major)), fabs(this_side.dot(*out_minor)));
			if(my_corr > 0.996194698091746)
			{
				my_corr *= len;
				if(my_corr > corr_len)
				{
					longest = i;
					corr_len = my_corr;
				}			
			}
		}
		if(longest >= 0)
		{
			int i = longest;
			int j = (longest + 1) % pts.size();
			Vector2	v_a(pts[i].loc,pts[j].loc);
			v_a.normalize();
			Vector2 v_b(v_a.perpendicular_ccw());

			if(out_segment) *out_segment = Segment2(pts[i].loc,pts[j].loc);
			*out_major = v_a;
			*out_minor = v_b;			
		}
	}
	
	
	
	
	bounds[2] = bounds[0] = out_major->dot(Vector2(pts[0].loc));
	bounds[3] = bounds[1] = out_minor->dot(Vector2(pts[0].loc));
	for(int n = 1; n < pts.size(); ++n)
	{
		double ca = out_major->dot(Vector2(pts[n].loc));
		double cb = out_minor->dot(Vector2(pts[n].loc));
		bounds[0] = min(bounds[0],ca);
		bounds[1] = min(bounds[1],cb);
		bounds[2] = max(bounds[2],ca);
		bounds[3] = max(bounds[3],cb);
	}
	
//	if(fabs(bounds[3] - bounds[1]) > fabs(bounds[2] - bounds[0]))
//	{
//		*out_major = out_major->perpendicular_ccw();
//		*out_minor = out_minor->perpendicular_ccw();
//
//		bounds[2] = bounds[0] = out_major->dot(Vector2(pts[0].loc));
//		bounds[3] = bounds[1] = out_minor->dot(Vector2(pts[0].loc));
//		for(int n = 1; n < pts.size(); ++n)
//		{
//			double ca = out_major->dot(Vector2(pts[n].loc));
//			double cb = out_minor->dot(Vector2(pts[n].loc));
//			bounds[0] = min(bounds[0],ca);
//			bounds[1] = min(bounds[1],cb);
//			bounds[2] = max(bounds[2],ca);
//			bounds[3] = max(bounds[3],cb);
//		}
//
//	}
}

bool	sides_can_merge(Pmwx::Halfedge_handle e1, Pmwx::Halfedge_handle e2)
{
	if(width_for_he(e1) != width_for_he(e2))	
		return false;
	if(ground_road_access_for_he(e1) != ground_road_access_for_he(e2))	
		return false;
	return true;
}

bool	within_err_metric(const Point_2& p1, const Point_2& p2, const Point_2& p3, const CoordTranslator2& trans, double max_err_sq)
{
	// If this happens, (1) we have an antenna...
	if(p1 == p3)
		return false;
	
	Segment2	s(
		trans.Forward(cgal2ben(p1)),
		trans.Forward(cgal2ben(p3)));
	Point2	t(trans.Forward(cgal2ben(p2)));
	
	return (s.squared_distance_supporting_line(t) < max_err_sq);
}

bool	within_err_metric(Pmwx::Halfedge_handle h1, Pmwx::Halfedge_handle h2, const CoordTranslator2& trans, double max_err_sq)
{
	Point_2	p1(h1->source()->point());
	Point_2	p2(h2->target()->point());
	
	while(h1 != h2)
	{
		if(!within_err_metric(p1,h1->target()->point(),p2,trans,max_err_sq))
			return false;
		h1 = h1->next();	
	}
	return true;
	
}

bool	build_convex_polygon(
				Pmwx::Ccb_halfedge_circulator									ccb,
				vector<pair<Pmwx::Halfedge_handle, Pmwx::Halfedge_handle> >&	sides,
				const CoordTranslator2&											trans,
				Polygon2&														metric_bounds,
				double															max_err_mtrs,
				double															min_side_len)
{
	double	e_sq = max_err_mtrs*max_err_mtrs;
	sides.clear();
	metric_bounds.clear();
	
	Pmwx::Ccb_halfedge_circulator circ(ccb);
//	Bbox2				bounds;
//	
//	do {
//		bounds += cgal2ben(circ->source()->point());
//	} while (++circ != ccb);

	Pmwx::Ccb_halfedge_circulator start,next;
	start = ccb;
	do {
		--start;
		if(!sides_can_merge(start,ccb))
			break;
		if(!within_err_metric(start,ccb,trans,e_sq))
			break;		
	} while(start != ccb);
	++start;
	
	// now we can go around.

	circ = start;
	//int ne = count_circulator(start);
//	printf("Poly has %d sides.\n", ne);
	do {
	 
		Pmwx::Ccb_halfedge_circulator stop(circ);
		do {
			++stop;
		} while(sides_can_merge(circ,stop) && within_err_metric(circ,stop,trans,e_sq) && stop != start);
	
		--stop;
//		printf("Pushing side of %d, %d\n", circulator_distance_to(start, circ),circulator_distance_to(start,stop));
		sides.push_back(pair<Pmwx::Halfedge_handle,Pmwx::Halfedge_handle>(circ, stop));
		++stop;
		circ = stop;
	
	} while(circ != start);
	
	if(sides.size() < 3)	
	{
		//debug_mesh_point(bounds.centroid(),1,1,1);
		return false;
	}
	
	int i, j, k;
	
	vector<Segment2>	msides;
	for(i = 0; i < sides.size(); ++i)
	{
		j = (i + 1) % sides.size();		
		DebugAssert(sides[i].second->target() == sides[j].first->source());
		msides.push_back(Segment2(
						trans.Forward(cgal2ben(sides[i].first->source()->point())),
						trans.Forward(cgal2ben(sides[i].second->target()->point()))));						
	}	
	vector<Segment2>	debug(msides);
	for(i = 0; i < sides.size(); ++i)
	{
		j = (i + 1) % sides.size();		
		Vector2	v1(msides[i].p1,msides[i].p2);
		Vector2	v2(msides[j].p1,msides[j].p2);
		v1.normalize();
		v2.normalize();
		if(v1.dot(v2) >  0.9998 ||
		   v1.dot(v2) < -0.9998 ||
			!v1.left_turn(v2))
		{
			//debug_mesh_point(trans.Reverse(msides[i].p2),1,0,0);
			return false;
		}
		double w = width_for_he(sides[i].first);
		if(w)
		{
			v1 = v1.perpendicular_ccw();
			v1 *= w;
			msides[i].p1 += v1;
			msides[i].p2 += v1;
		}
	}
	for(j = 0; j < sides.size(); ++j)
	{
		i = (j + sides.size() - 1) % sides.size();
		Line2 li(msides[i]), lj(msides[j]);
		Point2	p;
		if(!li.intersect(lj,p))
		{
			/*
			for(int k = 0; k < sides.size(); ++k)
			{
				Segment2 d(trans.Reverse(msides[k].p1), trans.Reverse(msides[k].p2));
				if(k == j)
				debug_mesh_line(d.p1, d.p2, 0.5,0.5,0.0, 1,1,0);
				else if(k == i)
				debug_mesh_line(d.p1, d.p2, 0.5,0.0,0.0, 1.0,0,0);
				else
				debug_mesh_line(d.p1, d.p2, 0.5,0.5,0.5, 0.5,0,0);
			}
			*/
		
			Assert(!"Failure to intersect.\n");
			return false;
		}
		metric_bounds.push_back(p);
	}
	
	for(i = 0; i < metric_bounds.size(); ++i)
	{
		j = (i + 1) % metric_bounds.size();
		k = (i + 2) % metric_bounds.size();
		if(metric_bounds.side(i).squared_length() < (min_side_len*min_side_len))
		{
			//debug_mesh_line(trans.Reverse(metric_bounds.side(i).p1),trans.Reverse(metric_bounds.side(i).p2),1,1,0,1,1,0);
			return false;
		}
		if(!left_turn(metric_bounds[i],metric_bounds[j],metric_bounds[k]))
		{
			//debug_mesh_point(trans.Reverse(metric_bounds[j]),1,1,0);
			return false;
		}
		if(Vector2(msides[i].p1,msides[i].p2).dot(Vector2(metric_bounds[i],metric_bounds[j])) < 0.0)
		{
			//debug_mesh_line(trans.Reverse(msides[i].p1),trans.Reverse(msides[i].p2),1,0,0,1,0,0);
			return false;
		}
	}
	DebugAssert(metric_bounds.size() == msides.size());
	DebugAssert(msides.size() == sides.size());
		
	return true;
}

int	pick_major_axis(
				vector<pair<Pmwx::Halfedge_handle, Pmwx::Halfedge_handle> >&	sides,				// Per side: inclusive range of half-edges "consolidated" into the sides.
				Polygon2&														bounds,			// Inset boundary in metric, first side matched to the list.
				Vector2&														v_x,
				Vector2&														v_y)
{
	// special case: if we find a block with exactly ONE right angle, the longer of the two
	// sides going into the right angle is hte major axis, full stop, we're done.
	int right_angle = -1;
	for(int i = 0; i < sides.size(); ++i)
	{
		int j = (i + 1) % sides.size();
		int k = (i + 2) % sides.size();
		Vector2	vx(Vector2(bounds[i],bounds[j]));
		Vector2	vy(Vector2(bounds[j],bounds[k]));
		vx.normalize();
		vy.normalize();
		double dot = fabs(vx.dot(vy));
		if(dot < 0.087155742747658)
		{
			if(right_angle == -1)
				right_angle = j;
			else
				right_angle = -2;		// "more than one right angle" flag - causes us to NOT try this algo.
		}
	}
	if(right_angle >= 0)
	{
		int prev = (right_angle + sides.size() - 1) % sides.size();
		int next = (right_angle +				 1) % sides.size();
		Vector2	vp(Vector2(bounds[prev],bounds[right_angle]));
		Vector2	vn(Vector2(bounds[right_angle],bounds[next]));
		double pl = vp.normalize();
		double nl = vn.normalize();
		if(pl > nl)
			v_x = vp;	
		else
			v_x = vn;
		v_y = v_x.perpendicular_ccw();
		
		return right_angle;
	}

	// THIS is the algo we shipped with - it tries to minimize the short side axis of the block.  This works
	// okay but tends to make the diagonal of diagonal cuts (like Broadway) the main axis since (by the pythag
	// theorem) that slightly reduces the block depth.

#if 0
	int shortest = -1;
	double	thinnest_so_far = 0.0;
	for(int i = 0; i < sides.size(); ++i)
	{
		Vector2 vx = Vector2(bounds.side(i).p1,bounds.side(i).p2);
		vx.normalize();
		Vector2 vy = vx.perpendicular_ccw();
		double bbox[4];
		bbox[0] = bbox[2] = vx.dot(Vector2(bounds[0]));
		bbox[1] = bbox[3] = vy.dot(Vector2(bounds[0]));
		for(int j = 0; j < sides.size(); ++j)
		{
			double x = vx.dot(Vector2(bounds[j]));
			double y = vy.dot(Vector2(bounds[j]));
			bbox[0] = dobmin2(bbox[0], x);
			bbox[1] = dobmin2(bbox[1], y);
			bbox[2] = dobmax2(bbox[2], x);
			bbox[3] = dobmax2(bbox[3], y);
		}
		double xdist = fabs(bbox[2]-bbox[0]);
		double ydist = fabs(bbox[3]-bbox[1]);
		double my_dist = dobmin2(xdist,ydist);
		if(shortest == -1 || my_dist < thinnest_so_far)
		{
			shortest = i;
			thinnest_so_far = my_dist;
			if(xdist < ydist)
			{
				v_x = vx.perpendicular_ccw();
				v_y = vy.perpendicular_ccw();
			}
			else
			{
				v_x = vx;
				v_y = vy;
			}
		}
	}
	
	DebugAssert(shortest >= 0);
	return shortest;
#endif

#if 1

//	#error This algo works 95% of the time, but 5% of the time it picks a slashed short end as the
//	#error long axis, which gives a long thin block a wrong axis alignment and a huge AABB.  Bad!


	// The basic idea: we want to pick the grid axis MOST aligned with the block such that
	// the major axis supports roads.  

	double best_corr = 0;
	double best = -1;
	Vector2	best_vec;

	bool elev_ok = false;

	int i, j, tries;
	for(tries = 0; tries < 2; ++tries)	
	{
		for(i = 0; i < sides.size(); ++i)
		if(elev_ok || ground_road_access_for_he(sides[i].first))	
		{
			double score = 0.0;
			Vector2 si = Vector2(bounds.side(i).p1,bounds.side(i).p2);
			si.normalize();
			Vector2 si_n = si.perpendicular_ccw();
			
			for(int j = 0; j < sides.size(); ++j)
			if(elev_ok || ground_road_access_for_he(sides[j].first))	
			{
				Vector2	sj(bounds.side(j).p1,bounds.side(j).p2);
				double my_corr = fltmax2(fabs(si.dot(sj)),fabs(si_n.dot(sj)));

				score += my_corr;
			}

			if(score > best_corr){
				best = i;
				best_corr = score;
				best_vec = si;
			}
		}
		
		if(best >= 0)
			break;
		elev_ok = true;
	}

	if(best >= 0)
	{
		Vector2	best_vec_n = best_vec.perpendicular_ccw();
		
		int longest = -1;
		double corr_len = -1;
		for(int i = 0; i < sides.size(); ++i)
		if(/*elev_ok ||*/ ground_road_access_for_he(sides[i].first))
		{
			Vector2 this_side(bounds.side(i).p1,bounds.side(i).p2);
			double len = this_side.normalize();
			double my_corr = fltmax2(fabs(best_vec.dot(this_side)), fabs(best_vec_n.dot(this_side)));
			if(my_corr > 0.996194698091746)
			{
				my_corr *= len;
				if(my_corr > corr_len)
				{
					longest = i;
					corr_len = my_corr;
				}			
			}
		}
		if(longest >= 0)
			best = longest;
	}
		
	v_x = Vector2(bounds.side(best).p1,bounds.side(best).p2);
	v_x.normalize();
	v_y = v_x.perpendicular_ccw();

	//printf("So far our best is %d, with axes %lf, %lf to %lf, %lf\n", best, v_x.dx,v_x.dy,v_y.dx,v_y.dy);
					

	double bbox[4];
	bbox[0] = bbox[2] = v_x.dot(Vector2(bounds[0]));
	bbox[1] = bbox[3] = v_y.dot(Vector2(bounds[0]));
	for(i = 1; i < sides.size(); ++i)
	{
		double va = v_x.dot(Vector2(bounds[i]));
		double vb = v_y.dot(Vector2(bounds[i]));
		bbox[0]=dobmin2(bbox[0], va);
		bbox[1]=dobmin2(bbox[1], vb);
		bbox[2]=dobmax2(bbox[2], va);
		bbox[3]=dobmax2(bbox[3], vb);
	}
	
	//printf("Our bbox is %lf,%lf to %lf,%lf\n", bbox[0],bbox[1],bbox[2],bbox[3]);

	if(0)
	if((bbox[2] - bbox[0]) < (bbox[3] - bbox[1]))
	{
		v_x = v_x.perpendicular_ccw();
		v_y = v_y.perpendicular_ccw();
		//printf("Must rotate, the winner was a SHORT side.\n");
	}

//	best = 0;
//	double best_dot = 0;
//	for(i = 0; i < sides.size(); ++i)
//	if(ground_road_access_for_he(sides[i].first))
//	{
//		Vector2	side_vec(bounds.side(i).p1,bounds.side(i).p2);
//		double slen = side_vec.normalize();
//		double corr = fabs(v_x.dot(side_vec));
//		if(corr > best_dot)
//		{
//			best = i;
//			corr = best_dot;
//		}
//	}
//	DebugAssert(best >= 0.0);
//	v_x = Vector2(bounds.side(best).p1,bounds.side(best).p2);
//	v_x.normalize();
//	v_y = v_x.perpendicular_ccw();
	return best;
#endif	
}
