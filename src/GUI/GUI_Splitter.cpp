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

#include "GUI_Splitter.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "GUI_Resources.h"
#include "GUI_DrawUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

//const int	gui_SplitSize = 5;

GUI_Splitter::GUI_Splitter(int direction)
	: mDirection(direction),
	  mClick(0)
{
}

GUI_Splitter::~GUI_Splitter()
{
}

void		GUI_Splitter::SetImage(const char * image_res)
{
	mImage = image_res;
}

void		GUI_Splitter::SetImage1(const char * image_res)
{
	mImage1 = image_res;
}

void		GUI_Splitter::SetImage2(const char * image_res)
{
	mImage2 = image_res;
}


void		GUI_Splitter::Draw(GUI_GraphState * state)
{
		int tile[4] = { 0, 0, 1, 1 };

		glColor3f(1,1,1);

	if (!mImage.empty())
	{
		int		bounds[4];
		GetBounds(bounds);
		GUI_DrawStretched(state, mImage.c_str(), bounds, tile);
	}

	if (CountChildren() > 1)
	{
		int		b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);

		if (!mImage1.empty())	GUI_DrawStretched(state,mImage1.c_str(),b1,tile);
		if (!mImage2.empty())	GUI_DrawStretched(state,mImage2.c_str(),b2,tile);

		if (mDirection == gui_Split_Vertical)
		{

			int tile_sel[4] = { 0, mClick ? 1 : 0, 1, 2 };
			b[1] = b1[3];
			b[3] = b2[1];
			GUI_DrawHorizontalStretch(state, "splitter_h.png", b, tile_sel);
		} else {
			b[0] = b1[2];
			b[2] = b2[0];
			int tile_sel[4] = { mClick ? 1 : 0, 0, 2, 1 };
			GUI_DrawVerticalStretch(state, "splitter_v.png", b, tile_sel);
		}
	}
}


int			GUI_Splitter::MouseDown(int x, int y, int button)
{
	if (CountChildren() > 1)
	{
		int b[4];
		GetNthChild(0)->GetBounds(b);

		mSlop = (mDirection == gui_Split_Horizontal ? x : y) -
				(mDirection == gui_Split_Horizontal ? b[2] : b[3]);

		mClick = 1;
		Refresh();
		return 1;
	}
	return 0;
}

void		GUI_Splitter::MouseDrag(int x, int y, int button)
{
	// slop = mouse - boundary so
	// boundary = mouse - slop

	if (CountChildren() > 1)
	{
		int b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);

		if (mDirection == gui_Split_Horizontal)
		{
			b1[2] = x - mSlop;
			b2[0] = x - mSlop + GetSplitSize();
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[3] = y - mSlop;
			b2[1] = y - mSlop + GetSplitSize();
			b1[0] = b2[0] = b[0];
			b1[2] = b2[2] = b[2];
		}
		GetNthChild(0)->SetBounds(b1);
		GetNthChild(1)->SetBounds(b2);
	}
}

void		GUI_Splitter::MouseUp(int x, int y, int button)
{
	mClick = 0;
	Refresh();
}

int			GUI_Splitter::GetCursor(int x, int y)
{
	if (CountChildren() < 2) return gui_Cursor_None;
		int		b1[4], b2[4];

	GetNthChild(0)->GetBounds(b1);
	GetNthChild(1)->GetBounds(b2);

	if (mDirection == gui_Split_Vertical)
	{
		if (y >= b1[3] && y <= b2[1])
			return gui_Cursor_Resize_V;
	} else {
		if (x >= b1[2] && x <= b2[0])
			return gui_Cursor_Resize_H;
	}
	return gui_Cursor_None;
}


void		GUI_Splitter::AlignContents()
{
	if (CountChildren() > 1)
	{
		int b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);

		int split = mDirection == gui_Split_Horizontal ?
			(b1[2] + b2[0] + GetSplitSize()) / 2 :
			(b1[3] + b2[1] + GetSplitSize()) / 2;

		if (mDirection == gui_Split_Horizontal)
		{
			b1[0] = b[0];
			b1[2] = split;
			b2[0] = split + GetSplitSize();
			b2[2] = b[2];
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[1] = b[1];
			b1[3] = split;
			b2[1] = split + GetSplitSize();
			b2[3] = b[3];
			b1[0] = b2[0] = b[0];
			b1[2] = b2[2] = b[2];
		}
		GetNthChild(0)->SetBounds(b1);
		GetNthChild(1)->SetBounds(b2);
	}

}


void		GUI_Splitter::AlignContentsAt(int split)
{
	if (CountChildren() > 1)
	{
		int b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);

		if (mDirection == gui_Split_Horizontal)
		{
			b1[0] = b[0];
			b1[2] = split;
			b2[0] = split + GetSplitSize();
			b2[2] = b[2];
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[1] = b[1];
			b1[3] = split;
			b2[1] = split + GetSplitSize();
			b2[3] = b[3];
			b1[0] = b2[0] = b[0];
			b1[2] = b2[2] = b[2];
		}
		GetNthChild(0)->SetBounds(b1);
		GetNthChild(1)->SetBounds(b2);
	}

}

int		GUI_Splitter::GetSplitPoint(void)
{
	if (CountChildren() > 1)
	{
		int b1[4], b2[4];
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);

		if (mDirection == gui_Split_Horizontal)
		{
			return b1[2];
		} else {
			return b2[1];
		}
	}
	return 0;
}


int		GUI_Splitter::GetSplitSize(void)
{
	if (mDirection == gui_Split_Vertical)
	{
		return GUI_GetImageResourceHeight("splitter_h.png") / 2;
	} else {
		return GUI_GetImageResourceWidth("splitter_v.png") / 2;
	}
}
