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
#include "PerfUtils.h"
#include "SceneryPackages.h"
#include "DEMTables.h"
#include <md5.h>
#include "SceneryPackages.h"

#include "MapDefs.h"
#include "GISUtils.h"

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


#define make_terrain_package_HELP \
"This command creates a scenery package with the library, ter, pol and png files based on the\n"\
"current spreadsheet.  Pass in one argument, the directy path of the scenery package, with\n"\
"trailing slash.  PNG files are only created if they do not already exist.  All ter files are\n"\
"updated/rewritten, as is the library.\n"
int DoMakeTerrainPackage(const vector<const char *>& args)
{
	return CreateTerrainPackage(args[0],true);
}


int DoDumpForests(const vector<const char *>& args)
{
	map<int,int>	fm;
	set<int>		ts;
	GetForestMapping(fm);
	for(map<int,int>::iterator i = fm.begin(); i != fm.end(); ++i)
	{
		ts.insert(i->second);
		printf("EXPORT\tlib/g8/%s.for\t%s.for\n", FetchTokenString(i->first), FetchTokenString(i->second));
	}
	for(set<int>::iterator i = ts.begin(); i != ts.end(); ++i)
		printf("# %s.for\n", FetchTokenString(*i));
	return 0;
}


static	GISTool_RegCmd_t		sMiscCmds[] = {
{ "-kill_bad_dsf", 1, 1, KillBadDSF,				"Delete a DSF file if its checksum fails.", "" },
{ "-showcoverage", 1, 1, DoShowCoverage,			"Show coverage of a file as text", "" },
{ "-coverage", 4, 4, DoMakeCoverage, 				"prefix suffix master, md5 - make coverage.", "" },
{ "-obj2config", 	2, -1, 	DoObjToConfig, 			"Make obj spreadsheet from a real OBJ.", "" },
{ "-checkdem",		0, 0,  DoCheckSpreadsheet,		"Check spreadsheet coverage.", "" },
{ "-checkwaterconform", 3, 3, DoCheckWaterConform, 	"Check water matchup", "" },
{ "-forest_types",	0,	0, DoDumpForests,			"Output types of forests from the spreadsaheet.", "" },
{ "-make_terrain_package", 1, 1, DoMakeTerrainPackage, "Create or update a terrain package based on the spreadsheets.", make_terrain_package_HELP },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterMiscCmds(void)
{
	GISTool_RegisterCommands(sMiscCmds);
}
