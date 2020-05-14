#include "XWinGL.h"
#include <FL/Fl.H>
#include <FL/filename.H>
	#include "AssertUtils.h"

glWidget::glWidget(XWinGL* xwin,int w,int h,Fl_Gl_Window* share) : Fl_Gl_Window(w,h)
{
   mXWinGL = xwin;
   set_visible();
   printf("glWidget ctor \n");
}

glWidget::~glWidget()
{
   printf("glWidget dtor \n");
}

void glWidget::draw()
{
	mXWinGL->GLDraw();
}

void glWidget::resize(int X,int Y,int W,int H)
{
	printf("glWidget::resize \n");
    Fl_Gl_Window::resize(X,Y,W,H);
    mXWinGL->GLReshaped(w(),h());
}


XWinGL::XWinGL(int default_dnd, XWinGL* inShare) : XWin(default_dnd), mInited(false)
{
	mGlWidget = new glWidget(this, 100,100,inShare?inShare->mGlWidget:0);
	add_resizable(*mGlWidget);
	//XWinGL::mInited = true;
	// Ben says: pixel packing expected to be "byte-packed" on all OSes - put here to mimic behavior of other
	// OSes.  If someone wants to push this down into the implementation to factor it, go for it - I'm avoiding
	// jamming stuff into code I don't have a ton of situational wwareness for.
   glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
   glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight), mInited(false)
{
	mGlWidget = new glWidget(this,inWidth,inHeight,inShare?inShare->mGlWidget:0);
	if(inAttributes & xwin_style_resizable)
		add_resizable(*mGlWidget);
	else
		add(*mGlWidget);

   glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
   glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);
 }

XWinGL::~XWinGL()
{
	printf("XWinGL dtor\n");
}

void XWinGL::Resized(int w, int h)
{
	printf("XWinGL::Resized\n");
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