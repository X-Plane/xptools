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
#include "PolyRasterUtils.h"
#include "MeshAlgs.h"
#include "ParamDefs.h"
#include "Airports.h"
#include "NetAlgs.h"
#include "DSFBuilder.h"
#include "ObjPlacement.h"
#include "SceneryPackages.h"
#include "CompGeomUtils.h"
//#include "TensorRoads.h"
#include "MapPolygon.h"
#include "Forests.h"
#include "Zoning.h"
#include "DEMDefs.h"
#include "ObjTables.h"
#include "Hydro.h"
#include "DEMTables.h"
#include "MapAlgs.h"
#include "PerfUtils.h"
#include "BlockFill.h"
#include "RF_Selection.h"
#include "MapTopology.h"
#include "RTree2.h"
#include "MapRaster.h"
#include "MapHelpers.h"
#include "ForestTables.h"

#define NO_FOREST_TYPES 0
#define DEBUG_SHOW_FOREST_POLYS 0

bool pred_want_ag(CDT::Face_handle f)
{
	int t = f->info().terrain;
	if(gNaturalTerrainInfo[t].autogen_mode == URBAN) return true;
	for(set<int>::iterator l = f->info().terrain_border.begin(); l != f->info().terrain_border.end(); ++l)
	if(gNaturalTerrainInfo[*l].autogen_mode == URBAN) return true;
	return false;
}

typedef bool(* want_tri_pred_f)(CDT::Face_handle f);

template<want_tri_pred_f pred>
void SetupRasterizerForMesh(PolyRasterizer<double>& rasterizer, CDT& ioMesh, DEMGeo& dem)
{
	for(CDT::Edge_iterator ei = ioMesh.edges_begin(); ei != ioMesh.edges_end(); ++ei)
	{
		CDT::Face_handle f1 = ei->first;
		CDT::Face_handle f2 = f1->neighbor(ei->second);
		bool want_f1 = !ioMesh.is_infinite(f1) && pred(f1);
		bool want_f2 = !ioMesh.is_infinite(f2) && pred(f2);
		if(want_f1 != want_f2)
		{
			Point2	p1(cgal2ben(f1->vertex(CDT::ccw(ei->second))->point()));
			Point2	p2(cgal2ben(f1->vertex(CDT::cw (ei->second))->point()));
			rasterizer.AddEdge(
				dem.lon_to_x(p1.x()),
				dem.lat_to_y(p1.y()),
				dem.lon_to_x(p2.x()),
				dem.lat_to_y(p2.y()));			
		}
	}
	rasterizer.SortMasters();
}

void	RasterizerFill(PolyRasterizer<double>& rasterizer, DEMGeo& ag_ok, float v)
{
	int y = 0;
	rasterizer.StartScanline(y);
	while (!rasterizer.DoneScan())
	{
		int x1, x2;
		while (rasterizer.GetRange(x1, x2))
		{
			for(int x = x1; x < x2; ++x)
			{
				ag_ok(x,y) = v;
			}
		}
		// Yeah we could be more clever about modulus in the Y axis, but..the rasterizer might
		// be unhappy skipping scanlines with "events" on them.
		++y;
		if (y >= ag_ok.mHeight) break;
		rasterizer.AdvanceScanline(y);
	}
}


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
	case 0:		gMeshPrefs.max_points = 10000;		gMeshPrefs.max_error = 30; gMeshPrefs.max_tri_size_m = 8000;	break;
	case 1:		gMeshPrefs.max_points = 25000;		gMeshPrefs.max_error = 15; gMeshPrefs.max_tri_size_m = 5000;	break;
	case 2:		gMeshPrefs.max_points = 30000;		gMeshPrefs.max_error = 12; gMeshPrefs.max_tri_size_m = 3500;	break;
	case 3:		gMeshPrefs.max_points = 50000;		gMeshPrefs.max_error = 10; gMeshPrefs.max_tri_size_m = 3500;	break;
	case 4:		gMeshPrefs.max_points = 70000;		gMeshPrefs.max_error =  8; gMeshPrefs.max_tri_size_m = 3500;	break;
	}
	return 0;
}

/*
static int DoRoads(const vector<const char *>& args)
{
	BuildRoadsForFace(gMap, gDem[dem_Elevation], gDem[dem_Slope], gDem[dem_UrbanDensity], gDem[dem_UrbanRadial], gDem[dem_UrbanSquare], Face_handle(),  gProgress, NULL, NULL);
	return 0;
}
*/


static int DoUpsample(const vector<const char *>& args)
{
	if (gVerbose)	printf("Upsampling environmental parameters...\n");
	UpsampleEnvironmentalParams(gDem, gProgress);
	return 0;
}

static int DoCalcSlope(const vector<const char *>& args)
{
	if (gVerbose)	printf("Calculating slope...\n");
	if(args.size() > 0)
	{
		gDem[dem_Elevation	 ].calc_normal(gDem[dem_NormalX],gDem[dem_NormalY],gDem[dem_NormalZ],gProgress);
	}
	else
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
	ZoneManMadeAreas(gMap, gDem[dem_LandUse], gDem[dem_ForestType], gDem[dem_ParkType], gDem[dem_Slope],gApts,	Pmwx::Face_handle(), 	gProgress);
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
	DeriveDEMs(gMap, gDem,gApts, gAptIndex, atoi(args[0]), gProgress);
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

	Pmwx	forest_stands;

#if NO_FOREST_TYPES
	// stupid experiment to homogenize forests...to see how it would affect perf.  Leave this off!
	DEMGeo forests(gDem[dem_ForestType]);
	int mt = NO_VALUE;
	for(DEMGeo::iterator i = forests.begin(); i != forests.end(); ++i)
	{
		int v = *i;
		if(v != NO_VALUE)
		{
			if(mt == NO_VALUE)
				mt = v;
			*i = mt;
		}
	}
#else
	const DEMGeo& forests(gDem[dem_ForestType]);
#endif	

	MapFromDEM(forests,0,0,forests.mWidth,forests.mHeight, NO_VALUE, forest_stands,NULL);
	SimplifyMap(gMap, false, gProgress);

	arrangement_simplifier<Pmwx> simplifier;
	simplifier.simplify(forest_stands, 0.003, arrangement_simplifier<Pmwx>::traits_type(), gProgress);

	ForestIndex								forest_index;
	vector<ForestIndex::item_type>			forest_faces;
	forest_faces.reserve(forest_stands.number_of_faces());
	
	for(Pmwx::Face_handle f = forest_stands.faces_begin(); f != forest_stands.faces_end(); ++f)
	if(!f->is_unbounded())
	if(f->data().mTerrainType != NO_VALUE)
	{
		Bbox2	bounds;
		Pmwx::Ccb_halfedge_circulator circ,stop;
		circ=stop=f->outer_ccb();
		do
		{
			bounds += cgal2ben(circ->source()->point());
		}while(++circ != stop);
		forest_faces.push_back(pair<Bbox2,Pmwx::Face_handle>(bounds,f));
	}
	forest_index.insert(forest_faces.begin(),forest_faces.end());
	forest_faces.clear();
	trim(forest_faces);
	
	#if DEBUG_SHOW_FOREST_POLYS
	for(Pmwx::Edge_iterator e = forest_stands.edges_begin(); e != forest_stands.edges_end(); ++e)
	{
		int ee = e->face()->data().mTerrainType;
		if(ee == NO_VALUE)
			ee = e->twin()->face()->data().mTerrainType;
		DebugAssert(ee != NO_VALUE);
		if(gEnumColors.count(ee))
		{
			RGBColor_t& c(gEnumColors[ee]);
			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),c.rgb[0],c.rgb[1],c.rgb[2],c.rgb[0],c.rgb[1],c.rgb[2]);
		} else
			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,0,1, 1,0,1);
	}
	#endif
	
	

	
	
	PROGRESS_START(gProgress, 0, 2, "Creating 3-d.")
	trim_map(gMap);
	int idx = 0;
	int t = gMap.number_of_faces();
	int step = t / 100;
	if(step < 1) step = 1;

	#if OPENGL_MAP
		bool no_sel = gFaceSelection.empty();
	#endif
	
//	map<int,double> by_zone, by_sides;
	
	DEMGeo& ag_ok(gDem[dem_Wizard]);
	
	ag_ok.resize(121,121);
	ag_ok = DEM_NO_DATA;
	ag_ok.copy_geo_from(forests);
	
	PolyRasterizer<double> raster;
	
	SetupRasterizerForMesh<pred_want_ag>(raster, gTriangulationHi, ag_ok);
	RasterizerFill(raster, ag_ok, 1);
	DEMGeo temp(ag_ok);
	dem_copy_buffer_one(ag_ok,temp, DEM_NO_DATA);
	dem_copy_buffer_one(temp,ag_ok, DEM_NO_DATA);
	
	// want it all? slow?  to test?  ok...
	//ag_ok=1;

	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f, ++idx)
	if(!f->is_unbounded())
	if(!f->data().IsWater())
	#if OPENGL_MAP
	if(gFaceSelection.count(f) || no_sel)
	#endif
	{
//		unsigned long long before, after;
//		Microseconds((UnsignedWide *)&before);
		PROGRESS_CHECK(gProgress, 0, 1, "Creating 3-d.", idx, t, step);
		process_block(f,gTriangulationHi, ag_ok, forests, forest_index);
//		Microseconds((UnsignedWide *)&after);
//		double elapsed = (double) (after - before) / 1000000.0;
//		by_zone[f->data().GetZoning()] += elapsed;
//		int ns = count_circulator(f->outer_ccb());
//		by_sides[ns] += elapsed;
	}

	printf("Blocks: %d.  Split: %d. Forests: %d.  Parts: %d\n",  num_block_processed, num_blocks_with_split, num_forest_split, num_line_integ);
	
//	multimap<double, int> r_zone, r_sides;
//	reverse_histo(by_zone,r_zone);
//	reverse_histo(by_sides,r_sides);
//	multimap<double,int>::iterator r;
//	for(r = r_zone.begin(); r != r_zone.end(); ++r)
//		printf("%lf: %s\n", r->first, FetchTokenString(r->second));
//	for(r = r_sides.begin(); r != r_sides.end(); ++r)
//		printf("%lf: %d\n", r->first, r->second);
	
	trim_map(gMap);
	PROGRESS_DONE(gProgress, 0, 1, "Creating 3-d.")


//histo of used ag
	map<int,int>	asset_histo;
	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	for(GISPolyObjPlacementVector::iterator i = f->data().mPolyObjs.begin(); i != f->data().mPolyObjs.end(); ++i)
	if(!IsForestType(i->mRepType))
		asset_histo[i->mRepType]++;

	multimap<int,int>	rh;
	int ta = reverse_histo(asset_histo,rh);
	printf("Total art assets: %d\n", ta);
	for(multimap<int,int>::iterator r = rh.begin(); r != rh.end(); ++r)
		printf("%d (%f): %s\n", r->first, (float) (r->first * 100.0) / (float) ta, FetchTokenString(r->second));

	
//	if (gVerbose)	printf("Instantiating objects...\n");
//	vector<PreinsetFace>	insets;
//
//	set<int>				the_types;
//	GetObjTerrainTypes		(the_types);
//
//	Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);
//
//	{
//		StElapsedTime	time_inset("insets");
//		GenerateInsets(gMap, gTriangulationHi, lim, the_types, true, insets, gProgress);
//	}
//	
//	{
//		StElapsedTime	time_gt_poly("Place objs");
//		InstantiateGTPolygonAll(insets, gDem, gTriangulationHi, gProgress);
//	}
//	DumpPlacementCounts();
	return 0;

}

static int DoInstantiateForests(const vector<const char *>& args)
{
	if (gVerbose) printf("Instantiating forests...\n");


	vector<PreinsetFace>	insets;
	set<int>				the_types;
//	GetAllForestLUs(the_types);
	Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);
	GenerateInsets(gMap, gTriangulationHi, lim, the_types, false, insets, gProgress);
	GenerateForests(gMap, insets, gTriangulationHi, gProgress);
	return 0;
}






static int DoInstantiateObjsForests(const vector<const char *>& args)
{
	if (gVerbose)	printf("Instantiating objects...\n");
	vector<PreinsetFace>	insets;

	set<int>				the_types;
	GetObjTerrainTypes		(the_types);
//	GetAllForestLUs			(the_types);

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
	
	{
		StElapsedTime	time_gt_poly("remove placed objs");
		for(vector<PreinsetFace>::iterator pif = insets.begin(); pif != insets.end(); ++pif)
		{
			SubtractPlaced(*pif);
			SimplifyPolygonMaxMove(pif->second, 0.0001);
		}
	}
	
	{		
		GenerateForests(gMap, insets, gTriangulationHi, gProgress);
	}
	
	return 0;

}


static int DoBuildRoads(const vector<const char *>& args)
{
	if (gVerbose) printf("Building roads...\n");
	CalcRoadTypes(gMap, gDem[dem_Elevation], gDem[dem_UrbanDensity],gDem[dem_Temperature],gDem[dem_Rainfall],gProgress);
	repair_network(gMap);
	return 0;
}

static int DoAssignLandUse(const vector<const char *>& args)
{
	if (gVerbose) printf("Assigning land use...\n");
	AssignLandusesToMesh(gDem,gTriangulationHi,args[0],gProgress);

//	map<int, int> lus;
//	int t = CalcMeshTextures(gTriangulationHi,lus);
//	multimap<int,int> sorted;
//	for(map<int,int>::iterator l = lus.begin(); l != lus.end(); ++l)
//		sorted.insert(multimap<int,int>::value_type(l->second,l->first));
//	for(multimap<int,int>::iterator s = sorted.begin(); s != sorted.end(); ++s)
//		printf("%f (%d): %s\n", (float) s->first / (float) t, s->first, FetchTokenString(s->second));

	return 0;
}


static int DoBuildDSF(const vector<const char *>& args)
{
	char buf1[1024], buf2[1024];
	if (gVerbose) printf("Build DSF...\n");
	char * b1 = buf1, * b2 = buf2;

	if(strcmp(args[0],"-") == 0) b1 = NULL; else CreatePackageForDSF(args[0], (int) gDem[dem_LandUse].mWest,(int) gDem[dem_LandUse].mSouth, buf1);
	if(strcmp(args[1],"-") == 0) b2 = NULL; else CreatePackageForDSF(args[1], (int) gDem[dem_LandUse].mWest,(int) gDem[dem_LandUse].mSouth, buf2);
	BuildDSF(b1,b2, 
		gDem[dem_Elevation],
		gDem[dem_WaterSurface],
		gDem[dem_Bathymetry],
		gDem[dem_UrbanDensity],
		gTriangulationHi, /*gTriangulationLo,*/ gMap, gProgress);
	return 0;
}

static	GISTool_RegCmd_t		sProcessCmds[] = {
//{ "-roads",			0, 0, DoRoads,			"Generate Fake Roads.",				  "" },
{ "-spreadsheet",	1, 2, DoSpreadsheet,	"Set the spreadsheet file.",		  "" },
{ "-mesh_level",	1, 1, DoSetMeshLevel,	"Set mesh complexity.",				  "" },
{ "-upsample", 		0, 0, DoUpsample, 		"Upsample environmental parameters.", "" },
{ "-calcslope", 	0, 1, DoCalcSlope, 		"Calculate slope derivatives.", 	  "" },
{ "-calcmesh", 		1, 1, DoCalcMesh, 		"Calculate Terrain Mesh.", 	 		  "" },
{ "-burnapts", 		0, 0, DoBurnAirports, 	"Burn Airports into vectors.", 		  "" },
{ "-zoning",	 	0, 0, DoZoning, 		"Calculate Zoning info.", 			  "" },
//{ "-hydro",	 		1, 2, DoHydroReconstruct,"Rebuild coastlines from hydro model.",  "" },
//{ "-hydrosimplify", 0, 0, DoHydroSimplify, 	"Simplify Coastlines.", 			  "" },
//{ "-hydrobridge",	0, 0, DoBridgeRebuild,	"Rebuild bridgse after hydro.",		  "" },
{ "-derivedems", 	1, 1, DoDeriveDEMs, 	"Derive DEM data.", 				  "" },
{ "-removedupes", 	0, 0, DoRemoveDupeObjs, "Remove duplicate objects.", 		  "" },
{ "-instobjs", 		0, 0, DoInstantiateObjs, "Instantiate Objects.", 			  "" },
{ "-forests", 		0, 0, DoInstantiateForests, "Build 3-d Forests.",	 		  "" },
{ "-instobjsforests",0, 0, DoInstantiateObjsForests, "Instantiate Objects And Forests.", 			  "" },
{ "-buildroads", 	0, 0, DoBuildRoads, 	"Pick Road Types.", 	  			"" },
{ "-assignterrain", 1, 1, DoAssignLandUse, 	"Assign Terrain to Mesh.", 	 		 "" },
{ "-exportdsf", 	2, 2, DoBuildDSF, 		"Build DSF file.", 					  "" },



{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterProcessingCmds(void)
{
	GISTool_RegisterCommands(sProcessCmds);
}
