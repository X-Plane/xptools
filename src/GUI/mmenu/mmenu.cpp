#include "GUI_Resources.h"
#include "mmenu.h"

// TODO: mmenu and mmenuitem are basicly the same, combine these two to a generic menuitem class
// TODO: locking and such, threading code is very messy

mmenu::mmenu() : a_isRunning(false), a_isVisible(false), a_height(18), a_currMenuItem(0), a_app(0)
{
    a_mutexHandle = new pthread_mutex_t;
    if (!a_mutexHandle)
        throw "could not initialize mutex";
    pthread_mutex_init(a_mutexHandle, 0);
    pthread_mutex_lock(a_mutexHandle);
    execute();
    return;
}

mmenu::~mmenu()
{
    if (!a_menubarWindow)
        return;
    a_isRunning = false;
    pthread_join(*a_threadHandle, 0);
    pthread_detach(*a_threadHandle);
    return;
}

void* mmenu::threadmain(void* arg)
{
    mmenu* tmp = reinterpret_cast<mmenu*>(arg);
    if (tmp) tmp->enter_eventloop();
    pthread_exit(0);
}

void mmenu::enter_eventloop(void)
{
    XEvent xevent;
    Window currWindow;
    XSetWindowAttributes    windowAttr;
    unsigned long           attrMask;

    a_defDisplay = XOpenDisplay(NULL);
    if (!a_defDisplay)
        throw "could not open display (:0)";
    a_screenNumber = DefaultScreen(a_defDisplay);
    a_defVisual = DefaultVisual(a_defDisplay, a_screenNumber);
    if (!a_defVisual)
        throw "could not determine default visual";
    a_defDepth  = DefaultDepth(a_defDisplay, a_screenNumber);
    if (!a_defDepth)
        throw "could not determine default color depth";
    a_displayWidth  = DisplayWidth(a_defDisplay, a_screenNumber);
    a_displayHeight = DisplayHeight(a_defDisplay, a_screenNumber);
    a_rootWindow = RootWindow(a_defDisplay, a_screenNumber);
    if (!a_rootWindow)
        throw "could not determine root window";

    windowAttr.override_redirect = True;
    windowAttr.event_mask = EnterWindowMask;
    attrMask = CWEventMask | CWOverrideRedirect;
    a_inputWindow = XCreateWindow(a_defDisplay, a_rootWindow, 0, 0, a_displayWidth, 3, 0, 0, InputOnly, a_defVisual, attrMask, &windowAttr);
    if (!a_inputWindow)
        throw "could not create input window of mainmenu";
    XMapWindow(a_defDisplay, a_inputWindow);

    windowAttr.border_pixel = 0;
    windowAttr.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | LeaveWindowMask |
                            KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask;
    windowAttr.colormap = XCreateColormap(a_defDisplay, a_rootWindow, a_defVisual, AllocNone);
    windowAttr.override_redirect = True;
    attrMask = CWBorderPixel | CWEventMask | CWColormap | CWOverrideRedirect;
    a_menubarWindow = XCreateWindow(a_defDisplay, a_rootWindow, 0, 0, a_displayWidth, a_height, 0, a_defDepth, InputOutput, a_defVisual, attrMask, &windowAttr);
    if (!a_menubarWindow)
        throw "could not create menubar window";

    double trans = 0.9;
    unsigned int opac = (unsigned int) (0xFFFFFFFF * trans);
    Atom wndOpac = XInternAtom(a_defDisplay, "_NET_WM_WINDOW_OPACITY", False);
    XChangeProperty(a_defDisplay, a_menubarWindow, wndOpac, XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &opac, 1);

    int nfbConfig = 0;
    int fbAttr[] = {GLX_DRAWABLE_TYPE,
                    GLX_WINDOW_BIT,
                    GLX_RENDER_TYPE,
                    GLX_RGBA_BIT,
                    GLX_DOUBLEBUFFER,
                    True,
                    None
                   };
    XVisualInfo* xvisual;

    bool haveVisual = false;

    a_defFbConfig = glXChooseFBConfig(a_defDisplay, a_screenNumber, fbAttr, &nfbConfig);
    if (!a_defFbConfig)
        throw "no valid glx framebuffer configurations found";
    for (int i = 0; i < nfbConfig; i++)
    {
        xvisual = glXGetVisualFromFBConfig(a_defDisplay, a_defFbConfig[i]);
        if (!xvisual) continue;
        a_currFbConfig = a_defFbConfig[i];
        haveVisual = true;
        break;
    }
    if (!haveVisual)
        throw "no valid glx visual found";
    a_defVisual = xvisual->visual;
    a_defDepth = xvisual->depth;
    a_screenNumber = xvisual->screen;
    a_defGlxContext = glXCreateNewContext(a_defDisplay, a_currFbConfig, GLX_RGBA_TYPE, NULL, 1);
    if (!a_defGlxContext)
        throw "could not create glx context";
    init_gl();

    std::string filename;
    GUI_GetTempResourcePath("sans8_8.laft", filename);
	a_defFont = new GUI_Laftfont(0xFF);
	a_defFont->initLists(filename, 8);
    a_menuAtom = XInternAtom(a_defDisplay, "_MMENU_EVENT", False);

    a_isRunning = true;
    pthread_mutex_unlock(a_mutexHandle);
    while (a_isRunning)
    {
        XNextEvent(a_defDisplay, &xevent);
        currWindow = ((XAnyEvent*)&xevent)->window;

        if (currWindow == a_inputWindow)
        {
            if (xevent.type == EnterNotify)
            {
                if (!a_isVisible)
                    show();
            }
        }

        if (currWindow == a_menubarWindow)
        {
            if (xevent.type == LeaveNotify)
                if (a_isVisible)
                {
                    if (!a_currMenuItem)
                        hide();
                    if (a_currMenuItem && !a_currMenuItem->a_isVisible) hide();
                }
            if (xevent.type == Expose)
                onExposure();
            if (xevent.type == MotionNotify)
            {
                if (items.back()->a_xmax < xevent.xmotion.x)
                {
                    if (a_currMenuItem) a_currMenuItem->hide();
                    a_currMenuItem = 0;
                    onExposure();
                    continue;
                }
                for (std::list<mmenuitem*>::iterator it = items.begin(); it != items.end(); ++it)
                    if ((*it)->isItemAreaX(xevent.xmotion.x))
                    {
                        if ((*it) == a_currMenuItem) continue;
                        else
                        {
                            if (a_currMenuItem) a_currMenuItem->hide();
                            a_currMenuItem = (*it);
                            onExposure();
                            (*it)->show();
                        }
                    }
            }
            if (xevent.type == ButtonPress && xevent.xbutton.button == Button1)
            {
                if (a_currMenuItem && a_app)
                    handlemenucommand(a_currMenuItem->a_id, a_app);
            }
        }

        for (std::list<mmenuitem*>::iterator it = items.begin(); it != items.end(); ++it)
        {
            if (currWindow == (*it)->getItemWindow())
            {
                if (xevent.type == Expose)
                    (*it)->onExposure();
                if (xevent.type == LeaveNotify)
                {
                    (*it)->a_currMenuItem = 0;
                    (*it)->onExposure();
                    if (xevent.xcrossing.y > 0)
                    {
                        hide();
                        (*it)->hide();
                    }
                }
                if (xevent.type == ButtonPress && xevent.xbutton.button == Button1)
                {
                    if ((*it)->a_currMenuItem && a_app)
                        handlemenucommand((*it)->a_currMenuItem->a_id, a_app);
                }
                if ((*it)->items.empty()) continue;
                if (xevent.type == MotionNotify)
                {
                    if ((*it)->items.back()->a_ymax < xevent.xmotion.y)
                    {

                        if ((*it)->a_currMenuItem) (*it)->a_currMenuItem->hide();
                        (*it)->a_currMenuItem = 0;
                        (*it)->onExposure();
                        continue;
                    }
                    for (std::list<mmenuitem*>::iterator iter = (*it)->items.begin(); iter != (*it)->items.end(); ++iter)
                        if ((*iter)->isItemAreaY(xevent.xmotion.y))
                        {
                            if ((*iter) == (*it)->a_currMenuItem) continue;
                            else
                            {
                                if ((*it)->a_currMenuItem) (*it)->a_currMenuItem->hide();
                                (*it)->a_currMenuItem = (*iter);
                                (*it)->onExposure();
                                (*iter)->show();
                            }
                        }
                }
            }
        }

    }
}

void mmenu::registerApp(GUI_Application* app)
{
    a_app = app;
    return;
}

void mmenu::registerCB(void (*dispatcher)(int, void*))
{
    handlemenucommand = dispatcher;
}

void mmenu::addItem(unsigned int id, std::string& name)
{
    pthread_mutex_lock(a_mutexHandle);
    mmenuitem* item = new mmenuitem(a_defDisplay, a_defVisual, a_defDepth, a_screenNumber, a_rootWindow, a_defGlxContext, a_defFont);
    item->a_id = id;
    if (items.empty())
        item->a_xmin = 0;
    else
        item->a_xmin = items.back()->a_xmax;
    item->setName(name);
    items.push_back(item);
    pthread_mutex_unlock(a_mutexHandle);
    return;
}

void mmenu::addItem(unsigned int id, std::string& name, std::string& parent)
{
    pthread_mutex_lock(a_mutexHandle);
    for (std::list<mmenuitem*>::iterator it = items.begin(); it != items.end(); ++it)
    {
        if ((*it)->name() == parent)
        {
            (*it)->addItem(id, name);
            (*it)->a_height = (*it)->items.back()->a_ymax + 5;
            XResizeWindow(a_defDisplay, (*it)->getItemWindow(), (*it)->a_width, (*it)->a_height);
            glXMakeContextCurrent(a_defDisplay, (*it)->getItemWindow(), (*it)->getItemWindow(), (*it)->a_defGlxContext);
            glViewport(0, 0, (*it)->a_width, (*it)->a_height);
            glMatrixMode(GL_PROJECTION);
        	glLoadIdentity();
        	glOrtho(0, (*it)->a_width, (*it)->a_height, 0, -100000.0f, 0.1f);
        	glMatrixMode(GL_MODELVIEW);
        	glLoadIdentity();

            break;
        }
    }
    pthread_mutex_unlock(a_mutexHandle);
}

void mmenu::init_gl()
{
    glXMakeContextCurrent(a_defDisplay, a_menubarWindow, a_menubarWindow, a_defGlxContext);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glClearColor(0.9255f, 0.9137f, 0.9137f, 1.0f);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, a_displayWidth, a_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, a_displayWidth, a_height, 0, -100000.0f, 0.1f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glXSwapBuffers(a_defDisplay, a_menubarWindow);
    return;
}

int mmenu::menuShowed(Display* display, XEvent* event, XPointer arg)
{
    return (event->type == MapNotify) && (event->xmap.window == (Window)arg);
}

void mmenu::destroy()
{
    glXDestroyContext(a_defDisplay, a_defGlxContext);
    XDestroyWindow(a_defDisplay, a_menubarWindow);
    XDestroyWindow(a_defDisplay, a_inputWindow);
    a_menubarWindow = 0;
    a_inputWindow = 0;
    a_isRunning = false;
    return;
}

void mmenu::show()
{
    if (items.empty())
        return;
    XEvent xevent;
    XMapWindow(a_defDisplay, a_menubarWindow);
    XIfEvent(a_defDisplay, &xevent, menuShowed, (XPointer)a_menubarWindow);
    a_isVisible = true;
    return;
}

void mmenu::hide()
{
    XUnmapWindow(a_defDisplay, a_menubarWindow);
    a_isVisible = false;
    a_currMenuItem = 0;
    return;
}

int mmenu::execute()
{
    a_threadHandle = new pthread_t;
    if (!a_threadHandle)
        throw "insufficient memory, __FILE__, __FUNCTION__, __LINE__";
    a_isRunning = true;
    if (pthread_create(a_threadHandle, NULL, threadmain, this))
        throw "could not create thread, __FILE__, __FUNCTION__, __LINE__";

}

void mmenu::onExposure()
{
    glXMakeContextCurrent(a_defDisplay, a_menubarWindow, a_menubarWindow, a_defGlxContext);
    glClear(GL_COLOR_BUFFER_BIT);
    for (std::list<mmenuitem*>::iterator it = items.begin(); it != items.end(); ++it)
    {
        if ((*it) == a_currMenuItem)
            glColor4f(1.0f, 1.0f, 0.8f, 1.0f);
        else
            glColor4f(0.3f, 0.3f, 0.2f, 1.0f);
        std::wstring t((*it)->name().begin(), (*it)->name().end());
        a_defFont->printgl((*it)->a_renderx, a_height/2 + (*it)->a_th/2, 0, t);
    }
    glBegin(GL_LINE_STRIP);
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    glVertex2i(0, 18);
    glVertex2i(a_displayWidth, 18);
    glEnd();
    glXSwapBuffers(a_defDisplay, a_menubarWindow);
    return;
}



// menuitem


mmenuitem::mmenuitem(Display* displ, Visual* vis, int depth, int screen, Window parent, GLXContext ctx, GUI_Laftfont* font) :
    a_defDisplay(displ), a_defVisual(vis), a_defDepth(depth), a_screenNumber(screen),
    a_rootWindow(parent), a_defFont(font), a_currMenuItem(0), a_ymin(0), a_ymax(0), a_width(250), a_height(100)
{
    XSetWindowAttributes    windowAttr;
    unsigned long           attrMask;

    windowAttr.border_pixel = 0;
    windowAttr.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | LeaveWindowMask |
                            KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask;
    windowAttr.colormap = XCreateColormap(a_defDisplay, a_rootWindow, a_defVisual, AllocNone);
    windowAttr.override_redirect = True;
    attrMask = CWBorderPixel | CWEventMask | CWOverrideRedirect | CWColormap;
    a_itemchildWindow = XCreateWindow(a_defDisplay, a_rootWindow, 0, 0, a_height, a_width, 0, a_defDepth, InputOutput, a_defVisual, attrMask, &windowAttr);
    if (!a_itemchildWindow)
        throw "could not create menuitem window";

    double trans = 0.9;
    unsigned int opac = (unsigned int) (0xFFFFFFFF * trans);
    Atom wndOpac = XInternAtom(a_defDisplay, "_NET_WM_WINDOW_OPACITY", False);
    XChangeProperty(a_defDisplay, a_itemchildWindow, wndOpac, XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &opac, 1);

    int nfbConfig = 0;
    int fbAttr[] = {GLX_DRAWABLE_TYPE,
                    GLX_WINDOW_BIT,
                    GLX_RENDER_TYPE,
                    GLX_RGBA_BIT,
                    GLX_DOUBLEBUFFER,
                    True,
                    None
                   };
    XVisualInfo* xvisual;

    bool haveVisual = false;

    a_defFbConfig = glXChooseFBConfig(a_defDisplay, a_screenNumber, fbAttr, &nfbConfig);
    if (!a_defFbConfig)
        throw "no valid glx framebuffer configurations found";
    for (int i = 0; i < nfbConfig; i++)
    {
        xvisual = glXGetVisualFromFBConfig(a_defDisplay, a_defFbConfig[i]);
        if (!xvisual) continue;
        a_currFbConfig = a_defFbConfig[i];
        haveVisual = true;
        break;
    }
    if (!haveVisual)
        throw "no valid glx visual found";
    a_defVisual = xvisual->visual;
    a_defDepth = xvisual->depth;
    a_screenNumber = xvisual->screen;
    a_defGlxContext = glXCreateNewContext(a_defDisplay, a_currFbConfig, GLX_RGBA_TYPE, ctx, 1);
    if (!a_defGlxContext)
        throw "could not create glx context";
    init_gl();
    return;
}

mmenuitem::~mmenuitem()
{
    return;
}

Window mmenuitem::getItemWindow()
{
    return a_itemchildWindow;
}

void mmenuitem::init_gl()
{
    glXMakeContextCurrent(a_defDisplay, a_itemchildWindow, a_itemchildWindow, a_defGlxContext);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glClearColor(0.9255f, 0.9137f, 0.9137f, 1.0f);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, a_height, a_width);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, a_height, a_width, 0, -100000.0f, 0.1f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glXSwapBuffers(a_defDisplay, a_itemchildWindow);
    return;
}

int mmenuitem::itemShowed(Display* display, XEvent* event, XPointer arg)
{
    return (event->type == MapNotify) && (event->xmap.window == (Window)arg);
}

void mmenuitem::addItem(unsigned int id, std::string& name)
{
    mmenuitem* item = new mmenuitem(a_defDisplay, a_defVisual, a_defDepth, a_screenNumber, a_rootWindow, a_defGlxContext, a_defFont);
    item->a_id = id;
    if (items.empty())
    {
        item->a_ymin = 0;
        item->a_ymax = 0;
    }
    else
        item->a_ymin = items.back()->a_ymax;
    item->setName(name);
    items.push_back(item);
    return;
}

void mmenuitem::destroy()
{
    return;
}

void mmenuitem::show()
{
    if (items.empty())
        return;
    XEvent xevent;
    XMapWindow(a_defDisplay, a_itemchildWindow);
    XIfEvent(a_defDisplay, &xevent, itemShowed, (XPointer)a_itemchildWindow);
    a_isVisible = true;
    return;
}

void mmenuitem::hide()
{
    XUnmapWindow(a_defDisplay, a_itemchildWindow);
    a_isVisible = false;
    a_currMenuItem = 0;
    return;
}

void mmenuitem::setName(std::string& name)
{
    if (name == "-") name = "  _____________________";
    a_name = name;
    std::wstring t(a_name.begin(), a_name.end());
    a_tw = a_defFont->textWidth(t);
    a_th = a_defFont->textHeight(t);
    a_xmax = a_xmin + a_tw + 20;
    a_renderx = a_xmin + ((a_xmax - a_xmin) / 2) - a_tw/2;
    a_ymax = a_ymin + a_th + 10;
    a_rendery = a_ymin + ((a_ymax - a_ymin) / 2) + a_th/2;
    XMoveWindow(a_defDisplay, a_itemchildWindow, a_xmin, 18);
    return;
}

std::string& mmenuitem::name()
{
    return a_name;
}

bool mmenuitem::isItemAreaX(int x)
{
    return (x >= a_xmin && x <= a_xmax);
}

bool mmenuitem::isItemAreaY(int y)
{
    return (y >= a_ymin && y <= a_ymax);
}

void mmenuitem::onExposure()
{
    glXMakeContextCurrent(a_defDisplay, a_itemchildWindow, a_itemchildWindow, a_defGlxContext);
    glClear(GL_COLOR_BUFFER_BIT);
    for (std::list<mmenuitem*>::iterator it = items.begin(); it != items.end(); ++it)
    {
        if (((*it) == a_currMenuItem) && ((*it)->name() != "  _____________________"))
        {
            glColor4f(1.0f, 1.0f, 0.8f, 1.0f);
        }
        else
            glColor4f(0.3f, 0.3f, 0.2f, 1.0f);
        std::wstring t((*it)->name().begin(), (*it)->name().end());
        a_defFont->printgl(10, (*it)->a_rendery, 0, t);
        glBegin(GL_LINE_STRIP);
        glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
        glVertex2i(0, 1);
        glVertex2i(0, a_height);
        glVertex2i(a_width-1, a_height);
        glVertex2i(a_width-1, 1);
        glEnd();
    }
    glXSwapBuffers(a_defDisplay, a_itemchildWindow);
    return;
}
