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

#include "XGrinderApp.h"
#include "XWin.h"
#include "XUtils.h"
#include "PlatformUtils.h"
#if APL
#include "ObjCUtils.h"
#endif
#if LIN
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <stdarg.h>
#endif

class	XGrinderWin;

static	char			gCurMessage[1024] = { 0 };
static	string			gTitle = "XGrinder";
static	XGrinderWin * 	gWin = NULL;

#if IBM
HINSTANCE	gInstance = NULL;
#endif

class	XGrinderWin : public XWin {
public:

							XGrinderWin();
	virtual					~XGrinderWin() { }
	virtual	void			Timer(void) { }
	virtual	bool			Closed(void) { XGrinder_Quit(); return true; }
	virtual	void			Resized(int inWidth, int inHeight) { }
	virtual	void			Update(XContext ctx);
	virtual void			Activate(int inActive) { }
	virtual	void			ClickDown(int inX, int inY, int inButton) { }
	virtual	void			ClickUp(int inX, int inY, int inButton) { }
	virtual	void			ClickDrag(int inX, int inY, int inButton) { }
	virtual	void			ClickMove(int inX, int inY) { }
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis) { }
	virtual	void			DragEnter(int inX, int inY) { }
	virtual	void			DragOver(int inX, int inY) { }
	virtual	void			DragLeave(void) { }
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int x, int y) { XGrindFiles(inFiles); }
	virtual	int				KeyPressed(uint32_t, long, long, long) { return 1; }
	virtual	int				HandleMenuCmd(xmenu inMenu, int inCommand) { return XGrinderMenuPick(inMenu, inCommand); };
};

XGrinderWin::XGrinderWin() : XWin(1, gTitle.c_str(),
	xwin_style_resizable  | xwin_style_centered | xwin_style_visible,
	50, 100, 512, 100)
{
}

void XGrinderWin::Update(XWin::XContext ctx)
{
	int		w, h;
	this->GetBounds(&w, &h);

#if LIN

	int mh = gWin->GetMenuBarHeight();
	fl_rectf (0,mh,w,h,FL_BACKGROUND2_COLOR);

	fl_color(0);
	int fh = fl_height() - fl_descent();
	int y = mh + fh;

	char msg[1024];int n = -1;double width;
	const char * p = gCurMessage;
	while ( n != 0)
    {
		p = fl_expand_text(p,msg,1024,w,n,width,1,0);
		fl_draw(msg,0,y);
		y  += fh;
	}
#endif
#if APL
	erase_a_rect(0,0,w,h);
	draw_text(0,0,w,h,gCurMessage);
#endif
#if IBM
	RECT	bounds;
	bounds.left = 0;
	bounds.right = w;
	bounds.top = 0;
	bounds.bottom = h;
	FillRect(ctx, &bounds, (HBRUSH) (COLOR_WINDOW+1));
	if (gCurMessage[0] != 0)
		TextOut(ctx, 0, 0, gCurMessage, strlen(gCurMessage));
#endif
}

void	XGrinder_ShowMessage(const char * fmt, ...)
{
	va_list		args;
	va_start(args, fmt);

	vsprintf(gCurMessage, fmt, args);

	if (gWin)
		gWin->ForceRefresh();

}

void	XGrinder_SetWindowTitle(const char * title)
{
	if (gWin)
		gWin->SetTitle(title);
}

void	XGrinder_Quit(void)
{
#if !LIN
	exit(0);
#endif
}

xmenu	XGrinder_AddMenu(const char * title, const char ** items)
{
	xmenu	theMenu = XWin::CreateMenu(gWin->GetMenuBar(), -1, title);
	int n = 0;
	while (items[n])
	{
		if (strcmp(items[n], "-"))
			XWin::AppendMenuItem(theMenu, items[n]);
		else
			XWin::AppendSeparator(theMenu);
		++n;
	}
	return theMenu;
}

#if LIN
int main(int argc, char* argv[])
{
	gWin = new XGrinderWin();
	XGrindInit(gTitle);
    gWin->show(argc,argv);
	gWin->ForceRefresh();

	return Fl::run();
}
#endif

#if APL
int		main(int argc, char ** argv)
{
	init_ns_stuff();

	gWin = new XGrinderWin();
	XGrindInit(gTitle);
	gWin->SetTitle(gTitle.c_str());
	gWin->ForceRefresh();

	run_app();

	return 0;
}
#endif

#if IBM
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	printf("Cmd line was: %s\n", lpCmdLine);
	gInstance = hInstance;

	MSG msg;
//	HACCEL hAccelTable;

	XWin::RegisterClass(hInstance);
	if(OleInitialize(NULL) != S_OK)
		return FALSE;

	gWin = new XGrinderWin();
	XGrindInit(gTitle);
	gWin->SetTitle(gTitle.c_str());
	gWin->DrawMenuBar();
	gWin->ForceRefresh();

	while (GetMessage(&msg, NULL, 0, 0))
	{
//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
//		}
	}

	OleUninitialize();
	return msg.wParam;
}
#endif



