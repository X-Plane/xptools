/*
 * Copyright (c) 2007, Laminar Research.
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

#include "MapDefsCGAL.h"

#include "DEMToVector.h"
#include "DEMDefs.h"
#include "ParamDefs.h"
#include "XESConstants.h"
#include "CompGeomUtils.h"

static int	dir_x[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
static int	dir_y[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

void BuildSmoothedPts(const Polygon2& pts, Polygon2& op, bool smooth)
{
	int sz;
	double scaling = cos(pts[0].y() * DEG_TO_RAD);
	vector<Point2>	huge(pts);
	/*
	sz = pts.size();
	huge.resize(sz*2);
	for (int n = 0; n < sz; ++n)
	{
		int pr = (n+sz-1) % sz;
		int pn = (n+1) % sz;
		Segment2	s1(pts.at(pr), pts.at(n));
		Segment2	s2(pts.at(n), pts.at(pn));
		huge[n*2  ] = s1.midpoint(0.75);
		huge[n*2+1] = s2.midpoint(0.25);
	}
	*/
	sz = huge.size();
	for (int n = 0; n < sz; ++n)
	{
		int pr = (n+sz-1) % sz;
		int pn = (n+1) % sz;
		Vector2	v1(huge[pr], huge[n]);
		Vector2	v2(huge[n], huge[pn]);
		v1.dx /= scaling;
		v2.dx /= scaling;
		v1.normalize();
		v2.normalize();
		if (v1.dot(v2) < 0.97)
			op.push_back(huge[n]);
	}

	// Ben sez: gotta use safety-checked smoothing to prevent topological insanity!

//	MidpointSimplifyPolygon(op);

	//if (smooth)
	//{
	//	SimplifyPolygonMaxMove(op, 0.000416666666, true, true);
//		SmoothPolygon(op, 0.000277777778, 30);
//	}
}

int	FindNextCCW(const DEMGeo& dem, int x, int y, int last_dir)
{
	int n;
	for (n = 0; n < 8; ++n)
	{
		int p = (n+last_dir)%8;
		if (dem.get(x+dir_x[p],y+dir_y[p]) != DEM_NO_DATA)
			return p;
	}
	return -1;
}

int IndexDEM(const DEMGeo& inDEM, vector<int>& outIndex)
{
	int ctr = 0;
	outIndex.resize(inDEM.mHeight, 0);
	for (int y = 0; y < inDEM.mHeight; ++y)
	for (int x = 0; x < inDEM.mWidth; ++x)
	{
		if (inDEM.get(x,y) != DEM_NO_DATA)
			++ctr, outIndex[y]++;
	}
	return ctr;
}

void ClearDEMPt(DEMGeo& ioDEM, vector<int>& ioIndex, int x, int y)
{
	DebugAssert(ioIndex[y] > 0);
	DebugAssert(ioDEM.get(x,y) != DEM_NO_DATA);

	ioDEM(x,y) = DEM_NO_DATA;
	ioIndex[y]--;
}

bool FindHighestLeft(const DEMGeo& inDEM, const vector<int>& ioIndex, int& x, int& y, int y_start)
{
	for (y = y_start; y >= 0; --y)
	if (ioIndex[y] > 0)
		for (x = 0; x < inDEM.mWidth; ++x)
		if (inDEM.get(x,y) != DEM_NO_DATA)
			return true;
	return false;
}


void DemToVector(DEMGeo& ioDEM, Pmwx& ioMap, bool doSmooth, int inPositiveTerrain, ProgressFunc func)
{
	int sx, sy, ox, oy;
	int x, y;
	int y_start = ioDEM.mHeight-1;
	vector<int>	idx;
	int total = IndexDEM(ioDEM, idx);

	PROGRESS_START(func, 0, 1, "Building vectors...")

	int ctr = 0;

	int raw_pts = 0, smooth_pts = 0;
	CGAL_precondition(false);
	/*
	while(FindHighestLeft(ioDEM, idx, x, y, y_start))
	{
		Polygon2	pts;

		y_start = y;
		sx = x;
		sy = y;
		int step = 0;
		do {
			PROGRESS_CHECK(func, 0, 1, "Building vectors...", ctr, total, ioDEM.mWidth)
			ox = x;
			oy = y;
			step = FindNextCCW(ioDEM, x, y, step);
			Assert(step != -1);

			x = x + dir_x[step];
			y = y + dir_y[step];
			step = (step+5)%8;
			// Edge from ox, oy to x, y

			Point2	op(ioDEM.x_to_lon(ox), ioDEM.y_to_lat(oy));
			Point2	np(ioDEM.x_to_lon(x), ioDEM.y_to_lat(y));
			pts.push_back(np);

			ClearDEMPt(ioDEM, idx, x, y);
			++ctr;
		} while (x != sx || y != sy);

		Polygon2	pts_smooth;
		BuildSmoothedPts(pts, pts_smooth, doSmooth);
		raw_pts += pts.size();
		smooth_pts += pts_smooth.size();

		Assert(!pts_smooth.empty());
		Pmwx::Locate_type	loc;
		GISHalfedge * he = ioMap.locate_point(pts_smooth[0], loc);
		Assert(loc == Pmwx::locate_Face);
		GISFace * face = he ? he->face() : ioMap.unbounded_face();

		GISFace * newf = ioMap.insert_ring(face, pts_smooth);
		if (face == ioMap.unbounded_face())
			newf->mTerrainType = inPositiveTerrain;
		else if (face->mTerrainType != inPositiveTerrain)
			newf->mTerrainType  = inPositiveTerrain;
	}
	 */
	PROGRESS_DONE(func, 0, 1, "Building vectors...")
	printf("Ratio: %lf\n", raw_pts ? ((double) smooth_pts / (double) raw_pts) : 0.0);
}
