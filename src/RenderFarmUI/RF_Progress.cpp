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
#include "RF_Progress.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

#include "GUI_GraphState.h"
//#include "XPWidgetWin.h"

enum {
	kDlgLeft = 200,
	kDlgRight = 600,
	kDlgBottom = 300,
	kDlgTop = 400,

	kPrgLeft = 230,
	kPrgRight = 570,
	kPrgBottom = 320,
	kPrgTop = 340,

	kTex1X = 230,
	kTex1Y = 370,
	kTex2X = 230,
	kTex2Y = 355

};

bool	RF_ProgressFunc(
						int				inCurrentStage,
						int				inCurrentStageCount,
						const char *	inCurrentStageName,
						float			inProgress)
{
/*
	static	char	uiProgBuf[256];
	sprintf(uiProgBuf, "%d of %d - %02.lf%%", inCurrentStage + 1, inCurrentStageCount, inProgress * 100.0);
	XPLMSetGraphicsState(0, 0, 0,   0, 0,   0, 0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	glVertex2i(kDlgLeft, kDlgBottom);
	glVertex2i(kDlgLeft, kDlgTop);
	glVertex2i(kDlgRight, kDlgTop);
	glVertex2i(kDlgRight, kDlgBottom);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_LOOP);
	glVertex2i(kDlgLeft, kDlgBottom);
	glVertex2i(kDlgLeft, kDlgTop);
	glVertex2i(kDlgRight, kDlgTop);
	glVertex2i(kDlgRight, kDlgBottom);
	glEnd();

	int	progRight = kPrgLeft + ((double) (kPrgRight - kPrgLeft) * inProgress);

	glColor3f(0.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glVertex2i(kPrgLeft, kPrgBottom);
	glVertex2i(kPrgLeft, kPrgTop);
	glVertex2i(progRight, kPrgTop);
	glVertex2i(progRight, kPrgBottom);
	glEnd();

	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_LOOP);
	glVertex2i(kPrgLeft, kPrgBottom);
	glVertex2i(kPrgLeft, kPrgTop);
	glVertex2i(kPrgRight, kPrgTop);
	glVertex2i(kPrgRight, kPrgBottom);
	glEnd();

	GLfloat	col[3] = { 1.0, 1.0, 1.0 };

	XPLMDrawString(col, kTex1X, kTex1Y, inCurrentStageName, NULL, xplmFont_Basic);
	XPLMDrawString(col, kTex2X, kTex2Y, uiProgBuf, NULL, xplmFont_Basic);

	gWidgetWin->SwapBuffer();
*/
	return false;
}
