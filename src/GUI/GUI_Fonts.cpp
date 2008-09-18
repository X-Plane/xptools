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

#include "GUI_Fonts.h"
#include "FontMgr.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

static const char * kFontNames[font_Max] = {
	"Vera.ttf",
	"Vera.ttf"
//	"Arial"
};

static const int	kFontSizes[font_Max] = { 
	10,
	8,
};

static FontHandle	sFonts[font_Max] = { 0 };
static FontMgr *	sFontMgr = NULL;

static const int	kAlign[3] = { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

static void MyGenerateTextures(int n,	int* textures) { glGenTextures (n, (GLuint *) textures); }
static void MyBindTexture(int target,	int texture)   
{	 
	glBindTexture(target, texture);
}

static void	EstablishFont(int id)
{
	if (sFontMgr == NULL)
	{
		sFontMgr = new FontMgr;
		FontFuncs funcs = { MyGenerateTextures, MyBindTexture };
		sFontMgr->InstallCallbacks(&funcs);
	}	
	if (sFonts[id] == NULL)
	{
		GUI_Resource res = GUI_LoadResource(kFontNames[id]);
		if (res)
		{
			sFonts[id] = sFontMgr->LoadFont(kFontNames[id], GUI_GetResourceBegin(res), GUI_GetResourceEnd(res), kFontSizes[id], true);	
			GUI_UnloadResource(res);
		}
	}
}

void	GUI_FontDraw(	
				GUI_GraphState *				inState,
				int 							inFontID,
				float							color[4],	//	4-part color, featuring alpha.
				float							inX,
				float							inY,
				const char *					inString)
{
	EstablishFont(inFontID);
	inState->SetState(0,1,0,  1,1,  0, 0);
	
	sFontMgr->DrawString(sFonts[inFontID], color, inX, inY, inString);
}

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
				int								inAlign)
{
	EstablishFont(inFontID);
	inState->SetState(0,1,0,  1,1,  0, 0);
	
	sFontMgr->DrawRange(sFonts[inFontID], color, inLeft, inBottom, inRight, inTop, inStart, inEnd, kAlign[inAlign]);
}
float	GUI_MeasureRange(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd)
{
	EstablishFont(inFontID);
	return sFontMgr->MeasureRange(
							sFonts[inFontID], 
							sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]),
							inCharStart,inCharEnd);
}

int		GUI_FitForward(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace)
{
	EstablishFont(inFontID);
	return sFontMgr->FitForward(
							sFonts[inFontID], 
							sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]),
							inSpace, inCharStart, inCharEnd);

}

int		GUI_FitReverse(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace)
{
	EstablishFont(inFontID);
	return sFontMgr->FitReverse(
							sFonts[inFontID], 
							sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]),
							inSpace, inCharStart, inCharEnd);
}

float	GUI_GetLineHeight(int inFontID)
{
	EstablishFont(inFontID);
	return sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]);
}

float	GUI_GetLineDescent(int inFontID)
{
	EstablishFont(inFontID);
	return sFontMgr->GetLineDescent(sFonts[inFontID], kFontSizes[inFontID]);
}

float	GUI_GetLineAscent(int inFontID)
{
	EstablishFont(inFontID);
	return sFontMgr->GetLineAscent(sFonts[inFontID], kFontSizes[inFontID]);
}

void	GUI_TruncateText(
				string&							ioText,
				int								inFontID,
				float							inSpace)
{
	if (ioText.empty()) return;

	int chars = GUI_FitForward(inFontID, &*ioText.begin(), &*ioText.end(), inSpace);
	if (chars == ioText.length()) return;
	if (chars < 0) { ioText.clear(); return; }
	ioText.erase(chars);
	if (ioText.length() > 0)	ioText[ioText.length()-1] = '.';
	if (ioText.length() > 1)	ioText[ioText.length()-2] = '.';
	if (ioText.length() > 2)	ioText[ioText.length()-3] = '.';

}

