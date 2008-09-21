#include "XWin.h"

// this will only work with one diplay and visual, so no native multiscreen support
// (Xinerama and such will work anyway), but makes life much easier

static Display*        mDisplay = 0;
static Visual*         mXVisual = 0;
static int             a_defDepth = 0;
static int             a_screenNumber = 0;
static bool            sIniting = false;
static std::map<Window, XWin*> sWindows;

static int i = 0;

static int ishexdig(char s) {
  return (((s >= '0') && (s <= '9'))
	  || ((s >= 'a') && (s <= 'f'))
	  || ((s >= 'A') && (s <= 'F')));
}

static int hexval(char s) {
  return (((s >= '0') && (s <= '9'))
	  ? s - '0'
	  : (((s >= 'a') && (s <= 'f'))
	     ? s - 'a' + 10
	     : s - 'A' + 10));
}

static void decode_percent_escapes(char *s)
{
  int src = 0, dest = 0;
  while (s[src]) {
    if ((s[src] == '%')
	&& ishexdig(s[src+1])
	&& ishexdig(s[src+2])) {
      int v;
      v = ((hexval(s[src+1]) << 4) + hexval(s[src+2]));
      s[dest++] = v;
      src += 3;
    } else {
      s[dest++] = s[src++];
    }
  }
  s[dest] = 0;
}


KeySym TkpGetKeySym(Display* dispPtr, XEvent * eventPtr)
{
    KeySym sym;
    int index;

    index = 0;
    if (eventPtr->xkey.state & Mod5Mask) {
	index = 2;
    }
    if ((eventPtr->xkey.state & ShiftMask) || (eventPtr->xkey.state & LockMask))
    {
	index += 1;
    }
    sym = XKeycodeToKeysym(dispPtr, eventPtr->xkey.keycode, index);

    if ((index & 1) && !(eventPtr->xkey.state & ShiftMask)
	    ) {
	if (!(((sym >= XK_A) && (sym <= XK_Z))
		|| ((sym >= XK_Agrave) && (sym <= XK_Odiaeresis))
		|| ((sym >= XK_Ooblique) && (sym <= XK_Thorn)))) {
	    index &= ~1;
	    sym = XKeycodeToKeysym(dispPtr, eventPtr->xkey.keycode,
		    index);
	}
    }
    if ((index & 1) && (sym == NoSymbol)) {
	sym = XKeycodeToKeysym(dispPtr, eventPtr->xkey.keycode,
		index & ~1);
    }
    return sym;
}

void XWin::RegisterClass(Display* display, int screen, int depth, Visual* visual)
{
    mDisplay = display;
    mXVisual = visual;
    a_defDepth = depth;
    a_screenNumber = screen;
    return;
}

void XWin::WinEventHandler(XAnyEvent* xevent, int* visualstate)
{
    if (sWindows.find(xevent->window) == sWindows.end())
        return;
    XWin* obj = sWindows[xevent->window];
    XEvent e = *(XEvent*)xevent;
    Atom _wmp = XInternAtom(mDisplay, "WM_PROTOCOLS", False);
    Atom _wdw = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);

    switch (e.type)
    {
    case ClientMessage:
    {
    if (obj)
    {
        if ( ((Atom)e.xclient.message_type == _wmp) && ((Atom)e.xclient.data.l[0] == _wdw) )
        {
            if (!obj->Closed()) break;
            sWindows.erase(xevent->window);
            if (sWindows.empty())
            {
                *visualstate = 0;
            }
            XUnmapWindow(mDisplay, xevent->window);
            XDestroyWindow(mDisplay, xevent->window);
            return;
        }
        if (e.xclient.message_type == obj->dnd.XdndEnter)
        {}
        else if (e.xclient.message_type == obj->dnd.XdndPosition)
        {
            xdnd_convert_selection(&obj->dnd, XDND_POSITION_SOURCE_WIN(&e), e.xclient.window, obj->dnd.text_uri_list, CurrentTime);
            xdnd_send_status(&obj->dnd, XDND_POSITION_SOURCE_WIN(&e), e.xclient.window, 1, 0, 0, 0, 100, 100, obj->dnd.XdndActionPrivate);
        }
        else if (e.xclient.message_type == obj->dnd.XdndDrop)
        {
            if (!xdnd_convert_selection(&obj->dnd, XDND_DROP_SOURCE_WIN(&e), e.xclient.window, obj->dnd.text_uri_list, CurrentTime))
            {
                unsigned long remain, len = 0;
                char *data = NULL;
                char *data2 = NULL;
                Atom ret_type;
                int ret_format;
                vector<string> inFiles;
                string f;
                int read_bytes = 1024;
                // read whole property data
                do
                {
                    if(data != 0)
                    XFree(data);
                    XGetWindowProperty(mDisplay, e.xclient.window, obj->dnd.dnd_data, 0, read_bytes, False, AnyPropertyType, &ret_type, &ret_format, &len, &remain, (unsigned char**)&data);
                    read_bytes *= 2;
                } while(remain != 0);
                if (data)
                {
                    long offset = 0;
                    while (offset < len)
                    {
                        long elem_end;
                        for (elem_end = offset; (elem_end < len) && (data[elem_end] != '\r'); elem_end++) {}
                        if ((offset + 7 <= len) && !strncmp(data + offset, "file://", 7))
                        {
                            long i = offset + 7;
                            while ((i < elem_end) && (data[i] != '/')) i++;
                            if (i < elem_end)
                            {
                                data2 = new char[elem_end - i + 1];
                                memcpy(data2, data + i, elem_end - i);
                                data2[elem_end - i] = 0;
                                decode_percent_escapes(data2);
                                f.assign(data2);
                                inFiles.push_back(f);
                                delete[] data2;
                            }
                        }
                        offset = elem_end + 2; /* assume CRLF */
	                }
	                XFree(data);
	                obj->ReceiveFilesFromDrag(inFiles);
                }
            }
            xdnd_send_finished(&obj->dnd, XDND_DROP_SOURCE_WIN(&e), e.xclient.window, 0);
        }
        else if (e.xclient.message_type == obj->dnd.XdndLeave)
        {
            xdnd_send_finished(&obj->dnd, XDND_LEAVE_SOURCE_WIN(&e), e.xclient.window, 0);
        }
    }
    break;
    }
    case KeyPress:
    {
        #if SOTHIS_REMARK
        #warning < this is dirty, KeySym is UCS2 afaik, maybe change key callback to \
                   support unicode, at least UTF16, on Windows this would be triggered by WM_CHAR >
        #endif
        if (obj)
        {
            char c = (char)TkpGetKeySym(mDisplay, &e);
            if (!obj->KeyPressed(c, 0, 0, 0))
		    {
    			if (c == '=')
			        obj->MouseWheel(obj->mMouse.x,obj->mMouse.y, 1, 0);
		        else if (c == '-')
    				obj->MouseWheel(obj->mMouse.x,obj->mMouse.y, -1, 0);
            }
        }
        break;
    }
    case KeyRelease:
    {
        break;
    }
    case ButtonPress:
    {
        int btn = 0;
        if (obj)
		{
            obj->mMouse.x = e.xbutton.x;
		    obj->mMouse.y = e.xbutton.y;
            switch (e.xbutton.button)
            {
            case Button1: // left
                btn = 0;
                break;
            case Button2: // middle
                btn = 2;
                break;
            case Button3: // right
                btn = 1;
                break;
            }
    		if(obj->mDragging[btn]==0)
            {
		    		obj->mDragging[btn]=1;
			    	obj->ClickDown(obj->mMouse.x, obj->mMouse.y, btn);
            }
            // mouse capturing is on per default
		}
        break;
    }
    case ButtonRelease:
    {
        int btn = 0;
        if (obj)
		{
		    obj->mMouse.x = e.xbutton.x;
		    obj->mMouse.y = e.xbutton.y;
            switch (e.xbutton.button)
            {
            case Button1: // left
                btn = 0;
                break;
            case Button2: // middle
                btn = 2;
                break;
            case Button3: // right
                btn = 1;
                break;
            }
    		if(obj->mDragging[btn])
			    obj->ClickUp(obj->mMouse.x, obj->mMouse.y, btn);
			obj->mDragging[btn]=0;
		}
		switch (e.xbutton.button)
        {
            case Button4: // wheel up
               if (obj)
		        {
			        POINT	p;
                    p.x = e.xbutton.x;
			        p.y = e.xbutton.y;
			        obj->MouseWheel(p.x, p.y, 1, 0);
		        }
		        break;
            case Button5: // wheel down
                if (obj)
		        {
			        POINT	p;
			        p.x = e.xbutton.x;
			        p.y = e.xbutton.y;
			        obj->MouseWheel(p.x, p.y, -1, 0);
		        }
                break;
        }
        break;
    }
    case MotionNotify: // mouse move
    {
        if (obj)
		{
            obj->mMouse.x = e.xmotion.x;
		    obj->mMouse.y = e.xmotion.y;
			int bc=0;
			for(int b=0;b<BUTTON_DIM;++b)
			if(obj->mDragging[b])
			{
				++bc;
                obj->ClickDrag(obj->mMouse.x,obj->mMouse.y, b);
			}
			if(bc==0)
				obj->ClickMove(obj->mMouse.x,obj->mMouse.y);
	    }
		break;
    }
    case ConfigureNotify:
    {
        if (obj && !sIniting)
		{
            obj->Resized(e.xconfigure.width, e.xconfigure.height);
			obj->width = e.xconfigure.width;
			obj->height = e.xconfigure.height;
		}
        break;
    }
    case UnmapNotify:
    {
        // minimized or invisible
        break;
    }
    case MapNotify:
    {
        // restored or visible
        break;
    }
    case Expose: // client area destroyed due to overlapped window or something
    {
        if (obj && !sIniting && !e.xexpose.count)
		{
            obj->Update(xevent->window);
			obj->refresh_requests = 0;
		}
        break;
    }
    default:
        break;
    }
    return;
}


XWin::XWin(int default_dnd)
{
    XEvent xevent;
    XSizeHints  sizehints;
    sIniting = true;
    visible = false;

    _mDisplay = mDisplay;

    windowAttr.border_pixel = 0;
    windowAttr.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
                            KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask;
    windowAttr.colormap = XCreateColormap(mDisplay, RootWindow(mDisplay, a_screenNumber), mXVisual, AllocNone);
    attrMask = CWBorderPixel | CWEventMask | CWColormap;

    mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, a_screenNumber), 10, 10, 200,
                            100, 0, a_defDepth, InputOutput, mXVisual, attrMask, &windowAttr);
    Atom wdw = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, mWindow, &wdw, 1);
    title.value    = (unsigned char*)"XWin Window";
    title.encoding = XA_STRING;
    title.format   = 8;
    title.nitems   = strlen((char*)title.value);
    XSetWMName(mDisplay, mWindow, &title);
    sizehints.flags = PMaxSize|PMinSize;
    sizehints.min_width = 200;
    sizehints.min_height = 100;
    // should be enough, except someone has Xinerama with 64 screens :P
    sizehints.max_width = 65535;
    sizehints.max_height = 65535;
    XSetWMNormalHints(mDisplay, mWindow, &sizehints);

    // support for MIME text/uri-list only atm
    if (default_dnd)
    {
        Atom l[2];
        l[0] = dnd.text_uri_list;
        l[1] = 0;
        xdnd_init(&dnd, mDisplay);
        xdnd_set_dnd_aware(&dnd, mWindow, l);
    }

    // just some standard values (black)
    defGCvalues.foreground = 0;
    defGCvalues.background = 0;
    defGCmask = GCForeground | GCBackground;
    defGC = XCreateGC(mDisplay, mWindow, defGCmask, &defGCvalues);

    if (!mWindow)
        throw mWindow;
    visible = 0;
    sWindows[mWindow] = this;
    memset(mDragging,0,sizeof(mDragging));
	mMouse.x = 0;
	mMouse.y = 0;
    sIniting = false;
	refresh_requests = 0;
    return;
}

XWin::~XWin()
{
/*    XEvent xevent;
    Atom _wmp = XInternAtom(mDisplay, "WM_PROTOCOLS", False);
    Atom _wdw = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    xevent.xclient.type                 = ClientMessage;
    xevent.xclient.send_event       = True;
    xevent.xclient.window           = mWindow;
    xevent.xclient.message_type     = _wmp;
    xevent.xclient.data.l[0]        = _wdw;
    xevent.xclient.format           = 32;
    xevent.xclient.serial           = 0;
    XSendEvent(mDisplay, mWindow, False, 0, &xevent);
	XSync(mDisplay, False); */

    return;
}


void                    XWin::SetTitle(const char * inTitle)
{
    title.value    = (unsigned char*)inTitle;
    title.encoding = XA_STRING;
    title.format   = 8;
    title.nitems   = strlen((char*)title.value);
    XSetWMName(mDisplay, mWindow, &title);
//	XFlush(mDisplay);
    return;
}

void                    XWin::MoveTo(int inX, int inY)
{
    XMoveWindow(mDisplay, mWindow, inX, inY);
//	XFlush(mDisplay);
    return;
}

void                    XWin::Resize(int inWidth, int inHeight)
{
    XResizeWindow(mDisplay, mWindow, inWidth, inHeight);
//	XFlush(mDisplay);
	width = inWidth;
	height = inHeight;
    return;
}

void                    XWin::ForceRefresh(void)
{
    XEvent xevent;
    xevent.xexpose.type             = Expose;
    xevent.xexpose.send_event       = True;
    xevent.xexpose.window           = mWindow;
    xevent.xexpose.serial           = 0;
	xevent.xexpose.count			= 0;
	if (!refresh_requests)
    	XSendEvent(mDisplay, mWindow, False, 0, &xevent);
//	XFlush(mDisplay);
//	XSync(mDisplay, False);
	refresh_requests++;
    return;
}

void                    XWin::Update(XContext ctx)
{
    return;
}

void                    XWin::UpdateNow(void)
{
	Update(0);
    return;
}

void                    XWin::SetVisible(bool visible)
{
    visible?XMapWindow(mDisplay, mWindow):XUnmapWindow(mDisplay, mWindow);
    visible ^= 1;
//    XFlush(mDisplay);
    return;
}

bool XWin::GetVisible(void) const
{
    return visible;
}

bool XWin::GetActive(void) const
{
    return true;
}


void XWin::SetTimerInterval(double seconds)
{
    return;
}

void XWin::GetBounds(int * outX, int * outY)
{
	Window rw;
	int x_return, y_return;
	unsigned int width_return, height_return, border_width_return, depth_return;

	XGetGeometry(mDisplay, mWindow, &rw, &x_return, &y_return, &width_return,
				&height_return, &border_width_return, &depth_return);

    if (outX) *outX = width;
    if (outY) *outY = height;

    return;
}

void XWin::GetMouseLoc(int * outX, int * outY)
{
    if (outX) *outX = mMouse.x;
	if (outY) *outY = mMouse.y;
    return;
}

void	XWin::ReceiveFilesFromDrag(const vector<string>& inFiles)
{
	ReceiveFiles(inFiles, 0, 0);
	return;
}


xmenu XWin::CreateMenu(xmenu parent, int item, const char * inTitle)
{
    return 0;
}

int XWin::AppendMenuItem(xmenu menu, const char * inTitle)
{
    return 0;
}

int XWin::AppendSeparator(xmenu menu)
{
    return 0;
}

void XWin::CheckMenuItem(xmenu menu, int item, bool inCheck)
{
    return;
}
void XWin::EnableMenuItem(xmenu menu, int item, bool inEnable)
{
    return;
}

void XWin::DrawMenuBar(void)
{

}

int XWin::TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int current)
{
    return -1;
}


