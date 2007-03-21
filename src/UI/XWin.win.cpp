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

#define 	IDT_TIMER1	0x01

/*
 TODO:
 	Timers
 	mouse drag, click, wheel
 	keyboard
 	resizing notifications
	closing
*/	

#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120


typedef	map<int, pair<xmenu, int> >	MenuMap;
MenuMap	gMenuMap;


static	bool	sIniting = false; 	

static TCHAR sWindowClass[] = "XGrinderWindow";

extern	HINSTANCE	gInstance;

map<HWND, XWin *>	sWindows;

XWin::XWin()
{
	sIniting = true;
	mWindow = CreateWindow(sWindowClass, "FullScreen", 
		(WS_OVERLAPPED     | 
         WS_CAPTION        | 
         WS_SYSMENU        | 
         WS_THICKFRAME      ) ,	// Style,
		10, 10, 50, 50,
		NULL,	// Parent
		NULL,	// Menu
		gInstance,	// (app)
		this);	// Misc param

	if (mWindow == NULL)
		throw mWindow;
		
	sWindows[mWindow] = this;

	mDropTarget = new CDropTarget;	
	RegisterDragDrop(mWindow, mDropTarget);
	mDropTarget->SetReceiver(this, mWindow);

	ShowWindow(mWindow, SW_SHOWMAXIMIZED);
	sIniting = false;
}	

XWin::XWin(
	const char * 	inTitle,
	int				inX,
	int				inY,
	int				inWidth,
	int				inHeight)
{
	sIniting = true;
	mWindow = CreateWindow(sWindowClass, inTitle, 
		WS_OVERLAPPEDWINDOW,	// Style,
		inX, inY, inWidth, inHeight,
		NULL,	// Parent
		NULL,	// Menu
		gInstance,	// (app)
		this);	// Misc param

	if (mWindow == NULL)
		throw mWindow;
		
	sWindows[mWindow] = this;

	mDropTarget = new CDropTarget;	
	RegisterDragDrop(mWindow, mDropTarget);
	mDropTarget->SetReceiver(this, mWindow);

	ShowWindow(mWindow, SW_SHOW);
	sIniting = false;
}	

XWin::~XWin()
{
	if (mWindow)
	{
		sWindows.erase(mWindow);
		KillTimer(mWindow, IDT_TIMER1);
	}

	if (mWindow)
		DestroyWindow(mWindow);
	delete mDropTarget;
	
}

void			XWin::SetTitle(const char * inTitle)
{
	SetWindowText(mWindow, inTitle);
}

void			XWin::MoveTo(int inX, int inY)
{
	SetWindowPos(mWindow, NULL, inX, inY, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
}

void			XWin::Resize(int inWidth, int inHeight)
{
	SetWindowPos(mWindow, NULL, 0, 0, inWidth, inHeight, SWP_NOOWNERZORDER | SWP_NOMOVE);
}

void			XWin::ForceRefresh(void)
{
//	UpdateWindow(mWindow);					// This does a sync refresh
	::InvalidateRect(mWindow, NULL, false);	// Invalidate whole window, no erase, async
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

void			XWin::GetMouseLoc(int * outX, int * outY)
{
	if (outX) *outX = mMouseX;
	if (outY) *outY = mMouseY;
}

LRESULT CALLBACK XWin::WinEventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int	dragging = -1;
	XWin * obj = sWindows[hWnd];
	LRESULT result = 0;
	
	PAINTSTRUCT ps;
	HDC hdc;

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
		if (obj)
			obj->ClickDown(LOWORD(lParam), HIWORD(lParam), 0);
		if (obj) obj->mMouseX = LOWORD(lParam);
		if (obj) obj->mMouseY = HIWORD(lParam);		
		dragging = 0;
		SetCapture(hWnd);
		break;

	case WM_RBUTTONDOWN:
		if (obj)
			obj->ClickDown(LOWORD(lParam), HIWORD(lParam), 1);
		if (obj) obj->mMouseX = LOWORD(lParam);
		if (obj) obj->mMouseY = HIWORD(lParam);		
		dragging = 1;
		SetCapture(hWnd);
		break;

	case WM_LBUTTONUP:
		if (obj)
			obj->ClickUp(LOWORD(lParam), HIWORD(lParam), 0);
		if (obj) obj->mMouseX = LOWORD(lParam);
		if (obj) obj->mMouseY = HIWORD(lParam);		
		dragging = -1;
		ReleaseCapture();
		break;

	case WM_RBUTTONUP:
		if (obj)
			obj->ClickUp(LOWORD(lParam), HIWORD(lParam), 1);
		if (obj) obj->mMouseX = LOWORD(lParam);
		if (obj) obj->mMouseY = HIWORD(lParam);		
		dragging = -1;
		ReleaseCapture();
		break;

	case WM_MOUSEWHEEL:
		if (obj)
		{
			RECT	rect;
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (::GetWindowRect(obj->mWindow, &rect)) {
				x -= rect.left;
				y -= rect.top;
			}
			obj->MouseWheel(x, y, (short) HIWORD(wParam) / WHEEL_DELTA, 0);
		}
		break;

	case WM_MOUSEMOVE:
		if (obj) obj->mMouseX = LOWORD(lParam);
		if (obj) obj->mMouseY = HIWORD(lParam);		
		if (obj && dragging > -1)
			obj->ClickDrag(LOWORD(lParam), HIWORD(lParam), dragging);
		else
			result = DefWindowProc(hWnd, message, wParam, lParam);		
		break;

	case WM_WINDOWPOSCHANGED:
		if (obj && !sIniting)
		{
			LPWINDOWPOS	p = (LPWINDOWPOS) lParam;
			if ((p->flags & SWP_NOSIZE) == 0)			
				obj->Resized(p->cx, p->cy);
		}	
		result = DefWindowProc(hWnd, message, wParam, lParam);
		break;
	
	case WM_ERASEBKGND:
		break;
		
	case WM_KEYDOWN:		
		if (obj)
		{
			char	c = MapVirtualKeyEx(wParam, 2, NULL);
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
			obj->KeyPressed(c, message, wParam, lParam);
		}
      break;

	case WM_SYSCOMMAND:
		if (obj && wParam == SC_CLOSE) {
			if (obj->Closed())
				result = DefWindowProc(hWnd, message, wParam, lParam);			
		} else	
			result = DefWindowProc(hWnd, message, wParam, lParam);
		break;
		
	case WM_COMMAND:
		if (gMenuMap.find(LOWORD(wParam)) != gMenuMap.end())
		{
			if (!obj->HandleMenuCmd(gMenuMap[LOWORD(wParam)].first, gMenuMap[LOWORD(wParam)].second))
				result = DefWindowProc(hWnd, message, wParam, lParam); 
		} else {
			if (!obj->HandleMenuCmd(NULL, LOWORD(wParam)))
				result = DefWindowProc(hWnd, message, wParam, lParam); 
		}
		break;
	default:
		result = DefWindowProc(hWnd, message, wParam, lParam);
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
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WinEventHandler;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL; // LoadIcon(hInstance, (LPCTSTR)IDI_TESTDND);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= sWindowClass;
	wcex.hIconSm		= NULL; // LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	RegisterClassEx(&wcex);
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

	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.hSubMenu = the_menu;
	mif.fType = MFT_STRING;
	mif.dwTypeData = const_cast<char *>(inTitle);
	mif.fMask = (item >= 0) ? MIIM_SUBMENU : (MIIM_TYPE | MIIM_SUBMENU);

	if (item == -1)
	{
		::InsertMenuItem(parent, -1, true, &mif);
	} else {
		::SetMenuItemInfo(parent, item, true, &mif);
		
	}		
	return the_menu;
}

int				XWin::AppendMenuItem(xmenu menu, const char * inTitle)
{
	static	int	gIDs = 1000;
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fType = MFT_STRING;
	mif.dwTypeData = const_cast<char *>(inTitle);
	mif.fMask = MIIM_TYPE | MIIM_ID;
	mif.wID = gIDs;
	::InsertMenuItem(menu, -1, true, &mif);
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

int				XWin::TrackPopup(vector<string>& choices, int mouse_x, int mouse_y)
{
	#error DO THIS
	TrackPopupMenuEx
}

