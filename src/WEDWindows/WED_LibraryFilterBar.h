/* 
 * Copyright (c) 2016, Laminar Research.
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

#ifndef WED_LIBRARYFILTERBAR_H
#define WED_LIBRARYFILTERBAR_H

#include "GUI_Table.h"
#include "WED_FilterBar.h"
#include "GUI_TextTable.h"
#include "GUI_SimpleTableGeometry.h"
#include "WED_LibraryMgr.h"

class	WED_LibraryFilterBar : public WED_FilterBar {
public:
	WED_LibraryFilterBar(
			GUI_Commander *	cmdr,
			WED_LibraryMgr *mLibrary);

	// GUI_TextTableProvider
	virtual void	GetCellContent(
						int							cell_x,
						int							cell_y,
						GUI_CellContent&			the_content);
	virtual	void	GetEnumDictionary(
						int							cell_x,
						int							cell_y,
						GUI_EnumDictionary&			out_dictionary);

private:
	WED_LibraryMgr		*mLibrary;
};						

#endif
