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

#include "GUI_Window.h"
#include "PlatformUtils.h"
#include "GUI_Application.h"
#include "AssertUtils.h"
#include "GUI_Clipboard.h"

#if IBM
#include "GUI_Unicode.h"
#endif
#include "ObjCUtils.h"

#if APL
#define __DEBUGGING__
#include <Carbon/Carbon.h>		// we use this for vkeys/low mem accessors to keyboard
#endif

static set<GUI_Window *>	sWindows;

#if APL
inline int Client2OGL_X(int x, void* w) { return x; }
inline int Client2OGL_Y(int y, void* w) { int h = get_ns_window_height(w); return h-y; }
inline int OGL2Client_X(int x, void* w) { return x; }
inline int OGL2Client_Y(int y, void* w) { int h = get_ns_window_height(w); return h-y; }
#endif

#if IBM
inline int Client2OGL_X(int x, HWND w) { return x; }
inline int Client2OGL_Y(int y, HWND w) { RECT r; GetClientRect(w,&r); return r.bottom-y; }
inline int OGL2Client_X(int x, HWND w) { return x; }
inline int OGL2Client_Y(int y, HWND w) { RECT c; GetClientRect(w,&c); return c.bottom-y; }

#if MINGW_BUILD
#define _TRUNCATE 0

static int strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count)
{
	strncpy(strDest, strSource, strlen(strSource));
	return 0;
}

#endif /* MINGW_BUILD */
#endif /* IBM */

#if LIN

#define mWindow 0

inline int GUI_Window::Client2OGL_X(int x, void* w) { return x; }
inline int GUI_Window::Client2OGL_Y(int y, void* w) { return (this->size().height() - y ); }
inline int GUI_Window::OGL2Client_X(int x, void* w) { return x; }
inline int GUI_Window::OGL2Client_Y(int y, void* w) { return (this->size().height() - y ); }

//---------------------------------------------------------------------------------------------------------------------------------------
// LIN DND
//---------------------------------------------------------------------------------------------------------------------------------------


//TODO:mroe we are shipping no data yet !
//			providing calls in DragData seems sufficient for WED

void GUI_Window::dragEnterEvent(QDragEnterEvent* e)
{
	int x = OGL2Client_X(e->pos().x(),mWindow);
	int y = OGL2Client_Y(e->pos().y(),mWindow);

	GUI_DragData_Adapter  adapter(NULL);
	GUI_DragOperation allowed;
	allowed = (this->InternalDragEnter(x,y,&adapter,
				OP_LIN2GUI(e->possibleActions()),
				OP_LIN2GUI(e->proposedAction())));

	this->mInDrag = 1;
	this->SetTimerInterval(0.05);
	this->mLastDragX = x;
	this->mLastDragY = y;

	if (allowed == gui_Drag_None)
		e->setDropAction(Qt::IgnoreAction);
	//FIXME:mroe:if we comein from outside , drop is not allowed from pane
	//untill the targetrect riched , anyhow we must allow the drag here .
	e->acceptProposedAction();
}

void GUI_Window::dragMoveEvent(QDragMoveEvent* e)
{
	int x = OGL2Client_X(e->pos().x(),mWindow);
	int y = OGL2Client_Y(e->pos().y(),mWindow);

	GUI_DragData_Adapter  adapter(NULL);
	GUI_DragOperation allowed;
	allowed = (this->InternalDragOver(x,y,&adapter,
				OP_LIN2GUI(e->possibleActions()),
				OP_LIN2GUI(e->proposedAction())));

	this->mLastDragX = x;
	this->mLastDragY = y;

	if (allowed == gui_Drag_None)
		e->setDropAction(Qt::IgnoreAction);
	else
		e->acceptProposedAction();
}

void GUI_Window::dragLeaveEvent(QDragLeaveEvent* e)
{
	this->mInDrag = 0;
	this->SetTimerInterval(0);
	this->InternalDragLeave();
}

void GUI_Window::dropEvent(QDropEvent* e)
{
	int x = OGL2Client_X(e->pos().x(),mWindow);
	int y = OGL2Client_Y(e->pos().y(),mWindow);

	this->mInDrag = 0;
	this->SetTimerInterval(0);

	GUI_DragData_Adapter  adapter(NULL);
	GUI_DragOperation allowed;
	allowed = (this->InternalDrop(x,y,&adapter,
				OP_LIN2GUI(e->possibleActions()),
				OP_LIN2GUI(e->proposedAction())));

	this->InternalDragLeave();
}
#endif

//---------------------------------------------------------------------------------------------------------------------------------------
// WINDOWS DND
//---------------------------------------------------------------------------------------------------------------------------------------

#if IBM

#define OleStdGetDropEffect(grfKeyState)    \
    ( (grfKeyState & MK_CONTROL) ?          \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ) :  \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 ) )


// GUI_Window_DND is an implementation of the COM IDropTarget interface that passes drop requests through to the window's base class
// (GUI_Pane).  It handles coordinate conversion, but uses a helper class (GUI_OLE_Adapter) to convert data from the system's native
// COM interfaces to something we can understand.

class GUI_Window_DND : public IDropTarget {
public:
    GUI_Window_DND(GUI_Pane * iTarget, HWND inWindow);
   ~GUI_Window_DND();

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID, LPVOID*);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

   // IDropTarget methods
   STDMETHOD(DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
   STDMETHOD(DragOver)(DWORD, POINTL, LPDWORD);
   STDMETHOD(DragLeave)(void);
   STDMETHOD(Drop)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);

private:
	ULONG			mRefCount;
	GUI_Pane *		mTarget;
	HWND			mWindow;
	IDataObject *	mData;

};

GUI_Window_DND::GUI_Window_DND(GUI_Pane * inTarget, HWND inWindow) :
	mRefCount(1), mWindow(inWindow), mTarget(inTarget)
{
	RegisterDragDrop(inWindow, this);
}

GUI_Window_DND::~GUI_Window_DND()
{
}

STDMETHODIMP GUI_Window_DND::QueryInterface(REFIID riid, LPVOID* ppvOut)
{
	*ppvOut = NULL;

		 if(IsEqualIID(riid, IID_IUnknown))		*ppvOut = this;
	else if(IsEqualIID(riid, IID_IDropTarget))	*ppvOut = (IDropTarget*)this;

	if(*ppvOut)
	{
		(*(LPUNKNOWN*)ppvOut)->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) GUI_Window_DND::AddRef(void)
{
	return ++mRefCount;
}

STDMETHODIMP_(ULONG) GUI_Window_DND::Release(void)
{
	if (--mRefCount == 0)
	{
		delete this;
		return 0;
	}
	return mRefCount;
}

STDMETHODIMP GUI_Window_DND::DragEnter(LPDATAOBJECT data_obj, DWORD key_state, POINTL where, LPDWORD effect)
{
	mData = data_obj;
	mData->AddRef();

	GUI_OLE_Adapter	adapter(mData);

	DWORD allowed = *effect;
	*effect = DROPEFFECT_NONE;

	DWORD recommended = OleStdGetDropEffect(key_state);

	POINT p;
	p.x = where.x;
	p.y = where.y;
	ScreenToClient(mWindow, &p);

	*effect = OP_GUI2Win(mTarget->InternalDragEnter(
				Client2OGL_X(p.x, mWindow),
				Client2OGL_Y(p.y, mWindow),
				&adapter, OP_Win2GUI(allowed), OP_Win2GUI(recommended)));
	mTarget->InternalDragScroll(
				Client2OGL_X(p.x, mWindow),
				Client2OGL_Y(p.y, mWindow));
	return S_OK;
}

STDMETHODIMP GUI_Window_DND::DragOver(DWORD key_state, POINTL where, LPDWORD effect)
{
	GUI_OLE_Adapter	adapter(mData);

	DWORD allowed = *effect;
	*effect = DROPEFFECT_NONE;
	DWORD recommended = OleStdGetDropEffect(key_state);

	POINT p;
	p.x = where.x;
	p.y = where.y;
	ScreenToClient(mWindow, &p);

	*effect = OP_GUI2Win(mTarget->InternalDragOver(
			Client2OGL_X(p.x, mWindow),
			Client2OGL_Y(p.y, mWindow),
			&adapter, OP_Win2GUI(allowed),OP_Win2GUI(recommended)));
	 mTarget->InternalDragScroll(
			Client2OGL_X(p.x, mWindow),
			Client2OGL_Y(p.y, mWindow));
	return S_OK;
}

STDMETHODIMP GUI_Window_DND::DragLeave(void)
{

	mTarget->InternalDragLeave();
	mData->Release();
	return S_OK;
}

STDMETHODIMP GUI_Window_DND::Drop(LPDATAOBJECT data_obj, DWORD key_state, POINTL where, LPDWORD effect)
{
	GUI_OLE_Adapter	adapter(data_obj);

	DWORD allowed = *effect;
	*effect = DROPEFFECT_NONE;
	DWORD recommended = OleStdGetDropEffect(key_state);

	POINT p;
	p.x = where.x;
	p.y = where.y;
	ScreenToClient(mWindow, &p);

	*effect = OP_GUI2Win(mTarget->InternalDrop(
			Client2OGL_X(p.x, mWindow),
			Client2OGL_Y(p.y, mWindow),
			&adapter, OP_Win2GUI(allowed),OP_Win2GUI(recommended)));
	mTarget->InternalDragLeave();

	return S_OK;

}

//	From Raymond Chen's blog:
//	http://blogs.msdn.com/oldnewthing/archive/2004/12/06/275659.aspx

class GUI_DropSource : public IDropSource {
public:
	STDMETHODIMP		 QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHODIMP GiveFeedback(DWORD dwEffect);

	GUI_DropSource() : m_cRef(1) { }
private:
	ULONG m_cRef;
};

HRESULT GUI_DropSource::QueryInterface(REFIID riid, void **ppv)
{
	IUnknown *punk = NULL;
			if (riid == IID_IUnknown)		punk = static_cast<IUnknown*>(this);
	else	if (riid == IID_IDropSource)	punk = static_cast<IDropSource*>(this);

	*ppv = punk;
	if (punk)
	{
		punk->AddRef();
		return S_OK;
	} else
	return E_NOINTERFACE;
}

ULONG GUI_DropSource::AddRef()
{
	return ++m_cRef;
}

ULONG GUI_DropSource::Release()
{
	ULONG cRef = --m_cRef;
	if (cRef == 0) delete this;
	return cRef;
}

HRESULT GUI_DropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed)								return DRAGDROP_S_CANCEL;
	if (!(grfKeyState & (MK_LBUTTON | MK_RBUTTON)))	return DRAGDROP_S_DROP;
													return S_OK;
}

HRESULT GUI_DropSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}


#endif

//---------------------------------------------------------------------------------------------------------------------------------------
// MAC DND
//---------------------------------------------------------------------------------------------------------------------------------------

#if APL

int		GUI_Window::AdvancedDragEntered(void * ns_dragging_info)
{
	GUI_DragMgr_Adapter adapter(ns_dragging_info);
	int x, y;
	GUI_DragOperation allowed = OP_Mac2GUI(get_drag_operations(ns_dragging_info));
	GUI_DragOperation recommended = allowed;
	get_drag_location(ns_dragging_info, &x, &y);

	if ((recommended & (gui_Drag_Move | gui_Drag_Copy)) == (gui_Drag_Move | gui_Drag_Copy))
	{
		if(drag_has_option_key(ns_dragging_info))
			recommended = gui_Drag_Copy;
	}
	
	allowed = InternalDragEnter(x, y, &adapter, allowed, recommended);
	mInDrag = 1;
	SetTimerInterval(0.05);
	mLastDragX = x;
	mLastDragY = y;
	
	ForceRefresh();
	
	return OP_GUI2Mac(allowed);
}
	
int		GUI_Window::AdvancedDragUpdated(void * ns_dragging_info)
{
	GUI_DragMgr_Adapter adapter(ns_dragging_info);
	int x, y;
	GUI_DragOperation allowed = OP_Mac2GUI(get_drag_operations(ns_dragging_info));
	GUI_DragOperation recommended = allowed;
	get_drag_location(ns_dragging_info, &x, &y);

	if ((recommended & (gui_Drag_Move | gui_Drag_Copy)) == (gui_Drag_Move | gui_Drag_Copy))
	{
		if(drag_has_option_key(ns_dragging_info))
			recommended = gui_Drag_Copy;
	}
	
	allowed = InternalDragOver(x, y, &adapter, allowed, recommended);
	mLastDragX = x;
	mLastDragY = y;
	
	ForceRefresh();
	
	return OP_GUI2Mac(allowed);
}

void	GUI_Window::AdvancedDragExited(void * ns_dragging_info)
{
	mInDrag = 0;
	SetTimerInterval(0.0);
	InternalDragLeave();
	ForceRefresh();
}

int		GUI_Window::AdvancedPerformDrop(void * ns_dragging_info)
{
	GUI_DragMgr_Adapter adapter(ns_dragging_info);
	int x, y;
	GUI_DragOperation allowed = OP_Mac2GUI(get_drag_operations(ns_dragging_info));
	GUI_DragOperation recommended = allowed;
	get_drag_location(ns_dragging_info, &x, &y);

	if ((recommended & (gui_Drag_Move | gui_Drag_Copy)) == (gui_Drag_Move | gui_Drag_Copy))
	{
		if(drag_has_option_key(ns_dragging_info))
			recommended = gui_Drag_Copy;
	}
	
	allowed = InternalDrop(x, y, &adapter, allowed, recommended);
	mLastDragX = x;
	mLastDragY = y;
	
	mInDrag = 0;
	SetTimerInterval(0.0);
	InternalDragLeave();
	ForceRefresh();
	
	return OP_GUI2Mac(allowed);
}
#endif


//---------------------------------------------------------------------------------------------------------------------------------------
// COMMON CODE
//---------------------------------------------------------------------------------------------------------------------------------------

#if IBM
void CopyMenusRecursive(HMENU src, HMENU dst)
{
	char buf[1024];
	int num_items = ::GetMenuItemCount(src);
	for (int i = 0; i < num_items; ++i)
	{
		MENUITEMINFOA mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_STATE | MIIM_ID;
		mif.cch = sizeof(buf);
		mif.dwTypeData = buf;
		GetMenuItemInfoA(src, i, true, &mif);
		HMENU orig_submenu = mif.hSubMenu;
		if (mif.hSubMenu != NULL)
			mif.hSubMenu = CreateMenu();
		InsertMenuItemA(dst,-1,true,&mif);
		if (mif.hSubMenu)
			CopyMenusRecursive(orig_submenu, mif.hSubMenu);
	}
}
#endif

GUI_Window::GUI_Window(const char * inTitle, int inAttributes, const int inBounds[4], GUI_Commander * inCommander) : GUI_Commander(inCommander),
	XWinGL(0, inTitle, inAttributes, inBounds[0], inBounds[1], inBounds[2]-inBounds[0], inBounds[3]-inBounds[1], sWindows.empty() ? NULL : *sWindows.begin())
{
	mInDrag = 0;
	#if IBM
		mDND = new GUI_Window_DND(this, mWindow);
		mBaseProc = (WNDPROC) GetWindowLongPtr(mWindow,GWLP_WNDPROC);
		SetWindowLongPtrW(mWindow,GWLP_USERDATA,(LONG_PTR)this);
		SetWindowLongPtrW(mWindow,GWLP_WNDPROC,(LONG_PTR)SubclassFunc);

		if (!sWindows.empty())
		{
			HMENU new_mbar = ::CreateMenu();
			::SetMenu(mWindow,new_mbar);
			CopyMenusRecursive(::GetMenu((*sWindows.begin())->mWindow),new_mbar);
			::DrawMenuBar(mWindow);
		}

		mToolTip = CreateWindowEx(
					WS_EX_TOPMOST,
					TOOLTIPS_CLASS,
					NULL,
					WS_POPUP | TTS_NOPREFIX,
					CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT,
					mWindow,
					NULL,
					NULL,
					NULL);
		SetWindowPos(mToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		RECT	cl;
		GetClientRect(mWindow,&cl);
		TOOLINFOW ti;
		ti.cbSize = sizeof(ti) - 4;
		ti.uFlags = 0;	//TTF_SUBCLASS;
		ti.hwnd = mWindow;
		ti.uId = 1;
		ti.lpszText = LPSTR_TEXTCALLBACKW;
		ti.rect.bottom = cl.bottom;
		ti.rect.top = cl.top;
		ti.rect.left = cl.left;
		ti.rect.right = cl.right;

		SendMessage(mToolTip, TTM_ADDTOOL, 0, (LPARAM) &ti);
	#endif
	#if LIN
		this->setMenuBar(gApplication->getqmenu());
		this->Resize(inBounds[2]-inBounds[0],inBounds[3]-inBounds[1]);
		QApplication::setActiveWindow(this);
		setFocusPolicy(Qt::StrongFocus);
		setAcceptDrops(true);
		setFocus();
	#endif
	sWindows.insert(this);
	mBounds[0] = 0;
	mBounds[1] = 0;
	XWinGL::GetBounds(mBounds+2,mBounds+3);
	memset(mMouseFocusPane,0,sizeof(mMouseFocusPane));
	mVisible = inAttributes & xwin_style_visible;
	mClearColorRGBA[0] = 1.0;
	mClearColorRGBA[1] = 1.0;
	mClearColorRGBA[2] = 1.0;
	mClearColorRGBA[3] = 1.0;
	mClearDepth = false;
	mClearColor = true;
	mDesc = inTitle;
	mState.Init();

	// BEN SEZ: this is probably a bad idea...
	FocusChain(1);
}

void	GUI_Window::SetClearSpecs(bool inDoClearColor, bool inDoClearDepth, float inClearColor[4])
{
	mClearColorRGBA[0] = inClearColor[0];
	mClearColorRGBA[1] = inClearColor[1];
	mClearColorRGBA[2] = inClearColor[2];
	mClearColorRGBA[3] = inClearColor[3];
	mClearDepth = inDoClearDepth;
	mClearColor = inDoClearColor;
}



GUI_Window::~GUI_Window()
{
	#if IBM
//		SetWindowLongPtrW(mWindow,GWLP_WNDPROC,(LONG_PTR) mBaseProc);
		mDND->Release();
	#endif
	sWindows.erase(this);
}

void			GUI_Window::ClickDown(int inX, int inY, int inButton)
{
	DebugAssert(mMouseFocusPane[inButton] == NULL);
	this->GetRootForCommander()->BeginDefer();
	mMouseFocusPane[inButton] = InternalMouseDown(Client2OGL_X(inX, mWindow), Client2OGL_Y(inY, mWindow), inButton);

//		Ben says - we should not need to poll on mouse clickig...turn off for now
//					until we find out what the hell needed this!
//		(This was probably to do auto-scroll on tracking handlers without having to wiggle the mouse.  Do we
//		have ANY cases in WED where we can see this?!
//	if (mMouseFocusPane)
//		SetTimerInterval(0.1);
}

void			GUI_Window::ClickUp(int inX, int inY, int inButton)
{
	// Ben says: note that if we don't have a mouse focus paine we just "eat" the up-click.
	// This deals with a Win32 design problem: Windows D&D _eats_ the up-click events.  So we
	// post synthetic ones later.
	// But this is NOT 100% correct - on windows we can quit a D&D event with the escape key - in
	// this case the mouse up hasn't happened, so it isn't eaten.  When we hit escape we post a
	// synthetic mouse up (which is probably good) and then the real one later hits.  But it no-ops
	// because there is no focus pain.
	if (mMouseFocusPane[inButton])
	{
		mMouseFocusPane[inButton]->MouseUp(Client2OGL_X(inX, mWindow), Client2OGL_Y(inY, mWindow), inButton);
		SetTimerInterval(0.0);
	}
	mMouseFocusPane[inButton] = NULL;
	this->GetRootForCommander()->EndDefer();
	
}

void			GUI_Window::ClickDrag(int inX, int inY, int inButton)
{
	if (mMouseFocusPane[inButton])
		mMouseFocusPane[inButton]->MouseDrag(Client2OGL_X(inX, mWindow), Client2OGL_Y(inY, mWindow), inButton);
}

void		GUI_Window::ClickMove(int inX, int inY)
{
	this->InternalMouseMove(Client2OGL_X(inX, mWindow), Client2OGL_Y(inY, mWindow));
	#if APL
		// Windows handles this separately...to avoid thrash with WM_SETCURSOR
		int cursor = this->InternalGetCursor(Client2OGL_X(inX, mWindow), Client2OGL_Y(inY, mWindow));
		switch(cursor) {
		case gui_Cursor_Resize_H:	set_left_right_cursor();	break;
		case gui_Cursor_Resize_V:	set_up_down_cursor();		break;
		case gui_Cursor_None:
		case gui_Cursor_Arrow:
		default:					set_arrow_cursor();	break;
		}
	#endif
	#if LIN
		int cursor = this->InternalGetCursor(Client2OGL_X(inX, mWindow), Client2OGL_Y(inY, mWindow));
		switch(cursor) {
		case gui_Cursor_Resize_H:	this->setCursor(Qt::SizeHorCursor);	break;
		case gui_Cursor_Resize_V:	this->setCursor(Qt::SizeVerCursor);	break;
		case gui_Cursor_None:
		case gui_Cursor_Arrow:
		default:					this->setCursor(Qt::ArrowCursor);	break;
		}
	#endif
}

void			GUI_Window::MouseWheel(int inX, int inY, int inDelta, int inAxis)
{
	InternalMouseWheel(Client2OGL_X(inX, mWindow), Client2OGL_Y(inY, mWindow), inDelta, inAxis);
}

void			GUI_Window::GLReshaped(int inWidth, int inHeight)
{
#if IBM
		RECT cl;
		TOOLINFOW ti;
					GetClientRect(mWindow, &cl);
					ti.cbSize = sizeof(ti) - 4;
					ti.hwnd = mWindow;
					ti.uId = 1;
					ti.rect.bottom = cl.bottom;
					ti.rect.top= cl.top;
					ti.rect.left = cl.left;
					ti.rect.right= cl.right;
					SendMessage(mToolTip, TTM_NEWTOOLRECT, 0, (LPARAM) &ti);

	// Windoze: avoid shrinking and changing side-bar position when minimized.
	// If there's a better test for this we should change it!
	if(inWidth == 0 && inHeight == 0)
		return;

#endif

	int oldBounds[4] = { mBounds[0], mBounds[1], mBounds[2], mBounds[3] };

	mBounds[0] = 0;
	mBounds[1] = 0;
	mBounds[2] = inWidth;
	mBounds[3] = inHeight;

	if (oldBounds[0] != mBounds[0] ||
		oldBounds[1] != mBounds[1] ||
		oldBounds[2] != mBounds[2] ||
		oldBounds[3] != mBounds[3])
	{
		for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
			(*c)->ParentResized(oldBounds, mBounds);
#if !LIN
			Refresh();
#endif
	}
}

void			GUI_Window::GLDraw(void)
{
	SetGLContext();

	int	w, h;
	XWinGL::GetBounds(&w, &h);
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (mClearColor)
		glClearColor(mClearColorRGBA[0],mClearColorRGBA[1],mClearColorRGBA[2],mClearColorRGBA[3]);
	glClear((mClearColor ? GL_COLOR_BUFFER_BIT : 0) + (mClearDepth ? GL_DEPTH_BUFFER_BIT : 0));
	mState.Reset();
	glEnable(GL_SCISSOR_TEST);
	InternalDraw(&mState);
	glDisable(GL_SCISSOR_TEST);
}

void		GUI_Window::Refresh(void)
{
	ForceRefresh();
}

void	GUI_Window::Show(void)
{
	mVisible = true;
	SetVisible(true);
}

void GUI_Window::Hide(void)
{
	mVisible = false;
	SetVisible(false);
}

void	GUI_Window::SetBounds(int inBounds[4])
{
	int oldBounds[4] = { mBounds[0], mBounds[1], mBounds[2], mBounds[3] };

	XWinGL::MoveTo(inBounds[0], inBounds[1]);
	XWinGL::Resize(inBounds[2]-inBounds[0], inBounds[3]-inBounds[1]);
}

void		GUI_Window::SetBounds(int x1, int y1, int x2, int y2)
{
	int b[4] = { x1, y1, x2, y2 };
	SetBounds(b);
}

void		GUI_Window::SetDescriptor(const string& inDesc)
{
	mDesc = inDesc;
	XWinGL::SetTitle(inDesc.c_str());
}

bool		GUI_Window::IsActiveNow(void) const
{
	return XWin::GetActive();
}

bool		GUI_Window::IsVisibleNow(void) const
{
	return XWin::GetVisible();
}

#if APL

const char	gui_Key_Map [256] = {
/*			00					01					02					03					04					05					06					07*/
/* 00 */	GUI_VK_A,			GUI_VK_S,			GUI_VK_D,			GUI_VK_F,			GUI_VK_H,			GUI_VK_G,			GUI_VK_Z,			GUI_VK_X,
/* 08 */	GUI_VK_C,			GUI_VK_V,			0,					GUI_VK_B,			GUI_VK_Q,			GUI_VK_W,			GUI_VK_E,			GUI_VK_R,
/* 10 */	GUI_VK_Y,			GUI_VK_T,			GUI_VK_1,			GUI_VK_2,			GUI_VK_3,			GUI_VK_4,			GUI_VK_6,			GUI_VK_5,
/* 18 */	GUI_VK_EQUAL,		GUI_VK_9,			GUI_VK_7,			GUI_VK_MINUS,		GUI_VK_8,			GUI_VK_0,			GUI_VK_RBRACE,		GUI_VK_O,
/* 20 */	GUI_VK_U,			GUI_VK_LBRACE,		GUI_VK_I,			GUI_VK_P,			GUI_VK_RETURN,		GUI_VK_L,			GUI_VK_J,			GUI_VK_QUOTE,
/* 28 */	GUI_VK_K,			GUI_VK_SEMICOLON,	GUI_VK_BACKSLASH,	GUI_VK_COMMA,		GUI_VK_SLASH,		GUI_VK_N,			GUI_VK_M,			GUI_VK_PERIOD,
/* 30 */	GUI_VK_TAB,			GUI_VK_SPACE,		GUI_VK_BACKQUOTE,	GUI_VK_BACK,		GUI_VK_ENTER,		GUI_VK_ESCAPE,		0,					0,
/* 38 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 40 */	0,					GUI_VK_DECIMAL,		0,					GUI_VK_MULTIPLY,	0,					GUI_VK_ADD,			0,					GUI_VK_CLEAR,
/* 48 */	0,					0,					0,					GUI_VK_DIVIDE,		GUI_VK_NUMPAD_ENT,	0,					GUI_VK_SUBTRACT,	0,
/* 50 */	0,					GUI_VK_NUMPAD_EQ,	GUI_VK_NUMPAD0,		GUI_VK_NUMPAD1,		GUI_VK_NUMPAD2,		GUI_VK_NUMPAD3,		GUI_VK_NUMPAD4,		GUI_VK_NUMPAD5,
/* 58 */	GUI_VK_NUMPAD6,		GUI_VK_NUMPAD7,		0,					GUI_VK_NUMPAD8,		GUI_VK_NUMPAD9,		0,					0,					0,
/* 60 */	GUI_VK_F5,			GUI_VK_F6,			GUI_VK_F7,			GUI_VK_F3,			GUI_VK_F8,			GUI_VK_F9,			0,					GUI_VK_F11	,
/* 68 */	0,					0,					0,					0,					0,					GUI_VK_F10,			0,					GUI_VK_F12,
/* 70 */	0,					0,					0,					GUI_VK_HOME,		GUI_VK_PRIOR,		GUI_VK_DELETE,		GUI_VK_F4,			GUI_VK_END,
/* 78 */	GUI_VK_F2,			GUI_VK_NEXT,		GUI_VK_F1,			GUI_VK_LEFT,		GUI_VK_RIGHT,		GUI_VK_DOWN,		GUI_VK_UP,			0
};

//#elif IBM
#else

const char	gui_Key_Map [256] = {
/*			00					01					02					03					04					05					06					07*/
/* 00 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 08 */	GUI_VK_BACK,		GUI_VK_TAB,		0,					0,					GUI_VK_CLEAR,		GUI_VK_RETURN,		0,					0,
/* 10 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 18 */	0,					0,					0,					GUI_VK_ESCAPE,		0,					0,					0,					0,
/* 20 */	GUI_VK_SPACE,		GUI_VK_PRIOR,		GUI_VK_NEXT,		GUI_VK_END,		GUI_VK_HOME,		GUI_VK_LEFT,		GUI_VK_UP,			GUI_VK_RIGHT,
/* 28 */	GUI_VK_DOWN,		GUI_VK_SELECT,		GUI_VK_PRINT,		GUI_VK_EXECUTE,	GUI_VK_SNAPSHOT,	GUI_VK_INSERT,		GUI_VK_DELETE,		GUI_VK_HELP,
/* 30 */	GUI_VK_0,			GUI_VK_1,			GUI_VK_2,			GUI_VK_3,			GUI_VK_4,			GUI_VK_5,			GUI_VK_6,			GUI_VK_7,
/* 38 */	GUI_VK_8,			GUI_VK_9,			0,					0,					0,					0,					0,					0,
/* 40 */	0,					GUI_VK_A,			GUI_VK_B,			GUI_VK_C,			GUI_VK_D,			GUI_VK_E,			GUI_VK_F,			GUI_VK_G,
/* 48 */	GUI_VK_H,			GUI_VK_I,			GUI_VK_J,			GUI_VK_K,			GUI_VK_L,			GUI_VK_M,			GUI_VK_N,			GUI_VK_O,
/* 50 */	GUI_VK_P,			GUI_VK_Q,			GUI_VK_R,			GUI_VK_S,			GUI_VK_T,			GUI_VK_U,			GUI_VK_V,			GUI_VK_W,
/* 58 */	GUI_VK_X,			GUI_VK_Y,			GUI_VK_Z,			0,					0,					0,					0,					0,
/* 60 */	GUI_VK_NUMPAD0,	GUI_VK_NUMPAD1,	GUI_VK_NUMPAD2,	GUI_VK_NUMPAD3,	GUI_VK_NUMPAD4,	GUI_VK_NUMPAD5,	GUI_VK_NUMPAD6,	GUI_VK_NUMPAD7,
/* 68 */	GUI_VK_NUMPAD8,	GUI_VK_NUMPAD9,	GUI_VK_MULTIPLY,	GUI_VK_ADD,		0,					GUI_VK_SUBTRACT,	GUI_VK_DECIMAL,	GUI_VK_DIVIDE,
/* 70 */	GUI_VK_F1,			GUI_VK_F2,			GUI_VK_F3,			GUI_VK_F4,			GUI_VK_F5,			GUI_VK_F6,			GUI_VK_F7,			GUI_VK_F8,
/* 78 */	GUI_VK_F9,			GUI_VK_F10,		GUI_VK_F11,		GUI_VK_F12,		GUI_VK_F13,		GUI_VK_F14,		GUI_VK_F15,		GUI_VK_F16,
/* 80 */	GUI_VK_F17,		GUI_VK_F18,		GUI_VK_F19,		GUI_VK_F20,		GUI_VK_F21,		GUI_VK_F22,		GUI_VK_F23,		GUI_VK_F24,
/* 88 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 90 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 88 */	0,					0,					0,					0,					0,					0,					0,					0,
/* A0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* A8 */	0,					0,					0,					0,					0,					0,					0,					0,
/* B0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* B8 */	0,					0,					GUI_VK_SEMICOLON,	GUI_VK_EQUAL,		GUI_VK_COMMA,		GUI_VK_MINUS,		GUI_VK_PERIOD,		GUI_VK_SLASH,
/* C0 */	GUI_VK_BACKQUOTE,	0,					0,					0,					0,					0,					0,					0,
/* C8 */	0,					0,					0,					0,					0,					0,					0,					0,
/* D0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* D8 */	0,					0,					0,					GUI_VK_LBRACE,		GUI_VK_BACKSLASH,	GUI_VK_RBRACE,		GUI_VK_QUOTE,		GUI_VK_BACKQUOTE,
/* E0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* E8 */	0,					0,					0,					0,					0,					0,					0,					0,
/* F0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* F8 */	0,					0,					0,					0,					0,					0,					0,					0
};

#endif



int			GUI_Window::KeyPressed(uint32_t inKey, long inMsg, long inParam1, long inParam2)
{
	GUI_KeyFlags		flags = 0;
	uint32_t			charCode = 0;
	uint32_t		virtualCode = 0;

#if IBM
	HKL hKL;
	unsigned int vKey, RetCode, ScanCode;
	unsigned short Char = 0;
	BYTE KeyState[256];
	long shiftKey, controlKey, optionKey, keyDown, keyUp, charCodeMask,	keyCodeMask;
	long ExtKeyMask, ShiftControlMask, scrollLockKey, capsLockKey, numLockKey;
	int ExtendedKey;
#endif

#if APL
	enum {
    NSAlphaShiftKeyMask         = 1 << 16,
    NSShiftKeyMask              = 1 << 17,
    NSControlKeyMask            = 1 << 18,
    NSAlternateKeyMask          = 1 << 19,
    NSCommandKeyMask            = 1 << 20,
    NSNumericPadKeyMask         = 1 << 21,
    NSHelpKeyMask               = 1 << 22,
    NSFunctionKeyMask           = 1 << 23,
    NSKeyDown                   = 10,
    NSKeyUp                     = 11,
	};

	charCode = inParam1 & 0xFF;
	virtualCode = (inParam1 & 0xFF00) >> 8L;
	if (inParam2 & NSShiftKeyMask)
		flags |= gui_ShiftFlag;
	if (inParam2 & NSCommandKeyMask)
		flags |= gui_ControlFlag;
	if (inParam2 & NSAlternateKeyMask)
		flags |= gui_OptionAltFlag;
	if (inMsg == NSKeyDown)
		flags |= gui_DownFlag;
	if (inMsg == NSKeyUp)
		flags |= gui_UpFlag;

	TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardLayoutInputSource();
	CFDataRef uchr_data = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
	const UCKeyboardLayout *uchr = uchr_data ? (const UCKeyboardLayout*)CFDataGetBytePtr(uchr_data) : NULL;
	CFRelease(currentKeyboard);


	int scan_code = ((inParam1 & 0xFF00) >> 8) & 0xFF	;			// The vkey and the scan code are the same on the Mac.
	int os_vkey =   ((inParam1 & 0xFF00) >> 8) & 0xFF	;			// So use for both.  Vkey codes are relatively low numbres.

	if(uchr)
	{
		static UInt32	dead_state[3] = { 0 }, dead_lower = 0, dead_upper = 0;								// Translate 3 separate times - to generate the real key, and hypothetical
		UniCharCount ct=0;																					// shift-down and shift-up keys.
		UniChar buf[2];
		OSStatus result;

		result = UCKeyTranslate(uchr,os_vkey, inMsg - keyDown + kUCKeyActionDown, (inParam2 >> 8) & 0xFF , LMGetKbdType(), 0, &dead_state[inMsg - keyDown], 2, &ct, buf);
		if(result == noErr && ct > 0) charCode = buf[0];

//		result = UCKeyTranslate(uchr, os_vkey, kUCKeyActionDisplay, 0, LMGetKbdType(), kUCKeyTranslateNoDeadKeysMask, &dead_lower, 2, &ct, buf);
//		if (result == noErr && ct > 0) uni_lower = buf[0];

//		result = UCKeyTranslate(uchr, os_vkey, kUCKeyActionDisplay, shiftKey >> 8, LMGetKbdType(), kUCKeyTranslateNoDeadKeysMask, &dead_upper, 2, &ct, buf);
//		if (result == noErr && ct > 0) uni_upper = buf[0];
	}
	else
	{
//		uni_lower = uni_upper = event->message & charCodeMask;												// Last fallback, take message a priori.  Should never get this desparate!
	}


#elif IBM
	numLockKey = 0x01450000;
	capsLockKey =0x003a0000;
	scrollLockKey = 0x00460000;
	shiftKey = 0x002a0000;
	controlKey = 0x001d0000;
	optionKey = 0x00380000;
	keyDown = WM_KEYDOWN;
	keyUp = WM_KEYUP;
	charCodeMask = 0xff;
	keyCodeMask = 0xff;
	ShiftControlMask = 0x00ff0000;
	ExtKeyMask = 0x01ff0000;

#if BENTODO
	examine - is this right?
#endif

	if (((inParam2 & ExtKeyMask) == numLockKey) || ((inParam2 & ShiftControlMask) == capsLockKey) || ((inParam2 & ShiftControlMask) == scrollLockKey))
		return 1;

	if (inParam1 == VK_SHIFT ||
		inParam1 == VK_CONTROL ||
		inParam1 == VK_MENU)
	{
		return 1;
	}

	hKL = GetKeyboardLayout(0);
	ScanCode = ((inParam2 >> 16) & 0xff);
	ExtendedKey =  ((inParam2 >> 24) & 0x01);
	vKey = MapVirtualKeyEx(ScanCode, 1, hKL);
	RetCode = GetKeyboardState((unsigned char*)&KeyState);
	RetCode = ToAsciiEx(vKey, ScanCode, (unsigned char*)&KeyState, &Char, 0, hKL);

	if (RetCode != 0)
	{
		charCode = Char;
		if (ExtendedKey == 0)
			virtualCode = vKey;
		else
			virtualCode = inParam1 & keyCodeMask;
	}
	else
	{
		charCode = inParam1 & charCodeMask;
		virtualCode = inParam1 & keyCodeMask;
	}

	if(::GetKeyState(VK_SHIFT) & ~1) flags |= gui_ShiftFlag;
	if(::GetKeyState(VK_CONTROL) & ~1) flags |= gui_ControlFlag;
	if(::GetKeyState(VK_MENU) & ~1) flags |= gui_OptionAltFlag;

	if (inMsg == keyDown)
		flags |= gui_DownFlag;
	if (inMsg == keyUp)
		flags |= gui_UpFlag;

	// NOTE: the GUI_KEY ASCII defines are all mac-compatible.

	// Finally, control and option keys are not available as ASCII because
	// the ASCII codes are whacko.
	if ( ((inParam2 & ShiftControlMask) == controlKey) || ((inParam2 & ShiftControlMask) == optionKey))
		charCode = 0;
#if 0
	UTF16_decode((const UTF16*)&inKey, charCode);
#endif
#endif
#if !LIN
	virtualCode = gui_Key_Map[virtualCode];
#endif

#if IBM || APL
	switch (virtualCode)
	{
		case GUI_VK_RETURN:	charCode = GUI_KEY_RETURN;	break;
		case GUI_VK_ESCAPE:	charCode = GUI_KEY_ESCAPE;	break;
		case GUI_VK_TAB:	charCode = GUI_KEY_TAB;		break;
		case GUI_VK_BACK:	charCode = GUI_KEY_BACK;	break;
		case GUI_VK_DELETE:	charCode = GUI_KEY_DELETE;	break;
		case GUI_VK_LEFT:	charCode = GUI_KEY_LEFT;	break;
		case GUI_VK_UP:		charCode = GUI_KEY_UP;		break;
		case GUI_VK_RIGHT:	charCode = GUI_KEY_RIGHT;	break;
		case GUI_VK_DOWN:	charCode = GUI_KEY_DOWN;	break;
		case GUI_VK_NUMPAD0:	charCode = GUI_KEY_0;		break;
		case GUI_VK_NUMPAD1:	charCode = GUI_KEY_1;		break;
		case GUI_VK_NUMPAD2:	charCode = GUI_KEY_2;		break;
		case GUI_VK_NUMPAD3:	charCode = GUI_KEY_3;		break;
		case GUI_VK_NUMPAD4:	charCode = GUI_KEY_4;		break;
		case GUI_VK_NUMPAD5:	charCode = GUI_KEY_5;		break;
		case GUI_VK_NUMPAD6:	charCode = GUI_KEY_6;		break;
		case GUI_VK_NUMPAD7:	charCode = GUI_KEY_7;		break;
		case GUI_VK_NUMPAD8:	charCode = GUI_KEY_8;		break;
		case GUI_VK_NUMPAD9:	charCode = GUI_KEY_9;		break;
		case GUI_VK_DECIMAL:	charCode = GUI_KEY_DECIMAL;	break;
	}
#endif
#if LIN
	charCode = inKey;
        Qt::KeyboardModifiers modstate = QApplication::keyboardModifiers();

	if (modstate & Qt::AltModifier||modstate & Qt::MetaModifier)
		flags |= gui_OptionAltFlag;
	if (modstate & Qt::ShiftModifier)
		flags |= gui_ShiftFlag;
	if (modstate & Qt::ControlModifier)
		flags |= gui_ControlFlag;

 	if (!(modstate & Qt::AltModifier))
 		flags |= gui_DownFlag;

	switch (inMsg)
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:	charCode = GUI_KEY_RETURN;	break;
		case Qt::Key_Escape:	charCode = GUI_KEY_ESCAPE;	break;
		case Qt::Key_Tab:
		case Qt::Key_Backtab:	charCode = GUI_KEY_TAB;		break;
		case Qt::Key_Back:	charCode = GUI_KEY_BACK;	break;
		case Qt::Key_Delete:	charCode = GUI_KEY_DELETE;	break;
		case Qt::Key_Left:	charCode = GUI_KEY_LEFT;	break;
		case Qt::Key_Up:	charCode = GUI_KEY_UP;		break;
		case Qt::Key_Right:	charCode = GUI_KEY_RIGHT;	break;
		case Qt::Key_Down:	charCode = GUI_KEY_DOWN;	break;
	}
    // are the same as virtualkey
	if ((0x2F < inMsg) && (inMsg < 0x5b))
	  virtualCode=inMsg;
	else
	{
	  switch(inMsg)
	  {
		case Qt::Key_Enter:
		case Qt::Key_Return:	virtualCode = GUI_VK_RETURN;	break;
		case Qt::Key_Escape:	virtualCode = GUI_VK_ESCAPE;	break;
		case Qt::Key_Tab:
		case Qt::Key_Backtab:	virtualCode = GUI_VK_TAB;	break;						
		case Qt::Key_PageUp:	virtualCode = GUI_VK_PRIOR;	break;
		case Qt::Key_PageDown:	virtualCode = GUI_VK_NEXT;	break;
		case Qt::Key_End:	virtualCode = GUI_VK_END;	break;
		case Qt::Key_Home:	virtualCode = GUI_VK_HOME;	break;
		case Qt::Key_Left :	virtualCode = GUI_VK_LEFT;	break;
		case Qt::Key_Right:	virtualCode = GUI_VK_RIGHT;	break;
		default: virtualCode = 0;
	  }
	}

#endif

	if ((flags == 0) && (charCode == 0) && (virtualCode == 0))
		return 1;
	if (this->DispatchKeyPress(charCode, virtualCode, flags)) return 1;

	return 0;
}

void		GUI_Window::Activate(int active)
{
	string d;
	GetDescriptor(d);
	if (active && !this->IsFocusedChain())
	{
		FocusChain(1);
	}
}

void		GUI_Window::Timer(void)
{
		int	x, y;

	for(int btn=0;btn<BUTTON_DIM;++btn)
	if (mMouseFocusPane[btn])
	{
		XWinGL::GetMouseLoc(&x,&y);
		mMouseFocusPane[btn]->MouseDrag(Client2OGL_X(x, mWindow), Client2OGL_Y(y, mWindow), btn);
	}

	// BEN SAYS: Mac D&D mgr does not call us back during drag if the mouse is still.  So use a timer to tell us
	// that time has passed.  Why here?  Well, we need to force a redraw since update events are blocked.
	// So do this down in the window where we can know these things.  Prevent knowledge of blocked window redraws
	// from getting out into the rest of GUI or worse the whole app.
	if (mInDrag)
	{
		InternalDragScroll(mLastDragX, mLastDragY);
		UpdateNow();
	}
}

void		GUI_Window::GetMouseLocNow(int * out_x, int * out_y)
{
	int x, y;
	XWinGL::GetMouseLoc(&x, &y);
	if (out_x) *out_x = Client2OGL_X(x, mWindow);
	if (out_y) *out_y = Client2OGL_Y(y, mWindow);;
}

void		GUI_Window::PopupMenu(GUI_Menu menu, int x, int y, int button)
{
	TrackPopupCommands((xmenu) menu,OGL2Client_X(x, mWindow),OGL2Client_Y(y,mWindow),button, -1);
}

int		GUI_Window::PopupMenuDynamic(const GUI_MenuItem_t items[], int x, int y, int button, int current)
{
	static GUI_Menu popup_temp = NULL;

	DebugAssert(gApplication);

	if  (popup_temp)  gApplication->RebuildMenu(popup_temp, items);
	else popup_temp = gApplication->CreateMenu("popup temp", items, gApplication->GetPopupContainer(),0);
		
	int ret = TrackPopupCommands((xmenu) popup_temp,OGL2Client_X(x,mWindow), OGL2Client_Y(y,mWindow), button, current);
	return ret;	
}


bool				GUI_Window::IsDragClick(int x, int y, int button)
{
	#if APL
	
		return run_event_tracking_until_move_or_up(button);

	#elif IBM
		POINT p;
		p.x = OGL2Client_X(x,mWindow);
		p.y = OGL2Client_Y(y,mWindow);
		ClientToScreen(mWindow,&p);

		int ret = DragDetect(mWindow,p);
		SetCapture(NULL);
		// Ben says: if we are NOT detecting a drag, it means the user either let go of the mouse or hit
		// escape.  Good enough for me - pretend it's an up-click (if it was, DragDetect ate it.)
		if (!ret)
			PostMessage(mWindow, button ? WM_RBUTTONUP : WM_LBUTTONUP, 0, MAKELONG(OGL2Client_X(x,mWindow),OGL2Client_Y(y,mWindow)));
		else
			mMouseFocusButton = button;
		return ret;

	#else

	bool isdrag = false;

	unsigned int sbtn = 1 << button;
	QPoint startPos(OGL2Client_X(x,mWindow),OGL2Client_Y(y,mWindow));
	
	mBlockEvents = 1;// mroe:this blocks all button events for the GUI in XWin
	while( !isdrag && (QApplication::mouseButtons() & (Qt::MouseButton)sbtn))
	{
		QCoreApplication::processEvents() ;
		QPoint currentPos(mMouse.x,mMouse.y);
		isdrag = ((startPos - currentPos).manhattanLength() >= QApplication::startDragDistance());
	}
	mBlockEvents = 0;
	mWantFakeUp  = 1;

	return isdrag;

	#endif
}

GUI_DragOperation	GUI_Window::DoDragAndDrop(
							int						x,
							int						y,
							int						button,
							int						where[4],
							GUI_DragOperation		operations,
							int						type_count,
							GUI_ClipType			inTypes[],
							int						sizes[],
							const void *			ptrs[],
							GUI_GetData_f			fetch_func,
							void *					ref)
{
	DebugAssert(fetch_func == NULL);
	#if APL
	
		vector<string> types;
		GUI_GetMacNativeDragTypeList(types);
		vector<const char *> raw(types.size());
		for(int i = 0; i < types.size(); ++i)
		{
			raw[i] = types[i].c_str();
		}
	
		register_drag_types_for_window(mWindow, raw.size(), &raw[0]);
	
		void * item = GUI_LoadOneSimpleDrag(type_count, inTypes, sizes, ptrs, where);

		DoMacDragAndDrop(1, &item, OP_GUI2Mac(operations));
		
		return gui_Drag_None;

	#elif IBM
		GUI_DropSource	* drop_source = new GUI_DropSource;
		GUI_SimpleDataObject * data = new GUI_SimpleDataObject(type_count, inTypes, sizes, ptrs, fetch_func, ref);
		DWORD effect;

		if (DoDragDrop(data, drop_source, OP_GUI2Win(operations), &effect) != DRAGDROP_S_DROP)
			effect = DROPEFFECT_NONE;

		data->Release();
		drop_source->Release();

		GUI_DragOperation result = OP_Win2GUI(effect);

		// Repost a fake up click - if we were dragging, the drag probably ate the up-click
		PostMessage(mWindow, mMouseFocusButton ? WM_RBUTTONUP : WM_LBUTTONUP, 0, MAKELONG(OGL2Client_X(x,mWindow),OGL2Client_Y(y,mWindow)));

		return result;
	#else

	// TODO:mroe must create a dataobj class ( a wrapper around Qmimedata maybe) ;
	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;
	//mimeData->setData(mimeType, data);
	drag->setMimeData(mimeData);
	//start the drag
	GUI_DragOperation result = OP_LIN2GUI(drag->start(OP_GUI2LIN(operations)));

	return result;
	#endif
}

#if IBM
HWND GUI_Window::AnyHWND(void)
{
	if (sWindows.empty()) return NULL;
	return (*(sWindows.begin()))->mWindow;
}

LRESULT CALLBACK GUI_Window::SubclassFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GUI_Window * me = (GUI_Window *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
		 message == WM_NCMOUSEMOVE)
	{
		int x = Client2OGL_X(LOWORD(lParam),hWnd);
		int y = Client2OGL_Y(HIWORD(lParam),hWnd);
		if (x < me->mTipBounds[0] || y < me->mTipBounds[1] || x > me->mTipBounds[2] || y > me->mTipBounds[3])
			SendMessage(me->mToolTip,TTM_POP,0,0);

		MSG msg;
		msg.hwnd = hWnd;
		msg.message = message;
		msg.lParam = lParam;
		msg.wParam = wParam;
		SendMessage(me->mToolTip, TTM_RELAYEVENT, 0, (LPARAM) &msg);
	}
	switch(message) {
		case WM_NOTIFY:
			{
				NMHDR * hdr = (NMHDR *) lParam;
				NMTTDISPINFOW * di = (NMTTDISPINFOW *) lParam;
				TOOLINFOW ti;
				string tip;
				int has_tip;
				RECT cl;
				switch(hdr->code) {
				case TTN_GETDISPINFOW:
					has_tip = me->InternalGetHelpTip(
						Client2OGL_X(me->mMouse.x,hWnd),
						Client2OGL_Y(me->mMouse.y,hWnd),
						me->mTipBounds, tip);
					if (tip.empty())
					{
						di->szText[0] = 0;
					}
					else
					{
						string_utf16 clean_str = convert_str_to_utf16(tip);
						if (clean_str.size() > 79)
						{
							clean_str = clean_str.substr(0, 79).replace(76, 3, L"...");
						}

						//Beware! A buffer over run inside of wcscpy will silently crash the program!
						//https://msdn.microsoft.com/en-us/library/windows/desktop/bb760258(v=vs.85).aspx
						wcscpy_s(di->szText, 80, clean_str.c_str());
					}
					return 0;
				default:
					return CallWindowProc(me->mBaseProc, hWnd, message, wParam, lParam);
				}
			}
			return 0;
		case WM_INITMENU:
			EnableMenusWin();
			return 0;
		case WM_DESTROY:
			// Default behavior of xwin is quit-on-close - this stops that.
			return 0;
		case WM_CLOSE:
			delete me;
			return 0;
		case WM_COMMAND:
			if(LOWORD(wParam))
				me->DispatchHandleCommand(LOWORD(wParam));
			return 0;
		case WM_SETCURSOR:
			{
				int x, y;
				me->GetMouseLoc(&x,&y);
				int curs = me->InternalGetCursor(Client2OGL_X(x,me->mWindow),Client2OGL_Y(y,me->mWindow));
				switch(curs) {
				case gui_Cursor_Resize_H:	SetCursor(LoadCursor(NULL,IDC_SIZEWE));	return 0;
				case gui_Cursor_Resize_V:	SetCursor(LoadCursor(NULL,IDC_SIZENS));	return 0;
				}
			}
			return DefWindowProcW(hWnd, message, wParam, lParam);
		default:
			return CallWindowProcW(me->mBaseProc, hWnd, message, wParam, lParam);
	}
}

	struct CmdEval_t {
		string	new_name;
		int		enabled;
		int		checked;
	};
	typedef map<int, CmdEval_t>	CmdMap_t;

static void FindCmdsRecursive(HMENU menu, CmdMap_t& io_map)
{
	int ct = GetMenuItemCount(menu);
	CmdEval_t blank;
	blank.enabled = 0;
	blank.checked = 0;
	for (int n = 0; n < ct; ++n)
	{
		MENUITEMINFO mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.fMask = MIIM_ID | MIIM_SUBMENU;
		GetMenuItemInfo(menu, n, true, &mif);
		if (mif.wID != 0)
		if (io_map.count(mif.wID) == 0)
			io_map.insert(CmdMap_t::value_type(mif.wID,blank));
		if (mif.hSubMenu != NULL)
			FindCmdsRecursive(mif.hSubMenu, io_map);
	}
}

static void ApplyCmdsRecursive(HMENU menu, const CmdMap_t& io_map)
{
	char buf[256];
	int ct = GetMenuItemCount(menu);
	for (int n = 0; n < ct; ++n)
	{
		MENUITEMINFOA mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_TYPE;
		mif.cch = sizeof(buf);
		mif.dwTypeData = buf;
		GetMenuItemInfoA(menu, n, true, &mif);
		string suffix;
		if (mif.fType == MFT_STRING)
		{
			string old_name(buf);
			string::size_type tab = old_name.find('\t');
			if (tab != old_name.npos)
				suffix = old_name.substr(tab);
		}
		if (mif.hSubMenu != NULL)
			ApplyCmdsRecursive(mif.hSubMenu,io_map);
		if (mif.wID != 0)
		{
			CmdMap_t::const_iterator iter = io_map.find(mif.wID);
			if (iter != io_map.end())
			{
				mif.fMask = iter->second.new_name.empty() ? MIIM_STATE : (MIIM_TYPE | MIIM_STATE);
				mif.fType = MFT_STRING;
				string total_name;
				if (!iter->second.new_name.empty())
				{
					total_name = iter->second.new_name + suffix;
					mif.dwTypeData = const_cast<char*>(total_name.c_str());
				}
				mif.fState = (iter->second.enabled ? MFS_ENABLED : MFS_DISABLED) | (iter->second.checked ? MFS_CHECKED : MFS_UNCHECKED);
				SetMenuItemInfoA(menu, n, true, &mif);
			}
		}
	}
}

void GUI_Window::EnableMenusWin(void)
{
	if (sWindows.empty()) return;
	GUI_Window * me = *sWindows.begin();

	CmdMap_t cmds;

	FindCmdsRecursive(GetMenu(me->mWindow),cmds);

	// Why do we need the root commander?  We want to follow
	// the _active_ window's focus chain, now the latent focus
	// chain of the first window.  Maybe we can rewrite this
	// someday to take the window that actually got the WM 
	// message?? 
	for(CmdMap_t::iterator cmd = cmds.begin(); cmd != cmds.end(); ++cmd)
		cmd->second.enabled = me->GetRootForCommander()->DispatchCanHandleCommand(cmd->first,cmd->second.new_name,cmd->second.checked);

	for (set<GUI_Window *>::iterator iter = sWindows.begin(); iter != sWindows.end(); ++iter)
	{
		GUI_Window * who = *iter;
		ApplyCmdsRecursive(GetMenu(who->mWindow),cmds);
	}
}



#endif

#if APL

void				GUI_Window::GotCommandHack(int command)
{
	this->DispatchHandleCommand(command);
}


int			GUI_Window::CalcHelpTip(int x, int y, int bounds[4], string& msg)
{
	x = Client2OGL_X(x, mWindow);
	y = Client2OGL_Y(y, mWindow);
	if (this->InternalGetHelpTip(x, y, bounds, msg))
	{
		bounds[0] = OGL2Client_X(bounds[0], mWindow);
		bounds[1] = OGL2Client_Y(bounds[1], mWindow);
		bounds[2] = OGL2Client_X(bounds[2], mWindow);
		bounds[3] = OGL2Client_Y(bounds[3], mWindow);
		swap(bounds[3], bounds[1]);
		return 1;
	}
	else
		return 0;
}

#endif
