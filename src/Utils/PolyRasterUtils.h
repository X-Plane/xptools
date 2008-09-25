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
#ifndef POLYRASTERUTILS_H
#define POLYRASTERUTILS_H

#include <vector>
using std::vector;

// A single polygon segment.  The first point should
// be the more bottomward one.  We should not ever insert
// a pure-horizontal segment - it has no bearing on rasterization.
// cur_x is the X coordinate of this line at the current raster scan.

struct	PolyRasterSeg_t {

	PolyRasterSeg_t(double a, double b, double c, double d) :
		x1(a), y1(b), x2(c), y2(d), cur_x(0.0) { }

	double	x1;
	double	y1;
	double	x2;
	double	y2;
	double	cur_x;

	bool operator<(const PolyRasterSeg_t& rhs) const { return y1 < rhs.y1; }

	void	CalcCurX(double y);
	double	X_InFuture(void);

};

struct	PolyRasterizer {

	// This is every segment we have.  This
	// structure does not change, but we do update
	// cur_y in place.
	vector<PolyRasterSeg_t>		masters;

	// This is an index into the current line
	// segments that are active in our current
	// scanline.  They are ordered from min to max Y
	// at the current position.
	vector<int>					actives;


	// Sort masters by min X once we're set up.
	void		SortMasters(void);

	void		StartScanline(double y);
	void		AdvanceScanline(double y);

	bool		GetRange(int& x1, int& x2);	// Y1 is inclusive, Y2 is exclusive

	bool		DoneScan(void) { return unused_master_index >= masters.size() && actives.empty(); }

	// Book keeping
	int			unused_master_index;	// The first unused master.
	double		current_scan_y;			// The current scan X pos.
	int			current_active_index;	// The starting polyline for the current scanline.

	// Utils

	void		RecalcActiveCurX(void);			// Recalc current Y intercepts.
	void		CleanupFinishedMasters(void);	// Throw out masters that we're not using.
	void		InsertNewMasters(void);			// Add any masters that should be in but are not.

	void		Validate(void);

};

#endif
