#include "XWinGL.h"

XWinGL::XWinGL(int default_dnd, XWinGL* inShare) : XWin(default_dnd)
{
    int nfbConfig = 0;
	int found_config = 0;
	GLXFBConfig	fb_config;
    int fbAttr[] = {GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
					GLX_RENDER_TYPE, GLX_RGBA_BIT,
					GLX_DOUBLEBUFFER, True,
					None
                   };
    GLXFBConfig* fb_configs = glXChooseFBConfig(_mDisplay,
		DefaultScreen(_mDisplay), fbAttr, &nfbConfig);

	for (int i = 0; i < nfbConfig; i++)
	{
		XVisualInfo* xvisual_info = glXGetVisualFromFBConfig(_mDisplay,
			fb_configs[i]);
		if (!xvisual_info)
			continue;
		fb_config = fb_configs[i];
		found_config = 1;
		break;
	}
	if (!found_config)
		throw "no valid framebuffer configuration found.";
    mContext = glXCreateNewContext(_mDisplay, fb_config, GLX_RGBA_TYPE,
		inShare ? inShare->mContext :NULL, 1);
	glXMakeCurrent(_mDisplay, mWindow, mContext);
	XFree(fb_configs);
    SetTitle("XWinGL Window");
    SetVisible(true);
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight)
{
    int nfbConfig = 0;
	int found_config = 0;
	GLXFBConfig	fb_config;
    int fbAttr[] = {GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
					GLX_RENDER_TYPE, GLX_RGBA_BIT,
					GLX_DOUBLEBUFFER, True,
					None
                   };
    GLXFBConfig* fb_configs = glXChooseFBConfig(_mDisplay,
		DefaultScreen(_mDisplay), fbAttr, &nfbConfig);

	for (int i = 0; i < nfbConfig; i++)
	{
		XVisualInfo* xvisual_info = glXGetVisualFromFBConfig(_mDisplay,
			fb_configs[i]);
		if (!xvisual_info)
			continue;
		fb_config = fb_configs[i];
		found_config = 1;
		break;
	}
	if (!found_config)
		throw "no valid framebuffer configuration found.";
    mContext = glXCreateNewContext(_mDisplay, fb_config, GLX_RGBA_TYPE,
		inShare ? inShare->mContext :NULL, 1);
	glXMakeCurrent(_mDisplay, mWindow, mContext);
	XFree(fb_configs);
    SetTitle("XWinGL Window");
	if (inAttributes & xwin_style_visible)
        SetVisible(true);
}

XWinGL::~XWinGL()
{
	//glXMakeContextCurrent(_mDisplay, None, None, mContext);
	glXDestroyContext(_mDisplay, mContext);
}

void                    XWinGL::SetGLContext(void)
{
	glXMakeCurrent(_mDisplay, mWindow, mContext);
}

void                    XWinGL::SwapBuffer(void)
{
	glXSwapBuffers(_mDisplay, mWindow);
}

void                    XWinGL::Resized(int inWidth, int inHeight)
{
    glViewport(0, 0, inWidth, inHeight);
    this->GLReshaped(inWidth, inHeight);
}

void                    XWinGL::Update(XContext ctx)
{
	glXMakeCurrent(_mDisplay, mWindow, mContext);
    this->GLDraw();
	glXSwapBuffers(_mDisplay, mWindow);
}
