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
#include "GISUtils.h"
#include <geotiffio.h>
#include <geo_normalize.h>
#if defined(__MWERKS__)
#include <libxtiff/xtiffio.h>
#else
#include <xtiffio.h>
#endif
#include <projects.h>
#include <cpl_serv.h>
#include "XESConstants.h"
#include "CompGeomUtils.h"

static	bool	TransformTiffCorner(GTIF * gtif, GTIFDefn * defn, double x, double y, double& outLon, double& outLat)
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

bool	FetchTIFFCorners(const char * inFileName, double corners[8])
{
	bool retVal = false;
	TIFF * tiffFile = XTIFFOpen(inFileName, "r");
	if (tiffFile)
	{
		retVal = FetchTIFFCornersWithTIFF(tiffFile, corners);
		XTIFFClose(tiffFile);
	}
	return retVal;
}

bool	FetchTIFFCornersWithTIFF(TIFF * tiffFile, double corners[8])
{
	bool retVal = false;
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
	        	TransformTiffCorner(gtif, &defn, xsize, 0,     corners[6], corners[7]))
	        {
	        	retVal = true;
	        }
		}
		GTIFFree(gtif);
	}    
	return retVal;
}

hash_map<int, projPJ>	sUTMProj;
struct CTABLE *		sNADGrid = NULL;

static	void	SetupUTMMap(int inZone)
{
	if (sUTMProj.find(inZone) != sUTMProj.end()) return;
	
	char ** args;
	char	argString[512];
	projPJ	proj; 

//	sprintf(argString,"+units=m +proj=utm +zone=%d +ellps=WGS84 ", inZone);
	sprintf(argString,"+units=m +proj=utm +zone=%d +ellps=clrk66 ", inZone);

	args = CSLTokenizeStringComplex(argString, " +", TRUE, FALSE);
	proj = pj_init(CSLCount(args), args);
	CSLDestroy(args);
	if (proj != NULL)
		sUTMProj.insert(hash_map<int, projPJ>::value_type(inZone, proj));
		
//	sNADGrid = nad_init("conus.bin");
}

void	UTMToLonLat(double x, double y, int zone, double * outLon, double * outLat)
{
	SetupUTMMap(zone);
	if (sUTMProj.find(zone) == sUTMProj.end())
		return;
	
      projUV	sUV;

    sUV.u = x;
    sUV.v = y;
    
//    sUV = nad_cvt(sUV, false, sNADGrid);

	sUV = pj_inv( sUV, sUTMProj[zone]);

	if (outLon) *outLon = sUV.u * RAD_TO_DEG;
	if (outLat) *outLat = sUV.v * RAD_TO_DEG;	
}


double	LonLatDistMeters(double lon1, double lat1, double lon2, double lat2)
{
	double dx = lon2 - lon1;
	double dy = lat2 - lat1;
	dy *= DEG_TO_MTR_LAT;
	dx *= (DEG_TO_MTR_LAT * cos((lat1 + lat2) * 0.5 * DEG_TO_RAD));
	return sqrt(dy * dy  + dx * dx);
}

double	LonLatDistMetersWithScale(double lon1, double lat1, double lon2, double lat2, 
								double deg_to_mtr_x, double deg_to_mtr_y)
{
	double dx = lon2 - lon1;
	double dy = lat2 - lat1;
	dy *= (deg_to_mtr_y);
	dx *= (deg_to_mtr_x);
	return sqrt(dy * dy  + dx * dx);
	
}


void	CreateTranslatorForPolygon(
					const Polygon2&		poly,
					CoordTranslator&	trans)
{
	if (poly.empty()) return;
	trans.mSrcMin = poly[0];
	trans.mSrcMax = poly[0];
	for (int n = 1; n < poly.size(); ++n)
	{
		trans.mSrcMin.x = min(trans.mSrcMin.x, poly[n].x);
		trans.mSrcMin.y = min(trans.mSrcMin.y, poly[n].y);
		trans.mSrcMax.x = max(trans.mSrcMax.x, poly[n].x);
		trans.mSrcMax.y = max(trans.mSrcMax.y, poly[n].y);
	}
	
	trans.mDstMin.x = 0.0;
	trans.mDstMax.x = 0.0;
	trans.mDstMax.x = (trans.mSrcMax.x - trans.mSrcMin.x) * DEG_TO_MTR_LAT * cos((trans.mSrcMin.y + trans.mSrcMax.y) * 0.5 * DEG_TO_RAD);
	trans.mDstMax.y = (trans.mSrcMax.y - trans.mSrcMin.y) * DEG_TO_MTR_LAT;
}					

void NorthHeading2Vector(const Point2& ref, const Point2& p, double heading, Vector2& dir)
{
	double lon_delta = p.x - ref.x;
	double real_heading = heading - lon_delta * sin(p.y * DEG_TO_RAD);
	
	dir.dx = sin(real_heading * DEG_TO_RAD);
	dir.dy = cos(real_heading * DEG_TO_RAD);
}

void MetersToLLE(const Point2& ref, int count, Point2 * pts)
{
	while(count--)
	{
		pts->y = ref.y + pts->y * DEG_TO_MTR_LAT;
		pts->x = ref.x + pts->x * DEG_TO_MTR_LAT * cos(pts->y * DEG_TO_RAD);
		
		++pts;
	}
}

