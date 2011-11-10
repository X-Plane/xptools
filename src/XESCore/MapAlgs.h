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
#ifndef MAPALGS_H
#define MAPALGS_H

#include "XESConstants.h"
#define kOverscale 1.0

#include "MapDefs.h"
#include "ProgressUtils.h"

#include "MeshDefs.h"

template<typename Number>
struct	PolyRasterizer;
struct	DEMGeo;



//these need to go

/*
 * CCBToPolygon
 *
 * Given a CCB (ring) in a map, this converts it to a simple polygon.  You can pass a weight function in which case a second array
 * of per-side weights (0th weight is the CCB edge passed in) are also built up.
 *
 */
//void		CCBToPolygon(Halfedge_const_handle ccb, Polygon2& outPolygon, vector<double> * road_types, double (* weight_func)(Halfedge_const_handle edge), Bbox2 * outBounds);

/*
 * FaceToComplexPolygon
 *
 * Given a face, this builds a "complex" polygon (multiple rings, first is outside CCW, rest are inside CW) from each ring (the outer CCB and holes) of the polygon.
 * Like above a weighting function is used.
 *
 */
//void	FaceToComplexPolygon(Face_const_handle face, vector<Polygon2>& outPolygon, vector<vector<double> > * road_types, double (* weight_func)(Halfedge_const_handle edge), Bbox2 * outBounds);
/*
 * ComplexPolygonToPmwx
 *
 * Given a complex polygon (outer bounds and holes) this builds a PMWX with the same topolgoy.  InTerrain and outTerrain are terrain types assigned to space
 * in the PMWX inside and outside the complex polygon.
 *
 */
//Face_handle	ComplexPolygonToPmwx(const vector<Polygon2>& inPolygons, Pmwx& outPmwx, int inTerrain, int outTerain);

/************************************************************************************************
 * MAP EDITING
 ************************************************************************************************
 *
 * Our fundamental map editing operations are:
 *
 * Swap - given two rings of equal geometry in two maps, swap what's inside and on them.
 * Merge - insert all of one map into another.
 *
 * We build all of our algorithms up from these.  One other noteworthy algorithm: "TopoIntegration"
 * just means pre-inserting the intersections of all edges from two maps into both so that we know
 * that there are no edge-edge intersections between the two.  See the topointegrate notes for why
 * we would want this.
 *
 * Besides the algs derived from swap, and merge, some of the "cleaning" algorithms simply go around
 * deleting edges.  These routines are slow because the topological transformations they can induce
 * are complex and must be computed one-at-a-time.
 *
 * (Where possible algorithms that work on defined extents like "crop" use swap ops, which are MUCH
 * faster.  While swap algs are still linear to the number of elements "swapped", almost all
 * topological operations - inserting and removing edges fundamentally change topology - have a time
 * complexity greater than constant, so a topological op on a linear set is worse than linear time,
 * often by a lot!)
 *
 */

#if DEV
void	RebuildMap(
			Pmwx&			in_map,
			Pmwx&			out_map);
#endif

// keep - but...Point_2 based?
/*
 * CropMap
 *
 * Crop a map along a square box and keep one half.  This is a higher level version of the other
 * crop-map.
 *
 * Performance: O(N) + O(K*M) where
 *	N = number of halfedges inserted to form the cropped boundary and
 *  K = number of halfedges outside crop that are deleted and
 *  M = averge number of halfedges in a CCB.
 *
 */
void	CropMap(
			Pmwx&			ioMap,
			double			inWest,
			double			inSouth,
			double			inEast,
			double			inNorth,
			bool			inKeepOutside,	// If true, keep outside crop zone (cut a hole), otherwise keep only inside (normal crop)
			ProgressFunc	inProgress);

// Keep
//void	CutInside(
//			Pmwx&				ioMap,
//			const Polygon_2&	inBoundary,
//			bool				inWantOutside,
//			ProgressFunc		inProgress);

// Lose - can't do quick
/*
 * CropMap - advanced form.
 *
 * Pass in a map, and a blank map to receive what is inside the ring passed in inRingCCW.
 * The ring MUST be counterclockwise.  This gives you more precise control over what is cropped
 * and what is saved and chucked.  (This can be used to do cut, copy, paste, clear, etc.)
 *
 * (This is a convenient high level swap.)
 *
 */
void	CropMap(
			Pmwx&					ioMap,
			Pmwx&					outCutout,
			const vector<Point_2>&	inRingCCW,
			ProgressFunc			inProgress);

//void	SwapFace(
//			Pmwx&			inMaster,
//			Pmwx&			inSlave,
//			Face_handle		inFace,
//			ProgressFunc	inFunc);


//void	SwapMaps(	Pmwx& 							ioMapA,
//					Pmwx& 							ioMapB,
//					const vector<Halfedge_handle>&	inBoundsA,
//					const vector<Halfedge_handle>&	inBoundsB);

//void TopoIntegrateMaps(Pmwx * mapA, Pmwx * mapB);

//Face_handle SafeInsertRing(Pmwx * inPmwx, Face_handle parent, const vector<Point2>& inPoints);


/*
 * MapSimplify
 *
 * Given a map and an error metric (in degrees) this routine merges edges and removes vertices
 * such that the maximum distance from any point on any edge to its original position is less
 * than metric, and the map is not transformed in any way topologically.  This is the best way
 * to de-rez a map.
 *
 */
#if CGAL_BETA_SIMPLIFIER
void MapSimplify(Pmwx& pmwx, double metric);
#endif
/*
 * MapDesliver
 *
 * A sliver is a very long thin map face, typically induced by having slightly conflicting 
 * vector data.  This routine attempts to prevent slivering problems by changing the underlying
 * face properties of such slivers so that they will merge with their neighbors.  Generally after
 * a desliver the map should be simplified to eliminate unnecessary half-edges.  
 *
 * The metric is a degrees-lat/lon metric for the acceptable minimum radius of any feature...any
 * smaller feature will be a candidate for deslivering.
 * 
 * Returns the number of changed faces.
 *
 */
int MapDesliver(Pmwx& pmwx, double metric, ProgressFunc func);

int RemoveOutsets(Pmwx& io_map, double max_size, double max_area);
int RemoveIslands(Pmwx& io_map, double max_area);
int KillWetAntennaRoads(Pmwx& io_map);
int LandFillStrandedRoads(Pmwx& io_map, double dist_lo, double dist_hi);

int KillSliverWater(Pmwx& pmwx, double metric, ProgressFunc func);
int KillSlopedWater(Pmwx& pmwx, 
			DEMGeo& elev, 
			DEMGeo& landuse, 
			int max_horizontal_err_pix,
			int	minimum_lu_size_pix,
			float maximum_lu_err_pix,
			double zlimit, 
			ProgressFunc func);


/************************************************************************************************
 * MAP ANALYSIS AND RASTERIZATION/ANALYSIS
 ************************************************************************************************/

// Keep
/*
 * CalcBoundingBox
 *
 * Given a map, find its boundingbox.
 *
 */
void	CalcBoundingBox(
			const Pmwx&		inMap,
			Point_2&		sw,
			Point_2&		ne);

// These next routines, move elsewhere to just GIS-related as opposed to comp-geom related

/*
 * GetMapFaceAreaMeters
 *
 * Given a map in lat/lon and a face, return its area in meters.
 *
 */
double	GetMapFaceAreaMeters(const Face_handle f, Bbox2 * out_bounds = NULL);

/*
 * GetMapFaceAreaDegrees
 *
 * Given a map in lat/lon and a face, return its area in degrees.
 *
 */
double	GetMapFaceAreaDegrees(const Face_handle f);


/*
 * GetMapEdgeLengthMeters
 *
 * Given an edge in lat/lon, return is length in meters.
 *
 */
double	GetMapEdgeLengthMeters(const Halfedge_handle e);

/*
 * GetParamAverage
 * GetParamHistogram
 *
 * Given a face and a raster DEM in the same coordinate system, find either the min, max and average
 * of the value in the DEM over the face area, or find a full histogram for the face erea.
 * Please note that the histogram is NOT initialized; so that you can run it on multiple faces.
 *
 */
float	GetParamAverage(const Face_handle f, const DEMGeo& dem, float * outMin, float * outMax);
int		GetParamHistogram(const Face_handle f, const DEMGeo& dem, map<float, int>& outHistogram);


// Move these to some kind of DEM-map interaction file
/*
 * ClipDEMToFaceSet
 *
 * Given a set of faces, copy the points in the src DEM to the dest DEM only if we're in the faces.
 * Return the bounds (inclusive min, exclusive max) of the copied area.  Returns true if any points
 * were copied, otherwise the X and Y params may nto be valid.
 *
 */
bool	ClipDEMToFaceSet(const set<Face_handle>& inFaces, const DEMGeo& inSrcDEM, DEMGeo& inDstDEM, int& outX1, int& outY1, int& outX2, int& outY2);

/*
 * SetupRasterizerForDEM
 *
 * Given a face (or edge set containing a finite area) and a DEM in the same coordinates, set up the polygon
 * rasterizer to rasterize the DEM over the face set.
 *
 * This is useful for preparing iterating to iterate over every DEM point contained within a face or edge set.
 * The lowest Y coordinate in the DEM that is within the rasterized area is returned as a good start value to
 * the rasterize outer loop.
 *
 */
int		SetupRasterizerForDEM(const Face_handle f, const DEMGeo& dem, PolyRasterizer<double>& rasterizer);
int		SetupRasterizerForDEM(const set<Halfedge_handle>& inEdges, const DEMGeo& dem, PolyRasterizer<double>& rasterizer);

/************************************************************************************************
 * POLYGON TRUNCATING AND EDITING
 ************************************************************************************************/
// LOSE

/*
 * InsetPmwx
 *
 * WARNING WARNING WARNING: This routine's implementation is INCORRECT and UNRELIABLE.
 * The SK_Skeleton APIs are designed to provide correct straight-skeleton-based inset
 * calculations; InsetPmwx is an old attempt to write an inset routine without proper
 * skeleton treatmant and should not be used!
 *
 * Given a Pmwx and a face, inset the face.  This is a complex and powerful inset routine;
 * unlike InsetPolygon, it can handle (1) holes in the polygon and (2) generacy (e.g. the
 * resulting polygon may be massively different than the first).
 *
 * The API is pretty strange: you pass in a map with a face to be inset.  While there is no
 * requirement that this face be in the unbounded face, it is strongly recommended for performance
 * and stability that you pass in a Pmwx where the face is the only hole in the CCB.
 *
 * Make sure the face's terrain type is not water.  Set each halfedge of the face to the inset
 * distance in meters.
 *
 * On output, you will get the map back where every bounded face whose terrain type is not water
 * is an empty usable area not consumed by the inset.  You can then iterate across all bounded
 * faces checking terrain type to receive a bunch of polygons (with potential holes) that represent
 * the non-inset space.
 *
 * There is no guarantee that any given edge will continue to exist or that the face will not be
 * destroyed and recreated.  The algorithm has to do a lot of processing to produce the inset, so
 * the map gets pretty badly beaten up.
 *
 */
// Ben says: to be replaced with constructive inseting of polygon sets using boolean ops
//void	InsetPmwx(Face_handle inFace, Pmwx& outMap, double dist);

/************************************************************************************************
 * INLINE STUFF
 ************************************************************************************************/

/*

template <typename InputIterator>
Point_2	FindRefPoint(InputIterator begin, InputIterator end)
{
	if (begin == end) return CGAL::ORIGIN;
	Point_2	best = *begin;
	for (InputIterator i = begin; i != end; ++i)
	{
		if ((*i).x < best.x)
			best = Point_2((*i).x, best.y);
		if ((*i).y < best.y)
			best = Point_2(best.x, (*i).y);
	}

	return best;
}

*/

/*
template <typename ForwardIterator>
void LatLonToLocalMeters(ForwardIterator begin, ForwardIterator end, const Point_2& ref)
{
	Vector__2	offset(ref);
	double		DEG_TO_MTR_LON = DEG_TO_MTR_LAT * cos(CGAL::to_double(ref.y) * DEG_TO_RAD);
	for (ForwardIterator i = begin; i != end; ++i)
	{
		Point_2 p = *i - offset;
		Point_2	p2(p.x * DEG_TO_MTR_LON / kOverscale,
				   p.y * DEG_TO_MTR_LAT / kOverscale);
		*i = p2;
	}
}

template <typename ForwardIterator>
void LocalMetersToLatLon(ForwardIterator begin, ForwardIterator end, const Point_2& ref)
{
	Vector_2	offset(ref);
	double		DEG_TO_MTR_LON = DEG_TO_MTR_LAT * cos(CGAL::to_double(ref.y) * DEG_TO_RAD);
	for (ForwardIterator i = begin; i != end; ++i)
	{
		Point_2	p ((*i).x * kOverscale / DEG_TO_MTR_LON,
				   (*i).y * kOverscale / DEG_TO_MTR_LAT);
		Point_2 p2 = p + offset;
		*i = p2;
	}
}
*/

// Stubs?  or move to map draw?


#endif
