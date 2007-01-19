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
#include "PolyRasterUtils.h"
#include <stdio.h>
#include <algorithm>
#include <math.h>

#define DEBUG_RASTERIZER 0

void	PolyRasterSeg_t::CalcCurX(double now)
{
	if (now == y1)
		cur_x = x1;
	else if (now == y2)
		cur_x = x2;
	else if (y1 == y2)
		cur_x = x1;
	else
		cur_x = x1 + (x2 - x1) * ((now - y1) / (y2 - y1));
}			

double	PolyRasterSeg_t::X_InFuture(void)
{
	if (y1 == y2) return x1;
	if (x1 == x2) return x1;
	return cur_x + (x2-x1)/(y2-y1);
}


// Sort masters by min X once we're set up.  Easy: we define operator< for our segs.
void		PolyRasterizer::SortMasters(void)
{	
	std::sort(masters.begin(), masters.end());
	current_scan_y = 0.0;
	unused_master_index = 0;
	current_active_index = 0;
}

void		PolyRasterizer::StartScanline(double y)
{
	current_scan_y = y;
	InsertNewMasters();
	CleanupFinishedMasters();
	current_active_index = 0;
#if DEBUG_RASTERIZER
	Validate();
#endif					
}

void		PolyRasterizer::AdvanceScanline(double y)
{
	current_scan_y = y;
	RecalcActiveCurX();
	CleanupFinishedMasters();
	InsertNewMasters();
	CleanupFinishedMasters();
#if DEBUG_RASTERIZER
	Validate();
#endif				
	current_active_index = 0;
}

// Return an integer scan-range value.  This is pretty easy...
// the first two unused active elements are the next range.  
// One warning, a range can be zero either because (1) the points
// touch in a shitty pinching way or (2) the range is smaller than
// one integral unit.  So we loop until we get a real range.
bool		PolyRasterizer::GetRange(int& x1, int& x2)
{
#if DEBUG_RASTERIZER
	Validate();
#endif		
	while (1)
	{
		if (current_active_index >= actives.size()) return false;
		double	x1f = masters[actives[current_active_index++]].cur_x;
		double	x2f = masters[actives[current_active_index++]].cur_x;

		// BEN SEZ: oops - we need to use ceil here, not count on the
		// murky ability of float->int casting.
		x1 = ceil(x1f);
		x2 = floor(x2f) + 1;
		if (x1 < x2) 
			return true;
	}
}


// Recalc current X intercepts.
void		PolyRasterizer::RecalcActiveCurX(void)
{
	for (int i = 0; i < actives.size(); ++i)
	{
		int ii = actives[i];
		masters[ii].CalcCurX(current_scan_y);
	}
}

// Throw out masters that we're not using.
void		PolyRasterizer::CleanupFinishedMasters(void)
{
	for (vector<int>::iterator i = actives.begin(); i != actives.end(); )
	{
		// Off-by-one insanity.  Generally we want to take the leading and not trailing
		// point of an element.  If for some freak reason a scanline corresponds with an ending,
		// we pull the scanline UNLESS it is vertical.  This is because we want to use the vertical
		// scanline exactly once.  We tend to use it at the bottom, but this is just edge-case semantics.
		if (masters[*i].y2 <= current_scan_y && masters[*i].y1 < current_scan_y)
			i = actives.erase(i);
		else
			++i;
	}
}

// Add any masters that should be in but are not.
void		PolyRasterizer::InsertNewMasters(void)
{
	while (1)
	{
		if (unused_master_index >= masters.size()) break;
		
		if (masters[unused_master_index].y1 > current_scan_y) break;
		
		masters[unused_master_index].CalcCurX(current_scan_y);
		
		vector<int>::iterator ins;
		for (ins = actives.begin(); ins != actives.end(); ++ins)
		{
			// In this case,we 've found a segment that is above us.  Stick us in before!
			if (masters[*ins].cur_x > masters[unused_master_index].cur_x)
				break;
			// This case is funny, we start at the same point as the other segment.
			// We need to make sure that it will become above us as we diverge.
			// This happens when two polygon segments start at the same point AND 
			// the y-scan is at exactly that y coordinate!  (Happens more with integer polygons.)
			if (masters[*ins].cur_x == masters[unused_master_index].cur_x)
			if (masters[*ins].X_InFuture() > masters[unused_master_index].X_InFuture())
				break;
		}
		actives.insert(ins, unused_master_index);
		unused_master_index++;
	}
}

void	PolyRasterizer::Validate(void)
{
	bool p = false;
	if (actives.size() % 2)
		p = true, printf("ERROR: Odd number of elements in the active queue!\n");
		
	for (int i = 1; i < actives.size(); ++i)
	{
		if (masters[actives[i]].cur_x < masters[actives[i - 1]].cur_x)
			p = true, printf("ERROR: Out of order active queue!");
			
	}
	if (p)
	{
		printf("At pos %f:\n", current_scan_y);
		for (int i = 0; i < actives.size(); ++i)
		{
			printf("%d) %f,%f -> %f, %f,   NOW %f\n", 
				masters[actives[i]].x1,
				masters[actives[i]].y1,
				masters[actives[i]].x2,
				masters[actives[i]].y2,
				masters[actives[i]].cur_x);
		}
	}
}