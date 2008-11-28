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

#ifndef WED_DOCUMENTWINDOW_H
#define WED_DOCUMENTWINDOW_H

#include "GUI_Window.h"
#include "GUI_Listener.h"

//class WED_ObjectLayers;
//class WED_LayerGroup;
//class WED_LayerTable;
//class WED_LayerTableGeometry;


class	WED_MapPane;
class	WED_Document;
class	WED_PropertyTable;
class	WED_PropertyTableHeader;
class	GUI_Splitter;

class	WED_DocumentWindow : public GUI_Window, public GUI_Listener {
public:

				 WED_DocumentWindow(
				 		const char * 	inTitle,
				 		GUI_Commander * inCommander,
				 		WED_Document *	inDocument);
	virtual		~WED_DocumentWindow();

	virtual	int	KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	int	HandleCommand(int command);
	virtual	int	CanHandleCommand(int command, string& ioName, int& ioCheck);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

	virtual	bool	Closed(void);

private:

	WED_Document *				mDocument;
	WED_MapPane *				mMapPane;


	GUI_Splitter *				mMainSplitter;
	GUI_Splitter *				mMainSplitter2;
	GUI_Splitter *				mPropSplitter;	

//	WED_PropertyTable *			mTestTable;
//	WED_PropertyTableHeader *	mTestTableHeader;

//	WED_ObjectLayers *		mObjects;
//	WED_LayerGroup *		mObjectGroup;
//	WED_LayerTable *		mLayerTable;
//	WED_LayerTableGeometry*	mLayerTableGeometry;
};

#endif

