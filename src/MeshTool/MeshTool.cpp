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

#include "MapDefsCGAL.h"
#include "MapOverlay.h"
#include "XESInit.h"
#include "GISTool_Globals.h"
#include "XESIO.h"
#include "DEMIO.h"
#include "DEMAlgs.h"
#include "DEMTables.h"
#include "MapAlgs.h"
#include "MeshAlgs.h"
#include "DSFBuilder.h"
#include "Zoning.h"
#include "MemFileUtils.h"
#include "MeshDefs.h"
#include "NetPlacement.h"
#include "ObjTables.h"
#include "ObjPlacement.h"
#include <CGAL/assertions.h>
#include "MapTopology.h"
#if LIN
#include <execinfo.h>
#endif


void	CGALFailure(
        const char* what, const char* expr, const char* file, int line, const char* msg)
{
#if LIN
	void* callstack[128];
	int i, frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	for (i = 0; i < frames; ++i) {
		fprintf(stdout,"%s\n", strs[i]);
	}
	free(strs);
#else
	fprintf(stderr,"callstack for mingw builds not yet implemented\n");
#endif	
	fprintf(stdout,"Terminating due to a CGAL exception.\n");
	fprintf(stdout,"****************************************************************************\n");
	fprintf(stdout,"ERROR  (%d,%d) %s: %s (%s:%d).%s\n", gMapWest, gMapSouth, what, expr, file, line, msg ? msg : "");
	fprintf(stdout,"****************************************************************************\n");

	fprintf(stderr,"ERROR  (%d,%d) %s: %s (%s:%d).%s\n", gMapWest, gMapSouth, what, expr, file, line, msg ? msg : "");
	exit(1);
}

static int			line_num;
static const char * fname;

static void die_parse(const char * msg, ...)
{
	va_list	va;
	va_start(va,msg);
	vfprintf(stderr,msg,va);
	fprintf(stderr,"(%s: line %d.)\n",fname,line_num);
	exit(1);
}



int	main(int argc, char * argv[])
{
	Pmwx *the_map = new Pmwx;
	
	if(argc == 2 && !strcmp(argv[1],"--version"))
	{
		fprintf(stdout, "MeshTool version 1.0b1, compiled " __DATE__ ".\n");
		exit(0);
	}

	if(argc == 2 && !strcmp(argv[1],"--auto_config"))
	{
		exit(0);
	}

	try {

		// Set CGAL to throw an exception rather than just
		// call exit!
		CGAL::set_error_handler(CGALFailure);

		XESInit();

		if(argc != 6)
		{
			fprintf(stderr, "USAGE: MeshTool <script.txt> <file.xes> <file.hgt> <dir_base> <file.dsf>\n");
			exit(1);
		}
		
		fprintf(stderr,"Open XES\n");
		MFMemFile *	xes = MemFile_Open(argv[2]);
		if(xes == NULL)
		{
			fprintf(stderr,"ERROR: could not read XES file:%s\n", argv[2]);
			exit(1);
		}

		{
			Pmwx		dummy_vec;
			CDT			dummy_mesh;
			AptVector	dummy_apt;
			ReadXESFile(xes, &dummy_vec, &dummy_mesh, &gDem, &dummy_apt, ConsoleProgressFunc);
		}
		MemFile_Close(xes);
		
		if(strstr(argv[3],".hgt")) 
		{
			if (!ReadRawHGT(gDem[dem_Elevation], argv[3]))
			{
				fprintf(stderr,"Could not read HGT file: %s\n", argv[3]);
				exit(1);
			}
		}
		else if(strstr(argv[3],".tif")) 
		{
			if (!ExtractGeoTiff(gDem[dem_Elevation], argv[3]))
			{
				fprintf(stderr,"Could not read GeoTIFF file: %s\n", argv[3]);
				exit(1);
			}
		} 
		else
		{
			fprintf(stderr,"ERROR: unknown file extension for DEM: %s\n", argv[3]);
			exit(1);
		}
		
		FILE * script = fopen(argv[1], "r");
		fname=argv[1];
		if(!script)
		{
			fprintf(stderr, "ERROR: could not open %s\n", argv[1]);
			exit(1);
		}

		int								num_cus_terrains = 0;
		Polygon_2						ring, the_hole;
		vector<Polygon_2>				holes;
		vector<Polygon_with_holes_2> 	layer;
		vector<X_monotone_curve_2>		net;
		int								terrain_type;
		int								layer_type = NO_VALUE;
		double							coords[4];
		char							cus_ter[256];
		char							typ[256];
		char							buf[1024];

		int				last_ter = -1;
		int				proj_pt;
		tex_proj_info	pinfo;
		int				use_wat;
		int				zlimit=0,zmin=30000,zmax=-2000;
		int				is_layer = 0;
		
		line_num=0;
		while (fgets(buf, sizeof(buf), script))
		{
			++line_num;
			if(sscanf(buf,"DEFINE_CUSTOM_TERRAIN %d %s",&use_wat, cus_ter)==2)
			{
				proj_pt = 0;
				if(LookupToken(cus_ter) != -1)
					die_parse("ERROR: The terrain name '%s' already defined or name is reserved.\n", cus_ter);

				int tt = NewToken(cus_ter);
				last_ter = tt;
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
				nt.custom_ter = use_wat ? tex_custom_water : tex_custom;

				int rn = gNaturalTerrainTable.size();
				gNaturalTerrainTable.insert(gNaturalTerrainTable.begin()+(num_cus_terrains++),nt);
			}
			if(sscanf(buf,"PROJECT_POINT %lf %lf %lf %lf",coords,coords+1,coords+2,coords+3)==4)
			{
				if(last_ter==-1)
					die_parse("ERROR: PROJECT_POINT not allowed until custom terrain defined, or you have more than 4 projection pooints.\n");

				pinfo.corners[proj_pt] = Point2(coords[0],coords[1]);
				pinfo.ST[proj_pt] = Point2(coords[2],coords[3]);
				proj_pt++;
				if(proj_pt==4)
				{
					gTexProj[last_ter] = pinfo;
					last_ter = -1;
				}
			}
			if(strncmp(buf,"BEGIN_LAYER",strlen("BEGIN_LAYER"))==0)
			{
				is_layer=1;
				layer.clear();
				ring.clear();
				the_hole.clear();
				holes.clear();
			}

			if(sscanf(buf,"CUSTOM_POLY %s",cus_ter)==1)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = LookupToken(cus_ter);
				zmin=30000,zmax=-2000;
				if(terrain_type == -1)
					die_parse("ERROR: cannot find custom terrain type '%s'\n", cus_ter);
			}

			if(strncmp(buf,"LAND_POLY",strlen("LAND_POLY"))==0)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = terrain_Natural;
				zmin=30000,zmax=-2000;
			}
			if(strncmp(buf,"WATER_POLY",strlen("WATER_POLY"))==0)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = terrain_Water;
				zmin=30000,zmax=-2000;
			}
			if(strncmp(buf,"APT_POLY",strlen("APT_POLY"))==0)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = terrain_Airport;
				zmin=30000,zmax=-2000;
			}
			if(strncmp(buf,"BEGIN_HOLE",strlen("BEGIN_HOLE"))==0)
			{
				the_hole.clear();
			}
			if(strncmp(buf,"END_HOLE",strlen("END_HOLE"))==0)
			{
				if (the_hole.is_simple()) {
					if (the_hole.orientation() != CGAL::CLOCKWISE)
						the_hole.reverse_orientation();
					holes.push_back(the_hole);
				} else {
					die_parse("ERROR: This hole is a non-simple polygon - make sure none of the sides intersect with each other!\n");
				}
			}
			if(strncmp(buf,"END_POLY",strlen("END_POLY"))==0)
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
						die_parse("ERROR: this polygon is not simple.  Make sure none of the sides intersect with each other.\n");
					}
				} 
				
				if(!is_layer)
				{
					Polygon_set_2		layer_map;					
					if (!layer.empty()) 
					{
						layer_map.join(layer.begin(), layer.end());
						
						for(Pmwx::Face_iterator f = layer_map.arrangement().faces_begin(); f != layer_map.arrangement().faces_end(); ++f) 
						if (f->contained()) 
							f->data().mTerrainType = terrain_type;

						Pmwx *	new_map = new Pmwx;
						MapOverlay(*the_map, layer_map.arrangement(), *new_map);
						delete the_map;
						the_map = new_map;
					}
					layer.clear();
					ring.clear();
					the_hole.clear();
					holes.clear();
				} else {
					if(layer_type == NO_VALUE)
						layer_type = terrain_type;
					else
						if(layer_type != terrain_type)
							die_parse("This polygon's type does not match the others in the layer.  Layer is %s, but this is: %s.\n",
								FetchTokenString(layer_type), FetchTokenString(terrain_type));
				}
			}
			if(strncmp(buf,"END_LAYER",strlen("END_LAYER"))==0)
			{			
				Polygon_set_2		layer_map;					
				if (!layer.empty()) 
				{
					if(layer_type == NO_VALUE)
						die_parse("This layer does not have a terrain type.\n");
				
					layer_map.join(layer.begin(), layer.end());
					
					for(Pmwx::Face_iterator f = layer_map.arrangement().faces_begin(); f != layer_map.arrangement().faces_end(); ++f) 
					if (f->contained()) 
						f->data().mTerrainType = terrain_type;

					Pmwx *	new_map = new Pmwx;
					MapOverlay(*the_map, layer_map.arrangement(), *new_map);
					delete the_map;
					the_map = new_map;
				}			
				is_layer=0;
				layer_type=NO_VALUE;
			}
			if (sscanf(buf, "POLYGON_POINT %lf %lf", &coords[0], &coords[1])==2)
			{
				ring.push_back(Point_2(coords[0],coords[1]));
				if (zlimit != 0) {
					int z = gDem[dem_Elevation].xy_nearest(coords[0],coords[1]);
					if (z<zmin) zmin=z;
					if (z>zmax) zmax=z;
				}
			}
			if (sscanf(buf, "HOLE_POINT %lf %lf", &coords[0], &coords[1])==2)
			{
				the_hole.push_back(Point_2(coords[0],coords[1]));
			}
			if (sscanf(buf, "ZLIMIT %d", &zlimit)==1)
			{
				// store limit^2
				zlimit *= zlimit;
			}
			if(sscanf(buf,"BEGIN_NET %s",typ)==1)
			{
				net.clear();
			}
			if (sscanf(buf, "NET_SEG %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3])==4)
			{
				net.push_back(X_monotone_curve_2(Segment_2(Point_2(coords[0],coords[1]), Point_2(coords[2],coords[3])),0));
			}
			if(strncmp(buf,"END_NET",strlen("END_NET"))==0)
			{
				int net_type = LookupToken(typ);
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
				}
			}
		}
		fclose(script);

		Pmwx gMap(*the_map);
		delete the_map;
		the_map = NULL;

		
		gNaturalTerrainIndex.clear();
		for(int rn = 0; rn < gNaturalTerrainTable.size(); ++rn)
		if (gNaturalTerrainIndex.count(gNaturalTerrainTable[rn].name) == 0)
			gNaturalTerrainIndex[gNaturalTerrainTable[rn].name] = rn;
		
		// -simplify
		SimplifyMap(gMap, true, ConsoleProgressFunc);

		//-calcslope
		CalcSlopeParams(gDem, true, ConsoleProgressFunc);

		// -upsample
		UpsampleEnvironmentalParams(gDem, ConsoleProgressFunc);

		// -derivedems
		DeriveDEMs(gMap, gDem,gApts, gAptIndex, ConsoleProgressFunc);

		// -zoning
		ZoneManMadeAreas(gMap, gDem[dem_LandUse], gDem[dem_Slope],gApts,ConsoleProgressFunc);

		// -calcmesh
		TriangulateMesh(gMap, gTriangulationHi, gDem, argv[4], ConsoleProgressFunc);		
		
		CalcRoadTypes(gMap, gDem[dem_Elevation], gDem[dem_UrbanDensity],ConsoleProgressFunc);
		
		// -assignterrain
		AssignLandusesToMesh(gDem,gTriangulationHi,argv[4],ConsoleProgressFunc);

		#if DEV
		for (CDT::Finite_faces_iterator tri = gTriangulationHi.finite_faces_begin(); tri != gTriangulationHi.finite_faces_end(); ++tri)
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
		
		Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);
		GenerateInsets(gMap, gTriangulationHi, lim, the_types, true, insets, ConsoleProgressFunc);
		
		InstantiateGTPolygonAll(insets, gDem, gTriangulationHi, ConsoleProgressFunc);
		DumpPlacementCounts();
	
		
		// -exportDSF
		BuildDSF(argv[5], argv[5], gDem[dem_LandUse],gTriangulationHi, /*gTriangulationLo,*/ gMap, ConsoleProgressFunc);			

	} catch (exception& e) {
		fprintf(stdout,"****************************************************************************\n");
		fprintf(stdout,"ERROR (%d,%d): Caught unknown exception %s.  Exiting.\n", gMapWest, gMapSouth, e.what());
		fprintf(stdout,"****************************************************************************\n");

		fprintf(stderr,"ERROR (%d,%d): Caught unknown exception %s.  Exiting.\n", gMapWest, gMapSouth, e.what());
		exit(0);
	} catch (...) {
		fprintf(stdout,"****************************************************************************\n");
		fprintf(stdout,"ERROR (%d,%d): Caught unknown exception.  Exiting.\n", gMapWest, gMapSouth);
		fprintf(stdout,"****************************************************************************\n");

		fprintf(stderr,"ERROR (%d,%d): Caught unknown exception.  Exiting.\n", gMapWest, gMapSouth);
		exit(0);
	}
}
