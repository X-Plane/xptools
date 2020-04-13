#include "XWinGL.h"
#include <FL/Fl.H>
#include <FL/filename.H>
	#include "AssertUtils.h"

glWidget::glWidget(XWinGL* xwin,int w,int h,Fl_Gl_Window* share) : Fl_Gl_Window(w,h)
{
   mXWinGL = xwin;
   //Fl::visual(FL_RGB8);
   set_visible();
   printf("glWidget ctor \n");
}

glWidget::~glWidget()
{
   printf("glWidget dtor \n");
}

void glWidget::draw()
{
//		if(!valid())
//		{
//			int W= w();int H=h();
//			glLoadIdentity();
//			glViewport(0,0,W,H);
//			valid(1);
//		}

		mXWinGL->GLDraw();
}

int glWidget::handle(int e)
{

	int ret = Fl_Gl_Window::handle(e);

 	/* DnD related events */
	switch(e)
	{
		case FL_DND_ENTER:
		case FL_DND_DRAG:
		case FL_DND_LEAVE:
			return 1;
		case FL_DND_RELEASE:

			return 1;
		case FL_PASTE:{
			  //TODO: can carry a list of filenames , can become much more
			 char c[2048];
			 strncpy(c, Fl::event_text(), sizeof(c));
			 fl_decode_uri(c);
			 printf("glWidget::handle FL_PASTE Win %s\n",c);
			 mXWinGL->ReceiveFilesFromDrag(c);
			return 1;}
	}

	return ret;
}


void glWidget::resize(int X,int Y,int W,int H)
{
	printf("glWidget::resize \n");
    Fl_Gl_Window::resize(0,mXWinGL->GetMenuBarHeight(),W,H-mXWinGL->GetMenuBarHeight());
	//glViewport(0,0,w(),h());
    mXWinGL->GLReshaped(w(),h());
}



// void glWidget::focusInEvent(QFocusEvent* e)
// {
// 	if (mXWinGL->mInited && e->reason()==Qt::ActiveWindowFocusReason)
// 		mXWinGL->Activate(1);
// }
//
// void glWidget::focusOutEvent(QFocusEvent* e)
// {
// 	if (mXWinGL->mInited && e->reason()==Qt::ActiveWindowFocusReason)
// 		mXWinGL->Activate(0);
// }

XWinGL::XWinGL(int default_dnd, XWinGL* inShare) : XWin(default_dnd), mInited(false)
{
	mGlWidget = new glWidget(this, 100,100,inShare?inShare->mGlWidget:0);
	add(mGlWidget);
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
	add(mGlWidget);

	//mGlWidget->show();

	//mGlWidget->make_current();
	//XWinGL::mInited = true;

//	if (inAttributes & xwin_style_visible) {
//		XWin::SetVisible(true);
//	}



   glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
   glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);
 }

XWinGL::~XWinGL()
{
	printf("XWinGL dtor\n");
	mGlWidget->make_current();
	mGlWidget->hide();
	delete mGlWidget;
}

void XWinGL::Resized(int w, int h)
{
	printf("XWinGL::Resized\n");
	mGlWidget->size(w,h);
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