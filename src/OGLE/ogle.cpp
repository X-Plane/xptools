#include "ogle.h"
#include <vector>
using std::vector;



/*

	TODO: handle return key as newline immediate bail.
	TODO: how are spaces handled in word break func?

	OPEN ISSUES:
		when we insert a line or delete a line, the algorithm will cascade...
		what's the best way to catch the optimized cascade case?
*/

/*******************************************************************************************
 * C IMPLEMENTATION
 *******************************************************************************************/


struct OGLE_Rec {
	OGLE_Callbacks 			callbacks;
	void *					ref;

	vector<int>				line_starts;

	int						sel_start;
	int						sel_end;
	int						active_side;	// 0 = start, 1 = end
	
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

static int OGLE_CharPosToLine(OGLE_Handle h, int charpos)
{
	int line = 0;
	const char * startp, * endp;
	h->callbacks.GetText_f(h,&startp,&endp);
	int count = endp - startp;
	while(line < h->line_starts.size() && charpos > OGLE_LineEnd(h, line, count))
		++line;
	if (line >= h->line_starts.size())
		line = h->line_starts.size()-1;
	return line;
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
	if (line >= h->line_starts.size()) return endp - startp;
	if (line < 0) return 0;
	
	int ls = OGLE_LineStart(h,line, endp - startp);
	int le = OGLE_LineEnd  (h,line, endp - startp);
	
	return ls + cb->FitStringFwd_f(h, startp + ls, startp + le, x - bounds[0]);
}

static int OGLE_CharPosVerticalAdjust(OGLE_Handle h, int charpos, int delta)
{
	// First find line # of char
	const char * start_p, * end_p;
	h->callbacks.GetText_f(h, &start_p, &end_p);
	int line = OGLE_CharPosToLine(h, charpos);
	float xdelta = h->callbacks.MeasureString_f(h, start_p + OGLE_LineStart(h,line,end_p-start_p), start_p + charpos);
	
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
			++cur_p;
		
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
	
	callbacks->GetLogicalBounds_f(handle, logical);
	logical[4] = logical[2] - logical[0];
	logical[5] = logical[3] - logical[1];
	
	callbacks->GetText_f(handle, &base_p, &end_p);
	
	line_height = callbacks->GetLineHeight_f(handle);

	if (handle->line_starts.empty())	handle->line_starts.push_back(0);
	else								handle->line_starts[0] = 0;
	
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
						OGLE_Handle			handle)
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
	int sline = OGLE_CharPosToLine(handle, handle->sel_start);
	switch(key) {
	case ogle_DeleteBack:
		if (handle->sel_end == handle->sel_start)
		{
			if (handle->sel_start > 0)
			{
				handle->callbacks.ReplaceText_f(handle, handle->sel_start-1,handle->sel_start,NULL, NULL);
				handle->sel_start--;
				handle->sel_end = handle->sel_start;
				OGLE_RepaginateInternal(handle, sline, handle->sel_end);
			}
		}
		else
		{
			handle->callbacks.ReplaceText_f(handle, handle->sel_start,handle->sel_end,NULL, NULL);
			handle->sel_end = handle->sel_start;
			OGLE_RepaginateInternal(handle, sline, handle->sel_end);
		}
		handle->active_side = 1;
		break;

	case ogle_Left:
		if (extend)
		{
			if (handle->active_side)
				--handle->sel_end;
			else
				--handle->sel_start;
			OGLE_NormalizeSelectionInternal(handle);
		}
		else
		{
			if (handle->sel_end == handle->sel_start)
				--handle->sel_start;
			handle->sel_end = handle->sel_start;
			OGLE_NormalizeSelectionInternal(handle);
		}
		break;
	case ogle_Right:
		if (extend)
		{
			if (handle->active_side)
				++handle->sel_end;
			else
				++handle->sel_start;
			OGLE_NormalizeSelectionInternal(handle);
		}
		else
		{
			if (handle->sel_end == handle->sel_start)
				++handle->sel_end;
			handle->sel_start = handle->sel_end;
			OGLE_NormalizeSelectionInternal(handle);
		}
		break;
	case ogle_Up:
		if (extend)
		{
			if (handle->active_side)		
				handle->sel_start = OGLE_CharPosVerticalAdjust(handle,handle->sel_start,-1);
			else
				handle->sel_end = OGLE_CharPosVerticalAdjust(handle,handle->sel_end,-1);			
			OGLE_NormalizeSelectionInternal(handle);		
		}
		else
		{
			if (handle->sel_start == handle->sel_end)
				handle->sel_start = OGLE_CharPosVerticalAdjust(handle,handle->sel_start,-1);
			handle->sel_end = handle->sel_start;
			OGLE_NormalizeSelectionInternal(handle);		
		}
		break;
	case ogle_Down:
		if (extend)
		{
			if (handle->active_side)		
				handle->sel_start = OGLE_CharPosVerticalAdjust(handle,handle->sel_start,1);
			else
				handle->sel_end = OGLE_CharPosVerticalAdjust(handle,handle->sel_end,1);			
			OGLE_NormalizeSelectionInternal(handle);		
		}
		else
		{
			if (handle->sel_start == handle->sel_end)
				handle->sel_end = OGLE_CharPosVerticalAdjust(handle,handle->sel_end,1);
			handle->sel_start = handle->sel_end;
			OGLE_NormalizeSelectionInternal(handle);		
		}
		break;
	default:		
		handle->callbacks.ReplaceText_f(handle, handle->sel_start,handle->sel_end,&key,(&key)+1);
		handle->sel_start++;
		handle->sel_end = handle->sel_start;
		handle->active_side = 1;
		OGLE_RepaginateInternal(handle, sline, handle->sel_end);
		break;
	}
	
	// left arrow: move left one char, collapse sel if needed
	// right arrow: move right one char, collapse sel if needed

	// up/down arrow:
	//	first collapse in the right direction
	//	then calc position in logical space based on line start and char pos
	//  use fitstringfwd to find that place on the line above/below
	//  clamp and translate back
	
	// delete:
	//	if we have a selection, delete that text, set to 0
	//
	// otherwise: delete char before sel, move sel bkwd 1
	//
	// key:
	//  if we have a sel, replace sel with key
	//  otherwise, insert key after sel, move sel forward 1
	//
	// TODO - RELATIVE REPAGINATION AND LINE BREAKS!
	// Recalc selection LINE numbers
	//
	// Finally, calc new sel pos and scroll bounds and auto scroll?
}

void			OGLE_Click(
						OGLE_Handle			handle,
						float				x,
						float				y,
						int					extend)
{
	int p = OGLE_CoordToCharPos(handle, x,y);
	if (extend)
	{
		if (p < handle->sel_start)
			handle->sel_start = p;
		else
			handle->sel_end = p;		
	}
	else
	{
		handle->sel_start = p;
		handle->sel_end = p;
		handle->active_side =1;
	}
}

void			OGLE_Drag(
						OGLE_Handle			handle,
						float				x,
						float				y)
{
	int p = OGLE_CoordToCharPos(handle, x,y);
	if (handle->active_side == 0)
		handle->sel_start = p;
	else
		handle->sel_end = p;
	OGLE_NormalizeSelectionInternal(handle);
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
	
	int line = 0;
	while (line < handle->line_starts.size() && handle->line_starts[line] < offset1)
		++line;			
	--line;
	if (line < 0) line = 0;
	OGLE_RepaginateInternal(handle, line, offset2);
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
		WordBreakCB
	};
	
	mHandle = OGLE_Create(&cbs, reinterpret_cast<void *>(this));
}

OGLE::~OGLE()
{
	OGLE_Destroy(mHandle);
}

void			OGLE::Draw(void)
{
	OGLE_Draw(mHandle);
}

void			OGLE::Key(char				key, int extend)
{
	OGLE_Key(mHandle, key, extend);
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

					
