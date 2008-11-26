#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <string>
#include "XMessagebox.h"
#include "GUI_Resources.h"
#include "GUI_Laftfont.h"

using std::string;

static string msgbox_msg;
static GUI_Laftfont* defFont = 0;

typedef struct xwindow
{
	Display*	display;
	Window		window;
	Window		parent;
	GLXContext	context;
} xwindow;

extern Display* mDisplay __attribute__ ((weak));


static void init_opengl(int w, int h)
{
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

static int gld_create_modalwindow(int w, int h, const char* titletext, xwindow* window)
{
    XSetWindowAttributes	window_attributes;
    unsigned long			attribute_mask;
    Visual*					visual;
    int						depth;
    int						screen;
    unsigned int			displaywidth;
    unsigned int			displayheight;
    GLXFBConfig*            default_fbconfig;
    GLXFBConfig             current_fbconfig;
    XSizeHints				sizehints;
	XTextProperty			title;
	Atom					atom_wnd_type;
	Atom					atom_wnd_state;
	Atom					atom_type_dialog;
	Atom					atom_state_modal;
	Atom					atom_state_above;
	Atom					atom_state_skip_taskbar;
	Atom					atom_state_skip_pager;
	Atom					atom_wm_delete;
    int got_visual;
	int nfbconfig;
	int fbattr[] = {GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
					GLX_RENDER_TYPE, GLX_RGBA_BIT,
					GLX_DOUBLEBUFFER, True,
					None};
    XVisualInfo* xvisual;

    if (!titletext || !window) return 0;
	if (!window->display)
	{
		if (mDisplay)
			window->display = mDisplay;
		if (!window->display) window->display = XOpenDisplay(0);
		if (!window->display) return 0;
	}
	screen = DefaultScreen(window->display);

	default_fbconfig = glXChooseFBConfig(window->display, screen, fbattr, &nfbconfig);
    if (!default_fbconfig)
		return 0;

    for (int i = 0; i < nfbconfig; i++)
	{
		xvisual = glXGetVisualFromFBConfig(window->display, default_fbconfig[i]);
		if (!xvisual) continue;
        current_fbconfig = default_fbconfig[i];
        got_visual = 1;
        break;
    }
    if (!got_visual)
		return 0;

	visual = xvisual->visual;
	depth = xvisual->depth;
	screen = xvisual->screen;
	displaywidth  = DisplayWidth(window->display, screen);
	displayheight = DisplayHeight(window->display, screen);
	XFree(xvisual);
	XFree(default_fbconfig);
	if (!window->parent)
	{
		window->parent = DefaultRootWindow(window->display);
		if (!window->parent)
			return 0;
    }
	window->context = glXCreateNewContext(window->display, current_fbconfig, GLX_RGBA_TYPE, NULL, 1);
    if (!window->context)
		return 0;

    window_attributes.border_pixel = 0;
	window_attributes.save_under = True;
    window_attributes.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask;
	window_attributes.colormap = XCreateColormap(window->display, window->parent, visual, AllocNone);
    attribute_mask = CWBorderPixel | CWEventMask | CWColormap | CWSaveUnder;
    window->window = XCreateWindow(window->display, DefaultRootWindow(window->display), displaywidth/2-w/2, displayheight/2-h/2, w, h, 0, 0, InputOutput, visual, attribute_mask, &window_attributes);
    if (!window->window)
		return 0;

    sizehints.flags = PMinSize | PMaxSize;
	sizehints.min_width = sizehints.max_width = w;
	sizehints.min_height = sizehints.max_height = h;
	XSetWMNormalHints(window->display, window->window, &sizehints);
	title.value    = (unsigned char*)titletext;
	title.encoding = XA_STRING;
	title.format   = 8;
	title.nitems   = strlen(titletext);
	XSetWMName(window->display, window->window, &title);

	atom_wm_delete = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(window->display, window->window, &atom_wm_delete, 1);

	atom_wnd_type = XInternAtom(window->display, "_NET_WM_WINDOW_TYPE", False);
	atom_type_dialog = XInternAtom(window->display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	atom_wnd_state = XInternAtom(window->display, "_NET_WM_STATE", False);
	atom_state_modal = XInternAtom(window->display, "_NET_WM_STATE_MODAL", False);
	atom_state_above = XInternAtom(window->display, "_NET_WM_STATE_ABOVE", False);
	atom_state_skip_taskbar = XInternAtom(window->display, "_NET_WM_STATE_SKIP_TASKBAR", False);
	atom_state_skip_pager = XInternAtom(window->display, "_NET_WM_STATE_SKIP_PAGER", False);
	XChangeProperty(window->display, window->window, atom_wnd_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&atom_type_dialog, 1);
	XChangeProperty(window->display, window->window, atom_wnd_state, XA_ATOM, 32, PropModeAppend, (unsigned char*)&atom_state_modal, 1);
/*
	XChangeProperty(window->display, window->window, atom_wnd_state, XA_ATOM, 32, PropModeAppend, (unsigned char*)&atom_state_above, 1);
	XChangeProperty(window->display, window->window, atom_wnd_state, XA_ATOM, 32, PropModeAppend, (unsigned char*)&atom_state_skip_taskbar, 1);
	XChangeProperty(window->display, window->window, atom_wnd_state, XA_ATOM, 32, PropModeAppend, (unsigned char*)&atom_state_skip_pager, 1);
*/
	XSetTransientForHint(window->display, window->window, window->parent);

    glXMakeContextCurrent(window->display, window->window, window->window, window->context);
    init_opengl(w, h);
	glXSwapBuffers(window->display, window->window);

	defFont = new GUI_Laftfont(0xFF);
	std::string filename;
	GUI_GetTempResourcePath("sans8_8.laft", filename);
	defFont->initLists(filename, 8);
    return 1;
}

static void gld_destroy_window(xwindow* window)
{
	if (defFont)
	{
		delete defFont;
		defFont = 0;
	}
	if (!window) return;
	if (window->display && window->context)
		glXDestroyContext(window->display, window->context);
	if (window->display && window->window)
	{
		XUnmapWindow(window->display, window->window);
		XDestroyWindow(window->display, window->window);
		//XSync(window->display, False);
	}
}

static void drawself(xwindow* window)
{
	if (!window) return;
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(0.3f, 0.3f, 0.2f, 1.0f);
	defFont->printgl(20, 30, 0, msgbox_msg);
	glXSwapBuffers(window->display, window->window);
}

static int enter_eventloop(xwindow* window)
{
	XEvent e;
	Atom wmp, wdw;
	int is_running = 1;

	if (!window) return GLDLG_ERROR;
	XMapRaised(window->display, window->window);
	wmp = XInternAtom(window->display, "WM_PROTOCOLS", False);
	wdw = XInternAtom(window->display, "WM_DELETE_WINDOW", False);

	while (is_running)
	{
		XNextEvent(window->display, &e);
		if (e.xany.window != window->window)
			continue;
		switch (e.type)
		{
			case ClientMessage:
				if ((e.xclient.message_type == wmp) && ((Atom)e.xclient.data.l[0] == wdw))
					return GLDLG_CLOSED;
				break;
			case Expose:
				glXMakeContextCurrent(window->display, window->window, window->window, window->context);
				drawself(window);
				break;
			default:
				break;
		}
	}
	return GLDLG_ERROR;
}

int XMessagebox(Window parent, const char* title, const char* message, int type, int defaultaction)
{
    xwindow msgbox = {};
    int res;

	msgbox_msg = message;
	msgbox.parent = parent;

	if (!gld_create_modalwindow(380, 100, title, &msgbox))
	{
		gld_destroy_window(&msgbox);
		return 0;
	}
	res = enter_eventloop(&msgbox);
	gld_destroy_window(&msgbox);
	return res;
}

