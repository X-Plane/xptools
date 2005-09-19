#include "ObjPointPool.h"

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
