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
#ifndef ROAD_EXTRUDE_H

#define ROAD_EXTRUDE_H



#include "ExtrudeFunc.h"



#include <vector>

#include <set>

#include <map>

using std::vector;

using std::set;

using std::map;



struct RoadJunction_t;

struct RoadChain_t;



#define ROAD_SECTIONS 1





struct RoadDef_t {

	struct SegPart {

		float	dx[2];	// Offset of segment in RATIO OF WIDTH!!!!!

		float	dy[2];	// Height in meters

		float	s[2];	// S coods

		float	lod_near;

		float	lod_far;

	};

	typedef vector<SegPart> SegVector;

	struct WireSpec {

		float	dx;			// Offset of segment in RATIO OF WIDTH!!!!!

		float	dy;			// Height in meters

		float	droop_rat;

		float	min_dis;	// meters

		float	max_dis;	// meters

		float	lod_near;

		float	lod_far;

	};

	typedef vector<WireSpec> WireVector;

	struct ObjSpec {

		int		model_index;

		float	dx;

		bool	on_ground;

		double	rotation;

		float	dist;

		float	offset;

	};

	typedef vector<ObjSpec> ObjVector;

	struct RoadType {

		SegVector	segments;

		WireVector	wires;

		ObjVector	objects;

		int			tex_index;

		bool		has_wires;

		float		width;	// Width in meters

		float		length;	// Length in meters of 1 texture rep

		float		color[3];

	};

	typedef map<xint, RoadType> RoadTypeMap;



	vector<int>		texes;

	vector<int>		texes_lit;

	vector<int>		offsets;

	RoadTypeMap		road_types;



	bool	ReadFromDef(const char * inFile);

	void	Clear();

};



struct RoadChain_t {

	int					type;

	RoadJunction_t *	start_node;

	RoadJunction_t *	end_node;

	vector<float>		shape_points;	// Dim is a multiple of 3



	void				reverse(void);

	float				get_heading(RoadJunction_t * junc, bool force_end=false);



	// Indexing as if shape and end pts are all one happy family.

	float *				get_nth_pt(int n);

	void				get_nth_dir(int n, float xyz[3]);

	void				check_validity(void);

};



struct RoadJunction_t {

	int						id;

	float					location[3];

	vector<RoadChain_t *>	chains;



	void				sort_clockwise(void);

};



class	RoadData {

public:



	typedef	vector<RoadChain_t>			RoadChainVector;

	typedef set<RoadChain_t *>			RoadChainSet;

	typedef vector<RoadJunction_t *>	RoadJunctionVector;

	typedef set<RoadJunction_t *>		RoadJunctionSet;

	typedef vector<RoadJunction_t *>	RoadJunctionMap;



	// These 2-d arrays will contain all of the finished junctions

	// and chains, in buckets.

	RoadChainVector		final_chains[ROAD_SECTIONS][ROAD_SECTIONS];

//	RoadJunctionVector	final_junctions[ROAD_SECTIONS][ROAD_SECTIONS];



	/************** INCREMENTAL CREATION API ****************/



	// This routine is used to add one simple segment.  If no_dupes is true, and the chain

	// already exists, then the existing chain is returned.  Otherwise a new chain is always

	// created.

	RoadChain_t *	AddSimpleSegment(

						int		type,

						int		node1,

						int		node2,

						float	xyz1[3],

						float	xyz2[3],

						bool	no_dupes);



	// This routine is used to add shape to a simple chain.  XYZ should have 3*count number

	// of floats.

	void			AddShapePoint(

						RoadChain_t *	inChain,

						int				count,

						float			xyz[]);



	// This routine sorts and organizes chains once they're all done

	// If simplify is true, then roads will be extended to make chains.  This is only needed

	// for non-pre-organized road segment sets.

	void			ProcessChains(

						bool			inSimplify,

						float			x1,

						float			x2,

						float			z1,

						float			z2);





	/************** PREALLOCATION API ****************/



	// This allocates all of the memory needed for all junctions, chains, etc.

	void			Preallocate(int max_node_id, int num_chains[ROAD_SECTIONS][ROAD_SECTIONS]);



	// Same as above, except the segment is not eligible for merging.  More memory efficient.

	RoadChain_t *	AddSimpleSegmentDirect(

						int		x_bucket,

						int		y_bucket,

						int		index,		// Zero based index for bucket

						int		type,

						int		node1,

						int		node2,

						float	xyz1[3],

						float	xyz2[3]);



	// Add all shape points at once and clear the vector.  This routine transfers the

	// vector's memory to avoid an allocate, copy, clear cycle.  Therefore it can

	// only be called once per chain.

	void			AddShapePointVectorAndClear(

						RoadChain_t *	inChain,

						vector<float>&	ioVec);



	// Chain processing for preallocated use.

	void			ProcessChainsPreallocated(void);







	/************** RENDERING API ****************/



	// This routine extrudes one section worth of stuff.

	void			ExtrudeArea(

						int					tex_index,

						const RoadDef_t&	defs,

						int					x_bucket,

						int					y_bucket,

						ExtrudeFunc_f		func,

						ReceiveObj_f		objFunc,

						void *				ref,

						void *				objRef,

						void *				objRef2);



	void			Clear(void);



	void			Dump(void);





private:

	void				MakeShapePoints(void);									// Goes through and eliminates trivial junctions

	void				BucketAll(float x1, float x2, float z1, float z2);		// Puts all items in their final locations.



	// These track all of our working elements.  We use sets so that we can quickly

	// wipe out anything we delete!!

	RoadChainSet		working_chains;

	RoadJunctionSet		working_junctions;

	// This tracks our junctiosn by ID, so we can rapidly identify dupe nodse as we build the system up.

	// It is NOT used to consolidate chains.

	RoadJunctionMap		building_junctions;

};





#endif