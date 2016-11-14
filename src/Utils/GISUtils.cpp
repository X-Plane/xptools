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
#define PVALUE LIBPROJ_PVALUE
#include <projects.h>
#include <cpl_serv.h>
#include "XESConstants.h"
#include "CompGeomUtils.h"
#include "DEMIO.h"
#include "MathUtils.h"
#include "PlatformUtils.h"
#if USE_GEOJPEG2K
#include <jasper/jasper.h>
#endif

// set to 1 to save geotiff inside geojp2 to disk
#define DUMP_GTIF 0

#if defined(_MSC_VER)
	#include <libxtiff/xtiffio.h>
#else
	#include <xtiffio.h>
#endif
void	make_cache_file_path(const char * cache_base, int west, int south, const char * cache_name, char path[1024])
{
	sprintf(path, "%s%s%+03d%+04d%s%+03d%+04d.%s.txt", cache_base, DIR_STR, latlon_bucket (south), latlon_bucket (west), DIR_STR, (int) south, (int) west, cache_name);
}


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

		int size = 0;
		tagtype_t type = TYPE_UNKNOWN;
		int key_count = GTIFKeyInfo(gtif, GTCitationGeoKey, &size, &type);
		
		if(key_count > 0 && key_count < 1024 && type == TYPE_ASCII && size == 1)
		{
			vector<char>	ascii(key_count);
			int r = GTIFKeyGet(gtif, GTCitationGeoKey, &ascii[0], 0, key_count);
			if(r == key_count)
			{
				DebugAssert(ascii.back() == 0);
				string citation = string(&ascii[0]);
				if(citation == "PCS Name = WGS_1984_Web_Mercator_Auxiliary_Sphere")
				{
					char ** args = CSLTokenizeStringComplex("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs", " +", TRUE, FALSE);
					PJ * psPJ = pj_init( CSLCount(args), args );
					CSLDestroy(args);
					if(psPJ)
					{
						projUV	sUV;

						sUV.u = x;
						sUV.v = y;

						sUV = pj_inv( sUV, psPJ );

						outLon = sUV.u * RAD_TO_DEG;
						outLat = sUV.v * RAD_TO_DEG;
						pj_free(psPJ);
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool	FetchTIFFCorners(const char * inFileName, double corners[8], int& post_pos)
{
	bool retVal = false;
	TIFF * tiffFile = XTIFFOpen(inFileName, "r");
	if (tiffFile)
	{
		retVal = FetchTIFFCornersWithTIFF(tiffFile, corners, post_pos);
		XTIFFClose(tiffFile);
	}
	
	return retVal;
}

static int pm(char * s, void * v)
{
	printf("%s",s);
	return 0;
}

bool	FetchTIFFCornersWithTIFF(TIFF * tiffFile, double corners[8], int& post_pos, int width, int height)
{
	bool retVal = false;
	GTIF * gtif = GTIFNew(tiffFile);
	if (gtif)
	{
//		GTIFPrint(gtif, pm, NULL);

		GTIFDefn 	defn;
        if( GTIFGetDefn( gtif, &defn ) )
        {
        	int xs, ys;
			double xsize,ysize;
            TIFFGetField( tiffFile, TIFFTAG_IMAGEWIDTH, &xs );
            TIFFGetField( tiffFile, TIFFTAG_IMAGELENGTH, &ys );

			uint16 pixel_type;
			double dx=0.0;
			double dy=0.0;

			//If there is the optional width and height
			if(width > 0 && height > 0)
			{
				xsize=width;
				ysize=height;
			}
			else
			{
				xsize=xs;
				ysize=ys;
			}

			if (GTIFKeyGet(gtif,GTRasterTypeGeoKey, &pixel_type, 0, 1) != 1)
				pixel_type=RasterPixelIsArea;

			// If we are a 'point sampled' file, the upper right edge _IS_ the last pixels!  Thus
			// passing in the number of pixels induces an off-by-one.  Cut the size by one to fix this.
			if(pixel_type == RasterPixelIsPoint)
			{
				xsize -= 1.0;
				ysize -= 1.0;
			}

			if(pixel_type==RasterPixelIsArea && post_pos == dem_want_Post)
			{
				// This is an area-pixel DEM, but we are going to reinterpret it via pixel centers.
				// This will INSET the corners of the pixels by 1/2 pixel to the sample centers.
				dx=0.5;
				dy=0.5;
			}

			if(pixel_type==RasterPixelIsPoint && post_pos == dem_want_Area)
			{
				// This is a center post sampled image, but we are going to treat it as area.  Each
				// pixel "sticks out" a bit in its coverage, so extend.
				dx=-0.5;
				dy=-0.5;
			}

			if(post_pos == dem_want_File)
				post_pos = (pixel_type==RasterPixelIsPoint) ? dem_want_Post : dem_want_Area;

        	if (TransformTiffCorner(gtif, &defn,	   dx, ysize-dy, corners[0], corners[1]) &&
	        	TransformTiffCorner(gtif, &defn, xsize-dx, ysize-dy, corners[2], corners[3]) &&
	        	TransformTiffCorner(gtif, &defn,	   dx,		 dy, corners[4], corners[5]) &&
	        	TransformTiffCorner(gtif, &defn, xsize-dx,		 dy, corners[6], corners[7]))
	        {
				// Ben says: we used to snap round.  Since the 'far' tie point is calculated by res * pixels
				// and res might be 1/1200 or 1/1201, we get floating point crud in our tiff calcs.  But
				// if we aren't known to be on 1-degree boundaries, this snap rounding is just wrong.  So:
				// don't round - we need good precision in other places.  Instead, we can round in the raster-import cmd.
//				corners[0]=round_by_parts_guess(corners[0],xs);
//				corners[2]=round_by_parts_guess(corners[2],xs);
//				corners[4]=round_by_parts_guess(corners[4],xs);
//				corners[6]=round_by_parts_guess(corners[6],xs);
//
//				corners[1]=round_by_parts_guess(corners[1],ys);
//				corners[3]=round_by_parts_guess(corners[3],ys);
//				corners[5]=round_by_parts_guess(corners[5],ys);
//				corners[7]=round_by_parts_guess(corners[7],ys);


	        	retVal = true;
	        }
		}
		GTIFFree(gtif);
	}
	return retVal;
}
#if USE_GEOJPEG2K

//Represents a jas_aux_buffer_t + the offset of bytes read.
struct	MemJASGeoFile {
	MemJASGeoFile(jas_aux_buffer_t * aux_buf_t) 
	{
		file = aux_buf_t;
		offset = 0; 
	}
	~MemJASGeoFile() { }

	//The jas_aux_buffer_t (containing the GeoTiff [in the form of aux_buf], and the length of said buffer)
	jas_aux_buffer_t *		file;
	//Number of bytes read
	int				offset;

	//Get's the end pointer of the file
	unsigned char * GetEnd()
	{
		//End = address of start + number of charecters * 8
		return 	(unsigned char *)(file->buf + (file->size * 8));
	}
};


static tsize_t	MemJASGeoRead(thandle_t handle, tdata_t data, tsize_t len)
{
	//Cast a the handle back to a MemJASGeoFile
	MemJASGeoFile * f = (MemJASGeoFile *) handle;

	//Get the remaining bytes
	//Start is pointer
	//End = address of start + number of charecters * 8

	//int remain = End-Start-Offset
	
	int remain = f->GetEnd() - (f->file->buf) - (f->offset);
	
	if (len > remain) 
		len = remain;
	if (len < 0) 
		len = 0;
	//If length is greater than 0, copy the data
	//Copy from the pointer + the offset
	if (len > 0)
		memcpy(data,f->file->buf+f->offset,len);

	f->offset += len;
	return len;
}

static tsize_t MemJASGeoWrite(thandle_t handle, tdata_t data, tsize_t len)
{
	return 0;
}

static toff_t 	MemJASGeoSeek(thandle_t handle, toff_t pos, int mode)
{
	MemJASGeoFile * f = (MemJASGeoFile *) handle;
	switch(mode) {
	case SEEK_SET:
	default:
		f->offset = pos;
		return f->offset;
	case SEEK_CUR:
		f->offset += pos;
		return f->offset;
	case SEEK_END:
		f->offset = f->GetEnd()-f->file->buf - pos;
		return f->offset;
	}
}

static int 		MemJASGeoClose(thandle_t)
{
	return 0;
}

static toff_t 	MemJASGeoSize(thandle_t handle)
{
	MemJASGeoFile * f = (MemJASGeoFile *) handle;
	return f->GetEnd()-f->file->buf;
}

static int 		MemJASGeoMapFile(thandle_t handle, tdata_t* dp, toff_t* len)
{
	MemJASGeoFile * f = (MemJASGeoFile *) handle;
	*dp = (tdata_t) f->file->buf;
	*len = f->GetEnd() - (f->file->buf);
	return 1;
}

static void 	MemJASGeoUnmapFile(thandle_t, tdata_t, toff_t)
{
}

bool	FetchTIFFCornersWithJP2K(const char * inFileName, double corners[8], int& post_pos)
{
	jas_stream_t *inStream;
	jas_image_t *image;

	if(jas_init() != 0 )
	{
		//If it failed then return error
		return -1;
	}

		//If the data stream cannot be created
	if((inStream = jas_stream_fopen(inFileName,"rb"))==false)
	{
		return false;
	}

	//Get the format ID
	int formatId;

	//If there are any errors in getting the format
	if((formatId = jas_image_getfmt(inStream)) < 0)
	{
		//It is an invalid format
		return false;
	}

	//If the image cannot be decoded
	if((image = jas_image_decode(inStream, formatId, 0)) == false)
	{
		//Return an error
		return false;
	}
	
	//If it turns out the .jp2 never had any geological data exit
	if(image->aux_buf.size == 0)
	{
		return false;
	}
	
	
	#if DUMP_GTIF
	FILE * foo = fopen("temp.tiff","wb");
	if(foo)
	{
		fwrite(image->aux_buf.buf, 1, image->aux_buf.size, foo);
		fclose(foo);
	}
	#endif
	
	//Create the handle to be used in XTIFFClientOpen
	MemJASGeoFile jasHandle(&image->aux_buf);
	//Create a TIFF handle
	TIFF * tif = XTIFFClientOpen(inFileName,"r",&jasHandle,MemJASGeoRead, MemJASGeoWrite,
	    MemJASGeoSeek, MemJASGeoClose,
	    MemJASGeoSize,
 	    MemJASGeoMapFile, MemJASGeoUnmapFile);

	int postType = dem_want_Area;
	//Pass in our TIF handle, post type, width, and height
	if(FetchTIFFCornersWithTIFF(tif,corners,postType,(image->brx_-image->tlx_),(image->bry_-image->tly_))==false)
	{
		return false;
	}
	//Shut downthe stream
	jas_stream_close(inStream);
	
	//Unintialize jasper
	jas_cleanup();
	//It all worked!
	return true;
}
#endif

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
					CoordTranslator2&	trans)
{
	if (poly.empty()) return;
	trans.mSrcMin = poly[0];
	trans.mSrcMax = poly[0];
	for (int n = 1; n < poly.size(); ++n)
	{
		trans.mSrcMin.x_ = min(trans.mSrcMin.x(), poly[n].x());
		trans.mSrcMin.y_ = min(trans.mSrcMin.y(), poly[n].y());
		trans.mSrcMax.x_ = max(trans.mSrcMax.x(), poly[n].x());
		trans.mSrcMax.y_ = max(trans.mSrcMax.y(), poly[n].y());
	}

	trans.mDstMin.x_ = 0.0;
	trans.mDstMax.y_ = 0.0;
	trans.mDstMax.x_ = (trans.mSrcMax.x() - trans.mSrcMin.x()) * DEG_TO_MTR_LAT * cos((trans.mSrcMin.y() + trans.mSrcMax.y()) * 0.5 * DEG_TO_RAD);
	trans.mDstMax.y_ = (trans.mSrcMax.y() - trans.mSrcMin.y()) * DEG_TO_MTR_LAT;
}

#if !NO_CGAL
void	CreateTranslatorForBounds(
					const Point_2&		inSrcMin,
					const Point_2&		inSrcMax,
					CoordTranslator_2&	trans)
{
	trans.mSrcMin = inSrcMin;
	trans.mSrcMax = inSrcMax;

	trans.mDstMin = Point_2(0,0);
	trans.mDstMax = Point_2(
					(trans.mSrcMax.x() - trans.mSrcMin.x()) * DEG_TO_MTR_LAT * cos(to_double((trans.mSrcMin.y() + trans.mSrcMax.y())) * 0.5 * DEG_TO_RAD),
					(trans.mSrcMax.y() - trans.mSrcMin.y()) * DEG_TO_MTR_LAT);
}
#endif

void	CreateTranslatorForBounds(
					const Bbox2&		inBounds,
					CoordTranslator2&	trans)
{
	trans.mSrcMin = inBounds.p1;
	trans.mSrcMax = inBounds.p2;

	trans.mDstMin = Point2(0,0);
	trans.mDstMax = Point2(
					(trans.mSrcMax.x() - trans.mSrcMin.x()) * DEG_TO_MTR_LAT * cos(((trans.mSrcMin.y() + trans.mSrcMax.y())) * 0.5 * DEG_TO_RAD),
					(trans.mSrcMax.y() - trans.mSrcMin.y()) * DEG_TO_MTR_LAT);
}


void NorthHeading2VectorMeters(const Point2& ref, const Point2& p, double heading, Vector2& dir)
{
	double lon_delta = p.x() - ref.x();
	double real_heading = heading - lon_delta * sin(p.y() * DEG_TO_RAD);

	dir.dx = sin(real_heading * DEG_TO_RAD);
	dir.dy = cos(real_heading * DEG_TO_RAD);
}

double VectorDegs2NorthHeading(const Point2& ref, const Point2& p, const Vector2& dir)
{
	double dx = dir.dx * cos (ref.y() * DEG_TO_RAD);
	double h = atan2(dx, dir.dy) * RAD_TO_DEG;
	if (h < 0.0) h += 360.0;
	double lon_delta = p.x() - ref.x();
	return h + lon_delta * sin(p.y() * DEG_TO_RAD);
}

void NorthHeading2VectorDegs(const Point2& ref, const Point2& p, double heading, Vector2& dir)
{
	double lon_delta = p.x() - ref.x();
	double real_heading = heading - lon_delta * sin(p.y() * DEG_TO_RAD);

	dir.dx = sin(real_heading * DEG_TO_RAD) / cos (ref.y() * DEG_TO_RAD);
	dir.dy = cos(real_heading * DEG_TO_RAD);
}

double VectorMeters2NorthHeading(const Point2& ref, const Point2& p, const Vector2& dir)
{
	double h = atan2(dir.dx, dir.dy) * RAD_TO_DEG;
	if (h < 0.0) h += 360.0;
	double lon_delta = p.x() - ref.x();
	return h + lon_delta * sin(p.y() * DEG_TO_RAD);
}


void MetersToLLE(const Point2& ref, int count, Point2 * pts)
{
	while(count--)
	{
		pts->y_ = ref.y() + pts->y() * MTR_TO_DEG_LAT;
		pts->x_ = ref.x() + pts->x() * MTR_TO_DEG_LAT / cos(pts->y() * DEG_TO_RAD);

		++pts;
	}
}
/*
double VectorLengthMeters(const Point2& ref, const Vector2& vec)
{
	printf("LL: %lf,%lf ",vec.dx,vec.dy);
	double dx = vec.dx * DEG_TO_MTR_LAT * cos(ref.y * DEG_TO_RAD);
	double dy = vec.dy * DEG_TO_MTR_LAT;
	printf("MTR: %lf,%lf\n",dx,dy);
	return sqrt(dx*dx+dy*dy);
}
*/

Vector2 VectorLLToMeters(const Point2& ref, const Vector2& v)
{
	Vector2	ret(v);
	ret.dx *= (DEG_TO_MTR_LAT * cos(ref.y() * DEG_TO_RAD) );
	ret.dy *= (DEG_TO_MTR_LAT							);
	return ret;
}


Vector2 VectorMetersToLL(const Point2& ref, const Vector2& v)
{
	Vector2	ret(v);
	ret.dx /= (DEG_TO_MTR_LAT * cos(ref.y() * DEG_TO_RAD) );
	ret.dy /= (DEG_TO_MTR_LAT							);
	return ret;
}


#pragma mark -

void	Quad_2to4(const Point2 ends[2], double width_mtr, Point2 corners[4])
{
	Vector2	dir(ends[0],ends[1]);
	dir.dx *= cos((ends[0].y() + ends[1].y()) * 0.5 * DEG_TO_RAD);
	dir.normalize();
	Vector2 right(dir.perpendicular_cw());
	Point2 zero;

	corners[0] = zero - right * width_mtr * 0.5;
	corners[1] = zero - right * width_mtr * 0.5;
	corners[2] = zero + right * width_mtr * 0.5;
	corners[3] = zero + right * width_mtr * 0.5;

	MetersToLLE(ends[0], 1, corners  );
	MetersToLLE(ends[1], 2, corners+1);
	MetersToLLE(ends[0], 1, corners+3);
}

void	Quad_4to2(const Point2 corners[4], Point2 ends[2], double& width_mtr)
{
	ends[0] = Segment2(corners[0],corners[3]).midpoint(0.5);
	ends[1] = Segment2(corners[1],corners[2]).midpoint(0.5);

	Point2 side1 = Segment2(corners[0],corners[1]).midpoint(0.5);
	Point2 side2 = Segment2(corners[2],corners[3]).midpoint(0.5);

	width_mtr = sqrt(VectorLLToMeters(Segment2(side1,side2).midpoint(),Vector2(side1,side2)).squared_length());
}

void	Quad_1to4(const Point2& ctr, double heading, double len_mtr, double width_mtr, Point2 corners[4])
{
	Vector2		dir;

	NorthHeading2VectorMeters(ctr, ctr, heading,dir);
	dir.normalize();
	Vector2 right(dir.perpendicular_cw());

	Point2	zero(0,0);
	corners[0] = zero - dir * len_mtr * 0.5 - right * width_mtr * 0.5;
	corners[1] = zero + dir * len_mtr * 0.5 - right * width_mtr * 0.5;
	corners[2] = zero + dir * len_mtr * 0.5 + right * width_mtr * 0.5;
	corners[3] = zero - dir * len_mtr * 0.5 + right * width_mtr * 0.5;

	MetersToLLE(ctr, 4, corners);
}

void	Quad_4to1(const Point2 corners[4], Point2& ctr, double& heading, double& len_mtr, double& width_mtr)
{
	Point2 ends1 = Segment2(corners[0],corners[3]).midpoint(0.5);
	Point2 ends2 = Segment2(corners[1],corners[2]).midpoint(0.5);

	Point2 side1 = Segment2(corners[0],corners[1]).midpoint(0.5);
	Point2 side2 = Segment2(corners[2],corners[3]).midpoint(0.5);

	ctr.x_ = (corners[0].x()  + corners[1].x()  + corners[2].x() + corners[3].x()) * 0.25;
	ctr.y_ = (corners[0].y()  + corners[1].y()  + corners[2].y() + corners[3].y()) * 0.25;

	heading = VectorDegs2NorthHeading(ctr,ends1,Vector2(ends1,ends2));
	width_mtr = sqrt(VectorLLToMeters(ctr,Vector2(side1, side2)).squared_length());
	len_mtr = sqrt(VectorLLToMeters(ctr,Vector2(ends1, ends2)).squared_length());

}

void	Quad_2to1(const Point2 ends[2], Point2& ctr, double& heading, double& len_mtr)
{
	heading = VectorDegs2NorthHeading(ends[0],ends[0],Vector2(ends[0],ends[1]));
	len_mtr = sqrt(VectorLLToMeters(ends[0],Vector2(ends[0],ends[1])).squared_length());
	ctr.x_ = (ends[0].x() + ends[1].x()) * 0.5;
	ctr.y_ = (ends[0].y() + ends[1].y()) * 0.5;
}

void	Quad_1to2(const Point2& ctr, double heading, double len_mtr, Point2 ends[2])
{
	Vector2		dir;

	NorthHeading2VectorMeters(ctr, ctr, heading,dir);
	dir.normalize();

	Point2	zero(0,0);
	ends[0] = zero - dir * len_mtr * 0.5;
	ends[1] = zero + dir * len_mtr * 0.5;

	MetersToLLE(ctr, 2, ends);
}

void	Quad_diagto1(const Point2 ends[2], double width_mtr, Point2& ctr, double& heading, double& len_mtr, int swapped)
{
	double diag_len = sqrt(VectorLLToMeters(ends[0],Vector2(ends[0],ends[1])).squared_length());
	len_mtr = sqrt(max(diag_len * diag_len - width_mtr * width_mtr,0.0));

	double diag_heading = VectorDegs2NorthHeading(ends[0],ends[0],Vector2(ends[0],ends[1]));

	// When user drags corners to < width, we need clamp to avoid NaN from asin out of range [-1..1]
	double offset = diag_len == 0.0 ? 0.0 : asin(doblim(width_mtr / diag_len,-1.0,1.0)) * RAD_TO_DEG;

	if (swapped)
		heading = diag_heading + offset;
	else
		heading = diag_heading - offset;

	ctr.x_ = (ends[0].x() + ends[1].x()) * 0.5;
	ctr.y_ = (ends[0].y() + ends[1].y()) * 0.5;
}

void	Quad_MoveSide2(Point2 ends[2], double& width_mtr, int side, const Vector2& delta)
{
	if (side == 2 || side == 0)
	{
		// ccw
		double h, len;
		Point2 ctr;

		Quad_2to1(ends, ctr, h, len);
		swap(width_mtr,len);
		h-= 90.0;
		Quad_1to2(ctr,h,len,ends);

		Quad_MoveSide2(ends, width_mtr, side+1, delta);

		Quad_2to1(ends, ctr, h, len);
		swap(width_mtr,len);
		h+= 90.0;
		Quad_1to2(ctr,h,len,ends);

		return;
	}

	if (side == 1) ends[1] += delta;
	if (side == 3) ends[0] += delta;

}

void Quad_ResizeSide4(Point2 corners[4], int side, const Vector2& move, bool symetric)
{
	Point2 ends1 = Segment2(corners[0],corners[3]).midpoint(0.5);
	Point2 ends2 = Segment2(corners[1],corners[2]).midpoint(0.5);

	Point2 side1 = Segment2(corners[0],corners[1]).midpoint(0.5);
	Point2 side2 = Segment2(corners[2],corners[3]).midpoint(0.5);

	Vector2	dir;
	switch(side) {
	case 0: dir = Vector2(side2,side1);	break;
	case 1: dir = Vector2(ends1,ends2);	break;
	case 2: dir = Vector2(side1,side2);	break;
	case 3: dir = Vector2(ends2,ends1);	break;
	}
	dir.normalize();
	Vector2	real_move = dir.projection(move);

	corners[side] += real_move;
	corners[(side+1)%4] += real_move;

	if (symetric)
	{
		corners[(side+2)%4] -= real_move;
		corners[(side+3)%4] -= real_move;
	}

}

void Quad_ResizeCorner1(Point2& ctr, double heading, double& l, double& w, int corner, const Vector2& move, bool symetric)
{
	if (!symetric) ctr += (move * 0.5);

	Vector2	move_mtrs = VectorLLToMeters(ctr, move);
	Vector2	axis;
	NorthHeading2VectorMeters(ctr, ctr, heading, axis);
	Vector2	right = axis.perpendicular_cw();

	if (corner == 0 || corner == 3) axis = -axis;
	if (corner == 0 || corner == 1) right = -right;

	Vector2	move_axis = axis.projection(move_mtrs);
	Vector2	move_right = right.projection(move_mtrs);

	double ascale = symetric ? 2.0  : 1.0;
	double rscale = symetric ? 2.0  : 1.0;
	if (axis.dot(move_axis) < 0.0) ascale = -ascale;
	if (right.dot(move_right) < 0.0) rscale = -rscale;

	l += (ascale * sqrt(move_axis.squared_length()));
	w += (rscale * sqrt(move_right.squared_length()));
}
