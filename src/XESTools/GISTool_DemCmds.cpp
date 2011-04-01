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

#include "GISTool_DemCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "DEMIO.h"
#include "DEMAlgs.h"
#include "GISUtils.h"
#include "PerfUtils.h"
#include "PlatformUtils.h"
#include "XFileTwiddle.h"
#include "FileUtils.h"
#include "MemFileUtils.h"

#if OPENGL_MAP
#include "RF_Notify.h"
#include "RF_Msgs.h"
#endif


#define DoRasterImport_HELP \
"Usage: raster_import <flags> <format> <file> <layer> [<null>] [<fill>] [<translation>]\n"\
"Imports a single raster file.  The file must be in geographic projection\n"\
"and have an axis-aligned bounding box.\n"\
"Flagged special ops (done in this order):\n"\
" p Pad the DEM by one pixel on the north and east sides.\n"\
" s Snap DEM bounds to 1x1 degree grid.\n"\
" n Treat <null> as null value.\n"\
" f Fill voids with <fill> value.\n"\
" t Translate DEM with translation file.\n"\
" o Overlay DEM on existing DEM\n"\
" e Do not use if DEM is empty.\n"\
" c Check for conflicts in duplicate posts between new and old DEM (only makes sense with o flag.\n"\
" a GEoTiff only - allow DEM to be area data if file contains area data.\n"\
"Format can be one of: tiff, bil, hgt, ascii\n"\
"File is a unix file name.\n"\
"Layer is the string name of the layer to import.  Usuallye one of: dem_Elevation, dem_LandUse\n"\
"If <translation> is provided it is the name of a translation file name (no path) in the config folder.\n"
static int DoRasterImport(const vector<const char *>& args)
{
	bool did_skip = false;
	int mode = dem_want_Post;
	
	// format file layer [mapping]
	int layer = LookupToken(args[3]);
	if (layer == -1)
	{
		fprintf(stderr,"Unknown layer: %s\n", args[3]);
		return 1;
	}

	if(strstr(args[0],"a"))
		mode = dem_want_File;

	DEMGeo * kill = NULL, * dem = NULL;
	if(strstr(args[0],"o") || strstr(args[0],"e"))
	{
		kill = dem = new DEMGeo;
	}
	else
	{
		dem = &gDem[layer];
	}
	
	if(strcmp(args[1],"ascii") == 0)
	{
		if(!ReadARCASCII(*dem,args[2]))
		{
			fprintf(stderr,"Unable to read ARC-ASCII file %s\n", args[2]);
			return 1;
		}
	}
	else if(strcmp(args[1],"tiff") == 0)
	{
		if(!ExtractGeoTiff(*dem, args[2], mode))
		{
			fprintf(stderr,"Unable to read GeoTiff file %s\n", args[2]);
			return 1;
		}
	}
	else if(strcmp(args[1],"hgt") == 0)
	{
		if(!ReadRawHGT(*dem, args[2])) 
		{
			fprintf(stderr,"Unable to read HGT file %s\n", args[2]);
			return 1;
		}
	}
	else if(strcmp(args[1],"bil") == 0)
	{
		if(!ReadRawBIL(*dem,args[2]))
		{
			fprintf(stderr,"Unable to read BIL file %s\n", args[2]);
			return 1;
		}
	}
	else
	{
		fprintf(stderr,"Unknown importer: %s\n", args[1]);
		return 1;
	}

	if(strstr(args[0],"p"))
	{
		dem->resize_save(dem->mWidth+1,dem->mHeight+1, DEM_NO_DATA);
	}

	if(strstr(args[0],"s"))
	{
		dem->mWest =round(dem->mWest );
		dem->mEast =round(dem->mEast );
		dem->mNorth=round(dem->mNorth);
		dem->mSouth=round(dem->mSouth);
	}

	int var_param = 4;

	if(strstr(args[0],"n"))
	{
		float n = atof(args[var_param]); 
		for(int y = 0; y < dem->mHeight; ++y)
		for(int x = 0; x < dem->mWidth ; ++x)
		if(round(dem->get(x,y)) == n)
			(*dem)(x,y) = DEM_NO_DATA;
		++var_param;
	}

	if(strstr(args[0],"f"))
	{
		float f = atof(args[var_param]); 
		for(int y = 0; y < dem->mHeight; ++y)
		for(int x = 0; x < dem->mWidth ; ++x)
		if(dem->get(x,y) == DEM_NO_DATA)
			(*dem)(x,y) = f;
		++var_param;
	}

	if(strstr(args[0],"t"))
	{
		if(!TranslateDEM(*dem, args[var_param]))
		{
			fprintf(stderr,"Unable to translsate DEM using mapping file: %s\n", args[4]);
			return 1;
		}
		++var_param;
	}

	if(strstr(args[0],"e"))
	{
		bool has_data = false;
		for(int y = 0; y < dem->mHeight; ++y)
		for(int x = 0; x < dem->mWidth ; ++x)
		if(dem->get(x,y) != DEM_NO_DATA)
		{
			has_data = true;
			break;
		}
		if(has_data)
			dem->swap(gDem[layer]);
		else
			did_skip=true;
		dem = &gDem[layer];
	}

	if(strstr(args[0],"o"))
	{
		DEMGeo * dst = &gDem[layer];
		
		if( ((double)(dst->mWidth-1) * (dem->mEast-dem->mWest)) !=
			((double)(dem->mWidth-1) * (dst->mEast-dst->mWest)))
		{
			fprintf(stderr,"Unable to overlay %s onto layer %s: horizontal resolution mismatch.\n", args[2], args[3]);
			return 1;
		}
		if( ((double)(dst->mHeight-1) * (dem->mNorth-dem->mSouth)) !=
			((double)(dem->mHeight-1) * (dst->mNorth-dst->mSouth)))
		{
			fprintf(stderr,"Unable to overlay %s onto layer %s: vertical resolution mismatch.\n", args[2], args[3]);
			return 1;
		}
			
		DEMGeo	tmp;
		tmp.mWest = min(dem->mWest ,dst->mWest );
		tmp.mSouth= min(dem->mSouth,dst->mSouth);
		tmp.mEast = max(dem->mEast ,dst->mEast );
		tmp.mNorth= max(dem->mNorth,dst->mNorth);
		
		tmp.resize(
				1+(double) (dem->mWidth -1) * (tmp.mEast -tmp.mWest ) / (dem->mEast -dem->mWest ),
				1+(double) (dem->mHeight-1) * (tmp.mNorth-tmp.mSouth) / (dem->mNorth-dem->mSouth));
		tmp = DEM_NO_DATA;
		
		bool err_check = strstr(args[0],"c");
		
		for(int y = 0; y < dst->mHeight; ++y)
		for(int x = 0; x < dst->mWidth ; ++x)
		{
			int xp = tmp.map_x_from(*dst,x);
			int yp = tmp.map_y_from(*dst,y);			
			tmp(xp,yp)=dst->get(x,y);
		}

		for(int y = 0; y < dem->mHeight; ++y)
		for(int x = 0; x < dem->mWidth ; ++x)
		{
			int xp = tmp.map_x_from(*dem,x);
			int yp = tmp.map_y_from(*dem,y);			
			
			float top_val = dem->get(x,y);
			if(top_val != DEM_NO_DATA)
			{
				if(err_check)
				{
					float bot_val = tmp.get(xp,yp);
					if(top_val != bot_val && bot_val != DEM_NO_DATA)
					{
						fprintf(stderr,"Error: DEMs do not match at %d,%d\n", x,y);
						return 1;
					}					
				}
				tmp(xp,yp)=top_val;
			}
		}

		tmp.swap(*dst);
		
		dem = dst;
	}

	if(kill) 
		delete kill;
	
	
	

	#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_FileLoaded, NULL);
	#endif

	if(gVerbose) 
	{
		if(did_skip)
			printf("Skipped DEM because source file is empty and 'e' option used.\n");
		else
			printf("Imported %d x %d DEM, post=%d.\n", dem->mWidth, dem->mHeight, dem->mPost);
	}
	return 0;
}

#define DoRasterExport_HELP \
"Usage: raster_export <flags> <format> <file> <layer>\n"\
"File Format must be: tiff\n"
static int DoRasterExport(const vector<const char *>& args)
{
	int layer = LookupToken(args[3]);
	if(layer == -1)
	{
		fprintf(stderr,"Raster Layer %s - unknown layer name.\n",args[3]);
		return 1;
	}
	
	if(gDem.count(layer) == 0)
	{
		fprintf(stderr,"Raster Layer %s does not exist.\n", args[3]);
		return 1;
	}

	if(strcmp(args[1],"tiff") == 0)
	{	
		if (!WriteGeoTiff(gDem[layer], args[2]))
		{
			fprintf(stderr,"Error writing file: %s\n", args[2]);
			return 1;
		} else
			if (gVerbose)	printf("Wrote %s\n",args[2]);
	}
	else
	{
		fprintf(stderr,"Unknown export file format: %s\n", args[1]);
		return 1;
	}
	return 0;
}


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

/*
static int DoGeoTiffImport(const vector<const char *>& args)
{
	return DoAnyImport(args, ExtractGeoTiff);
}
*/

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

#define DoHGTTileExport_HELP \
"Usage: -hgt_tiles <dest_dir>\n"\
"Given a multi-tile DEM loaded into memory as elevation, this routine exports one\n"\
"SRTM.hgt style DEM for each 1x1 degree tile that has at least one non-void data point.\n"\
"The intended use is to decompose 5x5 or other large-format DEMs into 1x1s.  Note that the\n"\
"exported files are zip-compressed for size.\n"
static int DoHGTTileExport(const vector<const char *>& args)
{
	list<DEMGeo>	tiles;
	MakeTiles(gDem[dem_Elevation], tiles);
	for(list<DEMGeo>::iterator t= tiles.begin(); t != tiles.end(); ++t)
	{
		char path[2048];
		sprintf(path,"%s" DIR_STR "%+03d%+04d" DIR_STR, args[0], latlon_bucket(t->mSouth), latlon_bucket(t->mWest));
		FILE_make_dir_exist(path);
		sprintf(path, "%s" DIR_STR "%+03d%+04d" DIR_STR "%+03d%+04d.hgt.zip", args[0], latlon_bucket(t->mSouth), latlon_bucket(t->mWest), (int) t->mSouth, (int) t->mWest);
		printf("Writing %s...\n", path);
		if (!WriteRawHGT(*t, path))
		{
			printf("Error writing %s\n", path);
			return 1;
		}		
	}
	return 0;
}

// Ben says: this routine was based on two assumptions, both of which may not be true:
// 1. that the CGIAR SRTM tiles are 6000x6000 GeoTiffs, incorrectly offset by half a pixel, missing
// one row and
// 2. that the GeoTiff importer will pad up one row.
// Thus this code tries to load the rows out of the neighboring tiles.  This should probably not be used
// because the GeoTiff importer has changed!!
// -bulksrtm <src dir> <dst dir> <x> <y>
static int DoBulkConvertSRTM(const vector<const  char *>& args)
{
	DEMGeo	me, north, east, northeast;
	char	path[512], path2[512];

	int x = atoi(args[2]);
	int y = atoi(args[3]);
	int n;
	int mode = dem_want_Post;
	sprintf(path, "%s" DIR_STR "srtm_%02d_%02d.zip", args[0], x, y);
	if (!ExtractGeoTiff(me, path,mode))
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
	if (ExtractGeoTiff(east, path2, mode))
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
	if (ExtractGeoTiff(north, path2, mode))
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
	if (ExtractGeoTiff(northeast, path2, mode))
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
{ "-hgt", 			1, 1, DoHGTImport, 			"Import 16-bit BE raw HGT DEM.", "" },
{ "-hgtzip", 		1, 1, DoHGTExport, 			"Export 16-bit BE raw HGT DEM.", "" },
{ "-hgt_tiles",		1, 1, DoHGTTileExport,		"Export 16-bit BE raw HGT DEMs in tiles.", DoHGTTileExport_HELP },
{ "-oz",			1, 1, DoShortOzImport,		"Read short DEM.", "" },
{ "-floatdem", 		1, 1, DoFloatDEMImport, 	"Import floating-point DEM", "" },
{ "-usgs_natural", 	1, 1, DoUSGSNaturalImport, 	"Import USGS Natural-format DEM", "" },
{ "-ida", 			1, 1, DoIDAImport, 			"Import IDA-format raster file.", "" },
//{ "-geotiff", 		1, 1, DoGeoTiffImport, 		"Import GeoTiff DEM", "" },
{ "-glcc", 			2, 2, DoGLCCImport, 		"Import GLCC land use raster data.", "" },
{ "-bulksrtm",		4, 4, DoBulkConvertSRTM,	"Bulk convert SRTM data.", "" },
{ "-markoverlay",	0, 0, DoRemember,			"Remember the current elevation as overlay.", "" },
{ "-readmask",		1, 1, DoMaskRemember,		"Remember the current elevation as overlay.", "" },
{ "-raster_import",	4, 7, DoRasterImport,		"Import one raster DEM file.", DoRasterImport_HELP },
{ "-raster_export", 4, 4, DoRasterExport,		"Export one rater DME file.", DoRasterExport_HELP }, 
{ "-applyoverlay",	0, 0, DoApply	,			"Use overlay.", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterDemCmds(void)
{
	GISTool_RegisterCommands(sDemCmds);
}

