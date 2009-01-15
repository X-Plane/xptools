/* 
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_TCEPane_H
#define WED_TCEPane_H

#include "GUI_Pane.h"
#include "GUI_Listener.h"
#include "GUI_Packer.h"
#include "GUI_Commander.h"

#include "WED_TCE.h"

class	IDocPrefs;
class	WED_Archive;
class	IResolver;
class	GUI_Commander;
class	GUI_ToolBar;

class WED_TCEPane  : public GUI_Packer, GUI_Listener, public GUI_Commander {
public:
		
						 WED_TCEPane(GUI_Commander * cmdr, IResolver * resolver, WED_Archive * archive);
	virtual				~WED_TCEPane();

			void		ZoomShowAll(void);

			int			TCE_KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)	 	;
			int			TCE_HandleCommand(int command) 									;
			int			TCE_CanHandleCommand(int command, string& ioName, int& ioCheck) ;

			void		FromPrefs(IDocPrefs * prefs);
			void		ToPrefs(IDocPrefs * prefs);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

private:

	WED_TCE *				mTCE;

	vector<WED_TCELayer *>	mLayers;
	vector<WED_TCEToolNew *>mTools;

	GUI_ToolBar *			mToolbar;

//	GUI_Table *				mTable;
//	GUI_TextTable *			mTextTable;
//	WED_ToolInfoAdapter *	mInfoAdapter;

	IResolver *				mResolver;
	
//	WED_CreatePointTool *	mObjTool;
//	WED_CreatePolygonTool *	mFacTool;
//	WED_CreatePolygonTool * mFstTool;
//	WED_CreatePolygonTool * mStrTool;
//	WED_CreatePolygonTool * mLinTool;
//	WED_CreatePolygonTool * mPolTool;
	
};

#endif /* WED_TCEPane_H */
