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
	mDepth = depth;
}

void	ObjPointPool::resize(int pts)
{
	mData.resize(pts * mDepth);
}

int		ObjPointPool::accumulate(const float pt[])
{
	for (int n = 0; n < mData.size(); n += mDepth)
	{
		if (memcmp(&mData[n], pt, sizeof(float) * mDepth) == 0)
			return n / mDepth;
	}
	return append(pt);
}

int		ObjPointPool::append(const float pt[])
{
	int ret = mData.size() / mDepth;
	mData.insert(mData.end(), pt, pt + mDepth);
	return ret;
}

void	ObjPointPool::set(int n, float pt[])
{
	memcpy(&mData[n*mDepth], pt, mDepth * sizeof(float));
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
