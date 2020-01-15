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
#include "IDocPrefs.h"

class	GUI_Commander;
class	GUI_ScrollerPane;
class	GUI_Table;
class	GUI_Header;
class	GUI_Side;
class	IResolver;
class	GUI_FilterBar;

enum {

	propPane_Hierarchy,
	propPane_Filtered,
	propPane_FilteredVertical,
	propPane_Selection

};

class	WED_PropertyPane : public GUI_Packer, public GUI_Commander, public GUI_Listener {
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

	void		ToPrefs(IDocPrefs * prefs,int id);
	void		FromPrefs(IDocPrefs * prefs,int id);

	virtual int		MouseMove(int x, int y);
	virtual	void	ReceiveMessage(
		GUI_Broadcaster *		inSrc,
		intptr_t    			inMsg,
		intptr_t				inParam);
private:

	GUI_ScrollerPane *				mScroller;
	GUI_Table *						mTable;
	GUI_Header *					mHeader;
	GUI_Side *						mSide;
	
	GUI_FilterBar *					mFilter;
	
	GUI_TextTable					mTextTable;
	GUI_TextTableHeader				mTextTableHeader;
	GUI_TextTableSide				mTextTableSide;

	WED_PropertyTable				mPropertyTable;
};

#endif /* WED_PROPERTYPANE_H */
