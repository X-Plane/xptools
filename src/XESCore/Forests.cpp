#include "Forests.h"
#include "MapDefs.h"
#include "MeshDefs.h"
#include "MapAlgs.h"
#include "AssertUtils.h"
#include "DEMTables.h"
#include "CompGeomUtils.h"
#include "Skeleton.h"

#define DEBUG_SHOW_MAP_MERGE_FAIL 0
#define DEBUG_SELECT_BAD_FACE 0
#define DEBUG_FOREST_TREE_MAPS_CTR 0

#if DEBUG_SHOW_MAP_MERGE_FAIL || DEBUG_SELECT_BAD_FACE
#include "PlatformUtils.h"
#include "WED_Notify.h"
#include "WED_Msgs.h"
#include "WED_Selection.h"
#include "WED_Globals.h"

#endif

inline	int	tri_forest_type(CDT::Face_handle f)
{
	return gNaturalTerrainTable[gNaturalTerrainIndex[f->info().terrain_specific]].forest_type;
}


struct	tri_hash_t {
	typedef	hash_multimap<int, CDT::Face_handle>	hash;
	
	int					dims;
	Bbox2				bounds;
	hash				tris;
	
	inline int hash_x(double x_c)
	{
		int x = ((x_c - bounds.p1.x) * (float) dims / (bounds.p2.x - bounds.p1.x));
		if (x < 0) x = 0;
		if (x >= dims) x = dims-1;
		return x;
	}
	inline int hash_y(double y_c)
	{
		int y = ((y_c - bounds.p1.y) * (float) dims / (bounds.p2.y - bounds.p1.y));
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
	int x1 = hash_x(f->vertex(0)->point().x());
	int x2 = hash_x(f->vertex(2)->point().x());
	int x3 = hash_x(f->vertex(1)->point().x());
	int y1 = hash_y(f->vertex(0)->point().y());
	int y2 = hash_y(f->vertex(2)->point().y());
	int y3 = hash_y(f->vertex(1)->point().y());
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
	int x1 = hash_x(bounds.p1.x);
	int x2 = hash_x(bounds.p2.x);
	int y1 = hash_y(bounds.p1.y);
	int y2 = hash_y(bounds.p2.y);
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

void GenerateForests(
				Pmwx&					ioMap,
				const set<GISFace *>&	inFaces,
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
	float	total;
	int		ctr;

	int		total_good = 0, total_bad = 0;

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
	CalcBoundingBox(ioMap, tri_hash.bounds.p1, tri_hash.bounds.p2);
	ctr = 0;
	total = ioMesh.number_of_faces();
	
	for (CDT::Finite_faces_iterator ffi = ioMesh.finite_faces_begin(); ffi != ioMesh.finite_faces_end(); ++ffi, ++ctr)	
	{
		if ((ctr % 1000) == 0 && inProg && inProg(0, 2, "Indexing mesh", (float) ctr / total)) return;

		if (ffi->info().terrain_general != terrain_Water)
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
	
	for (set<GISFace *>::const_iterator fp = inFaces.begin(); fp != inFaces.end(); ++fp, ++ctr)
	{
		GISFace * face = *fp;
		if (!face->is_unbounded())
		if (face->mTerrainType != terrain_Water)
		{
			if ((ctr % gap) == 0 && inProg && inProg(1, 2, "Processing faces", (float) ctr / total)) return;

#if DEBUG_SELECT_BAD_FACE
			try {
#endif
			
				// First we need to figure out what triangles ovrelap us.
				set<int>						forest_types;
				set<CDT::Face_handle>			forest_tris;
				
				Bbox2							face_bounds;		
				
				iter = stop = face->outer_ccb();
				do {
					if (iter == stop)
						face_bounds = iter->target()->point();
					else
						face_bounds += iter->target()->point();
					++iter;
				} while (iter != stop);

				hash_fetches += tri_hash.fetch_tris(face_bounds, forest_tris, forest_types);
				processed_tris += sizeof(forest_tris);
				++processed_gt;

				// Some polygons will have no forest triangles overlaying, so defer Gt polygon basemap
				// calculations until after hash check for speed.
				if (!forest_types.empty())
				{
					// Generate the "GT polygon base map".
					Pmwx				baseMap(*face);
//					Pmwx				baseMap;
					GISFace *			baseFace;
					
					for (Pmwx::Face_iterator f = baseMap.faces_begin(); f != baseMap.faces_end(); ++f)
					{			
						f->mTerrainType = terrain_Natural;
						f->mAreaFeature.mFeatType = NO_VALUE;
					}
					
					baseFace = (*baseMap.unbounded_face()->holes_begin())->twin()->face();
					baseFace->mTerrainType = terrain_ForestPark;
					baseFace->mAreaFeature.mFeatType = NO_VALUE;

					SimplifyMap(baseMap);
					DebugAssert(baseMap.is_valid());
					
					set<GISHalfedge *> e;
//					FindEdgesForFace(face, e);
					FindEdgesForFace(baseFace, e);
					for (set<GISHalfedge *>::iterator ee = e.begin(); ee != e.end(); ++ee)
					if ((*ee)->twin()->face()->is_unbounded())
						(*ee)->mTransition = 30.0;
					else
						(*ee)->mTransition = 30.0;

//					InsetPmwx(baseMap, baseFace);
					if (!SK_InsetPolygon(baseFace, baseMap, terrain_ForestPark, terrain_Water, 2000))
						++total_bad;
					else
					{
						++total_good;
					
						SimplifyMap(baseMap);
						RemoveUnboundedWater(baseMap);
						DebugAssert(baseMap.is_valid());
						
						int forest_type_ctr = 0;
						for (set<int>::iterator fiter = forest_types.begin(); fiter != forest_types.end(); ++fiter, ++forest_type_ctr)
						{
							/************************************************************************************
							 * BURN IN ONE FOREST TYPE
							 ************************************************************************************/
							// Burn in triangles that match this forest type.
							
								Pmwx					forestMap;
								set<GISHalfedge *>		inForest;
								map<Point2, GISVertex *, lesser_y_then_x> 			pt_index;
								map<Point2, GISVertex *, lesser_y_then_x>::iterator i1, i2;

							int ctr = 0;
							int num_total = 0;
							int num_slow = 0;

							multimap<double, pair<Point2, Point2> >	sides;
					
							for (set<CDT::Face_handle>::iterator tri = forest_tris.begin(); tri != forest_tris.end(); ++tri)
							if (tri_forest_type(*tri) == *fiter)
							for (int s = 0; s < 3; ++s)
							{
								if (tri_forest_type((*tri)->neighbor(s)) != *fiter ||
									forest_tris.count((*tri)->neighbor(s)) == 0)
								{
									FastKernel::Point_2 dp1 = (*tri)->vertex(CDT::ccw(s))->point();
									FastKernel::Point_2 dp2 = (*tri)->vertex(CDT:: cw(s))->point();
									Point2 p1(dp1.x(), dp1.y());
									Point2 p2(dp2.x(), dp2.y());
									sides.insert(multimap<double, pair<Point2, Point2> >::value_type(min(p1.x,p2.x), pair<Point2, Point2>(p1, p2)));
								}
							}
							
							for (multimap<double, pair<Point2, Point2> >::iterator s = sides.begin(); s != sides.end(); ++s)
							{
								Point2 p1 = s->second.first;
								Point2 p2 = s->second.second;
								{

									GISHalfedge * nh;
									i1 = pt_index.find(p1);
									i2 = pt_index.find(p2);
									if (i1 != pt_index.end())
									{
										if (i2 != pt_index.end())
										{
											/* CASE 1 - Both points already in. */
											nh = forestMap.nox_insert_edge_between_vertices(i1->second, i2->second);
										} 
										else
										{
											/* Case 2 - Point 1 in, point 2 new. */
											nh = forestMap.nox_insert_edge_from_vertex(i1->second, p2);
											pt_index[p2] = nh->target();
										}
									} 
									else
									{
										if (i2 != pt_index.end())
										{
											/* Case 3 - Point 1 new, point 2 in. */
											nh = forestMap.nox_insert_edge_from_vertex(i2->second, p1)->twin();
											pt_index[p1] = nh->source();
										} 
										else
										{
											/* Case 4 - both points new. */
											nh = forestMap.nox_insert_edge_in_hole(p1, p2);
											++num_slow;
											pt_index[p1] = nh->source();
											pt_index[p2] = nh->target();
										}
									}

									inForest.insert(nh);
									++num_total;
									
								}
							}
							DebugAssert(forestMap.is_valid());
	//						printf("Slow inserts: %d. total inserts: %d.  total halfedges: %d.  Total faces: %d.\n",
	//							num_slow, num_total, forestMap.number_of_halfedges(), forestMap.number_of_faces());

							for (Pmwx::Face_iterator f = forestMap.faces_begin(); f != forestMap.faces_end(); ++f)
							{
								f->mTerrainType = terrain_Natural;
								f->mAreaFeature.mFeatType = NO_VALUE;
							}

							for (set<GISHalfedge *>::iterator e = inForest.begin(); e != inForest.end(); ++e)
							{
								DebugAssert(!(*e)->face()->is_unbounded());
								(*e)->face()->mAreaFeature.mFeatType = terrain_ForestPark;
								(*e)->face()->mTerrainType = terrain_Natural;
							}

							Pmwx	baseClone(baseMap);
	#if DEBUG_SHOW_MAP_MERGE_FAIL					
							try {
	#endif						
								TopoIntegrateMaps(&forestMap, &baseClone);
	//							if (DEBUG_FOREST_TREE_MAPS_CTR == forest_type_ctr)
	//								throw forest_type_ctr;
								MergeMaps(forestMap, baseClone, true, NULL, true);
	#if DEBUG_SHOW_MAP_MERGE_FAIL							
							} catch (...) {
								if (ConfirmMessage("Would you like to see the forest map or original road map?",
												"Forest", "Roads"))
									gMap = forestMap;
								else
									gMap = baseClone;
								gFaceSelection.clear();
								gEdgeSelection.clear();
								gVertexSelection.clear();
								WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);							
								return;
							}
	#endif						
							SimplifyMap(forestMap);
							
							for (Pmwx::Face_iterator f = forestMap.faces_begin(); f != forestMap.faces_end(); ++f)
							if (f->mTerrainType == terrain_ForestPark)
							if (f->mAreaFeature.mFeatType == terrain_ForestPark)
							if (!f->is_unbounded())
							{
								GISPolyObjPlacement_t	placement;
								placement.mRepType = *fiter;
								iter = stop = f->outer_ccb();
								do {
									placement.mShape.push_back(iter->target()->point());
									++iter;
									forest_pt_count++;
								} while (iter != stop);
								
//								Polygon2	temp(placement.mShape);
//								InsetPolygon2(temp, NULL, 30.0 * MTR_TO_NM * NM_TO_DEG_LAT, true, placement.mShape, NULL, NULL);
								
								placement.mLocation = placement.mShape.centroid();
								placement.mHeight = 255.0;
								placement.mDerived = false;
								face->mPolyObjs.push_back(placement);					
								forest_poly_count++;
							}
						}
					}
				}
#if DEBUG_SELECT_BAD_FACE				
			} catch (...) { 
				gFaceSelection.clear();
				gFaceSelection.insert(face);
				throw;
			}
#endif			
		}
	}
	
	if (inProg && inProg(1, 2, "Processing faces", 1.0)) return;

	printf("Indexed Tris: %d, Hashed tris = %d, Processed Tris = %d, Processed GT Polys = %d, Hash ops = %d\n", indexed_tris, hashed_tris, processed_tris, processed_gt, hash_fetches);
	printf("Total forest polys = %d, total forest pts = %d\n", forest_poly_count, forest_pt_count);
	printf("Insets: %d good, %d bad, %d total.\n", total_good, total_bad, total_good + total_bad);
	
}