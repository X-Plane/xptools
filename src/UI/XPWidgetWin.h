/*
 * Copyright (c) 2004, Laminar Research.
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
#ifndef XPWIDGETWIN_H
#define XPWIDGETWIN_H

#include "XWinGL.h"

class	XPWidgetWin : public XWinGL {
public:

							XPWidgetWin();
	virtual					~XPWidgetWin();

	virtual void			Timer(void);
	virtual	void			GLReshaped(int inWidth, int inHeight);
	virtual	void			GLDraw(void);
	virtual	bool			Closed(void);		// return false to stop
	virtual	void			ClickDown(int inX, int inY, int inButton);
	virtual	void			ClickUp(int inX, int inY, int inButton);
	virtual	void			ClickDrag(int inX, int inY, int inButton);	// 0 = left
	virtual	void			ClickMove(int x, int y) { }
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis);	// 0 = up-down
	virtual	void			DragEnter(int x, int y);
	virtual	void			DragOver(int x, int y);
	virtual	void			DragLeave(void);
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int x, int y);
	virtual	int				KeyPressed(char inKey, long m, long p1, long p2);
	virtual	int				HandleMenuCmd(xmenu inMenu, int inCommand);

};

extern XPWidgetWin* gWidgetWin;

#endif