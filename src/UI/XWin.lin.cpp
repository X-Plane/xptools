#include "XWin.h"

// this will only work with one diplay and visual, so no native multiscreen support
// (Xinerama and such will work anyway), but makes life much easier

Display*		mDisplay = 0;
static Visual*	mXVisual = 0;
static int		a_defDepth = 0;
static int		a_screenNumber = 0;
static bool		sIniting = false;
std::map<Window, XWin*> sWindows;

int ShiftMod = 0;
int AltMod = 0;
int CtrlMod = 0;

#include "GUI_Timer.h"

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

void XWin::WinEventHandler(XEvent* xevent, int* visualstate)
{
	XEvent e = *xevent;
	XWin* obj;
    if (sWindows.find(xevent->xany.window) == sWindows.end())
    {
        return;
    }
    else
    	obj = sWindows[xevent->xany.window];

    Atom _wmp = XInternAtom(mDisplay, "WM_PROTOCOLS", False);
    Atom _wdw = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    Atom atom_timer = XInternAtom(mDisplay, "_WED_TIMER", False);
    if (e.xclient.message_type == atom_timer)
    {
	   	intptr_t t = (int32_t)e.xclient.data.l[1] & 0xFFFFFFFF;
	   	t = (t << 32) | ((int32_t)e.xclient.data.l[0] & 0xFFFFFFFF);
		GUI_Timer::TimerCB(reinterpret_cast<void*>(t));
		return;
	}

    switch (e.type)
    {
    case ClientMessage:
    {
    if (obj)
    {
        if ( ((Atom)e.xclient.message_type == _wmp) && ((Atom)e.xclient.data.l[0] == _wdw) )
        {
            if (!obj->Closed()) break;
            XUnmapWindow(mDisplay, xevent->xany.window);
            XDestroyWindow(mDisplay, xevent->xany.window);
            sWindows.erase(xevent->xany.window);
            if (sWindows.empty())
            {
                *visualstate = 0;
            }
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
					char* token = strtok(data, "\n");
					while (token)
					{
						char* temp;
						temp = token;
						if (strstr(token, "file://"))
							temp+=7;
						else continue;
						data2 = new char[strlen(temp) + 1];
						memset(data2, 0, strlen(temp) + 1);
						memcpy(data2, temp, strlen(temp));
						// this one is needed for gnome only
						decode_percent_escapes(data2);
						f.assign(data2);
						// remove remaining junk (gnome uses /r/n while kde uses only /n as delimiter)
						size_t p = std::string::npos;
						while ((p = f.find('\r')) != std::string::npos)
						{
							f.erase(p, 1);
						}
						inFiles.push_back(f);
						delete[] data2;
						token = strtok(0, "\n");
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

		if (e.xclient.message_type == XInternAtom(mDisplay, "_POPUP_ACTION", False))
			printf("got popup action\n");

    }
    break;
    }
    case KeyPress:
    {
		char c;
		ShiftMod = (e.xkey.state & ShiftMask);
		CtrlMod = (e.xkey.state & ControlMask);
		AltMod = (e.xkey.state & Mod5Mask);
        #if SOTHIS_REMARK
        #warning < this is dirty, KeySym is UCS2 afaik, maybe change key callback to \
                   support unicode, at least UTF16, on Windows this would be triggered by WM_CHAR >
        #endif
        if (obj)
        {
        // do not handle extended modifiers (until we have unicode support)
        	if (XLookupKeysym(&e.xkey, 0) >= 0xFE00 && XLookupKeysym(&e.xkey, 0) <= 0xFEFF)
        		return;
			switch(XLookupKeysym(&e.xkey, 0))
			{
				/*modifiers*/
				case XK_Shift_L:
				case XK_Shift_R:
				case XK_Control_L:
				case XK_Control_R:
				case XK_Caps_Lock:
				case XK_Shift_Lock:
				case XK_Meta_L:
				case XK_Meta_R:
				case XK_Alt_L:
				case XK_Alt_R:
				case XK_Super_L:
				case XK_Super_R:
				case XK_Hyper_L:
				case XK_Hyper_R:
					return;
				/* no-ops */
				case XK_KP_Space:
				case XK_KP_Home:
				case XK_KP_Page_Up:
				case XK_KP_Page_Down:
				case XK_KP_End:
				case XK_KP_Insert:
				case XK_KP_Equal:
				case XK_KP_Begin:
				case XK_KP_Multiply:
				case XK_KP_Add:
				case XK_KP_Separator:
				case XK_KP_Subtract:
				case XK_KP_Divide:
				case XK_Num_Lock:
				case XK_Select:
				case XK_Print:
				case XK_Execute:
				case XK_Insert:
				case XK_Undo:
				case XK_Redo:
				case XK_Menu:
				case XK_Find:
				case XK_Cancel:
				case XK_Help:
				case XK_Break:
				case XK_Mode_switch:
				case XK_Home:
				case XK_Page_Up:
				case XK_Page_Down:
				case XK_End:
				case XK_Begin:
				case XK_KP_0:
				case XK_KP_1:
				case XK_KP_2:
				case XK_KP_3:
				case XK_KP_4:
				case XK_KP_5:
				case XK_KP_6:
				case XK_KP_7:
				case XK_KP_8:
				case XK_KP_9:
					return;
				case XK_KP_Left:
				case XK_Left:
					c = 0x25;
					break;
				case XK_KP_Right:
				case XK_Right:
					c = 0x27;
					break;
				case XK_KP_Up:
				case XK_Up:
					c = 0x26;
					break;
				case XK_KP_Down:
				case XK_Down:
					c = 0x28;
					break;
				case XK_KP_Delete:
				case XK_Delete:
					c = 0x2E;
					break;
				case XK_KP_Tab:
				case XK_Tab:
					c = 0x09;
					break;
				case XK_KP_Enter:
				case XK_Return:
					c = 0x0D;
					break;
				case XK_BackSpace:
					c = 0x08;
					break;
				case XK_period:
					c = 0x6E;
					break;
				case XK_KP_Decimal:
					c = 0x6E;
					break;
				default:
					c = (char)TkpGetKeySym(mDisplay, &e);
					break;
			}
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
		ShiftMod = (e.xkey.state & ShiftMask);
		CtrlMod = (e.xkey.state & ControlMask);
		AltMod = (e.xkey.state & Mod5Mask);
		if (XLookupKeysym(&e.xkey, 0) == XK_F12)
		{
			obj->toggleFullscreen();
		}
        break;
    }
    case ButtonPress:
	{
		if (!obj) return;
        int btn = 0;
        obj->mMouse.x = e.xbutton.x;
	    obj->mMouse.y = e.xbutton.y;
		obj->isResizing = false;
	    if ((e.xbutton.button == Button4) || (e.xbutton.button == Button5))
        	return;
        if (e.xbutton.button == Button1)
			btn = 0;
        if (e.xbutton.button == Button2)
			btn = 2;
        if (e.xbutton.button == Button3)
			btn = 1;
		obj->mDragging[btn]=1;
		obj->ClickDown(e.xbutton.x, e.xbutton.y, btn);
		break;
	}
    case ButtonRelease:
    {
        int btn = 0;
        if (!obj) return;
        obj->mMouse.x = e.xbutton.x;
		obj->mMouse.y = e.xbutton.y;
		obj->isResizing = false;
        if (e.xbutton.button == Button1)
			btn = 0;
        if (e.xbutton.button == Button2)
			btn = 2;
        if (e.xbutton.button == Button3)
			btn = 1;
		obj->mDragging[btn]=0;
        if (e.xbutton.button == Button4)
		{
			obj->MouseWheel(e.xbutton.x, e.xbutton.y, 1, 0);
			return;
		}
        if (e.xbutton.button == Button5)
		{
			obj->MouseWheel(e.xbutton.x, e.xbutton.y, -1, 0);
			return;
		}
		obj->ClickUp(e.xbutton.x, e.xbutton.y, btn);
		break;
    }
    case MotionNotify: // mouse move
    {
        if (obj)
		{
			obj->isResizing = false;
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
			obj->isResizing = true;
			obj->width = e.xconfigure.width;
			obj->height = e.xconfigure.height;
			obj->Resized(e.xconfigure.width, e.xconfigure.height-(obj->mMenuOffset));
		}
        break;
    }
    case UnmapNotify:
    {
		if (obj) obj->visible = false;
        break;
    }
    case MapNotify:
    {
        if (obj) obj->visible = true;
        break;
    }
    case Expose: // client area destroyed due to overlapped window or something
    {
        if (obj && !sIniting)
		{
   			obj->isResizing = false;
			if (!e.xexpose.count)
				obj->Update(xevent->xany.window);
   			if (e.xexpose.send_event)
				obj->refresh_requests = 0;
		}
        break;
    }
    case FocusIn:
    	if (obj)
    	{
	    	obj->active = true;
    		obj->Activate(1);
    	}
    	break;
    case FocusOut:
    	if (obj)
    	{
	    	obj->active = false;
    		obj->Activate(0);
    	}
	    break;
	case SelectionNotify: // we want data from another application
		break;
	case SelectionRequest: // another application wants our data
		break;
	case SelectionClear: // data was selected in another application
		break;
    default:
        break;
    }
    return;
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
    XEvent xevent;
    XSizeHints  sizehints;
    sIniting = true;
    visible = false;

    _mDisplay = mDisplay;

    windowAttr.border_pixel = 1;
    windowAttr.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
                            KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask | FocusChangeMask;
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
 //   defGCvalues.foreground = 0;
 //   defGCvalues.background = 0;
 //   defGCmask = GCForeground | GCBackground;
 //   defGC = XCreateGC(mDisplay, mWindow, defGCmask, &defGCvalues);

    if (!mWindow)
        throw mWindow;
    visible = false;
    active = false;
    sWindows[mWindow] = this;
    memset(mDragging,0,sizeof(int)*BUTTON_DIM);
	mMouse.x = 0;
	mMouse.y = 0;
	SetTitle(inTitle);
	int x = (inAttributes & xwin_style_fullscreen) ? 0 : inX;
	int y = (inAttributes & xwin_style_fullscreen) ? 0 : inY;
	int w = (inAttributes & xwin_style_fullscreen) ? 1024 : inWidth;
	int h = (inAttributes & xwin_style_fullscreen) ? 768 : inHeight;

	MoveTo(x, y);
	Resize(w, h);
    sIniting = false;
	refresh_requests = 0;
	fsState = false;
	mMenuOffset = 0;
    return;
}

XWin::XWin(int default_dnd)
{
    XEvent xevent;
    XSizeHints  sizehints;
    sIniting = true;
    visible = false;
	fsState = false;

    _mDisplay = mDisplay;


    windowAttr.border_pixel = 1;
    windowAttr.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
                            KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask | FocusChangeMask;
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
 //   defGCvalues.foreground = 0;
 //   defGCvalues.background = 0;
 //   defGCmask = GCForeground | GCBackground;
 //   defGC = XCreateGC(mDisplay, mWindow, defGCmask, &defGCvalues);

    if (!mWindow)
        throw mWindow;
    visible = false;
    active = false;
    sWindows[mWindow] = this;
    memset(mDragging,0,sizeof(int)*BUTTON_DIM);
	mMouse.x = 0;
	mMouse.y = 0;
    sIniting = false;
	refresh_requests = 0;
	mMenuOffset = 0;
    return;
}

XWin::~XWin()
{
	XUnmapWindow(mDisplay, mWindow);
    XDestroyWindow(mDisplay, mWindow);
	sWindows.erase(mWindow);
    return;
}


void                    XWin::SetTitle(const char * inTitle)
{
    title.value    = (unsigned char*)inTitle;
    title.encoding = XA_STRING;
    title.format   = 8;
    title.nitems   = strlen((char*)title.value);
    XSetWMName(mDisplay, mWindow, &title);
	//XFlush(mDisplay);
    return;
}

void                    XWin::MoveTo(int inX, int inY)
{
    XMoveWindow(mDisplay, mWindow, inX, inY);
	//XFlush(mDisplay);
    return;
}

void                    XWin::Resize(int inWidth, int inHeight)
{
	width = inWidth;
	height = inHeight;
    XResizeWindow(mDisplay, mWindow, inWidth, inHeight);
	//XFlush(mDisplay);
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
	// only refresh on first request of every loop
	// since this technique can cause a real bad bottleneck
	// as there currently is a bad recursion issue in the GUI_Pane
	// and GUI_Window class (not a bug, more a design issue, which
	// only affects the X11 way of doing things)
	if (!refresh_requests)
		XSendEvent(mDisplay, mWindow, False, 0, &xevent);
	refresh_requests++;
    return;
}

// dirty h4x!: prevent pure virtual function call
// need to track down where this happens
/*
void                    XWin::Update(XContext ctx)
{
    return;
}
*/
void                    XWin::UpdateNow(void)
{
	ForceRefresh();
    return;
}

void  XWin::toggleFullscreen()
{
	XSizeHints	sizehints;
	XEvent		tmpEv;
	Atom		wmState;
	Atom		fscr;
/*
	if (fsState)
	{
		sizehints.flags = PMinSize | PMaxSize;
		sizehints.min_width = 0;
		sizehints.max_width = fsWidth;
		sizehints.min_height = 0;
		sizehints.max_height = fsHeight;
	}
	else
	{
		sizehints.flags = PMinSize | PMaxSize;
		sizehints.min_width = sizehints.max_width = wmWidth;
		sizehints.min_height = sizehints.max_height = wmHeight;
	}*/
	memset(&tmpEv, 0, sizeof(tmpEv));
	wmState = XInternAtom(mDisplay, "_NET_WM_STATE", False);
	fscr = XInternAtom(mDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	tmpEv.xclient.type		= ClientMessage;
	tmpEv.xclient.serial		= 0;
	tmpEv.xclient.send_event	= True;
	tmpEv.xclient.window		= mWindow;
	tmpEv.xclient.message_type	= wmState;
	tmpEv.xclient.format		= 32;
	tmpEv.xclient.data.l[0] 	= (_NET_WM_STATE_TOGGLE);
	tmpEv.xclient.data.l[1] 	= fscr;
	tmpEv.xclient.data.l[2] 	= 0;
	//XSetWMNormalHints(mDisplay, mWindow, &sizehints);
	XSendEvent(mDisplay, DefaultRootWindow(mDisplay), False, SubstructureRedirectMask | SubstructureNotifyMask, &tmpEv);
	fsState ^= 1;
	return;
}

void                    XWin::SetVisible(bool visible)
{
    visible?XMapRaised(mDisplay, mWindow):XUnmapWindow(mDisplay, mWindow);
    XFlush(mDisplay);
    return;
}

bool XWin::GetVisible(void) const
{
    return visible;
}

bool XWin::GetActive(void) const
{
	return active;
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

	//XLockDisplay(mDisplay);
	XSync(mDisplay, False);
	XGetGeometry(mDisplay, mWindow, &rw, &x_return, &y_return, &width_return,
				&height_return, &border_width_return, &depth_return);
	//XUnlockDisplay(mDisplay);

    if (outX) *outX = width_return;
    if (outY) *outY = height_return;
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


