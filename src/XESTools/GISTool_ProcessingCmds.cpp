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
#include "Forests.h"
#include "Zoning.h"
#include "DEMDefs.h"
#include "Hydro.h"
#include "DEMTables.h"

static int DoSpreadsheet(const vector<const char *>& args)
{
	if (gVerbose) printf("Setting spreadsheet to %s\n", args[0]);
	gNaturalTerrainFile = args[0];
	LoadDEMTables();
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
	TriangulateMesh(gMap, gTriangulationHi, gDem, gProgress);		
	return 0;
}

static int DoBurnAirports(const vector<const char *>& args)
{
	if (gVerbose)	printf("Burning airports into vector map...\n");
	ProcessAirports(gApts, gMap, gDem[dem_Elevation], gDem[dem_UrbanTransport], true, true, gProgress);
	return 0;
}

static int DoZoning(const vector<const char *>& args)
{
	if (gVerbose)	printf("Calculating zoning info...\n");
	ZoneManMadeAreas(gMap, gDem[dem_LandUse], gDem[dem_Slope],gApts,gProgress);
	return 0;
}

static int DoHydroReconstruct(const vector<const char *>& args)
{
	if (gVerbose)	printf("Rebuilding vectors from elevation...\n");
	HydroReconstruct(gMap,  gDem,args[0], gProgress);
	return 0;
}

static int DoHydroSimplify(const vector<const char *>& args)
{
	if (gVerbose)	printf("Simplifying coastlines...\n");
	SimplifyCoastlines(gMap, 5000.0 / (DEG_TO_NM_LAT * DEG_TO_NM_LAT * NM_TO_MTR * NM_TO_MTR), gProgress);
	return 0;
}

static int DoDeriveDEMs(const vector<const char *>& args)
{
	if (gVerbose)	printf("Deriving raster parameters...\n");
	DeriveDEMs(gMap, gDem,gProgress);			
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
	InstantiateGTPolygonAll(gMap, gDem, gTriangulationHi, gProgress);
	DumpPlacementCounts();
	return 0;
	
}

static int DoInstantiateForests(const vector<const char *>& args)
{
	if (gVerbose) printf("Instantiating forests...\n");

	set<GISFace *> faces;
	for (Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	if (!f->is_unbounded())
	if (!f->IsWater())
		faces.insert(f);
	GenerateForests(gMap, faces			, gTriangulationHi, gProgress);
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
	AssignLandusesToMesh(gDem,gTriangulationHi,gProgress);
	return 0;
}


static int DoBuildDSF(const vector<const char *>& args)
{
	char buf2[1024];
	if (gVerbose) printf("Build DSF...\n");

	CreatePackageForDSF(args[0], (int) gDem[dem_LandUse].mWest,(int) gDem[dem_LandUse].mSouth, buf2);				
	BuildDSF(buf2, gDem[dem_LandUse],gTriangulationHi, /*gTriangulationLo,*/ gMap, gProgress);
	return 0;
}

static	GISTool_RegCmd_t		sProcessCmds[] = {
{ "-spreadsheet",	1, 1, DoSpreadsheet,	"Set the spreadsheet file.",		  "" },
{ "-upsample", 		0, 0, DoUpsample, 		"Upsample environmental parameters.", "" },
{ "-calcslope", 	0, 0, DoCalcSlope, 		"Calculate slope derivatives.", 	  "" },
{ "-calcmesh", 		0, 0, DoCalcMesh, 		"Calculate Terrain Mesh.", 	 		  "" },
{ "-burnapts", 		0, 0, DoBurnAirports, 	"Burn Airports into vectors.", 		  "" },
{ "-zoning",	 	0, 0, DoZoning, 		"Calculate Zoning info.", 			  "" },
{ "-hydro",	 		1, 1, DoHydroReconstruct,"Rebuild coastlines from hydro model.",  "" },
{ "-hydrosimplify", 0, 0, DoHydroSimplify, 	"Simplify Coastlines.", 			  "" },
{ "-derivedems", 	0, 0, DoDeriveDEMs, 	"Derive DEM data.", 				  "" },
{ "-removedupes", 	0, 0, DoRemoveDupeObjs, "Remove duplicate objects.", 		  "" },
{ "-instobjs", 		0, 0, DoInstantiateObjs, "Instantiate Objects.", 			  "" },
{ "-forests", 		0, 0, DoInstantiateForests, "Build 3-d Forests.",	 		  "" },
{ "-buildroads", 	0, 0, DoBuildRoads, 	"Pick Road Types.", 	  			"" },
{ "-assignterrain", 0, 0, DoAssignLandUse, 	"Assign Terrain to Mesh.", 	 		 "" },
{ "-exportdsf", 	1, 1, DoBuildDSF, 		"Build DSF file.", 					  "" },



{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterProcessingCmds(void)
{
	GISTool_RegisterCommands(sProcessCmds);
}
