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
#include "DEMDefs.h"
#include "CompGeomDefs3.h"

#define HIST_MAX	10

struct	HistoHelper {

	int		choices;
	int		best;
	float	values[HIST_MAX];
	int		counts[HIST_MAX];

			HistoHelper() : choices(0), best(-1) { }
	bool	HasBest(void) const { return best != -1; }
	float	GetBest(void) const { return (best == -1) ? NO_DATA : values[best]; }
	void	Accum(float v, float ignore) 
	{
		if (v == ignore) return;
		for (int n = 0; n < choices; ++n)
		if (values[n] == v)
		{
			counts[n]++;
			if (best == -1 || counts[n] > counts[best])
				best = n;
			return;
		}
		if (choices < HIST_MAX)
		{
			counts[choices] = 1;
			values[choices] = v;
			if (best == -1)
				best = choices;
			++choices;
		}
	}
};


/* DEM min/max trees are organized in powers of 2.  So a 8x8 point summary (describing the max of 64 DEM points
 * is only available on an even grid of 8 DEM points.  
 *
 * This routine tells you the largest DEM summary block that your point is a member of that is also fully 
 * contained in an arbitrary range.  You specify the max power of 2 to work with.
 *
 */
inline int		aligned_block(int x, int y, int x1, int x2, int y1, int y2, int& x_loc, int& y_loc, int max_power)
{
	int i;
	x_loc = x;
	y_loc = y;	
	for (i = 0; i < max_power; ++i)
	{
		int block_size = 1 << i;
		int xa = x;
		int ya = y;
		xa &= ~(block_size-1);
		ya &= ~(block_size-1);
		if (xa < x1 || (xa+block_size) > x2 ||
			ya < y1 || (ya+block_size) > y2)
		{
			return i-1;
		}
		x_loc = xa;
		y_loc = ya;
	}
	return i-1;
}


DEMGeo::DEMGeo() :
	mWest(0.0),
	mSouth(0.0),
	mNorth(0.0),
	mEast(0.0),
	mWidth(0),
	mHeight(0),
	mData(0)
{
}

DEMGeo::DEMGeo(const DEMGeo& x) : 
	mWest(x.mWest),
	mSouth(x.mSouth),
	mEast(x.mEast),
	mNorth(x.mNorth),
	mWidth(x.mWidth),
	mHeight(x.mHeight)
{
	if (mWidth == 0 || mHeight == 0)
	{
		mData = 0;
	} else {
		mData = (float *) malloc(mWidth * mHeight * sizeof(float));
		if (mData == NULL)
			mWidth = mHeight = 0;
		else {
			if (x.mData) 
				memcpy(mData, x.mData, mWidth * mHeight * sizeof(float));
			else
				memset(mData, 0, mWidth * mHeight * sizeof(float));
		}
	}
}

DEMGeo::DEMGeo(int width, int height) :
	mSouth(0.0), mNorth(0.0), mEast(0.0), mWest(0.0),
	mWidth(width), mHeight(height)
{
	if (mWidth == 0 || mHeight == 0)
	{
		mData = 0;
	} else {
		mData = (float *) malloc(mWidth * mHeight * sizeof(float));
		if (mData == NULL)
			mWidth = mHeight = 0;
		else
			memset(mData, 0, mWidth * mHeight * sizeof(float));	
	}
}

DEMGeo::~DEMGeo()
{
	if (mData) free(mData);
}

DEMGeo& DEMGeo::operator=(float v)
{
	int sz = mWidth * mHeight;
	float * p = mData;
	while (sz--)
		*p++ = v;
	return *this;
}

DEMGeo& DEMGeo::operator+=(float v)
{
	int sz = mWidth * mHeight;
	float * p = mData;
	while (sz--)
	{
		if (*p != NO_DATA)
			*p += v;
		++p;
	}
	return *this;
}

DEMGeo& DEMGeo::operator+=(const DEMGeo& rhs)
{
	if (rhs.mWidth != mWidth || rhs.mHeight != mHeight || mData == NULL || rhs.mData == NULL) 
		return *this;
	int sz = mWidth * mHeight;
	const float * src = rhs.mData;
	float * dst = mData;
	while (sz--)
	{
		if (*dst != NO_DATA && *src != NO_DATA)
			*dst += *src;
		++dst;
		++src;
	}
	return *this;	
}

DEMGeo& DEMGeo::operator*=(float v)
{
	int sz = mWidth * mHeight;
	float * p = mData;
	while (sz--)
	{
		if (*p != NO_DATA)
			*p *= v;
		++p;
	}
	return *this;
}

DEMGeo& DEMGeo::operator*=(const DEMGeo& rhs)
{
	if (rhs.mWidth != mWidth || rhs.mHeight != mHeight || mData == NULL || rhs.mData == NULL) 
		return *this;
	int sz = mWidth * mHeight;
	const float * src = rhs.mData;
	float * dst = mData;
	while (sz--)
	{
		if (*dst != NO_DATA && *src != NO_DATA)
			*dst *= *src;
		++dst;
		++src;
	}
	return *this;	
}

DEMGeo& DEMGeo::operator=(const DEMGeo& x)
{
	if (this == &x) return *this;

	if (x.mWidth != mWidth || x.mHeight != mHeight || mData == NULL)
	{
		if (mData)	free(mData);
		mWidth = x.mWidth;
		mHeight = x.mHeight;
		mData = (float *) malloc(mWidth * mHeight * sizeof(float));
	}

	mSouth = x.mSouth;
	mNorth = x.mNorth;
	mWest = x.mWest;
	mEast = x.mEast;
	
	if (mData == NULL)
		mWidth = mHeight = 0;
	else {
		if (x.mData)
			memcpy(mData, x.mData, mWidth * mHeight * sizeof(float));	
		else
			memset(mData, 0, mWidth * mHeight * sizeof(float));	
	}
	return *this;
}

void DEMGeo::overlay(const DEMGeo& x)	// Overlay
{
	if (x.mWidth != mWidth || x.mHeight != mHeight || mData == NULL || x.mData == NULL)
	{
		return;
	}
		
	for (int i = 0; i < (mWidth * mHeight); ++i)
	{
		if (x.mData[i] != NO_DATA)
			 mData[i] = x.mData[i];		
	} 
}

void DEMGeo::overlay(const DEMGeo& rhs, int dx, int dy)
{
	if (mData == NULL || rhs.mData == NULL) return;
	
	if (mWidth < (rhs.mWidth + dx)) return;
	if (mHeight < (rhs.mHeight + dy)) return;
	
	for (int x = 0; x < rhs.mWidth; ++x)
	for (int y = 0; y < rhs.mHeight; ++y)
	{
		float e = rhs.get(x,y);
		if (e != NO_DATA)
			(*this)(x+dx,y+dy) = e;
	}
}

void	DEMGeo::copy_geo(const DEMGeo& rhs)
{
	mNorth = rhs.mNorth;
	mSouth = rhs.mSouth;
	mEast  = rhs.mEast ;
	mWest  = rhs.mWest ;
}


void DEMGeo::derez(int r)
{
	DEMGeo	smaller(mWidth / r + 1, mHeight / r + 1);
	smaller.mNorth = mNorth;
	smaller.mSouth = mSouth;
	smaller.mEast = mEast;
	smaller.mWest = mWest;
	
	for(int x = 0; x < smaller.mWidth; ++x)
	for(int y=  0; y < smaller.mHeight; ++y)
	{
		double	ct = 0;
		double tot = 0;
		for (int dx = 0; dx < r; ++dx)
		for (int dy = 0; dy < r; ++dy)
		{
			if ((x * r + dx) < mWidth)
			if ((y * r + dy) < mHeight)
			{
				float e = get(x * r + dx,y * r + dy);
				if (e != NO_DATA)
					tot += 1, ct += e;
			}
		}
		if (tot > 0.0)
			smaller(x,y) = (ct / tot);
		else
			smaller(x,y) = NO_DATA;
	}
	
	this->swap(smaller);	
}

void	DEMGeo::resize(int width, int height)
{
	if (width == mWidth && height == mHeight) return;
	if (mData) free(mData);
	
	mWidth = width; mHeight = height;
	
	if (mWidth == 0 || mHeight == 0)
	{
		mData = 0;
	} else {
		mData = (float *) malloc(mWidth * mHeight * sizeof(float));
		if (mData == NULL)
			mWidth = mHeight = 0;
		else
			memset(mData, 0, mWidth * mHeight * sizeof(float));	
	}
}




void	DEMGeo::subset(DEMGeo& newDEM, int x1, int y1, int x2, int y2) const
{
	newDEM.resize(x2 - x1 + 1, y2 - y1 + 1);
	for (int x = x1; x <= x2; ++x)
	for (int y = y1; y <= y2; ++y)
		newDEM(x - x1, y - y1) = (*this)(x, y);
		
	newDEM.mSouth = y_to_lat(y1);
	newDEM.mNorth = y_to_lat(y2);
	newDEM.mWest = x_to_lon(x1);
	newDEM.mEast = x_to_lon(x2);
}

void	DEMGeo::swap(DEMGeo& rhs)
{
	std::swap(mWest, rhs.mWest);
	std::swap(mSouth, rhs.mSouth);
	std::swap(mEast, rhs.mEast);
	std::swap(mNorth, rhs.mNorth);
	std::swap(mWidth, rhs.mWidth);
	std::swap(mHeight, rhs.mHeight);
	std::swap(mData, rhs.mData);
}

void	DEMGeo::calc_slope(DEMGeo& outSlope, DEMGeo& outHeading, ProgressFunc inProg) const
{
	outSlope.resize(mWidth, mHeight);
	outHeading.resize(mWidth, mHeight);
	
	double	x_res = x_dist_to_m(1);
	double	y_res = y_dist_to_m(1);
	float	h, hl, ht, hb, hr;
	float	ld, rd, bd, td;
	
	if (inProg) inProg(0, 1, "Calculating Slope", 0.0);
	for (int x = 0; x < mWidth; ++x)
	for (int y = 0; y < mHeight;++y)
	{
		if (y == 0 && (x % 50) == 0)
			if (inProg) inProg(0, 1, "Calculating Slope", (double) x / (double) mWidth);
		
		h = get(x,y);
		if (h == NO_DATA)
		{
			outSlope(x,y) = NO_DATA;
			outHeading(x,y) = NO_DATA;
		} else {
			Point3 me(0,0,h);
			hl = get_dir(x,y,-1,0,        x,NO_DATA,ld);	Point3 pl(-ld*x_res,0,hl);	
			hr = get_dir(x,y, 1,0, mWidth-x,NO_DATA,rd);	Point3 pr( rd*x_res,0,hr);
			hb = get_dir(x,y,0,-1,        y,NO_DATA,bd);	Point3 pb(0,-bd*y_res,hb);
			ht = get_dir(x,y,0, 1,mHeight-y,NO_DATA,td);	Point3 pt(0, td*y_res,ht);
			
			Point3 * ph = NULL, * pv = NULL;
			
			if (hl != NO_DATA) 
			{
				if (hr != NO_DATA)
					ph = (ld < rd) ? &pl : &pr;
				else
					ph = &pl;
			} else {
				if (hr != NO_DATA)
					ph = &pr;
				else
					fprintf(stderr, "NO H ELEVATION\n");
			}

			if (hb != NO_DATA) 
			{
				if (ht != NO_DATA)
					pv = (bd < td) ? &pb : &pt;
				else
					pv = &pb;
			} else {
				if (ht != NO_DATA)
					pv = &pt;
				else
					fprintf(stderr, "NO V ELEVATION\n");
			}
						
			if (!ph || !pv) 
			{
				outSlope(x,y) = NO_DATA;
				outHeading(x,y) = NO_DATA;
				continue;
			} 
			Vector3	v1(me,*ph);
			Vector3	v2(me,*pv);
			Vector3	normal(v1.cross(v2));
			if (normal.dz < 0.0)
				normal *= -1.0;
			normal.normalize();
//			double	xy = sqrt(normal.dx * normal.dx + normal.dy * normal.dy);
//			outHeading(x,y) = atan2(normal.dx, normal.dy) * RAD_TO_DEG;
			outSlope(x,y) = 1.0 - normal.dz;
			normal.dz = 0;
			normal.normalize();
			outHeading(x,y) = normal.dy;
//			outSlope(x,y) = atan2(xy, normal.dz) * RAD_TO_DEG;
			
		}	
	}
	if (inProg) inProg(0, 1, "Calculating Slope", 1.0);	
}

void DEMGeo::fill_nearest(void)
{
	int x1 = 0, x2 = mWidth, y1 = 0, y2 = mHeight;
	DEMGeo	fill(*this);
	while (x1 < x2 && y1 < y2)
	{
		bool changed = false;
		int x_min = mWidth, x_max = 0, y_min = mHeight, y_max = 0;
		for (int x = x1; x < x2; ++x)
		for (int y = y1; y < y2; ++y)
		{
			float e = (*this)(x,y);
			if (e == NO_DATA)
			{
				HistoHelper	helper;
				helper.Accum(get(x-1,y-1), NO_DATA);
				helper.Accum(get(x+1,y-1), NO_DATA);
				helper.Accum(get(x-1,y+1), NO_DATA);
				helper.Accum(get(x+1,y+1), NO_DATA);

				helper.Accum(get(x-1,y  ), NO_DATA);
				helper.Accum(get(x+1,y  ), NO_DATA);
				helper.Accum(get(x  ,y+1), NO_DATA);
				helper.Accum(get(x  ,y-1), NO_DATA);
				if (helper.HasBest())
				{
					changed = true;
					fill(x,y) = helper.GetBest();
				} else {
					x_min = min(x_min, x);
					x_max = max(x_max, x+1);
					y_min = min(y_min, y);
					y_max = max(y_max, y+1);
				}
			}
		}
		x1 = x_min;
		x2 = x_max;
		y1 = y_min;
		y2 = y_max;
		if (!changed) return;
		memcpy(mData, fill.mData, mWidth * mHeight * sizeof(float));
	}
}


int	DEMGeo::remove_linear(int iterations, float max_err)
{
	int	zap_count = 0;
	int last_zap = 0;
	for (int i = 0; i < iterations; ++i)
	{
	   int os=(i%2);
	   
//		for (int c = 0; c < 4; ++c)
		{
			int c = 0;
			int x1,y1,dx, dy;

			switch(c) 
			{
				case 0: x1=os    	  ; y1=os     	   ; dx = 1; dy = 1; break;
				case 1: x1=os    	  ; y1=mHeight-os-1; dx = 1; dy =-1; break;
				case 2: x1=mWidth-os-1; y1=mHeight-os-1; dx =-1; dy =-1; break;
				case 3: x1=mWidth-os-1; y1=os     	   ; dx =-1; dy = 1; break;
			}
				
			for (int x = x1; x >= 0 && x < mWidth ; x += dx)
			for (int y = y1; y >= 0 && y < mHeight; y += dy)
			{
				float t, l, r, b, c, v;
				float dl, dr, dt, db;
				c = get(x, y);
				
				if (c != NO_DATA)
				{
					t = get_dir(x, y, 0, 1, 30, NO_DATA, dt);
					b = get_dir(x, y, 0, -1, 30, NO_DATA, db);
					r = get_dir(x, y, 1, 0, 30, NO_DATA, dr);
					l = get_dir(x, y, -1, 0, 30, NO_DATA, dl);
					if (t != NO_DATA && b != NO_DATA && l != NO_DATA && r != NO_DATA)
					{
						v = (l * dr + r * dl + t * db + b * dt) / (dr + dl + db + dt);
						if (fabs(v - c) < max_err) { zap(x,y); zap_count++; }
					} else if (t != NO_DATA && b != NO_DATA)
					{
						v = (t * db + b * dt) / (db + dt);
						if (fabs(v - c) < max_err) { zap(x,y); zap_count++; }
					} else if (l != NO_DATA && r != NO_DATA)
					{
						v = (l * dr + r * dl) / (dr + dl);
						if (fabs(v - c) < max_err) { zap(x,y); zap_count++; }
					}
				}		
			}
		}
		if ((zap_count - last_zap) < 10) 
			return zap_count;
		last_zap = zap_count;
	}
	return zap_count;
}

float	DEMGeo::local_minmax(int x1, int y1, int x2, int y2,
						 int& minx, int& miny, float& minh,
						 int& maxx, int& maxy, float& maxh)
{
	minx = maxx = x1;
	miny = maxy = y1;
	minh = maxh = get(x1, y1);
	for (int x = x1; x < x2; ++x)
	for (int y = y1; y < y2; ++y)
	{
		float h = get(x, y);
		if (h != NO_DATA && h < minh)
		{
			minx = x; 
			miny = y;
			minh = h;
		}
		if (h != NO_DATA && h > maxh)
		{
			maxx = x;
			maxy = y;
			maxh = h;
		}
	}
	if (minh == NO_DATA && maxh == NO_DATA) return NO_DATA;
	float rise = maxh - minh;
	if (minx == x1 || minx == (x2-1) || miny == y1 || miny == (y2-1))
		minh = NO_DATA;
	if (maxx == x1 || maxx == (x2-1) || maxy == y1 || maxy == (y2-1))
		maxh = NO_DATA;
	
	return rise;
}						 

void	DEMGeo::filter_self(int dim, float * k)
{
	DEMGeo	temp(*this);
	for (int x = 0; x < temp.mWidth; ++x)
	for (int y = 0; y < temp.mHeight;++y)
	{
		(*this)(x,y) = temp.kernelN(x,y,dim,k);
	}
}



/*
 * Given DEMs that contain the minimum and maximum for a given point,
 * produce new DEMs of half the resolution that contain the min and max
 * at each pixel for the area in the original area that has been reduced.
 *
 */
void	DEMGeo_ReduceMinMax(
					const DEMGeo& inMin, 
					const DEMGeo& inMax,
						  DEMGeo& outMin,
						  DEMGeo& outMax)
{
	outMin.mNorth = inMin.mNorth;
	outMin.mSouth = inMin.mSouth;
	outMin.mEast = inMin.mEast;
	outMin.mWest = inMin.mWest;
	outMax.mNorth = inMax.mNorth;
	outMax.mSouth = inMax.mSouth;
	outMax.mEast = inMax.mEast;
	outMax.mWest = inMax.mWest;
	
	outMin.resize(inMin.mWidth / 2 + 1, inMin.mHeight / 2 + 1);
	outMax.resize(inMax.mWidth / 2 + 1, inMax.mHeight / 2 + 1);
	
	int x, y;
	float e1, e2, e3, e4;
	
	for (x = 0; x < outMin.mWidth; ++x)
	for (y = 0; y < outMin.mHeight; ++y)
	{
		e1 = inMin.get(x*2  ,y*2  );
		e2 = inMin.get(x*2+1,y*2  );
		e3 = inMin.get(x*2+1,y*2+1);
		e4 = inMin.get(x*2  ,y*2+1);
		
		outMin(x,y) = MIN_NODATA(
							MIN_NODATA(e1, e2), 
							MIN_NODATA(e3, e4));
	}
	

	for (x = 0; x < outMax.mWidth; ++x)
	for (y = 0; y < outMax.mHeight; ++y)
	{
		e1 = inMax.get(x*2  ,y*2  );
		e2 = inMax.get(x*2+1,y*2  );
		e3 = inMax.get(x*2+1,y*2+1);
		e4 = inMax.get(x*2  ,y*2+1);
		
		outMax(x,y) = MAX_NODATA(
							MAX_NODATA(e1, e2), 
							MAX_NODATA(e3, e4));
	}
}

/*
 * Given a DEM, build 1 or more reduced DEMs that summarize
 * the min and maxes over a given area.
 *
 */
void	DEMGeo_BuildMinMax(
					const DEMGeo& 		inDEM,
					vector<DEMGeo>&		outMin,
					vector<DEMGeo>&		outMax,
					int					g)
{
	outMin.resize(g);
	outMax.resize(g);
	for (int i = 0; i < g; ++i)
	{
		if (i == 0)
		{
			DEMGeo_ReduceMinMax(inDEM, inDEM, outMin[i], outMax[i]);
		} else {
			DEMGeo_ReduceMinMax(outMin[i-1], outMax[i-1], outMin[i], outMax[i]);
		}
	}
}

/*
 * Given one cache square, find the actual local min and max.
 
 */
float	DEMGeo_LocalMinOfCacheSquare(
					const DEMGeo&			inDEM,
					const vector<DEMGeo>&	inMin,
					int level,
					int x, int y, 
					int& minx, int& miny, float& minh)
{
	if (level < 0)
	{
		minx = x; miny = y; minh = inDEM(x,y); return minh;
	}
	
	float e, e1, e2, e3, e4;
	e = inMin[level](x,y);
	if (e == NO_DATA) return NO_DATA;
	const DEMGeo& reduc = ((level == 0) ? inDEM : inMin[level-1]);
	e1 = reduc.get(x*2  ,y*2  );
	e2 = reduc.get(x*2+1,y*2  );
	e3 = reduc.get(x*2+1,y*2+1);
	e4 = reduc.get(x*2  ,y*2+1);
	if (level == 0)
	{
			 if (e1 == e) {minx = x*2  ; miny = y*2  ; minh = e1;}
		else if (e2 == e) {minx = x*2+1; miny = y*2  ; minh = e2;}
		else if (e3 == e) {minx = x*2+1; miny = y*2+1; minh = e3;}
		else if (e4 == e) {minx = x*2  ; miny = y*2+1; minh = e4;}
		else return NO_DATA;
		return e;
	} else {
			 if (e == e1) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2  , y*2  , minx, miny, minh);
		else if (e == e2) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2+1, y*2  , minx, miny, minh);
		else if (e == e3) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2+1, y*2+1, minx, miny, minh);
		else if (e == e4) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2  , y*2+1, minx, miny, minh);
		else return NO_DATA;
	}
}					

float	DEMGeo_LocalMaxOfCacheSquare(
					const DEMGeo&			inDEM,
					const vector<DEMGeo>&	inMax,
					int level,
					int x, int y, 
					int& maxx, int& maxy, float& maxh)
{
	if (level < 0)
	{
		maxx = x; maxy = y; maxh = inDEM(x,y); return maxh;
	}
	
	float e, e1, e2, e3, e4;
	e = inMax[level](x,y);
	if (e == NO_DATA) return NO_DATA;
	const DEMGeo& reduc = ((level == 0) ? inDEM : inMax[level-1]);
	e1 = reduc.get(x*2  ,y*2  );
	e2 = reduc.get(x*2+1,y*2  );
	e3 = reduc.get(x*2+1,y*2+1);
	e4 = reduc.get(x*2  ,y*2+1);
	if (level == 0)
	{
			 if (e1 == e) {maxx = x*2  ; maxy = y*2  ; maxh = e1;}
		else if (e2 == e) {maxx = x*2+1; maxy = y*2  ; maxh = e2;}
		else if (e3 == e) {maxx = x*2+1; maxy = y*2+1; maxh = e3;}
		else if (e4 == e) {maxx = x*2  ; maxy = y*2+1; maxh = e4;}
		else return NO_DATA;
		return e;
	} else {
			 if (e == e1) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2  , y*2  , maxx, maxy, maxh);
		else if (e == e2) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2+1, y*2  , maxx, maxy, maxh);
		else if (e == e3) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2+1, y*2+1, maxx, maxy, maxh);
		else if (e == e4) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2  , y*2+1, maxx, maxy, maxh);
		else return NO_DATA;
	}
}					



/*
 * Given Cache DEMs, find the local minima and maxima over an area.  The edge of the
 * box DO NOT count as the local minimum or maximum!
 *
 */
float	DEMGeo_LocalMinMaxWithCache(
					const DEMGeo&			inDEM,
					const vector<DEMGeo>&	inMin,
					const vector<DEMGeo>&	inMax,
					int x1, int y1, int x2, int y2,
					int& minx, int& miny, float& minh,
					int& maxx, int& maxy, float& maxh,
					bool					inAllowEdges)
{
	int x, y;
	minh = maxh = NO_DATA;
	
	for (y = y1; y < y2; ++y)
	for (x = x1; x < x2; ++x)
	{
		int xp, yp;
		int bsize = aligned_block(x,y,x1,x2,y1,y2,xp,yp,inMin.size()+1);
		if (xp == x && yp == y)
		{
			int lx, ly;
			float lh;
			if (DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, bsize-1,x >> bsize, y >> bsize, lx, ly, lh) != NO_DATA)
			{
				if (minh == NO_DATA || lh < minh)
				{
					minh = lh;
					minx = lx;
					miny = ly;
				}
			}

			if (DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, bsize-1,x >> bsize, y >> bsize, lx, ly, lh) != NO_DATA)
			{
				if (maxh == NO_DATA || lh > maxh)
				{
					maxh = lh;
					maxx = lx;
					maxy = ly;
				}
			}
		}
	}
	if (minh == NO_DATA && maxh == NO_DATA) return NO_DATA;
	float rise = maxh - minh;
	if (!inAllowEdges) 
	{
		if (minx == x1 || minx == (x2-1) || miny == y1 || miny == (y2-1))
			minh = NO_DATA;
		if (maxx == x1 || maxx == (x2-1) || maxy == y1 || maxy == (y2-1))
			maxh = NO_DATA;
	}	
	return rise;	
}

float	DEMGeo_LocalMinMaxWithCache(
			const DEMGeo&			inDEM,
			const vector<DEMGeo>&	inMin,
			const vector<DEMGeo>&	inMax,
			int x1, int y1, int x2, int y2,
			float& 					minh,
			float& 					maxh)
{
	int x, y;
	int xp, yp, bsize;
	float lh;
	minh = maxh = NO_DATA;
	
	for (y = y1; y < y2; ++y)
	for (x = x1; x < x2; ++x)
	{
		bsize = aligned_block(x,y,x1,x2,y1,y2,xp,yp,inMin.size()+1);
		if (xp == x && yp == y)
		{
			if (bsize == 0)
			{
				lh = inDEM(xp,yp);
				minh = MIN_NODATA(minh, lh);
				maxh = MAX_NODATA(maxh, lh);
			} else {
				lh = inMin[bsize-1](xp >> bsize, yp >> bsize);
				minh = MIN_NODATA(minh, lh);
				lh = inMax[bsize-1](xp >> bsize, yp >> bsize);
				maxh = MAX_NODATA(maxh, lh);				
			}
		}
	}				
	return maxh - minh;
}

