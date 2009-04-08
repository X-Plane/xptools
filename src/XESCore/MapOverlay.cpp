/* 
 * Copyright (c) 2008, Laminar Research.
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

#include "MapOverlay.h"
#include "MapAlgs.h"
#include "MapTopology.h"
#include "MapHelpers.h"

/******************************************************************************************************************************************************
 * OVERLAY HELPERS
 ******************************************************************************************************************************************************/

/*
	This is a full overlay helper that attempts to maintain the meta-data attached to our map through the merge.  It takes merge-functors as template
	parameters.

*/


template <class ArrangementA, class ArrangementB, class ArrangementR,
class OverlayVertexData_,
class OverlayEdgeData_,
class OverlayFaceData_>
class Arr_full_overlay_traits :
public CGAL::_Arr_default_overlay_traits<ArrangementA, ArrangementB, ArrangementR> 
{
public:
	
	typedef typename ArrangementA::Face_const_handle    Face_handle_A;
	typedef typename ArrangementB::Face_const_handle    Face_handle_B;
	typedef typename ArrangementR::Face_handle          Face_handle_R;

	typedef typename ArrangementA::Halfedge_const_handle  Halfedge_handle_A;
	typedef typename ArrangementB::Halfedge_const_handle  Halfedge_handle_B;
	typedef typename ArrangementR::Halfedge_handle        Halfedge_handle_R;	

	typedef typename ArrangementA::Vertex_const_handle  Vertex_handle_A;
	typedef typename ArrangementB::Vertex_const_handle  Vertex_handle_B;
	typedef typename ArrangementR::Vertex_handle        Vertex_handle_R;	

	
	typedef OverlayEdgeData_                            Overlay_edge_data;
	typedef OverlayFaceData_                            Overlay_face_data;
	typedef OverlayVertexData_                            Overlay_vertex_data;
	
private:
	
	Overlay_edge_data         overlay_edge_data;
	Overlay_face_data         overlay_face_data;
	Overlay_vertex_data       overlay_vertex_data;
	
public:

	virtual void create_vertex (Vertex_handle_A v1, Vertex_handle_B v2, Vertex_handle_R v) const
	{
		v->set_data(overlay_vertex_data(v1->data(),v2->data()));
	}
	virtual void create_vertex (Vertex_handle_A v1, Halfedge_handle_B e2, Vertex_handle_R v) const
	{
		v->set_data(v1->data());
	}
	virtual void create_vertex (Vertex_handle_A v1, Face_handle_B f2, Vertex_handle_R v) const
	{
		v->set_data(v1->data());
	}
	virtual void create_vertex (Halfedge_handle_A e1, Vertex_handle_B v2, Vertex_handle_R v) const
	{
		v->set_data(v2->data());
	}
	virtual void create_vertex (Face_handle_A f1, Vertex_handle_B v2, Vertex_handle_R v) const
	{
		v->set_data(v2->data());
	}
	virtual void create_vertex (Halfedge_handle_A /* e1 */, Halfedge_handle_B /* e2 */, Vertex_handle_R /* v */) const
	{
	}

	virtual void create_edge (Halfedge_handle_A e1, Halfedge_handle_B e2, Halfedge_handle_R e) const
	{
		e->		   set_data (overlay_edge_data (e1->		data(), e2->		data()));
		e->twin()->set_data (overlay_edge_data (e1->twin()->data(), e2->twin()->data()));
	}
	virtual void create_edge (Halfedge_handle_A e1, Face_handle_B f2, Halfedge_handle_R e) const
	{
		e->set_data (e1->data());
		e->twin()->set_data (e1->twin()->data());
	}
	virtual void create_edge (Face_handle_A f1, Halfedge_handle_B e2, Halfedge_handle_R e) const
	{
		e->set_data (e2->data());
		e->twin()->set_data (e2->twin()->data());
	}
	
	virtual void create_face (Face_handle_A f1, Face_handle_B f2, Face_handle_R f) const
	{
		f->set_contained(!f2->is_unbounded());
		if(f1->is_unbounded())			f->set_data(f2->data());				// If one face is unbounded, use the other...
		else if (f2->is_unbounded())	f->set_data(f1->data());				// The unbounded face cannot contain featuers and land uses in our model!!
		else
			f->set_data (overlay_face_data (f1->data(), f2->data()));
	}
	
};



template <class ArrangementA, class ArrangementB, class ArrangementR>
class Arr_replace_overlay_traits :
public CGAL::_Arr_default_overlay_traits<ArrangementA, ArrangementB, ArrangementR> 
{
public:
	
	typedef typename ArrangementA::Face_const_handle    Face_handle_A;
	typedef typename ArrangementB::Face_const_handle    Face_handle_B;
	typedef typename ArrangementR::Face_handle          Face_handle_R;

	typedef typename ArrangementA::Halfedge_const_handle  Halfedge_handle_A;
	typedef typename ArrangementB::Halfedge_const_handle  Halfedge_handle_B;
	typedef typename ArrangementR::Halfedge_handle        Halfedge_handle_R;	

	typedef typename ArrangementA::Vertex_const_handle  Vertex_handle_A;
	typedef typename ArrangementB::Vertex_const_handle  Vertex_handle_B;
	typedef typename ArrangementR::Vertex_handle        Vertex_handle_R;	
	
public:

	vector<Halfedge_handle_R> *		dead;

	virtual void create_vertex (Vertex_handle_A v1, Vertex_handle_B v2, Vertex_handle_R v) const
	{
		v->set_data(v2->data());
	}
	virtual void create_vertex (Vertex_handle_A v1, Halfedge_handle_B e2, Vertex_handle_R v) const
	{
	}
	virtual void create_vertex (Vertex_handle_A v1, Face_handle_B f2, Vertex_handle_R v) const
	{
		if(f2->is_unbounded())
			v->set_data(v1->data());
	}
	virtual void create_vertex (Halfedge_handle_A e1, Vertex_handle_B v2, Vertex_handle_R v) const
	{
		v->set_data(v2->data());
	}
	virtual void create_vertex (Face_handle_A f1, Vertex_handle_B v2, Vertex_handle_R v) const
	{
		v->set_data(v2->data());
	}
	virtual void create_vertex (Halfedge_handle_A e1, Halfedge_handle_B e2, Vertex_handle_R v) const
	{
	}

	virtual void create_edge (Halfedge_handle_A e1, Halfedge_handle_B e2, Halfedge_handle_R e) const
	{
		e->		   set_data (e2->data());
		e->twin()->set_data (e2->twin()->data());
	}
	virtual void create_edge (Halfedge_handle_A e1, Face_handle_B f2, Halfedge_handle_R e) const
	{
		if(f2->is_unbounded())
		{
			e->set_data (e1->data());
			e->twin()->set_data (e1->twin()->data());
		} else if(dead)
			dead->push_back(e);
	}
	virtual void create_edge (Face_handle_A f1, Halfedge_handle_B e2, Halfedge_handle_R e) const
	{
		e->set_data (e2->data());
		e->twin()->set_data (e2->twin()->data());
	}
	
	virtual void create_face (Face_handle_A f1, Face_handle_B f2, Face_handle_R f) const
	{
		f->set_contained(!f2->is_unbounded());
		f->set_data (f2->is_unbounded() ? f1->data() : f2->data());
	}
	
};







struct Overlay_vertex
{
	GIS_vertex_data operator()(GIS_vertex_data a, GIS_vertex_data b) const 
	{
		GIS_vertex_data r;
		r.mTunnelPortal = a.mTunnelPortal || b.mTunnelPortal;
		return r;
	}
};

struct Overlay_terrain
{
	GIS_face_data operator() (GIS_face_data a, GIS_face_data b) const
	{
		GIS_face_data r;
		//fprintf(stderr, "%d-%d ", a.mTerrainType, b.mTerrainType);
		// Our overlay comes from the RHS, but it might be a hole (in which case mTerrainType will be 0)
		if (b.mTerrainType != 0 ) {
			r.mTerrainType = b.mTerrainType;
			return r;
		}
		if (a.mTerrainType != 0 ) {
			r.mTerrainType = a.mTerrainType;
			return r;
		}
		if(b.mAreaFeature.mFeatType != NO_VALUE)
			r.mAreaFeature = b.mAreaFeature;
		else
			r.mAreaFeature = a.mAreaFeature;
		return r;
	}
};

struct Overlay_network
{
	GIS_halfedge_data operator() (GIS_halfedge_data a, GIS_halfedge_data b) const
	{
//		DebugAssert(a.mDominant == b.mDominant);
//		DebugAssert(a.mDominant || a.mDominant == b.mDominant);
//		DebugAssert(!b.mDominant || a.mDominant == b.mDominant);
		GIS_halfedge_data r;
//		r.mDominant = a.mDominant;
		GISNetworkSegmentVector::iterator i;
		if (b.mSegments.empty())
			for (i = a.mSegments.begin(); i != a.mSegments.end(); ++i)
				r.mSegments.push_back(*i);
//		if(b.mDominant)
			for (i = b.mSegments.begin(); i != b.mSegments.end(); ++i)
				r.mSegments.push_back(*i);
		return r;
	}
};


void	MapMerge(Pmwx& src_a, Pmwx& src_b, Pmwx& result)
{
	Arr_full_overlay_traits<Pmwx, Pmwx, Pmwx, Overlay_vertex, Overlay_network, Overlay_terrain> 	t;
	CGAL::overlay(src_a,src_b,result,t);
}

void	MapOverlay(Pmwx& bottom, Pmwx& top, Pmwx& result)
{
	vector<Halfedge_handle>		dead;
	Arr_replace_overlay_traits<Pmwx,Pmwx,Pmwx>		t;
	t.dead = &dead;
	CGAL::overlay(bottom, top,result,t);
	for(vector<Halfedge_handle>::iterator k = dead.begin(); k != dead.end(); ++k)
	{
		DebugAssert((*k)->face()->contained());
		DebugAssert((*k)->twin()->face()->contained());
		result.remove_edge(*k);
	}
}

/************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************/


// Edge collection routines for various shapes...
void	CollectEdges(Pmwx& io_dst, edge_collector_t * collector, const Polygon_2& src, Locator * loc)
{
	DebugAssert(src.size() >= 3);
	DebugAssert(src.is_simple());
	for(int n = 0; n < src.size(); ++n)
	{
		collector->input = Curve_2(src.edge(n),0);
		collector->ctr = 0;
		DebugAssert(collector->input.source() != collector->input.target());
		if(loc)			insert_curve(io_dst, collector->input,*loc);
		else			insert_curve(io_dst, collector->input);
		DebugAssert(collector->ctr > 0);
	}
}

void	CollectEdges(Pmwx& io_dst, edge_collector_t * collector, const Polygon_with_holes_2& src, Locator * loc)
{
	DebugAssert(!src.is_unbounded());

		CollectEdges(io_dst,collector,src.outer_boundary(),loc);
	for(Polygon_with_holes_2::Hole_const_iterator h = src.holes_begin(); h != src.holes_end(); ++h)
		CollectEdges(io_dst,collector,*h,loc);				
}

// Polygon set is NOT REALLY a container of polygons with holes.  It is in fact a planar map.  Calling the output iterator
// requires a search over the entire map to capture the topology.  This will copy all edges, which we will then re-iterate.
// So instead, we simply go over every edge in the set (all of which "have meaning") and insert them.  gives us nice linear time
// processing, which is as good as it gets.
void		CollectEdges(Pmwx& io_dst, edge_collector_t * collector, const Polygon_set_2& src, Locator * loc)
{
	DebugAssert(!src.arrangement().unbounded_face()->contained());
	/*
	vector<Polygon_with_holes_2>	container;
	container.reserve(src.number_of_polygons_with_holes());
	src.polygons_with_holes(back_insert_iterator<vector<Polygon_with_holes_2> >(container));
	for(vector<Polygon_with_holes_2>::iterator p = container.begin(); p != container.end(); ++p)
		CollectEdges(io_dst, collector, *p, loc);
	*/
	for(Pmwx::Edge_const_iterator eit = src.arrangement().edges_begin(); eit != src.arrangement().edges_end(); ++eit)
	{
		DebugAssert(eit->face()->contained() || eit->twin()->face()->contained());			// If not true, why is this edge in the 
		DebugAssert(!eit->face()->contained() || !eit->twin()->face()->contained());		// arrangement?  Illegal for polygon set.
		if(eit->face()->contained())
			collector->input = Curve_2(Segment_2(eit->source()->point(),eit->target()->point()),0);
		else
			collector->input = Curve_2(Segment_2(eit->target()->point(),eit->source()->point()),0);
			
		collector->ctr = 0;
		if(loc)			insert_curve(io_dst, collector->input,*loc);
		else			insert_curve(io_dst, collector->input);
		DebugAssert(collector->ctr > 0);
		
	}
}



template <class __EdgeContainer>
void	MapMergePolygonAny(Pmwx& io_dst, const __EdgeContainer& src, set<Face_handle> * out_faces, Locator * loc)
{
	edge_collector_t	collector;
	collector.attach(io_dst);
	
	CollectEdges(io_dst, &collector, src, loc);
		
	if(out_faces)
	{
		out_faces->clear();
		FindFacesForEdgeSet(collector.results, *out_faces);
	}
}

template <class __EdgeContainer>
Face_handle		MapOverlayPolygonAny(Pmwx& io_dst, const __EdgeContainer& src, Locator * loc)
{
	edge_collector_t	collector;
	collector.attach(io_dst);
	
	CollectEdges(io_dst, &collector, src, loc);

	DebugAssert(!collector.results.empty());

	// Go through and find all internal edges to the area - we will nuke them!
	set<Halfedge_handle>	to_nuke;
	FindInternalEdgesForEdgeSet(collector.results, to_nuke);

	collector.detach();

	for(set<Halfedge_handle>::iterator k = to_nuke.begin(); k != to_nuke.end(); ++k)
	{
		DebugAssert(to_nuke.count((*k)->twin())==0);		// Make sure we didn't pick up edge twice!
		io_dst.remove_edge(*k);								// Land-mark locator doesn't do full 
	}
	
	// Now that we are clean, find the "face" that we have unified.  Get face later - now that edges are removed
	// we don't have to worry about which face was added and which was removed.
	Face_handle f = (*collector.results.begin())->face();

	// Dev check - make sure ALL half-edges point to the same face - otherwise something is dreadfully wrong.
	#if DEV
		for(set<Halfedge_handle>::iterator e = collector.results.begin(); e != collector.results.end(); ++e)
		{
			DebugAssert((*e)->face() == f);
		}
		
		DebugAssert(f->holes_begin() == f->holes_end());		// Also, we should have nuked any holes inside our area too!
	#endif
	return f;
}


void			MapMergePolygon(Pmwx& io_dst, const Polygon_2& src, set<Face_handle> * out_faces, Locator * loc)
{
	MapMergePolygonAny(io_dst,src,out_faces,loc);
}
void			MapMergePolygonWithHoles(Pmwx& io_dst, const Polygon_with_holes_2& src, set<Face_handle> * out_faces, Locator * loc)
{
	MapMergePolygonAny(io_dst,src,out_faces,loc);
}
void			MapMergePolygonSet(Pmwx& io_dst, const Polygon_set_2& src, set<Face_handle> * out_faces, Locator * loc)
{
	MapMergePolygonAny(io_dst,src,out_faces,loc);
}


Face_handle		MapOverlayPolygon(Pmwx& io_dst, const Polygon_2& src, Locator * loc)
{
	MapOverlayPolygonAny(io_dst,src,loc);
}
Face_handle		MapOverlayPolygonWithHoles(Pmwx& io_dst, const Polygon_with_holes_2& src, Locator * loc)
{
	MapOverlayPolygonAny(io_dst,src,loc);
}

void			MapOverlayPolygonSet(Pmwx& io_dst, const Polygon_set_2& src, Locator * loc, set<Face_handle> * faces)
{
	edge_collector_t	collector;
	collector.attach(io_dst);
	
	CollectEdges(io_dst, &collector, src, loc);

	DebugAssert(!collector.results.empty());

	// Go through and find all internal edges to the area - we will nuke them!
	set<Halfedge_handle>	to_nuke;
	FindInternalEdgesForEdgeSet(collector.results, to_nuke);

	collector.detach();

	for(set<Halfedge_handle>::iterator k = to_nuke.begin(); k != to_nuke.end(); ++k)
	{
		DebugAssert(to_nuke.count((*k)->twin())==0);		// Make sure we didn't pick up edge twice!
		io_dst.remove_edge(*k);								// Land-mark locator doesn't do full 
	}
	
	if(faces)
		FindFacesForEdgeSet(collector.results, *faces);
}



void OverlayMap_legacy(
			Pmwx& 	inDst, 
			Pmwx& 	inSrc)
{
	Pmwx	temp;
	MapOverlay(inDst,inSrc,temp);
	inDst=temp;
}





void MergeMaps_legacy(Pmwx& ioDstMap, Pmwx& ioSrcMap, bool inForceProps, set<Face_handle> * outFaces, bool pre_integrated, ProgressFunc func)
{
	DebugAssert(outFaces == NULL || !inForceProps);
	if(outFaces) outFaces->clear();
	
	if(inForceProps)
	{
		Pmwx temp;
		MapMerge(ioSrcMap, ioDstMap, temp);
		ioDstMap = temp;
	}
	else
	{
		Pmwx	temp;
		MapMerge(ioDstMap, ioSrcMap, temp);
		ioDstMap = temp;
		if(outFaces)
		for(Pmwx::Face_iterator f = ioDstMap.faces_begin(); f != ioDstMap.faces_end(); ++f)
		if(f->contained() && !f->is_unbounded())
			outFaces->insert(f);
	}
}

