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
#include <CGAL/assertions.h>


void	CGALFailure(
        const char* what, const char* expr, const char* file, int line, const char* msg)
{
	fprintf(stdout,"Terminating due to a CGAL exception.\n");
	fprintf(stdout,"****************************************************************************\n");
	fprintf(stdout,"ERROR  (%d,%d) %s: %s (%s:%d).%s\n", gMapWest, gMapSouth, what, expr, file, line, msg ? msg : "");
	fprintf(stdout,"****************************************************************************\n");

	fprintf(stderr,"ERROR  (%d,%d) %s: %s (%s:%d).%s\n", gMapWest, gMapSouth, what, expr, file, line, msg ? msg : "");
	exit(1);
}




int	main(int argc, char * argv[])
{
	if(argc == 2 && !strcmp(argv[1],"--version"))
	{
		fprintf(stdout, "MeshTool version 1.0b2, compiled " __DATE__ ".\n");
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
		
		MFMemFile *	xes = MemFile_Open(argv[2]);
		if(xes == NULL)
		{
			fprintf(stderr,"ERROR: could not read:%s\n", argv[2]);
			exit(1);
		}
		{
			Pmwx		dummy_vec;
			CDT			dummy_mesh;
			AptVector	dummy_apt;
			ReadXESFile(xes, &dummy_vec, &dummy_mesh, &gDem, &dummy_apt, ConsoleProgressFunc);
		}
		MemFile_Close(xes);
		
		if (!ReadRawHGT(gDem[dem_Elevation], argv[3]))
		{
			fprintf(stderr,"Could not read HGT file: %s\n", argv[3]);
			exit(1);
		}
		
		FILE * script = fopen(argv[1], "r");
		if(!script)
		{
			fprintf(stderr, "ERROR: could not open %s\n", argv[1]);
			exit(1);
		}
		char buf[1024];

		vector<Point2>	ring;
		int				terrain_type;
		double			coords[4];
		char			cus_ter[256];
		int last_ter = -1;
		int proj_pt;
		tex_proj_info pinfo;
		int use_wat;
		while (fgets(buf, sizeof(buf), script))
		{
			if(sscanf(buf,"DEFINE_CUSTOM_TERRAIN %d %s",&use_wat, cus_ter)==2)
			{
				proj_pt = 0;
				if(LookupToken(cus_ter) != -1)
				{
					fprintf(stderr,"ERROR: terrain '%s' already Defined or name is reserved.\n", cus_ter);
					exit(1);
				}
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
				gNaturalTerrainTable.insert(gNaturalTerrainTable.begin(),nt);				
			}
			if(sscanf(buf,"PROJECT_POINT %lf %lf %lf %lf",coords,coords+1,coords+2,coords+3)==4)
			{
				if(last_ter==-1)
				{
					fprintf(stderr,"PROJECT_POINT not allowde until custom terrain defined.\n");
					exit(1);
				}
				pinfo.corners[proj_pt] = Point2(coords[0],coords[1]);
				pinfo.ST[proj_pt] = Point2(coords[2],coords[3]);
				proj_pt++;
				if(proj_pt==4)
				{
					gTexProj[last_ter] = pinfo;
					last_ter = -1;
				}
			}

			if(sscanf(buf,"CUSTOM_POLY %s",cus_ter)==1)
			{
				ring.clear();
				terrain_type = LookupToken(cus_ter);
				if(terrain_type == -1)
				{
					fprintf(stderr,"ERROR: cannot find custom terrain type '%s'\n", cus_ter);
					exit(1);
				}
			}

			if(strncmp(buf,"LAND_POLY",strlen("LAND_POLY"))==0)
			{
				ring.clear();
				terrain_type = terrain_Natural;
			}
			if(strncmp(buf,"WATER_POLY",strlen("WATER_POLY"))==0)
			{
				ring.clear();
				terrain_type = terrain_Water;
			}
			if(strncmp(buf,"APT_POLY",strlen("APT_POLY"))==0)
			{
				ring.clear();
				terrain_type = terrain_Airport;
			}
			if(strncmp(buf,"END_POLY",strlen("END_POLY"))==0)
			{
				if (gMap.empty())
				{
					GISFace * f = gMap.insert_ring(gMap.unbounded_face(),ring);
					f->mTerrainType = terrain_type;
				} 
				else 
				{
					Pmwx	temp;
					GISFace * f = temp.insert_ring(temp.unbounded_face(),ring);
					f->mTerrainType = terrain_type;
					OverlayMap(gMap,temp);
				}
			}
			if (sscanf(buf, "POLYGON_POINT %lf %lf", &coords[0], &coords[1])==2)
			{
				ring.push_back(Point2(coords[0],coords[1]));
			}
		}
		fclose(script);

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
		
		// -assignterrain
		AssignLandusesToMesh(gDem,gTriangulationHi,argv[4],ConsoleProgressFunc);
		
		// -exportDSF
		BuildDSF(argv[5], NULL, gDem[dem_LandUse],gTriangulationHi, /*gTriangulationLo,*/ gMap, ConsoleProgressFunc);			

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
