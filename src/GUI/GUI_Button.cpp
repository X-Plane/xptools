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

#include "GUI_Button.h"
#include "GUI_GraphState.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"
#include "GUI_Fonts.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


GUI_Button::GUI_Button(
		const char *		in_res_name,
		GUI_ButtonType		behavior,
		int					off_regular[4],
		int					off_hilite[4],
		int					on_regular[4],
		int					on_hilite[4]) :
	mBehavior(behavior),
	mResource(in_res_name),
	mHilite(0)
{
	for (int n = 0; n < 4; ++n)
	{
		mCellOffReg[n] = off_regular[n];
		mCellOffHilite[n] = off_hilite[n];
		mCellOnReg[n] = on_regular[n];
		mCellOnHilite[n] = on_hilite[n];
	}
}

GUI_Button::~GUI_Button()
{
}

void	GUI_Button::SetTiles(
		int					off_regular[4],
		int					off_hilite[4],
		int					on_regular[4],
		int					on_hilite[4])
{
	for (int n = 0; n < 4; ++n)
	{
		mCellOffReg[n] = off_regular[n];
		mCellOffHilite[n] = off_hilite[n];
		mCellOnReg[n] = on_regular[n];
		mCellOnHilite[n] = on_hilite[n];
	}
}

void		GUI_Button::AddRadioFriend(GUI_Button * who)
{
	mRadios.push_back(who);
}

void		GUI_Button::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}

int		GUI_Button::MouseMove(int x, int y)
{
	if (mBehavior == btn_Web)
	{
		mHilite = 1;
		for(int n = 0; n <mRadios.size(); ++n)
			mRadios[n]->SetHilite(0);
		Refresh();
	}
	return 1;
}

int			GUI_Button::MouseDown(int x, int y, int button)
{
	mHilite = 1;
	Refresh();
	return 1;
}

void		GUI_Button::MouseDrag(int x, int y, int button)
{
	int b[4];
	GetBounds(b);
	int new_hilite = (x >= b[0] && x <= b[2] && y >= b[1] && y <= b[3]);
	if (new_hilite != mHilite)
	{
		mHilite = new_hilite;
		Refresh();
	}
}

void		GUI_Button::MouseUp  (int x, int y, int button)
{
	int b[4];
	GetBounds(b);
	int new_hilite = (x >= b[0] && x <= b[2] && y >= b[1] && y <= b[3]);
	if (new_hilite)
	{
		switch(mBehavior) {
		case btn_Push:
		case btn_Web:
			SetValue(0.0);
			break;
		case btn_Check:
			SetValue(1.0 - GetValue());
			break;
		case btn_Radio:
			SetValue(1.0);
			for (int n = 0; n < mRadios.size(); ++n)
				mRadios[n]->SetValue(0);
			break;
		}
	}

	mHilite = 0;
	Refresh();
}

void		GUI_Button::Draw(GUI_GraphState * state)
{
	float c[4] = { 1,1,1,1 };
	int w;
	int h;
	int bounds[4];
	string desc;
	glColor3f(1,1,1);
	GetDescriptor(desc);
	GetBounds(bounds);
	int * tile_p = GetValue() > 0 ?
					(mHilite ? mCellOnHilite : mCellOnReg ) :
					(mHilite ? mCellOffHilite : mCellOffReg );
	switch(mBehavior) {
	case btn_Push:
	case btn_Web:
		{
			GUI_DrawHorizontalStretch(state,mResource.c_str(),bounds,tile_p);

			if (!desc.empty())
			{
				w = GUI_MeasureRange(font_UI_Basic, &*desc.begin(), &*desc.end());
				w = (bounds[2] - bounds[0] - w) / 2;
				h = GUI_GetLineAscent(font_UI_Basic);
				h = (bounds[3] - bounds[1] - h) / 2;
				GUI_FontDraw(state, font_UI_Basic, c, bounds[0] + w, bounds[1] + h + GUI_GetLineDescent(font_UI_Basic), desc.c_str());
			}
		}
		break;
	case btn_Check:
	case btn_Radio:
		{
			GUI_DrawCentered(state, mResource.c_str(), bounds, -1, 0, tile_p, &w, &h);

			if (!desc.empty())
				GUI_FontDraw(state, font_UI_Basic, c, bounds[0] + w, bounds[1], desc.c_str());
		}
		break;
	}
}

void		GUI_Button::SetHilite(int hilite)
{
	if (mHilite != hilite)
	{
		mHilite = hilite;
		Refresh();
	}
}
