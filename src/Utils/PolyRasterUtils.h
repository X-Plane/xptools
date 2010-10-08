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
#include <math.h>
#include <assert.h>
using std::vector;

// Set to 1 to do heavy checking of the rasterizer.
#define DEBUG_RASTERIZER 0



// A single polygon segment.  The first point should
// be the more bottomward one.  We should not ever insert
// a pure-horizontal segment - it has no bearing on rasterization.
// This is a utility class and isn't used on its own.

template <typename Number>
struct	PolyRasterSeg_t {

	PolyRasterSeg_t(Number a, Number b, Number c, Number d) :
		x1(a), y1(b), x2(c), y2(d) { assert(y1 < y2); }

	Number	x1;
	Number	y1;
	Number	x2;
	Number	y2;

	// Our basic comparison is for our lower Y to see who is hit first by the scan line.
	bool operator<(const PolyRasterSeg_t& rhs) const { return y1 < rhs.y1; }
	// This tells if, given equal intercept with the scanline, we should come before rhs.  We do this with a slope comparison;
	// if we intersect at the scaneline, whomever has a more negative dx/dy (both have guaranteed positive dy since y2>y1)
	// goes first.
	bool LessInFutureThan(const PolyRasterSeg_t& rhs) const { return (x2-x1)*(rhs.y2-rhs.y1) < (rhs.x2-rhs.x1)*(y2-y1); }
	// This is our intercept (X) with a given scanline (Y).
	Number	CalcCurX(Number y) const;



};

/************************************************************************************************************************************
 * LINE RASTERIZER
 ************************************************************************************************************************************
 *
 * For a given polygon, this produces horizontal lines that are only inside the polygon.  Some notes:
 *
 * Left and lower edges are in, right and upper edges are outside.
 * Correct output is produced if the polygon is closed and non-self-intersecting.  Polygons can be disjoint or have holes.
 * If the polygon is invalid, client code simply has to be prepared to receive open-ended or out-of-order regions along the scanlines.
 *
 * Basic procedure is:
 * - Insert all edges with AddEdge
 * - Call SortMasters
 * - Call "StartScanline"
 * - while not DoneScan
 *     Call GetLine or while(GetRange)
 * -   Call AdvanceScanline
 *
 */

template<typename Number>
struct	PolyRasterizer {

	typedef PolyRasterSeg_t<Number>			PolyRasterSeg;
	typedef pair<PolyRasterSeg *, Number>	ActiveSeg;

	// This is every segment we have.  This
	// structure does not change once sorted.  Public so code can
	// draw the polygonal outline, etc.
	vector<PolyRasterSeg>		masters;

	// This is the bounds of all segments added using add-edge.
	Number						bounds[4];

	// Add an edge if needed, takes care of out of order and horizontal lines.
	// If we use this, "bounds" gets set to something sane.
	void		AddEdge(Number x1, Number y1, Number x2, Number y2);

	// Sort masters by min X once we're set up.  Call this once to initialize when
	// all edge are added.
	void		SortMasters(void);

	// Start at this scanline. Call once to init.
	void		StartScanline(Number y);

	// Go up to the next (increasing) scanline.  We can skip whole ranges of lines if we need!
	void		AdvanceScanline(Number y);

	// This gives us the next time to scan that is at least as large as y.  In particular, if we
	// are in a "dead zone" where there is no polygon, and y is where we would jump, this can return
	// a larger jump to the next point where something interesting happens.  This can be a huge win for
	// mostly sparse empty areas.  Note that the return value is in no way "grid aligned" so it may have
	// to be rounded down.
	Number		NextNonEmptyTime(Number y);

	// This returns an entire scan line as range pairs.  Note that some range pairs may be zero length.
	// next_y is input as the latest time we care about.  It will be reduced to an earlier time if there
	// is a topological event that needs consideration coming up.
	void		GetLine(vector<Number>& out_line, Number& next_y);

	// This returns the net integral inclusive range on our line or false when done.  We can use this to
	// iterate along the scanline without allocating memory.
	bool		GetRange(int& x1, int& x2);	// X1 is inclusive, X2 is exclusive

	// This returns true when the scanner has nothing more to do.  It will not return true until the
	// scanline has been advanced.  That is, if GetRange returns false on the last line, DoneScan won't
	// return true until we advance one last time.  Note also that NextNonEmptyTime will return a valid
	// scanline (the one you pass in) if there is nothing above us.  So bottom line: always just advance
	// the scanline, then check for done.
	bool		DoneScan(void) { return unused_master_index >= masters.size() && actives.empty(); }

private:

	// Book keeping
	int			unused_master_index;	// The first unused master.
	Number		current_scan_y;			// The current scan Y pos.
	int			current_active_index;	// The starting polyline for the current scanline for GetRange

	// Active segments.  A note on memory management: we keep two extra vectors in the class so that they
	// aren't deallocated and reallocated between calls into the scanner.  This saves a bunch of dealloc/realloc
	// cycles at the expense of a little bit of memory.
	vector<ActiveSeg>			actives;
	vector<ActiveSeg>			temp_actives;
	vector<ActiveSeg>			new_actives;

	void		RecalcActiveCurX(void);			// Recalc current Y intercepts.
	void		CleanupFinishedMasters(void);	// Throw out masters that we're not using.
	void		InsertNewMasters(void);			// Add any masters that should be in but are not.

	void		Validate(void) const;

	// This compares two active segments to figure out which one comes first.
	struct compare_active_segs {
		bool operator()(const ActiveSeg& lhs, const ActiveSeg& rhs) const
		{
			if(lhs.second == rhs.second)
				return lhs.first->LessInFutureThan(*rhs.first);
			return lhs.second < rhs.second;
		}
	};
	// This returns true if a segment should be removed (because its top is at or below the scanline).
	struct active_seg_dead {
		Number current_scan_y;
		active_seg_dead(Number n) : current_scan_y(n) { }
		bool operator()(const ActiveSeg& x) const
		{
			return x.first->y2 <= current_scan_y;
		}
	};
};

/************************************************************************************************************************************
 * BOX RASTERIZER
 ************************************************************************************************************************************
 * The Box rasterizer gives us boxes of a certain Y extent that are fully inside the polygon.  In this sense it is more conservative
 * of a rasterization than the line rasterizer.
 *
 */

template<typename Number>
struct	BoxRasterizer {

			BoxRasterizer(PolyRasterizer<Number> * raster);

	// Start box rasterization at the range from y1 to y2.
	void	StartScanline(Number y1, Number y2);

	// Continue at this interval...y1 must be at least >= to y2 from last time.
	void	AdvanceScanline(Number y1, Number y2);

	// Get the line from advance scanline.  It is a list of interval pairs.
	// Note that we can call this only once per advance, because it swaps
	// memory instead of copying.
	void	GetLineTrash(vector<Number>& out_line);

	// Are we done scanning?  Make sure to call advance first.
	bool	DoneScan();

private:

		PolyRasterizer<Number> *	rasterizer;

		Number				y1;							// Current output.
		Number				y2;
		vector<Number>		output;

	// Utilities to do interval intersection.  Not really part of the class, but this keeps them
	// from gumming up the namespace.
	void IntersectRanges(vector<Number>& o, const vector<Number>& a, const vector<Number>& b);
	void IntersectRanges(vector<Number>& o, const vector<Number>& a);

};

/************************************************************************************************************************************
 * INLINE DEFINITIONS
 ************************************************************************************************************************************/

template<typename Number>
Number	PolyRasterSeg_t<Number>::CalcCurX(Number now) const
{
	// This is heavily special-cased for end-points to avoid floating point heebie-jeebies; that way if we have exact end points
	// gonig in, we get exact intercepts going out.  This would NOT be guaranteed with the full intercept formula.
	if (now == y1)			return x1;
	else if (now == y2)		return x2;
	else					return x1 + (x2 - x1) * ((now - y1) / (y2 - y1));
}

template<typename Number>
void		PolyRasterizer<Number>::AddEdge(Number x1, Number y1, Number x2, Number y2)
{
	if(y1 != y2)		// Skip horizontals.
	{
		if(masters.empty())
		{
			bounds[0] = min(x1,x2);
			bounds[1] = min(y1,y2);
			bounds[2] = max(x1,x2);
			bounds[3] = max(y1,y2);
		}
		else
		{
			bounds[0] = min(bounds[0],min(x1,x2));
			bounds[1] = min(bounds[1],min(y1,y2));
			bounds[2] = max(bounds[2],max(x1,x2));
			bounds[3] = max(bounds[3],max(y1,y2));
		}
		if(y1 < y2)
			masters.push_back(PolyRasterSeg(x1,y1,x2,y2));
		else
			masters.push_back(PolyRasterSeg(x2,y2,x1,y1));
	}
}


// Sort masters by min X once we're set up.  Easy: we define operator< for our segs.
template<typename Number>
void		PolyRasterizer<Number>::SortMasters(void)
{
	std::sort(masters.begin(), masters.end());
	current_scan_y = 0.0;
	unused_master_index = 0;
	current_active_index = 0;
}

template<typename Number>
void		PolyRasterizer<Number>::StartScanline(Number y)
{
	actives.clear();
	current_scan_y = y;
	InsertNewMasters();
	current_active_index = 0;
#if DEBUG_RASTERIZER
	Validate();
#endif
}

template<typename Number>
void		PolyRasterizer<Number>::AdvanceScanline(Number y)
{
	current_scan_y = y;
	CleanupFinishedMasters();
	RecalcActiveCurX();
	InsertNewMasters();
#if DEBUG_RASTERIZER
	Validate();
#endif
	current_active_index = 0;
}

template<typename Number>
Number		PolyRasterizer<Number>::NextNonEmptyTime(Number y)
{
	// If we have active segments floating around, the client
	// just wants to go up one scanline..if nothing else, they will
	// have changed their intercepts to the scan line, maybe.
	if(!actives.empty()) return y;

	// No more segments to add and we're empty?  Let the client just go
	// up one - it'll figure out we are done-done anyway.
	if(unused_master_index == masters.size()) return y;

	// Hrm - no segments now?  Tell the client it's okay to jump
	// all the way up to the next insert...we probably have a big gap.
	return max(masters[unused_master_index].y1, y);
}


// Return an integer scan-range value.  This is pretty easy...
// the first two unused active elements are the next range.
// One warning, a range can be zero either because (1) the points
// touch in a shitty pinching way or (2) the range is smaller than
// one integral unit.  So we loop until we get a real range.
template<typename Number>
bool		PolyRasterizer<Number>::GetRange(int& x1, int& x2)
{
#if DEBUG_RASTERIZER
	Validate();
#endif
	while (1)
	{
		if (current_active_index >= actives.size()) return false;
		Number	x1f = actives[current_active_index++].second;
		if (current_active_index >= actives.size()) return false;
		Number	x2f = actives[current_active_index++].second;

		// BEN SEZ: oops - we need to use ceil here, not count on the
		// murky ability of float->int casting.
		x1 = ceil(x1f);
		x2 = floor(x2f) + 1;
		if (x1 < x2)
			return true;
	}
}

template<typename Number>
void		PolyRasterizer<Number>::GetLine(vector<Number>& out_line, Number& next_y)
{
	out_line.resize(actives.size());
	// If we have any unused lines, the next line starting might be an "event"
	if(unused_master_index < masters.size())
		next_y = min(next_y, masters[unused_master_index].y1);
	int n = 0;
	for(typename vector<ActiveSeg>::const_iterator a = actives.begin(); a != actives.end(); ++a, ++n)
	{
		out_line[n] = a->second;
		// If an active segment ends in the future, it may be the next "event."
		if(a->first->y2 > current_scan_y)
			next_y = min(next_y, a->first->y2);
	}
}


// Recalc current X intercepts.
template<typename Number>
void		PolyRasterizer<Number>::RecalcActiveCurX(void)
{
	for(typename vector<ActiveSeg>::iterator a = actives.begin(); a != actives.end(); ++a)
		a->second = a->first->CalcCurX(current_scan_y);
}

// Throw out masters that we're not using.
template<typename Number>
void		PolyRasterizer<Number>::CleanupFinishedMasters(void)
{
	// We use the STL remove_if because it moves all remaining items to the left in a single O(N) pass, avoiding some
	// thrash.
	typename vector<ActiveSeg>::iterator new_last = remove_if(actives.begin(), actives.end(), active_seg_dead(current_scan_y));
	actives.erase(new_last, actives.end());
}

// Add any masters that should be in but are not.
template<typename Number>
void		PolyRasterizer<Number>::InsertNewMasters(void)
{
	// Our basic strategy:
	// We accumulate all new active segments first, then sort just the new guys, then merge the sorted new guys with the
	// old guys (which are already sorted).  When doing a very heavy box rasterization (where a lot of masters are active
	// and a few are coming and going all of the time) the O(NlogN) sort time of active masters really gets out of control.
	// By only sorting the new ones and merging we get O(N) + O(kLogk) where N is total masters and k is new masters.  K
	// is usually smallr than N by a lot, so this is a logN times improvement in run time.

	assert(new_actives.empty());
	while (1)
	{
		if (unused_master_index >= masters.size()) break;					// No more masters at all?  We're done.
		if (masters[unused_master_index].y1 > current_scan_y) break;		// Master ABOVE the scanline?  We've done all the masters we care about now.

		if (masters[unused_master_index].y2 > current_scan_y)				// Quick check: if y2 were to be on or below the scanline, we are already DONE with this guy.
		{																	// Happens when a seg is so small it is BETWEEN scanlines!  Skip insert but DO increment unused index.
			new_actives.push_back(ActiveSeg(								// This saves having to resort actives, bla bla bla.
				&masters[unused_master_index],
				 masters[unused_master_index].CalcCurX(current_scan_y)));
		}
		unused_master_index++;
	}

	if(!new_actives.empty())
	{
		// if we have any work to do, first sort the new actives so they are in order.
		sort(new_actives.begin(), new_actives.end(), compare_active_segs());

		if(actives.empty())
		{
			// Fast case: just use them.
			actives.swap(new_actives);
		}
		else
		{
			// Slow case: use merge-sort to merge the two together, then swap the result back.
			temp_actives.resize(new_actives.size() + actives.size());
			#if DEV
				typename vector<ActiveSeg>::iterator l =
			#endif
			merge(actives.begin(),actives.end(),new_actives.begin(),new_actives.end(),temp_actives.begin(),compare_active_segs());
			#if DEV
			assert(l == temp_actives.end());
			#endif
			actives.swap(temp_actives);
			temp_actives.clear();
		}
		new_actives.clear();
	}
}

template<typename Number>
void	PolyRasterizer<Number>::Validate(void) const
{
	bool p = false;
	if (actives.size() % 2)
		p = true, printf("ERROR: Odd number of elements in the active queue!\n");

	typename vector<ActiveSeg>::const_iterator a1, a2;

	for (a1 = actives.begin(); a1 != actives.end(); ++a1)
	{
		a2 = a1;
		++a2;
		if(a2 != actives.end())
		if(a2->second < a1->second)
			p = true, printf("ERROR: Out of order active queue!");

	}
	if (p)
	{
		printf("At pos %f:\n", current_scan_y);
		int n = 0;
		for(a1 = actives.begin(); a1 != actives.end(); ++a1, ++n)
		{
			printf("%d) %f,%f -> %f, %f,   NOW %f\n", n,
				a1->first->x1,
				a1->first->y1,
				a1->first->x2,
				a1->first->y2,
				a1->second);
		}
	}
}

inline bool IS_POSITIVE(int n) 	{ return (n%2) == 0; }

#define __boolean_op_func(a,b) ((a) && (b))

template<typename Number>
void BoxRasterizer<Number>::IntersectRanges(vector<Number>& out, const vector<Number>& a, const vector<Number>& b)
{
	// First: make sure both regions are "bounded".  If they have an odd number of intervals, the last region "goes on forever"
	// and this algo's assumptions are moot.
	assert(IS_POSITIVE(a.size()));
	assert(IS_POSITIVE(b.size()));

	// Xon of anything with empty is empty.  Case this out BEFORE we 'reserve' memory.
	out.clear();
	if(a.empty())		return;
	if(b.empty())		return;

	int na = 0, nb = 0;

	bool sa = false, sb = false;
	out.reserve(a.size());				// Pre-allocate about 'a' number of cuts, to try to avoid vector hell.

	// Only do intersections whie BOTH are open.  Any hang-off-the-end of either is NOT in the final.
	while(na < a.size() && nb < b.size())
	{
		// Weird idiom note: is_positive(size) == what_we_should-be means: the next time we insert will have the same
		// "positivity" as the region we eval.
		if(a[na] < b[nb])
		{
			sa = IS_POSITIVE(na);
			if(IS_POSITIVE(out.size()) == __boolean_op_func(sa,sb))
				out.push_back(a[na]);
			++na;
		}
		else if(a[na] > b[nb])
		{
			sb = IS_POSITIVE(nb);
			if(IS_POSITIVE(out.size()) == __boolean_op_func(sa,sb))
				out.push_back(b[nb]);
			++nb;
		}
		else
		{
			sa = IS_POSITIVE(na);
			sb = IS_POSITIVE(nb);
			if(IS_POSITIVE(out.size()) == __boolean_op_func(sa,sb))
				out.push_back(b[nb]);
			++na;
			++nb;
		}
	}

	// Sanity check that we "sealed off" the ened and have an even number of regions; if we don't, it means
	// we screwed up.  Since our boolean op is & this would only happen if the is_positive of one of the last ones
	// is true, which would happen if we have unsealed.  So if we don't hit the top assert, this should never fire.
	assert(IS_POSITIVE(out.size()));
}

#undef __boolean_op_func

template<typename Number>
void BoxRasterizer<Number>::IntersectRanges(vector<Number>& o, const vector<Number>& a)
{
	vector<Number>	temp;
	IntersectRanges(temp, o, a);
	o.swap(temp);
}

template<typename Number>
BoxRasterizer<Number>::BoxRasterizer(PolyRasterizer<Number> * raster) : rasterizer(raster)
{
}

template<typename Number>
void	BoxRasterizer<Number>::StartScanline(Number iy1, Number iy2)
{
	y1 = iy1;
	y2 = iy2;

	// First: get what we have for the bottom of our range.
	rasterizer->StartScanline(y1);
	vector<Number>	r;
	Number			ny = y2;
	rasterizer->GetLine(output, ny);

	// 'ny' is the next time we might care about that is less than y2
	// (or y2) if we care about nothing else.  As long as we have some
	// region to keep checking and we're not at y2, advance and
	// merge the output in.
	while(!output.empty() && ny < y2)
	{
		rasterizer->AdvanceScanline(ny);
		ny = y2;
		rasterizer->GetLine(r, ny);
		IntersectRanges(output,r);
	}

	// Finally, advance to y2 (every time) and if we still have
	// output, merge in y2.
	rasterizer->AdvanceScanline(y2);
	if(!output.empty())
	{
		ny = y2;
		rasterizer->GetLine(r, ny);
		IntersectRanges(output,r);
	}
}

template<typename Number>
void	BoxRasterizer<Number>::AdvanceScanline(Number iy1, Number iy2)
{
	assert(iy1 >= y2);
	if(iy1 > y2)
		rasterizer->AdvanceScanline(iy1);

	y1 = iy1;
	y2 = iy2;
	vector<Number>	r;
	Number			ny = y2;
	rasterizer->GetLine(output, ny);
	while(!output.empty() && ny < y2)
	{
		rasterizer->AdvanceScanline(ny);
		ny = y2;
		rasterizer->GetLine(r, ny);
		IntersectRanges(output,r);
	}
	rasterizer->AdvanceScanline(y2);
	if(!output.empty())
	{
		ny = y2;
		rasterizer->GetLine(r, ny);
		IntersectRanges(output,r);
	}

}

template<typename Number>
void	BoxRasterizer<Number>::GetLineTrash(vector<Number>& out_line)
{
	out_line.swap(output);
}

template<typename Number>
bool	BoxRasterizer<Number>::DoneScan()
{
	return rasterizer->DoneScan();
}




#endif
