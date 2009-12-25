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

#include "GISTool_MiscCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "DSFLib.h"
#include "FileUtils.h"
#include "PolyRasterUtils.h"
#include "PerfUtils.h"
#include "SceneryPackages.h"
#include "DEMTables.h"
#include <md5.h>
#include "SceneryPackages.h"
#include "MapRaster.h"
#include "MapDefs.h"
#include "MapAlgs.h"
#include "GISUtils.h"
#include "MemFileUtils.h"
#include "XESIO.h"
#include "MapDefs.h"
#include "MeshAlgs.h"
#include "ForestTables.h"

static double calc_water_area(void)
{

	double total = 0.0;
	for (Pmwx::Face_iterator face = gMap.faces_begin(); face != gMap.faces_end(); ++face)
	if (!face->is_unbounded())
	if (face->data().IsWater())
	{
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			total += CGAL::to_double(Vector_2(stop->source()->point(), circ->source()->point()) *
									 Vector_2(stop->source()->point(), circ->target()->point()).perpendicular(CGAL::COUNTERCLOCKWISE));
			++circ;
		} while (circ != stop);
		for (Pmwx::Hole_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
		{
			circ = stop = *hole;
			do {
				total += CGAL::to_double(Vector_2(stop->source()->point(), circ->source()->point()) *
										 Vector_2(stop->source()->point(), circ->target()->point()).perpendicular(CGAL::COUNTERCLOCKWISE));
				++circ;
			} while (circ != stop);
		}
	}
	return total;
}


static int	DoObjToConfig(const vector<const char *>& args)
{
	const char * ouf = args[args.size()-1];
	FILE * fi = strcmp(ouf, "-") ? fopen(ouf, "w") : stdout;
	if (fi == NULL)
	{
		fprintf(stderr, "Could not open %s for writing.\n", args[args.size()-1]);
		return 1;
	}


	for (int n = 0;  n < (args.size()-1); ++n)
	{
		SpreadsheetForObject(args[n], fi);
	}

	if (fi != stdout)
		fclose(fi);

	return 0;
}

static int	DoCheckSpreadsheet(const vector<const char *>& args)
{
	CheckDEMRuleCoverage(gProgress);
	return 0;
}

static int DoCheckWaterConform(const vector<const char *>& args)
{
	// XES source, SHP source, output

	FILE * im = fopen(args[2], "wb");

	for (int y = 30; y < 40; ++y)
	for (int x = -130; x < -120; ++x)
	{
		char	shp_fname[255];
		char	xes_fname[255];
		sprintf(xes_fname,"%s%+03d%+04d/%+03d%+04d.xes", args[0], latlon_bucket(y), latlon_bucket(x), y, x);
		sprintf(shp_fname,"%s%+03d%+04d/%+03d%+04d.shp", args[1], latlon_bucket(y), latlon_bucket(x), y, x);

		vector<const char *> cmd;
		cmd.push_back("-load");
		cmd.push_back(xes_fname);
		if (GISTool_ParseCommands(cmd))
		{
			fputc(0, im);
			fputc(0, im);
			fputc(0, im);
			continue;
		}

		double wet_xes = calc_water_area();

		cmd[0] = "-shapefile";
		cmd[1] = shp_fname;

		if (GISTool_ParseCommands(cmd))
		{
			fputc(0, im);
			fputc(0, im);
			fputc(0, im);
			continue;
		}

		double wet_shp = calc_water_area();

		double err = fabs(wet_shp - wet_xes);
		double sat = max(wet_shp, wet_xes);
		double rel = (sat == 0.0) ? 0.0 : (err / sat);

		printf("%d,%d: SHP=%lf,XES=%lf   err=%lf, sat=%lf, rel=%lf\n", x, y, wet_shp, wet_xes, err, sat, rel);

		unsigned char err_c = err * 255.0 * 1000.0;
		unsigned char sat_c = sat * 255.0 * 1.0;
		unsigned char rel_c = rel * 255.0 * 10.0;

		fputc(err_c, im);
		fputc(sat_c, im);
		fputc(rel_c, im);
	}
	fclose(im);
	return 0;
}

int KillBadDSF(const vector<const char *>& args)
{
	if (DSFCheckSignature(args[0]) != dsf_ErrOK)
	{
		if (gVerbose) printf("Checksum failed: deleting %s\n", args[0]);
		FILE_delete_file(args[0],false);
		return 1;
	}
	if(gVerbose) printf("Checksum okay for: %s\n", args[0]);
	return 0;
}

int DoShowCoverage(const vector<const char *>& args)
{
	FILE * fi = fopen(args[0], "rb");
	if (fi == NULL)
	{
		fprintf(stderr, "Could not open %s\n", args[0]);
		return 1;
	}
	for (int y = gMapSouth; y < gMapNorth; ++y)
	for (int x = gMapWest; x < gMapEast; ++x)
	{
		char c = fgetc(fi);
		if (c != 0) printf("Includes %+03d%+04d\n", y, x);
	}
	fclose(fi);
	return 0;
}

int DoMakeCoverage(const vector<const char *>& args)
{
	char buf[1024];
	const char * dir = args[0];
	const char * ext = args[1];
	const char * fname = args[2];
	const char * fname2 = args[3];
	FILE * fi = fopen(fname, "wb");
	FILE * fi2 = (strcmp(fname2, "-")) ? fopen(fname2, "w") : NULL;
	if (!fi) { printf("Could not open '%s' to record output\n", fname); return 1; }
	else {
		printf("Computing coverage for %d,%d -> %d,%d at path '%s', extension '%s'\n", gMapWest, gMapSouth, gMapEast, gMapNorth,dir,ext);
		int c = 0;
		char dirchar = APL ? '/' : '\\';
		for (int y = gMapSouth; y < gMapNorth; ++y)
		for (int x = gMapWest; x < gMapEast; ++x)
		{
			sprintf(buf,"%s%+03d%+04d%c%+03d%+04d%s", dir, latlon_bucket(y), latlon_bucket(x), dirchar, y, x, ext);
			FILE * f = fopen(buf, "rb");
			if (f) {
				fputc(255,fi); ++c;
				if (fi2)
				{
					MD5_CTX	ctx;
					MD5Init(&ctx);
					int t = 0;
					while (!feof(f))
					{
						unsigned char	buf[1024];
						int len = fread(buf, 1, 1024, f);
						t += len;
						if (len)
							MD5Update(&ctx, buf, len);
					}
					MD5Final(&ctx);
					fprintf(fi2, "%+03d%+04d%c%+03d%+04d%s  Len = %30d MD5 = %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X\n",
						 latlon_bucket(y), latlon_bucket(x), dirchar, y, x, ext,
						 	t, ctx.digest[ 0],ctx.digest[ 1],ctx.digest[ 2],ctx.digest[ 3],
								ctx.digest[ 4],ctx.digest[ 5],ctx.digest[ 6],ctx.digest[ 7],
								ctx.digest[ 8],ctx.digest[ 9],ctx.digest[10],ctx.digest[11],
								ctx.digest[12],ctx.digest[13],ctx.digest[14],ctx.digest[15]);
				}
				fclose(f);

			} else fputc(0,fi);
		}
		fclose(fi);
		if (fi2) fclose(fi2);
		printf("Found %d files.\n", c);
	}
	return 0;
}

int DoMakeWetCoverage(const vector<const char *>& args)
{
	char buf[1024];
	const char * dir = args[0];
	const char * fname = args[1];
	unsigned long long total = 0;
	FILE * fi = fopen(fname, "wb");
	if (!fi) { printf("Could not open '%s' to record output\n", fname); return 1; }
	else {
		printf("Computing coverage for %d,%d -> %d,%d at path '%s'\n", gMapWest, gMapSouth, gMapEast, gMapNorth,dir);
		int c = 0;
		char dirchar = APL ? '/' : '\\';
		for (int y = gMapSouth; y < gMapNorth; ++y)
		for (int x = gMapWest; x < gMapEast; ++x)
		{
			sprintf(buf,"%s%+03d%+04d%c%+03d%+04d.xes", dir, latlon_bucket(y), latlon_bucket(x), dirchar, y, x);

			MFMemFile * mf = MemFile_Open(buf);
			if(mf == NULL)
			{
				fputc(255,fi);				
			}
			else 
			{
				Pmwx		a_map;

				ReadXESFile(mf,&a_map, NULL, NULL, NULL, gProgress);
				
				if(!a_map.is_valid())
				{
					fputc(254,fi);
				}
				else
				{					
					++c;
					total += (a_map.number_of_halfedges() / 2);
					double a = 0.0f;
					for(Pmwx::Face_iterator f = a_map.faces_begin(); f != a_map.faces_end(); ++f)
					if(!f->is_unbounded())
					if(f->data().IsWater())
						a += GetMapFaceAreaDegrees(f);
					
					fputc(a * 100.0, fi);
					printf("          %d percent.\n", (int) (a * 100.0));
				}

				MemFile_Close(mf);
			}
		}
		fclose(fi);
		printf("Found %d files.  %lld half-edges\n", c, total);
	}
	return 0;
}

int DoMakeLUCoverage(const vector<const char *>& args)
{
	const char * dir = args[0];
	const char * fname = args[2];
	float lu = atoi(args[1]);
	FILE * fi = fopen(fname, "wb");
	if (!fi) { printf("Could not open '%s' to record output\n", fname); return 1; }
	else {
		printf("Computing coverage for %d,%d -> %d,%d at path '%s'\n", gMapWest, gMapSouth, gMapEast, gMapNorth,dir);
		int c = 0;
		char dirchar = APL ? '/' : '\\';
		for (int y = gMapSouth; y < gMapNorth; ++y)
		for (int x = gMapWest; x < gMapEast; ++x)
		{
			char buf[2048];
			sprintf(buf,"%s%+03d%+04d%c%+03d%+04d.tif", dir, latlon_bucket(y), latlon_bucket(x), dirchar, y, x);

			DEMGeo	g;
			if(!ExtractGeoTiff(g,buf,dem_want_File))
			{
				fputc(255,fi);				
			}
			else 
			{
				int t = g.mWidth * g.mHeight, c = 0;
				for(int yy = 0; yy < g.mHeight; ++yy)
				for(int xx = 0; xx < g.mWidth; ++xx)
				if (g.get(xx,yy) == lu) ++c;
				
				fputc(100.0 * (float) c / (float) t,fi);
			}			
		}
		fclose(fi);
		printf("Found %d files.\n", c);
	}
	return 0;
	
}

int InitFromLU(const vector<const char *>& args)
{
	MFMemFile * mf = MemFile_Open(args[0]);
	if(mf == NULL) return 1;
	
	Curve_2	c1(Segment_2(Point_2(gMapWest,gMapSouth),Point_2(gMapEast,gMapSouth)),0);
	Curve_2	c2(Segment_2(Point_2(gMapEast,gMapSouth),Point_2(gMapEast,gMapNorth)),0);
	Curve_2	c3(Segment_2(Point_2(gMapEast,gMapNorth),Point_2(gMapWest,gMapNorth)),0);
	Curve_2	c4(Segment_2(Point_2(gMapWest,gMapNorth),Point_2(gMapWest,gMapSouth)),0);
	vector<Curve_2>	c;
	c.push_back(c1);
	c.push_back(c2);
	c.push_back(c3);
	c.push_back(c4);
	gMap.clear();
	CGAL::insert(gMap, c.begin(),c.end());

	const unsigned char * mem = (const unsigned char * ) MemFile_GetBegin(mf);

	int w = mem[360*(gMapSouth+90)+(gMapWest+180)];
	printf("Tile is %d%% wet.\n", w);
	int lu = (w > 50) ? terrain_Water : NO_VALUE;
	for(Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	if(!f->is_unbounded())
		f->data().mTerrainType = lu;
	
	MemFile_Close(mf);
	return 0;
}

int InitFromWet(const vector<const char *>& args)
{
	Curve_2	c1(Segment_2(Point_2(gMapWest,gMapSouth),Point_2(gMapEast,gMapSouth)),0);
	Curve_2	c2(Segment_2(Point_2(gMapEast,gMapSouth),Point_2(gMapEast,gMapNorth)),0);
	Curve_2	c3(Segment_2(Point_2(gMapEast,gMapNorth),Point_2(gMapWest,gMapNorth)),0);
	Curve_2	c4(Segment_2(Point_2(gMapWest,gMapNorth),Point_2(gMapWest,gMapSouth)),0);
	vector<Curve_2>	c;
	c.push_back(c1);
	c.push_back(c2);
	c.push_back(c3);
	c.push_back(c4);
	gMap.clear();
	CGAL::insert(gMap, c.begin(),c.end());

	int lu = terrain_Water;
	for(Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	if(!f->is_unbounded())
		f->data().mTerrainType = lu;
	
	return 0;
}


#define make_terrain_package_HELP \
"-make_terrain_package <path to pack base\n"\
"This command creates a scenery package with the library, ter, pol and png files based on the\n"\
"current spreadsheet.  Pass in one argument, the directy path of the scenery package, with\n"\
"trailing slash.  PNG files are only created if they do not already exist.  All ter files are\n"\
"updated/rewritten, as is the library.\n"
int DoMakeTerrainPackage(const vector<const char *>& args)
{
	return CreateTerrainPackage(args[0],true);
}

#define dump_forests_HELP \
"-dump_forests [<proxy>]\n"\
"This dumps out all forests used in the spreadsheet in a format appropriate for library files.  If a\n"\
"proxy forest file is provided, then the format is modified to export all forests to the proxy instead\n"\
"of to their real export type.  This can be useful if forest artwork is not finisihed yet.  Proxy does not\n"\
"need .for at the end."
int DoDumpForests(const vector<const char *>& args)
{
	set<int>		ts;
	GetForestTypes(ts);
	for(set<int>::iterator i = ts.begin(); i != ts.end(); ++i)
	{		
		printf("EXPORT\tlib/g8/%s.for\t%s.for\n", FetchTokenString(*i), args.size() > 0 ? args[0] : FetchTokenString(*i));
	}
	return 0;
}


static int DoHack(const vector<const char *>& args)
{
	DEMGeo	lu_forest(gDem[dem_LandUse]);
	DEMGeo&	lu_landuse(gDem[dem_LandUse]);
	DEMGeo&	lu_temp(gDem[dem_Temperature]);
	DEMGeo&	lu_temp_range(gDem[dem_TemperatureRange]);
	DEMGeo&	lu_rain(gDem[dem_Rainfall]);

	for(int y = 0; y < lu_forest.mHeight; ++y)
	for(int x = 0; x < lu_forest.mWidth ; ++x)
	{
		double lon = lu_landuse.x_to_lon(x);
		double lat = lu_landuse.y_to_lat(y);
		lu_forest(x,y) =
			FindForest(lu_landuse.get(x,y),
						lu_temp.value_linear(lon, lat),
//						lu_temp_range.value_linear(lon, lat),
						lu_rain.value_linear(lon, lat));
						
		
	}
	
	

	MapFromDEM(lu_forest, 0, 0, gDem[dem_LandUse].mWidth, gDem[dem_LandUse].mHeight, -2010/*lu_globcover_WATER*/, gMap);
	
	MapSimplify(gMap, 0.002);
	
}

/*
static int DoHack(const vector<const char *>& args)
{
	PolyRasterizer	rasterizer;
	
	DEMGeo temp(1201,1201);
	temp.mEast = gMapEast;
	temp.mWest = gMapWest;
	temp.mNorth = gMapNorth;
	temp.mSouth = gMapSouth;
	
	
	SetupWaterRasterizer(gMap, temp, rasterizer);
	
	BoxRasterizer	brasterizer(&rasterizer, 50, 1150, 2);
	double x1,y1,x2,y2;
	while(brasterizer.GetNextBox(x1,y1,x2,y2))
	{
		x1 = ceil(x1);
		x2 = floor(x2);

		Point2	p1(temp.x_to_lon(x1),temp.y_to_lat(y1));
		Point2	p2(temp.x_to_lon(x2),temp.y_to_lat(y2));
		
		if(p1.x() < p2.x())
		{
			debug_mesh_line(Point2(p1.x(),p1.y()),Point2(p1.x(),p2.y()),1,1,1,1,1,1);
			debug_mesh_line(Point2(p2.x(),p1.y()),Point2(p2.x(),p2.y()),1,1,1,1,1,1);
			debug_mesh_line(Point2(p1.x(),p1.y()),Point2(p2.x(),p1.y()),1,1,1,1,1,1);
			debug_mesh_line(Point2(p1.x(),p2.y()),Point2(p2.x(),p2.y()),1,1,1,1,1,1);
		}
	}	
}
*/

static int DoMeshErrStats(const vector<const char *>& s)
{
	float minv, maxv, mean, devsq;
	int n = CalcMeshError(gTriangulationHi, gDem[dem_Elevation], minv, maxv,mean,devsq, ConsoleProgressFunc);

	printf("mean=%f min=%f max=%f std dev = %f", mean, minv, maxv, devsq);
	return 0;
}

static	GISTool_RegCmd_t		sMiscCmds[] = {
{ "-kill_bad_dsf", 1, 1, KillBadDSF,				"Delete a DSF file if its checksum fails.", "" },
{ "-showcoverage", 1, 1, DoShowCoverage,			"Show coverage of a file as text", "Given a raw 360x180 file, this prints the lat-lon of every none-black point.\n" },
{ "-coverage", 4, 4, DoMakeCoverage, 				"prefix suffix master, md5 - make coverage.", "This makes a black & white coverage indicating what files exist.  Optionally also prints md5 signature of each file to another text file." },
{ "-wetcoverage", 2, 2, DoMakeWetCoverage,			"dir output.", "This produces a coverage from XES files - 0-100 = amount of water, 255=missing,254=invalid map.\n" },
{ "-lucoverage", 3, 3, DoMakeLUCoverage,			"dir LU output.", "This makes a coverage from geotif with a certain pixel value being treated as water.  Water=0-100,255=file missing or broken.\n" },
{ "-luinit", 1, 1, InitFromLU,						"init from landuse (LU file)." ,"Given a water coverage, this inits our tile to a single square that is all wet or dry, base on the coverage. " },
{ "-wetinit", 0, 0, InitFromWet,						"init to all water.", ""},
{ "-obj2config", 	2, -1, 	DoObjToConfig, 			"Make obj spreadsheet from a real OBJ.", "" },
{ "-checkdem",		0, 0,  DoCheckSpreadsheet,		"Check spreadsheet coverage.", "" },
{ "-checkwaterconform", 3, 3, DoCheckWaterConform, 	"Check water matchup", "" },
{ "-forest_types",	0,	1, DoDumpForests,			"Output types of forests from the spreadsaheet.", dump_forests_HELP },
{ "-make_terrain_package", 1, 1, DoMakeTerrainPackage, "Create or update a terrain package based on the spreadsheets.", make_terrain_package_HELP },
{ "-mesh_err_stats", 0, 0, DoMeshErrStats,			"Print statistics about mesh error.", "" },
{ "-hack",				   0, 0, DoHack, "", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterMiscCmds(void)
{
	GISTool_RegisterCommands(sMiscCmds);
}
