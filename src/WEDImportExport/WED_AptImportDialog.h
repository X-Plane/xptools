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

#ifndef WED_AptImportDialog_H
#define WED_AptImportDialog_H

#include "WED_Document.h"
class	WED_Archive;
class	GUI_FilterBar;
class	WED_MapPane;

#include "GUI_Window.h"
#include "GUI_Listener.h"
#include "GUI_Destroyable.h"

#include "WED_AptTable.h"

class GUI_Broadcaster;

class WED_AptImportDialog : public GUI_Window, public GUI_Listener, public GUI_Destroyable {
		
public:

						 WED_AptImportDialog(GUI_Commander * cmdr, AptVector& apts, const string& path, WED_Document * resolver, WED_Archive * archive, WED_MapPane * pane);
	virtual				~WED_AptImportDialog();
	
	virtual	bool		Closed(void);

			void		DoIt(void);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam);

private:

	WED_MapPane *			mMapPane;

	GUI_FilterBar *			mFilter;

	GUI_ScrollerPane *		mScroller;
	GUI_Table *				mTable;
	GUI_Header *			mHeader;

	GUI_TextTable			mTextTable;
	GUI_TextTableHeader		mTextTableHeader;

	AptVector				mApts;
	
	WED_AptTable			mAptTable;

	WED_Document *			mResolver;
	WED_Archive *			mArchive;
	string					mPath;
};

#endif
