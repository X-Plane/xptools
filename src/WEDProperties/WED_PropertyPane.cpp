#include "WED_PropertyPane.h"

const char * filters[] = { "WED_Runway", "WED_Taxiway" };

WED_PropertyPane::WED_PropertyPane(
						GUI_Commander *			inCommander,
						IResolver *				resolver,
						const char **			col_names,
						int *					def_col_widths,
						GUI_Broadcaster *		archive_broadcaster,
						int						pane_style,
						const char **			filter) :
	GUI_Commander(inCommander),
	mTextTable(this),
	mPropertyTable(resolver, col_names, def_col_widths, 
			pane_style == propPane_Selection || pane_style == propPane_FilteredVertical,
			pane_style == propPane_Selection,
			pane_style == propPane_Selection,
			filter)
{
	int vertical = pane_style == propPane_Selection || pane_style == propPane_FilteredVertical;
	int bounds[4] = { 0, 0, 100, 100 };
//	SetBounds(bounds);
	mScroller = new GUI_ScrollerPane(1,1);
	mScroller->SetParent(this);
	mScroller->Show();
//	mScroller->SetBounds(bounds);
	mScroller->SetSticky(1,1,1,1);
	
	mTextTable.SetProvider(&mPropertyTable);
	
	mTable = new GUI_Table;
	mTable->SetGeometry(&mPropertyTable);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->Show();
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);	
	mTextTable.SetParentPanes(mTable);
	
	if (!vertical)
	{
		mTextTableHeader.SetProvider(&mPropertyTable);
		mTextTableHeader.SetGeometry(&mPropertyTable);	
		mHeader = new GUI_Header;
		bounds[1] = 0;
		bounds[3] = 20;
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
		mSide = new GUI_Side;
		bounds[0] = 0;
		bounds[2] = 100;
		mSide->SetBounds(bounds);	
		mSide->SetGeometry(&mPropertyTable);
		mSide->SetSide(&mTextTableSide);
		mSide->SetParent(this);
		mSide->Show();
		mSide->SetSticky(1,1,0,1);
		mSide->SetTable(mTable);
	}
	
	#if !DEV
		this is real arbitrary - would be nice if we did not have to just KNOW all the braodcaster reelatoinships outside the impls
	#endif
	
	if (!vertical)	mTextTableHeader.AddListener(mHeader);		// Header listens to text table to know when to refresh on col resize
	if (!vertical)	mTextTableHeader.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
	if (vertical)	mTextTableSide.AddListener(mSide);		// Header listens to text table to know when to refresh on col resize
	if (vertical)	mTextTableSide.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
					mPropertyTable.AddListener(mTable);			// Table listens to actual property content to know when data itself changes
//	main_splitter->AlignContents();
	
	if (!vertical)	this->PackPane(mHeader, gui_Pack_Top);
	if (vertical)	this->PackPane(mSide, gui_Pack_Left);
					this->PackPane(mScroller, gui_Pack_Center);
					
	if ( vertical)  mScroller->PositionSidePane(mSide);
	if (!vertical)	mScroller->PositionHeaderPane(mHeader);
	
	archive_broadcaster->AddListener(&mPropertyTable);
	
}

WED_PropertyPane::~WED_PropertyPane()
{
}

		
	
