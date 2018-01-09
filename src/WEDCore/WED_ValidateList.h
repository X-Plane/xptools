/* 
 * Copyright (c) 2018, Laminar Research.
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

#ifndef WED_ValidateList_H
#define WED_ValidateList_H

class	GUI_ScrollerPane;
class	GUI_Table;
class	GUI_Header;
class	GUI_TextTable;
class	GUI_TextTableHeader;
class	WED_Document;
class	GUI_FilterBar;
class	WED_MapPane;

#include "GUI_Window.h"
#include "GUI_Listener.h"
#include "GUI_Destroyable.h"

#include "WED_AptTable.h"

class WED_ValidateDialog : public GUI_Window, public GUI_Listener, public GUI_Destroyable {

public:
	WED_ValidateDialog(WED_Document * resolver, WED_MapPane * pane, const validation_error_vector& msg);
	~WED_ValidateDialog();

	virtual void ReceiveMessage(
					GUI_Broadcaster *		inSrc,
					intptr_t    			inMsg,
					intptr_t				inParam);
private:
	WED_Document *		mResolver;
	WED_MapPane *		mMapPane;
	GUI_FilterBar *		mFilter;

	// List of errors and warnings
	GUI_ScrollerPane *		mScroller;
	GUI_Table *				mTable;
	GUI_Header *			mHeader;

	GUI_TextTable			mTextTable;
	GUI_TextTableHeader		mTextTableHeader;
	
	validation_error_vector	msgs_orig;
	double					mZoom;
	// brazenly mis-using these for our list
	AptVector				mMsgs;
	WED_AptTable			mMsgTable;
};

#endif