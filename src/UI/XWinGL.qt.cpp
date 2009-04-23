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

XWinGL::XWinGL(int default_dnd, XWinGL* inShare, QWidget* parent) : XWin(default_dnd, parent), mInited(false)
{
	mGlWidget = new glWidget(this, this, inShare?inShare->mGlWidget:0);
	mGlWidget->setMouseTracking(true);
	setCentralWidget(mGlWidget);
	mGlWidget->updateGL();
	XWin::show();
	XWinGL::mInited = true;
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare, QWidget* parent) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight, parent), mInited(false)
{
	mGlWidget = new glWidget(this, this, inShare?inShare->mGlWidget:0);
	mGlWidget->setMouseTracking(true);
	setCentralWidget(mGlWidget);
	mGlWidget->updateGL();
	if (inAttributes & xwin_style_visible)
		XWin::show();
	XWinGL::mInited = true;
}

XWinGL::~XWinGL()
{
	mGlWidget->makeCurrent();
	delete mGlWidget;
}

void                    XWinGL::SetGLContext(void)
{}

void                    XWinGL::SwapBuffer(void)
{}

void XWinGL::Update(XContext ctx)
{
	if (XWinGL::mInited)
		mGlWidget->update();
}
