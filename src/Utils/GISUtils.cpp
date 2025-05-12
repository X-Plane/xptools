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
#if USE_TIF
#include <geotiffio.h>
#include <geo_normalize.h>
#define PVALUE LIBPROJ_PVALUE
#include <proj.h>
#include <cpl_serv.h>
#include <xtiffio.h>
#endif
#include "XESConstants.h"
#include "CompGeomUtils.h"
#include "DEMIO.h"

#include "FileUtils.h"
#include "MathUtils.h"
#include "PlatformUtils.h"

#if IBM
	#include "GUI_Unicode.h"
#endif

void	make_cache_file_path(const char * cache_base, int west, int south, const char * cache_name, char path[1024])
{
	sprintf(path, "%s%s%+03d%+04d%s%+03d%+04d.%s.txt", cache_base, DIR_STR, latlon_bucket (south), latlon_bucket (west), DIR_STR, (int) south, (int) west, cache_name);
}

int	latlon_bucket(int p)
{
	if (p > 0) return (p / 10) * 10;
	else return ((-p + 9) / 10) * -10;
}

double round_by_parts(double c, int parts)
{
	double fparts = parts;
	return round(c * fparts) / fparts;
}

double round_by_parts_guess(double c, int parts)
{
	if (parts % 2)
		return round_by_parts(c, parts - 1);
	else
		return round_by_parts(c, parts);
}

#if USE_TIF
bool	TransformTiffCorner(GTIF * gtif, GTIFDefn * defn, double x, double y, double& outLon, double& outLat)
{
    /* Try to transform the coordinate into PCS space */
    if( !GTIFImageToPCS( gtif, &x, &y ) )
    {
		LOG_MSG("  GTIF to PCS failed\n");
        return false;
	}
    if( defn->Model == ModelTypeGeographic )
    {
		LOG_MSG("  ModelIsGeo=yes: %lf %lf\n",x,y);
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
			LOG_MSG("  Proj4 worked: %lf %lf\n",x,y);
			return true;
		}
		else
			LOG_MSG("  Proj4 failed\n");
	}
	return false;
}
#endif

bool	FetchTIFFCorners(const char * inFileName, double corners[8], int& post_pos, gcp_t * gcp)
{
	bool retVal = false;
#if USE_TIF
	TIFF * tiffFile;
#if SUPPORT_UNICODE
	XTIFFInitialize();
	tiffFile = TIFFOpenW(convert_str_to_utf16(inFileName).c_str(), "r");
#else
	tiffFile = XTIFFOpen(inFileName, "r");
#endif
	if (tiffFile)
	{
		LOG_MSG("I/Gis Importing geotiff %s\n", inFileName);
		retVal = FetchTIFFCornersWithTIFF(tiffFile, corners, post_pos, gcp);
		XTIFFClose(tiffFile);
	}
#endif
	return retVal;
}

static int GTIFPrintFunc(char * txt, void *a)
{
	LOG_MSG("%s", txt);
	return 0;
}

bool	FetchTIFFCornersWithTIFF(TIFF * tiffFile, double corners[8], int& post_pos, gcp_t * gcp)
{
	bool retVal = false;
#if USE_TIF
	GTIF * gtif = GTIFNew(tiffFile);
	if (gtif)
	{
		GTIFPrint(gtif, GTIFPrintFunc, 0);

		GTIFDefn 	defn;
        if( GTIFGetDefn( gtif, &defn ) )
        {
#if WED
			GTIFPrintDefn(&defn, gLogFile ? gLogFile : stdout);
#endif
			LOG_MSG("PROJ.4 Definition: %s\n", GTIFGetProj4Defn(&defn));
        	int xs, ys;
            TIFFGetField( tiffFile, TIFFTAG_IMAGEWIDTH, &xs );
            TIFFGetField( tiffFile, TIFFTAG_IMAGELENGTH, &ys );
			double xsize(xs), ysize(ys);

			uint16 pixel_type;
			double dx(0.0), dy(0.0);

			if (GTIFKeyGet(gtif,GTRasterTypeGeoKey, &pixel_type, 0, 1) != 1)
				pixel_type = RasterPixelIsArea;

			// If we are a 'point sampled' file, the upper right edge _IS_ the last pixels!  Thus
			// passing in the number of pixels induces an off-by-one.  Cut the size by one to fix this.
			if(pixel_type == RasterPixelIsPoint)
			{
				xsize -= 1.0;
				ysize -= 1.0;
			}

			if(pixel_type == RasterPixelIsArea && post_pos == dem_want_Post)
			{
				// This is an area-pixel DEM, but we are going to reinterpret it via pixel centers.
				// This will INSET the corners of the pixels by 1/2 pixel to the sample centers.
				dx=0.5;
				dy=0.5;
			}

			if(pixel_type == RasterPixelIsPoint && post_pos == dem_want_Area)
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
	        	retVal = true;
	        }
	        
			if (gcp)
			{
				gcp->pts.clear();
				gcp->size_x = 1;
				gcp->size_y = 1;
				if (xsize > 1536 || ysize > 1536)  // calculate control points for map warping/projection, if texture has high resolution
				{
					gcp->size_x = intlim(roundf((double) xsize / 1024.0), 2, 10) + 1;
					gcp->size_y = intlim(roundf((double) ysize / 1024.0), 2, 10) + 1;
					for (int y = 0; y < gcp->size_y; y++)
						for (int x = 0; x < gcp->size_x; x++)
						{
							double lon, lat;
							if (TransformTiffCorner(gtif, &defn, dx + x * xsize / (gcp->size_x - 1), ysize - y * ysize / (gcp->size_y - 1) - dy, lon, lat))
								gcp->pts.push_back(Point2(lon, lat));
						}
				}
			}
		}
		GTIFFree(gtif);
	}
#endif
	return retVal;
}

#if USE_TIF
hash_map<int, PJ*>	sUTMProj;
struct CTABLE *		sNADGrid = NULL;

static	void	SetupUTMMap(int inZone)
{
	if (sUTMProj.find(inZone) != sUTMProj.end()) return;

	char	argString[512];
	PJ*	proj;

//	sprintf(argString,"+units=m +proj=utm +zone=%d +ellps=WGS84 ", inZone);
	sprintf(argString,"+units=m +proj=utm +zone=%d +ellps=clrk66 ", inZone);

	proj = proj_create(PJ_DEFAULT_CTX, argString);
	if (proj != NULL)
		sUTMProj.insert(hash_map<int, PJ*>::value_type(inZone, proj));

//	sNADGrid = nad_init("conus.bin");
}

// WARNING: This function was updated to use proj_trans() instead of pj_inv(), but that change is completely untested.
// As far as I can tell, this is actually dead code.
void	UTMToLonLat(double x, double y, int zone, double * outLon, double * outLat)
{
	SetupUTMMap(zone);
	if (sUTMProj.find(zone) == sUTMProj.end())
		return;

	PJ_COORD coord;
	coord.xy.x = x;
	coord.xy.y = y;
	coord = proj_trans(sUTMProj[zone], PJ_INV, coord);//pj_inv( sUV, sUTMProj[zone]);

	if (outLon) *outLon = coord.uv.u * RAD_TO_DEG;
	if (outLat) *outLat = coord.uv.v * RAD_TO_DEG;
}
#endif

double	LonLatDistMeters(Point2 lonlat1, Point2 lonlat2)
{
	return LonLatDistMeters(lonlat1.x(), lonlat1.y(), lonlat2.x(), lonlat2.y());
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

struct deg2mtr {
	double lon;
	double lat;
	deg2mtr(double latitude) {

		// https://en.wikipedia.org/wiki/Earth_radius#Principal_radii_of_curvature

		const double a = 6378137.0;
		const double b = 6356752.3;
		const double eps_sqr = 1.0 - (b * b) / (a * a);

		double phi = latitude * DEG_TO_RAD;
		double sin_phi = sin(phi);
		double cos_phi = cos(phi);

		double N = a / sqrt(1.0 - eps_sqr * sin_phi * sin_phi);
		double M = (1.0 - eps_sqr) / (a * a) * N * N * N;

		lon = N * 2.0 * M_PI / 360.0 * cos_phi;
		lat = M * 2.0 * M_PI / 360.0;
	}
};

void	CreateTranslatorForPolygon(
					const Polygon2&		poly,
					CoordTranslator2&	trans)
{
	if (poly.empty()) return;

	Bbox2 bounds;
	for (int n = 0; n < poly.size(); ++n)
	{
		bounds += poly[n];
	}
	CreateTranslatorForBounds(bounds, trans);
}

#if !NO_CGAL
void	CreateTranslatorForBounds(
					const Point_2&		inSrcMin,
					const Point_2&		inSrcMax,
					CoordTranslator_2&	trans)
{
	struct deg2mtr scale(0.5 * CGAL_NTS to_double(inSrcMin.y() +inSrcMax.y()));

	trans.mSrcMin = inSrcMin;
	trans.mSrcMax = inSrcMax;

	trans.mDstMin = Point_2(0,0);
	trans.mDstMax = Point_2(
				(trans.mSrcMax.x() - trans.mSrcMin.x()) * scale.lon,
				(trans.mSrcMax.y() - trans.mSrcMin.y()) * scale.lat);
}
#endif

void	CreateTranslatorForBounds(
					const Bbox2&		inBounds,
					CoordTranslator2&	trans)
{
	// This accounts for the unequal lon/latitude scales due to a ellipsoid earth model first used in X-Plane 12.
	struct deg2mtr scale(inBounds.centroid().y());

	// But this still does not bring the coord translator to the same level of accuracy as the rest of WED's map:
	// Earth curvature also means scale varies with lattitude and y-meters vary with longitudinal difference 
	// from the reference point. Basically a gnomometric-like projection would be required to at least keep errors 
	// for relatively small objects at bay.
	// Improving on this would require using better than linear interpolation in CompGeomUtils.cpp

	trans.mSrcMin = inBounds.p1;
	trans.mSrcMax = inBounds.p2;

	trans.mDstMin = Point2(0,0);
	trans.mDstMax = Point2(
					(trans.mSrcMax.x() - trans.mSrcMin.x()) * scale.lon,
					(trans.mSrcMax.y() - trans.mSrcMin.y()) * scale.lat);
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
	double cos_lat = MTR_TO_DEG_LAT / cos(ref.y() * DEG_TO_RAD);
	
	while(count--)
	{
		pts->y_ = ref.y() + pts->y() * MTR_TO_DEG_LAT;
		pts->x_ = ref.x() + pts->x() * cos_lat;

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
	ends[0] = Segment2(corners[0],corners[3]).midpoint();
	ends[1] = Segment2(corners[1],corners[2]).midpoint();

	Point2 side1 = Segment2(corners[0],corners[1]).midpoint();
	Point2 side2 = Segment2(corners[2],corners[3]).midpoint();

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
	Point2 ends1 = Segment2(corners[0],corners[3]).midpoint();
	Point2 ends2 = Segment2(corners[1],corners[2]).midpoint();

	Point2 side1 = Segment2(corners[0],corners[1]).midpoint();
	Point2 side2 = Segment2(corners[2],corners[3]).midpoint();

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
	Point2 ends1 = Segment2(corners[0],corners[3]).midpoint();
	Point2 ends2 = Segment2(corners[1],corners[2]).midpoint();

	Point2 side1 = Segment2(corners[0],corners[1]).midpoint();
	Point2 side2 = Segment2(corners[2],corners[3]).midpoint();

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

// buffer/extend rectangular quad
// length is point 0-1 direction, width point 1-2 direction
void Quad_Resize(Point2 corners[4], double width_m, double end0_m, double end1_m)
{
	Vector2 len_vec_1m = Vector2(corners[0], corners[1]) / LonLatDistMeters(corners[0], corners[1]);
	Vector2 len_ext = len_vec_1m * end0_m;
	Vector2 wid_vec_1m = Vector2(corners[1], corners[2]) / LonLatDistMeters(corners[1], corners[2]);
	Vector2 side_ext = wid_vec_1m * width_m;
	corners[0] -= len_ext + side_ext;
	corners[3] -= len_ext - side_ext;

	len_ext = len_vec_1m * end1_m;
	corners[1] += len_ext - side_ext;
	corners[2] += len_ext + side_ext;
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
