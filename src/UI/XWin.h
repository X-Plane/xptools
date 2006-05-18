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
#ifndef XWIN_H
#define XWIN_H

#if IBM
#include "XWin32DND.h"
#endif

#if APL
#if defined(__MACH__)
#define _STDINT_H_
#endif
#include <Carbon.h>
#define xmenu MenuRef
#endif
#if IBM
#define xmenu HMENU
#endif

class	XWin 
#if IBM
: public XWinFileReceiver
#endif
{
public:

#if APL
		typedef	void *	XContext;
#endif
#if IBM
		typedef HDC		XContext;
#endif				

							XWin();
							XWin(
								const char * 	inTitle,
								int				inX,
								int				inY,
								int				inWidth,
								int				inHeight);
	virtual					~XWin();

	// Manipulators, etc.
			void			SetTitle(const char * inTitle);
			void			MoveTo(int inX, int inY);
			void			Resize(int inWidth, int inHeight);
			void			ForceRefresh(void);
			void			SetTimerInterval(double seconds);
			
			void			GetBounds(int * outX, int * outY);
			void			GetMouseLoc(int * outX, int * outY);
			
			void			SetVisible(bool visible);
			bool			GetVisible(void) const;
			bool			GetActive(void) const;

	// Callbacks
	virtual	void			Timer(void)=0;
	virtual	bool			Closed(void)=0;		// return false to stop
	virtual	void			Resized(int inWidth, int inHeight)=0;

	virtual	void			Update(XContext ctx)=0;
	virtual void			Activate(int inActive)=0;
	virtual	void			ClickDown(int inX, int inY, int inButton)=0;
	virtual	void			ClickUp(int inX, int inY, int inButton)=0;
	virtual	void			ClickDrag(int inX, int inY, int inButton)=0;	// 0 = left
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis)=0;	// 0 = up-down
	virtual	void			DragEnter(int inX, int inY)=0;
	virtual	void			DragOver(int inX, int inY)=0;
	virtual	void			DragLeave(void)=0;
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int inX, int inY)=0;
	virtual	int				KeyPressed(char inKey, long msg, long p1, long p2)=0;
	virtual	int				HandleMenuCmd(xmenu inMenu, int inCommand)=0;
	
			xmenu			GetMenuBar(void);
			void			DrawMenuBar(void);
			
	// Menu API
	static	xmenu			CreateMenu(xmenu parent, int item, const char * inTitle);
	static	int				AppendMenuItem(xmenu menu, const char * inTitle);
	static	int				AppendSeparator(xmenu menu);
	static	void			CheckMenuItem(xmenu menu, int item, bool inCheck);
	static	void			EnableMenuItem(xmenu menu, int item, bool inEnable);
						
protected:

#if APL

		WindowRef				mWindow;
		EventLoopTimerRef		mTimer;

public:
		static pascal OSStatus	MacEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
		static pascal OSErr		MacTrackingHandler(DragTrackingMessage message, WindowRef theWindow, void *handlerRefCon, DragRef theDrag);
		static pascal OSErr		MacReceiveHandler(WindowRef theWindow, void *handlerRefCon, DragRef theDrag);
		static pascal void		MacTimer(EventLoopTimerRef inTimer, void *inUserData);

#endif
#if IBM

		HWND			mWindow;
		CDropTarget *	mDropTarget;
		int				mMouseX;
		int				mMouseY;

		static LRESULT CALLBACK WinEventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	static	void	RegisterClass(HINSTANCE hInstance);
	virtual	void	ReceiveFilesFromDrag(
						const vector<string>& inFiles);

#endif			

};


#endif
