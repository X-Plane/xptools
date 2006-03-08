/* 
 * Copyright (c) 2005, Laminar Research.
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
#include "ObjPointPool.h"
using std::min;
using std::max;

ObjPointPool::ObjPointPool() : mDepth(8)
{
}

ObjPointPool::~ObjPointPool()
{
}

void	ObjPointPool::clear(int depth)
{
	mData.clear();
	mIndex.clear();
	mDepth = depth;
}

void	ObjPointPool::resize(int pts)
{
	mData.resize(pts * mDepth);
	mIndex.clear();
}

int		ObjPointPool::accumulate(const float pt[])
{
	index_type::iterator iter = mIndex.find(key_type(pt, pt + mDepth));
	if (iter != mIndex.end())
		return iter->second;
	return append(pt);
}

int		ObjPointPool::append(const float pt[])
{
	int ret = mData.size() / mDepth;	
	mData.insert(mData.end(), pt, pt + mDepth);
	mIndex.insert(index_type::value_type(key_type(pt,pt+mDepth), ret));
	return ret;
}

void	ObjPointPool::set(int n, float pt[])
{
	memcpy(&mData[n*mDepth], pt, mDepth * sizeof(float));
	mIndex.insert(index_type::value_type(key_type(pt,pt+mDepth), n));
}

int		ObjPointPool::count(void) const
{
	return mData.size() / mDepth;
}

float *	ObjPointPool::get(int index)
{
	return &mData[index * mDepth];
}

const float *	ObjPointPool::get(int index) const
{
	return &mData[index * mDepth];
}

void ObjPointPool::get_minmax(float minCoords[3], float maxCoords[3]) const
{
	if (mData.empty()) return;
	if (mDepth < 3) return;
	
	minCoords[0] = maxCoords[0] = mData[0];
	minCoords[1] = maxCoords[1] = mData[1];
	minCoords[2] = maxCoords[2] = mData[2];
	
	for (int n = mDepth; n < mData.size(); n += mDepth)
	{
		maxCoords[0] = max(maxCoords[0], mData[n+0]);	minCoords[0] = min(minCoords[0], mData[n+0]);
		maxCoords[1] = max(maxCoords[1], mData[n+1]);	minCoords[1] = min(minCoords[1], mData[n+1]);
		maxCoords[2] = max(maxCoords[2], mData[n+2]);	minCoords[2] = min(minCoords[2], mData[n+2]);
	}
}
