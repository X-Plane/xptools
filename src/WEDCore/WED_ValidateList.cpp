/* 
 * Copyright (c) 2018, Laminar Research.
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

#include "WED_Validate.h"

#include "WED_ValidateList.h"
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
	kMsg_ZoomTo,
	kMsg_ZoomOut,
	kMsg_Cancel
};

static int import_bounds_default[4] = { 0, 0, 800, 300 };

WED_ValidateDialog::WED_ValidateDialog(WED_Document * resolver, WED_MapPane * pane, const validation_error_vector& msgs) :
	GUI_Window("Validation Result List",xwin_style_visible|xwin_style_centered|xwin_style_resizable|xwin_style_modal,import_bounds_default,gApplication),
	mResolver(resolver),
	mMapPane(pane),
	mTextTable(this,100,0),
	mMsgTable(&mMsgs),
	mZoom(1.05)
{
	// mis-using this structure to get the table displayed
	for(int i = 0; i < msgs.size(); ++i)
	{
		AptInfo_t apt;
		if(msgs[i].airport)
			msgs[i].airport->GetICAO(apt.icao);
		else
			apt.icao = "Off Airport";
		apt.name  = msgs[i].err_code > warnings_start_here ? "Warning: " : "Error: ";
		apt.name += msgs[i].msg;
		mMsgs.push_back(apt);
	}
	msgs_orig = msgs;
	
	mResolver->AddListener(this);
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
	cncl_btn->SetBounds(5,5,155,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	cncl_btn->Show();
	cncl_btn->SetSticky(1,1,0,0);
	cncl_btn->SetDescriptor("Dismiss & edit scenery");
	cncl_btn->SetMsg(kMsg_Cancel,0);
	cncl_btn->SetParent(holder);
	cncl_btn->AddListener(this);

	mZoomBtn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	mZoomBtn->SetBounds(155,5,260,GUI_GetImageResourceHeight("push_buttons.png") / 3);
//	mZoomBtn->Show();
	mZoomBtn->SetSticky(0,1,1,0);
	mZoomBtn->SetDescriptor("Zoom to Issue");
	mZoomBtn->SetMsg(kMsg_ZoomTo,0);
	mZoomBtn->SetParent(holder);
	mZoomBtn->AddListener(this);
	
	mZoomOutBtn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	mZoomOutBtn->SetBounds(265,5,365,GUI_GetImageResourceHeight("push_buttons.png") / 3);
//	mZoomOutBtn->Show();
	mZoomOutBtn->SetSticky(0,1,1,0);
	mZoomOutBtn->SetDescriptor("Zoom out");
	mZoomOutBtn->SetMsg(kMsg_ZoomOut,0);
	mZoomOutBtn->SetParent(holder);
	mZoomOutBtn->AddListener(this);

	packer->PackPane(holder,gui_Pack_Bottom);
	packer->PackPane(mScroller,gui_Pack_Center);

	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mScroller->PositionHeaderPane(mHeader);
}

WED_ValidateDialog::~WED_ValidateDialog()
{
}

void WED_ValidateDialog::ReceiveMessage(
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
		mZoomBtn->Show();
		mZoomOutBtn->Show();
	case kMsg_ZoomTo:
		{
			WED_Thing * wrl = WED_GetWorld(mResolver);
			wrl->StartOperation("Select Invalid");
			ISelection * sel = WED_GetSelect(mResolver);
			sel->Clear();
			set<int>	selected;
			mMsgTable.GetSelection(selected);
			for(set<int>::iterator i = selected.begin(); i != selected.end(); ++i)
				for(vector<WED_Thing *>::iterator b = msgs_orig[*i].bad_objects.begin(); b != msgs_orig[*i].bad_objects.end(); ++b)
					sel->Insert(*b);
			wrl->CommitOperation();
		}
		if (inMsg == GUI_TABLE_CONTENT_CHANGED) break;
		mZoom = 1.05;
		mMapPane->ZoomShowSel(mZoom);
		break;
	case kMsg_ZoomOut:
		mZoom *= 1.4;
		mMapPane->ZoomShowSel(mZoom);
		break;
	case msg_DocumentDestroyed:
	case kMsg_Cancel:
		this->AsyncDestroy();
		break;
	default:
		printf("ValidateDia: src=%p msg=%p\n", inSrc, inMsg);
	}
}

//-------------------------------------------------------------
