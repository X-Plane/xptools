//
//  WED_Line_Selectorr.cpp
//
//  Created by Michael Minnhaar  5/25/18.
//
//

#include "WED_Line_Selector.h"
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


WED_Line_Selector::WED_Line_Selector(GUI_Commander * parent) : mChoice(-1), mR(0), mC(0), GUI_EditorInsert(parent)
{
}

#define HGT   18           // height of each text row
#define MARG   5           // padding all round the text fields
#define ICON  18           // width of the icon preceding a text field

void	WED_Line_Selector::Draw(GUI_GraphState * g)
{
	int b[4];
	GetBounds(b);
	g->SetState(0,0,0, 0,0, 0,0);
//	glColor4fv(WED_Color_RGBA(wed_TextField_Bkgnd));
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
			box[0] = tab_left + 2*ICON + 2;
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
				string icn(ENUM_Name(mDict[i][j].enu));
				icn += ".png";
				GUI_DrawCentered(g, icn.c_str(), box, 0, 0, selector, NULL, NULL);
				
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

int		WED_Line_Selector::MouseMove(int x, int y)
{
	int b[4];
	GetBounds(b);
	
	mR = (b[3] - MARG - y) / HGT;
	mC = x > b[0] + mColWidth[0] ? 1 : 0;
	Refresh();
	return 1;
}	

int		WED_Line_Selector::MouseDown(int x, int y, int button)
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

int		WED_Line_Selector::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
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

bool WED_Line_Selector::SetData(const GUI_CellContent& c)
{
	bool found(false);
	for(auto s : c.int_set_val)
	{
		for(int i = 0; i < mRows; ++i)
			for(int j = 0; j < mCols; ++j)
			{
				if(mDict[i][j].enu == s)
				{
					mDict[i][j].checked = true;
					mR = i; mC = j;
					found = true;
				}
			}
	}	
	Refresh();
	return found;        // could not find selection in available choices
}

void WED_Line_Selector::SetChoices(const vector<GUI_MenuItem_t> * dict)
{
	for(int i = 0; i < LINESEL_MAX_ROWS; ++i)
		for(int j = 0; j <2; ++j)
			mDict[i][j] = entry();

	mDict[0][0].name = "None";
	mDict[0][0].enu = 0;
			
	mRows = 1;
	mCols = 1; mColWidth[0] = 0; mColWidth[1] = 0;
	for(vector<GUI_MenuItem_t>::const_iterator d = dict->begin()+1; d != dict->end(); ++d)
	{	
		int e = 0;
		if(d->name)
			e = ENUM_LookupDesc(LinearFeature, d->name);
		else
			continue;
			
		float col_width = GUI_MeasureRange(font_UI_Basic, d->name, d->name+strlen(d->name)) + 2*ICON + MARG;

		int linetype = ENUM_Export(e);
		int col = 0;
		if(linetype < 50)
		{
			linetype += 50;
		}
		else if(linetype < 100)
		{
			linetype -= 50;
			mCols = 2;
			col = 1;
		}
		
		int i;
		for(i = 1; i < mRows; ++i)
			if(ENUM_Export(mDict[i][1-col].enu) == linetype)
			{
				mDict[i][col].name = d->name;
				mDict[i][col].enu = e;
				break;
			}
		if(i == mRows && mRows < LINESEL_MAX_ROWS-1)
		{
			mDict[i][col].name = d->name;
			mDict[i][col].enu = e;
			mRows++;
		}
		if(col_width > mColWidth[col])
		  mColWidth[col] = col_width;
	}
}

void WED_Line_Selector::GetSizeHint(int * w, int * h)
{	
	int tot_width = 2 * MARG;
	for(int i = 0; i < mCols; ++i)
		tot_width += mColWidth[i];
	
	*w = tot_width;
	*h = 2 * MARG + HGT * mRows;
}

void WED_Line_Selector::GetData(GUI_CellContent& c)
{
	c.int_val = mChoice;            // lines and lights are also exclusive sets, so only int_val is used in AcceptEdit();
}
