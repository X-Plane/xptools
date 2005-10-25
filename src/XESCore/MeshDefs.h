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

#include "ParamDefs.h"
#include "DemDefs.h"

#include <CGAL/Simple_cartesian.h>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

typedef	CGAL::Simple_cartesian<double>						FastKernel;

typedef multimap<float, void *, greater<float> >			FaceQueue;	// YUCK - hard cast to avoid snarky problems with forward decls

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
								border_blend = rhs.border_blend; return *this; }

	double					height;					// Height of mesh at this vertex.
	double					wave_height;			// ratio of vegetation to terrain at this vertex.
	float					normal[3];				// Normal - X,Y,Z in OGL coords(!)
	hash_map<int, float>	border_blend;			// blend level for a border of this layer at this triangle!

};

struct	MeshFaceInfo {
	MeshFaceInfo() : terrain(NO_DATA),feature(NO_VALUE),flag(0) { }
	MeshFaceInfo(const MeshFaceInfo& rhs) : 
								terrain(rhs.terrain),
								feature(rhs.feature), 
								flag(rhs.flag),
								terrain_border(rhs.terrain_border) { 
								normal[0] = rhs.normal[0]; 
								normal[1] = rhs.normal[1]; 
								normal[2] = rhs.normal[2]; }								
								
	MeshFaceInfo& operator=(const MeshFaceInfo& rhs) { 
								terrain = rhs.terrain; 
								feature = rhs.feature;
								flag = rhs.flag;
								terrain_border = rhs.terrain_border; 
								normal[0] = rhs.normal[0]; 
								normal[1] = rhs.normal[1]; 
								normal[2] = rhs.normal[2];
								return *this; }

	int				insert_x;
	int				insert_y;
	float			insert_err;
	double			plane_a;
	double			plane_b;
	double			plane_c;

	int				terrain;				// General terrain type for this triangle, e.g. terrain_Natural, terrain_Water
	int				feature;				// Specific terrain type, e.g. natural converted to a real land use. (This is a .ter enum, NOT a table index btw)
	int				flag;					// General purpose, useful for..um...algorithms.
	set<int>		terrain_border;			// All terrains on top of us!
	float			normal[3];				// Tri flat normal - not in final DSF but handy for other sh-t.

	FaceQueue::iterator	self;					// Queue ref to self!
};

typedef	CGAL::Triangulation_vertex_base_with_info_2<MeshVertexInfo, FastKernel>		Vb;
typedef CGAL::Triangulation_face_base_with_info_2<MeshFaceInfo, FastKernel>			Fbi;


typedef	CGAL::Constrained_triangulation_face_base_2<FastKernel, Fbi>				Fb;
typedef	CGAL::Triangulation_data_structure_2<Vb, Fb>								TDS;

typedef	CGAL::Constrained_Delaunay_triangulation_2<FastKernel, TDS, CGAL::No_intersection_tag>	CDTBase;

class CDT : public CDTBase { 
public:

	typedef hash_map<int, Face_handle>	HintMap;

	static	int			gen_cache_key(void);
			Face_handle locate_cache(const Point& p, Locate_type& lt, int& li, int cache_key) const;
			void		cache_reset(void);
			void		clear(void);

private:

	static	int		sKeyGen;
	mutable	HintMap	mHintMap;
	
};

#define CONVERT_POINT(__X)	(CDT::Point((__X).x,(__X).y))

#endif

