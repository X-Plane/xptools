#include "XWinGL.h"

glWidget::glWidget(QWidget *parent, XWinGL* xwin) : QGLWidget(parent)
{
	mXWinGL = xwin;
}

glWidget::~glWidget()
{}


void glWidget::resizeGL(int inWidth, int inHeight)
{
	glViewport(0, 0, inWidth, inHeight);
}

void glWidget::paintGL(void)
{
	if (mXWinGL->inited)
		mXWinGL->GLDraw();
}

void glWidget::initializeGL(void)
{}

XWinGL::XWinGL(int default_dnd, XWinGL* inShare, QWidget* parent) : XWin(default_dnd, parent)
{
	inited = false;
	mGlWidget = new glWidget(this, this);
	setCentralWidget(mGlWidget);
	XWin::show();
	inited = true;
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare, QWidget* parent) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight, parent)
{
	inited = false;
	mGlWidget = new glWidget(this, this);
	setCentralWidget(mGlWidget);
	XWin::show();
	inited = true;
}

XWinGL::~XWinGL()
{
	mGlWidget->makeCurrent();
	delete mGlWidget;
}

void                    XWinGL::SetGLContext(void)
{
	mGlWidget->makeCurrent();
}

void                    XWinGL::SwapBuffer(void)
{
	mGlWidget->swapBuffers();
}

void XWinGL::Resized(int inWidth, int inHeight)
{
	mGlWidget->makeCurrent();
	glViewport(0, 0, inWidth, inHeight);
	if (inited)
		GLReshaped(inWidth, inHeight);
}

void XWinGL::Update(XContext ctx)
{
	mGlWidget->updateGL();
}
