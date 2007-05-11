#ifndef WED_PROPERTYPANE_H
#define WED_PROPERTYPANE_H

/*
	WED_PropertyPane - THEORY OF OPERATION
	
	The idea of the WED_PropertyPane is to provide easy one-stop installment
	of property-based viewing of WEd-things using a single GUI pane.
	
	The property pane is implemented in terms of a GUI table, fed by a text-table
	adapter, fed by a property table provider.  A header is maintained.
	
	This class contains all of the parts - since GUI children are auto-deleted,
	as are we, the choice of how we contain our sub-objects completely covers
	memory-management issues.
	
*/

#include "GUI_Packer.h"
#include "GUI_TextTable.h"
#include "WED_PropertyTable.h"

class	GUI_Commander;
class	GUI_ScrollerPane;
class	GUI_Table;
class	GUI_Header;
class	GUI_Side;
class	IResolver;

enum {

	propPane_Hierarchy,
	propPane_Filtered,
	propPane_FilteredVertical,
	propPane_Selection

};

class	WED_PropertyPane : public GUI_Packer, public GUI_Commander {
public:

			 WED_PropertyPane(
						GUI_Commander *			inCommander,
						IResolver *				inResolver,
						const char **			col_names,
						int *					def_col_widths,
						GUI_Broadcaster *		archive_broadcaster,
						int						pane_style,
						const char **			filter);
	virtual	~WED_PropertyPane();
	
private:

	GUI_ScrollerPane *				mScroller;
	GUI_Table *						mTable;
	GUI_Header *					mHeader;
	GUI_Side *						mSide; 
	
	GUI_TextTable					mTextTable;
	GUI_TextTableHeader				mTextTableHeader;
	GUI_TextTableSide				mTextTableSide;
	
	WED_PropertyTable				mPropertyTable	;
	
};	

#endif /* WED_PROPERTYPANE_H */