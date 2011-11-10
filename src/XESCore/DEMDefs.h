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

	DEMGeos can run in either "post" mode (mPost=1) where each pixel sits 
	use center-pixel notation, meaning the edge pixels sit _on_ the edges of the DEM.
	(This was the default and only option for RF for years and matches the SRMT data.)
	If post = 0 the pixels are area-centric - they sit _inside_ the bounds and their
	sample center is 0.5 pixels in from the edge.
	
	The lower left pixel is the data that either sits on the southwest corner (post)
	or the pixel whose lower left corner touches the sw corner (area).  Therefore
	if we have 1200 samples per degree, the post-mode DEM will contain 1201 samples and
	two adjacent DEMs will duplicate data by sharing their common edges.  The area 
	mode DEM will have 1200 samples and not ovrelap at all.

	While DEM data are floating point, we sometimes stuff integer enumeration
	coded data into them.

	One or more DEMGeos are used in XES files to represent elevation, land type,
	and other continuous parameters.
	
	ITERATION AND ACCESS
	
	The DEM supports four kinds of access:
	
	-	Iterators refer to a specific pixel of a specific object, and are implemented
		via pointers; they are unstable when the DEM is resized.
	-	Addresses refer to the numeric address of a pixel within the DEM in one
		dimension; the advantage of addresses is that they provide single-word access
		but aren't tied to a specific DEM - given two DEMs of the same dimensions,
		overlapped pixels have the same address.
	-	Coordinates - 2-d pair of ints - the STL-pair form is rarely used in the API but
		is useful for internal calculations.
	-	Neighbor iterators.  A neighbor iterator iterates over zero to four/eight
		orthogonally connected pixels from a given address; the neighbor iterates
		on addresses.  Template with 4 for orthogonal connection or 8 for diagonal
		conncetion.
		
	

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

	/****************************************************************************
	 * RAW DATA MEMBERS
	 ****************************************************************************/

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
	int		mPost;		// If 1, pixels sit "on" grid-lines, e.g. 1201 samples on a 90m DEM. If 0, a pixel is an _area_ between grid-lines.

	// An array of width*height data points in floating point format.
	// The first sample is the southwest corner, we then proceed east.
	float *	mData;

	inline	float	pixel_offset() const { return mPost ? 0.0 : 0.5; }	// distance from the coordinate defining a pixel to its sampling center.
	inline	int		pixel_area() const { return mWidth * mHeight; }
	
	/****************************************************************************
	 * PIXEL ACCESS
	 ****************************************************************************/	

	typedef float *			iterator;
	typedef	const float *	const_iterator;
	typedef int				address;
	typedef	pair<int,int>	coordinates;

	template<int __dim>
	struct					neighbor_iterator;

	iterator		begin(void) { return mData; }
	iterator		end(void) { return mData + mWidth * mHeight; }
	const_iterator	begin(void) const { return mData; }
	const_iterator	end(void) const { return mData + mWidth * mHeight; }
	address			address_begin(void) const;
	address			address_end(void) const;
	
	template<int __dim>
	neighbor_iterator<__dim> neighbor_begin(address a);
	template<int __dim>
	neighbor_iterator<__dim> neighbor_end(address a);

	bool			valid(address a) const;
	bool			valid(iterator i) const;
	bool			valid(const coordinates& c) const;
	
	inline float&	operator()(int, int);						// Direct member access; individual coordinates must use operator() because no two-dim [] in C
	inline float	operator()(int, int) const;
	float&			operator[](const coordinates& C);
	float&			operator[](address a);
	float			operator[](const coordinates& C) const;
	float			operator[](address a) const;
	
	address			to_address(const_iterator i) const;			// Coordinate coersion.
	address			to_address(const coordinates& c) const;
	coordinates		to_coordinates(const_iterator i) const;
	coordinates		to_coordinates(address a) const;
	const_iterator	to_iterator(const coordinates& c) const;
	const_iterator	to_iterator(address a) const;
	iterator		to_iterator(const coordinates& c);
	iterator		to_iterator(address a);

	/****************************************************************************
	 * WHOLE-DEM OPS
	 ****************************************************************************/	

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

	void	resize(int width, int height);					// Resize, reset to 0
	void	set_rez(double x_res, double y_res);			// Given target res and geo already set, recalc dims and resize.
	void	resize_save(int width, int height, float fill);	// REsize, save lower left data,  fill with param
	void	copy_geo_from(const DEMGeo& rhs);				// geo-Coordinates from other
	void	clear_from(const DEMGeo&, float v);				// Copy/clear: size and coordinates and post from other, but filled with 'v'
	void	clear_from(const DEMGeo&		 );				// Copy/no-clear: size and coordinates and post from other, but UNDEFINED contents!

	void	derez(int n);									// Reduce size by a factor of "n".  Averages down (box filter).
	void	derez_nearest(void);							// Derez by a factor of two with nearest-neighbor down-sampling....good for enums.
	void	derez_nearest(DEMGeo& smaller);					// Derez by a factor of two with nearest-neighbor down-sampling....good for enums.
	void	overlay(const DEMGeo& onTop);					// Overlay - requires 1:1 layout
	void	overlay(const DEMGeo& onTop, int dx, int dy);	// Overlay - requires onTop <= main

	void	subset(DEMGeo& newDEM, int x1, int y1, int x2, int y2) const;					// INCLUSIVE for post, EXCLUSIVE for area.
	void	swap(DEMGeo& otherDEM);															// Swap all params, good for avoiding mem copies

	/****************************************************************************
	 * FILTER FUNCTIONS AND SPECIALIZED PIXEL ACCESS
	 ****************************************************************************/	

	// Access to the data points by discrete address
	inline float	get(int x, int y) const;									// Get value at x,y, DEM_NO_DATA if out of bonds
	inline void		set(int x, int y, float v);									// Safe set - no-op if off
	inline float	get_clamp(int x, int y) const;								// Get value at x,y, clamped to within the DEM
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

	inline void		zap(int x, int y);												// Set x,y to DEM_NO_DATA

	// Lat/lon access and erasing
	inline float	value_linear(double lon, double lat) const;					// Get linear interp value of coordinate
//	inline void		zap_linear(double lon, double lat);							// Zap all values that are near the point (up to 2x2)
	inline float	xy_nearest(double lon, double lat				 ) const;	// Get nearest-neighbor value (nearest non-void)
	inline float	xy_nearest(double lon, double lat, int& x, int& y) const;	// Get nearest-neighbor value, (nearest non-void), return coordinate used
	inline float	xy_nearest_raw(double lon, double lat            ) const;	// Get nearest-neighbor value void ok
	inline float	search_nearest(double lon, double lat) const;				// Get nearest-neighbor value, search indefinitely

	/****************************************************************************
	 * GEOCODING FUNCTIONS
	 ****************************************************************************/	

	// These routines convert grid positions to lat/lon.  NOTE: x/y positions are based on pixel _centroids, _not_ the lower left
	// corner.  Thus for an area-sampled file, the XY of the SW corner is usually -0.5,-0.5.
	inline double	x_to_lon(int inX) const;
	inline double	y_to_lat(int inY) const;
	inline double	x_to_lon_double(double inX) const;
	inline double	y_to_lat_double(double inY) const;
	inline double	lon_to_x(double inLon) const;
	inline double	lat_to_y(double inLat) const;
	inline double	x_dist_to_m(double inX) const;
	inline double	y_dist_to_m(double inY) const;
	
	inline double	x_res(void) const;
	inline double	y_res(void) const;
	
	inline int		map_x_from(const DEMGeo& src, int x) const;
	inline int		map_y_from(const DEMGeo& src, int y) const;

	// These routines return the grid point below or above a coordinate constrained
	// to within the grid.
	inline int		x_lower(double lon) const;
	inline int		x_upper(double lon) const;
	inline int		y_lower(double lat) const;
	inline int		y_upper(double lat) const;

	/****************************************************************************
	 * ADVANCED ROUTINES
	 ****************************************************************************/	


			void	calc_slope(DEMGeo& outSlope, DEMGeo& outHeading, ProgressFunc inFunc) const;
			void	calc_normal(DEMGeo& outX, DEMGeo& outY, DEMGeo& outZ, ProgressFunc inFunc) const;
			void	fill_nearest(void);
			int		remove_linear(int iterations, float max_err);
			float	local_minmax(int x1, int y1, int x2, int y2,
								 int& minx, int& miny, float& minh,
								 int& maxx, int& maxy, float& maxh);

			void	filter_self(int dim, float * k);
			void	filter_self_normalize(int dim, float * k);

	inline	float	gradient_x(int x, int y) const;					// These return exact gradients at HALF-POSTINGS!
	inline	float	gradient_y(int x, int y) const;					// So for a 1201 DEM there are 1200 gradients.  This is really utility funcs
	inline	float	gradient_x_bilinear(float x, float y) const;	// These return interp'd gradients on WHOLE POSTINGS, which is what we REALLY
	inline	float	gradient_y_bilinear(float x, float y) const;	// want...remember that we can't get an exact discrete gradient on the post (duh).

	/****************************************************************************
	 * NEIGHBOR-ITERATOR
	 ****************************************************************************/	

	template<int __dim>
	struct neighbor_iterator {
		neighbor_iterator();
		neighbor_iterator(const neighbor_iterator& rhs);
		neighbor_iterator& operator=(const neighbor_iterator& rhs);
		bool operator==(const neighbor_iterator& rhs) const;
		bool operator!=(const neighbor_iterator& rhs) const;		
		bool operator()(void) const;
		
		address operator * (void);
		neighbor_iterator operator++(int);
		neighbor_iterator& operator++(void);
	private:
		friend class DEMGeo;
		neighbor_iterator(const DEMGeo * d, address a);
		const DEMGeo *			parent;
		DEMGeo::coordinates		coords;
		int						step;
		void	inc(void);
	};
	
};

/*************************************************************************************
 * DEM MASK
 *************************************************************************************/

// Same idea, except we use a bit-mask.  Use STL vector to get the bit mask.  32x memory savings compared to a DEM.
struct	DEMMask {

	DEMMask();
	DEMMask(int w, int h, bool ini);
	DEMMask(const DEMGeo&);

	DEMMask& operator=(bool);			// Fill
	DEMMask& operator=(const DEMGeo&);	// Copy
	DEMMask& operator=(const DEMMask&);	// Copy

	void	resize(int width, int height, bool ini);		// Resize, reset to X
	void	copy_geo_from(const DEMGeo& rhs);
	void	copy_geo_from(const DEMMask& rhs);

	inline bool		operator()(int, int) const;
	inline bool		get(int x, int y) const;									// Get value at x,y, false
	inline void		set(int x, int y, bool v);									// Safe set - no-op if off

	double	mWest;
	double	mSouth;
	double	mEast;
	double	mNorth;

	int		mWidth;
	int		mHeight;
	int		mPost;

	vector<bool>	mData;
};

/*************************************************************************************
 * FREE LOW-LEVEL DEM PROCESSING FUNCS
 *************************************************************************************/

void		dem_coverage_nearest(const DEMGeo& d, double lon1, double lat1, double lon2, double lat2, int bounds[4]);

// IMPORTANT: the original values must ALL be filled in in orig_src - io_dst should have the voids!
void		dem_copy_buffer_one(const DEMGeo& orig_src, DEMGeo& io_dst, float null_value);
void		dem_erode(DEMGeo& io_dem, int steps, float null_value);

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
 * DEM address FIFO
 *************************************************************************************/

// Some raster algorithms use a FIFO of pixels to process - this is about the simplest
// FIFO we can build - a fixed capacity ring buffer built around a vector.
 
struct address_fifo {
public:
	typedef DEMGeo::address address;
	address_fifo(int capacity) { inp_ = outp_ = size_ = 0; data_.resize(capacity); }

	bool empty(void) const { return size_ == 0; }
	size_t size(void) const { return size_; }
	
	void push(address n) { DebugAssert(size_ < data_.size()); data_[inp_] = n; inp_ = (inp_+1) % data_.size(); ++size_; }
	address pop(void) { DebugAssert(size_ > 0); address r = data_[outp_]; outp_ = (outp_+1) % data_.size(); --size_; return r; }

	vector<address> data_;
	size_t size_;
	size_t inp_;
	size_t outp_;	
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

	x_fract *= (double) (mWidth-mPost);
	y_fract *= (double) (mHeight-mPost);

	x_fract -= pixel_offset();
	y_fract -= pixel_offset();

	int x = x_fract;
	int y = y_fract;
	x_fract -= (double) x;
	y_fract -= (double) y;

	float v1 = get(x    , y    );
	float v2 = get(x + 1, y    );
	float v3 = get(x    , y + 1);
	float v4 = get(x + 1, y + 1);
	float w1 = (1.0 - x_fract) * (1.0 - y_fract);
	float w2 = (      x_fract) * (1.0 - y_fract);
	float w3 = (1.0 - x_fract) * (      y_fract);
	float w4 = (      x_fract) * (      y_fract);

	if(v1 == DEM_NO_DATA) w1 = 0.0;
	if(v2 == DEM_NO_DATA) w2 = 0.0;
	if(v3 == DEM_NO_DATA) w3 = 0.0;
	if(v4 == DEM_NO_DATA) w4 = 0.0;

	float w = w1 + w2 + w3 + w4;

	if (w == 0.0) return DEM_NO_DATA;

	return (v1 * w1 + v2 * w2 + v3 * w3 + v4 * w4) / w;
}

/*
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
*/

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

	x_fract *= (double) (mWidth-mPost);
	y_fract *= (double) (mHeight-mPost);
	x_fract -= pixel_offset();
	y_fract -= pixel_offset();
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

inline float	DEMGeo::xy_nearest_raw(double lon, double lat) const
{
//	if (lon < mWest || lon > mEast || lat < mSouth || lat > mNorth) return DEM_NO_DATA;
	double x_fract = (lon - mWest) / (mEast - mWest);
	double y_fract = (lat - mSouth) / (mNorth - mSouth);

	x_fract *= (double) (mWidth-mPost);
	y_fract *= (double) (mHeight-mPost);
	x_fract -= pixel_offset();
	y_fract -= pixel_offset();
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
			return e4;
		else
			return e2;
	} else {
		if (y_fract > 0.5)
			return e3;
		else
			return e1;
	}
}


inline float	DEMGeo::search_nearest(double lon, double lat) const
{
	if (lon < mWest || lon > mEast || lat < mSouth || lat > mNorth) return DEM_NO_DATA;
	double x_fract = (lon - mWest) / (mEast - mWest);
	double y_fract = (lat - mSouth) / (mNorth - mSouth);

	x_fract *= (double) (mWidth-mPost);
	y_fract *= (double) (mHeight-mPost);
	int x = (x_fract - pixel_offset() + 0.5);		// Since this is doing a "floor" and not a divide at the half point we must add 0.5 for the 
	int y = (y_fract - pixel_offset() + 0.5);		// on-post sampling case!
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
	return mWest + (((double) inX + pixel_offset())  * (mEast - mWest) / (double) (mWidth-mPost));
}

inline double	DEMGeo::y_to_lat(int inY) const
{
	return mSouth + (((double) inY + pixel_offset()) * (mNorth - mSouth) / (double) (mHeight-mPost));
}

inline double	DEMGeo::x_to_lon_double(double inX) const
{
	return mWest + ((inX + pixel_offset()) * (mEast - mWest) / (double) (mWidth-mPost));
}

inline double	DEMGeo::y_to_lat_double(double inY) const
{
	return mSouth + ((inY + pixel_offset()) * (mNorth - mSouth) / (double) (mHeight-mPost));
}

inline double	DEMGeo::lon_to_x(double inLon) const
{
	return (inLon - mWest) * (double) (mWidth-mPost) / (mEast - mWest) - pixel_offset();
}

inline double	DEMGeo::lat_to_y(double inLat) const
{
	return (inLat - mSouth) * (double) (mHeight-mPost) / (mNorth - mSouth) - pixel_offset();
}

inline int		DEMGeo::x_lower(double lon) const
{
	if (lon <= mWest) return 0;
	if (lon >= mEast) return mWidth-mPost;

	lon -= mWest;
	lon *= (mWidth-mPost);
	lon /= (mEast - mWest);
	return floor(lon);
}

inline int		DEMGeo::x_upper(double lon) const
{
	if (lon <= mWest) return 0;
	if (lon >= mEast) return mWidth-mPost;

	lon -= mWest;
	lon *= (mWidth-mPost);
	lon /= (mEast - mWest);
	return ceil(lon);
}

inline int		DEMGeo::y_lower(double lat) const
{
	if (lat <= mSouth) return 0;
	if (lat >= mNorth) return mHeight-mPost;

	lat -= mSouth;
	lat *= (mHeight-mPost);
	lat /= (mNorth - mSouth);
	return floor(lat);
}

inline int		DEMGeo::y_upper(double lat) const
{
	if (lat <= mSouth) return 0;
	if (lat >= mNorth) return mHeight-mPost;

	lat -= mSouth;
	lat *= (mHeight-mPost);
	lat /= (mNorth - mSouth);
	return ceil(lat);
}

inline double	DEMGeo::x_dist_to_m(double inX) const
{
	double	d = inX / (double) (mWidth - mPost);
	d *= (mEast - mWest);
	d *= (DEG_TO_MTR_LAT * cos((mSouth+mNorth) * 0.5 * DEG_TO_RAD));
	return d;
}

inline double	DEMGeo::y_dist_to_m(double inY) const
{
	double	d = inY / (double) (mHeight - mPost);
	d *= (mNorth - mSouth);
	d *= (DEG_TO_MTR_LAT);
	return d;
}

inline double	DEMGeo::x_res(void) const
{
	return (mWidth - mPost) / (mEast - mWest);
}

inline double	DEMGeo::y_res(void) const
{
	return (mHeight - mPost) / (mNorth - mSouth); 
}


inline int	DEMGeo::map_x_from(const DEMGeo& src, int x) const
{
	return round(lon_to_x(src.x_to_lon_double(x)));
}

inline int	DEMGeo::map_y_from(const DEMGeo& src, int y) const
{
	return round(lat_to_y(src.y_to_lat_double(y)));
}

inline float	DEMGeo::gradient_x(int x, int y) const
{
	float h1 = this->get(x,y);
	float h2 = this->get(x+1,y);
	if(h1 == DEM_NO_DATA || h2 == DEM_NO_DATA) return DEM_NO_DATA;
	return (h2-h1) / x_dist_to_m(1);
}

inline float	DEMGeo::gradient_y(int x, int y) const
{
	float h1 = this->get(x,y);
	float h2 = this->get(x,y+1);
	if(h1 == DEM_NO_DATA || h2 == DEM_NO_DATA) return DEM_NO_DATA;
	return (h2-h1) / y_dist_to_m(1);
}

inline float	DEMGeo::gradient_x_bilinear(float x, float y) const
{
	x -= 0.5f;
	y -= 0.5f;

	float x1 = floor(x);
	float x2 = x1 + 1.0f;
	float xb = x - x1;
	float y1 = floor(y);
	float y2 = y1 + 1.0f;
	float yb = y - y1;

	float g11 = gradient_x(x1,y1);
	float g12 = gradient_x(x1,y2);
	float g21 = gradient_x(x2,y1);
	float g22 = gradient_x(x2,y2);

	float w11 = (1.0f - xb) * (1.0 - yb);
	float w12 = (1.0f - xb) * (		 yb);
	float w21 = (		xb) * (1.0 - yb);
	float w22 = (		xb) * (		 yb);
	if(g11 == DEM_NO_DATA) w11 = 0.0f;
	if(g12 == DEM_NO_DATA) w12 = 0.0f;
	if(g21 == DEM_NO_DATA) w21 = 0.0f;
	if(g22 == DEM_NO_DATA) w22 = 0.0f;

	float w = w11 + w12 + w21 + w22;
	if (w == 0.0f) return DEM_NO_DATA;
	w = 1.0f / w;
	w11 *= w;
	w12 *= w;
	w21 *= w;
	w22 *= w;

	return g11 * w11 + g21 * w21 + g12 * w12 + g22 * w22;
}

inline float	DEMGeo::gradient_y_bilinear(float x, float y) const
{
	x -= 0.5f;
	y -= 0.5f;

	float x1 = floor(x);
	float x2 = x1 + 1.0f;
	float xb = x - x1;
	float y1 = floor(y);
	float y2 = y1 + 1.0f;
	float yb = y - y1;

	float g11 = gradient_y(x1,y1);
	float g12 = gradient_y(x1,y2);
	float g21 = gradient_y(x2,y1);
	float g22 = gradient_y(x2,y2);

	float w11 = (1.0f - xb) * (1.0 - yb);
	float w12 = (1.0f - xb) * (		 yb);
	float w21 = (		xb) * (1.0 - yb);
	float w22 = (		xb) * (		 yb);
	if(g11 == DEM_NO_DATA) w11 = 0.0f;
	if(g12 == DEM_NO_DATA) w12 = 0.0f;
	if(g21 == DEM_NO_DATA) w21 = 0.0f;
	if(g22 == DEM_NO_DATA) w22 = 0.0f;

	float w = w11 + w12 + w21 + w22;
	if (w == 0.0f) return DEM_NO_DATA;
	w = 1.0f / w;
	w11 *= w;
	w12 *= w;
	w21 *= w;
	w22 *= w;

	return g11 * w11 + g21 * w21 + g12 * w12 + g22 * w22;
}


// STL doesn't do this - bvector is optimized!
/*
inline bool&	DEMMask::operator()(int x, int y)
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight)
		Assert(!"ERROR: ASSIGN OUTSIDE BOUNDS!");
	return mData[x + y * mWidth];
}
*/

inline bool	DEMMask::operator()(int x, int y) const
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) return DEM_NO_DATA;
	return mData[x + y * mWidth];
}

inline bool	DEMMask::get(int x, int y) const
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) return DEM_NO_DATA;
	return mData[x + y * mWidth];
}

inline void	DEMMask::set(int x, int y, bool v)
{
	if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) return;
	mData[x + y * mWidth] = v;
}




inline bool	DEMGeo::valid(address a) const
{
	return a >= address_begin() && a < address_end();
}
inline bool	DEMGeo::valid(iterator i) const
{
	return i >= begin() && i < end();
}

inline bool	DEMGeo::valid(const coordinates& c) const
{
	return c.first >= 0 && c.first < mWidth && c.second >= 0 && c.second < mHeight;
}
	
inline float& DEMGeo::operator[](const coordinates& c)
{
	return mData[c.first + c.second * mWidth];
}

inline float& DEMGeo::operator[](address a)
{
	return mData[a];
}

inline float DEMGeo::operator[](const coordinates& c) const
{
	return mData[c.first + c.second * mWidth];
}

inline float DEMGeo::operator[](address a) const
{
	return mData[a];
}

inline DEMGeo::address DEMGeo::address_begin(void) const
{
	return 0;
}

inline DEMGeo::address DEMGeo::address_end(void) const
{
	return mWidth * mHeight;
}
	
inline DEMGeo::address DEMGeo::to_address(const_iterator i) const
{
	return i - begin();
}

inline DEMGeo::address DEMGeo::to_address(const coordinates& c) const
{
	return c.first + c.second * mWidth;
}

inline DEMGeo::coordinates DEMGeo::to_coordinates(const_iterator i) const
{
	return to_coordinates(to_address(i));
}

inline DEMGeo::coordinates DEMGeo::to_coordinates(address a) const
{
	return pair<int,int>(a % mWidth, a / mWidth);
}

inline DEMGeo::const_iterator DEMGeo::to_iterator(const coordinates& c) const
{
	return to_iterator(to_address(c));
}

inline DEMGeo::iterator DEMGeo::to_iterator(const coordinates& c) 
{
	return to_iterator(to_address(c));
}

inline DEMGeo::iterator DEMGeo::to_iterator(address a)
{
	return begin() + a;
}

inline DEMGeo::const_iterator DEMGeo::to_iterator(address a) const
{
	return begin() + a;
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim>::neighbor_iterator() : parent(NULL), coords(DEMGeo::coordinates(0,0)), step(0)
{
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim>::neighbor_iterator(const neighbor_iterator& rhs) :
	parent(rhs.parent), coords(rhs.coords), step(rhs.step)
{
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim>& DEMGeo::neighbor_iterator<__dim>::operator=(const neighbor_iterator& rhs)
{
	parent = rhs.parent;
	coords = rhs.coords;
	step = rhs.step;
	return *this;
}

template<int __dim>
inline bool DEMGeo::neighbor_iterator<__dim>::operator==(const neighbor_iterator& rhs) const
{
	return parent == rhs.parent && coords == rhs.coords && step == rhs.step;
}

template<int __dim>
inline bool DEMGeo::neighbor_iterator<__dim>::operator!=(const neighbor_iterator& rhs) const
{
	return parent != rhs.parent || coords != rhs.coords || step != rhs.step;
}

template<int __dim>
inline bool DEMGeo::neighbor_iterator<__dim>::operator()(void) const
{
	return parent != NULL;
}

const int kStepsX[8] = { -1,  0, 1, 0,  1, 1, -1, -1 };
const int kStepsY[8] = {  0, -1, 0, 1, -1, 1, -1,  1 };

template<int __dim>
inline DEMGeo::address DEMGeo::neighbor_iterator<__dim>::operator * (void)
{
	return parent->to_address(DEMGeo::coordinates(coords.first+kStepsX[step],coords.second+kStepsY[step]));
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim> DEMGeo::neighbor_iterator<__dim>::operator++(int)
{
	DEMGeo::neighbor_iterator<__dim> ret(*this);
	++(*this);
	return ret;
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim>& DEMGeo::neighbor_iterator<__dim>::operator++(void)
{
	inc();
	return *this;
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim>::neighbor_iterator(const DEMGeo * d, address a) :
	parent(d), step(-1)
{
	coords = d->to_coordinates(a);
	inc();
	
}

template<int __dim>
inline void	DEMGeo::neighbor_iterator<__dim>::inc(void)
{
	while(parent)
	{
		++step;
		if(step >= __dim)
		{
			parent = NULL;
			coords = DEMGeo::coordinates(0,0);
			step = 0;
			return;
		}
		DEMGeo::coordinates dc(coords.first + kStepsX[step],coords.second + kStepsY[step]);
		if(parent->valid(dc))
			break;
	}
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim> DEMGeo::neighbor_begin(address i)
{
	return DEMGeo::neighbor_iterator<__dim>(this, i);
}

template<int __dim>
inline DEMGeo::neighbor_iterator<__dim> DEMGeo::neighbor_end(address i)
{
	return DEMGeo::neighbor_iterator<__dim>();
}



#endif

