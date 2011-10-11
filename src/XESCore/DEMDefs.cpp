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
#include "MathUtils.h"
#include <list>

#define HIST_MAX	10

struct	HistoHelper {

	int		choices;
	int		best;
	float	values[HIST_MAX];
	int		counts[HIST_MAX];

			HistoHelper() : choices(0), best(-1) { }
	bool	HasBest(void) const { return best != -1; }
	float	GetBest(void) const { return (best == -1) ? DEM_NO_DATA : values[best]; }
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
	mPost(1),
	mData(0)
{
}

DEMGeo::DEMGeo(const DEMGeo& x) :
	mWest(x.mWest),
	mSouth(x.mSouth),
	mEast(x.mEast),
	mNorth(x.mNorth),
	mWidth(x.mWidth),
	mPost(x.mPost),
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
	mWidth(width), mHeight(height), mPost(1)
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
		if (*p != DEM_NO_DATA)
			*p += v;
		++p;
	}
	return *this;
}

DEMGeo& DEMGeo::operator+=(const DEMGeo& rhs)
{
	if (rhs.mWidth != mWidth || rhs.mHeight != mHeight || mData == NULL || rhs.mData == NULL || mPost != rhs.mPost)
		return *this;
	int sz = mWidth * mHeight;
	const float * src = rhs.mData;
	float * dst = mData;
	while (sz--)
	{
		if (*dst != DEM_NO_DATA && *src != DEM_NO_DATA)
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
		if (*p != DEM_NO_DATA)
			*p *= v;
		++p;
	}
	return *this;
}

DEMGeo& DEMGeo::operator*=(const DEMGeo& rhs)
{
	if (rhs.mWidth != mWidth || rhs.mHeight != mHeight || mData == NULL || rhs.mData == NULL || mPost != rhs.mPost)
		return *this;
	int sz = mWidth * mHeight;
	const float * src = rhs.mData;
	float * dst = mData;
	while (sz--)
	{
		if (*dst != DEM_NO_DATA && *src != DEM_NO_DATA)
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
	mPost = x.mPost;
	
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

void DEMGeo::clear_from(const DEMGeo& x, float v)
{
	if (this == &x) 
	{	
		for(iterator i = begin(); i != end(); ++i)
			*i = v;
		return;
	}
	
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
	mPost = x.mPost;
	
	if (mData == NULL)
		mWidth = mHeight = 0;
	else {
		for(iterator i = begin(); i != end(); ++i)
			*i = v;
	}
}

void DEMGeo::clear_from(const DEMGeo& x)
{
	if (this == &x) 
		return;
	
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
	mPost = x.mPost;
	
	if (mData == NULL)
		mWidth = mHeight = 0;
}


void DEMGeo::overlay(const DEMGeo& x)	// Overlay
{
	if (x.mWidth != mWidth || x.mHeight != mHeight || mPost != x.mPost || mData == NULL || x.mData == NULL)
	{
		return;
	}

	for (int i = 0; i < (mWidth * mHeight); ++i)
	{
		if (x.mData[i] != DEM_NO_DATA)
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
		if (e != DEM_NO_DATA)
			(*this)(x+dx,y+dy) = e;
	}
}

void	DEMGeo::copy_geo_from(const DEMGeo& rhs)
{
	mNorth = rhs.mNorth;
	mSouth = rhs.mSouth;
	mEast  = rhs.mEast ;
	mWest  = rhs.mWest ;	
}


void DEMGeo::derez(int r)
{	
	DEMGeo	smaller((mWidth+r-1-mPost) / r + mPost, (mHeight+r-1-mPost) / r + mPost);
	smaller.mNorth = mNorth;
	smaller.mSouth = mSouth;
	smaller.mEast = mEast;
	smaller.mWest = mWest;
	smaller.mPost = mPost;
	
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
                if (e != DEM_NO_DATA)
                    tot += 1, ct += e;
			}
 		}
		if (tot > 0.0)
			smaller(x,y) = (ct / tot);
		else
			smaller(x,y) = DEM_NO_DATA;
	}

	this->swap(smaller);
}

void DEMGeo::derez_nearest(DEMGeo& smaller)
{
	smaller.resize((mWidth-mPost) / 2 + mPost, (mHeight-mPost) / 2 + mPost);
	smaller.mNorth = mNorth;
	smaller.mSouth = mSouth;
	smaller.mEast = mEast;
	smaller.mWest = mWest;
	smaller.mPost = mPost;

	for(int x = 0; x < smaller.mWidth; ++x)
	for(int y=  0; y < smaller.mHeight; ++y)
	{
		float samples[4] = {
			get(x*2  ,y*2  ),
			get(x*2+1,y*2  ),
			get(x*2  ,y*2+1),
			get(x*2+1,y*2+1) };
		
		float best_val = samples[0];
		for(int n = 1; n < 4; ++n)
		if(samples[n] != DEM_NO_DATA)
		{
			best_val = samples[n];
			break;
		}
		
		smaller(x,y) = best_val;
	}
}

void DEMGeo::derez_nearest(void)
{
	DEMGeo	smaller;
	derez_nearest(smaller);
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
		mData = (float *) malloc((size_t) mWidth * (size_t) mHeight * sizeof(float));
		if (mData == NULL)
			mWidth = mHeight = 0;
		else
			memset(mData, 0, (size_t) mWidth * (size_t) mHeight * sizeof(float));
	}
}

void DEMGeo::set_rez(double x_res, double y_res)
{
	int want_x = round(x_res * (mEast - mWest)) + mPost;
	int want_y = round(y_res * (mNorth - mSouth)) + mPost;
	this->resize(want_x,want_y);
}

void DEMGeo::resize_save(int w, int h, float fill_value)
{
	if(w == mWidth && h == mHeight) return;
	
	DEMGeo	other(w,h);
	other = fill_value;
	
	other.copy_geo_from(*this);
	int xx = min(mWidth, w);
	int yy = min(mHeight, h);
	
	for(int y = 0; y < yy; ++y)
	for(int x = 0; x < xx; ++x)
		other(x,y) = get(x,y);
	
	swap(other);	
}


void	DEMGeo::subset(DEMGeo& newDEM, int x1, int y1, int x2, int y2) const
{
	newDEM.resize(x2 - x1 + mPost, y2 - y1 + mPost);
	newDEM.mPost = mPost;
	for (int x = 0; x < newDEM.mWidth; ++x)
	for (int y = 0; y < newDEM.mHeight; ++y)
		newDEM(x, y) = (*this)(x + x1, y + y1);

	// x2,y2 are _exclusive_ (past the edge) if we are area pixe, so SUBTRACT.
	newDEM.mSouth = y_to_lat_double((double) y1 - pixel_offset());
	newDEM.mNorth = y_to_lat_double((double) y2 - pixel_offset());
	newDEM.mWest = x_to_lon_double((double) x1 - pixel_offset());
	newDEM.mEast = x_to_lon_double((double) x2 - pixel_offset());
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
	std::swap(mPost, rhs.mPost);
}

void	DEMGeo::calc_normal(DEMGeo& outX, DEMGeo& outY, DEMGeo& outZ, ProgressFunc inProg) const
{
	outX.resize(mWidth, mHeight);
	outX.copy_geo_from(*this);
	outX.mPost = mPost;
	outY.resize(mWidth, mHeight);
	outY.copy_geo_from(*this);
	outY.mPost = mPost;
	outZ.resize(mWidth, mHeight);
	outZ.copy_geo_from(*this);
	outZ.mPost = mPost;

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
		if (h == DEM_NO_DATA)
		{
			outX(x,y) = 0;
			outY(x,y) = 0;
			outZ(x,y) = 1;
		} else {
			Point3 me(0,0,h);
			hl = get_dir(x,y,-1,0,        x,DEM_NO_DATA,ld);	Point3 pl(-ld*x_res,0,hl);
			hr = get_dir(x,y, 1,0, mWidth-x,DEM_NO_DATA,rd);	Point3 pr( rd*x_res,0,hr);
			hb = get_dir(x,y,0,-1,        y,DEM_NO_DATA,bd);	Point3 pb(0,-bd*y_res,hb);
			ht = get_dir(x,y,0, 1,mHeight-y,DEM_NO_DATA,td);	Point3 pt(0, td*y_res,ht);

			Point3 * ph = NULL, * pv = NULL;

			if (hl != DEM_NO_DATA)
			{
				if (hr != DEM_NO_DATA)
					ph = (ld < rd) ? &pl : &pr;
				else
					ph = &pl;
			} else {
				if (hr != DEM_NO_DATA)
					ph = &pr;
				else
					fprintf(stderr, "NO H ELEVATION\n");
			}

			if (hb != DEM_NO_DATA)
			{
				if (ht != DEM_NO_DATA)
					pv = (bd < td) ? &pb : &pt;
				else
					pv = &pb;
			} else {
				if (ht != DEM_NO_DATA)
					pv = &pt;
				else
					fprintf(stderr, "NO V ELEVATION\n");
			}

			if (!ph || !pv)
			{
				outX(x,y) = 0;
				outY(x,y) = 0;
				outZ(x,y) = 1;
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
			outX(x,y)=normal.dx;
			outY(x,y)=normal.dy;
			outZ(x,y)=normal.dz;
//			outSlope(x,y) = atan2(xy, normal.dz) * RAD_TO_DEG;

		}
	}
	if (inProg) inProg(0, 1, "Calculating Slope", 1.0);
}


void	DEMGeo::calc_slope(DEMGeo& outSlope, DEMGeo& outHeading, ProgressFunc inProg) const
{
	outSlope.resize(mWidth, mHeight);
	outHeading.resize(mWidth, mHeight);
	outHeading.mPost = mPost;
	outSlope.mPost = mPost;

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
		if (h == DEM_NO_DATA)
		{
			outSlope(x,y) = DEM_NO_DATA;
			outHeading(x,y) = DEM_NO_DATA;
		} else {
			Point3 me(0,0,h);
			hl = get_dir(x,y,-1,0,        x,DEM_NO_DATA,ld);	Point3 pl(-ld*x_res,0,hl);
			hr = get_dir(x,y, 1,0, mWidth-x,DEM_NO_DATA,rd);	Point3 pr( rd*x_res,0,hr);
			hb = get_dir(x,y,0,-1,        y,DEM_NO_DATA,bd);	Point3 pb(0,-bd*y_res,hb);
			ht = get_dir(x,y,0, 1,mHeight-y,DEM_NO_DATA,td);	Point3 pt(0, td*y_res,ht);

			Point3 * ph = NULL, * pv = NULL;

			if (hl != DEM_NO_DATA)
			{
				if (hr != DEM_NO_DATA)
					ph = (ld < rd) ? &pl : &pr;
				else
					ph = &pl;
			} else {
				if (hr != DEM_NO_DATA)
					ph = &pr;
				else
					fprintf(stderr, "NO H ELEVATION\n");
			}

			if (hb != DEM_NO_DATA)
			{
				if (ht != DEM_NO_DATA)
					pv = (bd < td) ? &pb : &pt;
				else
					pv = &pb;
			} else {
				if (ht != DEM_NO_DATA)
					pv = &pt;
				else
					fprintf(stderr, "NO V ELEVATION\n");
			}

			if (!ph || !pv)
			{
				outSlope(x,y) = DEM_NO_DATA;
				outHeading(x,y) = DEM_NO_DATA;
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



//// Past here I have not checked for correct post use.

void dem_copy_buffer_one(const DEMGeo& orig_src, DEMGeo& io_dst, float null_value)
{
	address_fifo	fifo(io_dst.mWidth * io_dst.mHeight);
	for(DEMGeo::address a = io_dst.address_begin(); a != io_dst.address_end(); ++a)
	if(io_dst[a] == null_value)
	{
		const int x_off[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
		const int y_off[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
		
		DEMGeo::coordinates c(io_dst.to_coordinates(a));
		bool has_data = false;
		for(int n = 0; n < 8; ++n)
		{
			DEMGeo::coordinates nn(c.first + x_off[n], c.second + y_off[n]);
			if(io_dst.valid(nn) && io_dst[nn] != null_value)
			{
				has_data = true;
				break;
			}
		}
		if(has_data) 
			fifo.push(a);		
	}
	
	while(!fifo.empty())
	{
		DEMGeo::address a = fifo.pop();
		io_dst[a] = orig_src[a];
	}
}

void DEMGeo::fill_nearest(void)
{
	const int x_off[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	const int y_off[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	
	int x, y, n;
	
	// We have to use a queue of work - if we iterate over the entire DEM
	// multiple times (to get a breadth-first search) performance is too
	// slow with a 1000x1000 DEM.
	list<pair<int,int> >	todo_list;
	
	for(y = 0; y < mHeight; ++y)
	for(x = 0; x < mWidth; ++x)
	if(get(x,y) == DEM_NO_DATA)
	{
		for(n = 0; n < 8; ++n)
		if(get(x+x_off[n],y+y_off[n]) != DEM_NO_DATA)
			break;
		if(n < 8)
			todo_list.push_back(pair<int,int>(x,y));
	}
	
	while(!todo_list.empty())
	{		
		x = todo_list.begin()->first;
		y = todo_list.begin()->second;
		todo_list.pop_front();
		
		// We might discover a point from multiple sources but 
		// queue it because we don't have a "marked" value (and
		// this code is stupid).  So just skip if it's already
		// handled.  Not good, but adequate for a 1000 x 1000 DEM.
		if(get(x,y) != DEM_NO_DATA)
			continue;
		HistoHelper helper;
		for(n = 0; n < 8; ++n)
		{
			int dx= x + x_off[n];
			int dy = y + y_off[n];
			if(dx >= 0 && dx < mWidth && dy >= 0 && dy < mHeight)
			{
				float e = get(dx,dy);
				if(e == DEM_NO_DATA)
					todo_list.push_back(pair<int,int>(dx,dy));
				else
					helper.Accum(e, DEM_NO_DATA);
				}
		}
		DebugAssert(helper.HasBest());
		(*this)(x,y) = helper.GetBest();
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

				if (c != DEM_NO_DATA)
				{
					t = get_dir(x, y, 0, 1, 30, DEM_NO_DATA, dt);
					b = get_dir(x, y, 0, -1, 30, DEM_NO_DATA, db);
					r = get_dir(x, y, 1, 0, 30, DEM_NO_DATA, dr);
					l = get_dir(x, y, -1, 0, 30, DEM_NO_DATA, dl);
					if (t != DEM_NO_DATA && b != DEM_NO_DATA && l != DEM_NO_DATA && r != DEM_NO_DATA)
					{
						v = (l * dr + r * dl + t * db + b * dt) / (dr + dl + db + dt);
						if (fabs(v - c) < max_err) { zap(x,y); zap_count++; }
					} else if (t != DEM_NO_DATA && b != DEM_NO_DATA)
					{
						v = (t * db + b * dt) / (db + dt);
						if (fabs(v - c) < max_err) { zap(x,y); zap_count++; }
					} else if (l != DEM_NO_DATA && r != DEM_NO_DATA)
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
		if (h != DEM_NO_DATA && h < minh)
		{
			minx = x;
			miny = y;
			minh = h;
		}
		if (h != DEM_NO_DATA && h > maxh)
		{
			maxx = x;
			maxy = y;
			maxh = h;
		}
	}
	if (minh == DEM_NO_DATA && maxh == DEM_NO_DATA) return DEM_NO_DATA;
	float rise = maxh - minh;
	if (minx == x1 || minx == (x2-1) || miny == y1 || miny == (y2-1))
		minh = DEM_NO_DATA;
	if (maxx == x1 || maxx == (x2-1) || maxy == y1 || maxy == (y2-1))
		maxh = DEM_NO_DATA;

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

void	DEMGeo::filter_self_normalize(int dim, float * k)
{
	DEMGeo	temp(*this);
	for (int x = 0; x < temp.mWidth; ++x)
	for (int y = 0; y < temp.mHeight;++y)
	{
		(*this)(x,y) = temp.kernelN_Normalize(x,y,dim,k);
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

	outMin.resize((inMin.mWidth-inMin.mPost) / 2 + inMin.mPost, (inMin.mHeight-inMin.mPost) / 2 + inMin.mPost);
	outMax.resize((inMax.mWidth-inMax.mPost) / 2 + inMax.mPost, (inMax.mHeight-inMax.mPost) / 2 + inMax.mPost);

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

void	DEMGeo_ReduceMinMaxN(
					const DEMGeo& inDEM,
						  DEMGeo& outMin,
						  DEMGeo& outMax,
						  int N)
{
	outMin.copy_geo_from(inDEM);
	outMax.copy_geo_from(inDEM);

	outMin.resize((inDEM.mWidth-inDEM.mPost) / N + inDEM.mPost, (inDEM.mHeight-1) / N + inDEM.mPost);
	outMax.resize((inDEM.mWidth-inDEM.mPost) / N + inDEM.mPost, (inDEM.mHeight-1) / N + inDEM.mPost);

	int x, y, dx, dy;
	float e1, e2, e3;

	for (y = 0; y < outMin.mHeight; y++)
	for (x = 0; x < outMin.mWidth; x++)
	{
		int rx = x * N;
		int ry = y * N;
		e1 = e2 = DEM_NO_DATA;
		for (dy = 0; dy < N; ++dy)
		for (dx = 0; dx < N; ++dx)
		{
			e3 = inDEM.get(rx + dx, ry + dy);
			e1 = MIN_NODATA(e1, e3);
			e2 = MAX_NODATA(e2, e3);
		}
		outMin(x,y) = e1;
		outMax(x,y) = e2;
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
	if (e == DEM_NO_DATA) return DEM_NO_DATA;
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
		else return DEM_NO_DATA;
		return e;
	} else {
			 if (e == e1) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2  , y*2  , minx, miny, minh);
		else if (e == e2) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2+1, y*2  , minx, miny, minh);
		else if (e == e3) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2+1, y*2+1, minx, miny, minh);
		else if (e == e4) return DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, level-1, x*2  , y*2+1, minx, miny, minh);
		else return DEM_NO_DATA;
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
	if (e == DEM_NO_DATA) return DEM_NO_DATA;
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
		else return DEM_NO_DATA;
		return e;
	} else {
			 if (e == e1) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2  , y*2  , maxx, maxy, maxh);
		else if (e == e2) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2+1, y*2  , maxx, maxy, maxh);
		else if (e == e3) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2+1, y*2+1, maxx, maxy, maxh);
		else if (e == e4) return DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, level-1, x*2  , y*2+1, maxx, maxy, maxh);
		else return DEM_NO_DATA;
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
	minh = maxh = DEM_NO_DATA;

	for (y = y1; y < y2; ++y)
	for (x = x1; x < x2; ++x)
	{
		int xp, yp;
		int bsize = aligned_block(x,y,x1,x2,y1,y2,xp,yp,inMin.size()+1);
		if (xp == x && yp == y)
		{
			int lx, ly;
			float lh;
			if (DEMGeo_LocalMinOfCacheSquare(inDEM, inMin, bsize-1,x >> bsize, y >> bsize, lx, ly, lh) != DEM_NO_DATA)
			{
				if (minh == DEM_NO_DATA || lh < minh)
				{
					minh = lh;
					minx = lx;
					miny = ly;
				}
			}

			if (DEMGeo_LocalMaxOfCacheSquare(inDEM, inMax, bsize-1,x >> bsize, y >> bsize, lx, ly, lh) != DEM_NO_DATA)
			{
				if (maxh == DEM_NO_DATA || lh > maxh)
				{
					maxh = lh;
					maxx = lx;
					maxy = ly;
				}
			}
		}
	}
	if (minh == DEM_NO_DATA && maxh == DEM_NO_DATA) return DEM_NO_DATA;
	float rise = maxh - minh;
	if (!inAllowEdges)
	{
		if (minx == x1 || minx == (x2-1) || miny == y1 || miny == (y2-1))
			minh = DEM_NO_DATA;
		if (maxx == x1 || maxx == (x2-1) || maxy == y1 || maxy == (y2-1))
			maxh = DEM_NO_DATA;
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
	minh = maxh = DEM_NO_DATA;

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

DEMMask::DEMMask() :
	mWest(-180), mEast(180), mSouth(-90), mNorth(90),
	mWidth(0),mHeight(0), mPost(1)
{
}

DEMMask::DEMMask(int w, int h, bool ini) :
	mWest(-180), mEast(180), mSouth(-90), mNorth(90),
	mWidth(w),mHeight(h), mPost(1)
{
	mData.assign(mWidth * mHeight, ini);
}

DEMMask::DEMMask(const DEMGeo& rhs) : mWidth(rhs.mWidth), mHeight(rhs.mHeight),
	mNorth(rhs.mNorth), mSouth(rhs.mSouth), mEast(rhs.mEast), mWest(rhs.mWest), mPost(rhs.mPost)
{
	mData.resize(mWidth * mHeight);
	for(int y = 0; y < mHeight; ++y)
	for(int x = 0; x < mWidth; ++x)
		mData[y * mWidth + x] = (rhs.mData[y * mWidth + x] != DEM_NO_DATA);
}

DEMMask& DEMMask::operator=(bool x)
{
	mData.assign(mWidth * mHeight, x);
	return *this;
}

DEMMask& DEMMask::operator=(const DEMGeo& rhs)
{
	copy_geo_from(rhs);
	mWidth = rhs.mWidth;
	mHeight = rhs.mHeight;
	mPost = rhs.mPost;
	mData.resize(mWidth * mHeight);
	for(int y = 0; y < mHeight; ++y)
	for(int x = 0; x < mWidth; ++x)
		mData[y * mWidth + x] = (rhs.mData[y * mWidth + x] != DEM_NO_DATA);
	return *this;
}

DEMMask& DEMMask::operator=(const DEMMask& x)
{
	copy_geo_from(x);
	mWidth = x.mWidth;
	mHeight = x.mHeight;
	mData = x.mData;
	return *this;
}

void	DEMMask::resize(int width, int height, bool ini)
{
	mWidth = width;
	mHeight = height;
	mData.assign(mWidth * mHeight, ini);
}

void	DEMMask::copy_geo_from(const DEMGeo& rhs)
{
	mNorth = rhs.mNorth;
	mSouth = rhs.mSouth;
	mEast = rhs.mEast;
	mWest = rhs.mWest;
}

void	DEMMask::copy_geo_from(const DEMMask& rhs)
{
	mNorth = rhs.mNorth;
	mSouth = rhs.mSouth;
	mEast = rhs.mEast;
	mWest = rhs.mWest;
}

void		dem_coverage_nearest(const DEMGeo& d, double lon1, double lat1, double lon2, double lat2, int bounds[4])
{
	DebugAssert(lon1 >= d.mWest);
	DebugAssert(lat1 >= d.mSouth);
	DebugAssert(lon2 <= d.mEast);
	DebugAssert(lat2 <= d.mNorth);
	bounds[0] = floor(d.lon_to_x(lon1) + d.pixel_offset());
	bounds[1] = floor(d.lat_to_y(lat1) + d.pixel_offset());
	bounds[2] = ceil(d.lon_to_x(lon2) - d.pixel_offset()) + 1;
	bounds[3] = ceil(d.lat_to_y(lat2) - d.pixel_offset()) + 1;
	DebugAssert(bounds[0] >= 0);
	DebugAssert(bounds[1] >= 0);
	DebugAssert(bounds[2] <= d.mWidth);
	DebugAssert(bounds[3] <= d.mHeight);
}
