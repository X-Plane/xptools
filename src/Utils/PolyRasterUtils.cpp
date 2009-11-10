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

double		PolyRasterizer::NextInterestingTime(double y)
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
bool		PolyRasterizer::GetRange(int& x1, int& x2)
{
#if DEBUG_RASTERIZER
	Validate();
#endif
	while (1)
	{
		if (current_active_index >= actives.size() || actives[current_active_index] >= masters.size()) return false;
		double	x1f = masters[actives[current_active_index++]].cur_x;
		if (current_active_index >= actives.size() || actives[current_active_index] >= masters.size()) return false;
		double	x2f = masters[actives[current_active_index++]].cur_x;

		// BEN SEZ: oops - we need to use ceil here, not count on the
		// murky ability of float->int casting.
		x1 = ceil(x1f);
		x2 = floor(x2f) + 1;
		if (x1 < x2)
			return true;
	}
}

void		PolyRasterizer::GetLine(vector<double>& out_line, double& next_y)
{
	out_line.resize(actives.size());
	// If we have any unused lines, the next line starting might be an "event"
	if(unused_master_index < masters.size())
		next_y = min(next_y, masters[unused_master_index].y1);
	for(int n = 0; n <actives.size(); ++n)
	{
		out_line[n] = masters[actives[n]].cur_x;
		// If an active segment ends in the future, it may be the next "event."
		if(masters[actives[n]].y2 > current_scan_y)
			next_y = min(next_y, masters[actives[n]].y2);
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
			printf("%d) %f,%f -> %f, %f,   NOW %f\n", i,
				masters[actives[i]].x1,
				masters[actives[i]].y1,
				masters[actives[i]].x2,
				masters[actives[i]].y2,
				masters[actives[i]].cur_x);
		}
	}
}

inline bool IS_POSITIVE(int n) 	{ return (n%2) == 0; }

#define func(a,b) ((a) && (b))

void IntersectRanges(vector<double>& out, const vector<double>& a, const vector<double>& b)
{
	out.clear();
	if(a.empty())
	{
		out=b;
		return;
	}
	if(b.empty())
	{
		out=a;
		return;
	}
	
	int na = 0, nb = 0;
	
	bool sa = false, sb = false;
	
	while(na < a.size() || nb < b.size())
	{
		if(na == a.size())
		{
			sb = IS_POSITIVE(nb);		
			if(IS_POSITIVE(out.size()) == func(sa,sb))
				out.push_back(b[nb]);
			++nb;
		}
		else if (nb == b.size())
		{
			sa = IS_POSITIVE(na);		
			if(IS_POSITIVE(out.size()) == func(sa,sb))
				out.push_back(a[na]);
			++na;
		}
		else
		{
			if(a[na] < b[nb])
			{
				sa = IS_POSITIVE(na);		
				if(IS_POSITIVE(out.size()) == func(sa,sb))
					out.push_back(a[na]);
				++na;
			}
			else if(a[na] > b[nb])
			{
				sb = IS_POSITIVE(nb);		
				if(IS_POSITIVE(out.size()) == func(sa,sb))
					out.push_back(b[nb]);
				++nb;			
			}
			else
			{
				sa = IS_POSITIVE(na);		
				sb = IS_POSITIVE(nb);		
				if(IS_POSITIVE(out.size()) == func(sa,sb))
					out.push_back(b[nb]);
				++na;
				++nb;
			}
		}
	}
}

void IntersectRanges(vector<double>& o, const vector<double>& a)
{
	vector<double>	temp;
	IntersectRanges(temp, o, a);
	swap(o, temp);
}



BoxRasterizer::BoxRasterizer(PolyRasterizer * raster, double baseline, double limit, double i)
{
	stop = limit;
	rasterizer = raster;
	interval =  i;
	y = baseline;
	
	double next_event = y+1;
	rasterizer->GetLine(range, next_event);
	
	scan();	
}

	
bool	BoxRasterizer::GetNextBox(double& x1, double& y1, double& x2, double& y2)
{
	if (y >= stop) return false;
	if(rasterizer->DoneScan()) return false;
	
	if(output_idx < output.size())
	{
		y1 = output_y;
		y2 = y1 + interval;
		x1 = output[output_idx];
		x2 = output[output_idx+1];
		output_idx += 2;
		return true;
	}
	else
	{
		scan();
		if(rasterizer->DoneScan()) return false;
		if (y >= stop) return false;
		
//		assert(output.size() >= 2);
		y1 = output_y;
		y2 = y1 + interval;
		x1 = output[output_idx];
		x2 = output[output_idx+1];
		output_idx += 2;
		return true;
	}
}

void BoxRasterizer::scan(void)
{
	while(1)
	{
		if(range.empty())
		{
			// This scan line is already empty.  Either we never had any regions, or the constrictions are too tight.
			// Either way we want to advance to the next full scan line...or maybe even more.
			
			double dy = floor(rasterizer->NextInterestingTime(y+interval));
			y += interval*(floor((dy-y) / interval));
			rasterizer->AdvanceScanline(y);
			if(rasterizer->DoneScan())
				break;
			
			// And load up the next event.
			next_event = y+interval;
			rasterizer->GetLine(range, next_event);
		}
		else
		{		
			bool must_flush = next_event >= y+interval;

			rasterizer->AdvanceScanline(next_event);
			if(rasterizer->DoneScan()) 
				break;
			
			vector<double>	r;
			next_event = y+interval;
			if(must_flush) 
				next_event += interval;
			rasterizer->GetLine(r, next_event);
					
			if(must_flush)
			{
				IntersectRanges(output, r, range);
				range = r;
				
				if(!output.empty())
				{
					output_idx = 0;
					output_y = y;
					y += interval;
					return;
				}
				y += interval;
			}
			else
			{
				IntersectRanges(range, r);
			}
		}
	}
}


