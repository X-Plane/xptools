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
#include <gl.h>
	
XWinGL::XWinGL(XWinGL * inShare) :
	XWin()
{
	GLint wind_attribs[]={						// windowed hardware renderer
		AGL_RGBA,
		AGL_DOUBLEBUFFER ,
		AGL_DEPTH_SIZE   ,32,					// 32-bit depth buffer better
		AGL_ALL_RENDERERS,						// work on all cards possible, even if not 100% compliant
		AGL_NONE         };						// end-list

	GDHandle device = GetMainDevice();			// get main device, and choose a pixel format to run ONLY ON THAT DEVICE! This keeps part of screen from bleeding onto second monitor and slowing down the sim a LOT, if second monitor is not accelled!!
	AGLPixelFormat aglpixformat;				// pixel format

	aglpixformat = aglChoosePixelFormat(&device,1,wind_attribs);	// try for a windowed app first...
	if(aglpixformat == NULL) throw "no pixel format";

	mContext = aglCreateContext(aglpixformat,inShare ? inShare->mContext : NULL);
	if(mContext==NULL) throw "Create context failed";

	if(!aglSetDrawable(mContext, GetWindowPort(mWindow))) throw "can't set drawable";
	if(!aglSetCurrentContext(mContext)) throw "can't set context";

	aglDestroyPixelFormat(aglpixformat);
}		
	
XWinGL::XWinGL(const char * inTitle, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) :
	XWin(inTitle, inX, inY, inWidth, inHeight)
{
	GLint wind_attribs[]={						// windowed hardware renderer
		AGL_RGBA,
		AGL_DOUBLEBUFFER ,
		AGL_DEPTH_SIZE   ,32,					// 32-bit depth buffer better
		AGL_ALL_RENDERERS,						// work on all cards possible, even if not 100% compliant
		AGL_NONE         };						// end-list

	GDHandle device = GetMainDevice();			// get main device, and choose a pixel format to run ONLY ON THAT DEVICE! This keeps part of screen from bleeding onto second monitor and slowing down the sim a LOT, if second monitor is not accelled!!
	AGLPixelFormat aglpixformat;				// pixel format

	aglpixformat = aglChoosePixelFormat(&device,1,wind_attribs);	// try for a windowed app first...
	if(aglpixformat == NULL) throw "no pixel format";

	mContext = aglCreateContext(aglpixformat,inShare ? inShare->mContext : NULL);
	if(mContext==NULL) throw "Create context failed";

	if(!aglSetDrawable(mContext, GetWindowPort(mWindow))) throw "can't set drawable";
	if(!aglSetCurrentContext(mContext)) throw "can't set context";

	aglDestroyPixelFormat(aglpixformat);
}	

XWinGL::~XWinGL()
{
	aglDestroyContext(mContext);
}

void			XWinGL::SetGLContext(void)
{
	aglSetCurrentContext(mContext);
}

void			XWinGL::SwapBuffer(void)
{
	aglSwapBuffers(mContext);
}

void			XWinGL::Resized(int inWidth, int inHeight)
{
	aglUpdateContext(mContext);	
	aglSetCurrentContext(mContext);
	glViewport(0, 0, inWidth, inHeight);
	this->GLReshaped(inWidth, inHeight);	
}

void			XWinGL::Update(XContext ctx)
{
	aglSetCurrentContext(mContext);
	this->GLDraw();
	aglSwapBuffers(mContext);

}
