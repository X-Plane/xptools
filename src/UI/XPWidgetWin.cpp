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
#include "XPWidgetWin.h"
#include "XPLMPrivate.h"
#include "XPLMGraphics.h"
#include "XPLMDisplay.h"
#include "XPLMProcessing.h"
#include "XWidgetApp.h"

XPWidgetWin* gWidgetWin = NULL;

extern	int	DispatchMenuCmd(xmenu, int);

extern unsigned char * GetInterfaceBits(void);

XPWidgetWin::XPWidgetWin() :
	XWinGL(1, NULL)
{
	SetTimerInterval(0.01);

	SetGLContext();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glDisable(GL_DEPTH_TEST);
	glAlphaFunc		(GL_GREATER,0						);	// when ALPHA-TESTING  is enabled, we display the GREATEST alpha
	glBlendFunc		(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	// when ALPHA-BLENDING is enabled, we blend normally
	glDepthFunc		(GL_LEQUAL							);	// less than OR EQUAL plots on top
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	XPLMBindTexture2d(XPLMGetTexture(xplm_Tex_GeneralInterface), 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, GetInterfaceBits());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

XPWidgetWin::~XPWidgetWin()
{

}

void			XPWidgetWin::Timer(void)
{
	XPLMDoFlightLoopProcessing(XPLMGetElapsedTime());
	ForceRefresh();
}

void			XPWidgetWin::GLReshaped(int inWidth, int inHeight)
{
}

void			XPWidgetWin::GLDraw(void)
{
	SetGLContext();

	int	w, h;
	GetBounds(&w, &h);
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDepthMask(GL_TRUE);
	glClearColor(0.3, 0.5, 0.6, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	XPLMDisplayDoDrawingHook();
}

bool			XPWidgetWin::Closed(void)
{
	if (!XGrindCanQuit())
		return false;
	XGrinder_Quit();
	return true;
}

void			XPWidgetWin::ClickDown(int inX, int inY, int inButton)
{
	int y;
	this->GetBounds(NULL, &y);
	XPLMDisplayMouseClickHook(inX, y - inY, xplm_MouseDown, inButton);
}

void			XPWidgetWin::ClickUp(int inX, int inY, int inButton)
{
	int y;
	this->GetBounds(NULL, &y);
	XPLMDisplayMouseClickHook(inX, y - inY, xplm_MouseUp, inButton);
}

void			XPWidgetWin::ClickDrag(int inX, int inY, int inButton)
{
	int y;
	this->GetBounds(NULL, &y);
	XPLMDisplayMouseClickHook(inX, y - inY, xplm_MouseDrag, inButton);
}

void			XPWidgetWin::MouseWheel(int inX, int inY, int inDelta, int inAxis)
{
	if (inAxis == 0)
	{
		int y;
		this->GetBounds(NULL, &y);

		XPLMDisplayMouseClickHook(inX, y - inY, xplm_MouseWheel, inDelta);
	}
}

void			XPWidgetWin::DragEnter(int x, int y)
{
	XGrindDragStart(x, y);
}

void			XPWidgetWin::DragOver(int x, int y)
{
	XGrindDragOver(x, y);
}

void			XPWidgetWin::DragLeave(void)
{
	XGrindDragLeave();
}

void			XPWidgetWin::ReceiveFiles(const vector<string>& inFiles, int x, int y)
{
	XGrindFiles(inFiles, x, y);
}

int			XPWidgetWin::KeyPressed(uint32_t inKey, long msg, long param1, long param2)
{
	// Conventions: XPLM Returns 1 if key is for X-Plane.
	// XWin expects 0 if not used.
	return !XPLMDisplayKeyPressHook(msg, param1, param2);
}

int		XPWidgetWin::HandleMenuCmd(xmenu inMenu, int inCommand)
{
	return DispatchMenuCmd(inMenu, inCommand);
}
