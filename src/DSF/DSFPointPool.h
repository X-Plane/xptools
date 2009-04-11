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
#ifndef DSFPOINTPOOL_H
#define DSFPOINTPOOL_H

#include <vector>
#include <list>

#include "AssertUtils.h"

using namespace std;


/************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************/

/* A tuple - an N dimensional coordinate.  Real
 * handy because it can be used for any kind of DSF coordinate. */

/* Of course, in practice, variable sized arrays are way too
 * expensive to manipulate, so we're fixed sized.  Right now
 * 10 planes covers the worst case - XYZ, norm, STx2 + A */
 #define	MAX_TUPLE_LEN 10

class	DSFTuple {
public:

	DSFTuple();
	DSFTuple(int planes);
	DSFTuple(const DSFTuple& rhs);
	DSFTuple(const double * values, int length);
	~DSFTuple();

	inline DSFTuple& operator=(const DSFTuple& rhs);
	inline bool operator==(const DSFTuple& rhs) const;
	inline bool operator< (const DSFTuple& rhs) const;

	inline DSFTuple& operator += (const DSFTuple& rhs);
	inline DSFTuple& operator -= (const DSFTuple& rhs);
	inline DSFTuple& operator *= (const DSFTuple& rhs);
	inline DSFTuple& operator /= (const DSFTuple& rhs);

	inline DSFTuple operator+ (const DSFTuple& rhs) const;
	inline DSFTuple operator- (const DSFTuple& rhs) const;
	inline DSFTuple operator* (const DSFTuple& rhs) const;
	inline DSFTuple operator/ (const DSFTuple& rhs) const;

	inline bool in_range(const DSFTuple& offset, const DSFTuple& scale) const;
	inline bool encode(const DSFTuple& offset, const DSFTuple& scale);
	inline bool encode32(const DSFTuple& offset, const DSFTuple& scale);

	inline int size() const 				{ return mLen; 		}
	inline double operator[](int n) const 	{ return mData[n]; 	}
	inline double& operator[](int n) 		{ return mData[n]; 	}
	inline double * begin() 				{ return mData; 	}
	inline const double * begin() const 	{ return mData; 	}
	inline double * end() 					{ return mData+mLen;}
	inline const double * end() const 		{ return mData+mLen;}
	inline void push_back(double v);
	inline void insert(double * ptr, double v);

	void dump(void) const;
	void dumphex(void) const;

	inline size_t hash(void) const;

private:

	int		mLen;
	double	mData[MAX_TUPLE_LEN];

};

HASH_MAP_NAMESPACE_START
#if MSC
template<> inline
size_t hash_value<DSFTuple>(const DSFTuple& _Keyval)
{
	return _Keyval.hash();
}

#else
template <>
struct hash<DSFTuple> HASH_PARENT(DSFTuple, std::size_t) {
	std::size_t operator()(const DSFTuple& key) const { return key.hash(); }
};
#endif
HASH_MAP_NAMESPACE_END

typedef	vector<DSFTuple>			DSFTupleVector;
typedef list<DSFTupleVector>		DSFTupleVectorVector;

/* A shared point pool.  Every point is pooled, and the
 * points are sorted spatially.  The shared point pool
 * is really N sub-point-pools, so each point ends up
 * with a pair of indices. */

typedef	pair<int, int>	DSFPointPoolLoc;
typedef vector<DSFPointPoolLoc>	DSFPointPoolLocVector;

class	DSFSharedPointPool {
public:

	DSFSharedPointPool();
	DSFSharedPointPool(
				const DSFTuple& 		min,
				const DSFTuple& 		max);
	void			SetRange(
				const DSFTuple& 		min,
				const DSFTuple& 		max);


	void			AddPool(DSFTuple& minFrac, DSFTuple& maxFrac);

	// This returns true if the set of points can be kept as a run...
	// it tests only whether the points span subpools, and can be
	// run before any points are accepted.
	bool			CanBeContiguous(const DSFTupleVector& inPoints);

	// This routine accepts a run as a contiguous set in one pool, and
	// returns the pool and index, or -1, -1 if any of the points are
	// already in one of the point pools (and thus it should be shared).
	// You can also limit these to a single pool if you have a pool #
	// that you know is good (from above).
	DSFPointPoolLoc	AcceptContiguous(const DSFTupleVector& inPoints);
	// This routine accepts a single point, sharing if possible.
	DSFPointPoolLoc	AcceptShared(const DSFTuple& inPoint);

	void			ProcessPoints(void);
	int				MapPoolNumber(int);	// From full to used pool #s

	int				WritePoolAtoms(FILE * fi, int32_t id);
	int				WriteScaleAtoms(FILE * fi, int32_t id);

private:

	DSFTuple			mMin;
	DSFTuple			mMax;

	struct	SharedSubPool {

		DSFTuple					mOffset;
		DSFTuple					mScale;

		DSFTupleVector				mPoints;			// These are our points
		hash_map<DSFTuple, int>		mPointsIndex;		// This is used to see if we already have a point.

	};

	list<SharedSubPool>			mPools;
	vector<int>					mUsageMapping;

	DSFPointPoolLoc	AcceptContiguousPool(int pp, SharedSubPool * pool, const DSFTupleVector& inPoints);

};


/************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************/

class	DSFContiguousPointPool {
public:

	DSFContiguousPointPool();
	DSFContiguousPointPool(
				const DSFTuple& 		min,
				const DSFTuple& 		max);
	void			SetRange(
				const DSFTuple& 		min,
				const DSFTuple& 		max);

	void			AddPool(DSFTuple& minFrac, DSFTuple& maxFrac);
	DSFPointPoolLoc	AccumulatePoint(const DSFTuple& inPoint);
	DSFPointPoolLoc	AccumulatePoints(const DSFTupleVector& inPoints);
	void			ProcessPoints(void);
	int				MapPoolNumber(int);	// From full to used pool #s

	int				WritePoolAtoms(FILE * fi, int32_t id);
	int				WriteScaleAtoms(FILE * fi, int32_t id);

private:

	DSFTuple			mMin;
	DSFTuple			mMax;

	struct	ContiguousSubPool {

		DSFTuple					mOffset;
		DSFTuple					mScale;

		DSFTupleVector			mPoints;

	};

	list<ContiguousSubPool>		mPools;
	vector<int>					mUsageMapping;
};


class	DSF32BitPointPool {
public:

	DSF32BitPointPool();
	DSF32BitPointPool(
				const DSFTuple& 		min,
				const DSFTuple& 		max);
	void			SetRange(
				const DSFTuple& 		min,
				const DSFTuple& 		max);

	int				CountShared(const DSFTupleVector& inPoints);
	DSFPointPoolLoc	AcceptContiguous(const DSFTupleVector& inPoints);
	DSFPointPoolLoc	AcceptShared(const DSFTuple& inPoint);

	int				WritePoolAtoms(FILE * fi, int32_t id);
	int				WriteScaleAtoms(FILE * fi, int32_t id);

private:

	DSFTuple			mMin;
	DSFTuple			mMax;

	DSFTuple					mOffset;
	DSFTuple					mScale;

	DSFTupleVector				mPoints;			// These are our points
	hash_map<DSFTuple, int>		mPointsIndex;		// This is used to see if we already have a point.

};

/************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************/

struct DSFPrimitive {
	int					kind;
	DSFTupleVector		vertices;
};

void DSFOptimizePrimitives(
					vector<DSFPrimitive>& io_primitives);

/************************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************************/

#pragma mark -

inline DSFTuple::DSFTuple() : mLen(0)
{
}

inline DSFTuple::DSFTuple(int planes) : mLen(planes)
{
	if (planes > MAX_TUPLE_LEN)
		AssertPrintf( "ERROR: overrun DSF tuple.\n");
	double * d = mData;
	while (planes--)
		*d++ = 0.0;
}

inline DSFTuple::DSFTuple(const DSFTuple& rhs) : mLen(rhs.mLen)
{
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
		*d1++ = *d2++;
}

inline DSFTuple::DSFTuple(const double * values, int length) : mLen(length)
{
	double * d1 = mData;
	const double * d2 = values;
	while (length--)
		*d1++ = *d2++;
}

inline DSFTuple::~DSFTuple()
{
}

inline DSFTuple& DSFTuple::operator=(const DSFTuple& rhs)
{
	mLen = rhs.mLen;
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
		*d1++ = *d2++;
	return *this;
}

inline bool DSFTuple::operator==(const DSFTuple& rhs) const
{
	if (mLen != rhs.mLen) return false;
	const double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
		if (*d1++ != *d2++) return false;
	return true;
}

inline bool DSFTuple::operator< (const DSFTuple& rhs) const
{
	if (mLen < rhs.mLen) return true;
	if (mLen > rhs.mLen) return false;
	const double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
	{
		if (*d1 < *d2) return true;
		if (*d1 > *d2) return false;
		++d1, ++d2;
	}
	return false;
}

inline DSFTuple& DSFTuple::operator += (const DSFTuple& rhs)
{
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
	{
		(*d1++) += (*d2++);
	}
	return *this;
}

inline DSFTuple& DSFTuple::operator -= (const DSFTuple& rhs)
{
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
	{
		(*d1++) -= (*d2++);
	}
	return *this;
}

inline DSFTuple& DSFTuple::operator *= (const DSFTuple& rhs)
{
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
	{
		(*d1++) *= (*d2++);
	}
	return *this;
}

inline DSFTuple& DSFTuple::operator /= (const DSFTuple& rhs)
{
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
	{
		(*d1++) /= (*d2++);
	}
	return *this;
}

inline DSFTuple DSFTuple::operator+ (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me += rhs;
	return me;
}

inline DSFTuple DSFTuple::operator- (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me -= rhs;
	return me;
}

inline DSFTuple DSFTuple::operator* (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me *= rhs;
	return me;
}

inline DSFTuple DSFTuple::operator/ (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me /= rhs;
	return me;
}

inline bool DSFTuple::in_range(const DSFTuple& offset, const DSFTuple& scale) const
{
	if (size() != offset.size()) return false;
	if (size() != scale.size()) return false;

	const double * i = mData;
	const double * j = offset.mData;
	const double * k = scale.mData;
	int c = mLen;
	while (c--)
	{
		if (*i < *j) return false;
		if (*k != 0 && *i > (*j + *k)) return false;
		if (*k == 0 && *i > 65535.0) return false;
		++i, ++j, ++k;
	}
	return true;
}

inline bool DSFTuple::encode(const DSFTuple& offset, const DSFTuple& scale)
{
	if (size() != offset.size()) return false;
	if (size() != scale.size()) return false;

	double * i = mData;
	const double * j = offset.mData;
	const double * k = scale.mData;
	int c = mLen;
	while (c--)
	{
		if (*k)
//		printf("   %lf   ->   ", *i);
			*i = ((*i - *j) * 65535.0 / (*k) );
//		printf("   %lf\n", *i);
		if (*i < 0.0 || *i > 65535.0)
			return false;
		++i, ++j, ++k;
	}
	return true;
}


inline bool DSFTuple::encode32(const DSFTuple& offset, const DSFTuple& scale)
{
	if (size() != offset.size()) return false;
	if (size() != scale.size()) return false;

#if DEV
	DSFTuple	backup(*this);
#endif
	double * i = mData;
	const double * j = offset.mData;
	const double * k = scale.mData;
	int c = mLen;
	while (c--)
	{
		if (*k)
			*i = ((*i - *j) * 4294967295.0 / (*k) );
		if (*i < 0.0 || *i > 4294967295.0)
		{
#if DEV
			*this = backup;
#endif
			return false;
		}
		++i, ++j, ++k;
	}
	return true;
}

inline void DSFTuple::push_back(double v)
{
#if DEV
	if (mLen == MAX_TUPLE_LEN)
		AssertPrintf( "ERROR: overrun DSF tuple.\n");
#endif
	mData[mLen++] = v;
}

inline void DSFTuple::insert(double * ptr, double v)
{
#if DEV
	if (mLen == MAX_TUPLE_LEN)
		AssertPrintf( "ERROR: overrun DSF tuple.\n");
#endif
	int index = ptr - mData;
	if (index < mLen)
		memmove(ptr + sizeof(double), ptr, (mLen - index) * sizeof(double));
	*ptr = v;
	++mLen;
}


inline void DSFTuple::dump(void) const
{
	if (mLen < 0 || mLen > MAX_TUPLE_LEN)
		printf("Tuple bad length: %d", mLen);
	else
	for (int n = 0; n < mLen; ++n)
		printf("%c%lf",(n==0) ? ' ' : ',', mData[n]);
}

inline void DSFTuple::dumphex(void) const
{
	if (mLen < 0 || mLen > MAX_TUPLE_LEN)
		printf("Tuple bad length: %d", mLen);
	else
	for (int n = 0; n < mLen; ++n)
		printf("%c%lx%lx",(n==0) ? ' ' : ',', mData[n]);
}

inline size_t DSFTuple::hash(void) const
{
	size_t	ret = 0;
	size_t * p = (size_t *) mData;
	int words = mLen;

	while (words--)
	{
		ret = (ret << 5) ^ (ret >> (27)) ^ *p++;
		ret = (ret << 7) ^ (ret >> 25) ^ *p++;
	}
	return ret;
/*
	double res = 1.0;
	int words = mLen;
	const double * p = mData;
	while (words--)
	if (*p != 0.0)
		res *= *p++;

	size_t * pp = (size_t *) &res;
	size_t ret = *pp++;
	ret ^= *pp;
	return ret;
*/
}


#endif


