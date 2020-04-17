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

#if APL
	#include <OpenGL/gl.h>
	#if __OBJC__
		#import <AppKit/AppKit.h>
	#else
		typedef void NSOpenGLView;
	#endif
#elif IBM
	#include "glew.h"
#else  // LIN
	#include <FL/Fl_Gl_Window.H>
	#include <glew.h>

class	XWinGL;

class glWidget : public Fl_Gl_Window
{

public:
	glWidget(XWinGL* xwin,int w,int h,Fl_Gl_Window* share);
	virtual ~glWidget(void);
	void draw();

protected:

    void resize(int X,int Y,int W,int H);

private:
	XWinGL* mXWinGL;
};

#endif

#include "XWin.h"

#if 0 // IBM
	// Ben says: glext.h doesn't come with stock MSVC.  Here we include the few extensions we gotta have by hand.  Maybe someday we'll use GLEW.
    // Michaels answer: #define someday Decemer 13 2019

   typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC    )(GLenum,GLfloat,GLfloat);
   typedef void (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC   )(GLenum,const GLfloat *);
   typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC      )(GLenum                );
   typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture        );
   typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid* data);

   extern PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB	;
   extern PFNGLMULTITEXCOORD2FVARBPROC    glMultiTexCoord2fvARB;
   extern PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB	;
   extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
   extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC	glCompressedTexImage2DARB	;

#define GL_TEXTURE0_ARB                   0x84C0
#define GL_TEXTURE1_ARB                   0x84C1
#define GL_TEXTURE2_ARB                   0x84C2
#define GL_TEXTURE3_ARB                   0x84C3
#define GL_TEXTURE4_ARB                   0x84C4
#define GL_TEXTURE5_ARB                   0x84C5
#define GL_TEXTURE6_ARB                   0x84C6
#define GL_TEXTURE7_ARB                   0x84C7
#define GL_TEXTURE8_ARB                   0x84C8
#define GL_TEXTURE9_ARB                   0x84C9
#define GL_TEXTURE10_ARB                  0x84CA
#define GL_TEXTURE11_ARB                  0x84CB
#define GL_TEXTURE12_ARB                  0x84CC
#define GL_TEXTURE13_ARB                  0x84CD
#define GL_TEXTURE14_ARB                  0x84CE
#define GL_TEXTURE15_ARB                  0x84CF
#define GL_TEXTURE16_ARB                  0x84D0
#define GL_TEXTURE17_ARB                  0x84D1
#define GL_TEXTURE18_ARB                  0x84D2
#define GL_TEXTURE19_ARB                  0x84D3
#define GL_TEXTURE20_ARB                  0x84D4
#define GL_TEXTURE21_ARB                  0x84D5
#define GL_TEXTURE22_ARB                  0x84D6
#define GL_TEXTURE23_ARB                  0x84D7
#define GL_TEXTURE24_ARB                  0x84D8
#define GL_TEXTURE25_ARB                  0x84D9
#define GL_TEXTURE26_ARB                  0x84DA
#define GL_TEXTURE27_ARB                  0x84DB
#define GL_TEXTURE28_ARB                  0x84DC
#define GL_TEXTURE29_ARB                  0x84DD
#define GL_TEXTURE30_ARB                  0x84DE
#define GL_TEXTURE31_ARB                  0x84DF
#define GL_ACTIVE_TEXTURE_ARB             0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB      0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB          0x84E2

#define GL_COMPRESSED_RGB                 0x84ED
#define GL_COMPRESSED_RGBA                0x84EE
#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3

#define GL_CLAMP_TO_EDGE                  0x812F

#endif


class	XWinGL : public XWin
{
public:

	XWinGL(int default_dnd, XWinGL * inShare);
	XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare);

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
