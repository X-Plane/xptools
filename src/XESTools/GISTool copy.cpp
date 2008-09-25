/*
 * Copyright (c) 2004, Laminar Research.
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
#include "TIGERImport.h"
#include "TIGERRead.h"
#include "ParamDefs.h"
#include "PerfUtils.h"
#include "TigerProcess.h"
#include "ProgressUtils.h"
#include "MapAlgs.h"
#include "DEMAlgs.h"
#include "AssertUtils.h"
#include "DSFBuilder.h"
#include "MeshAlgs.h"
#include "MemFileUtils.h"
#include "XESIO.h"
#include "Zoning.h"
#include "memistreambuf.h"
#include "SDTSRead.h"
#include "SDTSReadTVP.h"
#include "VPFImport.h"
#include "gshhs.h"
#include "VPFTable.h"
#include "AptElev.h"
#include "FAA_Obs.h"
#include "XESInit.h"
#include "NetPlacement.h"
#include "ObjPlacement.h"
#include "md5.h"
#include <sdts++/io/sio_ConverterFactory.h>
#include <sdts++/container/sc_Record.h>

/* HACK FOR WED */
vector<Point2>		gMeshPoints;
vector<Point2>		gMeshLines;

int	bucket(int p)
{
	if (p > 0) return (p / 10) * 10;
	else return ((-p + 9) / 10) * -10;
}

static	char * MakeDEMName(int e)
{
	if (e == dem_Elevation) return "Elevation";
	if (e == dem_LandUse) return "LandUse";
	if (e == dem_UrbanDensity) return "RoadDensity";
	static char buf[60];
	sprintf(buf, "Unnamed DEM %d", e);
	return buf;
}

extern void	PrintDSFFile(const char * inPath, FILE * output);
extern void GenFakeDSFFile(const char * inPath);
extern void BuildFakeLib(const char * inDir);
extern void CheckLib(const char * inDir);

static	void	dump_sdts(const char *  ifs_name, const char * mod_name);
static	void 	dump_vpf_table(const char * inFileName);
static	void	import_tiger_repository(const string& root);

extern void	SelfTestAll(void);

void	DoHelp(void)
{
	printf("GISTool <options>...\n");
	printf("-help - Print this message.\n");
	printf("-noinit - Disable loading init tables for non-GIS functions.  Must be first.\n");
	printf("-selftest - run self diagnostics.\n");
	printf("-extent west south east north\n");
	printf("-tiger TGRxxxxxx [TGRxxxxxx ....] - Import US Census data.\n");
	printf("-tigerindex <indexfile> - Import tiger data using index file.\n");
	printf("-tigerbounds <indexfile>.\n");
	printf("-glcc <filename> <key file> - Import GLCC land use data.\n");
	printf("-ida <filename> <plane> <keyfile> - Import IDA data as DEM.\n");
	printf("-geotiff <filename> - Extract GeoTiff as DEM.\n");
	printf("-srtm <filename> - Import Space Shuttle HGT file.\n");
	printf("-floatdem <filename> - Import a floating point DEM.\n");
	printf("-usgs_natural <filename> - import a USGS DEM natural format.\n");
	printf("-sdts <filename> - Import SDTS VTP data.\n");
	printf("-vpf <coverage> <tile> - Import VPF data.\n");
	printf("-gshhs <file> - Import GSHHS data.\n");
	printf("-obs faa|deg|old|apt|nav|asr|arsr filename [...filename] -  read obstacles from files.\n");
	printf("\n");
	printf("-crop - cut down the data to the extent.\n");
	printf("-simplify - remove extra junk from a map.\n");
	printf("-simplify - remove extra vectors.\n");
	printf("-save <filename> - Save current data.\n");
	printf("-load <filename> - Load current data.\n");
	printf("-bbox - Show the bounds of the current data.\n");
	printf("-validate - Perform consistency checks on the current data.\n");
	printf("\n");
	printf("-obsapply - apply loaded objects to the current scenery.\n");
	printf("-upsample - upsample enviornment.\n");
	printf("-calcslope - calculate slope of DEM.\n");
	printf("-applylanduse - apply land use params.\n");
	printf("-buildroads - build basic road data and urban density.\n");
	printf("-dozoning - zone land or man-made vs natural.\n");
	printf("-buildmesh - build the mesh.\n");
	printf("-assignlanduse - assign land use to new mesh.\n");
	printf("-removedupefeatures - remove duplicate features.\n");
	printf("-genobjects - apply many objects.\n");
	printf("-showobjectratios - print the ratio of objects created.\n");
	printf("-exportdsf <file> - export DSF file.\n");
	printf("\n");
	printf("-dumpvpf filename - dump a VPF database table.\n");
	printf("-dumpsdts filename mod - dump a VPF database table.\n");
	printf("-dumpdsf filename -- dump a DSF file.\n");
	printf("-dumpobs - dump objects in memory.\n");
	printf("\n");
	printf("-markdeg - Remember occupied degrees.\n");
	printf("-builddeg directory - write out index of loaded obstacles in degree format.\n");
	printf("-buildaptelev runway_file apt_dat_file demdir outdir - build an airport elevation database.\n");
	printf("-aptapply file - apply airport elevations to terrain.\n");
	printf("-aptapplyzap file - apply airports to urban density.\n");
	printf("-coverage dir extension map md5 - produce a raw map of file coverage.\n");
	printf("-showcoverage filename - print which files are included in a file coverage map for the given extent.\n");
	printf("-bulkconvert dir filename [filename] - bulk convert USGS natural 250K DEMs to float format.\n");
	printf("-buildlib dir - build a lib based on objects.\n");
	printf("-checklib dir - see if a lib meets spesc.\n");
}

void	CGALFailure(
        const char* what, const char* expr, const char* file, int line, const char* msg)
{
	printf("Terminating due to a CGAL exception.\n");
	fprintf(stderr,"%s: %s (%s:%d).%s\n",
		what, expr, file, line, msg ? msg : "");
	exit(1);
}

#if DEV
void	HackFixTextures(const char * fnames[4], const char * fname);
#endif

const char * GetFileNameArg(int& index, int argc, char * argv[])
{
	if (index >= argc) return NULL;
	if (*(argv[index]) == '-') return NULL;
	return argv[index++];
}

int	main(int argc, char * argv[])
{
	try {


	StElapsedTime *	total = new StElapsedTime("Total time for operation");

	// Set CGAL to throw an exception rather than just
	// call exit!
	CGAL::set_error_handler(CGALFailure);
	{
		StElapsedTime	timer("Init");
		if (argc < 2 || strcmp(argv[1], "-noinit"))
			XESInit();
	}

#if 0
//	char * 	strs[] = { "", "-sdts", "/Volumes/GIS/data/test/boston_north_MA/transportation/1577430.RD.sdts/RD01CATD.DDF", "-validate", "-bbox" };

//	char * strs[] = { "", "-sdts", "/boston_north_MA/hydrography/1577847.HY.sdts", "-bbox", "-validate" };
//	char * strs[] = { "", "-sdts", "/boston_north_MA/boundaries/1576942.BD.sdts/", "-bbox", "-validate" };

//	char * strs[] = { "", "-dumpvpf", "/Volumes/GIS/data/vmap0/v0noa/vmaplv0/dht." };
//	char * strs[] = { "", "-dumpvpf", "/Volumes/GIS/data/vmap0/v0noa/vmaplv0/noamer/trans/a/e/edg" };
//	char * strs[] = { "", "-tiger", "/Volumes/GIS/data/tiger/NY/tgr36041.zip", "-srtm", "/Volumes/GIS/data/srtm/na/N43W075.hgt.zip", "-bbox" };
//	char * strs[] = { "", "-load", "ny_with_rivers.xes", "-glcc", "/Volumes/GIS/data/glcc/gusgs2_0ll.img", "43,", "-75" };

//	char * strs[] = { "", "-extent", "-180", "0", "0", "90", "-tiger", "tgr36061.zip", "-save", "manhattan.xes" };
//	char * strs[] = { "", "-gendsf", "test.dsf", "-dumpdsf", "test.dsf" };
//	char * strs[] = { "", "-extent", "-83", "31", "-82", "32",
//						"-tigerindex", "/Volumes/GIS/data/tiger/tiger_index.txt",  };
//						"-crop", "-save", "boston.xes", };
//	char * strs[] = { "", "-extent", "-156", "19", "-155", "20", "-tigerindex" ,"/Volumes/GIS/data/tiger/tiger_index.txt", "-crop", "-simplify", "-validate" };

//	char * strs[] = { "", "-extent", "-150", "20", "-60", "50", "-coverage", "nested", "/Volumes/GIS/data/DEM3/", ".DEM", "dem_coverage.raw" };
//	char * strs[] = { "", "-bulkconvert", "converts/", "/Volumes/GIS/data/DEM250/bay_city-w" };

//	char * strs[] = { "", "-buildaptelev", "/Volumes/GIS/data/robin/us_elev.txt",
//											"/Volumes/GIS/data/DEM3/", "/Volumes/GIS/data/aptelev/" };

//	char * strs[] = { "", "-load", "broken.xes", "-upsample", "-calcslope", "-applylanduse", "-buildroads", "-buidlmesh", "-applylanduse", "-removedupefeatures", "-genobjects", "-showobjectratios" };

//	char * strs[] = { "", "-checklib", "/authoring/XP8 MASTERS/genobjects/" };
//	char * strs[] = { "", "-noinit", "-selftest" };
//	char * strs[] = { "", "-geotiff", "/Users/bsupnik/Desktop/uh_srtm_39_03.tif", "-extent", "10", "40", "11", "41", "-crop" };

//	char * strs[] = { "", "-noinit", "-vpf", "/Volumes/GIS/data/vmap0/v0noa/vmaplv0/noamer/hydro", "hJ", "-validate", "-save", "hack.xes" };

	char * strs[] = { "", "-gshhs", "/Volumes/GIS/data/GSHHS/gshhs_1.3/gshhs_c.b" };
	argv = strs;
	argc = sizeof(strs) / sizeof(strs[0]);
#endif

	vector<pair<int, int> >	marked_degrees;

	if (argc == 1)
		DoHelp();

// BAS NOTE:
//	So a physicist, a mathematician, and a computer scientist...oh, never mind.
//	There are a few files (11 actually) that blow up based on the walk-line
// location - the naive one fixes this.  Who knew??
		Pmwx		bigMap;
//		Pmwx		bigMap(new CGAL::Pm_naive_point_location<Planar_map_2>());
		DEMGeoMap	bigDem;
		CDT			bigTriangulationLo, bigTriangulationHi;
		int			mapSouth = -90;
		int			mapWest = -180;
		int			mapNorth = 90;
		int			mapEast = 180;
		TigerMap	tigerMap;

	for (int n = 1; n < argc; ++n)
	{
		if (!strcmp(argv[n], "-selftest"))
		{
			SelfTestAll();
		}
#if DEV
		if (!strcmp(argv[n], "-fixtex"))
		{
			string p = argv[++n];
			p.erase(p.length() - 4);
			string v[5];
			v[0] = p+"_C1.bmp";
			v[1] = p+"_C2.bmp";
			v[2] = p+"_C3.bmp";
			v[3] = p+"_C4.bmp";
			v[4] = p+"_C.bmp";
			const char * k[4];
			k[0] = v[0].c_str();
			k[1] = v[1].c_str();
			k[2] = v[2].c_str();
			k[3] = v[3].c_str();

			HackFixTextures(k, v[4].c_str());
		}
#endif



		if (!strcmp(argv[n], "-markdeg"))
		{
			marked_degrees.clear();
			int n = 0;
			for (int x = -180; x < 180; ++x)
			for (int y =  -90; y <  90; ++y)
			{
				if (gFAAObs.find(HashLonLat(x,y)) != gFAAObs.end())
					++n,marked_degrees.push_back(pair<int,int>(x,y));
			}
			printf("Marked %d degrees.\n", n);
		}
		if (!strcmp(argv[n], "-builddeg"))
		{
			char * fname = argv[++n];
			for (int n = 0; n < marked_degrees.size(); ++n)
			{
				int x = marked_degrees[n].first;
				int y = marked_degrees[n].second;

				char buf[1024];
				sprintf(buf,"%s%+03d%+04d.obs",fname, y, x);
				if (!WriteDegFile(buf, x, y))
				{
					fprintf(stderr,"Couldn't write file %s\n", buf);
					x = 1000;
					y = 1000;

				}
			}
		}
		if (!strcmp(argv[n], "-aptapply"))
		{
			BuildDifferentialDegree(argv[++n], mapWest, mapSouth, 1201, 1201, bigDem[dem_Elevation], false);
		}
		if (!strcmp(argv[n], "-aptapplyzap"))
		{
			BuildDifferentialDegree(argv[++n], mapWest, mapSouth, 1201, 1201, bigDem[dem_UrbanDensity], true);
		}
		if (!strcmp(argv[n], "-buildaptelev"))
		{
			const char * fname = argv[++n];
			const char * demdir = argv[++n];
			const char * outdir = argv[++n];
			if (!ReadAirportRawElevations(fname))
				printf("Could not read airport elevations.\n");
//			if (!ProcessAirportElevations(demdir))
//				printf("Could not process DEMs.\n");
//			PurgeAirports();
			if (!WriteAirportElevations(demdir, outdir))
				printf("Could not write out airport elevations.\n");
		}
		if (!strcmp(argv[n], "-bulkconvert"))
		{
			const char * dir = argv[++n];
			const char * fname;
			++n;
			while ((fname = GetFileNameArg(n, argc, argv)) != NULL)
			{
				DEMGeo	dem;
				printf("Extracting %s\n", fname);
				if (ExtractUSGSNaturalFile(dem, fname))
				{
					printf("DEM %d by %d, from %lf,%lf to %lf,%lf.\n", dem.mWidth, dem.mHeight,
						dem.mWest, dem.mSouth, dem.mEast, dem.mNorth);
					char	destBuf[1024];
					int x = dem.mWest;
					int y = dem.mSouth;
					sprintf(destBuf, "%s%+03d%+04d/%+03d%+04d.DEM", dir, bucket(y), bucket(x), y, x);
					printf("Writing file %s\n", destBuf);
					FILE * t = fopen(destBuf, "rb");
					if (t != NULL) {
						printf("%s already exists.\n", destBuf);
					} else if (!WriteFloatHGT(dem, destBuf))
						printf("Write failed.\n");
				} else {
					printf("Extract %s failed.\n", fname);
				}
			}
			--n;
		}
		if (!strcmp(argv[n], "-checklib"))
		{
			CheckLib(argv[++n]);
		}
		if (!strcmp(argv[n], "-buildlib"))
		{
			BuildFakeLib(argv[++n]);
		}
		if (!strcmp(argv[n], "-showcoverage"))
		{
			FILE * fi = fopen(argv[++n], "rb");
			for (int y = mapSouth; y < mapNorth; ++y)
			for (int x = mapWest; x < mapEast; ++x)
			{
				char c = fgetc(fi);
				if (c != 0) printf("Includes %+03d%+04d\n", y, x);
			}
			fclose(fi);
		}
		if (!strcmp(argv[n], "-coverage"))
		{
			char buf[1024];
			char * dir = argv[++n];
//			dir = "F:\\XPlane\\X-System 730\\Resources\\Earth nav data\\";
			char * ext = argv[++n];
			char * fname = argv[++n];
			char * fname2 = argv[++n];
			FILE * fi = fopen(fname, "wb");
			FILE * fi2 = (strcmp(fname2, "-")) ? fopen(fname2, "w") : NULL;
			if (!fi) printf("Could not open '%s' to record output\n", fname);
			else {
				printf("Computing coverage for %d,%d -> %d,%d at path '%s', extension '%s'\n", mapWest, mapSouth, mapEast, mapNorth,dir,ext);
				int c = 0;
				char dirchar = APL ? '/' : '\\';
				for (int y = mapSouth; y < mapNorth; ++y)
				for (int x = mapWest; x < mapEast; ++x)
				{
					sprintf(buf,"%s%+03d%+04d%c%+03d%+04d%s", dir, bucket(y), bucket(x), dirchar, y, x, ext);
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
								 bucket(y), bucket(x), dirchar, y, x, ext,
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
		}

		if (!strcmp(argv[n], "-obsapply"))
		{
			ApplyObjects(bigMap);
		}
		if (!strcmp(argv[n], "-calcslope"))
		{
			printf("Calculating slope of DEM...\n");
			CalcSlopeParams(bigDem, true,NULL);
		}
		if (!strcmp(argv[n], "-applylanduse"))
		{
			printf("Applying land use...\n");
			DeriveDEMs(bigMap, bigDem,NULL);
		}
		if (!strcmp(argv[n], "-buildroads"))
		{
			printf("Building road densities...\n");
			CalcRoadTypes(bigMap, bigDem[dem_Elevation], bigDem[dem_UrbanDensity],NULL);
		}
		if (!strcmp(argv[n], "-dozoning"))
		{
			printf("Zoning polygons.\n");
			ZoneManMadeAreas(bigMap, bigDem[dem_LandUse], bigDem[dem_Slope],NULL);
		}
		if (!strcmp(argv[n], "-buildmesh"))
		{
			if ((n+1) < argc && argv[n+1][0] != '-')
			{
				gMeshPrefs.max_mountain_points = atoi(argv[++n]);
				printf("Mountain points set to: %d\n",gMeshPrefs.max_mountain_points);
			}
			printf("Building lo res mesh...\n");
			TriangulateMesh(bigMap, bigTriangulationLo, bigDem, NULL, false);
			printf("Building hi res mesh...\n");
			TriangulateMesh(bigMap, bigTriangulationHi, bigDem, NULL, true);
			printf("Hi res: %d, Lo res: %d\n", bigTriangulationHi.number_of_faces(),bigTriangulationLo.number_of_faces());
		}
		if (!strcmp(argv[n], "-assignlanduse"))
		{
			printf("Assigning land use to hi res mesh.\n");
			AssignLandusesToMesh(bigDem[dem_LandUse],bigDem[dem_Climate],bigDem[dem_Elevation],bigDem[dem_Slope],bigDem[dem_VegetationDensity],bigTriangulationHi,true ,NULL);
			printf("Assigning land use to lo res mesh.\n");
			AssignLandusesToMesh(bigDem[dem_LandUse],bigDem[dem_Climate],bigDem[dem_Elevation],bigDem[dem_Slope],bigDem[dem_VegetationDensity],bigTriangulationLo,false,NULL);
		}
		if (!strcmp(argv[n], "-removedupefeatures"))
		{
			int c = bigMap.number_of_faces();
			char	buf[256];
			sprintf(buf, "Clearing %d polygons.", c);
			if (c > 1000)	ConsoleProgressFunc(0, 1, buf, 0.0);
			int n = 0;
			for (Pmwx::Face_iterator i = bigMap.faces_begin(); i != bigMap.faces_end(); ++i)
			if (!i->is_unbounded())
			{
				RemoveDuplicates(i);
				++n;
				if ((n%1000) == 0)
				{
					ConsoleProgressFunc(0, 1, buf, (float) n / (float) c);
				}
			}
			if (c > 1000)	ConsoleProgressFunc(0, 1, buf, 1.0);
		}
		if (!strcmp(argv[n], "-genobjects"))
		{
			StElapsedTime genObojects("Generating objects.");
			int c = bigMap.number_of_faces();
			char	buf[256];
			sprintf(buf, "Instantiating %d polygons.", c);
			if (c > 500)	ConsoleProgressFunc(0, 1, buf, 0.0);
			int n = 0;
			for (Pmwx::Face_iterator i = bigMap.faces_begin(); i != bigMap.faces_end(); ++i)
			if (!i->is_unbounded())
			{
				if (!i->IsWater())
					InstantiateGTPolygon(i, bigDem[dem_UrbanDensity], bigDem[dem_UrbanPropertyValue], DEMGeo(), DEMGeo());
				++n;
				if ((n%500) == 0)
				{
					ConsoleProgressFunc(0, 1, buf, (float) n / (float) c);
				}
			}
			if (c > 500)	ConsoleProgressFunc(0, 1, buf, 1.0);
		}
		if (!strcmp(argv[n], "-showobjectratios"))
		{
			DumpPlacementCounts();
		}
		if (!strcmp(argv[n], "-exportdsf"))
		{
			const char * f = argv[++n];
			printf("Exporting DSF file %s...\n", f);
			BuildDSF(f, bigDem[dem_LandUse], bigDem[dem_VegetationDensity], bigTriangulationHi, bigTriangulationLo, bigMap,ConsoleProgressFunc);
		}

\		if (!strcmp(argv[n], "-gendsf"))
		{
			StElapsedTime gendsf("GenDSF");
			GenFakeDSFFile(argv[++n]);
		}







	}
	delete total;
	exit(0);
	return 0;
	} catch (exception& e) {
		fprintf(stderr,"Caught unknown exception %s.  Exiting.\n", e.what());
	} catch (...) {
		fprintf(stderr,"Caught unknown exception.  Exiting.\n");
	}
}

