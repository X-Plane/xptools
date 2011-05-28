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
#ifndef MESHDEFS_H
#define MESHDEFS_H

//#define CGAL_NO_PRECONDITIONS
//#ifndef NDEBUG
//#define NDEBUG
//#endif


#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

#include "ParamDefs.h"
#include "CGALDefs.h"
#include "DEMDefs.h"
#include "MapDefs.h"			// We need this for face-handle forward declaration.

/*

	Pmwx-CDT Synchronization - THEORY OF DESIGN
	
	The CDT is designed to be derived from a Pmwx, with the constraints in the CDT coming from Pmwx edges that needed to be "burned".

	The relationship between vertices in the CDT and vertices in the Pmwx is many-to-many:
	- Some Pmwx vertices will be dropped to simplify the complexity of the arrangement.  In particular, if a burned edge intersects a 
	  non-burned edge in the Pmwx, the vertex defining that point will NOT make it to the CDT.
	- The CDT may contain some vertices that don't come from the Pmwx, to shorten the max length of a constraint via subdivision.
	
	The orig_vertex field links the CDT back to the Pmwx.  In order to keep sync working we need to meet a few criteria:
	
	- If a CDT vertex has no incident constraints, it must have a null orig-ptr because it comes from an interior point and isn't synced
	  to a burnable edge.
	- If a CDT vertex has either one or more than two incident constraints, it must have a non-null orig-ptr because it is a "node" -
	  that is, a point of interest in the network topology of constraints.  These points cannot be optimized out, and they cannot
	  be created via subdivision, hence their original ptr.
	- Similarly, any Pmwx node whose degree of burned edges is 1 or more than 2 must have a corresponding CDT vertex with a ptr back to it.
	- If a CDT vertex has two incident constraints, it may or may not have an orig-ptr because it could be a subdivision.
	- Similarly, any Pmwx node whose degree of burned edges is 2 may or may not have a corresponding CDT vertex with a ptr back to it.

	Finally, there is one more rule for reduction that is slightly non-intuitive:
	- Define a burned poly-line as the extension of a must-burn Pmwx edge in both directions until we hit a vertex whose must-burn degree
	  is not two. (These are analogous to "shape points" in topology terms.)  If there is at least one interior burn-degree two vertex
	  along the line, then at least one of the shape points must be in the CDT.
	  
	In other words, we cannot drop ALL of the shape points of a poly-line unless there were none to begin with. 	This last rule is
	necessary to ensure that given multiple poly-liens between two vertices, that there is at least one disambiguating "sync point"
	in each.  (In theory we could drop the sync points out of one of the poly-lines, perhaps the one with the most direct path, but
	in practice the code would be more complex if we did.)
	
	Note that a triangle-loop can never be reduced to an edge because given any two points, the third point is the only "shape point'
	between the first two.  The "one sync point" rule thus makes a "don't reduce triangles to lines" rule unnecessary.

 */


typedef multimap<float, void *, greater<float> >			FaceQueue;	// YUCK - hard cast to avoid snarky problems with forward decls
typedef multimap<double, void *>							VertexQueue;

struct	MeshVertexInfo {
	MeshVertexInfo() : height(0.0), wave_height(1.0) { }
	MeshVertexInfo(const MeshVertexInfo& rhs) :
								height(rhs.height),
								border_blend(rhs.border_blend) {
								normal[0] = rhs.normal[0];
								normal[1] = rhs.normal[1];
								normal[2] = rhs.normal[2]; }
	MeshVertexInfo& operator=(const MeshVertexInfo& rhs) {
								height = rhs.height;
								normal[0] = rhs.normal[0];
								normal[1] = rhs.normal[1];
								normal[2] = rhs.normal[2];
								orig_vertex = rhs.orig_vertex;
								border_blend = rhs.border_blend; return *this; }

	double					height;					// Height of mesh at this vertex.
	double					wave_height;			// ratio of vegetation to terrain at this vertex.
	float					normal[3];				// Normal - X,Y,Z in OGL coords(!)
	hash_map<int, float>	border_blend;			// blend level for a border of this layer at this triangle!
	
	Vertex_handle			orig_vertex;			// Original vertex in the Pmwx.
	VertexQueue::iterator	self;

};

struct	MeshFaceInfo {
	MeshFaceInfo() : terrain(DEM_NO_DATA),feature(NO_VALUE),flag(0), orig_face(NULL) { }
	MeshFaceInfo(const MeshFaceInfo& rhs) :
								terrain(rhs.terrain),
								feature(rhs.feature),
								flag(rhs.flag),
								terrain_border(rhs.terrain_border) {
								normal[0] = rhs.normal[0];
								normal[1] = rhs.normal[1];
								normal[2] = rhs.normal[2];
								orig_face = rhs.orig_face; }


	MeshFaceInfo& operator=(const MeshFaceInfo& rhs) {
								terrain = rhs.terrain;
								feature = rhs.feature;
								flag = rhs.flag;
								terrain_border = rhs.terrain_border;
								normal[0] = rhs.normal[0];
								normal[1] = rhs.normal[1];
								normal[2] = rhs.normal[2];
								orig_face = rhs.orig_face;
								return *this; }

	int				insert_x;
	int				insert_y;
	float			insert_err;
	double			plane_a;
	double			plane_b;
	double			plane_c;

	int				terrain;				// Specific terrain type, e.g. natural converted to a real land use. (This is a .ter enum, NOT a table index btw)
	int				feature;				// General terrain type for this triangle, e.g. terrain_Natural, terrain_Water
	int				flag;					// General purpose, useful for..um...algorithms.
	set<int>		terrain_border;			// All terrains on top of us!
	float			normal[3];				// Tri flat normal - not in final DSF but handy for other sh-t.

	Face_handle		orig_face;				// If a face caused us to get the terrain we did, this is who!

	FaceQueue::iterator	self;					// Queue ref to self!

	float			mesh_temp;				// These are not debug - beach code uses this.
	float			mesh_rain;

#if OPENGL_MAP
	int				debug_terrain_orig;
	float			debug_slope_dem;
	float			debug_slope_tri;
	float			debug_temp_range;
	float			debug_heading;
	float			debug_re;
	float			debug_er;	
	float			debug_lu[5];
#endif	
};

typedef	CGAL::Triangulation_vertex_base_with_info_2<MeshVertexInfo, Traits_2>		Vb;
typedef CGAL::Triangulation_face_base_with_info_2<MeshFaceInfo, Traits_2>			Fbi;


typedef	CGAL::Constrained_triangulation_face_base_2<Traits_2, Fbi>				Fb;
typedef	CGAL::Triangulation_data_structure_2<Vb, Fb>								TDS;

typedef	CGAL::Constrained_Delaunay_triangulation_2<FastKernel, TDS, CGAL::Exact_predicates_tag>	CDTBase;

class CDT : public CDTBase {
public:

	typedef hash_map<int, Face_handle>	HintMap;

	static	int			gen_cache_key(void);
			Face_handle locate_cache(const Point& p, Locate_type& lt, int& li, int cache_key) const;
			void		cache_reset(void);
			void		clear(void);

	Vertex_handle	insert_collect_flips(const Point& p, Face_handle hint, set<Face_handle>& all);

private:

	void			my_propagating_flip(Face_handle& f,int i, set<Face_handle>& all);

	static	int		sKeyGen;
	mutable	HintMap	mHintMap;

};

// Directed edge is one whose triangle face is on the LEFT when walking from src to dst.  This returns
// the srcand dst vertices.
inline CDT::Vertex_handle	CDT_he_source(const CDT::Edge& e);
inline CDT::Vertex_handle	CDT_he_target(const CDT::Edge& e);

// Return directed TWIN
inline CDT::Edge			CDT_he_twin(CDT::Edge& e);

// Make a DIRECTED halfedge from A to B.  Returns NULL if not connected!
inline CDT::Edge			CDT_make_he(CDT::Vertex_handle a, CDT::Vertex_handle b);

// Given a directed halfedge, if there is only one other constraint with e's target
// as it's source, return it, otherwise null.
inline CDT::Edge			CDT_next_constraint(CDT::Edge& e);

// Is this vertex on the edge of the triangulation?
inline bool IsEdgeVertex(CDT& inMesh, CDT::Vertex_handle v);


/*************
 * INLINES 
 *************/

inline bool IsEdgeVertex(CDT& inMesh, CDT::Vertex_handle v)
{
	CDT::Vertex_circulator circ, stop;
	circ = stop = inMesh.incident_vertices(v);
	do {
		if (inMesh.is_infinite(circ)) return true;
		circ++;
	} while (circ != stop);
	return false;
}


inline CDT::Vertex_handle	CDT_he_source(const CDT::Edge& e) { return e.first->vertex(CDT::ccw(e.second)); }
inline CDT::Vertex_handle	CDT_he_target(const CDT::Edge& e) { return e.first->vertex(CDT::cw (e.second)); }

inline CDT::Edge			CDT_he_twin(CDT::Edge& e) { 	
	CDT::Face_handle neighbor = e.first->neighbor(e.second);
	CDT::Vertex_handle my_src = CDT_he_source(e);
	int target_index = neighbor->index(my_src);	
	#if DEV
		CDT::Edge r(neighbor, CDT::ccw(target_index));
		DebugAssert(CDT_he_source(e) == CDT_he_target(r));
		DebugAssert(CDT_he_source(r) == CDT_he_target(e));
	#endif
	return CDT::Edge(neighbor, CDT::ccw(target_index));
}

inline CDT::Edge			CDT_make_he(const CDT& cdt, CDT::Vertex_handle a, CDT::Vertex_handle b)
{
	CDT::Edge r;
	if (cdt.is_edge(a,b,r.first,r.second))
	{
		DebugAssert(CDT_he_source(r) == a || CDT_he_source(r) == b);
		DebugAssert(CDT_he_target(r) == a || CDT_he_target(r) == b);
		if(CDT_he_source(r) == a)
			return r;
		else
			return CDT_he_twin(r);
	}
	else
	{
		DebugAssert(r.first == CDT::Face_handle());
		return r;
	}
}

inline CDT::Edge			CDT_next_constraint(CDT::Edge& e)
{
	//printf("\tLooking for next constraint for %s.\n",print_edge(e));
	DebugAssert(e.first->is_constrained(e.second));
	CDT::Edge	best;
	DebugAssert(best.first == CDT::Face_handle());
	CDT::Edge_circulator circ, stop;
	CDT::Vertex_handle	t = CDT_he_target(e);
	//printf("\tCirculating target %p.\n", &*t);
	circ = stop = t->incident_edges();
	do 
	{
		//printf("\t\tcirculator edge %s.\n", print_edge(*circ));
		if (circ->first->is_constrained(circ->second))
		{
			//printf("\t\t\tIs constrained.\n");
			// Get C, an edge pointing OUT of e's target (the circulated vertex).
			CDT::Edge c(*circ);
			if(CDT_he_source(c) != t)
				c = CDT_he_twin(c);
			//printf("\t\t\tright dir is: %s.\n",print_edge(c));
				
			// If C and e aren't twins, this is a new departure route.
			if(CDT_he_twin(c) != e)
			{
				//printf("\t\t\tDoesn't match previous.\n");
				// Two exits?  We're done.
				if(best.first != CDT::Face_handle())	
				{
					//printf("   Bail - hit a Y split.\n");
					return CDT::Edge();
				}
				best = c;
			}
		}
	}
	while(++circ != stop);
	//printf("\tBest was: %s\n", print_edge(best));
	return best;
}



#endif

