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

#include "GUI_TextField.h"
#include "GUI_Messages.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_Menus.h"
#include "GUI_Clipboard.h"
#include "GUI_Unicode.h"
#include <ctype.h>

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

GUI_TextField::GUI_TextField(int scrollH, GUI_Commander * parent) :
	GUI_Commander(parent),
	mState(NULL),
	mScrollH(scrollH),
	mCaret(0),
	mFont(font_UI_Basic)
{
	mColorText[0] = 0.0;	mColorText[1] = 0.0;	mColorText[2] = 0.0;	mColorText[3] = 1.0;
	mColorHilite[0] = 1.0;	mColorHilite[1] = 1.0;	mColorHilite[2] = 0.0;	mColorHilite[3] = 1.0;
	mColorBkgnd[0] = 1.0;	mColorBkgnd[1] = 1.0;	mColorBkgnd[2] = 1.0;	mColorBkgnd[3] = 1.0;
	mColorBox[0] = 0.3;		mColorBox[1] = 0.5;		mColorBox[2] = 1.0;		mColorBox[3] = 1.0;

	mLogicalBounds[0] = 0;
	mLogicalBounds[1] = 0;
	mLogicalBounds[2] = 100;
	mLogicalBounds[3] = 100;
	mMargins[0] = mMargins[1] = mMargins[2] = mMargins[3] = 0.0f;

	for (int n = 0; n < 256; ++n)
		mAllowedVK[n] = mAllowed[n] = true;
}

GUI_TextField::~GUI_TextField()
{
}

void		GUI_TextField::SetFont(int font)
{
	mFont = font;
}

void		GUI_TextField::SetColors(
								float text_color[4],
								float hilite_color[4],
								float bkgnd_color[4],
								float box_color[4])
{
	for (int n = 0; n < 4; ++n)
	{
		mColorText[n] = text_color[n];
		mColorHilite[n] = hilite_color[n];
		mColorBkgnd[n] = bkgnd_color[n];
		mColorBox[n] = box_color[n];
	}
}

void		GUI_TextField::SetKeyAllowed(char key, bool allowed)
{
	mAllowed[(unsigned char) key] = allowed;
}

void		GUI_TextField::SetVKAllowed(int vk, bool allowed)
{
	mAllowedVK[(unsigned char) vk] = allowed;
}

void		GUI_TextField::SetMargins(float l, float b, float r, float t)
{
	mMargins[0] = l;
	mMargins[1] = b;
	mMargins[2] = r;
	mMargins[3] = t;
}

void		GUI_TextField::SetWidth(float width)
{
	mLogicalBounds[2] = mLogicalBounds[0] + width;
	ConstrainLogicalBounds();
	Repaginate();
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();
}


void		GUI_TextField::Draw(GUI_GraphState * state)
{
	state->SetState(0,0,0,  1,1, 0,0);
	glColor4fv(mColorBkgnd);
	int b[4];
	GetBounds(b);
	glBegin(GL_QUADS);
	glVertex2i(b[0],b[1]);
	glVertex2i(b[0],b[3]);
	glVertex2i(b[2],b[3]);
	glVertex2i(b[2],b[1]);
	glEnd();
	glLineWidth(2);
	glColor4fv(mColorBox);
	glBegin(GL_LINE_LOOP);
	glVertex2i(b[0],b[1]);
	glVertex2i(b[0],b[3]);
	glVertex2i(b[2],b[3]);
	glVertex2i(b[2],b[1]);
	glEnd();
	glLineWidth(1);

	mState = state;
	OGLE::Draw(mCaret && IsActiveNow() && IsFocused());
	mState = NULL;
}

int			GUI_TextField::MouseDown(int x, int y, int button)
{
	if (!IsFocused())
		TakeFocus();
	Click(x,y,GetModifiersNow() & gui_ShiftFlag);
	Refresh();
	return 1;
}

void		GUI_TextField::MouseDrag(int x, int y, int button)
{
	Drag(x,y);
	Refresh();
}

void		GUI_TextField::MouseUp(int x, int y, int button)
{
}

int			GUI_TextField::ScrollWheel(int x, int y, int dist, int axis)
{
	this->ScrollV(dist);
	return 1;
}

void		GUI_TextField::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);
	if (!mScrollH)
	{
		mLogicalBounds[0] = x1;
		mLogicalBounds[2] = x2;
	}
	ConstrainLogicalBounds();
	Repaginate();
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();
}

void		GUI_TextField::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	if (!mScrollH)
	{
		mLogicalBounds[0] = inBounds[0];
		mLogicalBounds[2] = inBounds[2];
	}
	ConstrainLogicalBounds();
	Repaginate();
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();
}
	
int			GUI_TextField::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if(inKey == 0)		return 0;
	if(!mAllowed[0] && inKey > 255)						return 0;		// use char 0 as the "extended" bit.
	if (inKey < 256 && !mAllowed[(unsigned char) inKey]) return 0;
	if (inKey < 256 && !mAllowedVK[(unsigned char) inVK]) return 0;
	
	UTF8 buf[4];
	int l = UTF8_encode(inKey, buf);
	if(l > 0)
	Key_MBCS(l,(const char*)buf,inFlags & gui_ShiftFlag);
	Refresh();
	return 1;
}

int			GUI_TextField::HandleCommand(int command)
{
	int s1, s2;
	string txt;
	switch(command) {
	case gui_Cut:
		GetSelection(&s1, &s2);
		if (s1 != s2)
		{
			GUI_SetTextToClipboard(mText.substr(s1,s2-s1));
			DoReplaceText(s1, s2, NULL, NULL);
		}
		return 1;
	case gui_Copy:
		GetSelection(&s1, &s2);
		if (s1 != s2)
		{
			GUI_SetTextToClipboard(mText.substr(s1,s2-s1));
		}
		return 1;
	case gui_Paste:
		{
			if (GUI_GetTextFromClipboard(txt))
			{
				GetSelection(&s1, &s2);
				DoReplaceText(s1, s2, &*txt.begin(), &*txt.end());
				SetSelection(s1+txt.size(),s1+txt.size());
			}
		}
		return 1;
	case gui_Clear:
		GetSelection(&s1, &s2);
		if (s1 != s2)
		{
			DoReplaceText(s1, s2, NULL, NULL);
		}
		return 1;
	case gui_SelectAll:
		SetSelection(0, mText.size());
		Refresh();
		return 1;
	}
	return 0;
}

int			GUI_TextField::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case gui_Cut:
	case gui_Copy:
	case gui_Paste:
	case gui_Clear:
	case gui_SelectAll:
		return 1;
	}
	return 0;
}

int			GUI_TextField::AcceptTakeFocus(void)
{
	mCaret = 1;
	Start(0.25);
	return 1;
}

int			GUI_TextField::AcceptLoseFocus(int inForce)
{
	Stop();
	mCaret = 0;
	return 1;
}

int			GUI_TextField::AcceptFocusChain(void)
{
	return 1;
}

void		GUI_TextField::GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4])
{
	int vbounds[4];
	outTotalBounds[0] = mLogicalBounds[0];
	outTotalBounds[1] = mLogicalBounds[1];
	outTotalBounds[2] = mLogicalBounds[2];
	outTotalBounds[3] = mLogicalBounds[3];
	GetBounds(vbounds);
	outVisibleBounds[0] = vbounds[0];
	outVisibleBounds[1] = vbounds[1];
	outVisibleBounds[2] = vbounds[2];
	outVisibleBounds[3] = vbounds[3];
}

void		GUI_TextField::ScrollV(float vOffset)
{
	int	vbounds[4];
	GetBounds(vbounds);
	mLogicalBounds[3] -= mLogicalBounds[1];
	mLogicalBounds[1] = vbounds[1] - vOffset;
	mLogicalBounds[3] += mLogicalBounds[1];
	Refresh();
}

void		GUI_TextField::ScrollH(float hOffset)
{
	int	vbounds[4];
	GetBounds(vbounds);
	mLogicalBounds[2] -= mLogicalBounds[0];
	mLogicalBounds[0] = vbounds[0] - hOffset;
	mLogicalBounds[2] += mLogicalBounds[0];
	Refresh();
}

void		GUI_TextField::TimerFired(void)
{
	mCaret = !mCaret;
	Refresh();
}

void			GUI_TextField::GetVisibleBounds(
						float			bounds[4])
{
	int vbounds[4];
	GetBounds(vbounds);
	bounds[0] = vbounds[0] + mMargins[0];
	bounds[1] = vbounds[1] + mMargins[1];
	bounds[2] = vbounds[2] - mMargins[2];
	bounds[3] = vbounds[3] - mMargins[3];
}

void			GUI_TextField::GetLogicalBounds(
						float			bounds[4])
{
	bounds[0] = mLogicalBounds[0] + mMargins[0];
	bounds[1] = mLogicalBounds[1] + mMargins[1];
	bounds[2] = mLogicalBounds[2] - mMargins[2];
	bounds[3] = mLogicalBounds[3] - mMargins[3];
}

void			GUI_TextField::SetLogicalHeight(
						float 			height)
{
	mLogicalBounds[1] = mLogicalBounds[3] - height;
	ConstrainLogicalBounds();
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();
}

void			GUI_TextField::ScrollTo(
						float			where[2])
{
	int	vbounds[4];
	GetBounds(vbounds);
	mLogicalBounds[2] -= mLogicalBounds[0];
	mLogicalBounds[3] -= mLogicalBounds[1];
	mLogicalBounds[0] = vbounds[0] - where[0];
	mLogicalBounds[1] = vbounds[1] - where[1];
	mLogicalBounds[2] += mLogicalBounds[0];
	mLogicalBounds[3] += mLogicalBounds[1];
	ConstrainLogicalBounds();
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();
}

void			GUI_TextField::GetText(
						const char **	start_p,
						const char **	end_p)
{
	GetDescriptor(mText);
	*start_p = &*mText.begin();
	*end_p = &*mText.end();
}

void			GUI_TextField::ReplaceText(
						int				offset1,
						int				offset2,
						const char *	t1,
						const char *	t2)
{
	GetDescriptor(mText);
	mText.replace(mText.begin()+offset1,mText.begin()+offset2,t1,t2);
	SetDescriptor(mText);
}

float			GUI_TextField::GetLineHeight(void)
{
	return GUI_GetLineHeight(mFont);
}

float			GUI_TextField::MeasureString(
						const char * 	tStart,
						const char * 	tEnd)
{
	return GUI_MeasureRange(mFont, tStart, tEnd);
}

int				GUI_TextField::FitStringFwd(
						const char * 	tStart,
						const char * 	tEnd,
						float 			space)
{
	return GUI_FitForward(mFont, tStart, tEnd, space);
}

int				GUI_TextField::FitStringRev(
						const char * 	tStart,
						const char * 	tEnd,
						float 			space)
{
	return GUI_FitReverse(mFont, tStart, tEnd, space);

}

void			GUI_TextField::DrawString(
						const char *	tStart,
						const char *	tEnd,
						float			x,
						float			y)
{
	int yy = y;
	GUI_FontDrawScaled(mState, mFont, mColorText,
						x, yy, x + 10000, yy + GUI_GetLineHeight(mFont),
						tStart, tEnd, align_Left);
}

void			GUI_TextField::DrawSelection(
								float			bounds[4])
{
	mState->SetState(false, 0, false, false, false, false, false);
	if (bounds[0] == bounds[2])
	{
		glColor4fv(mColorText);
		glBegin(GL_LINES);
		glVertex2f(bounds[0], bounds[1]);
		glVertex2f(bounds[0], bounds[3]);
		glEnd();
	}
	else
	{
		glColor4fv(mColorHilite);
		glBegin(GL_QUADS);
		glVertex2f(bounds[0], bounds[1]);
		glVertex2f(bounds[0], bounds[3]);
		glVertex2f(bounds[2], bounds[3]);
		glVertex2f(bounds[2], bounds[1]);
		glEnd();
	}
}


const char *	GUI_TextField::WordBreak(
						const char *	t1,
						const char *	t2)
{
	const char * p = t1;
	while (p != t2 && isspace(*p))
		++p;
	while (p != t2 && !isspace(*p))
		++p;
	return p;
}


const char *	GUI_TextField::MBCS_Next(
								const char *	ptr)
{
	return (const char *) UTF8_next((const UTF8*) ptr);

}

int				GUI_TextField::MBCS_NextPos(
								int				pos)
{
	if(pos >= mText.size()) return pos+1;					// OGLE will run off end and then fix it...do not attempt to 
	const UTF8 * base = (const UTF8*) mText.c_str();		// access char data to decide if we're wide.
	return UTF8_next(base+pos) - base;

}

int				GUI_TextField::MBCS_PrevPos(
								int				pos)
{
	if(pos == 0) return -1;										// OGLE will run off end and then fix it...do not attempt to 
	const UTF8 * base = (const UTF8*) mText.c_str();			// access char data to decide if we're wide.
	return UTF8_prev(base+pos) - base;
	
}


void	GUI_TextField::ConstrainLogicalBounds(void)
{
	int	vbounds[4];
	GetBounds(vbounds);

	if (mLogicalBounds[2] < vbounds[2])
	{
		mLogicalBounds[0] -= (mLogicalBounds[2] - vbounds[2]);
		mLogicalBounds[2] -= (mLogicalBounds[2] - vbounds[2]);
	}

	if (mLogicalBounds[1] > vbounds[1])
	{
		mLogicalBounds[3] -= (mLogicalBounds[1] - vbounds[1]);
		mLogicalBounds[1] -= (mLogicalBounds[1] - vbounds[1]);
	}

	if (mLogicalBounds[0] > vbounds[0])
	{
		mLogicalBounds[2] -= (mLogicalBounds[0] - vbounds[0]);
		mLogicalBounds[0] -= (mLogicalBounds[0] - vbounds[0]);
	}

	if (mLogicalBounds[3] < vbounds[3])
	{
		mLogicalBounds[1] -= (mLogicalBounds[3] - vbounds[3]);
		mLogicalBounds[3] -= (mLogicalBounds[3] - vbounds[3]);
	}
}
