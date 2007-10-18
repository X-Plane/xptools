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
#ifndef DEMDEFS_H
#define DEMDEFS_H

#include <math.h>
#include <algorithm>

#include "XESConstants.h"
#include "ProgressUtils.h"
#include "AssertUtils.h"

/*

	DEMGEO - THEORY OF OPERATION
	
	A DEMGeo (Geographic DEM) is really an array-based value map, or a raster
	representation of a continuous floating point parameter.
	It is applied to a geoprojected area with a constant (but controllable)
	resolution.
	
	DEMGeos use center-pixel notation, meaning the lower left pixel is the data
	that is precisely sampled from the southwest corner of the DEM.  Therefore
	if we have 1200 samples per degree, the DEM will contain 1201 samples, and
	two adjacent DEMs will duplicate data by sharing their common edges.
	
	While DEM data are floating point, we sometimes stuff integer enumeration
	coded data into them.
	
	One or more DEMGeos are used in XES files to represent elevation, land type,
	and other continuous parameters.

 */


/*************************************************************************************
 * NO_DATA - MISSING RASTER POINTS
 *************************************************************************************/


// For continuous floating point data we use this as a flag indicating an absense of information.
#define DEM_NO_DATA	-32768.0

// These inline functions do min/max but are not fooled by an absense of data.
inline	float	MIN_NODATA(float a, float b)
{
	if (a == DEM_NO_DATA && b == DEM_NO_DATA) return DEM_NO_DATA;
	if (a == DEM_NO_DATA) return b;
	if (b == DEM_NO_DATA) return a;
	return (a < b) ? a : b;
}

inline	float	MAX_NODATA(float a, float b)
{
	if (a == DEM_NO_DATA && b == DEM_NO_DATA) return DEM_NO_DATA;
	if (a == DEM_NO_DATA) return b;
	if (b == DEM_NO_DATA) return a;
	return (a > b) ? a : b;
}

inline float ADD_NODATA(float a, float b)
{
	if (a == DEM_NO_DATA && b == DEM_NO_DATA) return DEM_NO_DATA;
	if (a == DEM_NO_DATA) return b;
	if (b == DEM_NO_DATA) return a;
	return a + b;
}

inline	float	MIN_NODATA_XY(float a, float b, int& xo, int& yo, int xn, int yn )
{
	if (a == DEM_NO_DATA && b == DEM_NO_DATA) return DEM_NO_DATA;
	if (a == DEM_NO_DATA) { xo = xn; yo = yn; return b; }
	if (b == DEM_NO_DATA) return a;
	if (a <= b) return a;
	xo = xn; yo = yn; return b;
}


/*************************************************************************************
 * DEMGeo - SINGLE RASTER LAYER
 *************************************************************************************/

struct	DEMGeo {

	// The bounding box of the DEM, defining the lat/lons of the corner
	// heights.
	double	mWest;
	double	mSouth;
	double	mEast;
	double	mNorth;
	
	// The number of sample points east-west and north-south.  This includes
	// the 'extra' sample, e.g. for a 3 arc-second DEM, mWidth = 1201
	int		mWidth;
	int		mHeight;
	
	// An array of width*height data points in floating point format.
	// The first sample is the southwest corner, we then proceed east.
	float *	mData;
	
	DEMGeo();
	DEMGeo(const DEMGeo&);
	DEMGeo(int width, int height);
	~DEMGeo();
	
	DEMGeo& operator=(float);			// Fill
	DEMGeo& operator=(const DEMGeo&);	// Copy	
	DEMGeo& operator+=(float);			// These have standard mathematical meanings.
	DEMGeo& operator+=(const DEMGeo&);	// DEM->DEM requires size-similar DEMs!
	DEMGeo& operator*=(float);
	DEMGeo& operator*=(const DEMGeo&);
	
	
	void	resize(int width, int height);
	void	derez(int);
	void	overlay(const DEMGeo& onTop);					// Overlay - requires 1:1 layout
	void	overlay(const DEMGeo& onTop, int dx, int dy);	// Overlay - requires onTop <= main
	void	copy_geo_from(const DEMGeo& rhs);
	
	// Access to the data points by discrete address
	inline float&	operator()(int, int);
	inline float	operator()(int, int) const;
	inline float	get(int x, int y) const;									// Get value at x,y, DEM_NO_DATA if out of bonds
	inline void		set(int x, int y, float v);									// Safe set - no-op if off
	inline float	get_clamp(int x, int y) const;								// Get value at x,y, clamped to within the DME
	inline float	get_dir(int x, int y, int dx, int dy, 						// Get first value in a given direction and dist that doesn't
						int max_radius, float blank, float& outDist) const;		// Match 'blank'
	inline float	get_radial(int x, int y, int max, float blank) const;		// Get first value in any direction that doesn't match blank
	inline int		radial_dist(int x, int y, int max, float key) const;		// Get dist to nearest value or -1 if not in range!  0 if we have that val
	inline float	get_lowest(int x, int y, int r) const;						// Get lowest value within 1 dem point
	inline float	get_lowest(int x, int y, int r, int& xo, int& yo) const;	// Get lowest value within 1 dem point
	inline float	get_lowest_heuristic(int x, int y, int r) const;
	inline float	kernelN(int x, int y, int dim, float * k) const;			// Get value of kernel applied at N
	inline float	kernelmaxN(int x, int y, int dim, float * k) const;			// Get value of kernel applied at N, taking max instead of average
	inline float	kernelN_Normalize(int x, int y, int dim, float * k) const;	// ...

	inline void	zap(int x, int y);												// Set x,y to DEM_NO_DATA

	// Lat/lon access and erasing
	inline float	value_linear(double lon, double lat) const;					// Get linear interp value of coordinate
	inline void	zap_linear(double lon, double lat);							// Zap all values that are near the point (up to 2x2)
	inline float	xy_nearest(double lon, double lat				 ) const;	// Get nearest-neighbor value, return coordinate used
	inline float	xy_nearest(double lon, double lat, int& x, int& y) const;	// Get nearest-neighbor value, return coordinate used
	inline float	search_nearest(double lon, double lat) const;				// Get nearest-neighbor value, search indefinitely
	
	// These routines convert grid positions to lat/lon
	inline double	x_to_lon(int inX) const;
	inline double	y_to_lat(int inY) const;
	inline double	x_to_lon_double(double inX) const;
	inline double	y_to_lat_double(double inY) const;
	inline double	lon_to_x(double inLon) const;
	inline double	lat_to_y(double inLat) const;
	inline double	x_dist_to_m(int inX) const;
	inline double	y_dist_to_m(int inY) const;

	// These routines return the grid point below or above a coordinate constrained
	// to within the grid.
	inline int		x_lower(double lon) const;
	inline int		x_upper(double lon) const;
	inline int		y_lower(double lat) const;
	inline int		y_upper(double lat) const;

	// This routine creates an extracted DEM along grid lines.
	void	subset(DEMGeo& newDEM, int x1, int y1, int x2, int y2) const;					// This uses INCLUSIVE boundaries!!
	void	swap(DEMGeo& otherDEM);															// Swap all params, good for avoiding mem copies
	void	calc_slope(DEMGeo& outSlope, DEMGeo& outHeading, ProgressFunc inFunc) const;

	void	fill_nearest(void);

	// Advanced routines	
	int		remove_linear(int iterations, float max_err);
	float	local_minmax(int x1, int y1, int x2, int y2,
						 int& minx, int& miny, float& minh,
						 int& maxx, int& maxy, float& maxh);

	void	filter_self(int dim, float * k);
	void	filter_self_normalize(int dim, float * k);

#if !DEV
inline this
#endif
			float	gradient_x(int x, int y) const;					// These return exact gradients at HALF-POSTINGS!
			float	gradient_y(int x, int y) const;					// So for a 1201 DEM there are 1200 gradients.  This is really utility funcs
			float	gradient_x_bilinear(float x, float y) const;	// These return interp'd gradients on WHOLE POSTINGS, which is what we REALLY
			float	gradient_y_bilinear(float x, float y) const;	// want...remember that we can't get an exact discrete gradient on the post (duh).

};

/*************************************************************************************
 * DEMGeo - SINGLE RASTER LAYER
 *************************************************************************************/

// Given two DEMs that represent the minimum and maximum possible values for various
// points, this routine produces two DEMs of half dimension.  Each point has the min
// or max of the four points in the original DEMs that correspond spatially.
void	DEMGeo_ReduceMinMax(
			const DEMGeo& inMin, 
			const DEMGeo& inMax,
				  DEMGeo& outMin,
				  DEMGeo& outMax);
				  
void	DEMGeo_ReduceMinMaxN(
			const DEMGeo& inData, 
				  DEMGeo& outMin,
				  DEMGeo& outMax,
				  int N);
				  

// Given a DEM, produce a pair of vectors of DEMs of progressively smaller size that 
// summarize our min and max values.  (This is like a mipmap.)  
void	DEMGeo_BuildMinMax(
			const DEMGeo& 		inDEM,
			vector<DEMGeo>&		outMin,
			vector<DEMGeo>&		outMax,
			int					inGenerations);

// For a given "cache square" in the minmax'd DEM, find the actual lowest or highest			
float	DEMGeo_LocalMinOfCacheSquare(
			const DEMGeo&			inDEM,
			const vector<DEMGeo>&	inMin,
			int level,
			int x, int y, 
			int& minx, int& miny, float& minh);					

float	DEMGeo_LocalMaxOfCacheSquare(
			const DEMGeo&			inDEM,
			const vector<DEMGeo>&	inMax,
			int level,
			int x, int y, 
			int& maxx, int& maxy, float& maxh);					

// For a reduced DEM, find the highest and lowest points in a bounded range.  This 
// routine runs in O(ln(n)) instead of O(n) where n = the area to be searched.  
// if inAllowEdges is true, the edge may be the min/max.  Otherwise if the edge is the
// min max, it is treated as if no local min max was found.
float	DEMGeo_LocalMinMaxWithCache(
			const DEMGeo&			inDEM,
			const vector<DEMGeo>&	inMin,
			const vector<DEMGeo>&	inMax,
			int x1, int y1, int x2, int y2,
			int& minx, int& miny, float& minh,
			int& maxx, int& maxy, float& maxh,
			bool					inAllowEdges);

// Same as above, but faster because it does not return the location of the min/max
// within the DEM.  Edges are always included.
float	DEMGeo_LocalMinMaxWithCache(
			const DEMGeo&			inDEM,
			const vector<DEMGeo>&	inMin,
			const vector<DEMGeo>&	inMax,
			int x1, int y1, int x2, int y2,
			float& 					minh,
			float& 					maxh);


/*************************************************************************************
 * DEMGeoMap - MULTIPLE RASTER LAYERS BY CODE
 *************************************************************************************/

class DEMGeoMap : public hash_map<int, DEMGeo> {
public:
	// Write ourselves a hokey const-safe [] because I am impatient!!
	DEMGeo& operator[](int i) {
		return hash_map<int, DEMGeo>::operator[](i);
	}


	const DEMGeo& operator[](int i) const {
		static DEMGeo dummy;
		hash_map<int, DEMGeo>::const_iterator f = this->find(i);
		if (f == this->end()) return dummy;
		return f->second;
	}
 };



/*************************************************************************************
 * INLINE FUNCTION IMPLEMENTATIONS
 *************************************************************************************/



inline float&	DEMGeo::operator()(int x, int y)
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight)
		Assert(!"ERROR: ASSIGN OUTSIDE BOUNDS!");
	return mData[x + y * mWidth];
}

inline float	DEMGeo::operator()(int x, int y) const
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) return DEM_NO_DATA;
	return mData[x + y * mWidth];
}

inline float	DEMGeo::get(int x, int y) const
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) return DEM_NO_DATA;
	return mData[x + y * mWidth];
}

inline void	DEMGeo::set(int x, int y, float v)
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) return;
	mData[x + y * mWidth] = v;
}

inline float	DEMGeo::get_clamp(int x, int y) const
{
	if (x < 0) x = 0;
	if (x > (mWidth-1)) x = mWidth-1;
	if (y < 0) y = 0;
	if (y > (mHeight-1)) y = mHeight-1;
	return mData[x + y * mWidth];
}

inline float	DEMGeo::get_dir(int x, int y, int dx, int dy, int max, float blank, float& outDist) const
{
	outDist = 0;
	for (int i = 0; i < max; ++i)
	{
		x += dx;
		y += dy;
		outDist++;
		if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) return blank;
		float h = get(x, y);
		if (h != blank)
			return h;
	} 
	return DEM_NO_DATA;
}

inline float	DEMGeo::get_radial(int x, int y, int max, float blank) const
{
	float h;
	h = get(x,y); if (h != blank) return h;
	for (int n = 1; n <= max; ++n)
	{	
		h = get_clamp(x+n,y  );	if (h != blank) return h;
		h = get_clamp(x-n,y  );	if (h != blank) return h;
		h = get_clamp(x  ,y+n);	if (h != blank) return h;
		h = get_clamp(x  ,y-n);	if (h != blank) return h;
		h = get_clamp(x+n,y+n);	if (h != blank) return h;
		h = get_clamp(x-n,y+n);	if (h != blank) return h;
		h = get_clamp(x+n,y-n);	if (h != blank) return h;
		h = get_clamp(x-n,y-n);	if (h != blank) return h;
	}
	return blank;
}

inline int	DEMGeo::radial_dist(int x, int y, int max, float key) const
{
	float h;
	h = get(x,y); if (h == key) return 0;
	for (int n = 1; n <= max; ++n)
	{	
		h = get(x+n,y  );	if (h == key) return n;
		h = get(x-n,y  );	if (h == key) return n;
		h = get(x  ,y+n);	if (h == key) return n;
		h = get(x  ,y-n);	if (h == key) return n;
		h = get(x+n,y+n);	if (h == key) return n;
		h = get(x-n,y+n);	if (h == key) return n;
		h = get(x+n,y-n);	if (h == key) return n;
		h = get(x-n,y-n);	if (h == key) return n;
	}
	return -1;
}

inline float	DEMGeo::get_lowest_heuristic(int x, int y, int r) const
{	
	int xo, yo, 
			x1 = x-r, 
			x2 = x+r, 
			y1 = y-r, 
			y2 = y+r;
		
	vector<float>	es;
	float real = get(x,y);
	es.reserve((r+1)*(r+1));
	for (yo = y1; yo <= y2; ++yo)
	for (xo = x1; xo <= x2; ++xo)
	{
		float e = get(xo,yo);
		if (e != DEM_NO_DATA)
			es.push_back(e);
	}
	if (es.empty()) return DEM_NO_DATA;
	sort(es.begin(), es.end());

	if(es.size() < 9) return es[0];
	return es[3];
}



inline float	DEMGeo::get_lowest(int x, int y, int r) const
{
	float e = get(x,y);
	for (int n = 1; n < r; ++n)
	{
		e = MIN_NODATA(e, get(x-n,y+n));
		e = MIN_NODATA(e, get(x-n,y-n));
		e = MIN_NODATA(e, get(x+n,y-n));
		e = MIN_NODATA(e, get(x+n,y+n));
		e = MIN_NODATA(e, get(x-n,y  ));
		e = MIN_NODATA(e, get(x+n,y  ));
		e = MIN_NODATA(e, get(x  ,y-n));
		e = MIN_NODATA(e, get(x  ,y+n));
	}
	return e;
}

inline float	DEMGeo::get_lowest(int x, int y, int r, int& xo, int& yo) const
{
	xo = x;
	yo = y;
	float e = get(x,y);
	for (int n = 1; n < r; ++n)
	{
		e = MIN_NODATA_XY(e, get(x-n,y+n), xo, yo, x-n,y+n);
		e = MIN_NODATA_XY(e, get(x-n,y-n), xo, yo, x-n,y-n);
		e = MIN_NODATA_XY(e, get(x+n,y-n), xo, yo, x+n,y-n);
		e = MIN_NODATA_XY(e, get(x+n,y+n), xo, yo, x+n,y+n);
		e = MIN_NODATA_XY(e, get(x-n,y  ), xo, yo, x-n,y  );
		e = MIN_NODATA_XY(e, get(x+n,y  ), xo, yo, x+n,y  );
		e = MIN_NODATA_XY(e, get(x  ,y-n), xo, yo, x  ,y-n);
		e = MIN_NODATA_XY(e, get(x  ,y+n), xo, yo, x  ,y+n);
	}
	return e;
}

inline float	DEMGeo::kernelN(int x, int y, int dim, float * kernel) const 
{
	float	sum, e;
	sum = DEM_NO_DATA;
	int i = 0;
	int hdim = dim / 2;
	for (int dx = -hdim; dx <= hdim; ++dx)
	for (int dy = -hdim; dy <= hdim; ++dy)
	{
		e = get_clamp(x+dx,y+dy);
		if (e != DEM_NO_DATA)
		{
			e *= kernel[i];
			if (sum == DEM_NO_DATA) 
				sum = e;
			else 
				sum += e;
		}		
		++i;
	}
	return sum;
}

inline float	DEMGeo::kernelmaxN(int x, int y, int dim, float * kernel) const 
{
	float	sum, e;
	sum = DEM_NO_DATA;
	int i = 0;
	int hdim = dim / 2;
	for (int dx = -hdim; dx <= hdim; ++dx)
	for (int dy = -hdim; dy <= hdim; ++dy)
	{
		e = get_clamp(x+dx,y+dy);
		if (e != DEM_NO_DATA)
		{
			e *= kernel[i];
			if (sum == DEM_NO_DATA) 
				sum = e;
			else 
				sum = max(sum, e);
		}		
		++i;
	}
	return sum;
}

inline float	DEMGeo::kernelN_Normalize(int x, int y, int dim, float * kernel) const 
{
	float	sum, e;
	sum = DEM_NO_DATA;
	float t = 0.0;
	int i = 0;
	int hdim = dim / 2;
	for (int dx = -hdim; dx <= hdim; ++dx)
	for (int dy = -hdim; dy <= hdim; ++dy)
	{
		e = get_clamp(x+dx,y+dy);
		if (e != DEM_NO_DATA)
		{
			e *= kernel[i];
			t += kernel[i];
			if (sum == DEM_NO_DATA) 
				sum = e;
			else 
				sum += e;
		}		
		++i;
	}
	return (t == 0.0) ? DEM_NO_DATA : sum / t;
}


inline void	DEMGeo::zap(int x, int y)
{
	mData[x + y * mWidth] = DEM_NO_DATA;
}

inline float	DEMGeo::value_linear(double lon, double lat) const
{
	if (lon < mWest || lon > mEast || lat < mSouth || lat > mNorth) return DEM_NO_DATA;
	double x_fract = (lon - mWest) / (mEast - mWest);
	double y_fract = (lat - mSouth) / (mNorth - mSouth);
	
	x_fract *= (double) (mWidth-1);
	y_fract *= (double) (mHeight-1);
	
	int x = x_fract;
	int y = y_fract;
	x_fract -= (double) x;
	y_fract -= (double) y;
	
	float	v1, v2, v3, v4;
	
	if (x_fract == 0.0)
	{
		if (y_fract == 0.0)
		{
			return  get(x    , y    );
		} else {
			v1 = get(x    , y    );
			v2 = get(x    , y + 1);
			if (v1 == DEM_NO_DATA || v2 == DEM_NO_DATA) return DEM_NO_DATA;
			return  v1 * (1.0 - y_fract) +
					v2 * (      y_fract);
		}
	} else {
		if (y_fract == 0.0)
		{
			v1 = get(x    , y    );
			v2 = get(x + 1, y    );
			if (v1 == DEM_NO_DATA || v2 == DEM_NO_DATA) return DEM_NO_DATA;
			return  v1 * (1.0 - x_fract) +
					v2 * (      x_fract);
		} else {
			v1 = get(x    , y    );
			v2 = get(x + 1, y    );
			v3 = get(x    , y + 1);
			v4 = get(x + 1, y + 1);
			if (v1 == DEM_NO_DATA || v2 == DEM_NO_DATA || v3 == DEM_NO_DATA || v4 == DEM_NO_DATA) return DEM_NO_DATA;
			return  v1 * (1.0 - x_fract) * (1.0 - y_fract) +
					v2 * (      x_fract) * (1.0 - y_fract) +
					v3 * (1.0 - x_fract) * (      y_fract) +
					v4 * (      x_fract) * (      y_fract);
		}
	}
}

inline void DEMGeo::zap_linear(double lon, double lat)
{
	if (lon < mWest || lon > mEast || lat < mSouth || lat > mNorth) return;
	double x_fract = (lon - mWest) / (mEast - mWest);
	double y_fract = (lat - mSouth) / (mNorth - mSouth);
	
	x_fract *= (double) (mWidth-1);
	y_fract *= (double) (mHeight-1);
	
	int x = x_fract;
	int y = y_fract;
	x_fract -= (double) x;
	y_fract -= (double) y;
	
	if (x_fract == 0.0)
	{
		if (y_fract == 0.0)
		{
			zap(x    , y    );
		} else {
			zap(x    , y    );
			zap(x    , y + 1);
		}
	} else {
		if (y_fract == 0.0)
		{
			zap(x    , y    );
			zap(x + 1, y    );
		} else {
			zap(x    , y    );
			zap(x + 1, y    );
			zap(x    , y + 1);
			zap(x + 1, y + 1);
		}
	}
}

inline float	DEMGeo::xy_nearest(double lon, double lat) const
{
	int dx, dy;
	return xy_nearest(lon, lat, dx, dy);
}

inline float	DEMGeo::xy_nearest(double lon, double lat, int& xo, int& yo) const
{
//	if (lon < mWest || lon > mEast || lat < mSouth || lat > mNorth) return DEM_NO_DATA;
	double x_fract = (lon - mWest) / (mEast - mWest);
	double y_fract = (lat - mSouth) / (mNorth - mSouth);
	
	x_fract *= (double) (mWidth-1);
	y_fract *= (double) (mHeight-1);
	int x = x_fract;
	int y = y_fract;
	float e1, e2, e3, e4;
	e1 = get(x,y);
	e2 = get(x+1,y);
	e3 = get(x,y+1);
	e4 = get(x+1,y+1);
	
	x_fract -= x;
	y_fract -= y;
	if (x_fract > 0.5)
	{
		if (y_fract > 0.5)
		{	// x+1,y+1
				 if (e4 != DEM_NO_DATA) { xo = x+1; yo = y+1; return e4; }
			else if (e3 != DEM_NO_DATA) { xo = x  ; yo = y+1; return e3; }
			else if (e2 != DEM_NO_DATA) { xo = x+1; yo = y  ; return e2; }
			else if (e1 != DEM_NO_DATA)	{ xo = x  ; yo = y  ; return e1; }
			else 										  return DEM_NO_DATA;
		} else { // x+1,y
				 if (e2 != DEM_NO_DATA) { xo = x+1; yo = y  ; return e2; }
			else if (e4 != DEM_NO_DATA) { xo = x+1; yo = y+1; return e4; }
			else if (e1 != DEM_NO_DATA) { xo = x  ; yo = y  ; return e1; }
			else if (e3 != DEM_NO_DATA) { xo = x  ; yo = y+1; return e3; }
			else										  return DEM_NO_DATA;
		}
	} else {
		if (y_fract > 0.5)
		{	// x,y+1
				 if (e3 != DEM_NO_DATA) { xo = x  ; yo = y+1; return e3; }
			else if (e4 != DEM_NO_DATA) { xo = x+1; yo = y+1; return e4; }
			else if (e1 != DEM_NO_DATA) { xo = x  ; yo = y  ; return e1; }
			else if (e2 != DEM_NO_DATA) { xo = x+1; yo = y  ; return e2; }
			else										  return DEM_NO_DATA;
		} else { // x,y
				 if (e1 != DEM_NO_DATA) { xo = x  ; yo = y  ; return e1; }
			else if (e2 != DEM_NO_DATA) { xo = x+1; yo = y  ; return e2; }
			else if (e3 != DEM_NO_DATA) { xo = x  ; yo = y+1; return e3; }
			else if (e4 != DEM_NO_DATA) { xo = x+1; yo = y+1; return e4; }
			else										  return DEM_NO_DATA;
		}
	}
}

inline float	DEMGeo::search_nearest(double lon, double lat) const
{
	if (lon < mWest || lon > mEast || lat < mSouth || lat > mNorth) return DEM_NO_DATA;
	double x_fract = (lon - mWest) / (mEast - mWest);
	double y_fract = (lat - mSouth) / (mNorth - mSouth);
	
	x_fract *= (double) (mWidth-1);
	y_fract *= (double) (mHeight-1);
	int x = x_fract;
	int y = y_fract;
	float h;

	h = get_clamp(x,y); if (h != DEM_NO_DATA) return h;
	
	int r = 1;
	while (r < mWidth && r < mHeight)
	{
		h = get_clamp(x-r,y  );	if (h != DEM_NO_DATA) return h;
		h = get_clamp(x+r,y  );	if (h != DEM_NO_DATA) return h;
		h = get_clamp(x  ,y+r);	if (h != DEM_NO_DATA) return h;
		h = get_clamp(x  ,y-r);	if (h != DEM_NO_DATA) return h;
		h = get_clamp(x-r,y-r);	if (h != DEM_NO_DATA) return h;
		h = get_clamp(x+r,y-r);	if (h != DEM_NO_DATA) return h;
		h = get_clamp(x+r,y+r);	if (h != DEM_NO_DATA) return h;
		h = get_clamp(x-r,y+r);	if (h != DEM_NO_DATA) return h;
		++r;
	}
	return DEM_NO_DATA;
}

inline double	DEMGeo::x_to_lon(int inX) const
{
	return mWest + ((double) inX  * (mEast - mWest) / (double) (mWidth-1));
}

inline double	DEMGeo::y_to_lat(int inY) const
{
	return mSouth + ((double) inY * (mNorth - mSouth) / (double) (mHeight-1));
}

inline double	DEMGeo::x_to_lon_double(double inX) const
{
	return mWest + ((double) inX  * (mEast - mWest) / (double) (mWidth-1));
}

inline double	DEMGeo::y_to_lat_double(double inY) const
{
	return mSouth + ((double) inY * (mNorth - mSouth) / (double) (mHeight-1));
}

inline double	DEMGeo::lon_to_x(double inLon) const
{
	return (inLon - mWest) * (double) (mWidth-1) / (mEast - mWest);
}

inline double	DEMGeo::lat_to_y(double inLat) const
{
	return (inLat - mSouth) * (double) (mHeight-1) / (mNorth - mSouth);
}

inline int		DEMGeo::x_lower(double lon) const
{
	if (lon <= mWest) return 0;
	if (lon >= mEast) return mWidth-1;
	
	lon -= mWest;
	lon *= (mWidth-1);
	lon /= (mEast - mWest);
	return floor(lon);
}

inline int		DEMGeo::x_upper(double lon) const
{
	if (lon <= mWest) return 0;
	if (lon >= mEast) return mWidth-1;

	lon -= mWest;
	lon *= (mWidth-1);
	lon /= (mEast - mWest);
	return ceil(lon);
}

inline int		DEMGeo::y_lower(double lat) const
{
	if (lat <= mSouth) return 0;
	if (lat >= mNorth) return mHeight-1;

	lat -= mSouth;
	lat *= (mHeight-1);
	lat /= (mNorth - mSouth);
	return floor(lat);
}

inline int		DEMGeo::y_upper(double lat) const
{
	if (lat <= mSouth) return 0;
	if (lat >= mNorth) return mHeight-1;

	lat -= mSouth;
	lat *= (mHeight-1);
	lat /= (mNorth - mSouth);
	return ceil(lat);
}

inline double	DEMGeo::x_dist_to_m(int inX) const
{
	double	d = (double) inX / (double) mWidth;
	d *= (mEast - mWest);
	d *= (DEG_TO_MTR_LAT * cos((mSouth+mNorth) * 0.5 * DEG_TO_RAD));
	return d;
}

inline double	DEMGeo::y_dist_to_m(int inY) const
{
	double	d = (double) inY / (double) mHeight;
	d *= (mNorth - mSouth);
	d *= (DEG_TO_MTR_LAT);
	return d;
}


#endif

