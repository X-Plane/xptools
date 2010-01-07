/*
 * Copyright (c) 2007, Laminar Research.
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

#include "Forests.h"
#include "MapDefs.h"
#include "MeshDefs.h"
#include "MapAlgs.h"
#include "MapPolygon.h"
#include "AssertUtils.h"
#include "DEMTables.h"
#include "CompGeomUtils.h"
#include "Skeleton.h"
#include "ObjPlacement.h"
#include <CGAL/centroid.h>
#define DEBUG_SELECT_BAD_FACE 0
#define DEBUG_FOREST_TREE_MAPS_CTR 0

#define SHOW_MAP_MERGE 1

#if DEBUG_SHOW_MAP_MERGE_FAIL || DEBUG_SELECT_BAD_FACE || SHOW_MAP_MERGE
#include "PlatformUtils.h"
#include "RF_Notify.h"
#include "RF_Msgs.h"
#include "RF_Selection.h"
#include "RF_Globals.h"

#endif

#define POST_SIMPLIFY 0.001

int	tri_forest_type(CDT::Face_handle f)
{
	return NO_VALUE;
/*	
	// WHAT's THE IDEA?
	// Return the top terrain type's forest type - visually we want to match the guy who is topmost.
	// BUT if ANY terrain has no forest, return no forest.  This way we will never put trees over a potentially
	// incompatible bitmap, EVEN if the Italians decide to put the forest type on top for visual reasons.

	int forest_type = gNaturalTerrainTable[gNaturalTerrainIndex[f->info().terrain]].forest_type;
	int forest_terrain = f->info().terrain;
	if (forest_type == NO_VALUE) return NO_VALUE;

	for (set<int>::iterator border = f->info().terrain_border.begin(); border != f->info().terrain_border.end(); ++border)
	{
		int border_forest = gNaturalTerrainTable[gNaturalTerrainIndex[forest_terrain]].forest_type;
		if (border_forest == NO_VALUE) return NO_VALUE;
		if (LowerPriorityNaturalTerrain(forest_terrain, *border))
		{
			forest_terrain = *border;
			forest_type = border_forest;
		}
	}
	return forest_type;
*/	
}


struct	tri_hash_t {
	typedef	hash_multimap<int, CDT::Face_handle>	hash;

	int					dims;
	Bbox2				bounds;
	hash				tris;

	inline int hash_x(double x_c)
	{
		int x = ((x_c - bounds.p1.x()) * (float) dims / (bounds.p2.x() - bounds.p1.x()));
		if (x < 0) x = 0;
		if (x >= dims) x = dims-1;
		return x;
	}
	inline int hash_y(double y_c)
	{
		int y = ((y_c - bounds.p1.y()) * (float) dims / (bounds.p2.y() - bounds.p1.y()));
		if (y < 0) y = 0;
		if (y >= dims) y = dims-1;
		return y;
	}

	inline int	hash_point(int x, int y)
	{
		return x + y * dims;
	}

	inline int	accum_tri(CDT::Face_handle f);
	inline int fetch_tris(const Bbox2& bounds, set<CDT::Face_handle>& outTris, set<int>& outTypes);
};

// Returns the number of hash table buckets that the try was sent to.
int	tri_hash_t::accum_tri(CDT::Face_handle f)
{
	int c = 0;
	int x1 = hash_x(CGAL::to_double(f->vertex(0)->point().x()));
	int x2 = hash_x(CGAL::to_double(f->vertex(2)->point().x()));
	int x3 = hash_x(CGAL::to_double(f->vertex(1)->point().x()));
	int y1 = hash_y(CGAL::to_double(f->vertex(0)->point().y()));
	int y2 = hash_y(CGAL::to_double(f->vertex(2)->point().y()));
	int y3 = hash_y(CGAL::to_double(f->vertex(1)->point().y()));
	int x_min = min(x1, min(x2, x3));
	int y_min = min(y1, min(y2, y3));
	int x_max = max(x1, max(x2, x3));
	int y_max = max(y1, max(y2, y3));

	for (int x = x_min; x <= x_max; ++x)
	for (int y = y_min; y <= y_max; ++y)
	{
		++c;
		tris.insert(hash::value_type(hash_point(x,y), f));
	}
	return c;
}

// Returns number of hash table operations required.  You can tell the # of tris from the set returned.
int tri_hash_t::fetch_tris(const Bbox2& bounds, set<CDT::Face_handle>& outTris, set<int>& outTypes)
{
	outTris.clear();
	outTypes.clear();
	int ctr = 0;
	int x1 = hash_x(bounds.p1.x());
	int x2 = hash_x(bounds.p2.x());
	int y1 = hash_y(bounds.p1.y());
	int y2 = hash_y(bounds.p2.y());
	for (int x = x1; x <= x2; ++x)
	for (int y = y1; y <= y2; ++y)
	{
		pair<hash::iterator, hash::iterator>	range = tris.equal_range(hash_point(x,y));
		for (hash::iterator t = range.first; t != range.second; ++t)
		{
			outTris.insert(t->second);
			outTypes.insert(tri_forest_type(t->second));
		}
		++ctr;
	}
	return ctr;
}

static void AssertThrowQuiet(const char * msg, const char * file, int line)
{
	throw msg;
}

void GenerateForests(
				Pmwx&					ioMap,
				vector<PreinsetFace>&	inFaces,
				CDT&					ioMesh,
				ProgressFunc	 		inProg)
{

	// Keep some nice stats...
	int		indexed_tris = 0;			// How many tris were considered for indexing (subset of mesh that has forest)
	int		hashed_tris = 0;			// Same as above, but tri counts twice if in two bucket...this describes hash table inefficiency.
	int		processed_tris = 0;			// How many tris did we process...each tri is counted once each time it is involved in a GT Polygon
	int		processed_gt = 0;			// How many GT polygons did we try to build a forest for?
	int		hash_fetches = 0;			// How many hash fetches did we need (should be close to the # of GT polygons?)
	int		forest_poly_count = 0;		// Total forests built
	int		forest_pt_count = 0;		// Total count of all points in forests (describes DSF usage)
	int		poly_fail = 0;
	float	total;
	int		ctr;



	if (inProg && inProg(0, 2, "Indexing mesh", 0.0)) return;

		Pmwx::Ccb_halfedge_circulator	iter, stop;

	/************************************************************************************
	 * MESH PREPROCESSING AND INDEXING
	 ************************************************************************************/

	// First step - we need to index our triangle mesh...otherwise it's going to be
	// much much much too slow to dredge up the terrain polygons that overlay a given
	// GT polygon.

		tri_hash_t			tri_hash;

	tri_hash.dims = 200;
	Point_2	sw,ne;
	CalcBoundingBox(ioMap, sw,ne);
	tri_hash.bounds=Bbox2(cgal2ben(sw),cgal2ben(ne));
	ctr = 0;
	total = ioMesh.number_of_faces();

	for (CDT::Finite_faces_iterator ffi = ioMesh.finite_faces_begin(); ffi != ioMesh.finite_faces_end(); ++ffi, ++ctr)
	{
		if (total && (ctr % 1000) == 0 && inProg && inProg(0, 2, "Indexing mesh", (float) ctr / total)) return;

		if (ffi->info().terrain != terrain_Water)
		{
			if (tri_forest_type(ffi) != NO_VALUE)
			{
				hashed_tris += tri_hash.accum_tri(ffi);
				++indexed_tris;
			}
		}
	}
	if (inProg && inProg(0, 2, "Indexing mesh", 1.0)) return;

	/************************************************************************************
	 * GENERATE GT Polygons for consideration
	 ************************************************************************************/

	// Go through our GT polygons...if we are going to process it...

	ctr = 0;
	total = inFaces.size();
	int	gap = total / 100.0;
	if (inProg && inProg(1, 2, "Processing faces", 0.0)) return;

	for (vector<PreinsetFace>::iterator fp = inFaces.begin(); fp != inFaces.end(); ++fp, ++ctr)
	{
		Face_handle face = fp->first;
		DebugAssert(!face->is_unbounded());
		DebugAssert(face->data().mTerrainType != terrain_Water);

		if (total && gap && (ctr % gap) == 0 && inProg && inProg(1, 2, "Processing faces", (float) ctr / total)) return;

		// First we need to figure out what triangles ovrelap us.
		set<int>						forest_types;
		set<CDT::Face_handle>			forest_tris;

		Bbox2							face_bounds;

		iter = stop = face->outer_ccb();
		do {
			if (iter == stop)
				face_bounds = cgal2ben(iter->target()->point());
			else
				face_bounds += cgal2ben(iter->target()->point());
			++iter;
		} while (iter != stop);

		hash_fetches += tri_hash.fetch_tris(face_bounds, forest_tris, forest_types);
		processed_tris += sizeof(forest_tris);
		++processed_gt;

		// Some polygons will have no forest triangles overlaying, so defer Gt polygon basemap
		// calculations until after hash check for speed.
		Assert(forest_types.count(NO_VALUE) == 0);
		if (!forest_types.empty())
		{
//			// Generate the "GT polygon base map".
//			Pmwx				baseMap;

//			for (vector<ComplexPolygon2>::iterator poly = fp->second.begin(); poly != fp->second.end(); ++poly)
//			{
//				ComplexPolygonToPmwx(*poly, baseMap, terrain_ForestPark, terrain_Water);
//			}

//			DebugAssert(baseMap.is_valid());

			int forest_type_ctr = 0;
			for (set<int>::iterator fiter = forest_types.begin(); fiter != forest_types.end(); ++fiter, ++forest_type_ctr)
			{
				Polygon_set_2	forest_area;
				vector<Polygon_2>	tris;
				for (set<CDT::Face_handle>::iterator tri = forest_tris.begin(); tri != forest_tris.end(); ++tri)
				if (tri_forest_type(*tri) == *fiter)
				{
					Polygon_2	tri_poly;
					tri_poly.push_back((*tri)->vertex(0)->point());
					tri_poly.push_back((*tri)->vertex(1)->point());
					tri_poly.push_back((*tri)->vertex(2)->point());
					DebugAssert(tri_poly.is_counterclockwise_oriented());
					tris.push_back(tri_poly);
				}
				forest_area.join(tris.begin(), tris.end());

				forest_area.intersection(fp->second);
				
				SimplifyPolygonMaxMove(fp->second, POST_SIMPLIFY);

				vector<Polygon_with_holes_2>	forests;
				forest_area.polygons_with_holes(back_insert_iterator<vector<Polygon_with_holes_2> >(forests));

				for(vector<Polygon_with_holes_2>::iterator f = forests.begin(); f != forests.end(); ++f)
				{
					GISPolyObjPlacement_t	placement;
					placement.mRepType = *fiter;
					placement.mShape = *f;
					DebugAssert(!placement.mShape.is_unbounded());
					placement.mLocation = CGAL::centroid(f->outer_boundary().vertices_begin(),f->outer_boundary().vertices_end());
					placement.mHeight = 255.0;
					placement.mDerived = false;
					face->data().mPolyObjs.push_back(placement);
					forest_poly_count++;
				}
			}
		}
	}

	if (inProg && inProg(1, 2, "Processing faces", 1.0)) return;

	printf("Indexed Tris: %d, Hashed tris = %d, Processed Tris = %d, Processed GT Polys = %d, Hash ops = %d\n", indexed_tris, hashed_tris, processed_tris, processed_gt, hash_fetches);
	printf("Total forest polys = %d, total forest pts = %d, poly fail = %d\n", forest_poly_count, forest_pt_count, poly_fail);

}

/*
void GetAllForestLUs(
				set<int>&				out_types)
{
	for(int n = 0; n < gNaturalTerrainTable.size(); ++n)
	if(gNaturalTerrainTable[n].forest_type != NO_VALUE)
		out_types.insert(gNaturalTerrainTable[n].name);
}
*/