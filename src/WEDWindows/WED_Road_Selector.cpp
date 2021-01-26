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
#include "MathUtils.h"
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

WED_Road_Selector::WED_Road_Selector(GUI_Commander * parent, const GUI_EnumDictionary& dict)
	: mChoice(-1),
	mR(0), mC(0),
	mPfx(0), mSfx(0),
	GUI_EditorInsert(parent)
{
	set<string> pfx,sfx;
	for(auto d : dict)
	{
		string name = d.second.first;
		auto p = name.find('/');
		string prefix;
		if(p != string::npos)
		{
			prefix = name.substr(0,p);
			sfx.insert(name.substr(p+1));
		}
		else
			prefix = name;

		if(pfx.insert(prefix).second)
			mRd_prefix.push_back(prefix);
	}

	mColWidth[1] = 0;
	for(auto s : sfx)
	{
		auto col_width = GUI_MeasureRange(font_UI_Basic, s.c_str(), s.c_str()+s.size()) + ICON + MARG;
		if(col_width > mColWidth[1])
			mColWidth[1] = col_width;

		mRd_suffix.push_back(s);
	}
	sort(mRd_suffix.begin(),mRd_suffix.end());
	mRd_suffix.push_back("");

	mColWidth[0] = 0;
	for(auto s : pfx)
	{
		auto col_width = GUI_MeasureRange(font_UI_Basic, s.c_str(), s.c_str()+s.size()) + ICON + MARG;
		if(col_width > mColWidth[0])
			mColWidth[0] = col_width;
	}

	for(auto d : dict)
	{
		string name = d.second.first;
		auto p = name.find('/');

		string prefix, suffix;
		if(p != string::npos)
		{
			prefix =  name.substr(0,p);
			suffix  = name.substr(p+1);
		}
		else
			prefix = name;

		for(auto& r : mRd_prefix)
		{
			if(r.prefix == prefix)
			{
				int idx = find(mRd_suffix.begin(), mRd_suffix.end(), suffix) - mRd_suffix.begin();
				r.combis.push_back(road_choices::sfx_t(idx, d.first));
				break;
			}
		}
	}

	for(auto& r : mRd_suffix)
	{
		auto p = r.find("wet");
		if(p != string::npos)            // replace those 'wet' suffixes by something more meaningful to users
		{
			if(r[p-1] == '/' ) r[p-1] = ' ';
			r.erase(p);
			r += "<no sidewalk>";
		}
		if(r.empty())
			r = "<no suffix>";
	}
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
	glColor4fv(WED_Color_RGBA(wed_Table_Gridlines));
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glVertex2i(b[0],b[1]);
	glVertex2i(b[0],b[3]);
	glVertex2i(b[2],b[3]);
	glVertex2i(b[2],b[1]);
	glEnd();
	glLineWidth(2);

	int tab_top  = b[3] - MARG;
	int tab_left = b[0] + MARG;

	for(int i = 0; i < mRd_prefix.size(); i++)
	{
		int box[4];
		box[0] = tab_left + ICON + 2;
		box[1] = tab_top - HGT * (i+1);
		box[2] = tab_left + mColWidth[0];
		box[3] = tab_top - HGT * i;

		if(i == mR && mC == 0)
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
		GUI_FontDraw(g, font_UI_Basic, WED_Color_RGBA(wed_TextField_Text), box[0]+2, box[1] + 4, mRd_prefix[i].prefix.c_str());

		if(i == mPfx)
		{
			int selector[4] = { 1, 0, 2, 1 };
			box[0] = tab_left;
			box[2] = box[0] + ICON;
			GUI_DrawCentered(g, "check.png", box, 0, 0, selector, NULL, NULL);
		}
	}

	tab_left += mColWidth[0];

	for(int i = 0; i < mRd_suffix.size(); i++)
	{
		int box[4];
		box[0] = tab_left + ICON + 2;
		box[1] = tab_top - HGT * (i+1);
		box[2] = tab_left + mColWidth[1];
		box[3] = tab_top - HGT * i;

		if(i == mR && mC == 1)
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
		WED_Color tcolor = wed_pure_white;
		for(auto c : mRd_prefix[mPfx].combis)
		{
		if(c.idx == i)
			{
				tcolor = wed_TextField_Text;
				break;
			}
		}
		GUI_FontDraw(g, font_UI_Basic, WED_Color_RGBA(tcolor), box[0]+2, box[1] + 4, mRd_suffix[i].c_str());

		if(i == mSfx)
		{
			int selector[4] = { 1, 0, 2, 1 };
			box[0] = tab_left;
			box[2] = box[0] + ICON;
			glColor4fv(WED_Color_RGBA(tcolor));
			GUI_DrawCentered(g, "check.png", box, 0, 0, selector, NULL, NULL);
		}
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

	if(mC == 0)
	{
		mPfx = mR;
	}
	else
	{
		mSfx = mR;
		for(auto d : mRd_prefix[mPfx].combis)
		{
			if(d.idx == mR)
			{
				DispatchKeyPress(GUI_KEY_RETURN, GUI_VK_RETURN, GetModifiersNow());
				break;
			}
		}
	}
	Refresh();
	return 1;
}

int		WED_Road_Selector::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if(inFlags & gui_DownFlag)
		switch(inKey)
		{
			case GUI_KEY_LEFT:
				if (mC)
				{
					mC = 0;
					mR = intmin2(mR, mRd_prefix.size()-1);
				}
				Refresh();
				return 1;
			case GUI_KEY_RIGHT:
				if (mC == 0)
				{
					mC = 1;
					mR = intmin2(mR, mRd_suffix.size()-1);
				}
				Refresh();
				return 1;
			case GUI_KEY_UP:
				if (mR > 0) mR--;
				Refresh();
				return 1;
			case GUI_KEY_DOWN:
				if(mC)
				{
					if(mR < mRd_suffix.size()-1) mR++;
				}
				else
				{
					if(mR < mRd_prefix.size()-1) mR++;
				}
				Refresh();
				return 1;
			case GUI_KEY_RETURN:
				if(mC)
				{
					mSfx = mR;
					for(auto d : mRd_prefix[mPfx].combis)
					{
						if(d.idx == mR)
							return 0;
					}
					Refresh();
					return 1;
				}
				else
				{
					mPfx = mR;
					Refresh();
					return 1;
				}
		}
	return 0;
}

bool WED_Road_Selector::SetData(const GUI_CellContent& c)
{
	mChoice = c.int_val;

	for(int i = 0; i < mRd_prefix.size(); ++i)
		for(int j = 0; j < mRd_prefix[i].combis.size(); ++j)
		{
			if(mRd_prefix[i].combis[j].enu == c.int_val)
			{
				mPfx = i;
				mSfx = mRd_prefix[i].combis[j].idx;
				mR = i; mC = 0;
				Refresh();
				return true; // found;
			}
		}
	return true; //false; // could not find selection in available choices
}

void WED_Road_Selector::GetSizeHint(int * w, int * h)
{
	*w = 2 * MARG + mColWidth[0] + mColWidth[1];
	*h = 2 * MARG + HGT * max(mRd_prefix.size(), mRd_suffix.size());
}

void WED_Road_Selector::GetData(GUI_CellContent& c)
{
	if(mPfx >= 0 && mPfx < mRd_prefix.size())
		for(auto p : mRd_prefix[mPfx].combis)
			if(p.idx == mSfx)
			{
				c.int_val = p.enu;
				return;
			}

	c.int_val = mChoice;
}
