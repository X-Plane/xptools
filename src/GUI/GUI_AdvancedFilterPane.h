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

#ifndef GUI_AdvancedFilterPane_H
#define GUI_AdvancedFilterPane_H

#include "GUI_Commander.h"
#include "GUI_Broadcaster.h"
#include "WED_Messages.h"

#include "GUI_Packer.h"
#include "GUI_Splitter.h"
#include "GUI_TextTable.h"

#include "GUI_Timer.h"

#include "GUI_TextField.h"
#include "GUI_Button.h"

	/*Theory of Operation: The Advanced Filter Options Menu
	*
	* Introduction: 
	*	The Pane contains buttons, boxes, and various GUI elements that set options for filtering.
	* 
	* The Flow of User Input to Program Output: 
	*	1.) User changes the options here,
	*	2.) If a certain small amount of time has passed (to allow users to select multiple options
	*	quickly without having to keep repeating the search) broadcast message
	*	3.) Call the library's DoSearch() Command and pass in the search terms
	*	4.) Do search filters out the results and reflects them in the library pane
	*
	* Diagram in ASCII
	*
	*  -------------------------------------------------------
	* |_Header________________________________________________|
	* |                                  _____________________|
	* | [ ] Checkboxes "File Types"     |TextTable "Sources"|^|
	* |    [ ] .obj                     | [ ] Checkboxes    | |
	* |    [ ] .fac                     |    [ ] "Local"    | |
	* |    [ ] .pol                     |       [ ] Stuff   | |
	* |    [ ] .lin                     |    [ ] "Library"  | |
	* |			                        |       [ ] Stuff   | |
	* |  ______________________         |       [ ] Stuff   | |
	* | |_TextField____________|        |                   | |
	* |  ______________________         |                   | |
	* | |_TextField_"without"__|        |                   | |
	* |									|                   | |
	* |  ______________________         |                   | |
	* | |_Button_"Toggle All"__|        |___________________|v|
	* |_________________________________|_<_______________>___|
	*/

class GUI_AdvancedFilterPane : public GUI_Packer, public GUI_Commander, public GUI_Listener, public GUI_Broadcaster
{
public:
	GUI_AdvancedFilterPane(GUI_Commander * commander);
	~GUI_AdvancedFilterPane(void);
	//--Text Table Methods

	void			ToggleAllOptions();
	string			GetTFWith();
	string			GetTFWithout();

private:
	void SendSearch();
	GUI_ScrollerPane *		mScroller;
	GUI_TextTable *			mTextTable;
	GUI_Header *			mHeader;

	GUI_Button *			mResetAllBtn;
	GUI_TextField *			mTFWith;
	GUI_TextField *			mTFWithout;
	vector<GUI_Pane>		mGUIElements;
};

#endif /* GUI_AdvancedFilterPane_H*/

