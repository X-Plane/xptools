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
//                  GLX_STENCIL_SIZE, 8,
//                  GLX_DEPTH_SIZE, 16,
                    None
                   };
    GLXFBConfig * FbConfig = glXChooseFBConfig(_mDisplay, DefaultScreen(_mDisplay), fbAttr, &nfbConfig);
    if (FbConfig == NULL)
        throw "invalid framebuffer config";
    mContext = glXCreateNewContext(_mDisplay, *FbConfig, GLX_RGBA_TYPE, inShare ? inShare->mContext :NULL, 1);
    glXMakeCurrent(_mDisplay, mWindow, mContext);
    SetTitle("XWinGL Window");
	XFree(FbConfig);
    SetVisible(true);
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
 //                 GLX_STENCIL_SIZE, 8,
 //                 GLX_DEPTH_SIZE, 16,
                    None
                   };
    GLXFBConfig * FbConfig = glXChooseFBConfig(_mDisplay, DefaultScreen(_mDisplay), fbAttr, &nfbConfig);
    if (FbConfig == NULL)
        throw "invalid framebuffer config";

    mContext = glXCreateNewContext(_mDisplay, *FbConfig, GLX_RGBA_TYPE, inShare ? inShare->mContext : NULL, 1);
	glXMakeCurrent(_mDisplay,mWindow,mContext);
    SetTitle(inTitle);
    MoveTo(inX, inY);
    Resize(inWidth, inHeight);
	XFree(FbConfig);
    SetVisible(true);
}

XWinGL::~XWinGL()
{
	glXMakeCurrent(_mDisplay,NULL,NULL);
	glXDestroyContext(_mDisplay,mContext);
}

void                    XWinGL::SetGLContext(void)
{
	glXMakeCurrent(_mDisplay,mWindow,mContext); 
}

void                    XWinGL::SwapBuffer(void)
{
		glXSwapBuffers(_mDisplay, mWindow);
}

void                    XWinGL::Resized(int inWidth, int inHeight)
{
	glXMakeCurrent(_mDisplay,mWindow,mContext);
    glViewport(0, 0, inWidth, inHeight);
    this->GLReshaped(inWidth, inHeight);
}

void                    XWinGL::Update(XContext ctx)
{
	glXMakeCurrent(_mDisplay,mWindow,mContext);
    this->GLDraw();
	glXSwapBuffers(_mDisplay,mWindow);
}


