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
#include "DSFPointPool.h"
#include "XChunkyFileUtils.h"
#include <utility>

using std::pair;

DSFTuple::DSFTuple() : mLen(0)
{
}

DSFTuple::DSFTuple(int planes) : mLen(planes)
{
	if (planes > MAX_TUPLE_LEN)
		fprintf(stderr, "ERROR: overrun DSF tuple.\n");
	double * d = mData;
	while (planes--)
		*d++ = 0.0;
}

DSFTuple::DSFTuple(const DSFTuple& rhs) : mLen(rhs.mLen)
{
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
		*d1++ = *d2++;
}

DSFTuple::DSFTuple(const double * values, int length) : mLen(length)
{
	double * d1 = mData;
	const double * d2 = values;
	while (length--)
		*d1++ = *d2++;
}

DSFTuple::~DSFTuple()
{
}
	
DSFTuple& DSFTuple::operator=(const DSFTuple& rhs)
{
	mLen = rhs.mLen;
	double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
		*d1++ = *d2++;
	return *this;
}

bool DSFTuple::operator==(const DSFTuple& rhs) const
{
	if (mLen != rhs.mLen) return false;
	const double * d1 = mData;
	const double * d2 = rhs.mData;
	int c = mLen;
	while (c--)
		if (*d1++ != *d2++) return false;
	return true;
}

bool DSFTuple::operator< (const DSFTuple& rhs) const
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

DSFTuple& DSFTuple::operator += (const DSFTuple& rhs)
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

DSFTuple& DSFTuple::operator -= (const DSFTuple& rhs)
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

DSFTuple& DSFTuple::operator *= (const DSFTuple& rhs)
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

DSFTuple& DSFTuple::operator /= (const DSFTuple& rhs)
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

DSFTuple DSFTuple::operator+ (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me += rhs;
	return me;
}

DSFTuple DSFTuple::operator- (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me -= rhs;
	return me;
}

DSFTuple DSFTuple::operator* (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me *= rhs;
	return me;
}

DSFTuple DSFTuple::operator/ (const DSFTuple& rhs) const
{
	DSFTuple	me(*this);
	me /= rhs;
	return me;
}

bool DSFTuple::in_range(const DSFTuple& offset, const DSFTuple& scale) const
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
		if (*i > (*j + *k)) return false;
		++i, ++j, ++k;
	}
	return true;
}

bool DSFTuple::encode(const DSFTuple& offset, const DSFTuple& scale)
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


bool DSFTuple::encode32(const DSFTuple& offset, const DSFTuple& scale)
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
			*i = ((*i - *j) * 4294967295.0 / (*k) );
//		printf("   %lf\n", *i);
		if (*i < 0.0 || *i > 4294967295.0)
			return false;
		++i, ++j, ++k;
	}
	return true;
}

void DSFTuple::push_back(double v)
{
	if (mLen == MAX_TUPLE_LEN)
		fprintf(stderr, "ERROR: overrun DSF tuple.\n");

	mData[mLen++] = v;
}

void DSFTuple::insert(double * ptr, double v)
{
	if (mLen == MAX_TUPLE_LEN)
		fprintf(stderr, "ERROR: overrun DSF tuple.\n");
	int index = ptr - mData;
	if (index < mLen)
		memmove(ptr + sizeof(double), ptr, (mLen - index) * sizeof(double));
	*ptr = v;
	++mLen;
}

void DSFTuple::dump(void)
{
	if (mLen < 0 || mLen > MAX_TUPLE_LEN)
		printf("Tuple bad length: %d", mLen);
	else
	for (int n = 0; n < mLen; ++n)
		printf("%c%lf",(n==0) ? ' ' : ',', mData[n]);
}

void DSFTuple::dumphex(void)
{
	if (mLen < 0 || mLen > MAX_TUPLE_LEN)
		printf("Tuple bad length: %d", mLen);
	else
	for (int n = 0; n < mLen; ++n)
		printf("%c%lx%lx",(n==0) ? ' ' : ',', mData[n]);
}

#pragma mark -

DSFSharedPointPool::DSFSharedPointPool()
{
}


DSFSharedPointPool::DSFSharedPointPool(
				const DSFTuple& 		min,
				const DSFTuple& 		max)
{
	mMin = min;
	mMax = max;
}

void DSFSharedPointPool::SetRange(
				const DSFTuple& 		min,
				const DSFTuple& 		max)
{
	mMin = min;
	mMax = max;
}

void			DSFSharedPointPool::AddPool(DSFTuple& minFrac, DSFTuple& maxFrac)
{
	DSFTuple	submin = mMin + minFrac * (mMax - mMin);
	DSFTuple	submax = mMin + maxFrac * (mMax - mMin);
	
	mPools.push_back(SharedSubPool());
	mPools.back().mOffset = submin;
	mPools.back().mScale = submax - submin;	
}
	
bool			DSFSharedPointPool::CanBeContiguous(const DSFTupleVector& inPoints)
{
	for (int p = 0; p < mPools.size(); ++p)
	{
		bool ok = true;
		for (int n = 0; n < inPoints.size(); ++n)
		{
			if (!inPoints[n].in_range(mPools[p].mOffset, mPools[p].mScale))
			{
				ok = false;
				break;
			}
		}
		if (ok)
		{
			return true;
		}
	}
	return false;
}


pair<int, int>	DSFSharedPointPool::AcceptContiguous(const DSFTupleVector& inPoints)
{
	int n;
	DSFTupleVector	encoded;
	int	first_ok_pool = -1;
	for (int p = 0; p < mPools.size(); ++p)
	{
		bool ok = true;
		encoded.clear();
		for (n = 0; n < inPoints.size(); ++n)
		{
			encoded.push_back(inPoints[n]);
			if (!encoded.back().encode(mPools[p].mOffset, mPools[p].mScale))
			{
				ok = false;
				break;
			}
		}
		if (ok)
		{
			// This is the first pool we've found where we at least could 
			// all fit.  Check for sharing.
			for (n = 0; n < encoded.size(); ++n)
			{
				if (mPools[p].mPointsIndex.find(encoded[n]) !=
					mPools[p].mPointsIndex.end())
				{
					return pair<int,int>(-1,-1);
				}
			}
			first_ok_pool = p;
		}
	}
	if (first_ok_pool != -1)
		return AcceptContiguousPool(first_ok_pool, inPoints);
	return pair<int,int>(-1, -1);
}

pair<int, int>	DSFSharedPointPool::AcceptContiguousPool(int p, const DSFTupleVector& inPoints)
{
	int n;
	pair<int,int> retval(p, mPools[p].mPoints.size());
	for (n = 0; n < inPoints.size(); ++n)
	{
		DSFTuple	pt(inPoints[n]);
		pt.encode(mPools[p].mOffset,mPools[p].mScale);
		mPools[p].mPointsIndex.insert(map<DSFTuple, int>::value_type(
					pt, mPools[p].mPoints.size()));
		mPools[p].mPoints.push_back(pt);
	}
	return retval;
}

pair<int, int>	DSFSharedPointPool::AcceptShared(const DSFTuple& inPoint)
{
	int p;
	// First check every scale for the point already existing.
	for (p = 0; p < mPools.size(); ++p)
	{
		DSFTuple	point(inPoint);
		if (point.encode(mPools[p].mOffset, mPools[p].mScale))
		{
			map<DSFTuple,int>::iterator iter = mPools[p].mPointsIndex.find(point);
			if (iter != mPools[p].mPointsIndex.end())
				return pair<int,int>(p, iter->second);
		}
	}
	// Hrm...doesn't exist.  Try to add it.
	for (p = 0; p < mPools.size(); ++p)
	{
		DSFTuple	point(inPoint);
		if (point.encode(mPools[p].mOffset, mPools[p].mScale))
		{
			int our_pos = mPools[p].mPoints.size();
			mPools[p].mPoints.push_back(point);
			mPools[p].mPointsIndex.insert(map<DSFTuple, int>::value_type(point, our_pos));
			return pair<int, int>(p, our_pos);
		}
	}
	return pair<int, int>(-1, -1);
}


void			DSFSharedPointPool::ProcessPoints(void)
{
	int new_p = 0;
	for (vector<SharedSubPool>::iterator i = mPools.begin(); i != mPools.end(); )
	{
		if (i->mPoints.empty())
		{
			i = mPools.erase(i);
			mUsageMapping.push_back(-1);
		} else {
			mUsageMapping.push_back(new_p);
			++i;
			++new_p;
		}
	}
}

int				DSFSharedPointPool::MapPoolNumber(int n)
{
	return mUsageMapping[n];
}

int			DSFSharedPointPool::WritePoolAtoms(FILE * fi, int id)
{
	for (int p = 0; p < mPools.size(); ++p)
	{
		StAtomWriter	poolAtom(fi, id);
		vector<unsigned short>	shorts;
		for (DSFTupleVector::iterator i = mPools[p].mPoints.begin();
			i != mPools[p].mPoints.end(); ++i)
		{
			for (int j = 0; j < i->size(); ++j)
			{
				shorts.push_back((*i)[j]);
			}
		}
		WritePlanarNumericAtomShort(fi, mPools[p].mScale.size(), mPools[p].mPoints.size(), xpna_Mode_RLE_Differenced, 1, (short *) &*shorts.begin());
	}			
	return mPools.size();			
}

int			DSFSharedPointPool::WriteScaleAtoms(FILE * fi, int id)
{
	for (int p = 0; p < mPools.size(); ++p)
	{
		StAtomWriter	scaleAtom(fi, id);
		for (int d = 0; d < mPools[p].mScale.size(); ++d)
		{
			WriteFloat32(fi, mPools[p].mScale[d]);
			WriteFloat32(fi, mPools[p].mOffset[d]);
		}
	}
	return mPools.size();			
}

#pragma mark -

DSFContiguousPointPool::DSFContiguousPointPool()
{
}

DSFContiguousPointPool::DSFContiguousPointPool(
			const DSFTuple& 		min,
			const DSFTuple& 		max)
{
	mMin = min;
	mMax = max;
}

void DSFContiguousPointPool::SetRange(
			const DSFTuple& 		min,
			const DSFTuple& 		max)
{
	mMin = min;
	mMax = max;
}

void			DSFContiguousPointPool::AddPool(DSFTuple& minFrac, DSFTuple& maxFrac)
{
	DSFTuple	submin = mMin + minFrac * (mMax - mMin);
	DSFTuple	submax = mMin + maxFrac * (mMax - mMin);
	
	mPools.push_back(ContiguousSubPool());
	mPools.back().mOffset = submin;
	mPools.back().mScale = submax - submin;	
}

pair<int, int>	DSFContiguousPointPool::AccumulatePoint(const DSFTuple& inPoint)
{
	DSFTupleVector	v;
	v.push_back(inPoint);
	return AccumulatePoints(v);
}

pair<int, int>	DSFContiguousPointPool::AccumulatePoints(const DSFTupleVector& inPoints)
{
	for (int p = 0; p < mPools.size(); ++p)
	if ((mPools[p].mPoints.size() + inPoints.size()) <= 65535)
	{
		bool	ok = true;
		for (int n = 0; n < inPoints.size(); ++n)
		{
			if (!inPoints[n].in_range(mPools[p].mOffset, mPools[p].mScale)) {
				ok = false; break; }			
		}
		if (ok)
		{
			DSFTupleVector	trans(inPoints);
			for (int n = 0; n < trans.size(); ++n)
				trans[n].encode(mPools[p].mOffset, mPools[p].mScale);
			int pos = mPools[p].mPoints.size();
			mPools[p].mPoints.insert(mPools[p].mPoints.end(), trans.begin(), trans.end());
			return pair<int, int>(p, pos);
		}
	}		
	return pair<int, int>(-1, -1);
}

void			DSFContiguousPointPool::ProcessPoints(void)
{
	int new_p = 0;
	for (vector<ContiguousSubPool>::iterator i = mPools.begin(); i != mPools.end(); )
	{
		if (i->mPoints.empty())
		{
			i = mPools.erase(i);
			mUsageMapping.push_back(-1);
		} else {
			mUsageMapping.push_back(new_p);
			++i;
			++new_p;
		}
	}
}

int				DSFContiguousPointPool::MapPoolNumber(int n)
{
	return mUsageMapping[n];
}

int			DSFContiguousPointPool::WritePoolAtoms(FILE * fi, int id)
{
	for (int p = 0; p < mPools.size(); ++p)
	{
		StAtomWriter	poolAtom(fi, id);
		vector<unsigned short>	shorts;
		for (DSFTupleVector::iterator i = mPools[p].mPoints.begin();
			i != mPools[p].mPoints.end(); ++i)
		{
			for (int j = 0; j < i->size(); ++j)
			{
				shorts.push_back((*i)[j]);
//				printf("  %04X", shorts.back());
			}
		}
//		printf("\n");
		WritePlanarNumericAtomShort(fi, mPools[p].mScale.size(), mPools[p].mPoints.size(), xpna_Mode_RLE_Differenced, 1, (short *) &*shorts.begin());
	}
	return mPools.size();			
}

int			DSFContiguousPointPool::WriteScaleAtoms(FILE * fi, int id)
{
	for (int p = 0; p < mPools.size(); ++p)
	{
		StAtomWriter	scaleAtom(fi, id);
		for (int d = 0; d < mPools[p].mScale.size(); ++d)
		{
			WriteFloat32(fi, mPools[p].mScale[d]);
			WriteFloat32(fi, mPools[p].mOffset[d]);
		}
	}
	return mPools.size();				
}

#pragma mark -


DSF32BitPointPool::DSF32BitPointPool()
{
}

DSF32BitPointPool::DSF32BitPointPool(
			const DSFTuple& 		min,
			const DSFTuple& 		max)
{
	SetRange(min, max);
}

void			DSF32BitPointPool::SetRange(
			const DSFTuple& 		min,
			const DSFTuple& 		max)
{
	mMin = min; mMax = max;
	mOffset = min;
	mScale = mMax - mMin;
}			

int				DSF32BitPointPool::CountShared(const DSFTupleVector& inPoints)
{
	int count = 0;
	for (int n = 0; n < inPoints.size(); ++n)
	{
		DSFTuple	pt(inPoints[n]);
		if (!pt.encode32(mOffset, mScale))
			return -1;
		if (mPointsIndex.find(pt) != mPointsIndex.end())
			++count;
	}
	return count;
}

DSFPointPoolLoc	DSF32BitPointPool::AcceptContiguous(const DSFTupleVector& inPoints)
{
	DSFPointPoolLoc	result(0, mPoints.size());
	for (int n = 0; n < inPoints.size(); ++n)
	{
		DSFTuple	pt(inPoints[n]);
		if (!pt.encode32(mOffset, mScale))
			return DSFPointPoolLoc(-1, -1);
			
		mPointsIndex.insert(map<DSFTuple, int>::value_type(pt, mPoints.size()));
		mPoints.push_back(pt);
	}
	return result;
}

DSFPointPoolLoc	DSF32BitPointPool::AcceptShared(const DSFTuple& inPoint)
{
	DSFTuple	pt(inPoint);
	if (!pt.encode32(mOffset, mScale)) 
		return DSFPointPoolLoc(-1, -1);
	
	map<DSFTuple, int>::iterator iter = mPointsIndex.find(pt);
	if (iter != mPointsIndex.end())
		return DSFPointPoolLoc(0, iter->second);

	DSFPointPoolLoc	result(0, mPoints.size());
	mPointsIndex.insert(map<DSFTuple, int>::value_type(pt, mPoints.size()));
	mPoints.push_back(pt);
	return result;
}

int				DSF32BitPointPool::WritePoolAtoms(FILE * fi, int id)
{
	StAtomWriter	poolAtom(fi, id);
	vector<unsigned long>	longs;
	for (DSFTupleVector::iterator i = mPoints.begin();
		i != mPoints.end(); ++i)
	{
		for (int j = 0; j < i->size(); ++j)
		{
			longs.push_back((*i)[j]);
		}
	}
	WritePlanarNumericAtomInt(fi, mScale.size(), mPoints.size(), xpna_Mode_RLE_Differenced, 1, (int *) &*longs.begin());

	return 1;
}

int				DSF32BitPointPool::WriteScaleAtoms(FILE * fi, int id)
{
	StAtomWriter	scaleAtom(fi, id);
	for (int d = 0; d < mScale.size(); ++d)
	{
		WriteFloat32(fi, mScale[d]);
		WriteFloat32(fi, mOffset[d]);
	}
	return 1;
}

	
