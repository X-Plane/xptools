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
#ifndef XCULL_INLINE_H
#define XCULL_INLINE_H

#include "XCull.h"

template <typename __Traits>
XCull<__Traits>::XCull()
{
	Reset(0);
}

template <typename __Traits>
XCull<__Traits>::XCull(unsigned int 	inLayers)
{
	Reset(inLayers);
}

template <typename __Traits>
XCull<__Traits>::~XCull()
{
}

template <typename __Traits>
void
XCull<__Traits>::Reset(unsigned int 	inLayers)
{
	mVisible.resize(inLayers);
	mDistances.resize(inLayers);
	for (int n = 0; n < inLayers; ++n)
	{
		int	num_buckets = 1 << (n * 2);
		mDistances[n].resize(num_buckets);
		mVisible[n].resize(num_buckets);
	}
}

template <typename __Traits>
int
XCull<__Traits>::Cull(
						Camera			inCamera,
						Reference		reference)
{
	mTotalVisible = 0;
	if (!mVisible.empty())
		CullBucket(0, inCamera, reference);
	return mTotalVisible;
}

template <typename __Traits>
void
XCull<__Traits>::CullBucket(
						XBucketID		bucket,
						Camera			inCamera,
						Reference		reference)
{
	unsigned int level = XBucket_GetLevel(bucket);
	unsigned int index = XBucket_GetIndex(bucket);

	if (Traits::IsVisible(bucket, inCamera, reference))
	{
		Scalar	lod = Traits::DistanceSquared(bucket, inCamera, reference);

			mDistances[level][index] = lod;
			mVisible[level][index] = true;
		mTotalVisible++;

			if (level < (mVisible.size()-1))
			for (unsigned int n = 0; n < 4; ++n)
			{
				CullBucket(XBucket_GetChild(bucket, n), inCamera, reference);
			}

	} else {
		mVisible[level][index] = false;
	}
}

#endif
