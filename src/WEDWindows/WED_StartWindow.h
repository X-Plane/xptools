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

#ifndef WED_StartWindow_H
#define WED_StartWindow_H

#include "GUI_Window.h"
#include "GUI_Listener.h"

class	GUI_Button;
class	GUI_ScrollerPane;
class	GUI_TextTable;
class	GUI_Table;
class	WED_PackageListAdapter;

class WED_StartWindow : public GUI_Window, public GUI_Listener  {
public:

					 WED_StartWindow(GUI_Commander * cmder);
	virtual			~WED_StartWindow();

			void	ShowMessage(const string& msg);

	virtual	bool	Closed(void);
	
	virtual	int		MouseMove(int x, int y			  );
	virtual	void	Draw(GUI_GraphState * state);
	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

	virtual	int				KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)	 	;
	virtual	int				HandleCommand(int command) 									;
	virtual	int				CanHandleCommand(int command, string& ioName, int& ioCheck) ;

	virtual void			Activate(int inActive);

private:

			void			RecomputeButtonEnables();

	string				mCaption;

	GUI_Button *		mNew;
	GUI_Button *		mOpen;
	GUI_Button *		mChange;
	GUI_ScrollerPane *	mScroller;
	
	GUI_Table *			mTable;
	GUI_TextTable *					mTextTable;
	
	WED_PackageListAdapter *		mPackageList;
};

#endif /* WED_StartWindow_H */
