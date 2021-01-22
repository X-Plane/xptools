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

#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include "GUI_Commander.h"
#include "GUI_GraphState.h"
#include "GUI_Pane.h"
#include "XWinGL.h"

#if IBM
class	GUI_Window_DND;
#endif

class	GUI_Window : public XWinGL, public GUI_Pane, public GUI_Commander {
public:

							GUI_Window(const char * inTitle, int inAttributes, const int inBounds[4],GUI_Commander * inCommander);
	virtual					~GUI_Window();

			void			SetClearSpecs(bool inDoClearColor, bool inDoClearDepth, float inClearColor[4]);

							// Sets window bounds, but only if at least a 100x100 pixel corner remains visible.
			void			SetBoundsSafe(int x1, int y1, int x2, int y2);

	// From GUI_Pane
	virtual void			Refresh(void);
	virtual	void			PopupMenu(GUI_Menu menu, int x, int y, int button);
	virtual	int				PopupMenuDynamic(const GUI_MenuItem_t items[], int x, int y, int button, int current);
	virtual bool			IsDragClick(int x, int y, int button);								// Returns true if the click is a drag, false if it is just a mouse release.
	virtual	GUI_DragOperation
							DoDragAndDrop(
									int						x,
									int						y,
									int						button,
									int						where[4],
									GUI_DragOperation		operations,
									int						type_count,
									GUI_ClipType			inTypes[],
									int						sizes[],
									const void *			ptrs[],
									GUI_GetData_f			get_data_f,
									void *					ref);

	virtual void			Show(void);
	virtual void			Hide(void);
	virtual void			SetBounds(int x1, int y1, int x2, int y2);
	virtual void			SetBounds(int inBounds[4]);
	virtual	void			SetDescriptor(const string& inDesc);
	virtual bool			IsActiveNow(void) const;
	virtual bool			IsVisibleNow(void) const;
	virtual	void			GetMouseLocNow(int * out_x, int * out_y);
	virtual bool			IsKeyPressedNow(int virtualKey);

	// From GUI_Commander
	virtual	int				AcceptTakeFocus(void) 	{ return 1; }			// Because we START dispatching from here, do not refuse focus up to the app - we MUST be focused if active!

	// From XWinGL
	virtual	void			Timer(void);
	virtual	bool			Closed(void) { return true; }

	virtual	void			ClickDown(int inX, int inY, int inButton);
	virtual	void			ClickUp(int inX, int inY, int inButton);
	virtual	void			ClickDrag(int inX, int inY, int inButton);
	virtual	void			ClickMove(int inX, int inY);
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis);
	virtual	int				KeyPressed(uint32_t inKey, long msg, long p1, long p2);
	virtual void			Activate(int active);

	// TODO - we need non-stub impl of these!
	virtual	void			DragEnter(int inX, int inY) { }
	virtual	void			DragOver(int inX, int inY) { }
	virtual	void			DragLeave(void) { }
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int inX, int inY) { }


	virtual	void			GLReshaped(int inWidth, int inHeight);
	virtual	void			GLDraw(void);

private:

	friend class GUI_Application;
#if IBM
		static	HWND	AnyHWND(void);		// Used by app - we need to get SOME window to build the FIRST menubar.
		static	void	EnableMenusWin(void);

		GUI_Window_DND *	mDND;
		WNDPROC				mBaseProc;

		static LRESULT CALLBACK SubclassFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		HWND				mToolTip;
		int					mTipBounds[4];

#endif

#if APL

	// For OS SX, we get a bunch of call-backs from XWin because we can't run ObjC code directly in GUI_window.cpp.
	virtual int					CalcHelpTip(int x, int y, int bounds[4], string& msg);
	virtual	int					AdvancedDragEntered(void * ns_dragging_info);
	virtual	int					AdvancedDragUpdated(void * ns_dragging_info);
	virtual	void				AdvancedDragExited(void * ns_dragging_info);
	virtual	int					AdvancedPerformDrop(void * ns_dragging_info);

	virtual	void				GotCommandHack(int command);


#endif

	GUI_GraphState	mState;
	float			mClearColorRGBA[4];
	bool			mClearDepth;
	bool			mClearColor;
	GUI_Pane *		mMouseFocusPane[BUTTON_DIM];
	int				mInDrag;
	int				mLastDragX;
	int				mLastDragY;
#if IBM || LIN
	int				mMouseFocusButton;	// Remembered for a drag-and-drop
#endif
#if LIN

	int Client2OGL_X(int x, void* w);
	int Client2OGL_Y(int y, void* w);
	int OGL2Client_X(int x, void* w);
	int OGL2Client_Y(int y, void* w);

	int             mTipBounds[4];
	bool            mTipIsActive;

	int handle(int e);

	static	GUI_Window *  AnyXWND(void);		// Used by app - we need to get SOME window to build the FIRST menubar.
#endif
};



#endif

