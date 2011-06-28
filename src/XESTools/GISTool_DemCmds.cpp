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
"USAGE: raster_import <flags> <format> <file> <layer> [<null>] [<fill>] [<translation>]\n"\
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
" i Ignore DEM file if it does not exist.\n"\
" c Check for conflicts in duplicate posts between new and old DEM (only makes sense with o flag.\n"\
" a GeoTiff only - allow DEM to be area data if file contains area data.\n"\
" a bil/hgt only - force area-style DEM.  Otherwise area/point comes from the particular .hdr file.\n"\
"Format can be one of: \n"\
"tiff\n"\
"hgt\n"\
"ascii\n"\
"bil\n"\
"flt\n"\
"Note: for bil import, the current extent is used to position the DEM.\n"\
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
	if((strstr(args[0],"o") || strstr(args[0],"e")) && gDem.count(layer))
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
			if(strstr(args[0],"i")) return 0;
			fprintf(stderr,"Unable to read ARC-ASCII file %s\n", args[2]);
			return 1;
		}
	}
	else if(strcmp(args[1],"tiff") == 0)
	{
		if(!ExtractGeoTiff(*dem, args[2], mode))
		{
			if(strstr(args[0],"i")) return 0;
			fprintf(stderr,"Unable to read GeoTiff file %s\n", args[2]);
			return 1;
		}
	}
	else if(strcmp(args[1],"flt") == 0)
	{
		DEMSpec spec;
		spec.mWest = gMapWest;
		spec.mEast = gMapEast;
		spec.mNorth = gMapNorth;
		spec.mSouth = gMapSouth;
		spec.mPost = 1;
		spec.mBigEndian = true;
		spec.mBits = 32;
		spec.mNoData = DEM_NO_DATA;	// Use OUR no data flag...this means that no data is re-flagged if the header doesn't have a void flag.  User can fix this later with the n flag.
		spec.mFloat = true;
		spec.mHeaderBytes = 0;
		
		ReadHDR(args[2], spec, strstr(args[0],"a"));
		if(!ReadRawWithHeader(*dem, args[2], spec))
		{
			if(strstr(args[0],"i")) return 0;
			fprintf(stderr,"Unable to read flt file %s\n", args[2]);
			return 1;			
		}
	}
	else if(strcmp(args[1],"hgt") == 0)
	{
		if(!ReadRawHGT(*dem, args[2])) 
		{
			if(strstr(args[0],"i")) return 0;
			fprintf(stderr,"Unable to read HGT file %s\n", args[2]);
			return 1;
		}
	}
	else if(strcmp(args[1],"bil") == 0)
	{
		DEMSpec spec;
		spec.mWest = gMapWest;
		spec.mEast = gMapEast;
		spec.mNorth = gMapNorth;
		spec.mSouth = gMapSouth;
		spec.mPost = 1;
		spec.mBigEndian = true;
		spec.mBits = 16;
		spec.mNoData = DEM_NO_DATA; // Use OUR no data flag...this means that no data is re-flagged if the header doesn't have a void flag.  User can fix this later with the n flag.
		spec.mFloat = false;
		spec.mHeaderBytes = 0;
		
		ReadHDR(args[2], spec, strstr(args[0],"a"));
		
		if(!ReadRawWithHeader(*dem, args[2], spec))
		{
			if(strstr(args[0],"i")) return 0;
			fprintf(stderr,"Unable to read bil file %s\n", args[2]);
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
		
		if(dst->x_res() != dem->x_res())
		{
			fprintf(stderr,"Unable to overlay %s onto layer %s: horizontal resolution mismatch.\n", args[2], args[3]);
			return 1;
		}
		if(dst->y_res() != dem->y_res())
		{
			fprintf(stderr,"Unable to overlay %s onto layer %s: vertical resolution mismatch.\n", args[2], args[3]);
			return 1;
		}
		if(dst->mPost != dem->mPost)
		{
			fprintf(stderr,"Unable to overlay %s onto layer %s: both DEMs must be area or point based.\n", args[2], args[3]);
			return 1;
		}
			
		DEMGeo	tmp;
		
		bool in_place = (dst->mWest  <= dem->mWest ) &&
						(dst->mEast  >= dem->mEast ) &&
						(dst->mSouth <= dem->mSouth) &&
						(dst->mNorth >= dem->mNorth);
		bool err_check = strstr(args[0],"c");
			
		
		if (in_place)
		{
			tmp.swap(*dst);
		}
		else
		{		
			tmp.mWest = min(dem->mWest ,dst->mWest );
			tmp.mSouth= min(dem->mSouth,dst->mSouth);
			tmp.mEast = max(dem->mEast ,dst->mEast );
			tmp.mNorth= max(dem->mNorth,dst->mNorth);
			tmp.mPost = dem->mPost;
			
			tmp.set_rez(dem->x_res(),dem->y_res());
			tmp = DEM_NO_DATA;
			
			for(int y = 0; y < dst->mHeight; ++y)
			for(int x = 0; x < dst->mWidth ; ++x)
			{
				int xp = tmp.map_x_from(*dst,x);
				int yp = tmp.map_y_from(*dst,y);
				tmp(xp,yp)=dst->get(x,y);
			}
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
						fprintf(stderr,"Error: DEMs do not match at %d,%d: %f over %f\n", x,y, top_val, bot_val);
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
"USAGE: raster_export <flags> <format> <file> <layer> [<res>]\n"\
"This exports a raster layer to a raster file.\n"\
"Flags:\n"\
" x - no special flags.\n"\
" c - export only a cropping within the extent area.\n"\
" r - resample - res in samples per DEM on end.  Outputs in post format.\n"\
" f - generate filename based on location of export.  Pass a path with trailing /.\n"\
"File Format must be: tiff.\n"
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

	if(strstr(args[0],"c") && strstr(args[0],"r"))
	{
		fprintf(stderr,"You cannot use crop and resample in the same export.\n");
		return 1;
	}

	DEMGeo * src = &gDem[layer];
	DEMGeo	chopped;
	
	if (strstr(args[0],"c"))
	{	
		src->subset(chopped,
						 src->x_lower(gMapWest),
						 src->y_lower(gMapSouth),
						 src->x_upper(gMapEast),
						 src->y_upper(gMapNorth));
		src = &chopped;
	} 
	else if (strstr(args[0],"r"))
	{
		chopped.mNorth = gMapNorth;
		chopped.mSouth = gMapSouth;
		chopped.mWest = gMapWest;
		chopped.mEast = gMapEast;
		chopped.mPost = 1;
		int rz = atoi(args[4]);
		chopped.set_rez(rz,rz);
		ResampleDEM(*src, chopped);
		src = &chopped;
	}
	
	string fname(args[2]);
	string suffix;
	if(strcmp(args[1],"tiff") == 0)	suffix = "tif";
	if(strstr(args[0],"f"))
	{
		if(suffix.empty())
		{
			fprintf(stderr,"Cannot use 'f' option - the file type %s is unknown.\n", args[1]);
			return 1;
		}
		int s = round(src->mSouth);
		int w = round(src->mWest);
		char path[50];
		sprintf(path,"%+03d%+04d",latlon_bucket(s),latlon_bucket(w));
		fname += path;
		FILE_make_dir(fname.c_str());
		sprintf(path,"/%+03d%+04d.%s",s,w,suffix.c_str());
		fname += path;
		if(gVerbose)	printf("Will save %s.\n", fname.c_str());
	}

	if(strcmp(args[1],"tiff") == 0)
	{	
		if (!WriteGeoTiff(*src, fname.c_str()))
		{
			fprintf(stderr,"Error writing file: %s\n", fname.c_str());
			return 1;
		} else
			if (gVerbose)	printf("Wrote %s\n",fname.c_str());
	}
	else
	{
		fprintf(stderr,"Unknown export file format: %s\n", args[1]);
		return 1;
	}
	return 0;
}

#define DoRasterInit_HELP \
"USAGE: -raster_init x_res y_res post layer [init value]\n"\
"This initializes a raster layer to a given resolution.  The bounds\n"\
"of the raster layer is set by the global extent.  If post is 1 then\n"\
"the size of the raster layer will be one pixel larger than the resolutions\n"\
"provided.\n"
int DoRasterInit(const vector<const char *>& args)
{
	int layer = LookupToken(args[3]);
	if (layer == -1)
	{
		fprintf(stderr,"Layer %s unknown.\n",args[3]);
		return 1;
	}	

	DEMGeo& dem = gDem[layer];
	dem.mWest = gMapWest;
	dem.mEast = gMapEast;
	dem.mNorth = gMapNorth;
	dem.mSouth = gMapSouth;
	dem.mPost = atoi(args[2]);
	dem.set_rez(atoi(args[0]),atoi(args[1]));
	if (args.size() > 4)
		dem = atof(args[4]);
	return 0;
}

#define DoRasterResample_HELP \
"USAGE: -raster_resample x_res y_res post layer\n"\
"This resamples a raster layer.  The resampling occurs over\n"\
"the current global extent (which becomes the new DEM bounds.\n"\
"DEM_NO_DATA is set for samples inside the resampling region\n"\
"but outside the old layer data.\n"
int DoRasterResample(const vector<const char *>& args)
{
	int layer = LookupToken(args[3]);
	if (layer == -1)
	{
		fprintf(stderr,"Layer %s unknown.\n",args[3]);
		return 1;
	}	

	if (gDem.count(layer) == 0)
	{
		fprintf(stderr,"Layer %s not initialized.\n",args[3]);
		return 1;
	}

	DEMGeo& src = gDem[layer];
	DEMGeo	dem;
	dem.mWest = gMapWest;
	dem.mEast = gMapEast;
	dem.mNorth = gMapNorth;
	dem.mSouth = gMapSouth;
	dem.mPost = atoi(args[2]);
	dem.set_rez(atoi(args[0]),atoi(args[1]));

	ResampleDEM(src, dem);
	swap(src,dem);

	#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_FileLoaded, NULL);
	#endif

	return 0;
}

#define DoRasterAdjust_HELP \
"USAGE:  -raster_adjust base_layer overlay_layer temp_layer radius\n"\
"This adjusts the heights of the base layer to more closely match the\n"\
"overlay layer (for areas where the overlay layer is non-void).  The\n"\
"radius parameter is the radius (in pixels) for a gaussian blur that is\n"\
"used to ensure that adjustments are low frequency - thus a larger\n"\
"radius will mean a lower frequency adjustment to the base layer.\n"\
"The temporary layer will end up containing the adjustment mask.\n"
int DoRasterAdjust(const vector<const char *>& args)
{
	int layer1 = LookupToken(args[0]);
	int layer2 = LookupToken(args[1]);
	int layer3 = LookupToken(args[2]);
	if (layer1 == -1 || layer2 == -1 || layer3 == -1) return 1;
	
	if (gDem.count(layer1) == 0) return 1;
	if (gDem.count(layer2) == 0) return 1;

	DifferenceDEM(gDem[layer1],gDem[layer2],gDem[layer3]);
	
	if(atof(args[3]) > 0.0)
		GaussianBlurDEM(gDem[layer3], atof(args[3]));
	
	gDem[layer1] += gDem[layer3];
	
	//DifferenceDEM(gDem[layer1],gDem[layer2],gDem[layer3]);
	
	#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
	#endif
	
	return 0;

}

#define DoRasterMerge_HELP \
"USAGE: -raster_merge base overlay temp radius\n"\
"This merges the overlay layer into the base layer; roughly the\n"\
"original bottom layer is visible only where the top layer is void.\n"\
"Radius is the number of pixels of feathering to use to soften the\n"\
"transition between the bottom and top layers when there is a void in\n"\
"the top layer.  Larger numbers provide smoother blending.\n"\
"The temporary layer will contain the blending mask when the operation\n"\
"completes.\n"
int DoRasterMerge(const vector<const char *>& args)
{
	int fs = 2 * atoi(args[3]) + 1;
	vector<float> fd(fs*fs);
	float * k = &*fd.begin();
	
	int layer1 = LookupToken(args[0]);
	int layer2 = LookupToken(args[1]);
	int layer3 = LookupToken(args[2]);
	if (layer1 == -1 || layer2 == -1 || layer3 == -1) return 1;
	
	if (gDem.count(layer1) == 0) return 1;
	if (gDem.count(layer2) == 0) return 1;

	DEMGeo& base(gDem[layer1]);
	DEMGeo& over(gDem[layer2]);
	DEMGeo& mask(gDem[layer3]);

	mask = over;

	int	vc = BinaryDEMFromEnum(mask, DEM_NO_DATA, 1.0, 0.0);
	if(vc == 0)
	{
		base = over;
	}
	else
	{
		DEMGeo	weighted(mask.mWidth, mask.mHeight);
		weighted.copy_geo_from(mask);
		weighted.mPost = mask.mPost;
		CalculateFilter(fs, k, demFilter_Linear, false);
		for(int y = 0; y < mask.mHeight; ++y)
		for(int x = 0; x < mask.mWidth; ++x)
			weighted(x,y) = mask.kernelmaxN(x,y,fs,k);
		mask.swap(weighted);
		
		// Now we merge -- zero is top, 1 is bottom
		for(int y = 0; y < mask.mHeight; ++y)
		for(int x = 0; x < mask.mWidth; ++x)
		{
			float w = mask.get(x,y);
			if(w < 0.0f) w = 0.0f;
			if(w > 1.0f) w = 1.0f;
			float t = over.get(x,y);
			float b = base.get(x,y);
			DebugAssert(t != DEM_NO_DATA || w == 1.0);
			DebugAssert(b != DEM_NO_DATA || w == 0.0);
			if(t != DEM_NO_DATA)
				base(x,y) = b * w + t * (1.0f - w);
		}
	}
	return 0;
}

#define DoRasterWatershed_HELP \
"Usage: -raster_watershed <layer> <radius> <mmu_size>"
static int DoRasterWatershed(const vector<const char *>& args)
{
	int layer1 = LookupToken(args[0]);
	if(layer1 == -1) return 1;
	if(gDem.count(layer1) == 0) return 1;
	
	int radius = atoi(args[1]);
	int mmu_size = atoi(args[2]);
	
	
	DEMGeo& dem(gDem[layer1]);
	DEMGeo	lhi;
	DEMGeo	ws_dem;
	vector<DEMGeo::address> ws;
	
	NeighborHisto(dem, lhi, radius);	
	Watershed(lhi, ws_dem,&ws);
	#if DEV
	VerifySheds(ws_dem,ws);
	#endif
	
	MergeMMU(ws_dem,ws,mmu_size);

	SetWatershedsToDominant(dem,ws_dem, ws);

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

	#if OPENGL_MAP
	if(ok)
		RF_Notifiable::Notify(rf_Cat_File, rf_Msg_FileLoaded, NULL);
	#endif
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
{ "-raster_export", 4, 5, DoRasterExport,		"Export one raster DEM file.", DoRasterExport_HELP }, 
{ "-raster_init",	4, 5, DoRasterInit,			"Create new empty raster layer.", DoRasterInit_HELP }, 
{ "-raster_resample",4, 4, DoRasterResample,	"Resample raster layer.", DoRasterResample_HELP }, 
{ "-raster_adjust", 4, 4, DoRasterAdjust,		"Adjust levels of raster layers to match.", DoRasterAdjust_HELP },
{ "-raster_merge", 4, 4, DoRasterMerge,			"Merge two raster layers.", DoRasterMerge_HELP },
{ "-raster_watershed", 3, 3, DoRasterWatershed,	"Calculate watersheds from one layer, dump in another", DoRasterWatershed_HELP },
{ "-applyoverlay",	0, 0, DoApply	,			"Use overlay.", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterDemCmds(void)
{
	GISTool_RegisterCommands(sDemCmds);
}

