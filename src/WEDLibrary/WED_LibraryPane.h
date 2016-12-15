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

#ifndef WED_LibraryPane_H
#define WED_LibraryPane_H

#include "GUI_Packer.h"
#include "GUI_Commander.h"
#include "GUI_TextTable.h"
#include "WED_LibraryListAdapter.h"

class	GUI_ScrollerPane;
class	WED_LibraryMgr;
class	WED_LibraryFilterBar;

class WED_LibraryPane : public GUI_Packer, public GUI_Commander, public GUI_Listener {
public:

								 WED_LibraryPane(GUI_Commander * commander, WED_LibraryMgr * mgr);
	virtual						~WED_LibraryPane();

	WED_LibraryListAdapter *	GetAdapter(void) { return &mLibraryList; }

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam);

private:

	GUI_ScrollerPane *		mScroller;
	GUI_Table *				mTable;
	GUI_Header *			mHeader;

	GUI_TextTable			mTextTable;
	GUI_TextTableHeader		mTextTableHeader;


	WED_LibraryFilterBar *			mFilter;
	WED_LibraryListAdapter	mLibraryList;

};

#endif /* WED_LibraryPane_H */
