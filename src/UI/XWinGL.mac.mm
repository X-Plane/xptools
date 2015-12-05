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
#if APL
	#include <OpenGL/gl.h>
	#import <AppKit/AppKit.h>
#else
	#include <gl.h>
#endif

@interface	my_glview : NSOpenGLView {
@private
	XWinGL *			mOwner;
}

- (void) setOwner:(XWinGL *)owner;
@end

@implementation my_glview
- (void) setOwner:(XWinGL *)owner
{
	mOwner = owner;
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(mOwner)
	{
		mOwner->SetGLContext();
		if(!mOwner->mInInit)
		{
			mOwner->GLDraw();
			mOwner->SwapBuffer();
		}
	}
}

@end



XWinGL::XWinGL(int default_dnd, XWinGL * inShare) :
	XWin(default_dnd)
{
	mInInit = 1;
	NSOpenGLPixelFormatAttribute attribs[] = {
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize,		32,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAllRenderers,
		0 };

	
	NSOpenGLPixelFormat * pf = inShare ? [inShare->mContext pixelFormat] : NULL;

	if(pf == NULL)
		pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] autorelease];

	NSView * parent = [mWindow contentView];
	my_glview * gl_view = [[[my_glview alloc] initWithFrame:[parent frame] pixelFormat:pf] autorelease];

	// If our context is shared, we go throw out the context NS made an replace it with one with resource sharing.
	if(inShare)
	{
		NSOpenGLContext * shared = [[NSOpenGLContext alloc]initWithFormat:pf shareContext:[inShare->mContext openGLContext]];
		
		[gl_view setOpenGLContext:shared];
		[shared release];
	}
	[gl_view setOwner:this];
	
	[gl_view setAutoresizingMask:NSViewHeightSizable|NSViewWidthSizable];
	
	[parent addSubview:gl_view];
	
	[[gl_view openGLContext] makeCurrentContext];
	
	mContext = gl_view;
	
	if([mWindow isVisible])
		[mWindow display];
	
	glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
	glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);
	
	this->ForceRefresh();
	mInInit = 0;
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) :
	XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight)
{
	mInInit = 1;
	NSOpenGLPixelFormatAttribute attribs[] = {
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize,		32,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAllRenderers,
		0 };

	
	NSOpenGLPixelFormat * pf = inShare ? [inShare->mContext pixelFormat] : NULL;

	if(pf == NULL)
		pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] autorelease];

	NSView * parent = [mWindow contentView];
	my_glview * gl_view = [[[my_glview alloc] initWithFrame:[parent frame] pixelFormat:pf] autorelease];

	if(inShare)
	{
		NSOpenGLContext * shared = [[NSOpenGLContext alloc]initWithFormat:pf shareContext:[inShare->mContext openGLContext]];
		
		[gl_view setOpenGLContext:shared];
		[shared release];
	}

	[gl_view setOwner:this];
	
	[[gl_view openGLContext] makeCurrentContext];
	
	mContext = gl_view;

	[gl_view setAutoresizingMask:NSViewHeightSizable|NSViewWidthSizable];

	[parent addSubview:gl_view];

	if([mWindow isVisible])
		[mWindow display];
	
	glPixelStorei	(GL_UNPACK_ALIGNMENT,1				);
	glPixelStorei	(GL_PACK_ALIGNMENT  ,1				);	

	this->ForceRefresh();
	mInInit = 0;
}

XWinGL::~XWinGL()
{
	my_glview * my_view = (my_glview *) mContext;
	
	[my_view setOwner:NULL];
}

void			XWinGL::SetGLContext(void)
{
	[[mContext openGLContext] makeCurrentContext];
}

void			XWinGL::SwapBuffer(void)
{
	[[mContext openGLContext] makeCurrentContext];
	glSwapAPPLE();
}

void			XWinGL::Resized(int inWidth, int inHeight)
{
	[[mContext openGLContext] makeCurrentContext];
	glViewport(0, 0, inWidth, inHeight);
	this->GLReshaped(inWidth, inHeight);
	this->GLDraw();
	glSwapAPPLE();
}

void			XWinGL::Update(XContext ctx)
{
	[[mContext openGLContext] makeCurrentContext];
	this->GLDraw();
	glSwapAPPLE();

}
