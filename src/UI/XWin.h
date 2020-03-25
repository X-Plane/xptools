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

#include <stdint.h>

#if IBM
#include "XWin32DND.h"
#endif

#if APL

// Our helper window is declared here so we don't have to cast it all the time.
#if defined(__OBJC__)
	#import <AppKit/AppKit.h>
	class XWin;

	@interface XWinCocoa : NSWindow <NSDraggingDestination, NSDraggingSource> {
		XWin * mOwner;
		NSView * mView;
	};
	- (void) setOwner:(XWin*)owner withView:(NSView *) view;
	- (NSView *) view;
	- (void) timerFired;
	- (void) menuItemPicked:(id) sender;
	- (void) menu_picked:(id) sender;
	@end

#else
	#define XWinCocoa void
#endif
	typedef void * xmenu;		// Since this is in the public API, we get C++ binding errors if we redefine it by compiler type.
#endif

#if IBM
#define xmenu HMENU
#endif

#if LIN
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_.H>
#define xmenu Fl_Menu_Item *
enum {
  _NET_WM_STATE_REMOVE,
  _NET_WM_STATE_ADD,
  _NET_WM_STATE_TOGGLE
};

typedef struct tagPOINT {
  int x;
  int y;
} POINT, *PPOINT;

typedef struct t_xmenu_cmd{
  Fl_Menu_Item * menu;
  int cmd;
} xmenu_cmd,*x_menu_cmd;

#endif

#define	BUTTON_DIM 16

enum {
	xwin_style_thin				= 0,			// Thin window - just a rectangle
	xwin_style_movable			= 1,			// Movable - but no machinery to resize
	xwin_style_resizable		= 2,			// The works: resize, maximize, minimize, zoom, etc.
	xwin_style_modal			= 4,			// Modal - don't let the user have access to what's behind until they deal with this!
	xwin_style_visible			= 8,			// Start visible?
	xwin_style_centered			= 16,			// Center on screen
	xwin_style_fullscreen		= 32,			// start in full screen mode
	xwin_style_popup			= 64			// popup-window style, i.e. no menubar on Lin/Win
};



class	XWin
#if IBM
: public XWinFileReceiver
#endif
#if LIN
: public Fl_Window
#endif
{

public:

#if IBM
		typedef HDC		XContext;
#else
        typedef void *	XContext;
#endif
	XWin(int default_dnd);
	XWin(
		int		default_dnd,
		const char * 	inTitle,
		int		inAttributes,
		int		inX,
		int		inY,
		int		inWidth,
		int		inHeight);

	virtual					~XWin();

	// Manipulators, etc.
			void			SetTitle(const char * inTitle);
			void			SetFilePath(const char * inFilePath,bool modified);
			void			MoveTo(int inX, int inY);
			void			Resize(int inWidth, int inHeight);
			void			ForceRefresh(void);
			void			UpdateNow(void);
			void			SetTimerInterval(double seconds);

			void			GetBounds(int * outX, int * outY);			// "Matched" to Resize
			void			GetWindowLoc(int * outX, int * outY);		// "Matched" to MoveTo
			void			GetMouseLoc(int * outX, int * outY);
			void			SetVisible(bool visible);
			bool			GetVisible(void) const;
			bool			GetActive(void) const;
#if APL
			void			GetDesktop(int bounds[4]) {};
#else
			void			GetDesktop(int bounds[4]);  // absolute maximum size - encompassing all moniors or displays
#endif
			int				TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int button, int current);

	// Callbacks
	virtual	void			Timer(void)=0;
	virtual	bool			Closed(void)=0;		// return false to stop
	virtual	void			Resized(int inWidth, int inHeight)=0;

	virtual	void			Update(XContext ctx)=0;
	virtual	void			Activate(int inActive)=0;
	virtual	void			ClickDown(int inX, int inY, int inButton)=0;
	virtual	void			ClickUp(int inX, int inY, int inButton)=0;
	virtual	void			ClickDrag(int inX, int inY, int inButton)=0;	// 0 = left
	virtual	void			ClickMove(int inX, int inY)=0;	// 0 = left
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis)=0;	// 0 = up-down
	virtual	void			DragEnter(int inX, int inY)=0;
	virtual	void			DragOver(int inX, int inY)=0;
	virtual	void			DragLeave(void)=0;
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int inX, int inY)=0;
	virtual	int				KeyPressed(uint32_t inKey, long msg, long p1, long p2)=0;
	virtual	int				HandleMenuCmd(xmenu inMenu, int inCommand)=0;

			xmenu			GetMenuBar(void);
			void			DrawMenuBar(void);

	// Menu API
	static	xmenu			CreateMenu(xmenu parent, int item, const char * inTitle);
	static	int				AppendMenuItem(xmenu menu, const char * inTitle);
	static	int				AppendSeparator(xmenu menu);
	static	void			CheckMenuItem(xmenu menu, int item, bool inCheck);
	static	void			EnableMenuItem(xmenu menu, int item, bool inEnable);


#if APL

protected:

		XWinCocoa *				mWindow;
private:
#if defined(__OBJC__)
		NSTimer *				mTimer;
#else
		void *					mTimer;
#endif

public:
		int						mDefaultDND;		// If true, we dispatch the simple file-based D&D.  Otherwise we use the Mac-specific
													// advanced callbacks.

		int						mInInit;			// Init protection flag to keep obj-c from calling virtual functions from ctor
		int						mInPopup;			// This flag tells us we are in a popup menu, so we stash menu picks instead of dispatching them.
		int						mPopupPick;			// Local storage from popup callback handler for what we picked


		int						mInDrag;			// Button being dragged or -1 if none.  This ensures we send only one down/drag/up sequence.
		int						mInMouseHandler;	// Detector for re-entrancy in mouse handlers
		int						mWantFakeUp;		// True if the down or drag handler should fake an up click and end the gesture.
		int						mIsControlClick;	// Flag if control key down on the mInDrag down click - control + button 0 = button 1
		int						mLastX;				// Last known mouse event for when we have to fake an event.
		int						mLastY;

		void *					mToolTipMem;		// Tool tips - a retained NSString current tool tip casted to void.
		int						mToolTipLive;		// True if the tip is up.
		int						mToolTipBounds[4];	// If the tip is up, we kill the tip when it goes out of these bounds so we can recompute it.

		int						mCurrentDragOps;	// Legal drag mask (in NS constants) for current drag op)
		int						mInDragOp;			// Flag if we are in a drag and drop op - since the op is closed via a callback we have to keep a flag.
													// 0 = we are NOT in the drag op.
													// 1 = we are in the modal loop and the drag is happening
													// 2 = we are in the modal loop but the drag is over.  This tells us that we SHOULD
													//	   exit the loop but we haven't YET exited the loop.  See endedAtPoint in .mm

		// Mac common handlers for obj-C's 5 million event messages.
		void					EventButtonDown(int x, int y, int button, int has_control_key);
		void					EventButtonUp  (int x, int y, int button					 );
		void					EventButtonMove(int x, int y								 );
		void					ManageToolTipForMouse(int x, int y);

	// This is sent to the sub-class to get our help tip.  We need to do this in the Mac because the
	// tool tip call comes from obj-C.
	virtual int					CalcHelpTip(int x, int y, int bounds[4], string& msg) { return 0; }

	// These handlers are sent to the sub-class on Mac only if we are NOT doing default DND support so
	// that GUI_window can implement its own drag handling.   The passed in value is an NSDraggingInfo
	// casted to a generic ptr.
	virtual	int					AdvancedDragEntered(void * ns_dragging_info) { return 0; }
	virtual	int					AdvancedDragUpdated(void * ns_dragging_info) { return 0; }
	virtual	void				AdvancedDragExited (void * ns_dragging_info) {			 }
	virtual	int					AdvancedPerformDrop(void * ns_dragging_info) { return 0; }

	virtual	void				GotCommandHack(int command) { }

protected:
		void		initCommon(int dnd, const char * title, int attributes, int x, int y, int dx, int dy);

		// Utility to initiate a DnD - provided down here because we need to be in ObjC with access to NS
		// stuf to do a DnD.
		void		DoMacDragAndDrop(int item_count,
								  void *	items[],			// Memory for NSDragItem is released here!
								  int		mac_drag_types);

#endif
#if IBM

protected:


		HWND			mWindow;
		CDropTarget *	mDropTarget;
		POINT			mMouse;
		POINT			mSizeMin;
		int				mDragging;
		int				mWantFakeUp;
		int				mIsModal;

		static LRESULT CALLBACK WinEventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	static	void	RegisterClass(HINSTANCE hInstance);
	virtual	void	ReceiveFilesFromDrag(
						const vector<string>& inFiles);

#endif

#if LIN

protected:

	int		mDragging; // Button being dragged or -1 if none ;
	int		mWantFakeUp;
	int		mBlockEvents;
	int		mTimer;
	POINT	mMouse;


public:
	Fl_Menu_Bar * mBar;
	const Fl_Menu_Item * lastItem;
	int GetMenuBarHeight(void);
	virtual void ReceiveFilesFromDrag(const string& inFiles);
	bool mInited;

protected:

	void draw();
	int handle(int e);
	void resize(int x,int y,int w,int h);
	static void window_cb(Fl_Widget *widget, void * data);
	static void menu_cb(Fl_Widget *w, void * data);
//	void closeEvent(QCloseEvent* e);
//	void resizeEvent(QResizeEvent* e);
//	void mousePressEvent(QMouseEvent* e);
//	void mouseReleaseEvent(QMouseEvent* e);
//	void mouseMoveEvent(QMouseEvent* e);
//	void wheelEvent(QWheelEvent* e);
//	void keyPressEvent(QKeyEvent* e);
//	void keyReleaseEvent(QKeyEvent* e);
//	void dragEnterEvent(QDragEnterEvent* e);
//	void dragLeaveEvent(QDragLeaveEvent* e);
//	void dragMoveEvent(QDragMoveEvent* e);
//	void dropEvent(QDropEvent* e);
//	void focusInEvent(QFocusEvent* e);
//	void focusOutEvent(QFocusEvent* e);
//	void timerEvent(QTimerEvent *e);

#endif

};

#endif
