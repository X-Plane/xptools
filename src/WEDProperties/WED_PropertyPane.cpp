#include "WED_PropertyPane.h"

WED_PropertyPane::WED_PropertyPane(
						GUI_Commander *			inCommander,
						WED_Thing *				inRoot,
						WED_Select *			inSelect,
						const char **			col_names,
						int *					def_col_widths,
						GUI_Broadcaster *		archive_broadcaster) :
	mTextTable(inCommander),
	mPropertyTable(inRoot, inSelect, col_names, def_col_widths),
	mPropertyTableHeader(col_names, def_col_widths)
{
	int bounds[4] = { 0, 0, 100, 100 };
//	SetBounds(bounds);
	mScroller = new GUI_ScrollerPane(1,1);
	mScroller->SetParent(this);
	mScroller->Show();
//	mScroller->SetBounds(bounds);
	mScroller->SetSticky(1,1,1,1);
	
	mTextTable.SetProvider(&mPropertyTable);
	
	mTable = new GUI_Table;
	mTable->SetGeometry(mPropertyTable.GetGeometry());
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->Show();
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	
	mTextTable.SetParentPanes(mTable);
	
	mTextTableHeader.SetProvider(&mPropertyTableHeader);
	mTextTableHeader.SetGeometry(mPropertyTable.GetGeometry());
	
	mHeader = new GUI_Header;
	bounds[1] = 0;
	bounds[3] = 20;
	mHeader->SetBounds(bounds);	
	mHeader->SetGeometry(mPropertyTable.GetGeometry());
	mHeader->SetHeader(&mTextTableHeader);
	mHeader->SetParent(this);
	mHeader->Show();
	mHeader->SetSticky(1,0,1,1);

	mHeader->SetTable(mTable);
	
//	main_splitter->AlignContents();
	
	this->PackPane(mHeader, gui_Pack_Top);
	this->PackPane(mScroller, gui_Pack_Center);
	
	archive_broadcaster->AddListener(&mPropertyTable);
	
}

WED_PropertyPane::~WED_PropertyPane()
{
}

		
	
