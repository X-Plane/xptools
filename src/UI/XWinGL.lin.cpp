#include "XWinGL.h"
#include <FL/Fl.H>
#include <FL/filename.H>
#include "AssertUtils.h"

glWidget::glWidget(XWinGL* xwin,int w,int h,Fl_Gl_Window* share) : Fl_Gl_Window(w,h)
{
	//TODO: mroe: that is redundant because FLTK shares the context anyhow.
	//Probably we make a own context here for new windows not sharing context.
	//Having one before a window is shown  would also avoid all the deferred gl init.
	if(share) this->context(share->context(),0);
	mXWinGL = xwin;
	set_visible();
}

glWidget::~glWidget()
{}

void glWidget::draw()
{
    if (!mXWinGL->mInited)
	{
		make_current();

		glPixelStorei	(GL_UNPACK_ALIGNMENT,1);
		glPixelStorei	(GL_PACK_ALIGNMENT  ,1);

		if(GLint err = glewInit())
			LOG_MSG("I/WGL glewInit failed\n");
		else
			LOG_MSG("I/WGL glewInit OK\n");

		mXWinGL->mInited = true;
	}

	mXWinGL->GLDraw();
}

void glWidget::resize(int X,int Y,int W,int H)
{
    Fl_Gl_Window::resize(X,Y,W,H);
    mXWinGL->GLReshaped(w(),h());
}


XWinGL::XWinGL(int default_dnd, XWinGL* inShare) : XWin(default_dnd), mInited(false)
{
	mGlWidget = new glWidget(this, 100,100,inShare?inShare->mGlWidget:0);
	add_resizable(*mGlWidget);

	if(inShare) XWinGL::mInited = true;
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight), mInited(false)
{
	mGlWidget = new glWidget(this,inWidth,inHeight,inShare?inShare->mGlWidget:0);
	if(inAttributes & xwin_style_resizable)
		add_resizable(*mGlWidget);
	else
		add(*mGlWidget);

	if(inShare) XWinGL::mInited = true;
}

XWinGL::~XWinGL()
{
}

void XWinGL::Resized(int w, int h)
{
}

void XWinGL::SetGLContext(void)
{
	mGlWidget->make_current();
}

void XWinGL::SwapBuffer(void)
{
}

void XWinGL::Update(XContext ctx)
{
    mGlWidget->redraw();
}