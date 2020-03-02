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
#include "XWinGL.h"

XWinGL::XWinGL(int default_dnd, XWinGL * inShare) :
	XWin(default_dnd)
{
	mDC = ::GetDC(mWindow);

		PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd,0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize     =sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion  =1;
	pfd.dwFlags   =PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.iPixelType=PFD_TYPE_RGBA;
	pfd.cColorBits=32;
	pfd.cDepthBits=32;
	pfd.iLayerType=PFD_MAIN_PLANE;

	int pixelFormat = ChoosePixelFormat(mDC, &pfd);
	if (pixelFormat == 0)
		throw "no pixel format";

	if (!SetPixelFormat (mDC, pixelFormat, &pfd))
		throw "can't set pixel format";

	mContext = wglCreateContext(mDC);
	if (!mContext)
		throw "can't make context";

   if (inShare)
	   if (!wglShareLists(inShare->mContext,mContext))
		   throw "Can't share lists";

	if (!wglMakeCurrent(mDC, mContext))
		throw "can't set context";

   glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
   glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);
}


XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) :
	XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight)
{
	mDC = ::GetDC(mWindow);

		PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd,0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize     =sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion  =1;
	pfd.dwFlags   =PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.iPixelType=PFD_TYPE_RGBA;
	pfd.cColorBits=32;
	pfd.cDepthBits=32;
	pfd.iLayerType=PFD_MAIN_PLANE;

	int pixelFormat = ChoosePixelFormat(mDC, &pfd);
	if (pixelFormat == 0)
		throw "no pixel format";

	if (!SetPixelFormat (mDC, pixelFormat, &pfd))
		throw "can't set pixel format";

	mContext = wglCreateContext(mDC);
	if (!mContext)
		throw "can't make context";

   if (inShare)
	   if (!wglShareLists(inShare->mContext,mContext))
		   throw "Can't share lists";

	if (!wglMakeCurrent(mDC, mContext))
		throw "can't set context";

   glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
   glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);
}

XWinGL::~XWinGL()
{
	wglDeleteContext(mContext);
	ReleaseDC(mWindow, mDC);
}

void			XWinGL::SetGLContext(void)
{
	wglMakeCurrent(mDC, mContext);
}

void			XWinGL::SwapBuffer(void)
{
	SwapBuffers(mDC);
}

HDC				XWinGL::GetDC(void)
{
	return mDC;
}

void			XWinGL::Resized(int inWidth, int inHeight)
{
	wglMakeCurrent(mDC, mContext);
	glViewport(0, 0, inWidth, inHeight);
	this->GLReshaped(inWidth, inHeight);
}

void			XWinGL::Update(XContext ctx)
{
	wglMakeCurrent(mDC, mContext);
	this->GLDraw();
	SwapBuffers(mDC);
}
