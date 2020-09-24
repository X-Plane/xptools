/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef XWINGL_H
#define XWINGL_H

#include "glew.h"

#if APL
	#include <OpenGL/gl.h>
	#if __OBJC__
		#import <AppKit/AppKit.h>
	#else
		typedef void NSOpenGLView;
	#endif
#elif LIN
	#include <QtOpenGL/QGLWidget>

class	XWinGL;

class glWidget : public QGLWidget
{
	Q_OBJECT
public:
	glWidget(QWidget *parent, XWinGL* xwin, QGLWidget* share);
	virtual ~glWidget(void);
protected:
	void resizeGL(int inWidth, int inHeight);
	void paintGL(void);
	void initializeGL(void);
// 	void focusInEvent(QFocusEvent* e);
// 	void focusOutEvent(QFocusEvent* e);
private:
	XWinGL* mXWinGL;
};

#endif

#include "XWin.h"

class	XWinGL : public XWin
{
#if LIN
	Q_OBJECT
public:
	XWinGL(int default_dnd, XWinGL * inShare, QWidget* parent = 0);
	XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare, QWidget* parent = 0);
#else
public:

	XWinGL(int default_dnd, XWinGL * inShare);
	XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare);
#endif
	virtual					~XWinGL();

			void			SetGLContext(void);
			void			SwapBuffer(void);

#if IBM
			HDC				GetDC(void);
#endif
	virtual	void			Resized(int, int);

	// New hooks for you:
	virtual	void			GLReshaped(int inWidth, int inHeight)=0;
	virtual	void			GLDraw(void)=0;

	// Handled for you

	virtual	void			Update(XContext ctx);
	virtual	int				HandleMenuCmd(xmenu inMenu, int inCommand) { return 0; }
	virtual void			Activate(int active) { }

private:

#if APL
		NSOpenGLView *		mContext;
#endif

#if IBM

		HDC				mDC;
		HGLRC			mContext;

#endif

#if LIN
	glWidget*		mGlWidget;
public:
	bool			mInited;
#endif

};

#endif
