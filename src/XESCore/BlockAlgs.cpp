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

}

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


void clean_block(Block_2& block)
{
	vector<Block_2::Halfedge_handle> kill;
	for(Block_2::Edge_iterator eig = block.edges_begin(); eig != block.edges_end(); ++eig)
		if(eig->face()->data().usage == eig->twin()->face()->data().usage &&
		   eig->face()->data().feature == eig->twin()->face()->data().feature)
			kill.push_back(eig);

//	printf("Before:\n");
//	for(Block_2::Face_handle f = block.faces_begin(); f != block.faces_end(); ++f)
//		printf("%d/%s\n",f->data().usage, FetchTokenString(f->data().feature));

	for(vector<Block_2::Halfedge_handle>::iterator k = kill.begin(); k != kill.end(); ++k)
		block.remove_edge(*k);

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
