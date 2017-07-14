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

#include "GUI_TabControl.h"
#include "GUI_Messages.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "GUI_Resources.h"

#define		TAB_PADDING	5
#define		TAB_BASELINE 10
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

GUI_TabControl::GUI_TabControl()
{
	mTextColor[0] = mTextColor[1] = mTextColor[2] = 0.0;
	mTextColor[3] =									1.0;
}

GUI_TabControl::~GUI_TabControl()
{
}

void		GUI_TabControl::SetTextColor(float color[4])
{
	mTextColor[0] = color[0];
	mTextColor[1] = color[1];
	mTextColor[2] = color[2];
	mTextColor[3] = color[3];
}

int		GUI_TabControl::GetNaturalHeight(void)
{
	return GUI_GetImageResourceHeight("tabs.png") / 3;
}

void		GUI_TabControl::SetDescriptor(const string& inDesc)
{
	GUI_Control::SetDescriptor(inDesc);

	mItems.clear();

	string::const_iterator b, e;
	b = inDesc.begin();
	while (b != inDesc.end())
	{
		e = b;
		while (e != inDesc.end() && *e != '\n') ++e;
		mItems.push_back(string(b,e));
		if (e != inDesc.end()) ++e;
		b = e;
	}

	mWidths.resize(mItems.size());
	for (int n = 0; n < mItems.size(); ++n)
	{
		mWidths[n] = GUI_MeasureRange(font_UI_Basic, &*mItems[n].begin(), &*mItems[n].end()) + TAB_PADDING * 2;
	}

	SetMax(mItems.size());
	if (GetValue() > GetMax()) SetValue(GetMax());
	Refresh();
}

void		GUI_TabControl::Draw(GUI_GraphState * state)
{
	int	bounds[4];
	GetBounds(bounds);
	int tile_line[4] = { 0, 0, 1, 3 };
	glColor3f(1,1,1);
	GUI_DrawHorizontalStretch(state,"tabs.png",bounds,tile_line);

	int rs = bounds[2];

	int n;
	for (n = 0; n < mItems.size(); ++n)
	{
		int tile_tab[4] = { 0, (n == GetValue()) ? 2 : 1, 1, 3 };
		bounds[2] = bounds[0] + mWidths[n];
		GUI_DrawHorizontalStretch(state,"tabs.png",bounds,tile_tab);
		bounds[0] = bounds[2];
	}

	if (bounds[0] < rs)
	{
		bounds[2] = max(rs,bounds[0]+40);
		int tile_tab[4] = { 0, 1, 1, 3 };
		GUI_DrawHorizontalStretch(state,"tabs.png",bounds,tile_tab);
	}

	GetBounds(bounds);
	for (n = 0; n < mItems.size(); ++n)
	{
//		GUI_FontDraw(state, font_UI_Basic, (n == mTrackBtn && mHilite) ? ch : c, (bounds[0] + TAB_PADDING), bounds[1] + TAB_BASELINE, mItems[n].c_str());
		GUI_FontDraw(state, font_UI_Basic, mTextColor, (bounds[0] + TAB_PADDING), bounds[1] + TAB_BASELINE, mItems[n].c_str());

		bounds[0] += mWidths[n];
	}

}

int			GUI_TabControl::MouseDown(int x, int y, int button)
{
	int bounds[4];
	GetBounds(bounds);
	mTrackBtn = -1;
	for (int n = 0; n < mWidths.size(); ++n)
	{
		bounds[2] = bounds[0] + mWidths[n];
		if (x > bounds[0] &&
			x < bounds[2])
		{
			mTrackBtn = n;
			mHilite = 1;
			Refresh();
			return 1;
		}
		bounds[0] = bounds[2];
	}
	return 0;
}

void		GUI_TabControl::MouseDrag(int x, int y, int button)
{
	if (mTrackBtn == -1) return;

	int bounds[4];
	GetBounds(bounds);
	for (int n = 0; n < mTrackBtn; ++n)
		bounds[0] += mWidths[n];

	bounds[2] = bounds[0] + mWidths[mTrackBtn];

	int is_in = (x > bounds[0] && x < bounds[2] &&
			  y > bounds[1] && y < bounds[3]);
	if (is_in != mHilite)
	{
		mHilite = is_in;
		Refresh();
	}
}

void		GUI_TabControl::MouseUp  (int x, int y, int button)
{
	if (mTrackBtn == -1) return;

	int bounds[4];
	GetBounds(bounds);
	for (int n = 0; n < mTrackBtn; ++n)
		bounds[0] += mWidths[n];

	bounds[2] = bounds[0] + mWidths[mTrackBtn];

	int is_in = (x > bounds[0] && x < bounds[2] &&
			  y > bounds[1] && y < bounds[3]);
	if (is_in)
	{
		SetValue(mTrackBtn);
	}
	mTrackBtn = -1;
	mHilite = 0;
}

void		GUI_TabControl::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}

