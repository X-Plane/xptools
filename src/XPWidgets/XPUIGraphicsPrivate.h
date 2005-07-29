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
#ifndef _XPUIGraphicsPrivate_h_
#define _XPUIGraphicsPrivate_h_

#include "XPWidgetDefs.h"
#include "XPUIGraphics.h"

#ifdef __cplusplus
extern "C" {
#endif

void DrawTextureCentered(
						int inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight);

void DrawTextureCenteredRotated(
						int inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight);

void DrawTextureStretched(
						int	inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight);

void DrawTextureStretchedRotated(	
								int	inXOffset,
								int inYOffset,
								int inWidth,
								int inHeight,
								int inXTextureOffset,
								int inYTextureOffset,
								int inTextureWidth,
								int inTextureHeight);

/*
 * DrawTextureRepeated
 *
 * This routine will draw a window using a section of the gen interface bitmap as a texture
 * The window will be rotated 90 degrees clockwise
 *
 */
void DrawTextureRepeated(
						int	inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight,
						int xScalable,
						int yScalable);


void DrawTextureRepeatedRotated(
						int	inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight,
						int xScalable,
						int yScalable);

enum {

	xpColor_Bkgnd = 0,
	xpColor_MenuDarkTinge,
	xpColor_MenuHilite,
	xpColor_MenuLiteTinge,
	xpColor_MenuText,
	xpColor_MenuTextDisabled,
	xpColor_SubTitleText,
	xpColor_TabFront,
	xpColor_TabBack,
	xpColor_CaptionText,
	xpColor_ListText,
	xpColor_GlassText,
	xpColor_Count

};


void	SetupAmbientColor(int inColorID, float * outColor);
void	SetAlphaLevels(float inAlphaLevel);
void	SetProportional(int inProportional);

#ifdef __cplusplus
}
#endif

#endif
