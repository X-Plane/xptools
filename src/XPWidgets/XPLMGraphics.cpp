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
#include <glut.h>
#endif

/*
	TODO: set initial OGL state!
			Load interface bitmap
*/

int	gProportional = 0;
int	gInterface = 1;



void                 XPLMDrawString(
                                   float *              inColorRGB,    
                                   int                  inXOffset,    
                                   int                  inYOffset,    
                                   const char *         inChar,    
                                   int *                inWordWrapWidth,    /* Can be NULL */
                                   XPLMFontID           inFontID)
{	
#if IBM
	static	bool	firstTime = true;
	static	GLuint	listBase = glGenLists(255);
	if (firstTime)
	{
		firstTime = false;

		HDC	dc = gWidgetWin->GetDC();

		SelectObject (dc, GetStockObject (SYSTEM_FONT)); 
 		wglUseFontBitmaps(dc, 0, 255, listBase);
	}
	
#endif

	XPLMSetGraphicsState(0,0,0, 0,0,  0,0);
	glColor3fv(inColorRGB);
	glRasterPos2i(inXOffset, inYOffset);
#if APL
	while (*inChar)
	{
		glutBitmapCharacter(gProportional ? GLUT_BITMAP_HELVETICA_12 : GLUT_BITMAP_8_BY_13,
			*inChar);
		++inChar;
	}
#endif

#if IBM
	glListBase (listBase); 
	glCallLists (strlen(inChar), GL_UNSIGNED_BYTE, inChar);
	glListBase (0);
#endif
	
}

void                 XPLMGetFontDimensions(
                                   XPLMFontID           inFontID,    
                                   int *                outCharWidth,    /* Can be NULL */
                                   int *                outCharHeight,    /* Can be NULL */
                                   int *                outDigitsOnly)
{
	if (outCharWidth) *outCharWidth = 8;
	if (outCharHeight) *outCharHeight = 13;
	if (outDigitsOnly) *outDigitsOnly = 0;
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
	static	int	oldFog = 0;
	static	int	oldTexUnits = 0;
	static	int	oldLights = 0;
	static	int	oldAlphaT = 0;
	static	int	oldAlphaB = 0;
	static	int	oldDepthR = 0;
	static	int	oldDepthW = 0;
	
	if (oldFog != inEnableFog)
	{
		if (inEnableFog)
			glEnable(GL_FOG);
		else
			glDisable(GL_FOG);
		oldFog = inEnableFog;
	}
	
	for (int p = 0; p < 4; ++p)
	{
		if (inNumberTexUnits > p && oldTexUnits <= p)	{ glActiveTextureARB(GL_TEXTURE0_ARB + p); glEnable (GL_TEXTURE_2D); glActiveTextureARB(GL_TEXTURE0_ARB); }
		if (inNumberTexUnits <= p && oldTexUnits > p)	{ glActiveTextureARB(GL_TEXTURE0_ARB + p); glDisable(GL_TEXTURE_2D); glActiveTextureARB(GL_TEXTURE0_ARB); }
	}	
	oldTexUnits = inNumberTexUnits;		
	
	if (oldLights != inEnableLighting)
	{
		if (inEnableLighting)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
		oldLights = inEnableLighting;
	}
	
	if (oldAlphaT != inEnableAlphaTesting)
	{
		if (inEnableAlphaTesting)
			glEnable(GL_ALPHA_TEST);
		else
			glDisable(GL_ALPHA_TEST);
		oldAlphaT = inEnableAlphaTesting;
	}
	
	if (oldAlphaB != inEnableAlphaBlending)
	{
		if (inEnableAlphaBlending)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		oldAlphaB = inEnableAlphaBlending;
	}
	
	if (oldDepthR != inEnableDepthTesting)
	{
		if (inEnableDepthTesting)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		oldDepthR = inEnableDepthTesting;
	}
	
	if (oldDepthW != inEnableDepthWriting)
	{
		if (inEnableDepthWriting)
			glDepthMask(GL_TRUE);
		else		
			glDepthMask(GL_FALSE);
		oldDepthW = inEnableDepthWriting;
	}
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