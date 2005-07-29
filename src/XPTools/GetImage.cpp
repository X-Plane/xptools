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
#include "XGrinderApp.h"
#include "XUtils.h"
#include "BitmapUtils.h"
#include "TerraServer.h"
#include "PCSBSocket.h"
#include "EnvParser.h"
#include "EnvWrite.h"
#include "Persistence.h"
#include "PCSBSocketUDP.h"
#include <xtiffio.h>
#include <geotiff.h>
#include <geotiffio.h>
#include <geo_normalize.h>


int good = 0, fail = 0;

const char * kThemeNames[] = {
	"Relief",
	"Photo",
	"Topo",
	0
};

inline	int	VERTEX_NUM(int x, int y) { return x + y * 151; }

inline double	Interp2(double frac, double sml, double big)
{
	return sml + frac * (big - sml);
}

xmenu	gSettingsM = NULL;
const char *	kSettingsItems[] = {
	"Save BMP Files",
	"Save PNG Files",
	0
};

enum {
	save_BMP = 0,
	save_PNG = 1
};


ImageInfo * gRefImage = NULL;
double		gRefNW_Lat = 0.0;
double		gRefNE_Lat = 0.0;
double		gRefSW_Lat = 0.0;
double		gRefSE_Lat = 0.0;

double		gRefNW_Lon = 0.0;
double		gRefNE_Lon = 0.0;
double		gRefSW_Lon = 0.0;
double		gRefSE_Lon = 0.0;

int	gSaveMode = save_BMP;
//std::string	gTheme("Photo");
std::string	gTheme("1");

static	bool	TransformTiffCorner(struct GTIF * gtif, GTIFDefn * defn, double x, double y, double& outLon, double& outLat)
{
    /* Try to transform the coordinate into PCS space */
    if( !GTIFImageToPCS( gtif, &x, &y ) )
        return false;
    
    if( defn->Model == ModelTypeGeographic )
    {
    	outLon = x;
    	outLat = y;
    	return true;
    }
    else
    {
        if( GTIFProj4ToLatLong( defn, 1, &x, &y ) )
        {
			outLon = x;
			outLat = y;
			return true;
		}
	}	
	return false;    
}

// SW_lon SW_lat	SE_lon SE_lat	NW_lon NW_lat	NE_lon NE_lat	
static	bool	FetchTIFFCorners(const char * inFileName, double corners[8])
{
	bool retVal = false;
	TIFF * tiffFile = XTIFFOpen(inFileName, "r");
	if (tiffFile)
	{
		GTIF * gtif = GTIFNew(tiffFile);
		if (gtif)
		{
			GTIFDefn 	defn;
	        if( GTIFGetDefn( gtif, &defn ) )
	        {
	        	int xsize, ysize;
	            TIFFGetField( tiffFile, TIFFTAG_IMAGEWIDTH, &xsize );
	            TIFFGetField( tiffFile, TIFFTAG_IMAGELENGTH, &ysize );

	        	if (TransformTiffCorner(gtif, &defn, 0,     ysize, corners[0], corners[1]) &&
		        	TransformTiffCorner(gtif, &defn, xsize, ysize, corners[2], corners[3]) &&
		        	TransformTiffCorner(gtif, &defn, 0,     0,     corners[4], corners[5]) &&
		        	TransformTiffCorner(gtif, &defn, xsize, 9,     corners[6], corners[7]))
		        {
		        	retVal = true;
		        }
			}
			GTIFFree(gtif);
		}    
		
		XTIFFClose(tiffFile);
	}
	return retVal;
}


void	ConformCheckItems(void)
{
	XWin::CheckMenuItem(gSettingsM, save_BMP, 	gSaveMode == save_BMP);
	XWin::CheckMenuItem(gSettingsM, save_PNG, 	gSaveMode == save_PNG);
}

int		MyCreateBitmapFromFile(const char * inFilePath, struct ImageInfo * outImageInfo)
{
	string	pngPath = string(inFilePath) + ".png";
	string	bmpPath = string(inFilePath) + ".bmp";
	string	tifPath = string(inFilePath) + ".tif";
	if (CreateBitmapFromPNG(pngPath.c_str(), outImageInfo))
		if (CreateBitmapFromFile(bmpPath.c_str(), outImageInfo))
			return (CreateBitmapFromTIF(tifPath.c_str(), outImageInfo));
	return 0;
}

int	MyWriteBitmapToFile	(const struct ImageInfo * inImage, const char * inFilePath)
{
	string	bmpPath = string(inFilePath) + ".bmp";
	string	pngPath = string(inFilePath) + ".png";
	if (gSaveMode == save_BMP)
		return WriteBitmapToFile(inImage, bmpPath.c_str());
	else
		return WriteBitmapToPNG(inImage, pngPath.c_str());
}

#if 0

void	DrawPts(double pts[4][2], double y_min, double y_max, double x_min, double x_max)
{
	for (int n = 0; n <= 5; ++n)
	{
		int index = n % 4;
		int h = (pts[index][0] - x_min) / (x_max - x_min) * 400.0 + 20;
		int v = (pts[index][1] - y_min) / (y_max - y_min) * 400.0 + 20;
		if (n == 0)
			MoveTo(h,v);
		else
			LineTo(h, v);
	}
}

void HackShowRects(double old_bounds[4][2], double old_sub[4][2],	
					double new_bounds[4][2], double new_sub[4][2])
{
	Rect	r = { 50, 10, 550, 1010 };
	WindowRef	w;
	CreateNewWindow(kDocumentWindowClass, 0, &r, &w);
	ShowWindow(w);
	
	SetPortWindowPort(w);
	DrawPts(old_bounds, -71.05, -71.0, 42.3, 42.4);
	DrawPts(old_sub, -71.05, -71.0, 42.3, 42.4);
	SetOrigin(-500,0);
	DrawPts(new_bounds, -20, 1000, -20, 1000);
	DrawPts(new_sub, -20, 1000, -20, 1000);
	QDFlushPortBuffer(GetWindowPort(w), NULL);
	while (!Button()) { }
	while (Button()) { }
	DisposeWindow(w);
}
#endif

void	MapQuads(	double old_bounds[4][2], double old_sub[4][2],	
					double new_bounds[4][2], double new_sub[4][2])
{
	double A = old_bounds[1][0] - old_bounds[0][0];
	double B = old_bounds[3][0] - old_bounds[0][0];
	double C = old_bounds[1][1] - old_bounds[0][1];
	double D = old_bounds[3][1] - old_bounds[0][1];
	
	for (int n = 0; n < 4; ++n)
	{
		double i = (D * (old_sub[n][0] - old_bounds[0][0]) - B * (old_sub[n][1] - old_bounds[0][1])) / (A * D - B * C);
		double j = (C * (old_sub[n][0] - old_bounds[0][0]) - A * (old_sub[n][1] - old_bounds[0][1])) / (B * C - D * A);
		
		new_sub[n][0] = Interp2(j, 
							Interp2(i, new_bounds[0][0],new_bounds[1][0]), 
							Interp2(i, new_bounds[3][0],new_bounds[2][0]));
		new_sub[n][1] = Interp2(i,
							Interp2(j, new_bounds[0][1],new_bounds[3][1]),
							Interp2(j, new_bounds[1][1],new_bounds[2][1]));
	}

}					

void	Squarify(int x1, int y1, int x2, int y2)
{
	for (int i = x1; i <= x2; ++i)
	for (int j = y1; j <= y2; ++j)
	{
		gVertices[VERTEX_NUM(i,j)].longitude = 
			Interp2((double) i / 150.0, 
			gVertices[VERTEX_NUM(0,0)].longitude, 
			gVertices[VERTEX_NUM(150, 200)].longitude);
		gVertices[VERTEX_NUM(i,j)].latitude = 
			Interp2((double) j / 200.0, 
			gVertices[VERTEX_NUM(0,0)].latitude, 
			gVertices[VERTEX_NUM(150, 200)].latitude);
	}
}

void	TakeFromRefImage(
				int	x, int y,			// X and Y of quad, 0-149, 0-199
				int stretch,			// Number of quads to stretch over, 1 = 1x1, 2 = 2x2, etc.
				const char * texName,	// Name the new tex this...
				int texSize)			// Final size of texture, must be a power of 2 between [4-1024]
{
	string	shortName = texName;
	StripPath(shortName);
	XGrinder_ShowMessage("Cutting %s...", shortName.c_str());

	if (gRefImage != NULL)
	{	
			double	bounds[4][2];	// Location of our entire ref bitmap
			double	env[4][2];		// Location of the subsection we want to resample
			double 	src[4][2];		// Location of the whole bitmap as pixels
			double	pixels[4][2];	// Location of the subsection as pixels
		
		bounds[0][0] = gRefNW_Lat;
		bounds[0][1] = gRefNW_Lon;
		bounds[1][0] = gRefNE_Lat;
		bounds[1][1] = gRefNE_Lon;
		bounds[2][0] = gRefSE_Lat;
		bounds[2][1] = gRefSE_Lon;
		bounds[3][0] = gRefSW_Lat;
		bounds[3][1] = gRefSW_Lon;

		env[0][0] = gVertices[VERTEX_NUM(x, y + stretch)].latitude;
		env[0][1] = gVertices[VERTEX_NUM(x, y + stretch)].longitude;
		env[1][0] = gVertices[VERTEX_NUM(x + stretch, y + stretch)].latitude;
		env[1][1] = gVertices[VERTEX_NUM(x + stretch, y + stretch)].longitude;
		env[2][0] = gVertices[VERTEX_NUM(x + stretch, y)].latitude;
		env[2][1] = gVertices[VERTEX_NUM(x + stretch, y)].longitude;
		env[3][0] = gVertices[VERTEX_NUM(x, y)].latitude;
		env[3][1] = gVertices[VERTEX_NUM(x, y)].longitude;
		
		src[0][1] = 0.0;			src[0][0] = gRefImage->height;
		src[1][1] = gRefImage->width;	src[1][0] = gRefImage->height;
		src[2][1] = gRefImage->width;	src[2][0] = 0;
		src[3][1] = 0.0;			src[3][0] = 0;
			
		MapQuads(bounds, env, src, pixels);

		ImageInfo	final;
		CreateNewBitmap(texSize, texSize, gRefImage->channels, &final);
		
		CopyBitmapSectionWarped(gRefImage, &final,
			pixels[3][1], pixels[3][0],
			pixels[2][1], pixels[2][0],
			pixels[1][1], pixels[1][0],
			pixels[0][1], pixels[0][0],
			0, 0, texSize, texSize);
		
		string	fname = string(texName);
		if (gSaveMode == save_BMP && final.channels == 4)
			ConvertAlphaToBitmap(&final);
		MyWriteBitmapToFile(&final, fname.c_str());
//		fname = string(texName) + "non_distort.bmp";
//		WriteBitmapToFile(&image, fname.c_str());
		DestroyBitmap(&final);
	
		string	shortTexName = texName;
		StripPath(shortTexName);
		gTextures.push_back(shortTexName);
		int texIndex = gTextures.size() - 1;
		for (int i = x; i < (x + stretch); ++i)
		for (int j = y; j < (y + stretch); ++j)
		{
			gVertices[VERTEX_NUM(i,j)].custom = 1;
			gVertices[VERTEX_NUM(i,j)].landUse = texIndex;
			gVertices[VERTEX_NUM(i,j)].scale = stretch - 1;
			gVertices[VERTEX_NUM(i,j)].xOff = i - x;
			gVertices[VERTEX_NUM(i,j)].yOff = j - y;
			gVertices[VERTEX_NUM(i,j)].rotation = 0;
			
			gVertices[VERTEX_NUM(i,j)].latitude = 
				Interp2((double)(i-x) / (double) stretch,
					Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x,y)].latitude, gVertices[VERTEX_NUM(x,y+stretch)].latitude),
					Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x+stretch,y)].latitude, gVertices[VERTEX_NUM(x+stretch,y+stretch)].latitude));
			gVertices[VERTEX_NUM(i,j)].longitude = 
				Interp2((double)(i-x) / (double) stretch,
					Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x,y)].longitude, gVertices[VERTEX_NUM(x,y+stretch)].longitude),
					Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x+stretch,y)].longitude, gVertices[VERTEX_NUM(x+stretch,y+stretch)].longitude));
		} 		
		
	}	
}				




void	GetOrthoPhotos(
				const char * scale, 				// meters per pixel, usually 1 or 4
				int	x, int y,			// X and Y of quad, 0-149, 0-199
				int stretch,			// Number of quads to stretch over, 1 = 1x1, 2 = 2x2, etc.
				const char * texName,	// Name the new tex this...
				int texSize)			// Final size of texture, must be a power of 2 between [4-1024]
{
	if (!strcmp(scale, "1")) scale = "1m";
	if (!strcmp(scale, "4")) scale = "4m";
	string	shortName = texName;
	StripPath(shortName);
	XGrinder_ShowMessage("Fetching %s...", shortName.c_str());
	// We have to figure out the lat bounds that we are applying over.
	double	lonwest = 180.0, loneast = -180.0, latsouth = 90.0, latnorth = -90.0;
	for (int i = x; i <= (x + stretch); ++i)
	for (int j = y; j <= (y + stretch); ++j)
	{
		lonwest = min(lonwest,gVertices[VERTEX_NUM(i,j)].longitude);
		loneast = max(loneast,gVertices[VERTEX_NUM(i,j)].longitude);
		latsouth = min(latsouth,gVertices[VERTEX_NUM(i,j)].latitude);
		latnorth = max(latnorth,gVertices[VERTEX_NUM(i,j)].latitude);
	} 

	int tiles[4][3];
	
	if (GetTilesForArea(scale, gTheme.c_str(), latsouth, lonwest, latnorth, loneast, tiles) == 0)
	{	

		int domain = tiles[0][2];
		int	max_x = max(tiles[1][0], tiles[2][0]);
		int min_x = min(tiles[0][0], tiles[3][0]);
		int max_y = max(tiles[0][1], tiles[1][1]);
		int min_y = min(tiles[2][1], tiles[3][1]);
		
		int	x_size = (max_x - min_x + 1) * 200;
		int y_size = (max_y - min_y + 1) * 200;
		
		ImageInfo	image;
		CreateNewBitmap(x_size, y_size, 3, &image);

			double	coords[4][2];	// Location of any one tile
			double	bounds[4][2];	// Location of our entire bitmap
			double	env[4][2];		// Location of the subsection we want to resample
			double 	src[4][2];		// Location of the whole bitmap as pixels
			double	pixels[4][2];	// Location of the subsection as pixels
		
		bool	exists = true;
		if (FetchTilePositioning(scale, gTheme.c_str(), domain, min_x, max_y, coords) != 0) exists = false;
		bounds[0][0] = coords[0][0];	bounds[0][1] = coords[0][1];
		if (FetchTilePositioning(scale, gTheme.c_str(), domain, max_x, max_y, coords) != 0) exists = false;
		bounds[1][0] = coords[1][0];	bounds[1][1] = coords[1][1];
		if (FetchTilePositioning(scale, gTheme.c_str(), domain, max_x, min_y, coords) != 0) exists = false;
		bounds[2][0] = coords[2][0];	bounds[2][1] = coords[2][1];
		if (FetchTilePositioning(scale, gTheme.c_str(), domain, min_x, min_y, coords) != 0) exists = false;
		bounds[3][0] = coords[3][0];	bounds[3][1] = coords[3][1];
		
		env[0][0] = gVertices[VERTEX_NUM(x, y + stretch)].latitude;
		env[0][1] = gVertices[VERTEX_NUM(x, y + stretch)].longitude;
		env[1][0] = gVertices[VERTEX_NUM(x + stretch, y + stretch)].latitude;
		env[1][1] = gVertices[VERTEX_NUM(x + stretch, y + stretch)].longitude;
		env[2][0] = gVertices[VERTEX_NUM(x + stretch, y)].latitude;
		env[2][1] = gVertices[VERTEX_NUM(x + stretch, y)].longitude;
		env[3][0] = gVertices[VERTEX_NUM(x, y)].latitude;
		env[3][1] = gVertices[VERTEX_NUM(x, y)].longitude;
		
		src[0][1] = 0.0;			src[0][0] = image.height;
		src[1][1] = image.width;	src[1][0] = image.height;
		src[2][1] = image.width;	src[2][0] = 0;
		src[3][1] = 0.0;			src[3][0] = 0;

		if (exists)
		{
		
			for (int x = min_x; x <= max_x; ++x)
			for (int y = min_y; y <= max_y; ++y)
			{
				for (int n = 0; n < 4; ++n)
				{
					if (FetchTile(scale, gTheme.c_str(), domain, x, y, &image, (x - min_x) * 200, (y - min_y) * 200)==0)
						break;
					fail++;
				}
				good++;
			}
			
			MapQuads(bounds, env, src, pixels);

	//		HackShowRects(bounds, env, src, pixels);
			
			ImageInfo	final;
			CreateNewBitmap(texSize, texSize, 3, &final);
			
			CopyBitmapSectionWarped(&image, &final,
				pixels[3][1], pixels[3][0],
				pixels[2][1], pixels[2][0],
				pixels[1][1], pixels[1][0],
				pixels[0][1], pixels[0][0],
				0, 0, texSize, texSize);
			
			string	fname = string(texName);
			MyWriteBitmapToFile(&final, fname.c_str());
	//		fname = string(texName) + "non_distort.bmp";
	//		WriteBitmapToFile(&image, fname.c_str());
			DestroyBitmap(&final);
		
			string	shortTexName = texName;
			StripPath(shortTexName);
			gTextures.push_back(shortTexName);
			int texIndex = gTextures.size() - 1;
			for (int i = x; i < (x + stretch); ++i)
			for (int j = y; j < (y + stretch); ++j)
			{
				gVertices[VERTEX_NUM(i,j)].custom = 1;
				gVertices[VERTEX_NUM(i,j)].landUse = texIndex;
				gVertices[VERTEX_NUM(i,j)].scale = stretch - 1;
				gVertices[VERTEX_NUM(i,j)].xOff = i - x;
				gVertices[VERTEX_NUM(i,j)].yOff = j - y;
				gVertices[VERTEX_NUM(i,j)].rotation = 0;
				
				gVertices[VERTEX_NUM(i,j)].latitude = 
					Interp2((double)(i-x) / (double) stretch,
						Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x,y)].latitude, gVertices[VERTEX_NUM(x,y+stretch)].latitude),
						Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x+stretch,y)].latitude, gVertices[VERTEX_NUM(x+stretch,y+stretch)].latitude));
				gVertices[VERTEX_NUM(i,j)].longitude = 
					Interp2((double)(i-x) / (double) stretch,
						Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x,y)].longitude, gVertices[VERTEX_NUM(x,y+stretch)].longitude),
						Interp2((double)(j-y) / (double) stretch, gVertices[VERTEX_NUM(x+stretch,y)].longitude, gVertices[VERTEX_NUM(x+stretch,y+stretch)].longitude));
			} 		
		
		}
		
		DestroyBitmap(&image);
		
	}	
}				


void	DownloadRange(const char * scale, double inLatSouth, double inLonWest, double inLatNorth, double inLonEast, const char * inFileName, float inLimit)
{
	if (!strcmp(scale, "1")) scale = "1m";
	if (!strcmp(scale, "4")) scale = "4m";

	int tiles[4][3];
	
	if (GetTilesForArea(scale, gTheme.c_str(), inLatSouth, inLonWest, inLatNorth, inLonEast, tiles) == 0)
	{	

		int domain = tiles[0][2];
		int	max_x = max(tiles[1][0], tiles[2][0]);
		int min_x = min(tiles[0][0], tiles[3][0]);
		int max_y = max(tiles[0][1], tiles[1][1]);
		int min_y = min(tiles[2][1], tiles[3][1]);
		
		int	x_size = (max_x - min_x + 1) * 200;
		int y_size = (max_y - min_y + 1) * 200;
		
		ImageInfo	image;
		CreateNewBitmap(x_size, y_size, 3, &image);
		
		for (int x = min_x; x <= max_x; ++x)
		for (int y = min_y; y <= max_y; ++y)
		{
			for (int n = 0; n < 4; ++n)
			{
				if (FetchTile(scale, gTheme.c_str(), domain, x, y, &image, (x - min_x) * 200, (y - min_y) * 200) == 0)	
					break;
				++fail;
			}
			++good;
		}
		
		string	fname = string(inFileName);
		
		// If our bitmap is greater than 1024 x 1024 we have to rescale it!
		if ((image.width > inLimit || image.height > inLimit) && inLimit > 0)
		{
			double	xscale = (inLimit / (double) image.width);
			double	yscale = (inLimit / (double) image.height);
			
			double	scale = (xscale > yscale) ? yscale : xscale;
			
			int	new_width = (double) image.width * scale;
			int new_height = (double) image.height * scale;
			
			ImageInfo	scaled;
			CreateNewBitmap(new_width, new_height, 3, &scaled);
			
			CopyBitmapSection(&image, &scaled,
				0, 0, image.width, image.height,
				0, 0, scaled.width, scaled.height);
			
			MyWriteBitmapToFile(&scaled, fname.c_str());			
			DestroyBitmap(&scaled);
			
		} else {		
			MyWriteBitmapToFile(&image, fname.c_str());
		}
		DestroyBitmap(&image);
		
		fname = string(inFileName) + ".geo";
		FILE * fi = fopen(fname.c_str(), "w");
		if (fi)
		{
			double	coords[4][2];

			FetchTilePositioning(scale, gTheme.c_str(), domain, min_x, max_y, coords);
			fprintf(fi,"Northwest: %f,%f\n", coords[0][0], coords[0][1]);

			FetchTilePositioning(scale, gTheme.c_str(), domain, max_x, max_y, coords);
			fprintf(fi,"Northeast: %f,%f\n", coords[1][0], coords[1][1]);

			FetchTilePositioning(scale, gTheme.c_str(), domain, max_x, min_y, coords);
			fprintf(fi,"Southeast: %f,%f\n", coords[2][0], coords[2][1]);

			FetchTilePositioning(scale, gTheme.c_str(), domain, min_x, min_y, coords);
			fprintf(fi,"Southwest: %f,%f\n", coords[3][0], coords[3][1]);

			fclose(fi);
		}

	}	
}

void	MergePhotos(int x1, int y1, int x2, int y2, int res_per_quad, const char * inFileName, const char * inPath, bool isMerge)
{
	int x_res = (x2 - x1) * res_per_quad;
	int y_res = (y2 - y1) * res_per_quad;
	
	ImageInfo	big_image;
	
	int	err = 0;
	if (isMerge)
		err = CreateNewBitmap(x_res, y_res, 3, &big_image);
	else
		err = MyCreateBitmapFromFile(inFileName, &big_image);

	
	if (err == 0)
	{
		for (int x = x1; x < x2; ++x)
		for (int y = y1; y < y2; ++y)
		{
			int v = VERTEX_NUM(x, y);
			if (gVertices[v].custom)
			{
				string	fname = string(inPath) + gTextures[gVertices[v].landUse];
				ImageInfo	tex;

				int	bigLeft = (x - x1) * res_per_quad;
				int bigBottom = (y - y1) * res_per_quad;
				int bigRight = bigLeft + res_per_quad;
				int bigTop = bigBottom + res_per_quad;
				
				
				if (isMerge)
					err = MyCreateBitmapFromFile(fname.c_str(), &tex);
				else {
					err = MyCreateBitmapFromFile(fname.c_str(), &tex);
					if (err != 0)
						err = CreateNewBitmap(res_per_quad * (gVertices[v].scale + 1), res_per_quad * (gVertices[v].scale + 1), 3, &tex);
				}
				if (err == 0)
				{
					int	smlLeft = (double) tex.width * (double) gVertices[v].xOff / (double) (gVertices[v].scale + 1);
					int smlRight = (double) tex.width * (double) (gVertices[v].xOff + 1) / (double) (gVertices[v].scale + 1);
					int smlBottom = (double) tex.height * (double) gVertices[v].yOff / (double) (gVertices[v].scale + 1);
					int smlTop = (double) tex.height * (double) (gVertices[v].yOff + 1) / (double) (gVertices[v].scale + 1);

					if (isMerge)
						CopyBitmapSection(&tex, &big_image,
							smlLeft, smlBottom, smlRight, smlTop,
							bigLeft, bigBottom, bigRight, bigTop);
					else
						CopyBitmapSection(&big_image, &tex,
							bigLeft, bigBottom, bigRight, bigTop,
							smlLeft, smlBottom, smlRight, smlTop);
					
					if (!isMerge)
						MyWriteBitmapToFile(&tex, fname.c_str());
					DestroyBitmap(&tex);
				}
			}
		}
	
		if (isMerge)
			MyWriteBitmapToFile(&big_image, inFileName);
		DestroyBitmap(&big_image);
	}
}

void	XGrindInit(string& outName)
{
	gSettingsM = XGrinder_AddMenu("Settings", kSettingsItems);
	ConformCheckItems();

	XGrinder_ShowMessage("GetImage");
	outName = "Get Image";
	
	PCSBSocket::StartupNetworking(true);
	
}

int	XGrinderMenuPick(xmenu menu, int item)
{
	if (menu == gSettingsM)
	{
		switch(item) {
		case save_BMP:
		case save_PNG:
			gSaveMode = item;
			break;
		}
	}
	ConformCheckItems();
	return 0;
}

void	XGrindFiles(const vector<string>& inFiles)
{	
	string	envName;
	for (vector<string>::const_iterator c = inFiles.begin(); c != inFiles.end(); ++c)
	{	
		if (HasExtNoCase(*c, ".env"))
		{
			envName = *c;
		}
	}

/*
	FILE * infof = fopen("ThemeInfo", "w");
	int theme = 0;
	string	info[9];
	while (kThemeNames[theme])
	{
		if (GetThemeInfo(kThemeNames[theme], info) == 0)
		{
			for (int n = 0; n < 9; ++n)
				fprintf(infof, "%s\t",info[n].c_str());
			fprintf(infof, "\n");		
			fflush(infof);
		} else 
			fprintf(infof, "Theme %s not available.\n", kThemeNames[theme]);
		++theme;
	}
	fclose(infof);
*/

	for (vector<string>::const_iterator c = inFiles.begin(); c != inFiles.end(); ++c)
	{
		if (HasExtNoCase(*c, ".txt"))
		{
			StTextFileScanner	scanner(c->c_str(), true);

			string	foo = *c;
			StripPath(foo);
			string	path = c->substr(0,c->length() - foo.length());;
			

			string line;		
			while (GetNextNoComments(scanner, line))
			{	
				vector<string>	b;
				if (!line.empty() && line[0] != '#')
					BreakString(line, b);

				// Commands allowed:
				// FETCH name south west north east res [limit]
				// SQUARE x1 y1 x2 y2 (inclusive vertex numbers!)
				// APPLY res x y stretch filename pixmapsize

				if (!b.empty())
				{
					// FETCH filename  south west north east scale
					if (b[0] == "FETCH")
					{
						if (b.size() < 7)
						{
							XGrinder_ShowMessage("FETCH command requires 6 arguments");
							return;						
						}
						string	to_where = path + b[1];
						XGrinder_ShowMessage("Downloading %s...", b[0].c_str());
						DownloadRange(
								b[6].c_str(),
								atof(b[2].c_str()),
								atof(b[3].c_str()),
								atof(b[4].c_str()),
								atof(b[5].c_str()),
								to_where.c_str(),
								(b.size() > 7) ? atof(b[7].c_str()) : 0.0);
					}
					// THEME theme
					else if (b[0] == "THEME")
					{
						if (b.size() < 2)
						{
							XGrinder_ShowMessage("THEME command requires 1 arguments");
							return;												
						}
						gTheme = b[1];
					}
					// APPLY ortho-ppm-scale x y stretch-factor name final_tex_size
					else if (b[0] == "APPLY")
					{
						if (b.size() < 7)
						{
							XGrinder_ShowMessage("APPLY command requires 6 arguments");
							return;						
						}					
						string	to_where = path + b[5];
						GetOrthoPhotos(
								b[1].c_str(),
								atoi(b[2].c_str()),
								atoi(b[3].c_str()),
								atoi(b[4].c_str()),
								to_where.c_str(),
								atoi(b[6].c_str()));
					}
					// APPLY_IMAGE x y stretch-factor name final_tex_size
					else if (b[0] == "APPLY_IMAGE") 
					{
						if (b.size() < 6)
						{
							XGrinder_ShowMessage("APPLY_IMAGe command requires 5 arguments");
							return;
						}
						string	to_where = path + b[4];
						TakeFromRefImage(
								atoi(b[1].c_str()), 
								atoi(b[2].c_str()), 
								atoi(b[3].c_str()), 
								to_where.c_str(),
								atoi(b[5].c_str()));
					}
					// USE_IMAGE filename sw_lat sw_lon nw_lat nw_lon ne_lat ne_lon se_lat se_lon
					else if (b[0] == "USE_IMAGE")
					{
						if (b.size() != 2 && b.size() != 3 && b.size() < 10)
						{
							XGrinder_ShowMessage("USE_IMAGE requires 1, 2, or 9 arguments");
							return;
						}
						if (gRefImage != NULL)
						{
							DestroyBitmap(gRefImage);
							delete gRefImage;
						}
						gRefImage = new ImageInfo;
						string imgName = path + b[1];
						if (MyCreateBitmapFromFile(imgName.c_str(), gRefImage) != 0)
						{
							delete gRefImage;
							gRefImage = NULL;
						}
						
						double	coords[8];
						string	tifName = path + b[(b.size() == 3) ? 2 : 1] + ".tif";
						if (b.size() < 10 && FetchTIFFCorners(tifName.c_str(), coords))
						{
							gRefSW_Lon = coords[0];
							gRefSW_Lat = coords[1];
							gRefSE_Lon = coords[2];
							gRefSE_Lat = coords[3];
							gRefNW_Lon = coords[4];
							gRefNW_Lat = coords[5];
							gRefNE_Lon = coords[6];
							gRefNE_Lat = coords[7];
						} else {
							if (b.size() < 10)
							{
								XGrinder_ShowMessage("USE_IMAGE requires 9 arguments when not used with a GeoTIF");
								return;
							}
						
							gRefSW_Lat = atof(b[2].c_str());
							gRefSW_Lon = atof(b[3].c_str());
							gRefNW_Lat = atof(b[4].c_str());
							gRefNW_Lon = atof(b[5].c_str());
							gRefNE_Lat = atof(b[6].c_str());
							gRefNE_Lon = atof(b[7].c_str());
							gRefSE_Lat = atof(b[8].c_str());
							gRefSE_Lon = atof(b[9].c_str());
						}
						
					}
					// SQUARE west, south, east, north
					else if (b[0] == "SQUARE")
					{
						if (b.size() < 5)
						{
							XGrinder_ShowMessage("APPLY command requires 4 arguments");
							return;						
						}					
						Squarify(
							atoi(b[1].c_str()),
							atoi(b[2].c_str()),
							atoi(b[3].c_str()),
							atoi(b[4].c_str()));
					}
					// AUTO_APPLY x y stretch_factor final_pixel_size
					else if (b[0] == "AUTO_APPLY")
					{
						if (b.size() < 5)
						{
							XGrinder_ShowMessage("AUTOAPPLY command requires 4 arguments");
							return;						
						}					
						char	buf[30];
						sprintf(buf,"%d_%d",atoi(b[1].c_str()), atoi(b[2].c_str()));
						string	to_where = path + buf;
						double	ppq = atof(b[4].c_str()) / atof(b[3].c_str());
						double	mpp = 570.0 / ppq;
						std::string scale = "4m";
						if (mpp < 5.0)
							scale = "1m";
					
						GetOrthoPhotos(
								scale.c_str(),
								atoi(b[1].c_str()),	// h
								atoi(b[2].c_str()), // v
								atoi(b[3].c_str()), // stretch
								to_where.c_str(),	// tex
								atoi(b[4].c_str()));
					}
					// MERGE west south north east res_per_quad final_file_name
					else if (b[0] == "MERGE" || b[0] == "UNMERGE")
					{
						if (b.size() < 7)
						{
							XGrinder_ShowMessage("%s command requires 6 arguments", b[0].c_str());
							return;						
						}
					
						string	fname = path + b[6];
						MergePhotos(
							atoi(b[1].c_str()),
							atoi(b[2].c_str()),
							atoi(b[3].c_str()),
							atoi(b[4].c_str()),
							atoi(b[5].c_str()),
							fname.c_str(),
							path.c_str(), 
							b[0] == "MERGE");
					}
					// READ
					else if (b[0] == "READ")
					{
						if (envName.empty())
						{
							XGrinder_ShowMessage("Cannot do READ command if no .env is dragged with the script file.");
							return;
						}
						ClearEnvData();
						ReadEnvFile(envName.c_str());
					}
					// WRITE
					else if (b[0] == "WRITE")
					{
						if (envName.empty())
						{
							XGrinder_ShowMessage("Cannot do WRITE command if no .env is dragged with the script file.");
							return;
						}					
						string envName2 = envName.substr(0, envName.length() - 4) + "_new.env";
						EnvWrite(envName2.c_str());
					} else 
						XGrinder_ShowMessage("Bad line: %s", line.c_str());
				}
				
			}
		}
	}
	if ((good + fail) > 0)
		XGrinder_ShowMessage("Success rate: %f%%", 100.0 * (double) good / (double) (good + fail));
	else
		XGrinder_ShowMessage("Finished - success.");
}
