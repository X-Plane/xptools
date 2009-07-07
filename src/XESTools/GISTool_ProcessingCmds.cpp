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

#include "GISTool_ProcessingCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "DEMAlgs.h"
#include "MeshAlgs.h"
#include "ParamDefs.h"
#include "Airports.h"
#include "NetPlacement.h"
#include "DSFBuilder.h"
#include "ObjPlacement.h"
#include "SceneryPackages.h"
#include "TensorRoads.h"
#include "Forests.h"
#include "Zoning.h"
#include "DEMDefs.h"
#include "ObjTables.h"
#include "Hydro.h"
#include "DEMTables.h"
#include "MapAlgs.h"
#include "PerfUtils.h"

static int DoSpreadsheet(const vector<const char *>& args)
{
	if (gVerbose) printf("Setting spreadsheet to %s\n", args[0]);
	gNaturalTerrainFile = args[0];
	if (args.size() > 1)
		gObjPlacementFile = args[1];
	LoadDEMTables();
	if (args.size() > 1)
		LoadObjTables();
	if (args.size() > 1 && gVerbose)
		printf("New prefix is: %s\n",gObjLibPrefix.c_str());
	return 0;
}

static int DoSetMeshLevel(const vector<const char *>& args)
{
	if(gVerbose) printf("Setting mesh level to %s\n", args[0]);
	int ml = atoi(args[0]);
	switch(ml) {
	case 2:		gMeshPrefs.max_points = 10000;		gMeshPrefs.max_error = 30; gMeshPrefs.max_tri_size_m = 8000;	break;
	case 1:		gMeshPrefs.max_points = 30000;		gMeshPrefs.max_error = 12; gMeshPrefs.max_tri_size_m = 3500;	break;
	case 0:		gMeshPrefs.max_points = 25000;		gMeshPrefs.max_error = 15; gMeshPrefs.max_tri_size_m = 5000;	break;
	}
	return 0;
}

static int DoRoads(const vector<const char *>& args)
{
	BuildRoadsForFace(gMap, gDem[dem_Elevation], gDem[dem_Slope], gDem[dem_UrbanDensity], gDem[dem_UrbanRadial], gDem[dem_UrbanSquare], Face_handle(),  gProgress, NULL, NULL);
	return 0;
}


static int DoUpsample(const vector<const char *>& args)
{
	if (gVerbose)	printf("Upsampling environmental parameters...\n");
	UpsampleEnvironmentalParams(gDem, gProgress);
	return 0;
}

static int DoCalcSlope(const vector<const char *>& args)
{
	if (gVerbose)	printf("Calculating slope...\n");
	CalcSlopeParams(gDem, true, gProgress);
	return 0;
}


static int DoCalcMesh(const vector<const char *>& args)
{
	if (gVerbose)	printf("Calculating Mesh...\n");
	TriangulateMesh(gMap, gTriangulationHi, gDem, args[0], gProgress);
	return 0;
}

static int DoBurnAirports(const vector<const char *>& args)
{
	if (gVerbose)	printf("Burning airports into vector map...\n");
	ProcessAirports(gApts, gMap, gDem[dem_Elevation], gDem[dem_UrbanTransport], true, true, true, gProgress);
	return 0;
}

static int DoZoning(const vector<const char *>& args)
{
	if (gVerbose)	printf("Calculating zoning info...\n");
	ZoneManMadeAreas(gMap, gDem[dem_LandUse], gDem[dem_Slope],gApts,gProgress);
	return 0;
}

/*
static int DoHydroReconstruct(const vector<const char *>& args)
{
	if (gVerbose)	printf("Rebuilding vectors from elevation...\n");
	HydroReconstruct(gMap,  gDem,(args.size() <= 1)? NULL : args[1], args[0], gProgress);
	return 0;
}
*/

/*
static int DoHydroSimplify(const vector<const char *>& args)
{
	if (gVerbose)	printf("Simplifying coastlines...\n");
	Bbox2	bounds;

	Point_2	sw, ne;
	CalcBoundingBox(gMap, sw,ne);
	bounds.p1 = cgal2ben(sw);
	bounds.p2 = cgal2ben(ne);
	SimplifyCoastlines(gMap, bounds, gProgress);
	return 0;
}
*/
/*
static int DoBridgeRebuild(const vector<const char *>& args)
{
	if (gVerbose)	printf("Rebuilding roads and bridgse near coastlines...\n");
	BridgeRebuild(gMap, gProgress);
	return 0;
}
*/

static int DoDeriveDEMs(const vector<const char *>& args)
{
	if (gVerbose)	printf("Deriving raster parameters...\n");
	DeriveDEMs(gMap, gDem,gApts, gAptIndex, gProgress);
	return 0;
}

static int DoRemoveDupeObjs(const vector<const char *>& args)
{
	if (gVerbose)	printf("Removing dupe objects...\n");
	RemoveDuplicatesAll(gMap, gProgress);
	return 0;
}

static int DoInstantiateObjs(const vector<const char *>& args)
{
	if (gVerbose)	printf("Instantiating objects...\n");
	vector<PreinsetFace>	insets;

	set<int>				the_types;
	GetObjTerrainTypes		(the_types);

	Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);

	{
		StElapsedTime	time_inset("insets");
		GenerateInsets(gMap, gTriangulationHi, lim, the_types, true, insets, gProgress);
	}
	
	{
		StElapsedTime	time_gt_poly("Place objs");
		InstantiateGTPolygonAll(insets, gDem, gTriangulationHi, gProgress);
	}
	DumpPlacementCounts();
	return 0;

}

static int DoInstantiateForests(const vector<const char *>& args)
{
	if (gVerbose) printf("Instantiating forests...\n");


	vector<PreinsetFace>	insets;
	set<int>				the_types;
	GetAllForestLUs(the_types);
	Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);
	GenerateInsets(gMap, gTriangulationHi, lim, the_types, false, insets, gProgress);
	GenerateForests(gMap, insets, gTriangulationHi, gProgress);
	return 0;
}

static int DoBuildRoads(const vector<const char *>& args)
{
	if (gVerbose) printf("Building roads...\n");
	CalcRoadTypes(gMap, gDem[dem_Elevation], gDem[dem_UrbanDensity],gProgress);
	return 0;
}

static int DoAssignLandUse(const vector<const char *>& args)
{
	if (gVerbose) printf("Assigning land use...\n");
	AssignLandusesToMesh(gDem,gTriangulationHi,args[0],gProgress);

	map<int, int> lus;
	int t = CalcMeshTextures(gTriangulationHi,lus);
	multimap<int,int> sorted;
	for(map<int,int>::iterator l = lus.begin(); l != lus.end(); ++l)
		sorted.insert(multimap<int,int>::value_type(l->second,l->first));
	for(multimap<int,int>::iterator s = sorted.begin(); s != sorted.end(); ++s)
		printf("%f (%d): %s\n", (float) s->first / (float) t, s->first, FetchTokenString(s->second));

	return 0;
}


static int DoBuildDSF(const vector<const char *>& args)
{
	char buf1[1024], buf2[1024];
	if (gVerbose) printf("Build DSF...\n");
	char * b1 = buf1, * b2 = buf2;

	if(strcmp(args[0],"-") == 0) b1 = NULL; else CreatePackageForDSF(args[0], (int) gDem[dem_LandUse].mWest,(int) gDem[dem_LandUse].mSouth, buf1);
	if(strcmp(args[1],"-") == 0) b2 = NULL; else CreatePackageForDSF(args[1], (int) gDem[dem_LandUse].mWest,(int) gDem[dem_LandUse].mSouth, buf2);
	BuildDSF(b1,b2, gDem[dem_LandUse],gTriangulationHi, /*gTriangulationLo,*/ gMap, gProgress);
	return 0;
}

static	GISTool_RegCmd_t		sProcessCmds[] = {
{ "-roads",			0, 0, DoRoads,			"Generate Fake Roads.",				  "" },
{ "-spreadsheet",	1, 2, DoSpreadsheet,	"Set the spreadsheet file.",		  "" },
{ "-mesh_level",	1, 1, DoSetMeshLevel,	"Set mesh complexity.",				  "" },
{ "-upsample", 		0, 0, DoUpsample, 		"Upsample environmental parameters.", "" },
{ "-calcslope", 	0, 0, DoCalcSlope, 		"Calculate slope derivatives.", 	  "" },
{ "-calcmesh", 		1, 1, DoCalcMesh, 		"Calculate Terrain Mesh.", 	 		  "" },
{ "-burnapts", 		0, 0, DoBurnAirports, 	"Burn Airports into vectors.", 		  "" },
{ "-zoning",	 	0, 0, DoZoning, 		"Calculate Zoning info.", 			  "" },
//{ "-hydro",	 		1, 2, DoHydroReconstruct,"Rebuild coastlines from hydro model.",  "" },
//{ "-hydrosimplify", 0, 0, DoHydroSimplify, 	"Simplify Coastlines.", 			  "" },
//{ "-hydrobridge",	0, 0, DoBridgeRebuild,	"Rebuild bridgse after hydro.",		  "" },
{ "-derivedems", 	0, 0, DoDeriveDEMs, 	"Derive DEM data.", 				  "" },
{ "-removedupes", 	0, 0, DoRemoveDupeObjs, "Remove duplicate objects.", 		  "" },
{ "-instobjs", 		0, 0, DoInstantiateObjs, "Instantiate Objects.", 			  "" },
{ "-forests", 		0, 0, DoInstantiateForests, "Build 3-d Forests.",	 		  "" },
{ "-buildroads", 	0, 0, DoBuildRoads, 	"Pick Road Types.", 	  			"" },
{ "-assignterrain", 1, 1, DoAssignLandUse, 	"Assign Terrain to Mesh.", 	 		 "" },
{ "-exportdsf", 	2, 2, DoBuildDSF, 		"Build DSF file.", 					  "" },



{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterProcessingCmds(void)
{
	GISTool_RegisterCommands(sProcessCmds);
}
