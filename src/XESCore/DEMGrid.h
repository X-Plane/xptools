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

#ifndef DEMGrid_H
#define DEMGrid_H

#include "DEMDefs.h"
#include "CGALDefs.h"
#include "CompGeomDefs2.h"

struct DEMGrid {
	DEMGrid(const DEMGeo&);

	Point_2	xy_to_pt	(int x, int y) const { return mGrid[x + y * mWidth]; }
	Point2	xy_to_approx(int x, int y) const { return mApprox[x + y * mWidth]; }
	void	set_pt(int x, int y, const Point_2& p, int rank);
	void	clear_with_bounds(double west, double south, double east, double north, int post);
	
	int				mWidth;
	int				mHeight;
	vector<Point_2>	mGrid;
	vector<Point2>	mApprox;
	vector<int>		mRank;

private:

	DEMGrid();

};


#endif /* DEMGrid_H */
