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


	// Sort masters by min X once we're set up.  Call this once to initialize.
	void		SortMasters(void);

	// Start at this scanline. Call once to init.
	void		StartScanline(double y);
	
	// Go up to the next (increasing) scanline.  We can skip whole ranges of lines if we need!
	void		AdvanceScanline(double y);
	
	// This gives us the next time to scan that is at least as large as y.  In particular, if we 
	// are in a "dead zone" where there is no polygon, and y is where we would jump, this can return
	// a larger jump to the next point where something interesting happens.  This can be a huge win for
	// mostly sparse empty areas.  Note that the return value is in no way "grid aligned" so it may have
	// to be rounded down.
	double		NextInterestingTime(double y);
	
	// This returns an entire scan line as range pairs.  Note that some range pairs may be zero length.
	// next_y is input as the latest time we care about.  It will be reduced to an earlier time if there
	// is a topological event that needs consideration coming up.
	void		GetLine(vector<double>& out_line, double& next_y);

	// This returns the net integral inclusive range on our line or false when done.  We can use this to
	// iterate along the scanline without allocating memory.
	bool		GetRange(int& x1, int& x2);	// X1 is inclusive, X2 is exclusive

	// This returns true when the scanner has nothing more to do.  It will not return true until the 
	// scanline has been advanced.  That is, if GetRange returns false on the last line, DoneScan won't
	// return true until we advance one last time.  Note also that NextInterestingTime will return a valid
	// scanline (the one you pass in) if there is nothing above us.  So bottom line: always just advance
	// the scanline, then check for done.
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

void IntersectRanges(vector<double>& o, const vector<double>& a, const vector<double>& b);
void IntersectRanges(vector<double>& o, const vector<double>& a);

struct	BoxRasterizer {

			BoxRasterizer(PolyRasterizer * raster, double baseline, double limit, double interval);
		
	bool	GetNextBox(double& x1, double& y1, double& x2, double& y2);


			void scan(void);
	
		PolyRasterizer *	rasterizer;
		double				interval;
		double				y;
		double				stop;
		double				next_event;
		vector<double>		range;
		vector<double>		output;
		int					output_idx;
		double				output_y;

};

#endif
