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
#include "RF_Selection.h"
#include "DSFLib.h"
#include "FileUtils.h"
#include "PolyRasterUtils.h"
#include "PerfUtils.h"
#include "SceneryPackages.h"
#include "DEMTables.h"
#include "ForestTables.h"
#include "ObjPlacement.h"
#include "MapBuffer.h"
#include <md5.h>
#include "Zoning.h"
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
#include "BlockFill.h"
#include "MapPolygon.h"
#include "GISTool_Globals.h"

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
			if(circ->source() != stop->source() &&
			   circ->target() != stop->source())
			total += 0.5 * CGAL::to_double(Vector_2(stop->source()->point(), circ->source()->point()) *
									 Vector_2(stop->source()->point(), circ->target()->point()).perpendicular(CGAL::CLOCKWISE));
			++circ;
		} while (circ != stop);
		for (Pmwx::Hole_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
		{
			circ = stop = *hole;
			do {
				total += 0.5 * CGAL::to_double(Vector_2(stop->source()->point(), circ->source()->point()) *
										 Vector_2(stop->source()->point(), circ->target()->point()).perpendicular(CGAL::CLOCKWISE));
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

/*
static int	DoCheckSpreadsheet(const vector<const char *>& args)
{
	CheckDEMRuleCoverage(gProgress);
	return 0;
}
*/

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

static int DoPreCheckWaterConform(const vector<const char *>& args)
{
	// layer, lu, out-file

	int layer = LookupToken(args[0]);
	int lu = LookupToken(args[1]);
	if(layer == -1)
	{
		fprintf(stderr,"Unknown layer %s\n", args[0]);
		return 1;
	}
	if(lu == -1)
	{
		fprintf(stderr,"Unknown landuse %s\n", args[1]);
		return 1;
	}

	if(gDem.count(layer) == 0)
	{
		fprintf(stderr,"Empty layer %s\n", args[0]);
		return 1;
	}
	
	DEMGeo& d(gDem[layer]);
	double wet_lu = 0.0;
	for(DEMGeo::iterator i = d.begin(); i != d.end(); ++i)
	if(*i == lu)
		wet_lu += 1.0;
	wet_lu /= (((double) d.mWidth * (double) d.mHeight));
		
	double wet_vec = calc_water_area();

	printf("By vector: %lf.  By LU: %lf.  Absolute: %lf.  Relative: %lf.\n", 
			wet_vec, 
			wet_lu,
			fabs(wet_vec-wet_lu),
			max(wet_vec,wet_lu) == 0.0 ? 0.0 :
			fabs(wet_vec-wet_lu) / max(wet_vec,wet_lu));

	if(args.size() > 2)
	{

		FILE * im = fopen(args[2], "ab");
		if(!im)
		{
			fprintf(stderr,"Could not open %s\n", args[2]);
			return 1;
		}
		fputc(255.0 * fabsf(wet_vec - wet_lu),im);
		fclose(im);
	}
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
	int thresh = 1;
	if(args.size() > 1)
		thresh = atoi(args[1]);
	FILE * fi = fopen(args[0], "rb");
	if (fi == NULL)
	{
		fprintf(stderr, "Could not open %s\n", args[0]);
		return 1;
	}
//	for (int y = gMapSouth; y < gMapNorth; ++y)
//	for (int x = gMapWest; x < gMapEast; ++x)
	for(int y = -90; y < 90; ++y)
	for(int x = -180; x < 180; ++x)
	{
		int c = fgetc(fi);
		if(y >= gMapSouth && y <= gMapNorth)
		if(x >= gMapWest  && x <= gMapEast )
			if (c >= thresh) printf("Includes %+03d%+04d (%f)\n", y, x, (float) c * 100.0 / 255.0);
	}
	fclose(fi);
	return 0;
}

int DoDiffCoverage(const vector<const char *>& args)
{
	FILE * fi1 = fopen(args[0], "rb");
	if (fi1 == NULL)
	{
		fprintf(stderr, "Could not open %s\n", args[0]);
		return 1;
	}
	FILE * fi2 = fopen(args[1], "rb");
	if (fi2 == NULL)
	{
		fprintf(stderr, "Could not open %s\n", args[1]);
		fclose(fi1);
		return 1;
	}
	for (int y = gMapSouth; y < gMapNorth; ++y)
	for (int x = gMapWest; x < gMapEast; ++x)
	{
		char c1 = fgetc(fi1);
		char c2 = fgetc(fi2);
		if (c1 != 0 && c2 == 0) printf("%+03d%+04d\n", y, x);
	}
	fclose(fi1);
	fclose(fi2);
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
			if(!ExtractGeoTiff(g,buf,dem_want_File,0))
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
	return CreateTerrainPackage(args[0],true,false);
}

#define test_terrain_package_HELP \
"-test_terrain_package <path to pack base\n"\
"This command checks an existing scenery pack to see if any files are missing.\n"
int DoTestTerrainPackage(const vector<const char *>& args)
{
	return CreateTerrainPackage(args[0],true,true);
}

#define dump_forests_HELP \
"-dump_forests [<proxy>]\n"\
"This dumps out all forests used in the spreadsheet in a format appropriate for library files.  If a\n"\
"proxy forest file is provided, then the format is modified to export all forests to the proxy instead\n"\
"of to their real export type.  This can be useful if forest artwork is not finisihed yet.  Proxy does not\n"\
"need .for at the end."
int DoDumpForests(const vector<const char *>& args)
{
	for(ForestInfoMap::iterator i = gForestInfo.begin(); i != gForestInfo.end(); ++i)
	{		
		printf("EXPORT\tlib/g8/%s.for\t%s.for\n", FetchTokenString(i->first), args.size() > 0 ? args[0] : FetchTokenString(i->first));
	}
	return 0;
}

#if OPENGL_MAP
static int DoClear(const vector<const char *>& args)
{
	for(set<Pmwx::Face_handle>::iterator i = gFaceSelection.begin(); i != gFaceSelection.end(); ++i)
	{
		(*i)->data().mObjs.clear();
		(*i)->data().mPolyObjs.clear();
	}
	return 0;
}
#endif

static int DoHack(const vector<const char *>& args)
{
		set<Pmwx::Face_handle>	fail;

	gMeshLines.clear();
	gMeshPoints.clear();

		Locator						loc(gMap);

	int ctr = 0;
	PROGRESS_START(gProgress,0,1,"Insetting")
	for (Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f, ++ctr)
	if(!f->is_unbounded())
#if OPENGL_MAP
	if(gFaceSelection.empty() || gFaceSelection.count(f))
#endif	
	{
		PROGRESS_SHOW(gProgress,0,1,"Insetting",ctr,gMap.number_of_faces())
		Polygon_with_holes_2		bounds;
		PolyInset_t					lims;
		PolygonFromFace(f, bounds, &lims, GetInsetForEdgeDegs, NULL);
		Polygon_set_2				bs;

		try {

			BufferPolygonWithHoles(bounds, &lims, 1.0 , bs);

			ValidateBuffer(gMap,f, loc, bs);
		} catch (...) {
			fail.insert(f);
			#if DEV
				debug_mesh_point(cgal2ben(f->outer_ccb()->source()->point()),1,1,0);
			#endif
		}

		vector<Polygon_with_holes_2>	all;

		bs.polygons_with_holes(back_insert_iterator<vector<Polygon_with_holes_2> >(all));

		for(vector<Polygon_with_holes_2>::iterator a = all.begin(); a != all.end(); ++a)
		{
			GISPolyObjPlacement_t	res;
			cgal2ben(*a, res.mShape);
			res.mRepType = 1;
			res.mParam = 20;
			res.mDerived = true;
			(f)->data().mPolyObjs.push_back(res);
		}
	}
	PROGRESS_DONE(gProgress,0,1,"Insetting")
	if (!fail.empty())
	{
#if OPENGL_MAP
		printf("%llu out of %llu failed.\n", (unsigned long long)fail.size(), (unsigned long long)gFaceSelection.size());
		gEdgeSelection.clear();
		gFaceSelection = fail;
		gVertexSelection.clear();
#endif		
	}
	return 1;
}

static int DoHackTestrasterizer(const vector<const char *>& args)
{
	DEMGeo d(1201,1201);

	Point_2 sw, ne;
	CalcBoundingBox(gMap, sw,ne);
	d.mWest = round(CGAL::to_double(sw.x()));
	d.mSouth = round(CGAL::to_double(sw.y()));
	d.mEast = round(CGAL::to_double(ne.x()));
	d.mNorth = round(CGAL::to_double(ne.y()));
	
	{
		PolyRasterizer<double>	r;
		SetupWaterRasterizer(gMap, d, r, terrain_Water);
	
		StElapsedTime	time_scan("time_scan line");
		
		double y = 0;
		r.StartScanline(y);
		vector<double> f;
		while(!r.DoneScan())
		{
			double yy = y+1.0;
			r.GetLine(f,yy);
			assert(f.size() % 2 == 0);
			#if DEV
			for(int n = 0; n < f.size(); n += 2)
			{
				assert(f[n] <= f[n+1]);
				debug_mesh_line(Point2(
						d.x_to_lon_double(f[n]),
						d.y_to_lat_double(y)),
								Point2(
						d.x_to_lon_double(f[n+1]),
						d.y_to_lat_double(y)),
						0.2,0.2,1, 0.2,0.2,1);
			}
			#endif			
			++y;
			r.AdvanceScanline(y);
		}
	}
	{
		PolyRasterizer<double>	rr;
		SetupWaterRasterizer(gMap, d, rr, terrain_Water);
	
		StElapsedTime	time_scan("time_scan box");
		
		BoxRasterizer<double> b(&rr);
		
		double y = 0;
		b.StartScanline(y,y+1);
		while(!b.DoneScan())
		{
			vector<double> f;
			b.GetLineTrash(f);
			assert(f.size() % 2 == 0);
			#if DEV
			for(int n = 0; n < f.size(); n += 2)
			{
				assert(f[n] <= f[n+1]);
				debug_mesh_line(Point2(
						d.x_to_lon_double(f[n]),
						d.y_to_lat_double(y+0.5)),
								Point2(
						d.x_to_lon_double(f[n+1]),
						d.y_to_lat_double(y+0.5)),
						1,1,1, 1,1,1);
			}
			#endif			
			++y;
			b.AdvanceScanline(y,y+1);
		}
	}
	return 0;
}

static int DoMeshErrStats(const vector<const char *>& s)
{
	float minv, maxv, mean, devsq;
	int n = CalcMeshError(gTriangulationHi, gDem[dem_Elevation], minv, maxv,mean,devsq, ConsoleProgressFunc);

	printf("mean=%f min=%f max=%f std dev = %f", mean, minv, maxv, devsq);
	return 0;
}

static	GISTool_RegCmd_t		sMiscCmds[] = {
{ "-kill_bad_dsf", 1, 1, KillBadDSF,				"Delete a DSF file if its checksum fails.", "" },
{ "-showcoverage", 1, 2, DoShowCoverage,			"Show coverage of a file as text", "Given a raw 360x180 file, this prints the lat-lon of every none-black point.\n" },
{ "-diffcoverage", 2, 2, DoDiffCoverage,			"Difference two coverages.","Given two raw 360x180s, shows a list of all tiles in the first but NOT the second one.\n" },
{ "-coverage", 4, 4, DoMakeCoverage, 				"prefix suffix master md5|- - make coverage.", "This makes a black & white coverage indicating what files exist.  Optionally also prints md5 signature of each file to another text file." },
{ "-wetcoverage", 2, 2, DoMakeWetCoverage,			"dir output.", "This produces a coverage from XES files - 0-100 = amount of water, 255=missing,254=invalid map.\n" },
{ "-lucoverage", 3, 3, DoMakeLUCoverage,			"dir LU output.", "This makes a coverage from geotif with a certain pixel value being treated as water.  Water=0-100,255=file missing or broken.\n" },
{ "-luinit", 1, 1, InitFromLU,						"init from landuse (LU file)." ,"Given a water coverage, this inits our tile to a single square that is all wet or dry, base on the coverage. " },
{ "-wetinit", 0, 0, InitFromWet,						"init to all water.", ""},
{ "-obj2config", 	2, -1, 	DoObjToConfig, 			"Make obj spreadsheet from a real OBJ.", "" },
//{ "-checkdem",		0, 0,  DoCheckSpreadsheet,		"Check spreadsheet coverage.", "" },
{ "-checkwaterconform", 3, 3, DoCheckWaterConform, 	"Check water matchup", "" },
{ "-compare_water",	2,3,DoPreCheckWaterConform,		"Check LU vs. vector water", "" },
{ "-forest_types",	0,	1, DoDumpForests,			"Output types of forests from the spreadsaheet.", dump_forests_HELP },
{ "-make_terrain_package", 1, 1, DoMakeTerrainPackage, "Create or update a terrain package based on the spreadsheets.", make_terrain_package_HELP },
{ "-test_terrain_package", 1, 1, DoTestTerrainPackage, "Check a terrain package based on the spreadsheets.", test_terrain_package_HELP },
{ "-mesh_err_stats", 0, 0, DoMeshErrStats,			"Print statistics about mesh error.", "" },
{ "-hack",				   1, 1, DoHack, "", "" },
#if OPENGL_MAP
{ "-clear_block",		   0, 0, DoClear, "", "" },
#endif
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterMiscCmds(void)
{
	GISTool_RegisterCommands(sMiscCmds);
}
