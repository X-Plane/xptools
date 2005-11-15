#include "GISTool_ObsCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "PlatformUtils.h"
#include "Airports.h"
#include "MapDefs.h"
#include "DEMDefs.h"
#include "AssertUtils.h"
#include "XFileTwiddle.h"
#include "FAA_Obs.h"
#include "EnumSystem.h"
#include "MiscFuncs.h"
#include "GISUtils.h"
#include "AptIO.h"

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
			printf("Imported %s (%d obs)\n", fname, gFAAObs.size() - countl);				
		if (!ok) return 1;
	}
	printf("Imported %d obstacles.\n", gFAAObs.size());

	ApplyObjects(gMap);
//	gFAAObs.clear();
	return 0;
}

static int DoAptImport(const vector<const char *>& args)
{
	gApts.clear();
	gAptIndex.clear();
	if (!ReadAptFile(args[0], gApts)) return 1;
	IndexAirports(gApts,gAptIndex);
	return 0;
}


static int DoAptExport(const vector<const char *>& args)
{
	if (!WriteAptFile(args[0], gApts)) return 1;
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
				MakeDirExist(path);
				sprintf(path, "%s%+03d%+04d%c%+03d%+04d.apt", args[0],
								latlon_bucket(y), latlon_bucket(x), DIR_CHAR, y, x);
				WriteAptFile(path, aptCopy);
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
			printf("There was a problem with the airport '%s' %s\n",
				gApts[*bd].icao.c_str(), gApts[*bd].name.c_str());
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
	printf("Info on %d types.\n", mins.size());
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
{ "-obs", 			2, -1, DoObsImport, 			"Import obstacles.", "" },
{ "-apt", 			1, 1, DoAptImport, 				"Import airport data.", "" },
{ "-aptwrite", 		1, 1, DoAptExport, 				"Export airport data.", "" },
{ "-aptindex", 		1, 1, DoAptBulkExport, 			"Export airport data.", "" },
{ "-apttest", 		0, 0, DoAptTest, 			"Export airport data.", "" },
{ "-obsrange",		0,	0, DoShowObjRange,		"Show object height ranges.", "" },
{ "-makeobjlib",	1,	1,	DoBuildObjLib,		"Make a fake obj lib of placeholder objects.", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterObsCmds(void)
{
	GISTool_RegisterCommands(sObsCmds);
}
