#include "XWinGL.h"

glWidget::glWidget(QWidget *parent, XWinGL* xwin, QGLWidget* share) : QGLWidget(parent, share)
{
	mXWinGL = xwin;
}

glWidget::~glWidget()
{}


void glWidget::resizeGL(int inWidth, int inHeight)
{
	if (mXWinGL->mInited) {
		glViewport(0, 0, inWidth, inHeight);
		mXWinGL->GLReshaped(inWidth, inHeight);
	}
}

void glWidget::paintGL(void)
{
	if (mXWinGL->mInited)
		mXWinGL->GLDraw();
}

void glWidget::initializeGL(void)
{}

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

XWinGL::XWinGL(int default_dnd, XWinGL* inShare, QWidget* parent) : XWin(default_dnd, parent), mInited(false)
{
	mGlWidget = new glWidget(this, this, inShare?inShare->mGlWidget:0);

	mGlWidget->setMouseTracking(true);
	//mGlWidget->setFocusPolicy(Qt::StrongFocus);
	setCentralWidget(mGlWidget);
	layout()->update();
	layout()->activate();
	//mroe: in QT5 updateGL() the widgets rendering context will not become the current as it should
	//so we do it 
	mGlWidget->makeCurrent();
	mGlWidget->updateGL();
	XWin::SetVisible(true);
	XWinGL::mInited = true;
	// Ben says: pixel packing expected to be "byte-packed" on all OSes - put here to mimic behavior of other
	// OSes.  If someone wants to push this down into the implementation to factor it, go for it - I'm avoiding
	// jamming stuff into code I don't have a ton of situational wwareness for.
   glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
   glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);	
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare, QWidget* parent) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight, parent), mInited(false)
{
	mGlWidget = new glWidget(this, this, inShare?inShare->mGlWidget:0);
	mGlWidget->setMouseTracking(true);
	//mGlWidget->setFocusPolicy(Qt::StrongFocus);
	setCentralWidget(mGlWidget);
	layout()->update();
	layout()->activate();
	mGlWidget->makeCurrent();
	mGlWidget->updateGL();
	if (inAttributes & xwin_style_visible) {
		XWin::SetVisible(true);
	}
	XWinGL::mInited = true;
   glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
   glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);
}

XWinGL::~XWinGL()
{
	mGlWidget->makeCurrent();
	delete mGlWidget;
}

void                    XWinGL::Resized(int w, int h)
{}

void                    XWinGL::SetGLContext(void)
{}

void                    XWinGL::SwapBuffer(void)
{}

void XWinGL::Update(XContext ctx)
{
	if (XWinGL::mInited)
		mGlWidget->update();
}
