#if SOTHIS_H4X

#include "XWinGL.h"

XWinGL::XWinGL(int default_dnd, XWinGL* inShare) : XWin(default_dnd)
{
    int nfbConfig = 0;
    int fbAttr[] = {GLX_DRAWABLE_TYPE,
                    GLX_WINDOW_BIT,
                    GLX_RENDER_TYPE,
                    GLX_RGBA_BIT,
                    GLX_DOUBLEBUFFER,
                    True,
                    GLX_STENCIL_SIZE, 8,
                    GLX_DEPTH_SIZE, 16,
                    None
                   };
    mFbConfig = glXChooseFBConfig(_mDisplay, DefaultScreen(_mDisplay), fbAttr, &nfbConfig);
    if (mFbConfig == NULL)
    {
        throw "invalid framebuffer config";
    }
    mContext = glXCreateNewContext(_mDisplay, *mFbConfig, GLX_RGBA_TYPE, NULL, 1);
    mGlxWindow = glXCreateWindow(_mDisplay, *mFbConfig, mWindow, NULL);
    glXMakeContextCurrent(_mDisplay, mGlxWindow, mGlxWindow, mContext);
    SetTitle("XWinGL Window");
    if (inShare)
        {}
    return;
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) : XWin(default_dnd)
{
    int nfbConfig = 0;
    int fbAttr[] = {GLX_DRAWABLE_TYPE,
                    GLX_WINDOW_BIT,
                    GLX_RENDER_TYPE,
                    GLX_RGBA_BIT,
                    GLX_DOUBLEBUFFER,
                    True,
                    GLX_STENCIL_SIZE, 8,
                    GLX_DEPTH_SIZE, 16,
                    None
                   };
    mFbConfig = glXChooseFBConfig(_mDisplay, DefaultScreen(_mDisplay), fbAttr, &nfbConfig);
    if (mFbConfig == NULL)
    {
        throw "invalid framebuffer config";
    }
    mContext = glXCreateNewContext(_mDisplay, *mFbConfig, GLX_RGBA_TYPE, NULL, 1);
    mGlxWindow = glXCreateWindow(_mDisplay, *mFbConfig, mWindow, NULL);
    glXMakeContextCurrent(_mDisplay, mGlxWindow, mGlxWindow, mContext);
    SetTitle(inTitle);
    MoveTo(inX, inY);
    Resize(inWidth, inHeight);
    if (inShare)
        {}
    return;
}

XWinGL::~XWinGL()
{
    glXDestroyWindow(_mDisplay, mGlxWindow);
    return;
}

void                    XWinGL::SetGLContext(void)
{
    glXMakeContextCurrent(_mDisplay, mGlxWindow, mGlxWindow, mContext);
    return;
}

void                    XWinGL::SwapBuffer(void)
{
    glXSwapBuffers(_mDisplay, mGlxWindow);
    return;
}

void                    XWinGL::Resized(int inWidth, int inHeight)
{
    glXMakeContextCurrent(_mDisplay, mGlxWindow, mGlxWindow, mContext);
    glViewport(0, 0, inWidth, inHeight);
    this->GLReshaped(inWidth, inHeight);
    return;
}

#if SOTHIS_REMARK
#warning < what`s the sense of "ctx" here? i adopted it for now (from the current XWin win32 implementation). >
#endif
void                    XWinGL::Update(XContext ctx)
{
    glXMakeContextCurrent(_mDisplay, mGlxWindow, mGlxWindow, mContext);
    this->GLDraw();
    glXSwapBuffers(_mDisplay, mGlxWindow);
    return;
}

#endif // SOTHIS_H4X
