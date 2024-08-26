/*
 * Copyright (c) 2007, Laminar Research.
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
#include <FL/Fl.H>
#include <FL/filename.H>
#include "AssertUtils.h"

//TODO: mroe: this is a workaround for a bug in KWIN wayland with DnD and subwindow
// fltk issue #997
//
#ifdef FLTK_USE_WAYLAND
#include <FL/platform.H>
#include <wayland-client.h>

void prepare_widget_for_dnd(Fl_Widget *wdgt)
{
  if (!fl_wl_display()) return;
  if (!wdgt->as_window()->parent()) return;
  struct wld_window *xid = fl_wl_xid(wdgt->as_window());
  if (!xid) return;
  struct wl_region *input = wl_compositor_create_region(fl_wl_compositor());
  wl_region_add(input, 0, 0, 1000000, 1000000);
  float s = Fl::screen_scale(wdgt->as_window()->screen_num()* fl_wl_buffer_scale(wdgt->as_window()));
  wl_region_subtract(input, wdgt->x()*s, wdgt->y()*s, wdgt->w()*s, wdgt->h()*s);
  wl_surface_set_input_region(fl_wl_surface(xid), input);
  wl_region_destroy(input);
}
#endif

glWidget::glWidget(XWinGL* xwin,int w,int h,Fl_Gl_Window* share) : Fl_Gl_Window(w,h)
{
	//TODO: mroe:
	//Probably we make a own context here for new windows not sharing context.
	//Having one before a window is shown  would also avoid all the deferred gl init.
	if(share)
		mSharedContext = share->context();
	else
		mSharedContext = nullptr;

	mXWinGL = xwin;
	set_visible();
}

glWidget::~glWidget()
{}

void glWidget::draw()
{
    if (!this->context_valid())
	{
        if(mSharedContext != nullptr)
        {
            //mroe: fltk destroys the context after hidden and would get a new one due make_current() later.
            //thats why,if the context was shared we restore it here.
            this->context(mSharedContext,0);
            mXWinGL->mCtxValid = true;
        }
        else
        {
            mXWinGL->mCtxValid = false;
            glPixelStorei	(GL_UNPACK_ALIGNMENT,1);
            glPixelStorei	(GL_PACK_ALIGNMENT  ,1);

            if(!mXWinGL->mGLInited)
            {
                GLenum err = glewInit();
                #ifdef FLTK_USE_WAYLAND
                    if (fl_wl_display() && err == GLEW_ERROR_NO_GLX_DISPLAY) err = GLEW_OK;
                #endif
                if(err)
                {
                    LOG_MSG("I/WGL glewInit failed\n"); LOG_FLUSH();
                    throw runtime_error("can't init glew");
                }
                else
                {
                    mXWinGL->mGLInited = true;
                    LOG_MSG("I/WGL glewInit OK\n");
                }
            }
        }
    }
	else
	{
		mXWinGL->mCtxValid = true;
	}

	mXWinGL->GLDraw();
}

XWinGL::XWinGL(int default_dnd, XWinGL* inShare) : XWin(default_dnd), mGLInited(false), mCtxValid(false)
{
	mGlWidget = new glWidget(this, 100,100,inShare?inShare->mGlWidget:0);
	add_resizable(*mGlWidget);

	if(inShare) mGLInited = true;
}

XWinGL::XWinGL(int default_dnd, const char * inTitle, int inAttributes, int inX, int inY, int inWidth, int inHeight, XWinGL * inShare) : XWin(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight), mGLInited(false),mCtxValid(false)
{
	mGlWidget = new glWidget(this,inWidth,inHeight,inShare?inShare->mGlWidget:0);
	if(inAttributes & xwin_style_resizable)
		add_resizable(*mGlWidget);
	else
		add(*mGlWidget);

	if(inShare) mGLInited = true;
}

XWinGL::~XWinGL()
{
}

void XWinGL::Resized(int w, int h)
{
#ifdef FLTK_USE_WAYLAND
	prepare_widget_for_dnd(mGlWidget);
#endif
	GLReshaped(w,h);
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
