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

#ifndef GUI_FONTS_H
#define GUI_FONTS_H

class	GUI_GraphState;
enum {

	font_UI_Basic = 0,
	font_UI_Small,
	font_Max,

	align_Left = 0,
	align_Center,
	align_Right

};

void	GUI_FontDraw(
				GUI_GraphState *				inState,
				int 							inFontID,
				float							color[4],	//	4-part color, featuring alpha.
				float							inX,
				float							inY,
				const char *					inString);

void	GUI_FontDrawScaled(
				GUI_GraphState *				inState,
				int 							inFontID,
				float							color[4],	//	4-part color, featuring alpha.
				float							inLeft,
				float							inBottom,
				float							inRight,
				float							inTop,
				const char *					inStart,
				const char *					inEnd,
				int								inAlign);

float	GUI_MeasureRange(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd);
int		GUI_FitForward(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace);

int		GUI_FitReverse(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace);

float	GUI_GetLineHeight(int inFontID);
float	GUI_GetLineDescent(int inFontID);
float	GUI_GetLineAscent(int inFontID);

void	GUI_TruncateText(
				string&							ioText,
				int								inFontID,
				float							inSpace);

#endif
