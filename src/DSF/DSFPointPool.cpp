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

#define USE_PVRTC 0

#if USE_PVRTC
#include "PVRTTriStrip.h"
#else
#include "stdafx.h"
#include "tri_stripper.h"
using namespace	triangle_stripper;
#endif
#include <utility>
using std::pair;

#if __cplusplus <= 199711L
	#define DATA(container) ((container).empty() ? NULL : &(container)[0])
#else
	#define DATA(container) (container).data()
#endif


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

void			DSFSharedPointPool::AddPoolDirect(DSFTuple& minFrac, DSFTuple& maxFrac)
{
	DSFTuple	submin = minFrac;
	DSFTuple	submax = maxFrac;

	mPools.push_back(SharedSubPool());
	mPools.back().mOffset = submin;
	mPools.back().mScale = submax - submin;
}

bool			DSFSharedPointPool::CanBeContiguous(const DSFTupleVector& inPoints)
{
	for (list<SharedSubPool>::iterator p = mPools.begin(); p != mPools.end(); ++p)
	{
		// 65535?  yes, really.  The damn cross pool primitive uses [) notation, so it loses 1 unit capacity.
		if((p->mPoints.size() + inPoints.size()) > 65535)
			continue;
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
	SharedSubPool * found = NULL;

	for (list <SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool, ++p)
	{
		if((pool->mPoints.size() + inPoints.size()) > 65535)
		{
			//printf("Skipping full pool, pool has %d, we need to sink %d.\n", pool->mPoints.size(), inPoints.size());
			continue;
		}
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

int	DSFSharedPointPool::CountShared(const DSFTupleVector& inPoints)
{
	int c = 0;
	for(int n = 0; n < inPoints.size(); ++n)
	{
		// First check every scale for the point already existing.
		for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
		{
			DSFTuple	point(inPoints[n]);
			if (point.encode(pool->mOffset, pool->mScale))
			{
				hash_map<DSFTuple,int>::iterator iter = pool->mPointsIndex.find(point);
				if (iter != pool->mPointsIndex.end())
					++c;
			}
		}
	}
	return c;
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
	
	list<SharedSubPool>::iterator exemplar = mPools.end();
	for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool, ++p)
	{
		DSFTuple	point(inPoint);
		if (point.encode(pool->mOffset, pool->mScale))
		{
			if(pool->mPoints.size() < 65535)
			{
				int our_pos = pool->mPoints.size();
				pool->mPoints.push_back(point);
				pool->mPointsIndex.insert(hash_map<DSFTuple, int>::value_type(point, our_pos));
				return pair<int, int>(p, our_pos);
			}
			else if(exemplar == mPools.end())
				exemplar = pool;
		}
	}
	
	if(exemplar != mPools.end())
	{
		DSFTuple	point(inPoint);
		if (!point.encode(exemplar->mOffset, exemplar->mScale))
			Assert(!"Failure to re-encode into copied pool. This should never happen.");

		mPools.push_back(SharedSubPool());
		mPools.back().mOffset = exemplar->mOffset;
		mPools.back().mScale = exemplar->mScale;

		exemplar = mPools.end();
		--exemplar;

		int our_pos = exemplar->mPoints.size();
		exemplar->mPoints.push_back(point);
		exemplar->mPointsIndex.insert(hash_map<DSFTuple, int>::value_type(point, our_pos));
		return pair<int, int>(mPools.size()-1, our_pos);
	}

	// We hit this encode failure if we are out of pool bounds.
	return pair<int, int>(-1, -1);
}

void			DSFSharedPointPool::Trim(void)
{
	for (list<SharedSubPool>::iterator i = mPools.begin(); i != mPools.end(); ++i)
		trim(i->mPoints);
}

int				DSFSharedPointPool::Count() const
{
	int t = 0;
	for (list<SharedSubPool>::const_iterator i = mPools.begin(); i != mPools.end(); ++i)
		t += (i->mPoints.size());
	return t;
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

int			DSFSharedPointPool::WritePoolAtoms(FILE * fi, int32_t id)
{
	#if DSF_WRITE_STATS
		printf("Shared pool of depth %d\n", mMin.size());
		StFileSizeDebugger how_big(fi,"shared point pool total");
	#endif

	for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	poolAtom(fi, id, true);
		vector<uint16_t>	shorts;
		for (DSFTupleVector::iterator i = pool->mPoints.begin();
			i != pool->mPoints.end(); ++i)
		{
			for (int j = 0; j < i->size(); ++j)
			{
				shorts.push_back((*i)[j]);
			}
		}
		WritePlanarNumericAtomShort(fi, pool->mScale.size(), pool->mPoints.size(), xpna_Mode_RLE_Differenced, 1, (int16_t *) DATA(shorts));
	}
	return mPools.size();
}

int			DSFSharedPointPool::WriteScaleAtoms(FILE * fi, int32_t id)
{
	for (list<SharedSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	scaleAtom(fi, id, true);
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

void			DSFContiguousPointPool::AddPoolDirect(DSFTuple& minFrac, DSFTuple& maxFrac)
{
	DSFTuple	submin = minFrac;
	DSFTuple	submax = maxFrac;

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
			{
//				printf("%d: ",n);
//				trans[n].dump();
//				printf("\n");
				// BEN SAYS: always hard assert if we fail to encode..this indicates some kind of fubar rounding issue (hence we said we
				// COULD encode in in_range).  If we just bail out, we end up with half-encoded points, which sometimes will fail to sink, 
				// but ohter times will just leave junk data in the DSF, which is LOTS OF FUN to debug.
				if(!trans[n].encode(pool->mOffset, pool->mScale))
				{
					printf("There was a problem.  My bounds are ");
					mMin.dump();
					mMax.dump();
					printf("\n and this sub pool is ");
					pool->mOffset.dump();
					pool->mScale.dump();
					printf("\n bad pt is ");
					trans[n].dump();

					Assert(!"Internal failure in encode...we should never hit this!\n");
				}
			}
			int pos = pool->mPoints.size();
			pool->mPoints.insert(pool->mPoints.end(), trans.begin(), trans.end());
			return pair<int, int>(p, pos);
		}
	}
//	printf("point pool failure...%d out of %d tried.  Points = %llu.\n", tris, p, (unsigned long long)inPoints.size());
//		inPoints[n].dump();
//	printf("\n");
	return pair<int, int>(-1, -1);
}

void			DSFContiguousPointPool::Trim(void)
{
	for (list<ContiguousSubPool>::iterator i = mPools.begin(); i != mPools.end(); ++i)
	{
		trim(i->mPoints);
	}
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

int			DSFContiguousPointPool::WritePoolAtoms(FILE * fi, int32_t id)
{
	#if DSF_WRITE_STATS
		printf("Contiguous pool of depth %d\n", mPools.empty() ? mMin.size() : mPools.begin()->mScale.size());
		StFileSizeDebugger how_big(fi,"contiguous point pool total");
	#endif

	for (list<ContiguousSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	poolAtom(fi, id, true);
		vector<uint16_t>	shorts;
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
		WritePlanarNumericAtomShort(fi, pool->mScale.size(), pool->mPoints.size(), xpna_Mode_RLE_Differenced, 1, (int16_t *) DATA(shorts));
	}
	return mPools.size();
}

int			DSFContiguousPointPool::WriteScaleAtoms(FILE * fi, int32_t id)
{
	for (list<ContiguousSubPool>::iterator pool = mPools.begin(); pool != mPools.end(); ++pool)
	{
		StAtomWriter	scaleAtom(fi, id, true);
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

void				DSF32BitPointPool::Trim(void)
{
	trim(mPoints);
}

int				DSF32BitPointPool::WritePoolAtoms(FILE * fi, int32_t id)
{
	#if DSF_WRITE_STATS
		printf("32-bit pool of depth %d\n", mScale.size());
		StFileSizeDebugger how_big(fi,"32-bit point pool total");
	#endif
	StAtomWriter	poolAtom(fi, id, true);
	vector<uint32_t>	longs;
	for (DSFTupleVector::iterator i = mPoints.begin();
		i != mPoints.end(); ++i)
	{
		for (int j = 0; j < i->size(); ++j)
		{
			longs.push_back((*i)[j]);
		}
	}
	WritePlanarNumericAtomInt(fi, mScale.size(), mPoints.size(), xpna_Mode_RLE_Differenced, 1, (int *) DATA(longs));

	return 1;
}

int				DSF32BitPointPool::WriteScaleAtoms(FILE * fi, int32_t id)
{
	StAtomWriter	scaleAtom(fi, id, true);
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
	#if USE_PVRTC
	vector<unsigned short>				indices;
	#else
	vector<unsigned int>				indices;
	#endif
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

//	printf("input: %d indices.\n", indices.size());

#if !USE_PVRTC
	tri_stripper stripper_thingie(indices);
	tri_stripper::primitives_vector	stripped_primitives;
	stripper_thingie.Strip(&stripped_primitives);

	for(tri_stripper::primitives_vector::iterator new_prim = stripped_primitives.begin(); new_prim != stripped_primitives.end(); ++new_prim)
	{
//		printf(" output: type=%d, size=%d\n",new_prim->m_Type, new_prim->m_Indices.size());
		if(new_prim->m_Type == tri_stripper::PT_Triangles)
		{
			int offset = 0;
			while (1)
			{
				int num = min(new_prim->m_Indices.size() - offset, (size_t)255);
				if(num == 0)
					break;
				out_prims.push_back(DSFPrimitive());
				out_prims.back().kind = dsf_Tri;

				while(num--)
				{
					out_prims.back().vertices.push_back(vertices[new_prim->m_Indices[offset]]);
					++offset;
				}
			}
			DebugAssert(offset == new_prim->m_Indices.size());
		} else {
			out_prims.push_back(DSFPrimitive());
			out_prims.back().kind = dsf_TriStrip;
			// Ben says: make sure we have only 255 points...if we get a tri strip longer than that, the strip alg is REALLY amazing.
			Assert(new_prim->m_Indices.size() <= 255);
			for(tri_stripper::indices::iterator idx = new_prim->m_Indices.begin(); idx != new_prim->m_Indices.end(); ++idx)
				out_prims.back().vertices.push_back(vertices[*idx]);
		}
	}

#else
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
		int n = std::min(pure_tris.size(),(size_t)255);
		out_prims.push_back(DSFPrimitive());
		out_prims.back().kind = dsf_Tri;
		for(int i = 0; i < n; ++i)
		{
			out_prims.back().vertices.push_back(vertices[pure_tris[i]]);
		}
		pure_tris.erase(pure_tris.begin(),pure_tris.begin()+n);
	}
#endif
	swap(io_primitives,out_prims);
}

