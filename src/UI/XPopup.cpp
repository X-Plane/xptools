#include <string.h>
#include "XPopup.h"

XPopup::XPopup()
{
    XSetWindowAttributes	window_attributes;
    unsigned long			attribute_mask;
    Visual*					visual;
    int						depth;
    int						screen;
    GLXFBConfig*            default_fbconfig;
    GLXFBConfig             current_fbconfig;
	Atom					atom_wnd_type;
	Atom					atom_wnd_state;
	Atom					atom_type_popup;
//	Atom					atom_motif_hints;
	Atom					atom_opacity;
//	MwmHints				mwmhints = {};
	unsigned int opacity;
	double trans;
    int got_visual;
	int nfbconfig;
	int fbattr[] = {GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
					GLX_RENDER_TYPE, GLX_RGBA_BIT,
					GLX_DOUBLEBUFFER, True,
					None};
    XVisualInfo* xvisual;
    mW = 150;
    mH = 250;
    mX = 0;
    mY = 0;
    mVisible = false;
    
    memset(&mWnd, 0, sizeof(xwindow));
    memset(&mIWnd, 0, sizeof(xwindow));
    memset(&mTarget, 0, sizeof(xwindow));
    
	if (!mWnd.display)
	{
		mWnd.display = XOpenDisplay(0);
		if (!mWnd.display) throw 0;
	}

	default_fbconfig = glXChooseFBConfig(mWnd.display, DefaultScreen(mWnd.display), fbattr, &nfbconfig);
    if (!default_fbconfig)
		throw 0;

    for (int i = 0; i < nfbconfig; i++)
	{
		xvisual = glXGetVisualFromFBConfig(mWnd.display, default_fbconfig[i]);
		if (!xvisual) continue;
        current_fbconfig = default_fbconfig[i];
        got_visual = 1;
        break;
    }
    if (!got_visual)
		throw 0;

	visual = xvisual->visual;
	depth = xvisual->depth;
	screen = xvisual->screen;
	displaywidth  = DisplayWidth(mWnd.display, screen);
	displayheight = DisplayHeight(mWnd.display, screen);
	if (!mWnd.parent)
	{
		mWnd.parent = RootWindow(mWnd.display, screen);
		mWnd.root = RootWindow(mWnd.display, screen);
		if (!mWnd.parent)
			throw 0;
    }
	mWnd.context = glXCreateNewContext(mWnd.display, current_fbconfig, GLX_RGBA_TYPE, 0, 1);
    if (!mWnd.context)
		throw 0;

// input window which covers the whole display to handle clicks which will close the popup window 
	window_attributes.override_redirect = True;
    window_attributes.event_mask = ButtonPressMask;
    attribute_mask = CWEventMask | CWOverrideRedirect;
    mIWnd.window = XCreateWindow(mWnd.display, mWnd.root, 0, 0, displaywidth, displayheight, 0, 0, InputOnly, visual, attribute_mask, &window_attributes);
    if (!mIWnd.window)
		throw 0;

// the popup window
    window_attributes.border_pixel = 0;
    window_attributes.override_redirect = True;
    window_attributes.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask;
	window_attributes.colormap = XCreateColormap(mWnd.display, mWnd.root, visual, AllocNone);
    attribute_mask = CWBorderPixel | CWEventMask | CWOverrideRedirect | CWColormap;
    mWnd.window = XCreateWindow(mWnd.display, mWnd.root, mX, mY, mW, mH, 0, 0, InputOutput, visual, attribute_mask, &window_attributes);
    if (!mWnd.window)
		throw 0;

// we would use this if we wouldn't set override_redirect to remove decorations
/*
	atom_motif_hints = XInternAtom(mWnd.display, "_MOTIF_WM_HINTS", False);
	mwmhints.flags = MWM_HINTS_DECORATIONS;
	mwmhints.decorations = 0;
	XChangeProperty(mWnd.display, mWnd.window, atom_motif_hints, atom_motif_hints, 32, PropModeReplace, (unsigned char *)&mwmhints, PROP_MWM_HINTS_ELEMENTS);
*/
// this is only useful when using a composite manager, because these are allowed to redirect windows even if override_redirect is set
	atom_wnd_type = XInternAtom(mWnd.display, "_NET_WM_WINDOW_TYPE", False);
	atom_type_popup = XInternAtom(mWnd.display, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
	XChangeProperty(mWnd.display, mWnd.window, atom_wnd_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&atom_type_popup, 1);

// has only effect if a composite manager is present
	trans = 0.8;
    opacity = (unsigned int)(0xFFFFFFFF*trans);
    atom_opacity = XInternAtom(mWnd.display, "_NET_WM_WINDOW_OPACITY", False);
    XChangeProperty(mWnd.display, mWnd.window, atom_opacity, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&opacity, 1);

    _init_opengl(mW, mH);
    glXSwapBuffers(mWnd.display, mWnd.window);
   	XFree(xvisual);
	XFree(default_fbconfig);
	XFlush(mWnd.display);
}

XPopup::~XPopup()
{
}

void XPopup::show_at_pointer()
{
	_move_to_mousepointer();
	XMapRaised(mWnd.display, mIWnd.window);
    XMapRaised(mWnd.display, mWnd.window);
    XFlush(mWnd.display);
   	_event_loop();
   	_hide();
}

void XPopup::register_target(Display* dspl, Window wnd)
{
	mTarget.display = dspl;
	mTarget.window = wnd;
}

/* private members */

void XPopup::_init_opengl(int w, int h)
{
	glXMakeContextCurrent(mWnd.display, mWnd.window, mWnd.window, mWnd.context);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glClearColor(0.9255f, 0.9137f, 0.9137f, 1.0f);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, h, 0, -100000.0f, 0.1f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return;
}

void XPopup::_resize(int w, int h)
{
	mW = w;
	mH = h;
	XResizeWindow(mWnd.display, mWnd.window, w, h);
	_init_opengl(mW, mH);
	_drawself();
	glXSwapBuffers(mWnd.display, mWnd.window);
	XFlush(mWnd.display);
}

void XPopup::_move_to_mousepointer()
{
	Window child, root;
	int cx, cy;
	unsigned int mask;
	XQueryPointer(mWnd.display, mIWnd.window, &root, &child, &mX, &mY, &cx, &cy, &mask);
	// ensure that the whole popupwindow will be visible
	if ((mX + mW) > (displaywidth)) mX = mX - mW;
	if ((mY + mH) > (displayheight)) mY = mY - mH;
    XMoveWindow(mWnd.display, mWnd.window, mX, mY);
    XFlush(mWnd.display);
}

void XPopup::_hide()
{
	XUnmapWindow(mWnd.display, mIWnd.window);
	XUnmapWindow(mWnd.display, mWnd.window);
	XFlush(mWnd.display);
}

void XPopup::_drawself()
{
}

void XPopup::_event_loop()
{
	XEvent e;
	for (;;)
	{
		XNextEvent(mWnd.display, &e);
		if ((e.xany.window != mIWnd.window) && (e.xany.window != mWnd.window))
			continue;
		switch (e.type)
		{
			case Expose:
				glXMakeContextCurrent(mWnd.display, mWnd.window, mWnd.window, mWnd.context);
				glClear(GL_COLOR_BUFFER_BIT);
				_drawself();
				glXSwapBuffers(mWnd.display, mWnd.window);
				break;
			case ButtonPress:
				if ((e.xbutton.button != Button1) && (e.xbutton.button != Button3))
					break;
				if (e.xany.window == mIWnd.window)
					return;
				if (e.xany.window == mWnd.window)
				{
					if (!mTarget.window || !mTarget.display) return;
					XEvent e;
    				e.xclient.type			= ClientMessage;
    				e.xclient.send_event	= True;
    				e.xclient.window		= mTarget.window;
    				e.xclient.display		= mWnd.display;
    				e.xclient.message_type	= XInternAtom(mTarget.display, "_POPUP_ACTION", False);
    				e.xclient.data.l[0]		= 0;
    				e.xclient.format		= 32;
    				e.xclient.serial		= 0;
    				XSendEvent(mTarget.display, mTarget.window, False, 0, &e);
					return;
				}
				break;
			default:
				break;
		}
	}
}


