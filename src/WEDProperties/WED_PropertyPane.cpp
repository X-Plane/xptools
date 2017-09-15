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

#include "WED_PropertyPane.h"
#include "WED_UIMeasurements.h"
#include "WED_Colors.h"
#include "GUI_Resources.h"
#include "GUI_Messages.h"

#include "WED_HierarchyFilterBar.h"

WED_PropertyPane::WED_PropertyPane(
						GUI_Commander *			inCommander,
						IResolver *				resolver,
						const char **			col_names,
						int *					def_col_widths,
						GUI_Broadcaster *		archive_broadcaster,
						int						pane_style,
						const char **			filter) :
	GUI_Commander(inCommander),
	mTextTable(this,WED_UIMeasurement("table_indent_width"),0),
	mPropertyTable(resolver, col_names, def_col_widths,
			pane_style == propPane_Selection || pane_style == propPane_FilteredVertical,
			pane_style == propPane_Selection,
			pane_style == propPane_Selection,
			filter)
{
	int vertical = pane_style == propPane_Selection || pane_style == propPane_FilteredVertical;
	int horizontal = pane_style == propPane_Filtered;
	int bounds[4] = { 0, 0, 100, 100 };
//	SetBounds(bounds);
	mScroller = new GUI_ScrollerPane(1,1);

//	mScroller->SetImage("gradient.png");
	mScroller->SetParent(this);
	mScroller->Show();
//	mScroller->SetBounds(bounds);
	mScroller->SetSticky(1,1,1,1);

	mTextTable.SetProvider(&mPropertyTable);
	mTextTable.SetGeometry(&mPropertyTable);

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

	mTable = new GUI_Table(pane_style==propPane_Hierarchy);
	mTable->SetGeometry(&mPropertyTable);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->Show();
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mTextTable.SetParentTable(mTable);

	if (pane_style == propPane_Hierarchy)
	{
		mFilter = new WED_HierarchyFilterBar(this);
		mFilter->Show();
		mFilter->SetParent(this);
		mFilter->AddListener(this);
		mFilter->SetSticky(1, 0, 1, 1);
		this->PackPane(mFilter, gui_Pack_Top);
	}

	if (horizontal)
	{
		mTextTableHeader.SetProvider(&mPropertyTable);
		mTextTableHeader.SetGeometry(&mPropertyTable);

		mTextTableHeader.SetImage("header.png");
		mTextTableHeader.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

		mHeader = new GUI_Header(pane_style==propPane_Hierarchy);

		bounds[1] = 0;
		bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;
		mHeader->SetBounds(bounds);
		mHeader->SetGeometry(&mPropertyTable);
		mHeader->SetHeader(&mTextTableHeader);
		mHeader->SetParent(this);
		mHeader->Show();
		mHeader->SetSticky(1,0,1,1);
		mHeader->SetTable(mTable);
	}

	if (vertical)
	{
		mTextTableSide.SetProvider(&mPropertyTable);
		mTextTableSide.SetGeometry(&mPropertyTable);

		mTextTableSide.SetImage("sider.png");
		mTextTableSide.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

		mSide = new GUI_Side;
		bounds[0] = 0;
		bounds[2] = 140;                       // fixed width of the first property colums, showing the names of all properties
		mSide->SetBounds(bounds);
		mSide->SetGeometry(&mPropertyTable);
		mSide->SetSide(&mTextTableSide);
		mSide->SetParent(this);
		mSide->Show();
		mSide->SetSticky(1,1,0,1);
		mSide->SetTable(mTable);
	}

	#if BENTODO
		this is real arbitrary - would be nice if we did not have to just KNOW all the braodcaster reelatoinships outside the impls
	#endif

	if (horizontal)	mTextTableHeader.AddListener(mHeader);		// Header listens to text table to know when to refresh on col resize
	if (horizontal)	mTextTableHeader.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
	if (vertical)	mTextTableSide.AddListener(mSide);		// Header listens to text table to know when to refresh on col resize
	if (vertical)	mTextTableSide.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
					mPropertyTable.AddListener(mTable);			// Table listens to actual property content to know when data itself changes
//	main_splitter->AlignContents();

	if (horizontal)	this->PackPane(mHeader, gui_Pack_Top);
	if (vertical)	this->PackPane(mSide, gui_Pack_Left);
					this->PackPane(mScroller, gui_Pack_Center);

	if ( vertical)  mScroller->PositionSidePane(mSide);
	if (horizontal)	mScroller->PositionHeaderPane(mHeader);

	archive_broadcaster->AddListener(&mPropertyTable);

}

WED_PropertyPane::~WED_PropertyPane()
{
}

void	WED_PropertyPane::FromPrefs(IDocPrefs * prefs,int id)
{
	char buf[256];
	string key = "PropertyPane";
	sprintf(buf,"%d/Closed",id);
	key += buf;

	set<int> mClosedList;
	prefs->ReadIntSetPref(key.c_str(),mClosedList);
	mPropertyTable.SetClosed(mClosedList);
}

void	WED_PropertyPane::ToPrefs(IDocPrefs * prefs,int id)
{
	char buf[256];
	string key = "PropertyPane";
	sprintf(buf,"%d/Closed",id);
	key += buf;

	set<int> mClosedList;
	mPropertyTable.GetClosed(mClosedList);
	prefs->WriteIntSetPref(key.c_str(), mClosedList);
}

void	WED_PropertyPane::ReceiveMessage(
	GUI_Broadcaster *		inSrc,
	intptr_t				inMsg,
	intptr_t				inParam)
{
	if (inMsg == GUI_FILTER_FIELD_CHANGED)
	{
		mPropertyTable.SetFilter(mFilter->GetText());
	}
}
