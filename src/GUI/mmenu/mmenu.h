#ifndef MMENU_H
#define MMENU_H

#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <list>
#include <string>
#include "GUI_Laftfont.h"

class GUI_Application;
class mmenuitem;
class mmenu;

class mmenu
{
public:
    mmenu();
    virtual ~mmenu();

    int execute();
    void addItem(unsigned int id, std::string& name);
    void addItem(unsigned int id, std::string& name, std::string& parent);
    void registerApp(GUI_Application* app);
    void registerCB(void (*dispatcher)(int, void*));
private:
    static void*            threadmain(void* arg);
    void                    enter_eventloop(void);
    static int menuShowed(Display* display, XEvent* event, XPointer arg);
    void init_gl();
    void destroy();
    void show();
    void hide();
    virtual void onExposure();

    bool a_isRunning;
    bool a_isVisible;
    bool a_isIniting;
    Display*     a_defDisplay;
    Visual*      a_defVisual;
    int          a_defDepth;
    int          a_screenNumber;
    unsigned int a_displayWidth;
    unsigned int a_displayHeight;
    Atom         a_wdwAtom;
    Atom         a_wmpAtom;
    Atom         a_menuAtom;
    GLXFBConfig* a_defFbConfig;
    GLXFBConfig  a_currFbConfig;
    GLXContext   a_defGlxContext;
    pthread_t*   a_threadHandle;
    pthread_mutex_t*  a_mutexHandle;

    void (*handlemenucommand)(int, void*);
    GUI_Application* a_app;

    Window       a_rootWindow;
    Window       a_inputWindow;
    Window       a_menubarWindow;

    GUI_Laftfont* a_defFont;

    int          a_height;

    std::list<mmenuitem*> items;
    mmenuitem*   a_currMenuItem;
};

class mmenuitem
{
public:
    mmenuitem(Display* displ, Visual* vis, int depth, int screen, Window parent, GLXContext ctx, GUI_Laftfont* font);
    virtual ~mmenuitem();

    void show();
    void hide();
    void setName(std::string& name);
    bool isItemAreaX(int x);
    bool isItemAreaY(int y);
    void addItem(unsigned int id, std::string& name);

    std::string& name();
    Window getItemWindow();
    virtual void onExposure();
    int          a_xmin;
    int          a_xmax;
    int          a_ymin;
    int          a_ymax;
    int          a_tw;
    int          a_th;
    int          a_renderx;
    int          a_rendery;
    mmenuitem*   a_currMenuItem;
    std::list<mmenuitem*> items;
    GLXContext   a_defGlxContext;
    bool         a_isVisible;
    int          a_height;
    int          a_width;
    unsigned int a_id;
private:
    static int itemShowed(Display* display, XEvent* event, XPointer arg);
    void init_gl();
    void destroy();

    Display*     a_defDisplay;
    Visual*      a_defVisual;
    int          a_defDepth;
    int          a_screenNumber;
    Atom         a_wdwAtom;
    Atom         a_wmpAtom;
    GLXFBConfig* a_defFbConfig;
    GLXFBConfig  a_currFbConfig;

    Window       a_rootWindow;
    Window       a_itemchildWindow;

    GUI_Laftfont* a_defFont;

    std::string  a_name;


};

#endif /* MMENU_H */

