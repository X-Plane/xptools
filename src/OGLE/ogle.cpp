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

#include "ogle.h"
#include <vector>
#include <ctype.h>
#include <math.h>
#include <algorithm>
using std::vector;



/*

		BUG: cut-copy-paste reveals MAJOR repagination problems!
		BUG: scroll-reveal is off by a line on the down side.
		BUG: when we down-arrow or up-arrow and the line is too short, we tend to get in AFTER the return, which makes it seem like
				we go an extra line.  This also gets us stuck in a recursive patern
		BUG: click isnt centered on the MIDDLE of a character.  We need to deal with off-by-one-ish-ness in measure funcs!!
		BUG: scroll wheel busted
		BUG: word-break is bad when we cannot keep a whole word.  Not so good. we take 1 char - we should take the min # of chars??!
		BUG: when the window is bigger than the logical bounds, we are NOT properly aligned. Can we fix this??
		BUG: put char on very end of text.  press return - caret does not advance to next line
			This is because a new line is not started with ZERO chars on it.....hrm....

	TODO: double-click and tripple-click words?

*/

/*******************************************************************************************
 * C IMPLEMENTATION
 *******************************************************************************************/


struct OGLE_Rec {

	// The OGLE_rec doesn't contain any real data - it uses accessors from the callbacks to
	// get them.  Variables below are really internal algorithm control structs.
	OGLE_Callbacks 			callbacks;
	void *					ref;

	// This vector has the char pos of the first char of each line.  Lines are numbered 0 so
	// line_starts 0 is always 0.  This also tells us the number of lines.
	vector<int>				line_starts;

	// This is the character range of the selection (first char, one after the last char).
	// 0 <= sel_start <= sel_end always <= number of chars
	int						sel_start;
	int						sel_end;

	// When we are changing the edit selection with the shift key, we need to know if
	// we are changing the start or end - this stores 0 if the start is being moved or
	// 1 if the end is being moved.
	int						active_side;

	// As we arrow up and down, we remember the vertical line we are trying to trace and
	// recycle it.  This field stores the distance from the logical left side to the
	// insertion point and is reused as we go up or down.  It will be -1.0 if
	// we do not have a gap established yet.  This behavior is important because in a
	// proportionally spaced font, as the insertion point moves left or right due to letter
	// unalgnment, rounding errors could move us constantly left or right.
	float					horizontal_gap;	// -1.0 if no gap used

};

// This routine "safely" returns the first char of a given line, handling
// off the end.
inline int			OGLE_LineStart(OGLE_Handle h, int l, int total)
{
	if (l < 0) return 0;
	if (l >= h->line_starts.size()) return total;
	return h->line_starts[l];
}

// This routine "safely" returns one plus the last char of a given line,
// handling off the end.  (This is the more useful of the two, because
// we need the first char of the next line OR the last char in the text,
// whichever is first a LOT - this saves us the special-case.
inline int			OGLE_LineEnd(OGLE_Handle h, int l, int total)
{
	return OGLE_LineStart(h,l+1,total);
}

// Returns the line number of a given char position by searching the
// line starts.  For speeed we use an STL binary search.
static int OGLE_CharPosToLine(OGLE_Handle h, int charpos)
{
	vector<int>::iterator line_iter = lower_bound(h->line_starts.begin(), h->line_starts.end(), charpos);
	int n = line_iter - h->line_starts.begin();
	// Off the end - last line by definition.
	if (n == h->line_starts.size()) return n-1;
	// Direct hit? We're on the line?
	if (h->line_starts[n] == charpos)	return n;
	// Otherwise lower bound is the NEXT line - back up by 1.
	return n-1;

}

static int OGLE_CoordToCharPos(OGLE_Handle h, float x, float y)
{
	float	bounds[4];
	float	line_height;
	OGLE_Callbacks * cb = &h->callbacks;
	const char * startp, * endp;

	cb->GetLogicalBounds_f(h,bounds);
	cb->GetText_f(h, &startp, &endp);
	line_height = cb->GetLineHeight_f(h);

	if (x < bounds[0]) x = bounds[0];
	if (x > bounds[2]) x = bounds[2];

	int line = floor((bounds[3] - y) / line_height);
	if (line < 0) return 0;
	if (line >= h->line_starts.size()) return endp - startp;

	int ls = OGLE_LineStart(h,line, endp - startp);
	int le = OGLE_LineEnd  (h,line, endp - startp);

	return ls + cb->FitStringFwd_f(h, startp + ls, startp + le, x - bounds[0]);
}

static int OGLE_CharPosVerticalAdjust(OGLE_Handle h, int charpos, int delta, float& old_gap)
{
	// First find line # of char
	const char * start_p, * end_p;
	h->callbacks.GetText_f(h, &start_p, &end_p);
	int line = OGLE_CharPosToLine(h, charpos);
	float xdelta = (old_gap == -1.0) ? h->callbacks.MeasureString_f(h, start_p + OGLE_LineStart(h,line,end_p-start_p), start_p + charpos) : old_gap;
	if (old_gap == -1.0) old_gap = xdelta;

	line += delta;
	if (line < 0) return 0;
	if (line >= (h->line_starts.size())) return end_p - start_p;

	return h->line_starts[line] + h->callbacks.FitStringFwd_f(h, start_p + OGLE_LineStart(h,line,end_p-start_p),start_p + OGLE_LineEnd(h,line,end_p-start_p),xdelta);
}

// Given a blob of text to fit in a width, this returns how many characters fit
// within the line, using our line-break routine.  Also, it will ALWAYS return
// at least ONE character unless that would go past end_p.  (This protects us
// from infinite loops.)
static int			OGLE_LineLengthInternal(
						OGLE_Handle			handle,
						const char *		start_p,
						const char *		end_p,
						float				width)
{
	int		total_chars = 0;
	float	total_width = 0.0;

	while (start_p != end_p)
	{
		const char * cur_p = start_p;
		while (cur_p != end_p && isspace(*cur_p) && *start_p != '\r' && *start_p != '\n')
			cur_p = handle->callbacks.MBCS_Next_f(handle,cur_p);

		total_width += handle->callbacks.MeasureString_f(handle, start_p, cur_p);
		total_chars += (cur_p - start_p);

		// Out of text?  Done!
		if (cur_p == end_p) break;

		// Special case - newline - eat one newline and bail.
		if (*cur_p == '\n' || *cur_p == '\r')
			return total_chars+1;

		const char * break_p = handle->callbacks.WordBreak_f(handle, cur_p, end_p);
		total_width += handle->callbacks.MeasureString_f(handle, cur_p, break_p);

		if (total_width > width)
			return (total_chars > 0 ? total_chars : 1);

		total_chars += (break_p - cur_p);
		start_p = break_p;
	}
	return total_chars;
}

static void			OGLE_RepaginateInternal(
						OGLE_Handle			handle,
						int					start_line,
						int					change_stop_char)	// or 0
{
	float				logical[6];
	const char *		base_p;			// Start of textt
	const char *		start_p;		// Current processing pt
	const char *		end_p;			// End of text
	const char *		safe_p;			// End of changed text
	OGLE_Callbacks *	callbacks = &handle->callbacks;
	float				line_height;

	// MAJOR HACK ALERT!!
	start_line = 0;
	change_stop_char = 0;

	callbacks->GetLogicalBounds_f(handle, logical);
	logical[4] = logical[2] - logical[0];
	logical[5] = logical[3] - logical[1];

	callbacks->GetText_f(handle, &base_p, &end_p);

	line_height = callbacks->GetLineHeight_f(handle);

	if (handle->line_starts.empty())	handle->line_starts.push_back(0);
	else								handle->line_starts[0] = 0;

	if (start_line < 0) start_line = 0;
	if (start_line >= handle->line_starts.size())
		return;

	safe_p = (change_stop_char == 0) ? end_p : (base_p + change_stop_char);
	start_p = base_p + handle->line_starts[start_line];

	// We're going to go through each line and find the next start and put it on
	// the vector.
	do {
		start_p += OGLE_LineLengthInternal(
								handle,
								start_p,
								end_p,
								logical[4]);
		++start_line;

		// Stop case 1: if we've seen this line before AND
		// the stop is the same AND we're after the safe point,
		// well, we're not gonna change.  Early exit.
		if (handle->line_starts.size() > start_line &&
			handle->line_starts[start_line] == (start_p-base_p) &&
			start_p >= safe_p)
		{
			break;
		}

		// Stop case 2.  We're done.  If we have extra line starts
		// laying around (our text must have shrunk) then nuke 'em.
		if (start_p == end_p)
		{
			if (handle->line_starts.size() > start_line)
				handle->line_starts.resize(start_line);
			break;
		}

		if (handle->line_starts.size() <= start_line)
			handle->line_starts.push_back(start_p - base_p);
		else
			handle->line_starts[start_line] = start_p - base_p;
	} while(1);

	callbacks->SetLogicalHeight_f(handle, handle->line_starts.size() * line_height);
}

static void OGLE_NormalizeSelectionInternal(OGLE_Handle handle)
{
	if (handle->sel_end < handle->sel_start)
	{
		swap(handle->sel_end, handle->sel_start);
		handle->active_side = 1 - handle->active_side;
	}

	const char * p1, * p2;
	handle->callbacks.GetText_f(handle,&p1, &p2);
	int len = p2-p1;
	handle->sel_end = min(max(0, handle->sel_end), len);
	handle->sel_start = min(max(0, handle->sel_start), len);
}

OGLE_Handle		OGLE_Create(
						OGLE_Callbacks *	callbacks,
						void *				ref)
{
	OGLE_Rec * rec = new OGLE_Rec;
	memcpy(&rec->callbacks, callbacks, sizeof(rec->callbacks));
	rec->ref = ref;
	rec->sel_start = 0;
	rec->sel_end = 0;
	rec->horizontal_gap = -1.0;
	rec->active_side = 1;
//	OGLE_Repaginate(rec);
	return rec;
}

void			OGLE_Destroy(
						OGLE_Handle 		handle)
{
	delete handle;
}

void *			OGLE_GetRef(
						OGLE_Handle			handle)
{
	return handle->ref;
}


void			OGLE_Draw(
						OGLE_Handle			handle,
						int					draw_caret)
{
	float	logical[4];
	float	visible[4];
	float	line_height;
	int		total_len;
	const char * start_p, * end_p;
	OGLE_Callbacks * callbacks = &handle->callbacks;
	callbacks->GetVisibleBounds_f(handle, visible);
	callbacks->GetLogicalBounds_f(handle, logical);
	line_height = callbacks->GetLineHeight_f(handle);
	callbacks->GetText_f(handle, &start_p, &end_p);
	total_len = end_p - start_p;

	float	sbounds[4];


	int top_line = floor((logical[3] - visible[3]) / line_height);
	int bot_line = ceil((logical[3] - visible[1]) / line_height);

	if (top_line < 0) top_line = 0;
	if (bot_line > handle->line_starts.size()) bot_line = handle->line_starts.size();

	if (handle->sel_end > handle->sel_start)
	for (int l = top_line; l < bot_line; ++l)
	{
		int le = OGLE_LineEnd  (handle,l,total_len);
		if (le <= handle->sel_start)
			continue;

		int	ls = OGLE_LineStart(handle,l,total_len);

		if (handle->sel_start <= ls)
		{
			if (handle->sel_end > le)
			{
				// Whole line is selected
				sbounds[0] = logical[0];
				sbounds[2] = logical[2];
				sbounds[1] = logical[3] - line_height * (l+1);
				sbounds[3] = logical[3] - line_height * (l  );
				callbacks->DrawSelection_f(handle, sbounds);
			}
			else if (handle->sel_end > ls)
			{
				// Selection ends mid-line but started before
				sbounds[0] = logical[0];
				sbounds[2] = logical[0] + callbacks->MeasureString_f(handle, start_p + ls, start_p + handle->sel_end);
				sbounds[1] = logical[3] - line_height * (l+1);
				sbounds[3] = logical[3] - line_height * (l  );
				callbacks->DrawSelection_f(handle, sbounds);
			}
		}
		else if (handle->sel_start < le)
		{
			if (handle->sel_end < le)
			{
				// Selection all on one line
				sbounds[0] = logical[0] + callbacks->MeasureString_f(handle, start_p + ls, start_p + handle->sel_start);
				sbounds[2] = logical[0] + callbacks->MeasureString_f(handle, start_p + ls, start_p + handle->sel_end);
				sbounds[1] = logical[3] - line_height * (l+1);
				sbounds[3] = logical[3] - line_height * (l  );
				callbacks->DrawSelection_f(handle, sbounds);
			}
			else
			{
				// Selection stasrts on lnie and keeps goings
				sbounds[0] = logical[0] + callbacks->MeasureString_f(handle, start_p + ls, start_p + handle->sel_start);
				sbounds[2] = logical[2];
				sbounds[1] = logical[3] - line_height * (l+1);
				sbounds[3] = logical[3] - line_height * (l  );
				callbacks->DrawSelection_f(handle, sbounds);
			}
		}

		// If we're past the selection, no need to do any work.
		if (ls >= handle->sel_end)
			break;
	}

	for (int l = top_line; l < bot_line; ++l)
	{
		const char * o1 = start_p + OGLE_LineStart(handle,l,total_len);
		const char * o2 = start_p + OGLE_LineEnd  (handle,l,total_len);

		callbacks->DrawString_f(handle, o1, o2, logical[0], logical[3] - line_height * (l+1));
	}

	if (draw_caret)
	if (handle->sel_end == handle->sel_start)
	for (int l = top_line; l < bot_line; ++l)
	{
		int ls = OGLE_LineStart(handle, l, total_len);
		int le = OGLE_LineEnd  (handle, l, total_len);
		if (handle->sel_start < ls)
			break;
		if (handle->sel_start < le ||
		   (handle->sel_start == le && l == handle->line_starts.size()-1))
		{
			sbounds[0] = logical[0] + callbacks->MeasureString_f(handle, start_p + ls, start_p + handle->sel_start);
			sbounds[2] = sbounds[0];
			sbounds[1] = logical[3] - line_height * (l+1);
			sbounds[3] = logical[3] - line_height * (l  );
			callbacks->DrawSelection_f(handle, sbounds);
		}
	}
}

void			OGLE_Key(
						OGLE_Handle			handle,
						char				key,
						int					extend)
{
	int sline = OGLE_CharPosToLine(handle, handle->sel_start)-1;
	if (key != ogle_Up && key != ogle_Down)	handle->horizontal_gap = -1.0;
	switch(key) {
	case ogle_DeleteBack:
		if (handle->sel_end == handle->sel_start)
		{
			if (handle->sel_start > 0)
			{
				handle->callbacks.ReplaceText_f(handle, handle->callbacks.MBCS_PrevPos_f(handle,handle->sel_start),handle->sel_start,NULL, NULL);
				handle->sel_start=handle->callbacks.MBCS_PrevPos_f(handle,handle->sel_start);
				handle->sel_end = handle->sel_start;
				OGLE_RepaginateInternal(handle, sline, handle->sel_end);
				OGLE_RevealSelection(handle);
			}
		}
		else
		{
			handle->callbacks.ReplaceText_f(handle, handle->sel_start,handle->sel_end,NULL, NULL);
			handle->sel_end = handle->sel_start;
			OGLE_RepaginateInternal(handle, sline, handle->sel_end);
			OGLE_RevealSelection(handle);
		}
		handle->active_side = 1;
		break;

	case ogle_Left:
		if (extend)
		{
			if (handle->active_side)
				handle->sel_end=handle->callbacks.MBCS_PrevPos_f(handle,handle->sel_end);
			else
				handle->sel_start=handle->callbacks.MBCS_PrevPos_f(handle,handle->sel_start);
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		else
		{
			if (handle->sel_end == handle->sel_start)
				handle->sel_start=handle->callbacks.MBCS_PrevPos_f(handle,handle->sel_start);
			handle->sel_end = handle->sel_start;
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		break;
	case ogle_Right:
		if (extend)
		{
			if (handle->active_side)
				handle->sel_end=handle->callbacks.MBCS_NextPos_f(handle,handle->sel_end);
			else
				handle->sel_start=handle->callbacks.MBCS_NextPos_f(handle,handle->sel_start);
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		else
		{
			if (handle->sel_end == handle->sel_start)
				handle->sel_end=handle->callbacks.MBCS_NextPos_f(handle,handle->sel_end);
			handle->sel_start = handle->sel_end;
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		break;
	case ogle_Up:
		if (extend)
		{
			if (handle->active_side)
				handle->sel_start = OGLE_CharPosVerticalAdjust(handle,handle->sel_start,-1, handle->horizontal_gap);
			else
				handle->sel_end = OGLE_CharPosVerticalAdjust(handle,handle->sel_end,-1, handle->horizontal_gap);
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		else
		{
			if (handle->sel_start == handle->sel_end)
				handle->sel_start = OGLE_CharPosVerticalAdjust(handle,handle->sel_start,-1, handle->horizontal_gap);
			handle->sel_end = handle->sel_start;
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		break;
	case ogle_Down:
		if (extend)
		{
			if (handle->active_side)
				handle->sel_start = OGLE_CharPosVerticalAdjust(handle,handle->sel_start,1, handle->horizontal_gap);
			else
				handle->sel_end = OGLE_CharPosVerticalAdjust(handle,handle->sel_end,1, handle->horizontal_gap);
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		else
		{
			if (handle->sel_start == handle->sel_end)
				handle->sel_end = OGLE_CharPosVerticalAdjust(handle,handle->sel_end,1, handle->horizontal_gap);
			handle->sel_start = handle->sel_end;
			OGLE_NormalizeSelectionInternal(handle);
			OGLE_RevealSelection(handle);
		}
		break;
	default:
		handle->callbacks.ReplaceText_f(handle, handle->sel_start,handle->sel_end,&key,(&key)+1);
		handle->sel_start++;		// Okay - guaranteed input key is NOT MBCS!
		handle->sel_end = handle->sel_start;
		handle->active_side = 1;
		OGLE_RepaginateInternal(handle, sline, handle->sel_end);
			OGLE_RevealSelection(handle);
		break;
	}
}

void			OGLE_Key_MBCS(
						OGLE_Handle			handle,
						int					count,
						const char			keys[],
						int					extend)
{
	if(count == 1)
	{
		OGLE_Key(handle,keys[0],extend);
		return;
	}
	
	int sline = OGLE_CharPosToLine(handle, handle->sel_start)-1;
	handle->horizontal_gap = -1.0; 

	handle->callbacks.ReplaceText_f(handle, handle->sel_start,handle->sel_end,keys,keys + count);
	handle->sel_start+=count;
	handle->sel_end = handle->sel_start;
	handle->active_side = 1;
	OGLE_RepaginateInternal(handle, sline, handle->sel_end);
	OGLE_RevealSelection(handle);
}


void			OGLE_Click(
						OGLE_Handle			handle,
						float				x,
						float				y,
						int					extend)
{
	handle->horizontal_gap = -1.0;
	int p = OGLE_CoordToCharPos(handle, x,y);
	if (extend)
	{
		if (p < handle->sel_start)
		{
			handle->sel_start = p;
			handle->active_side = 0;
		} else {
			handle->sel_end = p;
			handle->active_side = 1;
		}
	}
	else
	{
		handle->sel_start = p;
		handle->sel_end = p;
		handle->active_side = 1;
	}
	OGLE_NormalizeSelectionInternal(handle);
	OGLE_RevealSelection(handle);
}

void			OGLE_Drag(
						OGLE_Handle			handle,
						float				x,
						float				y)
{
	handle->horizontal_gap = -1.0;
	int p = OGLE_CoordToCharPos(handle, x,y);
	if (handle->active_side == 0)
		handle->sel_start = p;
	else
		handle->sel_end = p;
	OGLE_NormalizeSelectionInternal(handle);
	OGLE_RevealSelection(handle);
}

void			OGLE_ReplaceText(
						OGLE_Handle			handle,
						int					offset1,
						int					offset2,
						const char *		t1,
						const char *		t2)
{
	int 	new_len = t2-t1;
	int		old_len = offset2 - offset1;
	int		delta = new_len - old_len;

	if (handle->sel_start >= offset1 && handle->sel_start < offset2)
	{
		handle->sel_end += delta;
		if (handle->sel_end < handle->sel_start)
		{
			handle->sel_start += delta;
			if (handle->sel_start < offset1)
				handle->sel_start = offset1;
			handle->sel_end = handle->sel_start;

		}
	}
	else if (handle->sel_start >= offset2)
	{
		handle->sel_start += delta;
		handle->sel_end += delta;
	}

	handle->callbacks.ReplaceText_f(handle, offset1, offset2, t1, t2);

	OGLE_RepaginateInternal(handle, OGLE_CharPosToLine(handle, offset1)-1, offset2);
}

void			OGLE_GetSelection(
						OGLE_Handle			handle,
						int *				offset1,
						int	*				offset2)
{
	if (offset1) *offset1 = handle->sel_start;
	if (offset2) *offset2 = handle->sel_end;
}

void			OGLE_SetSelection(
						OGLE_Handle			handle,
						int					offset1,
						int					offset2)
{
	handle->sel_start = offset1;
	handle->sel_end = offset2;
	OGLE_NormalizeSelectionInternal(handle);
}

void			OGLE_RevealSelection(
						OGLE_Handle			handle)
{
	float	logical[6];
	float	visible[6];
	float	sel_rect[2];
	float	line_height;
	const char * sp, * ep;

	handle->callbacks.GetText_f(handle, &sp, &ep);
	handle->callbacks.GetLogicalBounds_f(handle, logical);
	handle->callbacks.GetVisibleBounds_f(handle, visible);
	logical[4] = logical[2] - logical[0];
	logical[5] = logical[3] - logical[1];
	visible[4] = visible[2] - visible[0];
	visible[5] = visible[3] - visible[1];

	line_height = handle->callbacks.GetLineHeight_f(handle);
	int char_pos = handle->active_side ? handle->sel_end : handle->sel_start;
	int line = OGLE_CharPosToLine(handle, char_pos);

	sel_rect[1] = logical[3] - (line+(1-handle->active_side)) * line_height;
	sel_rect[0] = logical[0] + handle->callbacks.MeasureString_f(handle, sp + OGLE_LineStart(handle, line, ep-sp), sp + char_pos);

	float		where[2] = { visible[0] - logical[0], visible[1] - logical[1] };
	float	old_where[2] = { visible[0] - logical[0], visible[1] - logical[1] };

	if (sel_rect[0] < visible[0])	where[0] = sel_rect[0] - logical[0];
	if (sel_rect[0] > visible[2])	where[0] = sel_rect[0] - logical[0] - visible[4];
	if (sel_rect[1] < visible[1])
									where[1] = sel_rect[1] - logical[1];
	if (sel_rect[1] > visible[3])
									where[1] = sel_rect[1] - logical[1] - visible[5];

	float hmin = 0;
	float hmax = logical[4] - visible[4];
	if (hmax < hmin) hmax = hmin;
	float vmin = 0;
	float vmax = logical[5] - visible[5];
	if (hmax < hmin) hmin = hmax;

//	if (where[0] < hmin) where[0] = hmin;
//	if (where[0] > hmax) where[0] = hmax;
//	if (where[1] < vmin) where[1] = vmin;
//	if (where[1] > vmax) where[1] = vmax;

	if (where[0] != old_where[0] || where[1] != old_where[1])
		handle->callbacks.ScrollTo_f(handle, where);
}

void			OGLE_Repaginate(
						OGLE_Handle			handle)
{
	OGLE_RepaginateInternal(handle,0,0);
}


/*******************************************************************************************
 * C++ WRAPPER OBJECT
 *******************************************************************************************/


OGLE::OGLE()
{
	OGLE_Callbacks	cbs = {
		GetVisibleBoundsCB,
		GetLogicalBoundsCB,
		SetLogicalHeightCB,
		ScrollToCB,
		GetTextCB,
		ReplaceTextCB,
		GetLineHeightCB,
		MeasureStringCB,
		FitStringFwdCB,
		FitStringRevCB,
		DrawStringCB,
		DrawSelectionCB,
		WordBreakCB,
		MBCS_NextCB,
		MBCS_NextPosCB,
		MBCS_PrevPosCB
	};

	mHandle = OGLE_Create(&cbs, reinterpret_cast<void *>(this));
}

OGLE::~OGLE()
{
	OGLE_Destroy(mHandle);
}

void			OGLE::Draw(int draw_caret)
{
	OGLE_Draw(mHandle, draw_caret);
}

void			OGLE::Key(char				key, int extend)
{
	OGLE_Key(mHandle, key, extend);
}

void			OGLE::Key_MBCS(int count, const char				keys[], int extend)
{
	OGLE_Key_MBCS(mHandle, count, keys, extend);
}


void			OGLE::Click(
									float				x,
									float				y,
									int					extend)
{
	OGLE_Click(mHandle, x, y, extend);
}

void			OGLE::Drag(
									float				x,
									float				y)
{
	OGLE_Drag(mHandle,x,y);
}


void			OGLE::DoReplaceText(
					int					offset1,
					int					offset2,
					const char *		t1,
					const char *		t2)
{
	OGLE_ReplaceText(mHandle, offset1, offset2, t1, t2);
}

void			OGLE::GetSelection(
					int *				offset1,
					int	*				offset2)
{
	OGLE_GetSelection(mHandle, offset1, offset2);
}

void			OGLE::SetSelection(
					int					offset1,
					int					offset2)
{
	OGLE_SetSelection(mHandle, offset1, offset2);
}

void			OGLE::RevealSelection(void)
{
	OGLE_RevealSelection(mHandle);
}

void			OGLE::Repaginate(void)
{
	OGLE_Repaginate(mHandle);
}

void			OGLE::GetVisibleBoundsCB(
					OGLE_Handle		handle,
					float			bounds[4])
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->GetVisibleBounds(bounds);
}

void			OGLE::GetLogicalBoundsCB(
					OGLE_Handle		handle,
					float			bounds[4])
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->GetLogicalBounds(bounds);
}

void			OGLE::SetLogicalHeightCB(
					OGLE_Handle		handle,
					float			height)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->SetLogicalHeight(height);
}

void			OGLE::ScrollToCB(
					OGLE_Handle		handle,
					float			where[2])
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->ScrollTo(where);
}

void			OGLE::GetTextCB(
					OGLE_Handle		handle,
					const char **	start_p,
					const char **	end_p)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->GetText(start_p, end_p);
}

void			OGLE::ReplaceTextCB(
					OGLE_Handle		handle,
					int				offset1,
					int				offset2,
					const char *	t1,
					const char *	t2)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->ReplaceText(offset1, offset2, t1, t2);
}

float			OGLE::GetLineHeightCB(
					OGLE_Handle 	handle)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->GetLineHeight();
}

float			OGLE::MeasureStringCB(
					OGLE_Handle 	handle,
					const char * 	tStart,
					const char * 	tEnd)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->MeasureString(tStart,tEnd);
}

int				OGLE::FitStringFwdCB(
					OGLE_Handle 	handle,
					const char * 	tStart,
					const char * 	tEnd,
					float 			space)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->FitStringFwd(tStart,tEnd, space);
}

int				OGLE::FitStringRevCB(
					OGLE_Handle 	handle,
					const char * 	tStart,
					const char * 	tEnd,
					float 			space)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->FitStringRev(tStart, tEnd, space);
}

void			OGLE::DrawStringCB(
					OGLE_Handle		handle,
					const char *	tStart,
					const char *	tEnd,
					float			x,
					float			y)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->DrawString(tStart,tEnd, x,y);
}

void			OGLE::DrawSelectionCB(
					OGLE_Handle		handle,
					float			bounds[4])
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	me->DrawSelection(bounds);
}

const char *	OGLE::WordBreakCB(
					OGLE_Handle		handle,
					const char *	t1,
					const char *	t2)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->WordBreak(t1, t2);
}

const char *	OGLE::MBCS_NextCB(
								OGLE_Handle		handle,
								const char *	ptr)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->MBCS_Next(ptr);
}

int				OGLE::MBCS_NextPosCB(
								OGLE_Handle		handle,
								int				pos)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->MBCS_NextPos(pos);
}

int				OGLE::MBCS_PrevPosCB(
								OGLE_Handle		handle,
								int				pos)
{
	OGLE * me = reinterpret_cast<OGLE *>(OGLE_GetRef(handle));
	return me->MBCS_PrevPos(pos);
}
