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

#include "../XPTools/version.h"
#include "MeshTool_Create.h"

#include "XESInit.h"
#include "XESIO.h"
#include "DEMIO.h"
#include "MemFileUtils.h"
#include <CGAL/assertions.h>
#include "GISUtils.h"
#include "FileUtils.h"
#if LIN
#include "initializer.h"
#include <execinfo.h>
#include <stdarg.h>
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
	fprintf(stderr,"callstack not available\n");
#endif
	fprintf(stdout,"Terminating due to a CGAL exception.\n");
	fprintf(stdout,"****************************************************************************\n");
	fprintf(stdout,"ERROR  %s: %s (%s:%d).%s\n", what, expr, file, line, msg ? msg : "");
	fprintf(stdout,"****************************************************************************\n");

	fprintf(stderr,"ERROR  %s: %s (%s:%d).%s\n", what, expr, file, line, msg ? msg : "");
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

static void die_parse2(const char * msg, va_list va)
{
	vfprintf(stderr,msg,va);
	fprintf(stderr,"(%s: line %d.)\n",fname,line_num);
	exit(1);
}



int	main(int argc, char * argv[])
{
#if LIN
//	Initializer initializer(&argc, &argv, 0);
#endif

	if(argc == 2 && !strcmp(argv[1],"--version"))
	{
		print_product_version("MeshTool", MESHTOOL_VER, MESHTOOL_EXTRAVER);
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

		DEMGeo	dem_elev;


		if(strstr(argv[3],".hgt"))
		{
			if (!ReadRawHGT(dem_elev, argv[3]))
			{
				fprintf(stderr,"Could not read HGT file: %s\n", argv[3]);
				exit(1);
			}
		}
		else if(strstr(argv[3],".tif"))
		{
			if (!ExtractGeoTiff(dem_elev, argv[3]))
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

		char dump_f[24];
		sprintf(dump_f,"/%+03d%+04d",latlon_bucket(dem_elev.mSouth),latlon_bucket(dem_elev.mWest));
		string dump_dir = string(argv[4]) + dump_f;
		FILE_make_dir_exist(dump_dir.c_str());

		FILE * script = fopen(argv[1], "r");
		fname=argv[1];
		if(!script)
		{
			fprintf(stderr, "ERROR: could not open %s\n", argv[1]);
			exit(1);
		}

		int								terrain_type;
		int								layer_type = NO_VALUE;
		double							coords[4];
		char							shp_path[2048];
		char							cus_ter[256];
		char							typ[256];
		char							buf[1024];
		double							proj_lon[4],proj_lat[4],proj_s[4],proj_t[4];

		int				proj_pt = -1;


		int				use_wat;
		int				zlimit=0;
		int				is_layer = 0;

		MT_StartCreate(argv[2], dem_elev, die_parse2);

		line_num=0;
		while (fgets(buf, sizeof(buf), script))
		{
			++line_num;
			if(sscanf(buf,"DEFINE_CUSTOM_TERRAIN %d %s",&use_wat, cus_ter)==2)
			{
				proj_pt = 0;
			}
			if(sscanf(buf,"PROJECT_POINT %lf %lf %lf %lf",coords,coords+1,coords+2,coords+3)==4)
			{
				if(proj_pt==-1)
					die_parse("ERROR: PROJECT_POINT not allowed until custom terrain defined, or you have more than 4 projection pooints.\n");

				proj_lon[proj_pt] = coords[0];
				proj_lat[proj_pt] = coords[1];
				proj_s  [proj_pt] = coords[2];
				proj_t  [proj_pt] = coords[3];

				proj_pt++;
				if(proj_pt==4)
				{
					MT_CreateCustomTerrain(cus_ter,proj_lon,proj_lat,proj_s,proj_t,use_wat);
					proj_pt=-1;
				}
			}

			if(sscanf(buf,"SHAPEFILE_TERRAIN %s %s",cus_ter,shp_path)==2)
			{
				MT_LayerShapefile(shp_path,cus_ter);
			}

			if(sscanf(buf,"BACKGROUND %s",cus_ter)==1)
			{
				MT_LayerBackground(cus_ter);
			}

			if(strncmp(buf,"BEGIN_LAYER",strlen("BEGIN_LAYER"))==0)
			{
				is_layer=1;
				layer_type = NO_VALUE;
			}

			if(sscanf(buf,"BEGIN_POLYGON %s",cus_ter)==1)
			{
				terrain_type = LookupToken(cus_ter);
				if(terrain_type == -1)
					die_parse("ERROR: cannot find custom terrain type '%s'\n", cus_ter);
				if(layer_type == NO_VALUE)
				{
					layer_type = terrain_type;
					MT_LayerStart(layer_type);
					MT_PolygonStart();
				}
				else
					die_parse("ERROR: you cannot use two different terrains inside a single layer.\n");
			}

			if(sscanf(buf,"CUSTOM_POLY %s",cus_ter)==1)
			{
				terrain_type = LookupToken(cus_ter);
				if(terrain_type == -1)
					die_parse("ERROR: cannot find custom terrain type '%s'\n", cus_ter);
				if(layer_type == NO_VALUE)
				{
					layer_type = terrain_type;
					MT_LayerStart(layer_type);
					MT_PolygonStart();
				}
				else
					die_parse("ERROR: you cannot use two different terrains inside a single layer.\n");
			}

			if(strncmp(buf,"LAND_POLY",strlen("LAND_POLY"))==0)
			{
				if(layer_type == NO_VALUE)
				{
					layer_type = terrain_Natural;
					MT_LayerStart(layer_type);
					MT_PolygonStart();
				}
				else
					die_parse("ERROR: you cannot use two different terrains inside a single layer.\n");
			}
			if(strncmp(buf,"WATER_POLY",strlen("WATER_POLY"))==0)
			{
				if(layer_type == NO_VALUE)
				{
					layer_type = terrain_Water;
					MT_LayerStart(layer_type);
					MT_PolygonStart();
				}
				else
					die_parse("ERROR: you cannot use two different terrains inside a single layer.\n");
			}
			if(strncmp(buf,"APT_POLY",strlen("APT_POLY"))==0)
			{
				if(layer_type == NO_VALUE)
				{
					layer_type = terrain_Airport;
					MT_LayerStart(layer_type);
					MT_PolygonStart();
				}
				else
					die_parse("ERROR: you cannot use two different terrains inside a single layer.\n");
			}
			if(strncmp(buf,"BEGIN_HOLE",strlen("BEGIN_HOLE"))==0)
			{
				MT_HoleStart();
			}
			if(strncmp(buf,"END_HOLE",strlen("END_HOLE"))==0)
			{
				MT_HoleEnd();
			}
			if(strncmp(buf,"END_POLY",strlen("END_POLY"))==0)
			{
				MT_PolygonEnd();

				if(!is_layer)
				{
					MT_LayerEnd();
					layer_type = NO_VALUE;
				}
			}
			if(strncmp(buf,"END_LAYER",strlen("END_LAYER"))==0)
			{
				MT_LayerEnd();
				is_layer=0;
				layer_type=NO_VALUE;
			}
			if (sscanf(buf, "POLYGON_POINT %lf %lf", &coords[0], &coords[1])==2)
			{
				MT_PolygonPoint(coords[0],coords[1]);
			}
			if (sscanf(buf, "HOLE_POINT %lf %lf", &coords[0], &coords[1])==2)
			{
				MT_HolePoint(coords[0],coords[1]);
			}
			if (sscanf(buf, "ZLIMIT %d", &zlimit)==1)
			{
				MT_LimitZ(zlimit);
			}
			if(sscanf(buf,"BEGIN_NET %s",typ)==1)
			{
				MT_NetStart(typ);
			}
			if (sscanf(buf, "NET_SEG %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3])==4)
			{
				MT_NetSegment(coords[0],coords[1],coords[2],coords[3]);
			}
			if(strncmp(buf,"END_NET",strlen("END_NET"))==0)
			{
				MT_NetEnd();
			}

			if(sscanf(buf,"QMID %d %s",&use_wat,cus_ter)==2)
			{
				MT_QMID(cus_ter, use_wat);
			}

			if(sscanf(buf,"GEOTIFF %d %s",&use_wat,cus_ter)==2)
			{
				MT_GeoTiff(cus_ter, use_wat);
			}

			if(sscanf(buf,"ORTHOPHOTO %d %lf %lf %lf %lf %lf %lf %lf %lf %s",&use_wat,
					&proj_lon[0],&proj_lat[0],
					&proj_lon[1],&proj_lat[1],
					&proj_lon[2],&proj_lat[2],
					&proj_lon[3],&proj_lat[3],
					cus_ter) == 10)
			{
				proj_s[0] = proj_s[3] = 0.0;
				proj_s[1] = proj_s[2] = 1.0;
				proj_t[0] = proj_t[1] = 0.0;
				proj_t[2] = proj_t[3] = 1.0;
				MT_OrthoPhoto(cus_ter, proj_lon, proj_lat, proj_s,proj_t,use_wat);
			}

		}
		fclose(script);

		MT_FinishCreate();

		MT_MakeDSF(argv[4], argv[5]);


	} catch (exception& e) {
		fprintf(stdout,"****************************************************************************\n");
		fprintf(stdout,"ERROR: Caught unknown exception %s.  Exiting.\n", e.what());
		fprintf(stdout,"****************************************************************************\n");

		fprintf(stderr,"ERROR: Caught unknown exception %s.  Exiting.\n", e.what());
		exit(0);
	} catch (...) {
		fprintf(stdout,"****************************************************************************\n");
		fprintf(stdout,"ERROR: Caught unknown exception.  Exiting.\n");
		fprintf(stdout,"****************************************************************************\n");

		fprintf(stderr,"ERROR: Caught unknown exception.  Exiting.\n");
		exit(0);
	}

	MT_Cleanup();
}
