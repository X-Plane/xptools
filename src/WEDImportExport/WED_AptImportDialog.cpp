/* 
 * Copyright (c) 2012, Laminar Research.
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

#include "WED_AptImportDialog.h"
#include "WED_AptIE.h"
#include "WED_ToolUtils.h"
#include "WED_Thing.h"
#include "WED_Colors.h"
#include "GUI_Resources.h"
#include "GUI_Packer.h"
#include "GUI_Button.h"
#include "WED_LibraryFilterBar.h"
#include "GUI_Messages.h"
#include "STLUtils.h"
#include "WED_Messages.h"
#include "WED_Document.h"
#include "WED_MapPane.h"
#include "WED_Airport.h"

static int import_bounds_default[4] = { 0, 0, 500, 500 };

enum {
	kMsg_FilterChanged = WED_PRIVATE_MSG_BASE,
	kMsgImport,
	kMsgCancel
};

WED_AptImportDialog::WED_AptImportDialog(
		GUI_Commander * cmdr, 
		AptVector&		apts,
		const string&	file_path,
		WED_Document *	resolver, 
		WED_Archive *	archive,
		WED_MapPane *	pane) : 
	GUI_Window("Import apt.dat", xwin_style_resizable|xwin_style_visible|xwin_style_centered|xwin_style_modal, import_bounds_default, cmdr),
	mTextTable(this,100,0),
	mMapPane(pane),
	mResolver(resolver),
	mArchive(archive),
	mPath(file_path),
	mAptTable(&mApts)
{
	resolver->AddListener(this);

	swap(mApts,apts);

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

	mAptTable.SetFilter(mFilter->GetText());
	
	mScroller = new GUI_ScrollerPane(0,1);
	mScroller->SetParent(this);
	mScroller->Show();
	mScroller->SetSticky(1,1,1,1);

	mTextTable.SetProvider(&mAptTable);
	mTextTable.SetGeometry(&mAptTable);

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
	mTable->SetGeometry(&mAptTable);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->SetSticky(1,1,1,1);
	mTable->Show();	
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mTextTable.SetParentTable(mTable);

	mTextTableHeader.SetProvider(&mAptTable);
	mTextTableHeader.SetGeometry(&mAptTable);

	mTextTableHeader.SetImage("header.png");
	mTextTableHeader.SetColors(
			WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

	mHeader = new GUI_Header(true);

	bounds[1] = 0;
	bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;
	mHeader->SetBounds(bounds);
	mHeader->SetGeometry(&mAptTable);
	mHeader->SetHeader(&mTextTableHeader);
	mHeader->SetParent(this);
	mHeader->Show();
	mHeader->SetSticky(1,0,1,1);
	mHeader->SetTable(mTable);


					mTextTableHeader.AddListener(mHeader);		// Header listens to text table to know when to refresh on col resize
					mTextTableHeader.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
					mAptTable.AddListener(mTable);			// Table listens to actual property content to know when data itself changes

	packer->PackPane(mFilter,gui_Pack_Top);
	packer->PackPane(mHeader,gui_Pack_Top);
	
	int k_reg[4] = { 0, 0, 1, 3 };
	int k_hil[4] = { 0, 1, 1, 3 };
	
	GUI_Button * okay_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	okay_btn->SetBounds(105,5,205,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	okay_btn->Show();
	okay_btn->SetSticky(0,1,1,0);
	okay_btn->SetDescriptor("Import");
	okay_btn->SetMsg(kMsgImport,0);

	GUI_Button * cncl_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	cncl_btn->SetBounds(5,5,105,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	cncl_btn->Show();
	cncl_btn->SetSticky(1,1,0,0);
	cncl_btn->SetDescriptor("Cancel");
	cncl_btn->SetMsg(kMsgCancel,0);
	
	GUI_Pane * holder = new GUI_Pane;
	holder->SetBounds(0,0,210,GUI_GetImageResourceHeight("push_buttons.png") / 3 + 10);
	
	
	okay_btn->SetParent(holder);
	okay_btn->AddListener(this);
	cncl_btn->SetParent(holder);
	cncl_btn->AddListener(this);
	
	holder->SetParent(packer);
	holder->Show();
	holder->SetSticky(1,1,1,0);
	
	packer->PackPane(holder,gui_Pack_Bottom);

	
	packer->PackPane(mScroller,gui_Pack_Center);

	mScroller->PositionHeaderPane(mHeader);
}

WED_AptImportDialog::~WED_AptImportDialog()
{
}

bool	WED_AptImportDialog::Closed(void)
{
	return true;
}

void WED_AptImportDialog::DoIt(void)
{
	WED_Thing * wrl = WED_GetWorld(mResolver);

	AptVector apts;
	
	set<int>	selected;
	mAptTable.GetSelection(selected);
	
	for(int n = 0; n < mApts.size(); ++n)
	if(selected.count(n))
		apts.push_back(mApts[n]);
	
	
	
	if(!apts.empty())
	{
		wrl->StartOperation("Import apt.dat");
		vector<WED_Airport *>	new_apts;
		WED_AptImport(mArchive, wrl, mPath, apts, &new_apts);
		WED_SetAnyAirport(mResolver);

		if(!new_apts.empty())
		{
			ISelection * sel = WED_GetSelect(mResolver);
			sel->Clear();
			for (int n = 0; n < new_apts.size(); ++n)
				sel->Insert(new_apts[n]);
		}

		wrl->CommitOperation();
		mMapPane->ZoomShowSel();
	}
}

void	WED_AptImportDialog::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	switch(inMsg) {
	case kMsg_FilterChanged:	
		mAptTable.SetFilter(mFilter->GetText());
		break;
	case kMsgImport:
		DoIt();
		this->AsyncDestroy();
		break;
	case msg_DocumentDestroyed:
	case kMsgCancel:
		this->AsyncDestroy();
		break;
	}
}

