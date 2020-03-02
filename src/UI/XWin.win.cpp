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

#include "XWin.h"
#include "GUI_Unicode.h"

#define 	IDT_TIMER1	0x01

#if MINGW_BUILD
#define GET_XBUTTON_WPARAM(wParam)      (HIWORD(wParam))
#endif

typedef	map<int, pair<xmenu, int> >	MenuMap;
MenuMap	gMenuMap;


static	bool	sIniting = false;

static WCHAR sWindowClass[] = L"XGrinderWindow";

extern	HINSTANCE	gInstance;

map<HWND, XWin *>	sWindows;

XWin::XWin(int default_dnd)
{
	sIniting = true;
	mWindow = CreateWindowW(sWindowClass, L"FullScreen",
		WS_OVERLAPPEDWINDOW,	// Style,
		10, 10, 50, 50,
		NULL,	// Parent
		NULL,	// Menu
		gInstance,	// (app)
		this);	// Misc param

	if (mWindow == NULL)
		throw mWindow;

	sWindows[mWindow] = this;

	if (default_dnd)
	{
		mDropTarget = new CDropTarget;
		RegisterDragDrop(mWindow, mDropTarget);
		mDropTarget->SetReceiver(this, mWindow);
	} else
		mDropTarget = NULL;

	ShowWindow(mWindow, SW_SHOWMAXIMIZED);
	mDragging=-1;
	mWantFakeUp=0;
	mMouse.x = 0;
	mMouse.y = 0;
	mSizeMin.x = 0;
	mSizeMin.y = 0;
	sIniting = false;
	mIsModal = 0;
}

XWin::XWin(
	int				default_dnd,
	const char * 	inTitle,
	int				inAttributes,
	int				inX,
	int				inY,
	int				inWidth,
	int				inHeight)
{
	sIniting = true;
	RECT	bounds = { inX, inY, inX + inWidth, inY + inHeight };
	DWORD style = WS_SYSMENU | WS_CAPTION | WS_BORDER;

	(inAttributes & xwin_style_resizable) ?
		(style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME) :
		(style |= WS_MINIMIZEBOX);

	AdjustWindowRect(&bounds, style, true);
	mWindow = CreateWindowW(sWindowClass, 
		convert_str_to_utf16(inTitle).c_str(),
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		bounds.right-bounds.left, bounds.bottom-bounds.top-1,
		NULL,	// Parent
		NULL,	// Menu
		gInstance,	// (app)
		this);	// Misc param

	if (mWindow == NULL)
		throw mWindow;

	mSizeMin.x = 0;
	mSizeMin.y = 0;

	if (inAttributes & xwin_style_fullscreen)
	{
		mSizeMin.x = inWidth;
		mSizeMin.y = inHeight;
	}

	sWindows[mWindow] = this;

	if (default_dnd)
	{
		mDropTarget = new CDropTarget;
		RegisterDragDrop(mWindow, mDropTarget);
		mDropTarget->SetReceiver(this, mWindow);
	} else
		mDropTarget = NULL;

	if (inAttributes & xwin_style_visible)
		ShowWindow(mWindow, (inAttributes & xwin_style_fullscreen ) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	mDragging=-1;
	mWantFakeUp=0;
	mMouse.x = 0;
	mMouse.y = 0;

	mIsModal = inAttributes & xwin_style_modal;

	if(mIsModal)
	{
		for(map<HWND,XWin *>::iterator all = sWindows.begin(); all != sWindows.end(); ++all)
		{
				if(all->first != mWindow)
					EnableWindow(all->first,FALSE);
		}
	}

	sIniting = false;
}

XWin::~XWin()
{
	if(mIsModal)
	{
		for(map<HWND,XWin *>::iterator all = sWindows.begin(); all != sWindows.end(); ++all)
		{
				if(all->first != mWindow)
					EnableWindow(all->first,TRUE);
		}
	}

	if (mWindow)
	{
		sWindows.erase(mWindow);
		KillTimer(mWindow, IDT_TIMER1);
	}


	if (mWindow)
		DestroyWindow(mWindow);
	if (mDropTarget)
		mDropTarget->Release();
}

void			XWin::SetTitle(const char * inTitle)
{
	SetWindowTextW(mWindow, convert_str_to_utf16(inTitle).c_str());
}

void			XWin::SetFilePath(const char * inPath,bool modified)
{
}

void			XWin::MoveTo(int inX, int inY)
{
	SetWindowPos(mWindow, NULL, inX, inY, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
}

void			XWin::Resize(int inWidth, int inHeight)
{
	WINDOWINFO info = { 0 };
	if(::GetWindowInfo(mWindow,&info))
	{
		inWidth += (info.rcWindow.right - info.rcWindow.left) - (info.rcClient.right - info.rcClient.left);
		inHeight+= (info.rcWindow.bottom - info.rcWindow.top) - (info.rcClient.bottom - info.rcClient.top);
		if (info.dwStyle & WS_MAXIMIZE)
		{
			mSizeMin.x = 0;                  // stop WED from sending out size hints (do they EVER make sense ?)
			mSizeMin.y = 0;
			ShowWindow(mWindow, SW_RESTORE); // windows bug to not restore resizing via frame-dragging unless WS_MAXIMIZE is explicily cancelled
		}
	}
	SetWindowPos(mWindow, NULL, 0, 0, inWidth, inHeight, SWP_NOOWNERZORDER | SWP_NOMOVE);
}

void			XWin::ForceRefresh(void)
{
	::InvalidateRect(mWindow, NULL, false);	// Invalidate whole window, no erase, async
}

void			XWin::UpdateNow(void)
{
	UpdateWindow(mWindow);					// This does a sync refresh
}

void			XWin::SetVisible(bool visible)
{
	ShowWindow(mWindow, visible ? SW_SHOW : SW_HIDE);
	if (visible) BringWindowToTop(mWindow);
}

bool			XWin::GetVisible(void) const
{
	return ::IsWindowVisible(mWindow);
}

bool			XWin::GetActive(void) const
{
	return ::GetForegroundWindow() == mWindow;
}


void			XWin::SetTimerInterval(double seconds)
{
	if (seconds)
		SetTimer(mWindow, IDT_TIMER1, 1000.0 * seconds, NULL);
	else
		KillTimer(mWindow, IDT_TIMER1);
}

void			XWin::GetBounds(int * outX, int * outY)
{
	RECT	rect;
	if (::GetClientRect(mWindow, &rect))
	{
		if (outX) *outX = rect.right - rect.left;
		if (outY) *outY = rect.bottom - rect.top;
	}
}

void			XWin::GetWindowLoc(int * outX, int * outY)
{
	RECT	rect;
	if (::GetWindowRect(mWindow, &rect))
	{
		if (outX) *outX = rect.left;
		if (outY) *outY = rect.top;
	}
}

void XWin::GetDesktop(int bounds[4])
{
	bounds[0] = GetSystemMetrics(SM_XVIRTUALSCREEN);
	bounds[1] = GetSystemMetrics(SM_YVIRTUALSCREEN);
	bounds[2] = bounds[0] + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	bounds[3] = bounds[1] + GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

void			XWin::GetMouseLoc(int * outX, int * outY)
{
	if (outX) *outX = mMouse.x;
	if (outY) *outY = mMouse.y;
}

LRESULT CALLBACK XWin::WinEventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	XWin * obj = sWindows[hWnd];
	LRESULT result = 0;

	PAINTSTRUCT ps;
	HDC hdc;
	int btn;

	switch (message) {

	case WM_ACTIVATE:
		if (obj && !sIniting)
			obj->Activate(wParam != WA_INACTIVE);
		break;

	case WM_CREATE:
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (obj && !sIniting)
			obj->Update(hdc);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		RevokeDragDrop(hWnd);
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		if (obj && wParam == IDT_TIMER1)
			obj->Timer();
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		if (obj)
		{
			POINTSTOPOINT(obj->mMouse, MAKEPOINTS(lParam));
			btn = 0;
			switch(message) {
			case WM_LBUTTONDOWN:	btn = 0;	break;
			case WM_RBUTTONDOWN:	btn = 1;	break;
			case WM_MBUTTONDOWN:	btn = 2;	break;
			case WM_XBUTTONDOWN:	btn = GET_XBUTTON_WPARAM(wParam) - XBUTTON1 + 3; break;
			}
			if(obj->mDragging==-1)
			{
				obj->mDragging=btn;
				obj->ClickDown(obj->mMouse.x, obj->mMouse.y, btn);
				if(obj->mWantFakeUp)
				{
					btn=obj->mDragging;
					obj->mDragging=-1;
					obj->mWantFakeUp=0;
					obj->ClickUp(obj->mMouse.x,obj->mMouse.y,btn);
				}
			}
			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		if (obj)
		{
			btn = 0;
			switch(message) {
			case WM_LBUTTONUP:	btn = 0;	break;
			case WM_RBUTTONUP:	btn = 1;	break;
			case WM_MBUTTONUP:	btn = 2;	break;
			case WM_XBUTTONUP:	btn = GET_XBUTTON_WPARAM(wParam) - XBUTTON1 + 3; break;
			}

			POINTSTOPOINT(obj->mMouse, MAKEPOINTS(lParam));
			if(obj->mDragging==btn)
			{
				obj->mDragging=-1;
				obj->ClickUp(obj->mMouse.x, obj->mMouse.y, btn);
			}
		}
		ReleaseCapture();
		break;

	case WM_MOUSEWHEEL:
//	case WM_MOUSEHWHEEL:
		if (obj)
		{
			POINT	p;
			POINTSTOPOINT(p, MAKEPOINTS(lParam));
			ScreenToClient(obj->mWindow,&p);
			obj->MouseWheel(p.x, p.y, GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA, (message == WM_MOUSEWHEEL) ? 0 : 1);
		}
		break;

	case WM_MOUSEMOVE:
		if (obj)
		{
			POINTSTOPOINT(obj->mMouse, MAKEPOINTS(lParam));
			if(obj->mDragging >= 0)
			{
				obj->ClickDrag(obj->mMouse.x,obj->mMouse.y, obj->mDragging);
				if(obj->mWantFakeUp)
				{
					btn=obj->mDragging;
					obj->mDragging=-1;
					obj->mWantFakeUp=0;
					obj->ClickUp(obj->mMouse.x,obj->mMouse.y,btn);
				}
			}
			else
				obj->ClickMove(obj->mMouse.x,obj->mMouse.y);
		}
		break;

	case WM_WINDOWPOSCHANGED:
		if (obj && !sIniting)
		{
			LPWINDOWPOS p = (LPWINDOWPOS) lParam;
			if ((p->flags & SWP_NOSIZE) == 0)
			{
				RECT	rect;
				if (::GetClientRect(hWnd, &rect))
				obj->Resized(rect.right-rect.left,rect.bottom-rect.top);
				else
				obj->Resized(p->cx, p->cy);
			}
		}
		result = DefWindowProcW(hWnd, message, wParam, lParam);
		break;

	case WM_ERASEBKGND:
		break;

	case WM_GETMINMAXINFO:
		if (obj)
		{
			MINMAXINFO * mmi = (MINMAXINFO *) lParam;
			if (obj->mSizeMin.x != 0 && obj->mSizeMin.y != 0)
				mmi->ptMinTrackSize = obj->mSizeMin;
		}
		break;

	case WM_KEYUP:
		if (obj)
		{
			if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
			{
				if (obj->mDragging >= 0)
				{
					obj->ClickDrag(obj->mMouse.x,obj->mMouse.y, obj->mDragging);
					if(obj->mWantFakeUp)
					{
						btn = obj->mDragging;
						obj->mDragging = -1;
						obj->mWantFakeUp = 0;
						obj->ClickUp(obj->mMouse.x,obj->mMouse.y,btn);
					}
				}
				else
					obj->ClickMove(obj->mMouse.x,obj->mMouse.y);
			}
		}
		result = DefWindowProcW(hWnd, message, wParam, lParam);
		break;

	case WM_KEYDOWN:
		if (obj)
		{
			unsigned int vKey, RetCode, ScanCode;
			unsigned short Char = 0;
			BYTE KeyState[256];
			HKL hKL = GetKeyboardLayout(0);
			ScanCode = ((lParam>> 16) & 0xff);
			vKey = MapVirtualKeyEx(ScanCode, 1, hKL);
			GetKeyboardState((unsigned char*)&KeyState);
			ToAsciiEx(vKey, ScanCode, (unsigned char*)&KeyState, &Char, 0, hKL);
			char c = Char;

			if (c == 0)
			{
				switch(wParam & 0xFF) {
				case 0x28:
					c = 31;
					break;
				case 0x26:
					c = 30;
					break;
				case 0x25:
					c = 28;
					break;
				case 0x27:
					c = 29;
					break;
				}
			}
			if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
			{
				if (obj->mDragging >= 0)
				{
					obj->ClickDrag(obj->mMouse.x,obj->mMouse.y, obj->mDragging);
					if(obj->mWantFakeUp)
					{
						btn=obj->mDragging;
						obj->mDragging=-1;
						obj->mWantFakeUp=0;	
						obj->ClickUp(obj->mMouse.x,obj->mMouse.y,btn);
					}
				}
				else
					obj->ClickMove(obj->mMouse.x,obj->mMouse.y);
			}
#if 1
			else
			if (!obj->KeyPressed(c, message, wParam, lParam))
			{
				if (c == '=')
					obj->MouseWheel(obj->mMouse.x,obj->mMouse.y, 1, 0);
				else if (c == '-')
					obj->MouseWheel(obj->mMouse.x,obj->mMouse.y, -1, 0);
			}
#endif
		}
      break;
#if 0
	case WM_CHAR:
		if (obj) {
			obj->KeyPressed(wParam, message, wParam, lParam);
		}
		break;
#endif
	case WM_SYSCOMMAND:
		if (obj && wParam == SC_CLOSE) {
			if (obj->Closed())
				result = DefWindowProcW(hWnd, message, wParam, lParam);
		} else
			result = DefWindowProcW(hWnd, message, wParam, lParam);
		break;

	case WM_COMMAND:
		if (gMenuMap.find(LOWORD(wParam)) != gMenuMap.end())
		{
			if (!obj->HandleMenuCmd(gMenuMap[LOWORD(wParam)].first, gMenuMap[LOWORD(wParam)].second))
				result = DefWindowProcW(hWnd, message, wParam, lParam);
		} else {
			if (!obj->HandleMenuCmd(NULL, LOWORD(wParam)))
				result = DefWindowProcW(hWnd, message, wParam, lParam);
		}
		break;
	default:
		result = DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return result;
}

void	XWin::ReceiveFilesFromDrag(
						const vector<string>& inFiles)
{
	ReceiveFiles(inFiles, 0, 0);
}

void	XWin::RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEXW);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WinEventHandler;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(100));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= sWindowClass;
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(100));

	RegisterClassExW(&wcex);
}

HMENU	XWin::GetMenuBar(void)
{
	HMENU theMenu = ::GetMenu(mWindow);
	if (theMenu) return theMenu;
	theMenu = ::CreateMenu();
	::SetMenu(mWindow, theMenu);
	return theMenu;
}

#pragma mark -

xmenu			XWin::CreateMenu(xmenu parent, int item, const char * inTitle)
{
	xmenu	the_menu = ::CreateMenu();

	MENUITEMINFOA	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.hSubMenu = the_menu;
	mif.fType = MFT_STRING;
	mif.dwTypeData = const_cast<char *>(inTitle);
	mif.fMask = (item >= 0) ? MIIM_SUBMENU : (MIIM_TYPE | MIIM_SUBMENU);

	if (item == -1)
	{
		::InsertMenuItemA(parent, -1, true, &mif);
	} else {
		::SetMenuItemInfoA(parent, item, true, &mif);

	}
	return the_menu;
}

int				XWin::AppendMenuItem(xmenu menu, const char * inTitle)
{
	static	int	gIDs = 1000;
	MENUITEMINFOA	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fType = MFT_STRING;
	mif.dwTypeData = const_cast<char *>(inTitle);
	mif.fMask = MIIM_TYPE | MIIM_ID;
	mif.wID = gIDs;
	::InsertMenuItemA(menu, -1, true, &mif);
	int	itemNum = GetMenuItemCount(menu) - 1;
	gMenuMap.insert(MenuMap::value_type(gIDs, pair<xmenu,int>(menu, itemNum)));
	++gIDs;
	return itemNum;
}

int				XWin::AppendSeparator(xmenu menu)
{
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fType = MFT_SEPARATOR;
	mif.fMask = MIIM_TYPE;
	::InsertMenuItem(menu, -1, true, &mif);
	return GetMenuItemCount(menu);
}

void			XWin::CheckMenuItem(xmenu menu, int item, bool inCheck)
{
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fMask = MIIM_STATE;
	::GetMenuItemInfo(menu, item, true, &mif);
	if (inCheck)
		mif.fState |= MFS_CHECKED;
	else
		mif.fState &= ~MFS_CHECKED;
	::SetMenuItemInfo(menu, item, true, &mif);
}

void			XWin::EnableMenuItem(xmenu menu, int item, bool inEnable)
{
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fMask = MIIM_STATE;
	::GetMenuItemInfo(menu, item, true, &mif);
	if (!inEnable)
		mif.fState |= MFS_GRAYED;
	else
		mif.fState &= ~MFS_GRAYED;
	::SetMenuItemInfo(menu, item, true, &mif);
}

void			XWin::DrawMenuBar(void)
{
	::DrawMenuBar(mWindow);
}

int				XWin::TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int button, int current)
{
	POINT	p;
	p.x = mouse_x;
	p.y = mouse_y;
	ClientToScreen(mWindow, &p);
	mouse_x = p.x;
	mouse_y = p.y;

	vector<int> cmds(GetMenuItemCount(in_menu));
	for (int i = 0; i < cmds.size(); ++i)
	{
		MENUITEMINFO mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.fMask = MIIM_ID;
		GetMenuItemInfo(in_menu, i, true, &mif);
		cmds[i] = mif.wID;
		mif.wID = i+1;
		SetMenuItemInfo(in_menu, i, true, &mif);
	}

	int result = TrackPopupMenuEx(
			in_menu,
			TPM_RETURNCMD + TPM_NONOTIFY + TPM_LEFTALIGN + TPM_TOPALIGN,
			mouse_x,
			mouse_y,
			mWindow,
			NULL);

	for (int i = 0; i < cmds.size(); ++i)
	{
		MENUITEMINFO mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.fMask = MIIM_ID;
		mif.wID = cmds[i];
		SetMenuItemInfo(in_menu, i, true, &mif);
	}

	if(button == mDragging)
	{
		mWantFakeUp = 1;
	}

	return result-1;
}

