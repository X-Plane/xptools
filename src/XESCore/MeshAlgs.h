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
#ifndef MESHALGS_H
#define MESHALGS_H

#include "MeshDefs.h"
#include "ProgressUtils.h"
#include "CompGeomDefs3.h"

struct	PolyRasterizer;
struct	DEMGeo;
class	DEMGeoMap;
class	CDT;
class	Pmwx;

struct	MeshPrefs_t {
	int		max_points;
	float	max_error;
	int		border_match;
	int		optimize_borders;
	float	max_tri_size_m;
	float	rep_switch_m;	
};
extern MeshPrefs_t	gMeshPrefs;

void	TriangulateMesh(Pmwx& inMap, CDT& outMesh, DEMGeoMap& inDEMs, const char * mesh_folder, ProgressFunc inFunc);
void	AssignLandusesToMesh(	DEMGeoMap& inDems, 
								CDT& ioMesh,
								const char * mesh_folder, 
								ProgressFunc inProg);

void 	SetupWaterRasterizer(const Pmwx& inMap, const DEMGeo& inDEM, PolyRasterizer& outRasterizer);
double	HeightWithinTri(CDT& inMesh, CDT::Face_handle tri, double inLon, double inLat);
double	MeshHeightAtPoint(CDT& inMesh, double inLon, double inLat, int hint_id);
void	Calc2ndDerivative(DEMGeo& ioDEM);
int		CalcMeshError(CDT& inMesh, DEMGeo& inElevation, map<float, int>& outError, ProgressFunc inFunc);

// MESH MARCH - Walk from one lat/lon to another in the mesh, returning the heights at the start and end 
// points as well as all points that cross a mesh boundary.

// State struct - where are we?
struct CDT_MarchOverTerrain_t {
	CDT_MarchOverTerrain_t();
	
	CDT::Face_handle	locate_face;		// Face that contains the current point in its interior or boundary.
	CDT::Point			locate_pt;			// Current location
	double				locate_height;		// Vertical at that point
};
	
// Initialize the start point of a march.
void MarchHeightStart(CDT& inMesh, const CDT::Point& inLoc, CDT_MarchOverTerrain_t& march_info);
// March to a new point.  Fills out intermediates with at least two points plus any intermediates.
void MarchHeightGo   (CDT& inMesh, const CDT::Point& inLoc, CDT_MarchOverTerrain_t& march_info, vector<Point3>& intermediates);

#endif
