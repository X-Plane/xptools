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
class	WED_Thing;
class	WED_Select;

class	WED_PropertyPane : public GUI_Packer {
public:

			 WED_PropertyPane(
						GUI_Commander *			inCommander,
						WED_Thing *				inRoot,
						WED_Select *			inSelect,
						const char **			col_names,
						int *					def_col_widths,
						GUI_Broadcaster *		archive_broadcaster);						
	virtual	~WED_PropertyPane();
	
private:

	GUI_ScrollerPane *				mScroller;
	GUI_Table *						mTable;
	GUI_Header *					mHeader;
	
	GUI_TextTable					mTextTable;
	GUI_TextTableHeader				mTextTableHeader;
	
	WED_PropertyTable				mPropertyTable	;
	WED_PropertyTableHeader			mPropertyTableHeader	;
	
};	

#endif /* WED_PROPERTYPANE_H */