/* 
 * Copyright (c) 2004, Ben Supnik and Sandy Barbour.
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
#include "XPLMGraphics.h"
#include "XWinGL.h"
#include "XPWidgetWin.h"

#if APL
#include <OpenGL/glext.h>
#endif

/*
	TODO: set initial OGL state!
			Load interface bitmap
*/

int	gProportional = 0;
int	gInterface = 1;

#include "FontMgr.h"

#define UI_FONT_SIZE 9
static FontMgr * 		sMgr = NULL;
static FontHandle		sFont = NULL;
static void MyGenerateTextures(int n,			int* textures) { glGenTextures (n, (unsigned long *) textures); }
static void MyBindTexture		(int target,	int texture)   { XPLMBindTexture2d(texture,target); }
static void InitFonts(void) {
		sMgr = new FontMgr;
		FontFuncs funcs = { MyGenerateTextures, MyBindTexture };
		sMgr->InstallCallbacks(&funcs);
		sFont = sMgr->LoadFont("Courier.ttf", NULL, NULL, UI_FONT_SIZE, true);
}
		
void                 XPLMDrawString(
                                   float *              inColorRGB,    
                                   int                  inXOffset,    
                                   int                  inYOffset,    
                                   const char *         inChar,    
                                   int *                inWordWrapWidth,    /* Can be NULL */
                                   XPLMFontID           inFontID)
{
	if (sFont == NULL) InitFonts();
	GLfloat	col[4] = { inColorRGB[0],inColorRGB[1],inColorRGB[2],1.0f };
	XPLMSetGraphicsState(0,1,0, 1,1,  0,0);
	sMgr->DrawString(sFont, col, inXOffset, inYOffset,inChar);
	

/*	float desc = sMgr->GetLineDescent(sFont, UI_FONT_SIZE);
	float asc = sMgr->GetLineAscent(sFont, UI_FONT_SIZE);
	XPLMSetGraphicsState(0,0,0, 1,1,  0,0);
	glColor4f(0,1,0,0.3);
	glBegin(GL_LINES);
	glVertex2f(inXOffset, inYOffset-desc);
	glVertex2f(inXOffset+1000, inYOffset-desc);
	glVertex2f(inXOffset, inYOffset+asc);
	glVertex2f(inXOffset+1000, inYOffset+asc);
	glVertex2f(inXOffset, inYOffset);
	glVertex2f(inXOffset+1000, inYOffset);
	glEnd();
*/
}

void                 XPLMGetFontDimensions(
                                   XPLMFontID           inFontID,    
                                   int *                outCharWidth,    /* Can be NULL */
                                   int *                outCharHeight,    /* Can be NULL */
                                   int *                outDigitsOnly)
{
	if (sFont == NULL) InitFonts();
	
	if (outCharWidth) *outCharWidth = sMgr->MeasureString(sFont,sMgr->GetLineHeight(sFont,UI_FONT_SIZE),"O");
	if (outCharHeight) *outCharHeight = sMgr->GetLineHeight(sFont,UI_FONT_SIZE);
	if (outDigitsOnly) *outDigitsOnly = 0;
}	

int				XPLMMeasureString(
									const char *		inChar,
									XPLMFontID			inFont,
									int					inCount)
{
	if (sFont == NULL) InitFonts();	
	if (inCount == -1) inCount = strlen(inChar);
	return sMgr->MeasureRange(sFont,sMgr->GetLineHeight(sFont,UI_FONT_SIZE),inChar,inChar+inCount);

}

int				XPLMFitStringForward(
									const char *		inStartPtr,
									const char *		inEndPtr,
									XPLMFontID			inFont,
									int					inWidth)
{
	if (sFont == NULL) InitFonts();	
	return sMgr->FitForward(sFont, sMgr->GetLineHeight(sFont,UI_FONT_SIZE),inWidth,inStartPtr, inEndPtr);
}

int				XPLMFitStringBackward(
									const char *		inStartPtr,
									const char *		inEndPtr,
									XPLMFontID			inFont,
									int					inWidth)
{
	if (sFont == NULL) InitFonts();	
	return sMgr->FitReverse(sFont, sMgr->GetLineHeight(sFont,UI_FONT_SIZE),inWidth,inStartPtr, inEndPtr);
}


void                 XPLMBindTexture2d(
                                   int                  inTextureNum,    
                                   int                  inTextureUnit)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + inTextureUnit);
	glBindTexture(GL_TEXTURE_2D, inTextureNum);
	glActiveTextureARB(GL_TEXTURE0_ARB);	
}                                   

void                 XPLMSetGraphicsState(
                                   int                  inEnableFog,    
                                   int                  inNumberTexUnits,    
                                   int                  inEnableLighting,    
                                   int                  inEnableAlphaTesting,    
                                   int                  inEnableAlphaBlending,    
                                   int                  inEnableDepthTesting,    
                                   int                  inEnableDepthWriting)
{
	if (inEnableFog)
		glEnable(GL_FOG);
	else
		glDisable(GL_FOG);

	for (int p = 0; p < 4; ++p)
	{
		if (inNumberTexUnits > p)	{ glActiveTextureARB(GL_TEXTURE0_ARB + p); glEnable (GL_TEXTURE_2D); glActiveTextureARB(GL_TEXTURE0_ARB); }
		if (inNumberTexUnits <= p)	{ glActiveTextureARB(GL_TEXTURE0_ARB + p); glDisable(GL_TEXTURE_2D); glActiveTextureARB(GL_TEXTURE0_ARB); }
	}	

	if (inEnableLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	if (inEnableAlphaTesting)
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);

	if (inEnableAlphaBlending)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	if (inEnableDepthTesting)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (inEnableDepthWriting)
		glDepthMask(GL_TRUE);
	else		
		glDepthMask(GL_FALSE);
}                                   

void                 XPLMGenerateTextureNumbers(
                                   int *                outTextureIDs,    
                                   int                  inCount)
{
#if APL
	glGenTextures (inCount, (unsigned long *) outTextureIDs);
#endif
#if IBM
	glGenTextures (inCount, (unsigned int *) outTextureIDs);
#endif	
}                                   

int                  XPLMGetTexture(
                                   XPLMTextureID        inTexture)
{
	return gInterface;
}	                                   

void                 XPLMDrawTranslucentDarkBox(
                                   int                  inLeft,    
                                   int                  inTop,    
                                   int                  inRight,    
                                   int                  inBottom)
{
	XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);
	glColor4f(0.0, 0.0, 0.0, 0.8);
	glBegin(GL_QUADS);
	glVertex2i(inLeft, inBottom);
	glVertex2i(inLeft, inTop);
	glVertex2i(inRight, inTop);
	glVertex2i(inRight, inBottom);
	glEnd();
}