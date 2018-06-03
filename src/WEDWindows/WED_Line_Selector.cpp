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
	Refresh();
}


void WED_Line_Selector::GetSelection(set<int>& choices)
{
	choices.clear();
	int en = ENUM_LookupDesc(LinearFeature, mDict[mChoice].name);

	printf("getsel c=%d en=%d, n=%s\n",mChoice,en,ENUM_Desc(en));
	choices.insert(en);
//	choices.insert(mDict[mChoice].cmd);
}


#include "PlatformUtils.h"

// we mis-use this table here - its good enough for now
#include "WED_AptTable.h"

//--Table Code------------
#include "WED_Colors.h"
#include "WED_Messages.h"
#include "WED_Document.h"
#include "WED_PackageMgr.h"
#include "WED_MapPane.h"
#include "WED_LibraryFilterBar.h"
#include "GUI_Resources.h"
#include "GUI_Button.h"
#include "WED_Airport.h"
#include "WED_ToolUtils.h"
#include "GUI_Application.h"

enum {
	kMsg_FilterChanged = WED_PRIVATE_MSG_BASE,
	kMsg_Cancel
};

static int import_bounds_default[4] = { 0, 0, 800, 300 };

WED_EnumSelectDialog::WED_EnumSelectDialog(GUI_TextTable * resolver,  GUI_CellContent * info, const GUI_EnumDictionary& dict) :
	GUI_Window("Selection List",xwin_style_visible|xwin_style_centered|xwin_style_resizable|xwin_style_modal,import_bounds_default,gApplication),
	mInfo(info),
	mTextTable(this,100,0),
	mMsgTable(&mMsgs)
{
	// mis-using this structure to get the table displayed
	
	for(GUI_EnumDictionary::const_iterator i = dict.begin(); i != dict.end(); ++i)
	{
		if(i->second.second)
		{
			AptInfo_t apt;
			apt.icao = info->int_set_val.count(i->first) ? "Y" : "N";
			apt.kind_code = i->first;
			apt.name = i->second.first;
			mMsgs.push_back(apt);
		}
	}
	resolver->AddListener(this);
	int bounds[4];
	GUI_Packer * packer = new GUI_Packer;
	
	packer->SetParent(this);
	packer->SetSticky(1,1,1,1);
	packer->Show();
	GUI_Pane::GetBounds(bounds);
	packer->SetBounds(bounds);
	packer->SetBkgkndImage ("gradient.png");

	mFilter = new GUI_FilterBar(this,kMsg_FilterChanged,0,"Search:","",false);
	mFilter->Show();
	mFilter->SetSticky(1,0,1,1);
	mFilter->SetParent(packer);
	mFilter->AddListener(this);

	mMsgTable.SetFilter(mFilter->GetText());
	
	mScroller = new GUI_ScrollerPane(0,1);
	mScroller->SetParent(this);
	mScroller->Show();
	mScroller->SetSticky(1,1,1,1);

	mTextTable.SetProvider(&mMsgTable);
	mTextTable.SetGeometry(&mMsgTable);

	mTextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_Table_SelectText),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable.SetTextFieldColors(
				WED_Color_RGBA(wed_TextField_Text),
				WED_Color_RGBA(wed_TextField_Hilite),
				WED_Color_RGBA(wed_TextField_Bkgnd),
				WED_Color_RGBA(wed_TextField_FocusRing));

	mTable = new GUI_Table(true);
	mTable->SetGeometry(&mMsgTable);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->SetSticky(1,1,1,1);
	mTable->Show();	

	mTextTable.SetParentTable(mTable);
	mTextTableHeader.SetProvider(&mMsgTable);
	mTextTableHeader.SetGeometry(&mMsgTable);
	mTextTableHeader.SetImage("header.png");
	mTextTableHeader.SetColors(
			WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

	bounds[1] = 0;
	bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;

	mHeader = new GUI_Header(true);
	mHeader->SetBounds(bounds);
	mHeader->SetGeometry(&mMsgTable);
	mHeader->SetHeader(&mTextTableHeader);
	mHeader->SetParent(this);
	mHeader->Show();
	mHeader->SetSticky(1,0,1,1);
	mHeader->SetTable(mTable);

	mTextTableHeader.AddListener(mHeader);		// Header listens to text table to know when to refresh on col resize
	mTextTableHeader.AddListener(mTable);		// Table  listens to text table header to announce scroll changes (and refresh) on col resize
	mTextTable.AddListener(mTable);				// Table  listens to text table to know when content changes in a resizing way
	mMsgTable.AddListener(mTable);			    // Table  listens to actual property content to know when data itself changes
	
	mMsgTable.AddListener(this);				// We listen to the mMsgTable for changes of selection

	packer->PackPane(mFilter,gui_Pack_Top);
	packer->PackPane(mHeader,gui_Pack_Top);
	
	int k_reg[4] = { 0, 0, 1, 3 };
	int k_hil[4] = { 0, 1, 1, 3 };

	GUI_Pane * holder = new GUI_Pane;
	holder->SetBounds(0,0,370,GUI_GetImageResourceHeight("push_buttons.png") / 3 + 10);
	holder->SetParent(packer);
	holder->Show();
	holder->SetSticky(1,1,1,0);
	
	GUI_Button * cncl_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	cncl_btn->SetBounds(5,5,180,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	cncl_btn->SetSticky(1,1,0,0);
	cncl_btn->SetMsg(kMsg_Cancel,0);
	cncl_btn->SetParent(holder);
	cncl_btn->AddListener(this);
	cncl_btn->SetDescriptor("Done");
	cncl_btn->Show();

	packer->PackPane(holder,gui_Pack_Bottom);
	packer->PackPane(mScroller,gui_Pack_Center);

	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mScroller->PositionHeaderPane(mHeader);
}

WED_EnumSelectDialog::~WED_EnumSelectDialog()
{
}

void WED_EnumSelectDialog::ReceiveMessage(
						GUI_Broadcaster *		inSrc,
						intptr_t    			inMsg,
						intptr_t				inParam)
{
	switch(inMsg) 
	{
		case kMsg_FilterChanged:	
			mMsgTable.SetFilter(mFilter->GetText());
			break;
		case GUI_TABLE_CONTENT_CHANGED:
		{
			set<int>	selected;
			mMsgTable.GetSelection(selected);
			mInfo->int_set_val.clear();
			for(set<int>::iterator i = selected.begin(); i != selected.end(); ++i)
				mInfo->int_set_val.insert(mMsgs[*i].kind_code);
			break;
		}
		case msg_DocumentDestroyed:
		case kMsg_Cancel:
			this->AsyncDestroy();
			break;
	}
}