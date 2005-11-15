#include "GISTool_DemCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "DEMIO.h"
#include "GISUtils.h"
#include "PerfUtils.h"
#include "PlatformUtils.h"
#include "XFileTwiddle.h"
#include "MemFileUtils.h"

static int DoAnyImport(const vector<const char *>& args,
					bool (* import_f)(DEMGeo& inMap, const char * inFileName))
{
	DEMGeo&	dem = gDem[dem_Elevation];
	bool ok = import_f(dem, args[0]);
	if (!ok && gVerbose)
		fprintf(stderr, "Unable to load file %s\n", args[0]);
	else
		printf("Loaded file %s (%dx%d).\n", args[0], dem.mWidth, dem.mHeight);
	return ok ? 0 : 1;
}


static int DoHGTImport(const vector<const char *>& args)
{
	return DoAnyImport(args, ReadRawHGT);
}

static int DoFloatDEMImport(const vector<const char *>& args)
{
	return DoAnyImport(args, ReadFloatHGT);
}

static int DoShortOzImport(const vector<const char *>& args)
{
	return DoAnyImport(args, ReadShortOz);
}


static int DoUSGSNaturalImport(const vector<const char *>& args)
{
	return DoAnyImport(args, ExtractUSGSNaturalFile);
}

static int DoIDAImport(const vector<const char *>& args)
{
	return DoAnyImport(args, ExtractIDAFile);
}

static int DoGeoTiffImport(const vector<const char *>& args)
{
	return DoAnyImport(args, ExtractGeoTiff);
}

static int DoGLCCImport(const vector<const char *>& args)
{
	DEMGeo&	dem = gDem[dem_LandUse];
	if (!ExtractRawIMGFile(dem, args[0], gMapWest, gMapSouth, gMapEast, gMapNorth))
	{
		if (gVerbose)
			fprintf(stderr, "Unable to load file %s\n", args[0]);
		return 1;		
	} else {
		if (gVerbose)
			printf("Loaded file %s (%dx%d).\n", args[0], dem.mWidth, dem.mHeight);
	}
	if (!TranslateDEM(dem, args[1]))
	{
		if (gVerbose)
			fprintf(stderr, "Unable to load translation file %s\n", args[1]);
		return 1;		
	}
	return 0;
}

static int DoHGTExport(const vector<const char *>& args)
{
	WriteRawHGT(gDem[dem_Elevation], args[0]);
	return 0;
}

// -bulksrtm <src dir> <dst dir> <x> <y>
static int DoBulkConvertSRTM(const vector<const  char *>& args)
{
	DEMGeo	me, north, east, northeast;
	char	path[512], path2[512];
	
	int x = atoi(args[2]);
	int y = atoi(args[3]);
	int n;
	sprintf(path, "%s" DIR_STR "srtm_%02d_%02d.zip", args[0], x, y);
	if (!ExtractGeoTiff(me, path))
	{
		printf("File %s not found.\n", path);
		return 0;
	}
	
	if (me.mWidth != 6001 || me.mHeight != 6001)
	{
		printf("File %s has %d by %d samples - unexpected!!\n", path, me.mWidth, me.mHeight);
		return 0;
	}
	
	sprintf(path2, "%s" DIR_STR "srtm_%02d_%02d.zip", args[0], (x%72)+1, y);
	if (ExtractGeoTiff(east, path2))
	{
		if (east.mWest != me.mEast ||
			east.mSouth != me.mSouth ||
			east.mNorth != me.mNorth ||
			east.mHeight != me.mHeight)
		{
			printf("File %s has %d by %d samples - doesn't tile right with %s.!!\n", path2, east.mWidth, east.mHeight, path);
			return 0;
		}
		for (n = 0; n < me.mHeight; ++n)
			me(me.mWidth-1, n) = east(0, n);		
	}

	sprintf(path2, "%s" DIR_STR "srtm_%02d_%02d.zip", args[0], x, y - 1);
	if (ExtractGeoTiff(north, path2))
	{
		if (north.mSouth != me.mNorth ||
			north.mWest != me.mWest ||
			north.mEast != me.mEast ||
			north.mWidth != me.mWidth)
		{
			printf("File %s has %d by %d samples - doesn't tile right with %s.!!\n", path2, north.mWidth, north.mHeight, path);
			return 0;
		}
		for (n = 0; n < me.mWidth; ++n)
			me(n, me.mHeight-1) = north(n, 0);
	}

	sprintf(path2, "%s" DIR_STR "srtm_%02d_%02d.zip", args[0], (x%72)+1, y - 1);
	if (ExtractGeoTiff(northeast, path2))
	{
		if (northeast.mSouth != me.mNorth ||
			northeast.mWest != me.mEast)
		{
			printf("File %s has %d by %d samples - doesn't tile right with %s.!!\n", path2, northeast.mWidth, northeast.mHeight, path);
			return 0;
		}
		me(me.mWidth-1, me.mHeight-1) = northeast(0,0);
	}

	int i, j;
	
	DEMGeo	sub;
	for (i = 0; i < 5; ++i)
	for (j = 0; j < 5; ++j)
	{
		me.subset(sub, i * 1200, j * 1200, i * 1200 + 1200, j * 1200 + 1200);
		sprintf(path, "%s" DIR_STR "%+03d%+04d" DIR_STR, args[1], latlon_bucket(sub.mSouth), latlon_bucket(sub.mWest));
		MakeDirExist(path);
		sprintf(path, "%s" DIR_STR "%+03d%+04d" DIR_STR "%+03d%+04d.hgt.zip", args[1], latlon_bucket(sub.mSouth), latlon_bucket(sub.mWest), (int) sub.mSouth, (int) sub.mWest);
		printf("Writing %s...\n", path);
		if (!WriteRawHGT(sub, path))
		{
			printf("Error writing %s\n", path);
			return 1;
		}
	}

	return 0;
}

static DEMGeo	gMem, gMask;
static bool has_mask = false;

static int DoRemember(const vector<const char *>& args)
{
	gMem = gDem[dem_Elevation];
	return 0;
}

static int DoMaskRemember(const vector<const char *>& args)
{
	gMask.resize(1201, 1201);
	
	MFMemFile * fi = MemFile_Open(args[0]);
	if (!fi) { fprintf(stderr, "Could not open mask %s.\n", args[0]); return 1; }
	
	const unsigned char * p = (const unsigned char *) MemFile_GetBegin(fi);
	
	for (int y = 0; y < 1201; ++y)
	for (int x = 0; x < 1201; ++x)
		gMask(x,y) = *p++;
	
	MemFile_Close(fi);
	has_mask = true;
	return 0;
}

static int DoApply(const vector<const char *>& args)
{	
	if (has_mask)
	{
		DEMGeo& final(gDem[dem_Elevation]);
		Assert(final.mWidth == gMem.mWidth);
		Assert(final.mWidth == gMask.mWidth);
		Assert(final.mHeight == gMem.mHeight);
		Assert(final.mHeight == gMask.mHeight);

		int x, y;
		
		for (y = 0; y < final.mHeight; ++y)
		for (x = 0; x < final.mWidth; ++x)
		{
			float r = gMask.get(x,y);
			DebugAssert(r >= 0.0 && r <= 255.0);
			if (r != 0.0)
			{
				if (r == 255.0)
					final(x,y) = gMem(x,y);
				else {
					r = r / 255.0;
					final(x,y) = gMem(x,y) * r + final(x,y) * (1.0 - r);
				}
			}
		}

		return 0;	
	} else 
		gDem[dem_Elevation].overlay(gMem);
	return 0;
}

static	GISTool_RegCmd_t		sDemCmds[] = {
{ "-hgt", 			1, 1, DoHGTImport, 			"Import 16-bit raw HGT DEM.", "" },
{ "-hgtzip", 		1, 1, DoHGTExport, 			"Export 16-bit raw HGT DEM.", "" },
{ "-oz",			1, 1, DoShortOzImport,		"Read short DEM.", "" },
{ "-floatdem", 		1, 1, DoFloatDEMImport, 	"Import floating-point DEM", "" },
{ "-usgs_natural", 	1, 1, DoUSGSNaturalImport, 	"Import USGS Natural-format DEM", "" },
{ "-ida", 			1, 1, DoIDAImport, 			"Import IDA-format raster file.", "" },
{ "-geotiff", 		1, 1, DoGeoTiffImport, 		"Import GeoTiff DEM", "" },
{ "-glcc", 			2, 2, DoGLCCImport, 		"Import GLCC land use raster data.", "" },
{ "-bulksrtm",		4, 4, DoBulkConvertSRTM,	"Bulk convert SRTM data.", "" },
{ "-markoverlay",	0, 0, DoRemember,			"Remember the current elevation as overlay.", "" },
{ "-readmask",		1, 1, DoMaskRemember,		"Remember the current elevation as overlay.", "" },
{ "-applyoverlay",	0, 0, DoApply	,			"Use overlay.", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterDemCmds(void)
{
	GISTool_RegisterCommands(sDemCmds);
}

