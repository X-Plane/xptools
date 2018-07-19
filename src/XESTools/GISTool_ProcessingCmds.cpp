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
#include "SceneryPackages.h"
#include "CompGeomUtils.h"
//#include "TensorRoads.h"
#include "MapPolygon.h"
#include "Hydro2.h"
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
#include "GISUtils.h"
#include "MapCreate.h"
#include "MapOverlay.h"
#include "MobileAutogenAlgs.h"

// Hack to avoid forest pre-processing - to be used to speed up --instobjs for testing AG algos when
// we don't NEED good forest fill.
#define DEBUG_FAST_SKIP_FORESTS 0

// Experimental code to homogenize forests - leave off!
#define NO_FOREST_TYPES 0

// Debug visualization of forest polygons...
#define DEBUG_SHOW_FOREST_POLYS 0

#if !DEV
#if DEBUG_FAST_SKIP_FORESTS || NO_FOREST_TYPES || DEBUG_SHOW_FOREST_POLYS
	#error debug options were left on in a release build ... not good!
#endif
#endif



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

static bool	LowerPriorityFeature(GISPointFeature_t& lhs, GISPointFeature_t& rhs)
{
	if (Feature_IsAirportFurniture(lhs.mFeatType) && Feature_IsAirportFurniture(rhs.mFeatType))
		return lhs.mFeatType < rhs.mFeatType;

	if (Feature_IsAirportFurniture(lhs.mFeatType)) return false;
	if (Feature_IsAirportFurniture(rhs.mFeatType)) return true;

	bool	left_has_height = lhs.mParams.find(pf_Height) != lhs.mParams.end();
	bool	right_has_height = rhs.mParams.find(pf_Height) != rhs.mParams.end();
	if (left_has_height && right_has_height) return lhs.mParams[pf_Height] < rhs.mParams[pf_Height];
	if (left_has_height) return false;
	if (right_has_height) return true;

	return lhs.mFeatType < rhs.mFeatType;
}


#define	OVERLAP_MIN	10

void	RemoveDuplicates(Pmwx::Face_iterator inFace)
{
	int i, j;
	for (i = 0; i < inFace->data().mPointFeatures.size(); ++i)
	{
		for (j = i + 1; j < inFace->data().mPointFeatures.size(); ++j)
		{
			double dist = LonLatDistMeters(
						    CGAL::to_double(inFace->data().mPointFeatures[i].mLocation.x()),
							CGAL::to_double(inFace->data().mPointFeatures[i].mLocation.y()),
							CGAL::to_double(inFace->data().mPointFeatures[j].mLocation.x()),
							CGAL::to_double(inFace->data().mPointFeatures[j].mLocation.y()));
			if (dist < OVERLAP_MIN)
			{
#if LOG_REMOVE_FEATURES
				FILE * f = fopen("removed_features.txt", "a");
#endif
				if (LowerPriorityFeature(inFace->data().mPointFeatures[i],inFace->data().mPointFeatures[j]))
				{
#if LOG_REMOVE_FEATURES
					if(f) fprintf(f, "Removing %d:%s (%d) for %d:%s (%d) - dist was %lf\n",
											i,FetchTokenString(inFace->mPointFeatures[i].mFeatType),
											GetPossibleFeatureHeight(inFace->mPointFeatures[i]),
											j,FetchTokenString(inFace->mPointFeatures[j].mFeatType),
											GetPossibleFeatureHeight(inFace->mPointFeatures[j]), dist);
#endif
					inFace->data().mPointFeatures.erase(inFace->data().mPointFeatures.begin()+i);
					// Start this row over again because index i is now differnet.
					// j will be i+1
					j = i;
				} else {
#if LOG_REMOVE_FEATURES
					if(f) fprintf(f, "Removing %d:%s (%d) for %d:%s (%d) - dist was %lf\n",
											i,FetchTokenString(inFace->mPointFeatures[j].mFeatType),
											GetPossibleFeatureHeight(inFace->mPointFeatures[j]),
											j,FetchTokenString(inFace->mPointFeatures[i].mFeatType),
											GetPossibleFeatureHeight(inFace->mPointFeatures[i]), dist);
#endif
					inFace->data().mPointFeatures.erase(inFace->data().mPointFeatures.begin()+j);
					--j;
				}
#if LOG_REMOVE_FEATURES
				if (f) fclose(f);
#endif
			}
		}
	}
}

void	RemoveDuplicatesAll(
							Pmwx&				ioMap,
							ProgressFunc		inProg)
{
	int ctr = 0;
	PROGRESS_START(inProg, 0, 1, "Removing duplicate objects...")
	for (Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f, ++ctr)
	if (!f->data().IsWater() && !f->is_unbounded())
	{
		PROGRESS_CHECK(inProg, 0, 1, "Removing duplicate objects...", ctr, ioMap.number_of_faces(), 1000)
		RemoveDuplicates(f);
	}
	PROGRESS_DONE(inProg, 0, 1, "Removing duplicate objects...")
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
	
//	build_water_surface_dem(gTriangulationHi, gDem[dem_Elevation], gDem[dem_WaterSurface], gDem[dem_Bathymetry]);

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
	ZoneManMadeAreas(gMap, gDem[dem_Elevation], gDem[dem_LandUse], gDem[dem_ForestType], gDem[dem_ParkType], gDem[dem_Slope],gApts,	Pmwx::Face_handle(), 	gProgress);
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

	ForestIndex								forest_index;

#if !DEBUG_FAST_SKIP_FORESTS

	MapFromDEM(forests,0,0,forests.mWidth,forests.mHeight, 2, 60, 60, NO_VALUE, forest_stands,NULL,false);
	SimplifyMap(forest_stands, false, gProgress, false);

	arrangement_simplifier<Pmwx> simplifier;
	#if UHD_MESH || HD_MESH
		simplifier.simplify(forest_stands, 0.0003, arrangement_simplifier<Pmwx>::traits_type(), gProgress);
	#else
		simplifier.simplify(forest_stands, 0.003, arrangement_simplifier<Pmwx>::traits_type(), gProgress);
	#endif
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
#endif	
	
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
	dem_erode(ag_ok, 1, 1);

	gDem[dem_Wizard] = ag_ok;
	
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


static inline int MAJORITY_RULES(int a, int b, int c, int d)
{
	int la = 1, lb = 1, lc = 1, ld = 1;
	if (a == b) ++la, ++lb;
	if (a == c) ++la, ++lc;
	if (a == d) ++la, ++ld;
	if (b == c) ++lb, ++lc;
	if (b == d) ++lb, ++ld;
	if (c == d) ++lc, ++ld;

	if (la >= lb && la >= lc && la >= ld) return a;
	if (lb >= la && lb >= lc && lb >= ld) return b;
	if (lc >= la && lc >= lb && lc >= ld) return c;
	if (ld >= la && ld >= lb && ld >= lc) return d;
	return a;
}

static inline int MAJORITY_RULES(vector<int> values)
{
	DebugAssert(values.size() == 4);
	return MAJORITY_RULES(values[0], values[1], values[2], values[3]);
}

inline Polygon2 cgal_face_to_ben(Pmwx::Face_handle f)
{
	Polygon2 out;
	Pmwx::Ccb_halfedge_circulator edge = f->outer_ccb();
	const Point2 source_ben = cgal2ben(edge->source()->point());
	const double dsf_min_lon = floor(source_ben.x());
	const double dsf_min_lat = floor(source_ben.y());
	do {
		const Point2 source_ben = cgal2ben(edge->source()->point());
		out.push_back(Point2(doblim(source_ben.x(), dsf_min_lon, dsf_min_lon + 1),
							 doblim(source_ben.y(), dsf_min_lat, dsf_min_lat + 1)));
		--edge;
	} while(edge != f->outer_ccb());
	return out;
}

int choose_ortho_terrain(int land_use, double mean_urbanization)
{
	static const double min_urbanization_for_any_ortho = 0.001;
	static const double min_urbanization_for_ortho_on_non_matching_land_use = 0.1;
	if(land_use != lu_globcover_WATER &&
			mean_urbanization > min_urbanization_for_any_ortho)
	{
		switch(land_use)
		{
			case lu_globcover_URBAN_CROP_TOWN:
			case lu_globcover_URBAN_SQUARE_CROP_TOWN:
			case lu_globcover_URBAN_SQUARE_TOWN:
			case lu_globcover_URBAN_TOWN:
				return terrain_PseudoOrthoTown1;
			case lu_globcover_URBAN_LOW:
			case lu_globcover_URBAN_MEDIUM:
			case lu_globcover_URBAN_SQUARE_LOW:
			case lu_globcover_URBAN_SQUARE_MEDIUM:
			case lu_globcover_URBAN_HIGH:
			case lu_globcover_URBAN_SQUARE_HIGH:
				if(mean_urbanization <= 0.35)
				{
					return terrain_PseudoOrthoOuter1;
				}
				else if(mean_urbanization >= 0.55)
				{
					return terrain_PseudoOrthoInner1;
				}
				else if(land_use == lu_globcover_URBAN_HIGH || land_use == lu_globcover_URBAN_SQUARE_HIGH)
				{
					return terrain_PseudoOrthoInner1;
				}
				else
				{
					return terrain_PseudoOrthoOuter1;
				}
			case lu_globcover_INDUSTRY:
			case lu_globcover_INDUSTRY_SQUARE:
				return terrain_PseudoOrthoIndustrial1;
			default:
				if(mean_urbanization < min_urbanization_for_ortho_on_non_matching_land_use)
				{
					return NO_VALUE;
				}
				else if(mean_urbanization <= 0.15)
				{
					return terrain_PseudoOrthoTown1;
				}
				else if(mean_urbanization <= 0.4)
				{
					return terrain_PseudoOrthoOuter1;
				}
				else
				{
					return terrain_PseudoOrthoInner1;
				}
				break;
		}
	}
	return NO_VALUE;
}

void dump_histogram(const vector<double> &vals)
{
	cout << "histogram([";
	for(vector<double>::const_iterator t = vals.begin(); t != vals.end(); ++t)
	{
		cout << *t << ',';
	}
	cout << "], bins=20)" << endl;
}

Pmwx s_autogen_grid;

struct ag_terrain_dsf_description {
	int dsf_lon;
	int dsf_lat;
	int divisions_lon;
	int divisions_lat;
};

static vector<ag_terrain_dsf_description> initialize_autogen_pmwx()
{
	const DEMGeo & climate_style(gDem[dem_ClimStyle]);
	DebugAssertWithExplanation(climate_style.mWidth > 0 && climate_style.mHeight > 0, "No climate data available");

	const Bbox_2 bounding_box = Bbox_2(climate_style.mWest, climate_style.mSouth, climate_style.mEast, climate_style.mNorth);
	vector<Segment_2> grid_params;
	const int degrees_lon = intround(climate_style.mEast  - climate_style.mWest);
	const int degrees_lat = intround(climate_style.mNorth - climate_style.mSouth);
	DebugAssertWithExplanation(flt_abs(climate_style.mEast  - climate_style.mWest  - degrees_lon) < 0.01, "Working area should be an integer number of degrees longitude");
	DebugAssertWithExplanation(flt_abs(climate_style.mNorth - climate_style.mSouth - degrees_lat) < 0.01, "Working area should be an integer number of degrees latitude");

	vector<ag_terrain_dsf_description> out;
	out.reserve(degrees_lon * degrees_lat);

	for(int degree_lon = 0; degree_lon < degrees_lon; ++degree_lon)
	for(int degree_lat = 0; degree_lat < degrees_lat; ++degree_lat)
	{
		const double lon_min = climate_style.mWest  + degree_lon;
		const double lon_max = climate_style.mWest  + degree_lon + 1;
		const double lat_min = climate_style.mSouth + degree_lat;
		const double lat_max = climate_style.mSouth + degree_lat + 1;

		const int divisions_lon = divisions_longitude_per_degree(g_ortho_width_m, 0.5 * (lat_min + lat_max));
		for(int x = 0; x <= divisions_lon; ++x)
		{
			double lon = lon_min + ((double)x / divisions_lon);
			grid_params.push_back(Segment_2(Point_2(lon, lat_min), Point_2(lon, lat_max)));
		}

		const int divisions_lat = divisions_latitude_per_degree(g_ortho_width_m);
		for(int y = 0; y <= divisions_lat; ++y)
		{
			const double lat = lat_min + ((double)y / divisions_lat);
			grid_params.push_back(Segment_2(Point_2(lon_min, lat), Point_2(lon_max, lat)));
		}

		ag_terrain_dsf_description desc = {lon_min, lat_min, divisions_lon, divisions_lat};
		out.push_back(desc);
	}

	// These are effectively the gridlines for every grid square
	vector<vector<Pmwx::Halfedge_handle> >	halfedge_handles;
	// Edges will be adjacent to faces where we want to set the terrain type
	Map_CreateReturnEdges(s_autogen_grid, grid_params, halfedge_handles);

	return out;
}


struct pair_comparator
{
	bool operator()(const pair<int, int> &a, const pair<int, int> &b) const {
		return a.first * a.second < b.first * b.second;
	}
};

struct special_ter_repeat_rule {
	special_ter_repeat_rule(int min, int max, int ter_1) : min_radius(min), target_max_radius(max) { compatible_terrains.push_back(ter_1); compatible_terrains.push_back(ter_1 + 1); }
	int min_radius; // in terms of grid squares
	int target_max_radius; // in terms of grid squares
	vector<int> compatible_terrains;
};

static bool has_matching_ter_enum_in_radius(int ter_enum, int min_radius, const grid_coord_desc &point, const vector<vector<int> > &tile_assignments)
{
	DebugAssert(!tile_assignments.empty());
	for(int x = max(point.x - min_radius, 0); x <= intmin2(point.x + min_radius, tile_assignments.size()    - 1); ++x)
	for(int y = max(point.y - min_radius, 0); y <= intmin2(point.y + min_radius, tile_assignments[x].size() - 1); ++y)
	{
		if(x != point.x && y != point.y &&
				tile_assignments[x][y] == ter_enum)
		{
				return true;
		}
	}
	return false;
}

static void attempt_assign_special_ter_enum(int ter_enum, const map<int, special_ter_repeat_rule> &special_ter_repeat_rules, const grid_coord_desc &point, vector<vector<int> > &tile_assignments)
{
	DebugAssert(special_ter_repeat_rules.count(ter_enum));
	const special_ter_repeat_rule &rule = special_ter_repeat_rules.at(ter_enum);
	if(contains(rule.compatible_terrains, tile_assignments[point.x][point.y]) &&
			!has_matching_ter_enum_in_radius(ter_enum, rule.min_radius, point, tile_assignments))
	{
		tile_assignments[point.x][point.y] = ter_enum;
		//cout << "Assigned special terrain " << FetchTokenString(ter_enum) << " at (" << point.x << ", " << point.y << ")" << endl;
	}
}

static const int s_pseudorand[] = {918,422,359,512,181,657,814,87,418,288,944,295,56,755,709,56,211,394,408,936,959,752,143,866,664,511,434,562,81,899,22,758,803,145,578,648,874,841,60,738,275,507,899,941,263,156,346,722,889,124,988,458,318,447,189,532,557,209,98,946,909,629,375,246,812,563,489,744,890,334,95,808,249,967,608,803,428,258,962,747,864,875,645,58,518,124,794,868,125,896,203,501,801,557,353,65,646,759,347,413,50,608,442,289,183,34,104,196,458,430,375,992,308,515,120,203,888,626,652,411,495,64,960,991,588,398,815,107,813,948,410,186,444,748,724,195,373,165,474,989,934,580,221,953,542,338,990,819,754,454,360,308,888,634,326,30,599,399,970,3,405,415,712,40,204,779,554,379,145,318,229,540,633,945,215,161,351,457,32,304,210,874,664,0,302,24,492,818,605,760,574,490,282,761,360,992,120,802,449,312,130,573,599,696,12,946,785,82,129,471,438,924,879,224,122,97,420,260,497,581,360,589,7,390,547,985,359,604,408,802,847,388,653,466,148,708,160,924,655,274,508,595,469,964,73,580,490,533,700,0,17,473,842,383,709,735,728,713,931,57,5,555,484,226,216,787,66,753,880,211,434,262,855,389,60,26,889,257,903,65,514,825,868,376,191,617,396,331,681,545,771,469,154,566,36,674,84,771,890,487,15,259,709,103,861,309,359,172,778,336,373,532,365,996,40,28,242,539,854,67,415,178,525,767,243,360,73,175,231,989,26,48,88,41,58,979,496,524,827,889,310,58,629,441,813,606,618,344,537,485,108,885,412,472,572,452,832,829,748,147,798,174,756,293,466,890,170,158,196,107,702,976,451,868,213,429,316,672,808,826,421,444,681,868,525,848,217,261,753,836,589,703,927,523,806,284,518,266,370,168,233,718,985,775,326,484,376,507,76,41,678,233,427,927,505,176,601,259,613,386,784,768,271,902,651,474,265,733,80,286,820,32,715,234,237,653,381,288,922,515,195,329,234,602,725,851,174,117,873,112,650,856,411,883,8,869,490,559,222,513,802,930,884,75,707,513,982,471,764,487,638,805,605,447,765,464,371,143,279,643,764,475,240,767,36,823,763,507,713,739,571,891,355,275,741,689,705,403,688,797,438,181,567,593,98,258,723,288,31,291,585,27,169,753,536,290,284,731,331,463,437,725,530,369,401,485,445,748,449,379,693,104,208,1000,899,900,888,964,4,791,278,791,265,23,507,178,812,356,713,738,950,299,218,84,84,981,444,119,991,464,488,545,853,967,72,917,868,286,11,511,533,386,833,805,214,35,228,289,294,831,469,400,520,549,419,2,747,777,492,919,672,448,404,627,540,773,952,143,83,735,598,54,190,502,559,651,712,380,576,804,401,105,435,298,992,366,222,582,911,888,672,179,755,860,521,948,821,391,237,952,210,694,558,346,240,5,864,846,201,285,609,293,536,157,514,340,694,427,504,669,154,115,623,869,983,910,205,200,651,952,21,249,957,959,31,405,401,392,751,740,437,386,122,542,506,459,400,952,113,202,184,297,994,567,976,628,1,739,636,791,966,717,420,252,184,384,656,457,606,991,830,704,790,689,105,41,964,399,858,129,606,356,334,19,400,708,736,496,756,429,163,596,133,442,845,682,350,551,37,73,319,782,696,85,477,16,889,586,798,720,441,835,212,862,864,595,185,960,744,935,267,870,94,368,281,110,647,622,599,992,286,420,10,632,612,945,742,977,313,415,273,503,768,86,685,314,406,784,767,572,954,241,649,120,930,258,801,154,531,909,986,576,855,435,452,553,145,366,512,847,183,255,40,99,164,92,882,230,643,499,782,393,830,653,868,196,741,88,714,88,13,352,600,602,398,276,417,564,382,907,323,698,919,795,859,775,369,635,434,502,87,197,941,785,599,624,226,464,847,541,707,798,780,517,668,348,132,268,408,624,550,938,650,141,537,697,445,729,66,961,67,887,864,943,233,644,558,113,557,33,883,103,169,865,325,541,204,534,135,896,123,650,983,849,890,114,501,513,163,741,29,793,693,954,19,706,203,194,7,946,284,981,474,13,351,195,982,741,64,877,420,936,964,67,810,64,95,30,240,519,388,908,603,690,511,284,564,818,346,505,7,49,616,213,720,822,244,854,432,400,95,985,741,469,981,854,768,521,440,723,63,333,833,919,27,374,406,504,920,692,871,353,110,121,150,776,188,325,263,73,704,150,291,165,858,225,5,793,471,184,235,481,777,888,173,941,142,600,311,747};

/**
 * A repeatable method for scattering tile placements in apparently random places.
 */
static int pseudorandom_in_range(const special_ter_repeat_rule &rule, const pair<int, int> &dsf, int dim, int dsf_delta_dim)
{
	DebugAssert(rule.min_radius < rule.target_max_radius);
	const int offset = int_abs(dim + dsf.first * dsf.second * dsf_delta_dim * rule.min_radius * rule.target_max_radius);
	int pseudo_rand = s_pseudorand[offset % sizeof(s_pseudorand)];
	return rule.min_radius + pseudo_rand % (rule.target_max_radius - rule.min_radius);
}

static int DoMobileAutogenTerrain(const vector<const char *> &args)
{
	DebugAssertWithExplanation(gDem.count(dem_UrbanDensity), "Tried to add autogen terrain with no DEM urbanization data; you probably need to change the order of your scenery gen commands");
	DebugAssertWithExplanation(gDem.count(dem_LandUse), "Tried to add autogen terrain with no DEM land use data; you probably need to change the order of your scenery gen commands");
	DebugAssertWithExplanation(gDem.count(dem_ClimStyle), "No climate data loaded");

	const vector<ag_terrain_dsf_description> all_dsfs = initialize_autogen_pmwx();

	typedef map<pair<int, int>, vector<vector<int> >, pair_comparator> dsf_to_ortho_terrain_map;
	dsf_to_ortho_terrain_map ortho_terrain_by_dsf;
	for(vector<ag_terrain_dsf_description>::const_iterator dsf = all_dsfs.begin(); dsf != all_dsfs.end(); ++dsf)
	{
		const pair<int, int> dsf_lon_lat = make_pair(dsf->dsf_lon, dsf->dsf_lat);
		vector<vector<int> > &ortho_terrain_assignments = ortho_terrain_by_dsf[dsf_lon_lat];
		ortho_terrain_assignments.resize(dsf->divisions_lon);
		for(int lon_offset = 0; lon_offset < dsf->divisions_lon; ++lon_offset)
		{
			ortho_terrain_assignments[lon_offset].resize(dsf->divisions_lat);
		}
	}

	//--------------------------------------------------------------------------------------------------------
	// PASS 1
	// Choose a "starting point" orthophoto for each tile in the grid
	// based on land class and urban density data.
	// If we stopped here, it would look like shit, because:
	//   a) there would be no nice transitions between tile types
	//   b) we would never have used the "special" tiles of each type (e.g., sports stadiums)
	//   c) we wouldn't have done anything about standalone tiles, which just look awkward
	//--------------------------------------------------------------------------------------------------------
	const int dx = ortho_terrain_by_dsf.begin()->second.size();
	const int dy = ortho_terrain_by_dsf.begin()->second[0].size();
	for(dsf_to_ortho_terrain_map::iterator dsf = ortho_terrain_by_dsf.begin(); dsf != ortho_terrain_by_dsf.end(); ++dsf)
	for(int x = 0; x < dx; ++x)
	for(int y = 0; y < dy; ++y)
	{
		const pair<int, int> &dsf_lon_lat = dsf->first;

		double sum_urbanization = 0;
		vector<int> land_uses;
		int num_corners = 0;
		for(int corner_x = 0; corner_x < 2; ++corner_x)
		for(int corner_y = 0; corner_y < 2; ++corner_y)
		{
			const double lon = dsf_lon_lat.first + (double)(x + corner_x) / dx;
			const double lat = dsf_lon_lat.second + (double)(y + corner_y) / dy;
			const float urbanization = gDem[dem_UrbanDensity].value_linear(lon, lat);
			if(urbanization != DEM_NO_DATA)
			{
				sum_urbanization += urbanization;
				++num_corners;
			}

			const int land_use = intround(gDem[dem_LandUse].xy_nearest_raw(lon, lat));
			if(land_use != DEM_NO_DATA)
			{
				land_uses.push_back(land_use);
			}
		}

		if(num_corners > 0)
		{
			const double mean_urbanization = sum_urbanization / num_corners;
			const int land_use = MAJORITY_RULES(land_uses);

			const int ter_enum = choose_ortho_terrain(land_use, mean_urbanization);
			if(ter_enum != NO_VALUE)
			{
				// The variant gives us the perfect checkerboard tiling of the two "normal" variants of each ortho
				const int variant = (x + y) % 2;
				dsf->second[x][y] = ter_enum + variant;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------
	// PASS 2
	// Go through the existing map looking for point features which would correspond to our "special" orthophotos.
	//--------------------------------------------------------------------------------------------------------
	map<int, special_ter_repeat_rule> special_ter_repeat_rules;
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoInnerPark,				special_ter_repeat_rule(2, 5, terrain_PseudoOrthoInner1)));
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoInnerStadium,			special_ter_repeat_rule(3, 7, terrain_PseudoOrthoInner1)));
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoOuterBuilding,			special_ter_repeat_rule(2, 5, terrain_PseudoOrthoOuter1)));
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoOuterStadium,			special_ter_repeat_rule(3, 7, terrain_PseudoOrthoOuter1)));
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoTownLgBuilding,		special_ter_repeat_rule(2, 3, terrain_PseudoOrthoTown1)));
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoTownSpecial2,			special_ter_repeat_rule(2, 3, terrain_PseudoOrthoTown1)));
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoIndustrialSpecial1,	special_ter_repeat_rule(2, 3, terrain_PseudoOrthoIndustrial1)));
	special_ter_repeat_rules.insert(make_pair(terrain_PseudoOrthoIndustrialSpecial2,	special_ter_repeat_rule(2, 3, terrain_PseudoOrthoIndustrial1)));

	vector<int> large_building_features;
	large_building_features.push_back(feat_CommercialOffice);
	large_building_features.push_back(feat_CommercialShoppingPlaza);
	large_building_features.push_back(feat_Government);
	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	{
		if(!f->is_unbounded())
		{
			const GIS_face_data &fd = f->data();
			const Point2 centroid = cgal_face_to_ben(f).centroid();
			const pair<int, int> dsf_coords = make_pair(floor(centroid.x()), floor(centroid.y()));
			dsf_to_ortho_terrain_map::iterator dsf = ortho_terrain_by_dsf.find(dsf_coords);
			if(dsf != ortho_terrain_by_dsf.end())
			{
				const grid_coord_desc grid_pt = get_orth_grid_xy(centroid);
				int &ter_enum = dsf->second[grid_pt.x][grid_pt.y];
				if(ter_enum != NO_VALUE)
				{
					for(GISPointFeatureVector::const_iterator i = fd.mPointFeatures.begin(); i != fd.mPointFeatures.end(); ++i)
					{
						if(contains(large_building_features, i->mFeatType))
						{
							attempt_assign_special_ter_enum(terrain_PseudoOrthoOuterBuilding, special_ter_repeat_rules, grid_pt, dsf->second);
							attempt_assign_special_ter_enum(terrain_PseudoOrthoTownLgBuilding, special_ter_repeat_rules, grid_pt, dsf->second);
						}
						else if(i->mFeatType == feat_GolfCourse)
						{
							attempt_assign_special_ter_enum(terrain_PseudoOrthoInnerPark, special_ter_repeat_rules, grid_pt, dsf->second);
						}
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------
	// PASS 3
	// Ensure we're using the "special" terrain types with "just enough" regularity...
	// They should look "randomly" placed, and not too frequent.
	// (You shouldn't have stadiums too close to each other!)
	// At the same time, we don't want to *never* use the special terrain types!
	//--------------------------------------------------------------------------------------------------------
	for(map<int, special_ter_repeat_rule>::const_iterator rule = special_ter_repeat_rules.begin(); rule != special_ter_repeat_rules.end(); ++rule)
	{
		const int dy_for_randomization = dx == dy ? dy + 13 : dy;
		for(dsf_to_ortho_terrain_map::iterator dsf = ortho_terrain_by_dsf.begin(); dsf != ortho_terrain_by_dsf.end(); ++dsf)
		for(int x = 0; x < dx; x += pseudorandom_in_range(rule->second, dsf->first, x, dx))
		for(int y = 0; y < dy; y += pseudorandom_in_range(rule->second, dsf->first, y, dy_for_randomization))
		{
			grid_coord_desc pt = {x, y, dx, dy, dsf->first.first, dsf->first.second};
			attempt_assign_special_ter_enum(rule->first, special_ter_repeat_rules, pt, dsf->second);
		}
	}

	//--------------------------------------------------------------------------------------------------------
	// FINALLY
	// Assign the selected terrain types to the Pmwx
	//--------------------------------------------------------------------------------------------------------
	for(Pmwx::Face_handle f = s_autogen_grid.faces_begin(); f != s_autogen_grid.faces_end(); ++f)
	{
		if(!f->is_unbounded())
		{
			GIS_face_data &fd = f->data();
			fd.mTerrainType = NO_VALUE;
			fd.mTemp1 = NO_VALUE;
			fd.mTemp2 = NO_VALUE;
			#if OPENGL_MAP
				memset(fd.mGLColor, sizeof(fd.mGLColor), 0);
			#endif

			const Point2 centroid = cgal_face_to_ben(f).centroid();
			const pair<int, int> dsf = make_pair(floor(centroid.x()), floor(centroid.y()));
			const grid_coord_desc grid_pt = get_orth_grid_xy(centroid);
			DebugAssert(ortho_terrain_by_dsf.count(dsf) || cgal_face_to_ben(f).bounds().area() == 0);
			if(ortho_terrain_by_dsf.count(dsf))
			{
				DebugAssert(ortho_terrain_by_dsf[dsf].size() > grid_pt.x);
				DebugAssert(ortho_terrain_by_dsf[dsf][grid_pt.x].size() > grid_pt.y);
				const int ter_enum = ortho_terrain_by_dsf[dsf][grid_pt.x][grid_pt.y];
				if(ter_enum != NO_VALUE)
				{
					f->set_contained(true);
					fd.mTerrainType = ter_enum;
					//fd.mOverlayType = terrain_PseudoOrthophoto;		// Ben says: This would make an overlay.
					
					Pmwx::Ccb_halfedge_circulator edge = f->outer_ccb();
					do {
						// Must burn EVERY grid square.  This is mandatory for overlays so they aren't optimized away,
						// and for base terrain so that adjacent terrain gets a dividing edge in the mesh.
						edge->data().mParams[he_MustBurn] = 1;
						--edge;
					} while(edge != f->outer_ccb());
				}
			}
		}
	}
	return 0;
}

static int MergeTylersAg(const vector<const char *>& args)
{
	// Tyler says:
	// I think we need 3 layers here (from bottom to top):
	// 0. The global map as it exists before we manipulate it---no faces marked as "contained"
	// 1. The AG map overlaid on top of that, with all faces marked as "contained"
	//     - This will blit over important features in urban areas like water and airports!
	// 2. The original map *again*, with its important features (like water & airports) marked as contained
	//     - This lets us recover what the AG tried to clear
	Pmwx intermediate_autogen_on_top;
	MapOverlay(gMap, s_autogen_grid, intermediate_autogen_on_top);

	vector<int> terrain_types_to_keep;
	terrain_types_to_keep.push_back(terrain_Water);
	terrain_types_to_keep.push_back(terrain_VisualWater);
	terrain_types_to_keep.push_back(terrain_Airport);
	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	{
		GIS_face_data &fd = f->data();
		if(!f->is_unbounded() && contains(terrain_types_to_keep, fd.mTerrainType))
		{
			f->set_contained(true);
		}
	}

	Pmwx final;
	MapOverlay(intermediate_autogen_on_top, gMap, final);
	gMap = final;
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
	
	if (gVerbose) printf("Finding rural roads...\n");
	PatchCountryRoads(gMap, gTriangulationHi,gDem[dem_UrbanDensity]);


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
//		gDem[dem_WaterSurface],
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
{ "-autogenterrain",	0, 0, DoMobileAutogenTerrain,	"Mobile 'orthophoto'-based autogen.", "" },
{ "-buildroads", 	0, 0, DoBuildRoads, 	"Pick Road Types.", 	  			"" },
{ "-assignterrain", 1, 1, DoAssignLandUse, 	"Assign Terrain to Mesh.", 	 		 "" },
{ "-exportdsf", 	2, 2, DoBuildDSF, 		"Build DSF file.", 					  "" },

{ "-merge_ag_terrain",	0, 0, MergeTylersAg,	"Merge AG terrain into the map.", "" },


{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterProcessingCmds(void)
{
	GISTool_RegisterCommands(sProcessCmds);
}
