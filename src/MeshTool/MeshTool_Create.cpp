/*
 * Copyright (c) 2009, Laminar Research.
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

#if __POWERPC__
	#pragma optimization_level 0
#endif



#include "MeshTool_Create.h"
#include <stdarg.h>
#include "MapDefs.h"
#include "DEMDefs.h"
#include "MeshDefs.h"
#include "AptDefs.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "MapOverlay.h"
#include "XESIO.h"
#include "MemFileUtils.h"
#include "MeshAlgs.h"
#include "MapAlgs.h"
#include "DSFBuilder.h"
#include "MapTopology.h"
#include "BitmapUtils.h"
#include "DEMAlgs.h"
#include "Zoning.h"
#include "NetPlacement.h"
#include "NetAlgs.h"
#include "ObjPlacement.h"
#include "ObjTables.h"
#include "ShapeIO.h"
#include "FileUtils.h"
#include "NetAlgs.h"

static DEMGeoMap			sDem;
static CDT					sMesh;
static AptVector			sApts;
static AptIndex				sAptIndex;
static double				sBounds[4];
static string				g_qmid_prefix;

static int					sMakeDDS = 0;

static Pmwx *							the_map = NULL;
static int								layer_type = NO_VALUE;
static Polygon_2						ring, the_hole;
static vector<Polygon_2>				holes;
static vector<Polygon_with_holes_2>		layer;
static Polygon_set_2					layer_mask;
static vector<X_monotone_curve_2>		net;
static int								zlimit=0,zmin=30000,zmax=-2000;
static MT_Error_f						err_f=NULL;
static int								net_type=NO_VALUE;

static int								num_cus_terrains=0;

static void die_err(const char * msg, ...)
{
	va_list l;
	va_start(l,msg);
	if(err_f)
		err_f(msg,l);
	else
		vfprintf(stderr,msg,l);
}

void MT_StartCreate(const char * xes_path, const DEMGeo& in_dem, MT_Error_f err_handler)
{
	DebugAssert(err_handler != NULL);
	DebugAssert(the_map == NULL);
	err_f = err_handler;
	the_map = new Pmwx;

	MFMemFile *	xes = MemFile_Open(xes_path);
	if(xes == NULL)
	{
		MemFile_Close(xes);
		die_err("ERROR: could not read XES file:%s\n", xes_path);
		return;
	}

	{
		ReadXESFile(xes, NULL, NULL, &sDem, NULL, ConsoleProgressFunc);
		DEMGeo& lu(sDem[dem_LandUse]);
		for(int y = 0; y < lu.mHeight; ++y)
		for(int x = 0; x < lu.mWidth ; ++x)
			if (lu.get(x,y) == lu_usgs_INLAND_WATER ||
				lu.get(x,y) == lu_usgs_SEA_WATER)
			lu(x,y) = lu_globcover_WATER;
	}
	MemFile_Close(xes);
	
	sDem[dem_Elevation] = in_dem;
	
	// First: snap-round the DEM to the tile bounds.
	sDem[dem_Elevation].mWest = round(sDem[dem_Elevation].mWest );
	sDem[dem_Elevation].mEast = round(sDem[dem_Elevation].mEast );
	sDem[dem_Elevation].mNorth= round(sDem[dem_Elevation].mNorth);
	sDem[dem_Elevation].mSouth= round(sDem[dem_Elevation].mSouth);
	
	// Err check: DEM bounds don't match our XES tile?  That's a fatal error...almost certainly
	// indicates a wrong set of resources put together, or a very, very borked GeoTiff!
	
	if(sDem[dem_Elevation].mWest != sDem[dem_Temperature].mWest)	{ die_err("Error: west edge of DEM and XES data do not match.  DEM is: %lf, XES is: %lf\n",sDem[dem_Elevation].mWest,sDem[dem_Temperature].mWest); return; }
	if(sDem[dem_Elevation].mEast != sDem[dem_Temperature].mEast)	{ die_err("Error: east edge of DEM and XES data do not match.  DEM is: %lf, XES is: %lf\n",sDem[dem_Elevation].mEast,sDem[dem_Temperature].mEast); return; }
	if(sDem[dem_Elevation].mNorth != sDem[dem_Temperature].mNorth)	{ die_err("Error: north edge of DEM and XES data do not match.  DEM is: %lf, XES is: %lf\n",sDem[dem_Elevation].mNorth,sDem[dem_Temperature].mNorth); return; }
	if(sDem[dem_Elevation].mSouth != sDem[dem_Temperature].mSouth)	{ die_err("Error: south edge of DEM and XES data do not match.  DEM is: %lf, XES is: %lf\n",sDem[dem_Elevation].mSouth,sDem[dem_Temperature].mSouth); return; }

	// Err check: if the DEM had to be snap-rounded by more than 1%, that's probably a borked GeoTiff.  Barf.
	#define SNAP_ERR 0.01
	if(fabs(sDem[dem_Elevation].mWest -in_dem.mWest ) > SNAP_ERR)	{ die_err("Error: the west edge of your DEM (%lf) is more than 1% away from the tile boundary.  This is probably a bad mesh.\n", in_dem.mWest ); return; }
	if(fabs(sDem[dem_Elevation].mEast -in_dem.mEast ) > SNAP_ERR)	{ die_err("Error: the east edge of your DEM (%lf) is more than 1% away from the tile boundary.  This is probably a bad mesh.\n", in_dem.mEast ); return; }
	if(fabs(sDem[dem_Elevation].mNorth-in_dem.mNorth) > SNAP_ERR)	{ die_err("Error: the north edge of your DEM (%lf) is more than 1% away from the tile boundary.  This is probably a bad mesh.\n", in_dem.mNorth); return; }
	if(fabs(sDem[dem_Elevation].mSouth-in_dem.mSouth) > SNAP_ERR)	{ die_err("Error: the south edge of your DEM (%lf) is more than 1% away from the tile boundary.  This is probably a bad mesh.\n", in_dem.mSouth); return; }

	// aArnings if the alignment is, um, goofy!
	if(sDem[dem_Elevation].mWest !=in_dem.mWest  )	{ printf("Warning: the west edge of your DEM (%lf) is not aligned to the tile boundary.  This is probably a bad mesh.\n", in_dem.mWest ); }
	if(sDem[dem_Elevation].mEast !=in_dem.mEast  )	{ printf("warning: the east edge of your DEM (%lf) is not aligned to the tile boundary.  This is probably a bad mesh.\n", in_dem.mEast ); }
	if(sDem[dem_Elevation].mNorth!=in_dem.mNorth )	{ printf("Warning: the north edge of your DEM (%lf) is not aligned to the tile boundary.  This is probably a bad mesh.\n", in_dem.mNorth); }
	if(sDem[dem_Elevation].mSouth!=in_dem.mSouth )	{ printf("Warning: the south edge of your DEM (%lf) is not aligned to the tile boundary.  This is probably a bad mesh.\n", in_dem.mSouth); }	
	
	sBounds[0] = sDem[dem_Elevation].mWest;
	sBounds[1] = sDem[dem_Elevation].mSouth;
	sBounds[2] = sDem[dem_Elevation].mEast;
	sBounds[3] = sDem[dem_Elevation].mNorth;
	
	// Err check: voids in the DEM?
	for(int y = 0; y < in_dem.mHeight; ++y)
	for(int x = 0; x < in_dem.mWidth ; ++x)
	if(in_dem.get(x,y) <= -9999)		// -9999 is ESRI void, -32768 is our no data, and hell, -32767 shows up sometimes - no 10 km craters on earth please.
	{
		die_err("Error: your DEM is missing data at the point %d,%d.  Meshes must have no gaps or missing data!\n", x,y);
		return;
	}
}

void MT_FinishCreate(void)
{
	CropMap(*the_map, sBounds[0],sBounds[1],sBounds[2],sBounds[3],false,ConsoleProgressFunc);

}

static void print_mesh_stats(void)
{
	float minv, maxv, mean, devsq;
	int n = CalcMeshError(sMesh, sDem[dem_Elevation], minv, maxv,mean,devsq, ConsoleProgressFunc);

	printf("mean=%f min=%f max=%f std dev = %f", mean, minv, maxv, devsq);
}

void MT_MakeDSF(const char * dump, const char * out_dsf)
{
	// -simplify
	SimplifyMap(*the_map, true, ConsoleProgressFunc);

	//-calcslope
	CalcSlopeParams(sDem, true, ConsoleProgressFunc);

	// -upsample
	UpsampleEnvironmentalParams(sDem, ConsoleProgressFunc);

	// -derivedems
	DeriveDEMs(*the_map, sDem,sApts, sAptIndex, true, ConsoleProgressFunc);

	// -zoning
	ZoneManMadeAreas(*the_map, sDem[dem_LandUse], sDem[dem_ForestType], sDem[dem_Slope],sApts,Pmwx::Face_handle(),ConsoleProgressFunc);

	// -calcmesh
	TriangulateMesh(*the_map, sMesh, sDem, dump, ConsoleProgressFunc);

	WriteXESFile("temp1.xes", *the_map,sMesh,sDem,sApts,ConsoleProgressFunc);

	CalcRoadTypes(*the_map, sDem[dem_Elevation], sDem[dem_UrbanDensity],sDem[dem_Temperature], sDem[dem_Rainfall],ConsoleProgressFunc);

	// -assignterrain
	AssignLandusesToMesh(sDem,sMesh,dump,ConsoleProgressFunc);
	WriteXESFile("temp2.xes", *the_map,sMesh,sDem,sApts,ConsoleProgressFunc);

	print_mesh_stats();

	#if DEV
	for (CDT::Finite_faces_iterator tri = sMesh.finite_faces_begin(); tri != sMesh.finite_faces_end(); ++tri)
	if (tri->info().terrain == terrain_Water)
	{
		DebugAssert(tri->info().terrain == terrain_Water);
	}
	else
	{
		DebugAssert(tri->info().terrain != terrain_Water);
		DebugAssert(tri->info().terrain != -1);
	}
	#endif

	printf("Instantiating objects...\n");
	vector<PreinsetFace>	insets;

	set<int>				the_types;
	GetObjTerrainTypes		(the_types);

	printf("%llu obj types\n", (unsigned long long)the_types.size());
	for (set<int>::iterator i = the_types.begin(); i != the_types.end(); ++i)
		printf("%s ", FetchTokenString(*i));

	Bbox2	lim(sDem[dem_Elevation].mWest, sDem[dem_Elevation].mSouth, sDem[dem_Elevation].mEast, sDem[dem_Elevation].mNorth);
	GenerateInsets(*the_map, sMesh, lim, the_types, true, insets, ConsoleProgressFunc);

	InstantiateGTPolygonAll(insets, sDem, sMesh, ConsoleProgressFunc);
	DumpPlacementCounts();



	// -exportDSF
	BuildDSF(out_dsf, NULL, sDem[dem_Elevation],sDem[dem_Elevation],sMesh, /*sTriangulationLo,*/ *the_map, ConsoleProgressFunc);
}

void MT_Cleanup(void)
{
	err_f = NULL;
	delete the_map;
	the_map = NULL;
	sDem.clear();
	sMesh.clear();
	sApts.clear();
	sAptIndex.clear();
}

int MT_CreateCustomTerrain(
					const char * terrain_name,
					double		proj_lon[4],
					double		proj_lat[4],
					double		proj_s[4],
					double		proj_t[4],
					int			back_with_water)
{
	if(LookupToken(terrain_name) != -1)
	{
		die_err("ERROR: The terrain name '%s' already defined or name is reserved.\n", terrain_name);
		return NO_VALUE;
	}

	int tt = NewToken(terrain_name);
	NaturalTerrainInfo_t ni;
	NaturalTerrainRule_t nr = { 0 };
	nr.terrain = tt;
	nr.landuse = NO_VALUE;
	nr.climate = NO_VALUE;
	nr.elev_min = DEM_NO_DATA;;
	nr.elev_max = DEM_NO_DATA;;
	nr.slope_min = DEM_NO_DATA;;
	nr.slope_max = DEM_NO_DATA;;
	nr.temp_min = DEM_NO_DATA;;
	nr.temp_max = DEM_NO_DATA;;
	nr.temp_rng_min = DEM_NO_DATA;;
	nr.temp_rng_max = DEM_NO_DATA;;
	nr.rain_min = DEM_NO_DATA;;
	nr.rain_max = DEM_NO_DATA;;
	nr.near_water = 0;
	nr.slope_heading_min = DEM_NO_DATA;;
	nr.slope_heading_max = DEM_NO_DATA;;
	nr.rel_elev_min = DEM_NO_DATA;;
	nr.rel_elev_max = DEM_NO_DATA;;
	nr.elev_range_min = DEM_NO_DATA;;
	nr.elev_range_max = DEM_NO_DATA;;
	nr.urban_density_min = DEM_NO_DATA;;
	nr.urban_density_max = DEM_NO_DATA;;
	nr.urban_radial_min = DEM_NO_DATA;;
	nr.urban_radial_max = DEM_NO_DATA;;
	nr.urban_trans_min = DEM_NO_DATA;;
	nr.urban_trans_max = DEM_NO_DATA;
	nr.urban_square = 0;
	nr.lat_min = DEM_NO_DATA;
	nr.lat_max = DEM_NO_DATA;
	nr.variant = 0;
//	nr.related = -1;
	nr.name = tt;
	ni.layer = 0;
	ni.xon_dist = 0;
//	ni.xon_hack = 0;
	ni.custom_ter = (back_with_water == 2) ? tex_custom_soft_water : ((back_with_water == 1) ? tex_custom_hard_water : tex_custom_no_water);

	gNaturalTerrainRules.insert(gNaturalTerrainRules.begin(), nr);
	gNaturalTerrainInfo[tt] = ni;

	tex_proj_info	pinfo;
	for(int n = 0; n < 4; ++n)
	{
		pinfo.corners[n] = Point2(proj_lon[n],proj_lat[n]);
		pinfo.ST	 [n] = Point2(proj_s  [n],proj_t  [n]);
	}
	gTexProj[tt] = pinfo;

	return tt;
}

void MT_LimitZ(int limit)
{
	// store limit^2
	zlimit *= zlimit;
}

void MT_LayerStart(int in_terrain_type)
{
	if(layer_type != NO_VALUE)
		die_err("ERROR: new layer started while a layer is already in effect.\n");
	else if (in_terrain_type == NO_VALUE)
		die_err("ERROR: new layer needs a valid terrain type.\n");
	else
		layer_type = in_terrain_type;
}

void MT_LayerEnd(void)
{
	if(layer_type == NO_VALUE)
		die_err("ERROR: layer cannot be ended - it has not been started.\n");
	else
	{
		Polygon_set_2		layer_map;
		if (!layer.empty())
		{
			layer_map.join(layer.begin(), layer.end());
			if(!layer_mask.is_empty())
				layer_map.intersection(layer_mask);

			for(Pmwx::Face_iterator f = layer_map.arrangement().faces_begin(); f != layer_map.arrangement().faces_end(); ++f)
			if (f->contained())
				f->data().mTerrainType = layer_type;

			Pmwx *	new_map = new Pmwx;
			MapOverlay(*the_map, layer_map.arrangement(), *new_map);
			delete the_map;
			the_map = new_map;
		}
		layer.clear();
		layer_type = NO_VALUE;
	}
}

void MT_LayerBackground(const char * in_terrain_type)
{
	int t = LookupToken(in_terrain_type);
	if(t == -1)
	{
		die_err("Unknown terrain %s.\n", in_terrain_type);
		return;
	}

	Polygon_2	p;
	p.push_back(Point_2(sBounds[0],sBounds[1]));
	p.push_back(Point_2(sBounds[2],sBounds[1]));
	p.push_back(Point_2(sBounds[2],sBounds[3]));
	p.push_back(Point_2(sBounds[0],sBounds[3]));

	Polygon_set_2	layer_map(p);

	for(Pmwx::Face_iterator f = layer_map.arrangement().faces_begin(); f != layer_map.arrangement().faces_end(); ++f)
	if (f->contained())
		f->data().mTerrainType = t;

	Pmwx *	new_map = new Pmwx;
	MapOverlay(*the_map, layer_map.arrangement(), *new_map);
	delete the_map;
	the_map = new_map;
}

void MT_LayerShapefile(const char * fi, const char * in_terrain_type)
{
	int lu = LookupToken(in_terrain_type);
	if(lu == -1) 
	{
		die_err("Error: unknown terrain %s.\n", in_terrain_type);
		return;
	}
	
	Pmwx	layer_map;
	double b[4] = { sBounds[0],sBounds[1],sBounds[2],sBounds[3] };
	if(!ReadShapeFile(fi,layer_map,shp_Mode_Landuse | shp_Mode_Simple | shp_Use_Crop , in_terrain_type, b, 0.0, 0, ConsoleProgressFunc))
		die_err("Unable to load shape file: %s\n", fi);

	Pmwx *	new_map = new Pmwx;
	MapOverlay(*the_map, layer_map, *new_map);
	delete the_map;
	the_map = new_map;
}


void MT_PolygonStart(void)
{
	zmin=30000,zmax=-2000;
}

void MT_PolygonPoint(double lon, double lat)
{
	ring.push_back(Point_2(lon,lat));
	if (zlimit != 0) {
		int z = sDem[dem_Elevation].xy_nearest(lon,lat);
		if (z<zmin) zmin=z;
		if (z>zmax) zmax=z;
	}
}

bool MT_PolygonEnd(void)
{
	bool zyes = true;
	if (zlimit != 0) {
		Bbox_2 box = ring.bbox();
		double DEG_TO_NM_LON = DEG_TO_NM_LAT * cos(CGAL::to_double(box.ymin()) * DEG_TO_RAD);
		double rhs = (pow((box.xmax()-box.xmin())*DEG_TO_NM_LON*NM_TO_MTR,2) + pow((box.ymax()-box.ymin())*DEG_TO_NM_LAT*NM_TO_MTR,2));
		double lhs = pow((double)(zmax-zmin),2);
		//fprintf(stderr," %9.0lf,%9.0lf ", rhs, lhs);
		if (zlimit*lhs > rhs) zyes = false;
	}
	if (zyes) {
		if (ring.is_simple()) {
			if (ring.orientation() == CGAL::CLOCKWISE)
				ring.reverse_orientation();
			Polygon_set_2::Polygon_with_holes_2 P(ring, holes.begin(), holes.end());
			layer.push_back(P);
		} else {
			die_err("ERROR: this polygon is not simple.  Make sure none of the sides intersect with each other.\n");
		}
	}
	holes.clear();
	ring.clear();
}

void MT_HoleStart(void)
{
}

void MT_HolePoint(double lon, double lat)
{
	the_hole.push_back(Point_2(lon,lat));
}

void MT_HoleEnd(void)
{
	if (the_hole.is_simple()) {
		if (the_hole.orientation() != CGAL::CLOCKWISE)
			the_hole.reverse_orientation();
		holes.push_back(the_hole);
		the_hole.clear();
	} else {
		the_hole.clear();
		die_err("ERROR: This hole is a non-simple polygon - make sure none of the sides intersect with each other!\n");
	}
}

void MT_NetStart(const char * typ)
{
	net_type = LookupToken(typ);
	if(net_type == -1) 
	{
		die_err("Error: unknown network type %s.\n", typ);
		return;
	}	
}

void MT_NetSegment(double lon1, double lat1, double lon2, double lat2)
{
	net.push_back(X_monotone_curve_2(Segment_2(Point_2(lon1,lat1), Point_2(lon2,lat2)),0));
}

void MT_NetEnd(void)
{
	struct	GISNetworkSegment_t segdata = { net_type, net_type, 0.0, 0.0 };
	Pmwx road_grid;

	if (!net.empty())
	{
		CGAL::insert(road_grid, net.begin(), net.end());

		Pmwx::Edge_iterator the_edge;
		for (Pmwx::Edge_iterator e = road_grid.edges_begin(); e != road_grid.edges_end(); ++e)
			e->data().mSegments.push_back(GISNetworkSegment_t(segdata));

		Pmwx * new_map = new Pmwx;
		MapMerge(*the_map, road_grid,*new_map);
		delete the_map;
		the_map = new_map;
		net.clear();
	}
}

void MT_EnableDDSGeneration(int create)
{
	sMakeDDS = create;
}

void MT_SetMeshSpecs(int max_pts, float max_err)
{
	gMeshPrefs.max_points = max_pts;
	gMeshPrefs.max_error = max_err;
}

void MT_Mask(const char * shapefile)
{
	if(shapefile == NULL)
		layer_mask.clear();
	else
	{
		Pmwx	mask_map;
		double b[4] = { sBounds[0],sBounds[1],sBounds[2],sBounds[3] };
		if(!ReadShapeFile(shapefile,mask_map,shp_Mode_Landuse | shp_Mode_Simple | shp_Use_Crop , "terrain_Water", b, 0.0, 0, ConsoleProgressFunc))
			die_err("Unable to load shape file: %s\n", shapefile);

		for(Pmwx::Face_iterator f = mask_map.faces_begin(); f != mask_map.faces_end(); ++f)
			f->set_contained(!f->is_unbounded() && f->data().IsWater());
		
		layer_mask = mask_map;
	}
}

void MT_OrthoPhoto(
					const char * terrain_name,
					double		 proj_lon[4],
					double		 proj_lat[4],
					double		 proj_s[4],
					double		 proj_t[4],
					int			 back_with_water)
{
	string tname(terrain_name);
	if(back_with_water == 2)
		tname += "_soft";
	if(back_with_water == 1)
		tname += "_hard";
	int t = MT_CreateCustomTerrain(tname.c_str(), proj_lon,proj_lat,proj_s,proj_t,back_with_water);
	MT_LayerStart(t);
	MT_PolygonStart();
	for(int n = 0; n < 4; ++n)
		MT_PolygonPoint(proj_lon[n],proj_lat[n]);
	MT_PolygonEnd();
	MT_LayerEnd();
}

static void qmid_recurse(int q, double io_lon[4], double io_lat[4])
{
	// vert order
	// 3 2
	// 0 1
	// qmid parts
	// 0 1
	// 2 3
	if(q == 0 || q == 1)
	{
		io_lat[0] = 0.5 * (io_lat[0] + io_lat[3]);		// top half
		io_lat[1] = 0.5 * (io_lat[1] + io_lat[2]);
	} else {
		io_lat[3] = 0.5 * (io_lat[0] + io_lat[3]);		// bot half
		io_lat[2] = 0.5 * (io_lat[1] + io_lat[2]);
	}

	if(q == 1 || q == 3)
	{
		io_lon[0] = 0.5 * (io_lon[0] + io_lon[1]);	// left half
		io_lon[3] = 0.5 * (io_lon[3] + io_lon[2]);
	} else {
		io_lon[1] = 0.5 * (io_lon[0] + io_lon[1]);	// left half
		io_lon[2] = 0.5 * (io_lon[3] + io_lon[2]);
	}
}

inline const char * no_path(const char * f)
{
	const  char * r = f;
	while(*f)
	{
		if(*f == '/') r=f+1;
		++f;
	}
	return r;
}

inline int near2(int n) { 
	int r = 1;
	while(r < n && r < 2048) r <<= 1;
	return r;
}

void MT_GeoTiff(const char * fname, int back_with_water)
{
	double c[8];	// SW, SE, NW, NE lon,lat pairs
	int align = dem_want_Area;
	
	if(!FetchTIFFCorners(fname,c, align))
	{
		die_err("Unable to read corner coordinates from %s.\n",fname);
		return;
	}

	double lon[4] = { c[0], c[2], c[6], c[4] };
	double lat[4] = { c[1], c[3], c[7], c[5] };
	double s[4] = { 0.0, 1.0, 1.0, 0.0 };
	double t[4] = { 0.0, 0.0, 1.0, 1.0 };

	char tname[1024],dname[1024];
	strcpy(tname,fname);
	char * i = tname, * p = NULL;
	while(*i)
		if(*i++=='.') p = i;
	if(p) *p = 0;
	strcpy(dname,tname);
	strcat(tname,"ter");
	strcat(dname,"dds");

	MT_OrthoPhoto(tname,lon,lat,s,t,back_with_water);

	int meters= LonLatDistMeters(lon[0],lat[0],lon[2],lat[2]);

	int isize = 2048;

	if(!FILE_exists(dname))
	{
		if (sMakeDDS)
		{
			ImageInfo rgba;
			if(!CreateBitmapFromTIF(fname,&rgba))
			{
				ImageInfo smaller;
				if (!CreateNewBitmap(near2(rgba.width),near2(rgba.height), 4, &smaller))
				{
					isize = max(smaller.width,smaller.height);

					CopyBitmapSection(&rgba,&smaller, 0,0,rgba.width,rgba.height, 0, 0, smaller.width,smaller.height);				
					
					MakeMipmapStack(&smaller);
					WriteBitmapToDDS(smaller, 5, dname);
					DestroyBitmap(&smaller);
				}
				DestroyBitmap(&rgba);
			}
		}
	} else {
		ImageInfo comp;
		CreateBitmapFromDDS(dname,&comp);
		isize = max(comp.width,comp.height);
		DestroyBitmap(&comp);
	}

	if(!FILE_exists(tname))
	{

		FILE * fi = fopen(tname,"w");
		if(fi)
		{
			fprintf(fi,
				"A\n"
				"800\n"
				"TERRAIN\n\n"
				"BASE_TEX_NOWRAP %s\n",no_path(dname));
	//		if(want_lite)
	//			fprintf(fi,"LIT_TEX_NOWRAP %s_LIT.dds\n",id);
			fprintf(fi,"LOAD_CENTER %lf %lf %d %d\n\n",0.5 * (lat[0] + lat[2]), 0.5 * (lon[0] + lon[2]), meters, isize);

			fclose(fi);
		}
	}
}

void MT_QMID_Prefix(const char * prefix)
{
	g_qmid_prefix = prefix;
}

void MT_QMID(const char * id, int back_with_water)
{
	double lon[4] = { -180.0, 300.0, 300.0, -180.0 };
	double lat[4] = { -270.0, -270.0, 90.0, 90.0 };
	double s[4] = { 0.0, 1.0, 1.0, 0.0 };
	double t[4] = { 0.0, 0.0, 1.0, 1.0 };

	const char * i = id;
	while(*i)
		qmid_recurse((*i++) - '0',lon,lat);

	char fname[1024];
	sprintf(fname,"%s%s.ter",g_qmid_prefix.c_str(),id);

	printf("QMID: %s will go from: %lf,%lf to %lf,%lf\n",
		id,lon[0],lat[0],lon[2],lat[2]);

	MT_OrthoPhoto(fname, lon, lat, s, t, back_with_water);

	int want_lite = false;

	int isize = 1024;

	sprintf(fname,"%s%s.dds",g_qmid_prefix.c_str(), id);
	if(!FILE_exists(fname))
	{
		if(sMakeDDS)
		{
			sprintf(fname,"%s%sSu.bmp",g_qmid_prefix.c_str(),id);
			ImageInfo rgb;
			if(!CreateBitmapFromFile(fname,&rgb))
			{
				isize = max(rgb.width,rgb.height);
				if(!ConvertBitmapToAlpha(&rgb,false))
				{
					sprintf(fname,"%s%sBl.bmp",g_qmid_prefix.c_str(),id);
					ImageInfo alpha;
					if(!CreateBitmapFromFile(fname,&alpha))
					{
						if(rgb.width == alpha.width && rgb.height == alpha.height)
						for(int y = 0; y < alpha.height; ++y)
						for(int x = 0; x < alpha.width; ++x)
							rgb.data[x * rgb.channels + y * (rgb.width * rgb.channels + rgb.pad) + 3] =
							alpha.data[x * alpha.channels + y * (alpha.width * alpha.channels + alpha.pad)];

						DestroyBitmap(&alpha);
					}

					MakeMipmapStack(&rgb);
					sprintf(fname,"%s%s.dds",g_qmid_prefix.c_str(),id);
					WriteBitmapToDDS(rgb, 5, fname);
				}

				DestroyBitmap(&rgb);
			}
		}
	} else {
		ImageInfo comp;
		CreateBitmapFromDDS(fname,&comp);
		isize = max(comp.width,comp.height);
		DestroyBitmap(&comp);
	}

	sprintf(fname,"%s%s_LIT.dds",g_qmid_prefix.c_str(),id);
	if(FILE_exists(fname))
		want_lite=true;
	else
	{
		sprintf(fname,"%s%sLm.bmp",g_qmid_prefix.c_str(),id);
		ImageInfo lit;
		if(!CreateBitmapFromFile(fname,&lit))
		{
			sprintf(fname,"%s%s_LIT.dds",g_qmid_prefix.c_str(),id);
			ConvertBitmapToAlpha(&lit,false);
			MakeMipmapStack(&lit);
			WriteBitmapToDDS(lit,1,fname);
			DestroyBitmap(&lit);
			want_lite=true;
		}
	}

	int meters= LonLatDistMeters(lon[0],lat[0],lon[2],lat[2]);

	sprintf(fname,"%s%s.ter",g_qmid_prefix.c_str(),id);
	if(!FILE_exists(fname))
	{
		FILE * fi = fopen(fname,"w");
		if(fi)
		{
			fprintf(fi,
				"A\n"
				"800\n"
				"TERRAIN\n\n"
				"BASE_TEX_NOWRAP %s.dds\n",id);
			if(want_lite)
				fprintf(fi,"LIT_TEX_NOWRAP %s_LIT.dds\n",id);
			fprintf(fi,"LOAD_CENTER %lf %lf %d %d\n\n",0.5 * (lat[0] + lat[2]), 0.5 * (lon[0] + lon[2]), meters, isize);

			fclose(fi);
		}
	}


}

