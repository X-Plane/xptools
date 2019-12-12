/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_LibraryPane.h"
#include "GUI_ScrollerPane.h"
#include "WED_Colors.h"
#include "WED_LibraryFilterBar.h"
#include "GUI_Resources.h"
#include "GUI_Messages.h"

WED_LibraryPane::WED_LibraryPane(GUI_Commander * commander, WED_LibraryMgr * mgr) :
	GUI_Commander(commander),
	mTextTable(this,15,0),
	mLibraryList(mgr)
{
	int bounds[4] = { 0, 0, 100, 100 };
//	SetBounds(bounds);
	mScroller = new GUI_ScrollerPane(1,1);

	mScroller->SetParent(this);
	mScroller->Show();
	mScroller->SetSticky(1,1,1,1);

	//Library Pane's TextTable's provider is set to be the Library List Adapter
	mTextTable.SetProvider(&mLibraryList);
	
	mTextTable.SetGeometry(&mLibraryList);

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
	mTable->SetGeometry(&mLibraryList);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->Show();
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mTextTable.SetParentTable(mTable);

	mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
	mLibraryList.AddListener(mTable);			// Table listens to actual property content to know when data itself changes

	mFilter = new WED_LibraryFilterBar(this, mgr);
	mFilter->Show();
	mFilter->SetParent(this);
	mFilter->AddListener(this);
	mFilter->SetSticky(1,0,1,1);
	this->PackPane(mFilter,gui_Pack_Top);
	this->PackPane(mScroller, gui_Pack_Center);

					#if DEV
					//PrintDebugInfo();
					#endif
}

WED_LibraryPane::~WED_LibraryPane()
{
}

void	WED_LibraryPane::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	if(inMsg == GUI_FILTER_FIELD_CHANGED)
		mLibraryList.SetFilter(mFilter->GetText(),mFilter->GetEnumValue());
}
