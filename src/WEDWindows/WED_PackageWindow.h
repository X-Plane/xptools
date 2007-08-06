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

#ifndef WED_PACKAGEWINDOW_H
#define WED_PACKAGEWINDOW_H

#include "GUI_Window.h"
#include "GUI_Listener.h"

class	WED_Package;

class	WED_PackageWindow : public GUI_Window, public GUI_Listener {
public:

				 WED_PackageWindow(
				 		const char * 	inTitle, 
				 		int 			inBounds[4],
				 		GUI_Commander * inCommander,
				 		WED_Package *	inPackage);
	virtual		~WED_PackageWindow();
	
	virtual	int	KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	int	HandleCommand(int command);
	virtual	int	CanHandleCommand(int command, string& ioName, int& ioCheck);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

	virtual	bool	Closed(void);

private:

	WED_Package *		mPackage;
	
};

#endif
