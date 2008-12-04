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
    GLXFBConfig * FbConfig = glXChooseFBConfig(_mDisplay, DefaultScreen(_mDisplay), fbAttr, &nfbConfig);
    if (FbConfig == NULL)
        throw "invalid framebuffer config";
    mContext = glXCreateNewContext(_mDisplay, *FbConfig, GLX_RGBA_TYPE, inShare ? inShare->mContext :NULL, 1);
    glXMakeContextCurrent(_mDisplay, mWindow, mWindow, mContext);
    SetTitle("XWinGL Window");
	XFree(FbConfig);
// doesn't work
//	glXSwapIntervalSGI(1);
    SetVisible(true);
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight)
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
    GLXFBConfig * FbConfig = glXChooseFBConfig(_mDisplay, DefaultScreen(_mDisplay), fbAttr, &nfbConfig);
    if (FbConfig == NULL)
        throw "invalid framebuffer config";
    mContext = glXCreateNewContext(_mDisplay, *FbConfig, GLX_RGBA_TYPE, inShare ? inShare->mContext : NULL, 1);
	glXMakeContextCurrent(_mDisplay, mWindow, mWindow, mContext);
	XFree(FbConfig);
	if (inAttributes & xwin_style_visible)
        SetVisible(true);
}

XWinGL::~XWinGL()
{
	glXMakeContextCurrent(_mDisplay, mWindow, mWindow, mContext);
	glXDestroyContext(_mDisplay,mContext);
}

void                    XWinGL::SetGLContext(void)
{
	glXMakeContextCurrent(_mDisplay, mWindow, mWindow, mContext);
}

void                    XWinGL::SwapBuffer(void)
{
	//glFinish();
	glXSwapBuffers(_mDisplay, mWindow);
}

void                    XWinGL::Resized(int inWidth, int inHeight)
{
	//glXMakeContextCurrent(_mDisplay, mWindow, mWindow, mContext);
    glViewport(0, 0, inWidth, inHeight);
    this->GLReshaped(inWidth, inHeight);
}

void                    XWinGL::Update(XContext ctx)
{
	glXMakeContextCurrent(_mDisplay, mWindow, mWindow, mContext);
    this->GLDraw();
	glXSwapBuffers(_mDisplay,mWindow);
}


