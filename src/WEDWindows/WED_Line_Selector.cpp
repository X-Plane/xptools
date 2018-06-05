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

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


WED_Line_Selector::WED_Line_Selector(GUI_Commander * parent) : mChoice(0), GUI_Commander(parent)
{
}

void	WED_Line_Selector::Draw(GUI_GraphState * g)
{
	int b[4];
	GetBounds(b);
	g->SetState(0,0,0, 0,0, 0,0);
	glColor3f(0.5, 0.5, 0.5);                         // background color
	glBegin(GL_QUADS);
	glVertex2i(b[0],b[1]);
	glVertex2i(b[0],b[3]);
	glVertex2i(b[2],b[3]);
	glVertex2i(b[2],b[1]);
	glEnd();
	
	float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	glColor4fv(white);
	
	int y = b[3] - 15;
	int x = b[0] + 5;
	for(vector<GUI_MenuItem_t>::iterator i = mDict.begin(); i != mDict.end(); ++i)
	{
		g->SetState(0,0,0, 0,0, 0,0);
		if(i->name == NULL) break;
		GUI_FontDraw(g, font_UI_Basic, white, x+20, y, i->name);
	    if (i->checked)
		{
			int selector[4] = { 1, 0, 2, 1 };
			int box[4];
			box[0] = x;
			box[1] = y-2;
			box[2] = x+12;
			box[3] = y+10;
			g->SetState(0,1,0,  0,0, 0,0);
			GUI_DrawCentered(g, "check.png", box, 0, 0, selector, NULL, NULL);
	    }
	    y -= 15;
	    if (y < b[1]+5) { y = b[3]-15; x += 200; }
	}
}

int		WED_Line_Selector::MouseDown(int x, int y, int button)
{
	int b[4];
	GetBounds(b);
	
	// cancel button ?
	if(1)
	{
		if(!IsFocused()) TakeFocus();
		
		mChoice = (b[3] - 5 - y) / 15;
		
		printf("click %d\n",mChoice);
		
		if(mChoice == 16)
			DispatchKeyPress(GUI_KEY_RETURN, GUI_VK_RETURN, 0);
		else if(mChoice == 17)
//			DispatchKeyPress(GUI_KEY_ESCAPE, GUI_VK_ESCAPE, 0);
			DispatchKeyPress(GUI_KEY_ESCAPE, 0, 0);
		
		return 1;
	}
	
	
	return 1;
}

int		WED_Line_Selector::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if(inFlags & gui_DownFlag)
		switch(inKey) 
		{
			case GUI_KEY_LEFT:
			case GUI_KEY_RIGHT:
			case GUI_KEY_UP:
			case GUI_KEY_DOWN:
			case GUI_KEY_TAB:
					return 1;
		}
		
	return 0;
}

int	WED_Line_Selector::AcceptTakeFocus()
{
	Refresh();
	return 1;
}

int	WED_Line_Selector::AcceptLoseFocus(int force)
{
	Refresh();
	return 1;
}

bool WED_Line_Selector::SetSelection(set<int> selected)
{
//	if(0)   return false;                //    could not deal with selection
//	mChoice = selected.begin;
	for(set<int>::iterator i = selected.begin(); i != selected.end(); ++i)
	{
		const char * desc = ENUM_Desc(*i);
		for(int j = 1; j < mDict.size(); ++j)
		{
			if( string(mDict[j].name) == desc)
			{
				mDict[j].checked = true;
				mChoice = j;
				break;
			}
		}
	}	
	Refresh();
	return true;
}

void WED_Line_Selector::SetChoices(const vector<GUI_MenuItem_t> * dict)
{
	mDict = *dict;
	mDict[0].name = "(None)";
	mDict[0].checked = false;
	Refresh();
}

void WED_Line_Selector::GetSelection(int& choices)
{
	int en = ENUM_LookupDesc(LinearFeature, mDict[mChoice].name);

	printf("getsel c=%d en=%d, n=%s\n",mChoice,en,ENUM_Desc(en));
	choices = en;
}
