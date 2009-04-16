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
#include "ObjPlacement.h"
#include "ObjTables.h"
#include "ShapeIO.h"
#include "FileUtils.h"

static DEMGeoMap			sDem;
static CDT					sMesh;
static AptVector			sApts;
static AptIndex				sAptIndex;
static double				sBounds[4];



static Pmwx *							the_map = NULL;
static int								layer_type = NO_VALUE;
static Polygon_2						ring, the_hole;
static vector<Polygon_2>				holes;
static vector<Polygon_with_holes_2>		layer;
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
		Pmwx		dummy_vec;
		CDT			dummy_mesh;
		AptVector	dummy_apt;
		ReadXESFile(xes, &dummy_vec, &dummy_mesh, &sDem, &dummy_apt, ConsoleProgressFunc);
	}
	MemFile_Close(xes);

	sDem[dem_Elevation] = in_dem;

	sBounds[0] = in_dem.mWest;
	sBounds[1] = in_dem.mSouth;
	sBounds[2] = in_dem.mEast;
	sBounds[3] = in_dem.mNorth;
}
void MT_FinishCreate(void)
{
	CropMap(*the_map, sBounds[0],sBounds[1],sBounds[2],sBounds[3],false,ConsoleProgressFunc);
//	WriteXESFile("temp.xes", *the_map,sMesh,sDem,sApts,ConsoleProgressFunc);


	gNaturalTerrainIndex.clear();
	for(int rn = 0; rn < gNaturalTerrainTable.size(); ++rn)
	if (gNaturalTerrainIndex.count(gNaturalTerrainTable[rn].name) == 0)
		gNaturalTerrainIndex[gNaturalTerrainTable[rn].name] = rn;
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
	DeriveDEMs(*the_map, sDem,sApts, sAptIndex, ConsoleProgressFunc);

	// -zoning
	ZoneManMadeAreas(*the_map, sDem[dem_LandUse], sDem[dem_Slope],sApts,ConsoleProgressFunc);

	// -calcmesh
	TriangulateMesh(*the_map, sMesh, sDem, dump, ConsoleProgressFunc);

	WriteXESFile("temp1.xes", *the_map,sMesh,sDem,sApts,ConsoleProgressFunc);

	CalcRoadTypes(*the_map, sDem[dem_Elevation], sDem[dem_UrbanDensity],ConsoleProgressFunc);

	// -assignterrain
	AssignLandusesToMesh(sDem,sMesh,dump,ConsoleProgressFunc);
	WriteXESFile("temp3.xes", *the_map,sMesh,sDem,sApts,ConsoleProgressFunc);

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

	printf("%d obj types\n", the_types.size());
	for (set<int>::iterator i = the_types.begin(); i != the_types.end(); ++i)
		printf("%s ", FetchTokenString(*i));

	Bbox2	lim(sDem[dem_Elevation].mWest, sDem[dem_Elevation].mSouth, sDem[dem_Elevation].mEast, sDem[dem_Elevation].mNorth);
	GenerateInsets(*the_map, sMesh, lim, the_types, true, insets, ConsoleProgressFunc);

	InstantiateGTPolygonAll(insets, sDem, sMesh, ConsoleProgressFunc);
	DumpPlacementCounts();



	// -exportDSF
	BuildDSF(out_dsf, out_dsf, sDem[dem_LandUse],sMesh, /*sTriangulationLo,*/ *the_map, ConsoleProgressFunc);
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
	NaturalTerrainInfo_t nt = { 0 };
	nt.terrain = tt;
	nt.landuse = NO_VALUE;
	nt.climate = NO_VALUE;
	nt.elev_min = DEM_NO_DATA;;
	nt.elev_max = DEM_NO_DATA;;
	nt.slope_min = DEM_NO_DATA;;
	nt.slope_max = DEM_NO_DATA;;
	nt.temp_min = DEM_NO_DATA;;
	nt.temp_max = DEM_NO_DATA;;
	nt.temp_rng_min = DEM_NO_DATA;;
	nt.temp_rng_max = DEM_NO_DATA;;
	nt.rain_min = DEM_NO_DATA;;
	nt.rain_max = DEM_NO_DATA;;
	nt.near_water = 0;
	nt.slope_heading_min = DEM_NO_DATA;;
	nt.slope_heading_max = DEM_NO_DATA;;
	nt.rel_elev_min = DEM_NO_DATA;;
	nt.rel_elev_max = DEM_NO_DATA;;
	nt.elev_range_min = DEM_NO_DATA;;
	nt.elev_range_max = DEM_NO_DATA;;
	nt.urban_density_min = DEM_NO_DATA;;
	nt.urban_density_max = DEM_NO_DATA;;
	nt.urban_radial_min = DEM_NO_DATA;;
	nt.urban_radial_max = DEM_NO_DATA;;
	nt.urban_trans_min = DEM_NO_DATA;;
	nt.urban_trans_max = DEM_NO_DATA;
	nt.urban_square = 0;
	nt.lat_min = DEM_NO_DATA;
	nt.lat_max = DEM_NO_DATA;
	nt.variant = 0;
	nt.related = -1;
	nt.name = tt;
	nt.layer = 0;
	nt.xon_dist = 0;
	nt.xon_hack = 0;
	nt.custom_ter = back_with_water ? tex_custom_water : tex_custom;

	int rn = gNaturalTerrainTable.size();
	gNaturalTerrainTable.insert(gNaturalTerrainTable.begin()+(num_cus_terrains++),nt);

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
	Pmwx	layer_map;
	double b[4] = { sBounds[0],sBounds[1],sBounds[2],sBounds[3] };
	if(!ReadShapeFile(fi,layer_map,shp_Mode_Landuse | shp_Mode_Simple | shp_Use_Crop | shp_Fast, in_terrain_type, b, ConsoleProgressFunc))
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
		insert_x_monotone_curves(road_grid, net.begin(), net.end());

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

void MT_OrthoPhoto(
					const char * terrain_name,
					double		 proj_lon[4],
					double		 proj_lat[4],
					double		 proj_s[4],
					double		 proj_t[4],
					int			 back_with_water)
{
	int t = MT_CreateCustomTerrain(terrain_name, proj_lon,proj_lat,proj_s,proj_t,back_with_water);
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

void MT_GeoTiff(const char * fname, int back_with_water)
{
	double c[8];	// SW, SE, NW, NE lon,lat pairs
	if(!FetchTIFFCorners(fname,c))
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
	int isize=2048;

	if(!FILE_exists(tname))
	{

		FILE * fi = fopen(tname,"w");
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
	sprintf(fname,"%s.ter",id);

	printf("QMID: %s will go from: %lf,%lf to %lf,%lf\n",
		id,lon[0],lat[0],lon[2],lat[2]);

	MT_OrthoPhoto(fname, lon, lat, s, t, back_with_water);

	int want_lite = false;

	int isize = 1024;

	sprintf(fname,"%s.dds",id);
	if(!FILE_exists(fname))
	{
		sprintf(fname,"%sSu.bmp",id);
		ImageInfo rgb;
		if(!CreateBitmapFromFile(fname,&rgb))
		{
			isize = max(rgb.width,rgb.height);
			if(!ConvertBitmapToAlpha(&rgb,false))
			{
				sprintf(fname,"%sBl.bmp",id);
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

				sprintf(fname,"%s.dds",id);
				WriteBitmapToDDS(rgb, 5, fname);
			}

			DestroyBitmap(&rgb);
		}
	} else {
		ImageInfo comp;
		CreateBitmapFromDDS(fname,&comp);
		isize = max(comp.width,comp.height);
		DestroyBitmap(&comp);
	}

	sprintf(fname,"%s_LIT.dds",id);
	if(FILE_exists(fname))
		want_lite=true;
	else
	{
		sprintf(fname,"%sLm.bmp",id);
		ImageInfo lit;
		if(!CreateBitmapFromFile(fname,&lit))
		{
			sprintf(fname,"%s_LIT.dds",id);
			WriteBitmapToDDS(lit,1,fname);
			DestroyBitmap(&lit);
			want_lite=true;
		}
	}

	int meters= LonLatDistMeters(lon[0],lat[0],lon[2],lat[2]);

	sprintf(fname,"%s.ter",id);
	if(!FILE_exists(fname))
	{
		FILE * fi = fopen(fname,"w");
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

