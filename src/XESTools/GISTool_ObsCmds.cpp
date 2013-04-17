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

#include "GISTool_ObsCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "PlatformUtils.h"
#include "Airports.h"
#include "MapDefs.h"
#include "DEMDefs.h"
#include "AptAlgs.h"
#include "AptRouting.h"
#include "FileUtils.h"
#include "AssertUtils.h"
#include "FAA_Obs.h"
#include "EnumSystem.h"
#include "MiscFuncs.h"
#include "GISUtils.h"
#include "AptIO.h"


/*
	This code "indexes" current objs - was never imported! :-(
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

*/

// -obs <type> <files>
static int DoObsImport(const vector<const char *>& args)
{
	const char * format = args[0];
	for (int n = 1; n < args.size(); ++n)
	{
		int countl = gFAAObs.size();
		bool	ok = false;
		const char * fname = args[n];
			 if (!strcmp(format, "faa")) ok = LoadFAAObsFile(fname);
		else if (!strcmp(format, "deg")) ok = ReadDegFile(fname);
		else if (!strcmp(format, "old")) ok = LoadLegacyObjectArchive(fname);
		else if (!strcmp(format, "apt")) ok = ReadAPTNavAsObs(fname);
		else if (!strcmp(format, "nav")) ok = ReadAPTNavAsObs(fname);
		else if (!strcmp(format, "asr")) ok = LoadFAARadarFile(fname, true);
		else if (!strcmp(format, "arsr")) ok = LoadFAARadarFile(fname, false);
		else							 printf("Unknown format %s\n", format);
		if (!ok)
			fprintf(stderr, "Could not import obstacles from %s\n", fname);
		else
			printf("Imported %s (%lld obs)\n", fname, (long long)gFAAObs.size() - countl);
		if (!ok) return 1;
	}
	printf("Imported %llu obstacles.\n", (unsigned long long)gFAAObs.size());

	ApplyObjects(gMap);
//	gFAAObs.clear();
	return 0;
}

static int DoAptImport(const vector<const char *>& args)
{
	gApts.clear();
	gAptIndex.clear();
	for(int n = 0; n < args.size(); ++n)
	{
		AptVector a;
		string err = ReadAptFile(args[n], a);
		gApts.insert(gApts.end(),a.begin(),a.end());
		if (!err.empty()) {fprintf(stderr,"Error importing %s: %s\n", args[n],err.c_str()); return 1;}
	}
	IndexAirports(gApts,gAptIndex);
	return 0;
}


static int DoAptExport(const vector<const char *>& args)
{
	if (!WriteAptFile(args[0], gApts, LATEST_APT_VERSION)) return 1;
	return 0;
}

static int DoAptBulkExport(const vector<const char *>& args)
{
	if (gProgress)		gProgress(0, 1, "Indexing apt.dat", 0.0);
	for (int y = -90; y < 90; ++y)
	{
		for (int x = -180; x < 179; ++x)
		{
			Bbox2	bounds(x, y, x+1,y+1);
			set<int>	apts;
			FindAirports(bounds, gAptIndex, apts);

			AptVector	aptCopy;
			for (set<int>::iterator iter = apts.begin(); iter != apts.end(); ++iter)
			{
				if (bounds.overlap(gApts[*iter].bounds))
					aptCopy.push_back(gApts[*iter]);
			}

			if (!aptCopy.empty())
			{
				char	path[1024];
				sprintf(path, "%s%+03d%+04d%c", args[0],
								latlon_bucket(y), latlon_bucket(x), DIR_CHAR);
				FILE_make_dir_exist(path);
				sprintf(path, "%s%+03d%+04d%c%+03d%+04d.apt", args[0],
								latlon_bucket(y), latlon_bucket(x), DIR_CHAR, y, x);
				WriteAptFile(path, aptCopy, LATEST_APT_VERSION);
			}
		}
		if (gProgress)	gProgress(0, 1, "Indexing apt.dat", (double) (y+90) / 180.0);
	}
	if (gProgress)		gProgress(0, 1, "Indexing apt.dat", 1.0);
	return 0;
}

static void AssertThrower(const char * msg, const char * file, int line)
{
	if (gVerbose)
		printf("Failure processing airport: %s\n%s: %d\n", msg, file, line);
	throw msg;
}

static int DoAptInfo(const vector<const char *>& args)
{
	set<int>	new_bounded;
	set<int>	new_unbounded;
	set<int>	old_bounded;
	set<int>	empty_bounded;
	set<int>	new_old;
	for(int a = 0; a < gApts.size(); ++a)
	{
		if(gApts[a].boundaries.empty())
		{
			if(!gApts[a].taxiways.empty())
			{
				if (gApts[a].pavements.empty())		new_unbounded.insert(a);
				else								new_old.insert(a);
			}
		}
		else
		{
			if (gApts[a].pavements.empty())
			{
				if(gApts[a].taxiways.empty())			empty_bounded.insert(a);
				else									new_bounded.insert(a);
			}
			else
			{
				old_bounded.insert(a);
			}
		}
	}

	#define PRINT_LIST(vec,cap)	\
		printf(cap); \
		for (set<int>::iterator bd = vec.begin(); bd != vec.end(); ++bd) { \
			printf("   '%s' %s (%.6lf,%.06lf\n", \
				gApts[*bd].icao.c_str(), gApts[*bd].name.c_str(), \
				gApts[*bd].bounds.p1.x(), \
				gApts[*bd].bounds.p1.y()); }

	PRINT_LIST(new_bounded,"New airports with boundaries.\n");
	PRINT_LIST(new_unbounded,"New airports with no boundaries.\n");
	PRINT_LIST(empty_bounded,"Boundary-only airports.\n");
	PRINT_LIST(old_bounded,"Old airports with bounds.\n");
	PRINT_LIST(new_old,"New-old mixes!\n");
	return 0;
}

static int DoAptGenBounds(const vector<const char *>& args)
{
	for(int n = 0; n < gApts.size(); ++n)
	{
		if(gProgress)
		if ((n % 100) == 0)
			gProgress(0, 1, "Processing airports", (float) n / (float) gApts.size());
		GenBoundary(&gApts[n]);
		#if OPENGL_MAP
			GenerateOGL(&gApts[n]);
		#endif
	}
	if(gProgress)
			gProgress(0, 1, "Processing airports", 1.0);
	return 0;
}

static int DoAptFilter(const vector<const char *>& args)
{
	if (gApts.empty()) return 0;

	int wv = atoi(args[0]);
	int wb = strcmp(args[1],"yes")==0;
	if(gVerbose) printf("Want version: %d.  Want boundaries: %s.\n", wv, wb ? "yes" : "no");
	if(gVerbose) printf("Before filter: %llu airports.\n", (unsigned long long)gApts.size());

	vector<int>	keep(gApts.size());
	int keep_ctr = 0;

	for(int n = 0; n < gApts.size(); ++n)
	{
		if(gProgress)
		if ((n % 100) == 0)
			gProgress(0, 2, "Filtering airports", (float) n / (float) gApts.size());

		int v = (gApts[n].taxiways.empty()) ? 810 : 850;
		int b = (gApts[n].boundaries.empty()) ? 0 : 1;
		if (wv != v || wb != b)
		{
			keep[n] = 0;
		}
		else
		{
			++keep_ctr;
			keep[n] = 1;
		}
	}
	if(gProgress)
		gProgress(0, 2, "Filtering airports", 1.0);

	AptVector keepers(keep_ctr);
	int m = 0;
	for(int n = 0; n < gApts.size(); ++n)
	{
		if(gProgress)
		if ((n % 100) == 0)
			gProgress(1, 2, "Deleting airports", (float) n / (float) gApts.size());


		if(keep[n])
		{
			swap(gApts[n], keepers[m]);
			++m;
		}
	}
	swap(gApts,keepers);

	gAptIndex.clear();
	IndexAirports(gApts,gAptIndex);

	if(gProgress)
		gProgress(1, 2, "Deleting airports", 1.0);


	if(gVerbose) printf("After filter: %llu airports.\n", (unsigned long long)gApts.size());
	return 0;
}

static int DoAptRouting(const vector<const char *>& args)
{
	AssertHandler_f	h  = InstallDebugAssertHandler(AssertThrower);
	AssertHandler_f dh = InstallAssertHandler(AssertThrower);
	int ok = 0;
	int bad = 0;
	set<int>	bad_idx;
	for (int a = 0; a < gApts.size(); ++a)
	{
		try {
			if ((a % 100) == 0)
				gProgress(0, 1, "Processing airports", (float) a / (float) gApts.size());

			vector<Polygon2>	windings;
			
			GetAptPolygons(gApts[a], 0.0001, windings);
			
			WindingToFile(windings,"test.txt");
			WindingFromFile(windings,"test.txt");

			vector<Point2>	poi;
			GetAptPOI(&gApts[a], poi);

			cgal_net_t route;
			if (make_map_with_skeleton(windings, poi, route))
				++ok;
			else
			{
				++bad;
				bad_idx.insert(a);
				if (gVerbose)
					printf("There was a validation problem with the airport '%s' %s\n",
						gApts[a].icao.c_str(), gApts[a].name.c_str());
			}
		} catch (const char * msg) {
			if (gVerbose)
				printf("Assertion failure stopped airport: %s\n", msg);
			++bad;
			bad_idx.insert(a);
			if (gVerbose)
				printf("There was a problem with the airport '%s' %s\n",
					gApts[a].icao.c_str(), gApts[a].name.c_str());

		} catch (exception& e) {
			if (gVerbose)
				printf("Exception stopped airport: %s\n", e.what());
			++bad;
			bad_idx.insert(a);
			if (gVerbose)
				printf("There was a problem with the airport '%s' %s\n",
					gApts[a].icao.c_str(), gApts[a].name.c_str());


		} catch (...) {
			if (gVerbose)
				printf("Unknown Exception stopped airport.\n");
			++bad;
			bad_idx.insert(a);
			if (gVerbose)
				printf("There was a problem with the airport '%s' %s\n",
					gApts[a].icao.c_str(), gApts[a].name.c_str());
		}
	}
	printf("%d OK, %d bad.\n", ok, bad);

	if (!bad_idx.empty())
	{
		for (set<int>::iterator bd = bad_idx.begin(); bd != bad_idx.end(); ++bd)
		{
			printf("There was a problem with the airport '%s' %s (%.6lf,%.06lf\n",
				gApts[*bd].icao.c_str(), gApts[*bd].name.c_str(),
				gApts[*bd].bounds.p1.x(),
				gApts[*bd].bounds.p1.y());
		}
	}

	InstallDebugAssertHandler(h);
	InstallAssertHandler(dh);
	return 0;

}

static int DoAptTest(const vector<const char *>& args)
{
	AssertHandler_f	h  = InstallDebugAssertHandler(AssertThrower);
	AssertHandler_f dh = InstallAssertHandler(AssertThrower);
	int ok = 0;
	int bad = 0;
	set<int>	bad_idx;
	for (int a = 0; a < gApts.size(); ++a)
	{
		try {
			if ((a % 100) == 0)
				gProgress(0, 1, "Processing airports", (float) a / (float) gApts.size());
			Pmwx		victim;
			AptVector	one;
			DEMGeo		foo(4,4);
			foo.mWest = -180.0;
			foo.mEast =  180.0;
			foo.mSouth = -90.0;
			foo.mNorth =  90.0;
			DEMGeo		bar(foo);
			one.push_back(gApts[a]);

			ProcessAirports(one, victim, foo, bar, false, false, false, NULL);
//			if (gVerbose) printf("OK '%s' %s\n",
//				gApts[a].icao.c_str(), gApts[a].name.c_str());
			++ok;
		} catch (const char * msg) {
			if (gVerbose)
				printf("Assertion failure stopped airport: %s\n", msg);
			++bad;
			bad_idx.insert(a);
			if (gVerbose)
				printf("There was a problem with the airport '%s' %s\n",
					gApts[a].icao.c_str(), gApts[a].name.c_str());

		} catch (exception& e) {
			if (gVerbose)
				printf("Exception stopped airport: %s\n", e.what());
			++bad;
			bad_idx.insert(a);
			if (gVerbose)
				printf("There was a problem with the airport '%s' %s\n",
					gApts[a].icao.c_str(), gApts[a].name.c_str());


		} catch (...) {
			if (gVerbose)
				printf("Unknown Exception stopped airport.\n");
			++bad;
			bad_idx.insert(a);
			if (gVerbose)
				printf("There was a problem with the airport '%s' %s\n",
					gApts[a].icao.c_str(), gApts[a].name.c_str());
		}
	}
	printf("%d OK, %d bad.\n", ok, bad);

	if (!bad_idx.empty())
	{
		for (set<int>::iterator bd = bad_idx.begin(); bd != bad_idx.end(); ++bd)
		{
			printf("There was a problem with the airport '%s' %s (%.6lf,%.06lf\n",
				gApts[*bd].icao.c_str(), gApts[*bd].name.c_str(),
				gApts[*bd].bounds.p1.x(),
				gApts[*bd].bounds.p1.y());
		}
	}

	InstallDebugAssertHandler(h);
	InstallAssertHandler(dh);
	return 0;
}

static int DoShowObjRange(const vector<const char *>& args)
{
	map<int, float> mins, maxs;
	int num = GetObjMinMaxHeights(mins, maxs);
	printf("Info on %llu types.\n", (unsigned long long)mins.size());
	for(map<int,float>::iterator i = mins.begin(); i != mins.end(); ++i)
	{
		printf("%30s %7d %7d\n",
			FetchTokenString(i->first),
			(int) mins[i->first], (int) maxs[i->first]);
	}
	printf("Processed %d obs.\n", num);
	return 0;
}

static int DoBuildObjLib(const vector<const char *>& args)
{
	BuildFakeLib(args[0]);
	return 0;
}

static	GISTool_RegCmd_t		sObsCmds[] = {
{ "-obs", 			2, -1, DoObsImport, 		"Import obstacles.", "-obs faa|deg|old|apt|nav|asr|arsr <file>\nLoad an object file, NOT old ones, and import into current map.\n"
			"faa    FAA digital obstacle file (DOF) text file.\n"
			"deg    One-degree imported OBS files (this is the format we usually keep our files in.\n"
			"old    Old binary format OBS files, sources to the old v6 render.  Used for Europe for now.\n"
			"apt    Import an X-Plane apt.dat and create obstacles out of the airport furniture.\n"
			"nav    Imoprt an X-Plane nav.dat and create obstacles out of the navaids.\n"
			"asr    Import an FAA ASR file from the digital aero chart suplement (DAC) - pull out the asr data from asr.dat.\n"
			"arsr   Import an FAA ARSR file from the digital aero chart suplement (DAC) - pull out the arsr data from asr.dat.\n" },
{ "-apt", 			1, -1, DoAptImport, 			"Import airport data.", "-apt <file>\nClear loaded airports and load from this file." },
{ "-aptwrite", 		1, 1, DoAptExport, 			"Export airport data.", "-aptwrite <file>\nExports all loaded airports to one apt.adt file." },
{ "-aptindex", 		1, 1, DoAptBulkExport, 		"Export airport data.", "-aptindex <export_dir>/\nExport all loaded airports to a directory as individual tiled apt.dat files." },
{ "-aptrouting",	0, 0, DoAptRouting,			"Test the routing-generation code.", "-aptrouting\nTest the routing code on every single airport." },
{ "-apttest", 		0, 0, DoAptTest, 			"Test airport procesing code.", "-apttest\nThis command processes each loaded airport against an empty DSF to confirm that the polygon cutting logic works.  While this isn't a perfect proxy for the real render, it can identify airport boundaries that have sliver problems (since this is done before the airport is cut into the DSF." },
{ "-aptinfo", 		0, 0, DoAptInfo, 			"Test airport procesing code.", "-apttest\nThis command prints out diagnostics about all airports." },
{ "-aptfilter",		2, 2, DoAptFilter,			"Filter airports.", "-aptfilter <810|850> <yes|no>\nFilters airports to only take ones with taxiways of a certain version and boundaries (or not).\n" },
{ "-aptboundary",	0,	0,DoAptGenBounds,		"Generate boundaries", "-aptboundary\nGenerates boundaries for all airports that don't have them.\n" },
{ "-obsrange",		0,	0, DoShowObjRange,		"Show object height ranges.", "-obsrange\nPrints the range of heights of loaded objects, by type." },
{ "-makeobjlib",	1,	1,	DoBuildObjLib,		"Make a fake obj lib of placeholder objects.", "-makeobjlib <package>/\nMakes a package of stub objects that fit the loaded obj descs for autogen." },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterObsCmds(void)
{
	GISTool_RegisterCommands(sObsCmds);
}


