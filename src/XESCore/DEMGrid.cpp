/* 
 * Copyright (c) 2011, Laminar Research.
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

#include "DEMGrid.h"
#include "MathUtils.h"

DEMGrid::DEMGrid(const DEMGeo& d)
{
	mWidth = d.mWidth;
	mHeight = d.mHeight;
	mGrid.resize(mWidth * mHeight);
	mApprox.resize(mWidth * mHeight);
	mRank.resize(mWidth * mHeight);
	clear_with_bounds(d.mWest,d.mSouth,d.mEast,d.mNorth, d.mPost);
}

void	DEMGrid::set_pt(int x, int y, const Point_2& p, int rank)
{
	int i  = x + y * mWidth;
	if (rank >= mRank[i])
	{
		mGrid[i] = p;
		mApprox[i] = cgal2ben(p);
		mRank[i] = rank;
	}
}

void	DEMGrid::clear_with_bounds(double west, double south, double east, double north, int post)
{
	vector<Point_2>::iterator grid = mGrid.begin();
	vector<Point2>::iterator approx = mApprox.begin();
	vector<int>::iterator rank = mRank.begin();
	for(int y = 0; y < mHeight; ++y)
	for(int x = 0; x < mHeight; ++x)
	{
		*grid++ = Point_2(
			double_interp(0,west,mHeight-post,east,x),
			double_interp(0,south,mHeight-post,north,y));
		*approx++ = Point2(
			double_interp(0,west,mHeight-post,east,x),
			double_interp(0,south,mHeight-post,north,y));
		*rank++ = 0;
	}
}
