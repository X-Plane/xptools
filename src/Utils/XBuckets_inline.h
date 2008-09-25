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
#ifndef XBUCKETS_INLINE_H
#define XBUCKETS_INLINE_H

#if DEV

#include "hl_types.h"

void  __MACIBM_alert(xint close_to,	string C_string1,			// these guys are of unknown length because
									string C_string2,			// we are always sending in string literals
									string C_string3,			// of completely varying lengths!
									string C_string4,xint type,	// therefore we can NOT dimension the pointers here!
									const xchr * file,xint line);

#define MACIBM_alert(win,c1,c2,c3,c4,type) __MACIBM_alert((win),(c1),(c2),(c3),(c4),(type),__FILE__,__LINE__)

#endif

#include "AssertUtils.h"

template <typename __Object, typename __Traits>
XBuckets<__Object, __Traits>::XBuckets()
{
	mTotal = 0;
	mItems.resize(0);
}

template <typename __Object, typename __Traits>
XBuckets<__Object, __Traits>::XBuckets(unsigned int inLayers, const Point2& inMin, const Point2& inMax, int mode) :
	mMin(inMin), mMax(inMax), mMode(mode), mTotal(0)
{
	Traits::MakePoint(
				Traits::X(mMax) - Traits::X(mMin),
				Traits::Y(mMax) - Traits::Y(mMin),
				mSize);

	mItems.resize(inLayers);
}

template <typename __Object, typename __Traits>
XBuckets<__Object, __Traits>::~XBuckets()
{
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::Reset(unsigned int inLayers, const Point2& inMin, const Point2& inMax, int mode)
{
	mMin = inMin;
	mMax = inMax;
	mMode = mode;
	mItems.clear();
	Traits::MakePoint(
				Traits::X(mMax) - Traits::X(mMin),
				Traits::Y(mMax) - Traits::Y(mMin),
				mSize);

	mItems.resize(inLayers);
	mTotal = 0;
}


#pragma mark -

template <typename __Object, typename __Traits>
XBucketID
XBuckets<__Object, __Traits>::Insert(Object inObject)
{
#if DEV
	if (mItems.empty()) MACIBM_alert(0, "Attempting to add object to non-inited bucket.", "", "", "", t_exit);
#endif
	XBucketID	id = FindBucket(inObject);
	unsigned int level = XBucket_GetLevel(id);
	unsigned int index = XBucket_GetIndex(id);

	#if DEV
		#if defined(SIM) || defined(WRL)
			if(!intrange(level,0,mItems.size()-1))
				MACIBM_alert(0,"your level is not within the mitimes size range!","","","",t_exit);
		#else
			DebugAssert(level >= 0);
			DebugAssert(level < mItems.size());
		#endif
	#endif




	BucketMap& our_map = mItems[level];



	typename BucketMap::iterator our_bucket = our_map.find(index);
	if (our_bucket == our_map.end())
	{
		ObjectVector	new_vec;
		new_vec.push_back(inObject);
		our_map.insert(typename BucketMap::value_type(index, new_vec));
	} else {
		our_bucket->second.push_back(inObject);
	}
	++mTotal;
	return id;
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::Remove(Object inObject)
{
	XBucketID	id = FindBucket(inObject);
	unsigned int level = XBucket_GetLevel(id);
	unsigned int index = XBucket_GetIndex(id);
	BucketMap& our_map = mItems[index];
	typename BucketMap::iterator our_bucket = our_map.find(index);
	if (our_bucket != our_map.end())
	{
		typename ObjectVector::iterator i = find(our_bucket->second.begin(), our_bucket->second.end(), inObject);
		if (i != our_bucket->second.end())
		{
			our_bucket->second.erase(i);
			--mTotal;
		}
	}
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::RemoveAllAndDestroy(void)
{
	for (typename vector<BucketMap>::iterator i = mItems.begin(); i != mItems.end(); ++i)
	{
		for (typename BucketMap::iterator j = i->begin(); j != i->end(); ++j)
		{
			for (typename ObjectVector::iterator k = j->second.begin(); k != j->second.end(); ++k)
			{
				Traits::DestroyObject(*k);
			}
		}
		i->clear();
	}
	mTotal = 0;
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::Move(Object inObject, XBucketID inOldBucket)
{
	unsigned int level = XBucket_GetLevel(inOldBucket);
	unsigned int index = XBucket_GetIndex(inOldBucket);
	BucketMap& our_map = mItems[index];
	typename BucketMap::iterator our_bucket = our_map.find(index);
	if (our_bucket != our_map.end())
	{
		typename ObjectVector::iterator i = find(our_bucket->second.begin(), our_bucket->second.end(), inObject);
		if (i != our_bucket->second.end())
		{
			our_bucket->second.erase(i);
			--mTotal;
		}
	}
	Insert(inObject);
}

#pragma mark -

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::FindTouchesPt(const Point2& p, vector<Object>& outIDs) const
{
	FindTouchesPt(p, InsertIntoVector, &outIDs);
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::FindTouchesRect(const Point2& p1, const Point2& p2, vector<Object>& outIDs) const
{
	FindTouchesRect(p1, p2, InsertIntoVector, &outIDs);
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::FindFullyInRect(const Point2& p1, const Point2& p2, vector<Object>& outIDs) const
{
	FindFullyInRect(p1, p2, InsertIntoVector, &outIDs);
}



#pragma mark -

template <typename __Object, typename __Traits>
inline XBucketID
XBuckets<__Object, __Traits>::FindBucket(Object inObject) const
{
	Point2	omin, omax;
	Traits::GetObjectBounds(inObject, omin, omax);


	// The loose algorithm: we find the bucket underneath the center
	// of the bounding area.  But the level of buckets is based
	// on the size of the entity being bucketed.
	if (mMode == bucket_Organize_Draw)
	{
		Scalar	x = (Traits::X(omin) + Traits::X(omax)) * 0.5;
		Scalar	y = (Traits::Y(omin) + Traits::Y(omax)) * 0.5;
		Scalar	w = Traits::X(omax) - Traits::X(omin);
		Scalar	h = Traits::Y(omax) - Traits::Y(omin);

		w /= Traits::X(mSize);
		h /= Traits::Y(mSize);
		x -= Traits::X(mMin);
		y -= Traits::Y(mMin);
		x /= Traits::X(mSize);
		y /= Traits::Y(mSize);

		Scalar	frac = (w > h) ? w : h;

		int	level;
		for (level = 0; level < (mItems.size()-1); ++level)
		{
			frac *= 2.0;
			if (frac > 1.0) break;
		}

		int width = 1 << level;
		x *= (Scalar) width;
		y *= (Scalar) width;

#if DEV	&& (SIM || WRL)
		if (x < 0 || x >= width || y < 0 || y >= width)
		{
			deverr << " MISBUCKET!!  " << Traits::X(omin) << "," << Traits::Y(omin) << "->"
									   << Traits::X(omax) << "," << Traits::Y(omax) << " result "
									   	<< x << "," << y << " (width max was " << width << " level is " << level << ")\n";
			deverr.flush();
			MACIBM_alert(0, "Misbucket!", "", "", "", t_alert);
		}
#endif

		if (x < 0) x = 0;
		if (x > (width-1)) x = width-1;
		if (y < 0) y = 0;
		if (y > (width-1)) y = width-1;

		#if DEV
			if(level<0				)MACIBM_alert(0,"Lev el<0!","","","",t_exit);
			if(level>=mItems.size()	)MACIBM_alert(0,"level>=mItems.size()!","","","",t_exit);
		#endif


		return XBucket_Compose(level, x, y);

	} else {

		// The tight algorithm - find the smallest buckets that fully
		// contain us.

		Scalar	l = Traits::X(omin);
		Scalar	r = Traits::X(omax);
		Scalar	b = Traits::Y(omin);
		Scalar	t = Traits::Y(omax);

		l -= Traits::X(mMin);
		r -= Traits::X(mMin);
		b -= Traits::Y(mMin);
		t -= Traits::Y(mMin);

		l /= Traits::X(mSize);
		r /= Traits::X(mSize);
		b /= Traits::Y(mSize);
		t /= Traits::Y(mSize);

		Scalar	num_buckets_dense = (1L << mItems.size() - 1);

		int il = floor(l * num_buckets_dense);
		int ir = ceil(r * num_buckets_dense);
		int ib = floor(b * num_buckets_dense);
		int it = ceil(t * num_buckets_dense);

		for (int level = mItems.size() - 1; level >= 0; --level)
		{
			int bucket_size = 1L << (mItems.size() - level - 1);
			int ilm = il & ~(bucket_size-1);
			int ibm = ib & ~(bucket_size-1);
			int	irm = ilm + bucket_size;
			int itm = ibm + bucket_size;

			if (irm >= ir && itm >= it)
			{
				return XBucket_Compose(level, ilm >> (mItems.size() - level - 1), ibm >> (mItems.size() - level - 1));
			}
		}

		return 0;
	}
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::GetOneBucket(XBucketID inBucket, vector<Object>& outIDs) const
{
	GetOneBucket(inBucket, InsertIntoVector, &outIDs);
}

template <typename __Object, typename __Traits>
inline void
XBuckets<__Object, __Traits>::GetOneBucketUnsafe(XBucketID id, Object ** ioBegin, Object ** ioEnd)
{
	unsigned int level = XBucket_GetLevel(id);
	unsigned int index = XBucket_GetIndex(id);
	BucketMap& bucket_map = mItems[level];
	typename BucketMap::iterator bucket = bucket_map.find(index);
	if (bucket != bucket_map.end() && !bucket->second.empty())
	{
		*ioBegin = &*bucket->second.begin();
		*ioEnd = &*bucket->second.end();
	} else {
		*ioBegin = NULL;
		*ioEnd = NULL;
	}
}




template <typename __Object, typename __Traits>
inline void
XBuckets<__Object, __Traits>::GetBucketDimensions(XBucketID id, Point2& minp, Point2& maxp) const
{
	unsigned int level, x, y;
	XBucket_Decompose(id, level, x, y);
	unsigned int dim = 1 << level;
	Scalar	x1 = (Scalar) x / (Scalar) dim;
	Scalar	y1 = (Scalar) y / (Scalar) dim;
	Scalar	x2 = (Scalar) (x+1) / (Scalar) dim;
	Scalar	y2 = (Scalar) (y+1) / (Scalar) dim;

	Traits::MakePoint(
				Traits::X(mMin) + x1 * Traits::X(mSize),
				Traits::Y(mMin) + y1 * Traits::Y(mSize),
				minp);
	Traits::MakePoint(
				Traits::X(mMin) + x2 * Traits::X(mSize),
				Traits::Y(mMin) + y2 * Traits::Y(mSize),
				maxp);
}

#pragma mark -

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::InsertIntoVector(Object i, vector<Object> * v)
{
	v->push_back(i);
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::GetIndicesForLevel(
							const Point2&		p1,
							const Point2&		p2,
							unsigned int 		level,
							unsigned int&		x1,
							unsigned int&		y1,
							unsigned int&		x2,
							unsigned int&		y2) const
{
	unsigned int	dim = 1 << level;	// Buckets per side of our area?
	Scalar			dim_scalar = dim;

	// These are ratios in terms of buckets, e.g. 1.3 = bucket #1 + 1/3 of the way through.
	Scalar	rat_min_x = (Traits::X(p1) - Traits::X(mMin)) * dim_scalar / Traits::X(mSize);
	Scalar	rat_min_y = (Traits::Y(p1) - Traits::Y(mMin)) * dim_scalar / Traits::Y(mSize);
	Scalar	rat_max_x = (Traits::X(p2) - Traits::X(mMin)) * dim_scalar / Traits::X(mSize);
	Scalar	rat_max_y = (Traits::Y(p2) - Traits::Y(mMin)) * dim_scalar / Traits::Y(mSize);

	x1 = (rat_min_x >= 0.0) ? rat_min_x : 0;
	x2 = (rat_max_x >= 0.0) ? rat_max_x : 0;
	y1 = (rat_min_y >= 0.0) ? rat_min_y : 0;
	y2 = (rat_max_y >= 0.0) ? rat_max_y : 0;

	// NOTE: these bounds must become inclusive ranges that any bucket can even
	// touch.  So on the min side, if we are exactly on a bucket edge, there may be
	// buckets to our left that we need to consider.  On our right side we're ok
	// as is...if we hit an edge exactly then that bucket is included; if we didn't
	// hit the edge exactly that's ok because we don't need to worry about the 1-beyond
	// bucket.
	if (x1 > 0 && ((Scalar) x1) == rat_min_x)	--x1;
	if (y1 > 0 && ((Scalar) y1) == rat_min_y)   --y1;

	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 > (dim_scalar-1)) x2 = (dim_scalar-1);
	if (y2 > (dim_scalar-1)) y2 = (dim_scalar-1);
}

template <typename __Object, typename __Traits>
bool
XBuckets<__Object, __Traits>::Empty(void) const
{
	return mTotal == 0;
}

template <typename __Object, typename __Traits>
void
XBuckets<__Object, __Traits>::Dump(void)
{
	char	buf[1024];
	int n = 0;
	for (typename vector<BucketMap>::iterator level = mItems.begin(); level != mItems.end(); ++level, ++n)
	{
		sprintf(buf, "Level %d (%d x %d buckets, %d total)\n", n, 1 << n, 1 << n, 1 << (n*2));
#if SIM && DEV
		deverr << buf;
#else
		printf("%s", buf);
#endif
		int b = 0;	// num buckets
		int o = 0;	// Total per all buckets (for avergae)
		int m = 0;	// Max per bucket
		int w = 0;
		for (typename BucketMap::iterator bi = level->begin(); bi != level->end(); ++bi)
		{
			o += bi->second.size();
			b++;
			if (bi->second.size() >= m) { m = bi->second.size(); w = bi->first; }
		}
		if (b == 0)
			sprintf(buf, "    No buckets!\n");
		else {
			unsigned int x, y, l;
			XBucket_Decompose(w, l, x, y);
			sprintf(buf, "  %d Buckets, %d items total, average bucket size = %f, worst bucket size = %d at %d,%d,%d\n", b, o, (float) o / (float) b, m, l, x, y);
		}
#if SIM && DEV
		deverr<<buf;
#else
		printf("%s", buf);
#endif
	}
}

#endif
