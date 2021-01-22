/*
 * Copyright (c) 2021, Laminar Research.
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

#include "WED_Road_Selector.h"
#include "AssertUtils.h"
#include "GUI_DrawUtils.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"
#include "WED_EnumSystem.h"
#include "WED_Colors.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#define HGT   18           // height of each text row
#define MARG   5           // padding all round the text fields
#define ICON  18           // width of the checkmark icon preceding a text field

WED_Road_Selector::WED_Road_Selector(GUI_Commander * parent, const GUI_EnumDictionary& dict) :
	mChoice(-1),
	mR(0), mC(0),
	mRows(0), mCols(0),
	GUI_EditorInsert(parent)
{
	for(int i = 0; i < ROADSEL_MAX_ROWS; ++i)
		for(int j = 0; j < 2; ++j)
			mDict[i][j] = road_entry();

	mColWidth[0] = 0; mColWidth[1] = 0;
	int rows = 0;

	for(auto d : dict)
	{
		if(rows >= ROADSEL_MAX_ROWS)
		{
			if(mCols < 1) mCols++;
			else continue;
			rows = 0;
		}
		mDict[rows][mCols].name = d.second.first;
		mDict[rows][mCols].enu = d.first;
		mRows = max(mRows, rows);

		int col_width = GUI_MeasureRange(font_UI_Basic, d.second.first.c_str(), d.second.first.c_str()+d.second.first.size()) + ICON + MARG;
		mColWidth[mCols] = max(mColWidth[mCols], col_width);

		rows++;
	}
	mCols++;
}

void	WED_Road_Selector::Draw(GUI_GraphState * g)
{
	int b[4];
	GetBounds(b);
	g->SetState(0,0,0, 0,0, 0,0);
	glColor4fv(WED_Color_RGBA(wed_Tabs_Text));
	glBegin(GL_QUADS);
	glVertex2i(b[0],b[1]);
	glVertex2i(b[0],b[3]);
	glVertex2i(b[2],b[3]);
	glVertex2i(b[2],b[1]);
	glEnd();

	int tab_top  = b[3] - MARG;
	int tab_left = b[0] + MARG;

	for(int j = 0; j < mCols; ++j)
	{
		for(int i = 0; i < mRows; ++i)
		{
			if(mDict[i][j].name.empty()) continue;

			int box[4];
			box[0] = tab_left + ICON + 2;
			box[1] = tab_top - HGT * (i+1);
			box[2] = tab_left + mColWidth[j];
			box[3] = tab_top - HGT * i;

			if(i == mR && j == mC)
			{
				g->SetState(0,0,0, 0,0, 0,0);
				glColor4fv(WED_Color_RGBA(wed_TextField_Hilite));
				glBegin(GL_QUADS);
				glVertex2i(box[0],box[1]);
				glVertex2i(box[0],box[3]);
				glVertex2i(box[2],box[3]);
				glVertex2i(box[2],box[1]);
				glEnd();
			}
			GUI_FontDraw(g, font_UI_Basic, WED_Color_RGBA(wed_TextField_Text), box[0]+2, box[1] + 4, mDict[i][j].name.c_str());

			if (mDict[i][j].enu > 0)
			{
				box[0] = tab_left + ICON;
				box[2] = box[0] + ICON;

				glColor4f(1,1,1,1);
				int selector[4] = { 0, 0, 1, 1 };

				if(mDict[i][j].checked)
				{
					box[0] -= ICON;
					box[2] -= ICON;
				 	selector[0] = 1;
					selector[2] = 2;
					glColor4f(0,0,0,1);
					GUI_DrawCentered(g, "check.png", box, 0, 0, selector, NULL, NULL);
				}
			}
		}
		tab_left += mColWidth[j];
	}
}

int		WED_Road_Selector::MouseMove(int x, int y)
{
	int b[4];
	GetBounds(b);

	mR = (b[3] - MARG - y) / HGT;
	mC = x > b[0] + mColWidth[0] ? 1 : 0;
	Refresh();
	return 1;
}

int		WED_Road_Selector::MouseDown(int x, int y, int button)
{
	int b[4];
	GetBounds(b);

	mR = (b[3] - MARG - y) / HGT;
	mC = x > b[0] + mColWidth[0] ? 1 : 0;
//	printf("click @ %d,%d: r,c %d,%d\n",x,y, mR,mC);
	mChoice = mDict[mR][mC].enu;

	if(mChoice >= 0)
		DispatchKeyPress(GUI_KEY_RETURN, GUI_VK_RETURN, GetModifiersNow());

	return 1;
}

int		WED_Road_Selector::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if(inFlags & gui_DownFlag)
		switch(inKey)
		{
			case GUI_KEY_LEFT:
				if (mC > 0) mC--;
				Refresh();
				return 1;
			case GUI_KEY_RIGHT:
				if (mC < mCols-1) mC++;
				Refresh();
				return 1;
			case GUI_KEY_UP:
				if (mR > 0) mR--;
				Refresh();
				return 1;
			case GUI_KEY_DOWN:
				if (mR < mRows-1) mR++;
				Refresh();
				return 1;
			case GUI_KEY_RETURN:
				mChoice = mDict[mR][mC].enu;
				return 0;
		}
	return 0;
}

bool WED_Road_Selector::SetData(const GUI_CellContent& c)
{
	for(int i = 0; i < mRows; ++i)
		for(int j = 0; j < mCols; ++j)
		{
			if(mDict[i][j].enu == c.int_val)
			{
				mDict[i][j].checked = true;
				mR = i; mC = j;
				Refresh();
				return true; // found;
			}
		}
	return false; // could not find selection in available choices
}

void WED_Road_Selector::GetSizeHint(int * w, int * h)
{
	int tot_width = 2 * MARG;
	for(int i = 0; i < mCols; ++i)
		tot_width += mColWidth[i];

	*w = tot_width;
	*h = 2 * MARG + HGT * mRows;
}

void WED_Road_Selector::GetData(GUI_CellContent& c)
{
	c.int_val = mChoice;            // lines and lights are also exclusive sets, so only int_val is used in AcceptEdit();
}
