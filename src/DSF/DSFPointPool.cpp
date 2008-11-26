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
#include "DSFLib.h"
#include "PVRTTriStrip.h"

#include <utility>
using std::pair;



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
	for (list<SharedSubPool>::iterator p = mPools.begin(); p != mPools.end(); ++p)
	{
		bool ok = true;
		for (int n = 0; n < inPoints.size(); ++n)
		{
			if (!inPoints[n].in_range(p->mOffset, p->mScale))
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
	int p = 0;
	SharedSubPool * found;

	for (list <SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool, ++p)
	{
		bool ok = true;
		encoded.clear();
		for (n = 0; n < inPoints.size(); ++n)
		{
			encoded.push_back(inPoints[n]);
			if (!encoded.back().encode(pool->mOffset, pool->mScale))
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
				if (pool->mPointsIndex.find(encoded[n]) !=
					pool->mPointsIndex.end())
				{
					return pair<int,int>(-1,-1);
				}
			}
			found = &*pool;
			first_ok_pool = p;
		}
	}
	if (first_ok_pool != -1)
		return AcceptContiguousPool(first_ok_pool, found, inPoints);
	return pair<int,int>(-1, -1);
}

pair<int, int>	DSFSharedPointPool::AcceptContiguousPool(int p, SharedSubPool * pool, const DSFTupleVector& inPoints)
{
	int n;
	pair<int,int> retval(p, pool->mPoints.size());
	for (n = 0; n < inPoints.size(); ++n)
	{
		DSFTuple	pt(inPoints[n]);
		pt.encode(pool->mOffset,pool->mScale);
		pool->mPointsIndex.insert(hash_map<DSFTuple, int>::value_type(
					pt, pool->mPoints.size()));
		pool->mPoints.push_back(pt);
	}
	return retval;
}

pair<int, int>	DSFSharedPointPool::AcceptShared(const DSFTuple& inPoint)
{
	int p = 0;
	// First check every scale for the point already existing.
	for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool, ++p)
	{
		DSFTuple	point(inPoint);
		if (point.encode(pool->mOffset, pool->mScale))
		{
			hash_map<DSFTuple,int>::iterator iter = pool->mPointsIndex.find(point);
			if (iter != pool->mPointsIndex.end())
				return pair<int,int>(p, iter->second);
		}
	}
	// Hrm...doesn't exist.  Try to add it.
	p = 0;
	for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool, ++p)
	{
		DSFTuple	point(inPoint);
		if (point.encode(pool->mOffset, pool->mScale))
		{
			int our_pos = pool->mPoints.size();
			pool->mPoints.push_back(point);
			pool->mPointsIndex.insert(hash_map<DSFTuple, int>::value_type(point, our_pos));
			return pair<int, int>(p, our_pos);
		}
	}
	return pair<int, int>(-1, -1);
}


void			DSFSharedPointPool::ProcessPoints(void)
{
	int new_p = 0;
	for (list<SharedSubPool>::iterator i = mPools.begin(); i != mPools.end(); )
	{
/*
		map<int, int> counts;
		for (hash_map<DSFTuple, int>::const_iterator c = i->mPointsIndex.begin(); c != i->mPointsIndex.end(); ++c)
		{
			counts[i->mPointsIndex.collision(c)]++;
		}
		printf("32 bit pool buckets = %d pts = %d\n", i->mPointsIndex.bucket_count(), i->mPointsIndex.size());
		for (map<int, int>::iterator j = counts.begin(); j != counts.end(); ++j)
		{
			printf("   %d loading     %d times\n", j->first, j->second);
		}
*/

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
	for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	poolAtom(fi, id);
		vector<unsigned short>	shorts;
		for (DSFTupleVector::iterator i = pool->mPoints.begin();
			i != pool->mPoints.end(); ++i)
		{
			for (int j = 0; j < i->size(); ++j)
			{
				shorts.push_back((*i)[j]);
			}
		}
		WritePlanarNumericAtomShort(fi, pool->mScale.size(), pool->mPoints.size(), xpna_Mode_RLE_Differenced, 1, (short *) &*shorts.begin());
	}
	return mPools.size();
}

int			DSFSharedPointPool::WriteScaleAtoms(FILE * fi, int id)
{
	for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	scaleAtom(fi, id);
		for (int d = 0; d < pool->mScale.size(); ++d)
		{
			WriteFloat32(fi, pool->mScale[d]);
			WriteFloat32(fi, pool->mOffset[d]);
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
	int p = 0;
	int tris = 0;
	int n;
	for (list<ContiguousSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool, ++p)
	if ((pool->mPoints.size() + inPoints.size()) <= 65535)
	{
		++tris;
		bool	ok = true;
		for (n = 0; n < inPoints.size(); ++n)
		{
			if (!inPoints[n].in_range(pool->mOffset, pool->mScale)) {
				ok = false; break;
			}
		}
		if (ok)
		{
			DSFTupleVector	trans(inPoints);
			for (int n = 0; n < trans.size(); ++n)
				trans[n].encode(pool->mOffset, pool->mScale);
			int pos = pool->mPoints.size();
			pool->mPoints.insert(pool->mPoints.end(), trans.begin(), trans.end());
			return pair<int, int>(p, pos);
		}
	}
	printf("point pool failure...%d out of %d tried.  Points = %d.\n", tris, p, inPoints.size());
		inPoints[n].dump();
	printf("\n");
	return pair<int, int>(-1, -1);
}

void			DSFContiguousPointPool::ProcessPoints(void)
{
	int new_p = 0;
	for (list<ContiguousSubPool>::iterator i = mPools.begin(); i != mPools.end(); )
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
	for (list<ContiguousSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	poolAtom(fi, id);
		vector<unsigned short>	shorts;
		for (DSFTupleVector::iterator i = pool->mPoints.begin();
			i != pool->mPoints.end(); ++i)
		{
			for (int j = 0; j < i->size(); ++j)
			{
				shorts.push_back((*i)[j]);
//				printf("  %04X", shorts.back());
			}
		}
//		printf("\n");
		WritePlanarNumericAtomShort(fi, pool->mScale.size(), pool->mPoints.size(), xpna_Mode_RLE_Differenced, 1, (short *) &*shorts.begin());
	}
	return mPools.size();
}

int			DSFContiguousPointPool::WriteScaleAtoms(FILE * fi, int id)
{
	for (list<ContiguousSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	scaleAtom(fi, id);
		for (int d = 0; d < pool->mScale.size(); ++d)
		{
			WriteFloat32(fi, pool->mScale[d]);
			WriteFloat32(fi, pool->mOffset[d]);
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
		{
			return DSFPointPoolLoc(-1, -1);
		}

		mPointsIndex.insert(hash_map<DSFTuple, int>::value_type(pt, mPoints.size()));
		mPoints.push_back(pt);
	}
	return result;
}

DSFPointPoolLoc	DSF32BitPointPool::AcceptShared(const DSFTuple& inPoint)
{
	DSFTuple	pt(inPoint);
	if (!pt.encode32(mOffset, mScale))
		return DSFPointPoolLoc(-1, -1);

	hash_map<DSFTuple, int>::iterator iter = mPointsIndex.find(pt);
	if (iter != mPointsIndex.end())
		return DSFPointPoolLoc(0, iter->second);

	DSFPointPoolLoc	result(0, mPoints.size());
	mPointsIndex.insert(hash_map<DSFTuple, int>::value_type(pt, mPoints.size()));
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

bool is_strip(unsigned short * idx, int n)
{
	if (n % 2)
	{	//				   0 1 2    3 4 5
		// second tri, so: 2 1 3	2 3 4
		return idx[0] == idx[3] &&
			   idx[2] == idx[4];
	} 
	else
	{	//				  0 1 2     3 4 5
		// first tri, so: 0 1 2		2 1 3
		return idx[1] == idx[4] &&
			   idx[2] == idx[3];
	}
	
}

static unsigned short * strip_break(unsigned short * idx_start, unsigned short * idx_end)
{
	int ct = 0;
	idx_start += 3;
	while(idx_start < idx_end && is_strip(idx_start-3, ct))
	{
		idx_start += 3;
		++ct;
	}
	return idx_start;
}


void DSFOptimizePrimitives(
					vector<DSFPrimitive>& io_primitives)
{
	typedef	hash_map<DSFTuple, int>		idx_t;
	vector<DSFPrimitive>				out_prims;
	idx_t								point_index;
	vector<DSFTuple>					vertices;
	vector<unsigned short>				indices;
	
	for(vector<DSFPrimitive>::iterator p = io_primitives.begin(); p != io_primitives.end(); ++p)
	if(p->kind == dsf_Tri)
	{
		for(DSFTupleVector::iterator v = p->vertices.begin(); v != p->vertices.end(); ++v)
		{
			idx_t::iterator idx_ref = point_index.find(*v);
			if(idx_ref == point_index.end())
			{
				indices.push_back(vertices.size());
				vertices.push_back(*v);
				point_index.insert(idx_t::value_type(*v,vertices.size()-1));
			}
			else
			{
				indices.push_back(idx_ref->second);
			}
		}
	}
	else
		out_prims.push_back(*p);

	PVRTTriStripList(
		&*indices.begin(),
		indices.size() / 3);


	vector<unsigned short> pure_tris;
	
	unsigned short * s = &*indices.begin(), * e = &*indices.end();
	while(s != e)
	{
		unsigned short * b = strip_break(s,e);
		if((b - s) > 3)
		{
			
			out_prims.push_back(DSFPrimitive());
			out_prims.back().kind = dsf_TriStrip;
			out_prims.back().vertices.push_back(vertices[s[0]]);
			out_prims.back().vertices.push_back(vertices[s[1]]);
			out_prims.back().vertices.push_back(vertices[s[2]]);
			s += 3;
			bool odd = true;
			while(s != b)
			{
				if(odd)		out_prims.back().vertices.push_back(vertices[s[2]]);
				else		out_prims.back().vertices.push_back(vertices[s[2]]);
				odd = !odd;
				
				s += 3;
			}
		}
		else
		{
			pure_tris.insert(pure_tris.end(),s,b);
		}
		s=b;
	}
	
	while(!pure_tris.empty())
	{
		int n = min(pure_tris.size(),255UL);
		out_prims.push_back(DSFPrimitive());
		out_prims.back().kind = dsf_Tri;
		for(int i = 0; i < n; ++i)
		{
			out_prims.back().vertices.push_back(vertices[pure_tris[i]]);
		}
		pure_tris.erase(pure_tris.begin(),pure_tris.begin()+n);
	}
	
	swap(io_primitives,out_prims);
}

