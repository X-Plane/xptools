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
// XPUIGraphics.cpp : Defines the entry point for the DLL application.
//

#if IBM
#include <windows.h>
#include <gl/gl.h>										// Header File For The OpenGL32 Library
#include <gl/glu.h>										// Header File For The GLu32 Library
#include <gl/glaux.h>
#elif APL
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <gl.h>
#include <glu.h>
#endif

#include "XPUIGraphics.h"
#include "XPUIGraphicsPrivate.h"
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern int	gProportional;

#if IBM
extern double round(double inValue);
#endif

// Slice Hacking.  Here's the deal: Sergio made the new drag bars repeat every 8 pixels...I liked 
// the old ones better but he is the artist.  Anyway, the normal slicing code doesn't handle
// tesolation terribly well.  In the long term we need a good solution (like storing the slices
// in a table so they can be specifically hand-sliced), but for now this variable, when non-zero,
// allows client code to hand-program the slices.  Since the bar is 82 pixels wide, a 32-pixel
// slice is just about perfect...even tiling and a third of the area.
static	int		gHackSliceX = 0;

static	float	gColors[][3] = {
	{ 0.266, 0.333, 0.411 },			// Background rgb
	{ 0.380, 0.380, 0.360 },			// Menu dark
	{ 0.090, 0.156, 0.235 },			// Menu hilite
	{ 0.556, 0.560, 0.545 },			// Menu Lite
	{ 1.000, 1.000, 1.000 },			// Menu text
	{ 0.690, 0.690, 0.680 },			// Menu text disabled
	{ 0.945, 0.945, 0.945 },			// Subtitle text
	{ 0.090, 0.156, 0.235 },			// Tab front
	{ 0.439, 0.462, 0.501 },			// Tab back
	{ 0.192, 0.223, 0.278 },			// Caption text
	{ 0.215, 0.235, 0.270 },			// List text
	{ 0.564, 0.886, 0.733 }				// Glass text
};	

static float		gAlphaLevel = 1.0;

void	SetupAmbientColor(int inColorID, float * outColor)
{
	float	theColor[4];
	float * target = outColor ? outColor : theColor;
	
	memcpy(target, gColors[inColorID], sizeof(float) * 3);
	if (!outColor)
	{
		theColor[3] = gAlphaLevel;
		glColor4fv(theColor);
	}
}

void	SetAlphaLevels(float inAlphaLevel)
{
	gAlphaLevel = inAlphaLevel;
}

void	SetProportional(int inProportional)
{
	gProportional = inProportional;
}

/*
 * The general convention for these bitmaps:
 *
 * Each window has a 'wide' and a 'narrow' rectangle.  The wide rectangle is defined as the entire area surrounding
 * each UI element up to (but not including) the box drawn by Sergio.  The narrow box includes only the pixels of
 * the main widget, not shadows.  These are the ones users would expect to have do something.
 *
 * The first rectangle is usually the wide rectangle in left, bottom, width, height notation.  Negative widths or
 * heights notate scalable elements.  
 *
 * The second set of values are 'insets', the difference between the wide and narrow box, as a distance from the wide
 * to the narrow box, bottom, left, right, then top.  These numbers are always non-negative ast he narrow rectangle
 * is always inside the wide rectangle.
 *
 */

/*************************************************************************************
 * Window offsets.  
 *************************************************************************************/
 
int WindowOffsetTable[6][8] = {	
	// Left, bottom, width, height, Left inset, bot inset, right inset, top inset
	{304, 358, -100, -100,   9, 14,  9, 4},			//	xpWindow_SolidDkBlue				FIXED (help window)
	{1,   358, -100, -100,   9, 14,  9, 22},         //	xpWindow_DkBlueWithMetalFrame		FIXED (main window)
	{203, 358, -100, -100,	 9, 14,  9, 4},         //	xpWindow_MetalWithDkBlueDropShadow	FIXED (sub window)
	{256, 384, -128, -128,   0,  0,  0, 0},         //	xpWindow_DkMetalInsetMetal			WILL BE DEPRECIATED
	{405, 358, -100, -100,	 9, 14,  9, 4},         //	xpWindow_RoundedBlueMetalShadow		FIXED (glass screen)
	{1,   289, -68,  -68,    2,  2,  2, 2}          //	xpWindow_MonitorMetalBorder			FIXED (scrolling list bkgnd)
};


/*************************************************************************************
 * Element Offsets
 *************************************************************************************/

// Depreciated = will be dropped from API, so doesn't need to be fixed
// Ben = Ben will Fix
// Sandy = Sandy please fix this one
// Fixed = already fixed.


// 	Left bottom, width, height, left inset, bottom inset, right inset, top inset, whether it can be lit.
int ElementOffsetTable[66][9] = {
/*00*/	{260, 362,  -18,  -18,	 	0, 0, 0, 0,		0},	 //  xpElement_BlueSquare						DEPRECIATED
/*01*/	{327, 360,  -45,   22,	 	0, 0, 0, 0,		0},  //  xpElement_Slider							DEPRECIATED
/*02*/	{373, 360,  -22,  -20,	 	0, 0, 0, 0,		0},  //  xpElement_NumberField						DEPRECIATED
/*03*/	{397, 336,   22,   22,	 	0, 0, 0, 0,		1},  //  xpElement_SmallRadioButton					DEPRECIATED
/*04*/	{397, 359,   22,   22,	 	0, 0, 0, 0,		0},  //  xpElement_SmallRadioButtonLit				DEPRECIATED
/*05*/	{419, 338,   22,   43,	 	0, 0, 0, 0,		0},  //  xpElement_NumberFieldWithArrows			DEPRECIATEED
/*06*/	{262, 133,  -51,   26,	 	2, 6, 2, 5,		0},  //  xpElement_NumberFieldSmall					Ben Fixed
/*07*/	{465, 337,   22,   22,	 	0, 0, 0, 0,		1},  //  xpElement_SliderButton						DEPRECIATED
/*08*/	{465, 361,   22,   22,	 	0, 0, 0, 0,		0},  //  xpElement_SliderButtonLit					DEPRECIATED
/*09*/  {139, 178,   18,   18,	 	3, 3, 3, 3,		1},  //  xpElement_LargeRadioButton					Ben Fixed
/*10*/	{139, 197,   18,   18,	 	3, 3, 3, 3,		0},  //  xpElement_LargeRadioButtonLit				Ben Fixed
/*11*/	{326, 337,   23,   23,	 	0, 0, 0, 0,		0},  //  xpElement_Pointer							DEPRECIATED
/*12*/	{349, 337,   23,   23,	 	0, 0, 0, 0,		0},  //  xpElement_RightArrow						DEPRECIATED
/*13*/	{372, 337,   23,   23,	 	0, 0, 0, 0,		0},  //  xpElement_DoubleUpDownArrows				DEPRECIATED
/*14*/	{120, 178,   18,   18,	 	0, 0, 0, 0,		1},  //  xpElement_WindowCloseBox					Ben Fixed
/*15*/	{120, 197,   18,   18,	 	0, 0, 0, 0,		0},  //  xpElement_WindowCloseBoxPressed			Ben Fixed
/*16*/	{304, 486,  -52,   26,	 	4, 5, 4, 6,		1},  //  xpElement_PushButton						Ben Fixed	
/*17*/	{304, 459,  -52,   26,	 	4, 5, 4, 6,		0},  //  xpElement_PushButtonLit					Ben Fixed
/*18*/	{464, 292,   22,   22,	 	0, 0, 0, 0,		1},  //  xpElement_ScrollBarButton					DEPRECIATED
/*19*/	{464, 315,   22,   22,	 	0, 0, 0, 0,		0},  //  xpElement_ScrollBarButtonLit				DEPRECIATED
/*20*/	{495, 291,    9,   22,	 	0, 0, 0, 0,		0},  //  xpElement_ProgressIndicator				DEPRECIATED
/*21*/	{487, 314,    9,   22,	 	0, 0, 0, 0,		0},  //  xpElement_ProgressIndicatorYellow			DEPRECIATED
/*22*/	{495, 314,    9,   22,	 	0, 0, 0, 0,		0},  //  xpElement_ProgressIndicatorOrange			DEPRECIATED
/*23*/	{503, 314,    9,   22,	 	0, 0, 0, 0,		0},  //  xpElement_ProgressIndicatorGreen			DEPRECIATED
/*24*/	{448, 255,   17,   17,	 	3, 1, 2, 2,		0},  //  xpElement_OilPlatform						Sandy fixed
/*25*/	{448, 237,   17,   17,	 	4, 4, 5, 5,		0},  //  xpElement_OilPlatformSmall					Sandy fixed
/*26*/	{484, 237,   17,   53,	 	5,13, 6,18,		0},  //  xpElement_Ship								Sandy fixed
/*27*/	{396, 237,   17,   17,	 	4, 5, 4, 6,		0},  //  xpElement_ILSGlideScope					Sandy fixed
/*28*/	{414, 273,   16,   17,	 	5, 4, 5, 6,		0},  //  xpElement_MarkerLeft						Sandy fixed
/*29*/	{378, 237,   17,   17,	 	1, 1, 1, 1,		0},  //  xpElement_Airport							Sandy fixed
/*30*/	{378, 219,   17,   17,	 	7, 7, 7, 8,		0},  //  xpElement_Fix								Sandy fixed
/*31*/	{378, 255,   17,   17,	 	4, 4, 4, 4,		0},  //  xpElement_NDB								Sandy fixed
/*32*/	{378, 273,   17,   17,	 	1, 3, 1, 3,		0},  //  xpElement_VOR								Sandy fixed
/*33*/	{396, 219,   17,   17,	 	7, 6, 7, 7,		0},  //  xpElement_RadioTower						Sandy fixed
/*34*/	{466, 237,   17,   53,	 	4, 4, 4, 6,		0},  //  xpElement_AircraftCarrier					Sandy fixed
/*35*/	{448, 219,   17,   17,	 	4, 4, 4, 2,		0},  //  xpElement_Fire								Sandy fixed
/*36*/	{431, 273,   16,   17,	 	4, 4, 6, 6,		0},  //  xpElement_MarkerRight						Sandy fixed
/*37*/	{431, 237,   16,   17,	 	4, 1, 4, 3,		0},  //  xpElement_CustomObject						Sandy fixed
/*38*/	{414, 237,   16,   17,	 	4, 2, 5, 4,		0},  //  xpElement_CoolingTower						Sandy fixed
/*39*/	{431, 219,   16,   17,	 	7, 2, 8, 5,		0},  //  xpElement_SmokeStack						Sandy fixed
/*40*/	{431, 255,   16,   17,	 	5, 2, 5, 4,		0},  //  xpElement_Building							Sandy fixed
/*41*/	{414, 219,   16,   17,	 	4, 4, 4, 6,		0},  //  xpElement_PowerLine						Sandy fixed
/*42*/	{180,   0,   22,   27,	 	0, 0, 0, 0,		0},  //  xpElement_TextWindowButton					DEPRECIATED
/*43*/	{200,   4, -312,  -20,	 	0, 0, 0, 0,		0},  //  xpElement_TextWindow						DEPRECIATED
/*44*/	{180,   0, -333,   27,	 	0, 0, 0, 0,		0},  //  xpElement_TextWindowWithButton				DEPRECIATED
/*45*/	{  1,  83,   81,   20,	 	5, 2, 5, 3,		0},  //  xpElement_CopyButtons						Sandy fixed
/*46*/	{  1,  83,   81,  205,	 	3, 5, 3, 2,		0},  //  xpElement_CopyButtonsWithEditingGrid		Sandy fixed
/*47*/	{  1, 121,  -81, -167,	 	3, 3, 3, 2,		0},  //  xpElement_EditingGrid						Sandy fixed
/*48*/	{486,  26,   24,  263,	 	0, 0, 0, 0,		0},  //  xpElement_ScrollBar						DEPRECIATED
/*49*/	{306, 219,   71,   71,	 	9, 8, 8, 9,		0},  //  xpElement_VORWithCompassRose				Sandy Fixed
/*50*/	{487, 314,   25,   22,	 	0, 0, 0, 0,		0},  //  xpElement_ProgressIndicators				DEPRECIATED
/*51*/	{ 87,   0,   82,   96,	 	0, 0, 0, 0,		0},  //  xpElement_Zoomer							DEPRECIATED
/*52*/	{275, 133,  -26,  -26,	 	0, 6, 0, 5,		0},  //  xpElement_NumberFieldMiddle				Ben Fixed
/*53*/	{ 83, 178,   15,   18,	 	3, 4, 3, 4,		0},  //  xpElement_LittleDownArrow					Ben Fixed
/*54*/	{ 83, 197,   15,   18,	 	3, 4, 3, 4,		0},  //  xpElement_LittleUpArrow					Ben Fixed
/*55*/	{0  , 494, -128,   18,	 	0, 0, 0, 0,		0},  //  xpElement_MenuBarLightGray					DEPRECIATED
/*56*/	{128, 494, -128,   18,	 	0, 0, 0, 0,		0},  //  xpElement_MenuBarDarkGray					DEPRECIATED
/*57*/	{128, 348, -128,   18,	 	0, 0, 0, 0,		0},  //  xpElement_MenuBarRibbed					DEPRECIATED
/*58*/	{6,   233, -110,   4, 	 	0, 0, 0, 0,		0},  //  xpElement_Separator						DEPRECIATED
/*59*/	{300, 360,   22,  22, 	 	0, 0, 0, 0,		1},  //  xpElement_ButtonSpecial					DEPRECIATED
/*60*/	{279, 360,   22,  22, 	 	0, 0, 0, 0,		0},  //  xpElement_ButtonSpecialPressed				DEPRECIATED
/*61*/	{111, 464,  -82,  18, 	 	0, 0, 0, 0,		0},	//	xpElement_WindowDragBar						Ben Fixed
/*62*/	{10,  436,  -82,  18, 	 	0, 0, 0, 0,		0},	//	xpElement_WindowDragBarSmooth				Ben Fixed

/*63*/	{102, 486,  -100,  26, 	 	0, 0, 0, 4,		0},	//	xpElement_PopupBar						  = 63,
/*64*/	{203,  459,  100,  26, 	 	7, 6, 7, 3,		0},	//	xpElement_PopupButton					  = 64,
/*65*/	{203,  486,  100,  26, 	 	7, 6, 7, 3,		0},	//	xpElement_PopupButtonHilite			  = 65

};


/*************************************************************************************
 * TRACK BAR OFFSETS
 *************************************************************************************/
 
// This is the dimensions and slop for the actual track.  Negative dimensions mean scalable.
// left, bottom, width, height, left inset, bot inset, right inset, top inset
int	TBTrackTable[4][8] = {
	{	102, 248, 20,  -40,		4, 4,  5, 4	},	// Scroll Bar
	{	240, 242, -65, 24,		3, 12, 3, 5 },	// Slider
	{	112, 161, -28, 16,		2, 3,  2, 2 },	// Progress Bar
	{	135, 235, -41, 20,		4, 5,  4, 4 }
};

// This is the thumb for a track when it's not lit.
// NOTE: this does not currently address how the thumb is to be placed within the track in the non-expandable dimension.
int	TBThumbTable[4][8] = {
	{	83,  239, 18, 26,	4, 5, 5, 5 },	// Scroll bar
	{	240, 267, 21, 20,	5, 4, 5, 2 },	// Slider	
	{	0,   0,   0,  0,    0, 0, 0, 0 },	// Progress bar has no thumb!
	{	125, 216, 28, 18,	6, 5, 6, 4 }
};

// This is the thumb when lit, besides the wide rect and insets, it has a 'lit' color for when clicked.
// And the thumb when clicked, also RGB colorization (0-255)
int	TBThumbLitTable[4][11] = {
	{	83,  239, 18, 26,	4, 5, 5, 5,		144, 226, 187 },	// Scroll bar
	{	262, 267, 21, 20,	5, 4, 5, 2,		255, 255, 255 },	// Slider		offset will be 262, 267 for hilited
	{	0,   0,   0,  0,    0, 0, 0, 0,		255, 255, 255 },    // Progress bar has no thumb!
	{	125, 216, 28, 18,	6, 5, 6, 4,		144, 226, 187 }
};

// Fill is some gak that goes up to the thumb...for progress indicators this is the 'progress'.
// bot, left, w, h for fill.  The offsets are a correction from the tight box of the track bkgnd to the expanded box of the fill.
// To place the fill...once we know the tight bounds of the track...we calculate the tight bounds of the fill based on that.
// We then add the expansion and this expansion and then plot the fill.
// The last params are an offset to apply in the direction that doesn't stretch for the fill and a minimum to show fill.
int TBFillTable[4][10] = {
	{	0,   0,   0,   0,		4, 4,  5, 4,  0, 0 }, // Scrollbars have no fill!!
	{	288, 219, -17, 16, 		3, 12, 3, 5,  4, 6 }, // Slider
	{   83,  161, -28, 16,		2, 3,  2, 2,  0, 5 },
	{	0,   0,   0,   0,		4, 4,  5, 4,  0, 0 }
};

// Page up is pointing up for scroll bars
int TBPageUpTable[4] [8] = {
	{	83, 266, 18, 22, 			3, 3, 4, 6 },	// Scroll bar up-facing "page min" arrow
	{	0,  0,   0,  0,				0, 0, 0, 0 },	// No page btns on slider
	{	0,  0,   0,  0,				0, 0, 0, 0 },	// No page btns on prog indicator
	{	154,216, 22, 18,			3, 4, 6, 3 }
};

// STd. left, bot, width, height, insets...
int TBPageDownTable[4] [8] =  {
	{	83, 216, 18, 22, 			3, 7, 4, 1 },	// Scroll bar up-facing "page min" arrow
	{	0,  0,   0,  0,				0, 0, 0, 0 },	// No page btns on slider
	{	0,  0,   0,  0,				0, 0, 0, 0 },	// No page btns on prog indicator
	{	102,216,22, 18,				6, 4, 3, 3 }
};

// Is the natural orientation of this track vertical?
int	TBVertical[4] = { 1, 0, 0, 0 };




void XPDrawWindow(
				int				inX1,
				int				inY1,
				int				inX2,
				int				inY2,
				XPWindowStyle	inStyle)
{
	XPLMTextureID actualGenInterface;

	bool xScalable = true, yScalable = true;

	actualGenInterface = XPLMGetTexture(xplm_Tex_GeneralInterface);
	XPLMBindTexture2d(actualGenInterface, 0);
	XPLMSetGraphicsState(0,	1, 0, 0, 1, 0, 0);
	glColor4f(1.0, 1.0, 1.0, gAlphaLevel);

	// First modify the rectangle for offsets!
	inX1 -= WindowOffsetTable[inStyle][4];
	inY1 -= WindowOffsetTable[inStyle][5];
	inX2 += WindowOffsetTable[inStyle][6];
	inY2 += WindowOffsetTable[inStyle][7];

	int TempWidth = inX2 - inX1;
	int TempHeight = inY2 - inY1;

	int xTextureOffset = WindowOffsetTable[inStyle][0];
	int yTextureOffset = WindowOffsetTable[inStyle][1];
	int TextureWidth = abs(WindowOffsetTable[inStyle][2]);
	int TextureHeight = abs(WindowOffsetTable[inStyle][3]);
	int minWidth = WindowOffsetTable[inStyle][2];
	int minHeight = WindowOffsetTable[inStyle][3];

	if (minWidth > 0)	xScalable = false;	else	minWidth = -0.67 * minWidth;	
	if (minHeight > 0)	yScalable = false;	else	minHeight = -0.67 * minHeight;

	// Dimension enforcement: if an element cannot be scaled, force it's size.  (It will be aligned to the bottom
	// left corner.  If it can, it must at least be 2/3 the size of the bitmap, since we can only cut out the center of the bitmap.
	if (xScalable) {
		if (TempWidth < minWidth) TempWidth = minWidth;
	} else
		TempWidth = minWidth;

	if (yScalable) {
		if (TempHeight < minHeight) TempHeight = minHeight;
	} else
		TempHeight = minHeight;

	if ((xScalable) || (yScalable))
		DrawTextureRepeated(inX1, inY1, TempWidth, TempHeight, xTextureOffset, yTextureOffset, TextureWidth, TextureHeight, xScalable, yScalable);
	else
		DrawTextureStretched(inX1, inY1, TempWidth, TempHeight, xTextureOffset, yTextureOffset, TextureWidth, TextureHeight);
}

void XPGetWindowDefaultDimensions(
				XPWindowStyle	inStyle,
				int *			outWidth,
				int *			outHeight)
{
	if (outWidth) *outWidth = abs(WindowOffsetTable[inStyle][2]);
	if (outHeight) *outHeight = abs(WindowOffsetTable[inStyle][3]);
}

void XPDrawElement(
				int				inX1,
				int				inY1,
				int				inX2,
				int				inY2,
				XPElementStyle	inStyle,
				int				inLit)
{
	// If this element can be lit and is lit, use the next element ID!
	if (ElementOffsetTable[inStyle][8] && inLit)
		++inStyle;
		
	XPLMTextureID actualGenInterface;

	actualGenInterface = XPLMGetTexture(xplm_Tex_GeneralInterface);
	XPLMBindTexture2d(actualGenInterface, 0);
	XPLMSetGraphicsState(0,	1, 0, 0, 1, 0, 0);
	glColor4f(1.0, 1.0, 1.0, gAlphaLevel);

	inX1 -= ElementOffsetTable[inStyle][4];
	inY1 -= ElementOffsetTable[inStyle][5];
	inX2 += ElementOffsetTable[inStyle][6];
	inY2 += ElementOffsetTable[inStyle][7];
	
	int tempWidth = inX2 - inX1;
	int tempHeight = inY2 - inY1;

	int	texLeft = ElementOffsetTable[inStyle][0];
	int	texBottom = ElementOffsetTable[inStyle][1];
	int	texWidth = abs(ElementOffsetTable[inStyle][2]);
	int	texHeight = abs(ElementOffsetTable[inStyle][3]);
	int	minWidth = ElementOffsetTable[inStyle][2];
	int	minHeight = ElementOffsetTable[inStyle][3];
	bool xScalable = true, yScalable = true;

	if (minWidth > 0)	xScalable = false; else minWidth *= -0.67;
	if (minHeight > 0)	yScalable = false; else minHeight *= -0.67;

	if (xScalable && (tempWidth < minWidth)) tempWidth = minWidth;
	if (yScalable && (tempHeight < minHeight)) tempHeight = minHeight;

	/// If the texture is scalable then call texture repeating function
	/// otherwise just do normal texture draw.
	
	if (inStyle == xpElement_WindowDragBar)
		gHackSliceX = 32;
	if ((xScalable) || (yScalable))
		DrawTextureRepeated(inX1, inY1, tempWidth, tempHeight, texLeft, texBottom, texWidth, texHeight, xScalable, yScalable);
	else
		DrawTextureCentered(inX1, inY1, tempWidth, tempHeight, texLeft, texBottom, texWidth, texHeight);
	gHackSliceX = 0;
}


void XPGetElementDefaultDimensions(
				XPElementStyle	inStyle,
				int *			outWidth,
				int *			outHeight,
				int *			outCanBeLit)
{
	if (outWidth) *outWidth = abs(ElementOffsetTable[inStyle][2]) - ElementOffsetTable[inStyle][4] - ElementOffsetTable[inStyle][6];
	if (outHeight) *outHeight = abs(ElementOffsetTable[inStyle][3]) - ElementOffsetTable[inStyle][5] - ElementOffsetTable[inStyle][7];
	if (outCanBeLit) *outCanBeLit = ElementOffsetTable[inStyle][8];
}

void XPDrawTrack(
				int				inX1,
				int				inY1,
				int				inX2,
				int				inY2,
				int				inMin,
				int				inMax,
				int				inValue,
				XPTrackStyle	inTrackStyle,
				int				inLit)
{
	if (((inX2 - inX1) > (inY2 - inY1)) && (inTrackStyle == xpTrack_ScrollBar))
		inTrackStyle = xpTrack_Progress + 1;
		
	XPLMTextureID actualGenInterface = XPLMGetTexture(xplm_Tex_GeneralInterface);
	XPLMBindTexture2d(actualGenInterface, 0);
	XPLMSetGraphicsState(0,	1, 0, 0, 1, 0, 0);
	glColor4f(1.0, 1.0, 1.0, gAlphaLevel);

	int	isVertical, downBtnSize, downPageSize, thumbSize, upPageSize, upBtnSize;
	
	XPGetTrackMetrics(inX1, inY1, inX2, inY2, inMin, inMax, inValue, inTrackStyle,
		&isVertical, &downBtnSize, &downPageSize, &thumbSize, &upPageSize, &upBtnSize);
	int isRotated = isVertical != TBVertical[inTrackStyle];

	// If a thumb isn't an even number of pixels, calc two halves so we don't los a pixel!
	int	thumbBot = thumbSize / 2;
	int	thumbTop = thumbSize - thumbBot;

	// Now calc rects for five elements.
	int	trackL, trackB, trackR, trackT;
	int	downBtnL, downBtnB, downBtnR, downBtnT;
	int	upBtnL, upBtnB, upBtnR, upBtnT;
	int	thumbL, thumbB, thumbR, thumbT;
	int	fillL, fillB, fillR, fillT;
	
	if (isVertical)
	{
		thumbL = fillL = downBtnL = upBtnL = trackL = inX1;
		thumbR = fillR = downBtnR = upBtnR = trackR = inX2;
		downBtnB = inY1;
		downBtnT = fillB = trackB = inY1 + downBtnSize;
		upBtnB = trackT = inY2 - upBtnSize;
		upBtnT = inY2;
		fillT = inY1 + downBtnSize + downPageSize + thumbBot;
		thumbB = inY1 + downBtnSize + downPageSize;
		thumbT = inY2 - upBtnSize - upPageSize;
	} else {
		thumbB = fillB = downBtnB = upBtnB = trackB = inY1;
		thumbT = fillT = downBtnT = upBtnT = trackT = inY2;
		downBtnL = inX1;
		downBtnR = fillL = trackL = inX1 + downBtnSize;
		upBtnL = trackR = inX2 - upBtnSize;
		upBtnR = inX2;
		fillR = inX1 + downBtnSize + downPageSize + thumbBot;
		thumbL = inX1 + downBtnSize + downPageSize;
		thumbR = inX2 - upBtnSize - upPageSize;
	}
	
	// Now we have to 'whack' all of the rectangles to deal with expansion factors...
	if (isRotated) 
	{
		// FIX
		trackT += TBTrackTable[inTrackStyle][4];
		trackL -= TBTrackTable[inTrackStyle][5];
		trackB -= TBTrackTable[inTrackStyle][6];
		trackR += TBTrackTable[inTrackStyle][7];
		downBtnT += TBPageDownTable[inTrackStyle][4];
		downBtnL -= TBPageDownTable[inTrackStyle][5];
		downBtnB -= TBPageDownTable[inTrackStyle][6];
		downBtnR += TBPageDownTable[inTrackStyle][7];
		upBtnT += TBPageUpTable[inTrackStyle][4];
		upBtnL -= TBPageUpTable[inTrackStyle][5];
		upBtnB -= TBPageUpTable[inTrackStyle][6];
		upBtnR += TBPageUpTable[inTrackStyle][7];
		fillT += TBFillTable[inTrackStyle][4];
		fillL -= TBFillTable[inTrackStyle][5];
		fillB -= TBFillTable[inTrackStyle][6];
		fillR += TBFillTable[inTrackStyle][7];
		thumbT += (inLit ? TBThumbLitTable[inTrackStyle][4] : TBThumbTable[inTrackStyle][4]);
		thumbL -= (inLit ? TBThumbLitTable[inTrackStyle][5] : TBThumbTable[inTrackStyle][5]);
		thumbB -= (inLit ? TBThumbLitTable[inTrackStyle][6] : TBThumbTable[inTrackStyle][6]);
		thumbR += (inLit ? TBThumbLitTable[inTrackStyle][7] : TBThumbTable[inTrackStyle][7]);
	} else {
		trackL -= TBTrackTable[inTrackStyle][4];
		trackB -= TBTrackTable[inTrackStyle][5];
		trackR += TBTrackTable[inTrackStyle][6];
		trackT += TBTrackTable[inTrackStyle][7];
		downBtnL -= TBPageDownTable[inTrackStyle][4];
		downBtnB -= TBPageDownTable[inTrackStyle][5];
		downBtnR += TBPageDownTable[inTrackStyle][6];
		downBtnT += TBPageDownTable[inTrackStyle][7];
		upBtnL -= TBPageUpTable[inTrackStyle][4];
		upBtnB -= TBPageUpTable[inTrackStyle][5];
		upBtnR += TBPageUpTable[inTrackStyle][6];
		upBtnT += TBPageUpTable[inTrackStyle][7];
		fillL -= TBFillTable[inTrackStyle][4];
		fillB -= TBFillTable[inTrackStyle][5];
		fillR += TBFillTable[inTrackStyle][6];
		fillT += TBFillTable[inTrackStyle][7];
		thumbL -= (inLit ? TBThumbLitTable[inTrackStyle][4] : TBThumbTable[inTrackStyle][4]);
		thumbB -= (inLit ? TBThumbLitTable[inTrackStyle][5] : TBThumbTable[inTrackStyle][5]);
		thumbR += (inLit ? TBThumbLitTable[inTrackStyle][6] : TBThumbTable[inTrackStyle][6]);
		thumbT += (inLit ? TBThumbLitTable[inTrackStyle][7] : TBThumbTable[inTrackStyle][7]);
	}
	
	if (isVertical)
	{
		fillL += TBFillTable[inTrackStyle][8];
		fillR += TBFillTable[inTrackStyle][8];
	} else {
		fillB += TBFillTable[inTrackStyle][8];
		fillT += TBFillTable[inTrackStyle][8];
	}
	
	bool	hasFill = (TBFillTable[inTrackStyle][2] != 0) && (isVertical ? ((fillT - fillB) > TBFillTable[inTrackStyle][9]) : ((fillR - fillL) > TBFillTable[inTrackStyle][9]));
	
	if (isRotated)
	{		
		DrawTextureRepeatedRotated(trackL, trackB, trackR - trackL, trackT - trackB,
				TBTrackTable[inTrackStyle][0], TBTrackTable[inTrackStyle][1], abs(TBTrackTable[inTrackStyle][2]), abs(TBTrackTable[inTrackStyle][3]), isVertical, !isVertical);

		if (hasFill)
		{
			DrawTextureRepeatedRotated(fillL, fillB, fillR - fillL, fillT - fillB,
				TBFillTable[inTrackStyle][0], TBFillTable[inTrackStyle][1], abs(TBFillTable[inTrackStyle][2]), abs(TBFillTable[inTrackStyle][3]), isVertical, !isVertical);
		}
		if (inLit)
			glColor4f(	(float) TBThumbLitTable[inTrackStyle][8] / 255.0,
						(float) TBThumbLitTable[inTrackStyle][9] / 255.0,
						(float) TBThumbLitTable[inTrackStyle][10] / 255.0,
						gAlphaLevel);
		
		if (downBtnSize)
			DrawTextureCenteredRotated(downBtnL, downBtnB, downBtnR - downBtnL, downBtnT - downBtnB, 
				TBPageDownTable[inTrackStyle][0], TBPageDownTable[inTrackStyle][1], TBPageDownTable[inTrackStyle][2], TBPageDownTable[inTrackStyle][3]);
		if (upBtnSize)
			DrawTextureCenteredRotated(upBtnL, upBtnB, upBtnR - upBtnL, upBtnT - upBtnB, 
				TBPageUpTable[inTrackStyle][0], TBPageUpTable[inTrackStyle][1], TBPageUpTable[inTrackStyle][2], TBPageUpTable[inTrackStyle][3]);
		if (thumbSize)
		{
			if (inLit)
				DrawTextureCenteredRotated(thumbL, thumbB, thumbR - thumbL, thumbT - thumbB, 
					TBThumbLitTable[inTrackStyle][0], TBThumbLitTable[inTrackStyle][1], TBThumbLitTable[inTrackStyle][2], TBThumbLitTable[inTrackStyle][3]);
			else
				DrawTextureCenteredRotated(thumbL, thumbB, thumbR - thumbL, thumbT - thumbB, 
					TBThumbTable[inTrackStyle][0], TBThumbTable[inTrackStyle][1], TBThumbTable[inTrackStyle][2], TBThumbTable[inTrackStyle][3]);
		}
		
	} else {
		DrawTextureRepeated(trackL, trackB, trackR - trackL, trackT - trackB,
				TBTrackTable[inTrackStyle][0], TBTrackTable[inTrackStyle][1], abs(TBTrackTable[inTrackStyle][2]), abs(TBTrackTable[inTrackStyle][3]), !isVertical, isVertical);
		
		if (inLit)
			glColor4f(	(float) TBThumbLitTable[inTrackStyle][8] / 255.0,
						(float) TBThumbLitTable[inTrackStyle][9] / 255.0,
						(float) TBThumbLitTable[inTrackStyle][10] / 255.0,
						1.0);
		if (hasFill)
		{
			DrawTextureRepeated(fillL, fillB, fillR - fillL, fillT - fillB,
				TBFillTable[inTrackStyle][0], TBFillTable[inTrackStyle][1], abs(TBFillTable[inTrackStyle][2]), abs(TBFillTable[inTrackStyle][3]), !isVertical, isVertical);
		}
		
		if (downBtnSize)
			DrawTextureCentered(downBtnL, downBtnB, downBtnR - downBtnL, downBtnT - downBtnB, 
				TBPageDownTable[inTrackStyle][0], TBPageDownTable[inTrackStyle][1], TBPageDownTable[inTrackStyle][2], TBPageDownTable[inTrackStyle][3]);
		if (upBtnSize)
			DrawTextureCentered(upBtnL, upBtnB, upBtnR - upBtnL, upBtnT - upBtnB, 
				TBPageUpTable[inTrackStyle][0], TBPageUpTable[inTrackStyle][1], TBPageUpTable[inTrackStyle][2], TBPageUpTable[inTrackStyle][3]);
		if (thumbSize)
		{
			if (inLit)
				DrawTextureCentered(thumbL, thumbB, thumbR - thumbL, thumbT - thumbB, 
					TBThumbLitTable[inTrackStyle][0], TBThumbLitTable[inTrackStyle][1], TBThumbLitTable[inTrackStyle][2], TBThumbLitTable[inTrackStyle][3]);
			else
				DrawTextureCentered(thumbL, thumbB, thumbR - thumbL, thumbT - thumbB, 
					TBThumbTable[inTrackStyle][0], TBThumbTable[inTrackStyle][1], TBThumbTable[inTrackStyle][2], TBThumbTable[inTrackStyle][3]);
		}
	}
}

void XPGetTrackDefaultDimensions(
				XPTrackStyle	inStyle,
				int *			outWidth,
				int *			outCanBeLit)
{
	if (outWidth) *outWidth = 
		TBVertical[inStyle] ? 
		(TBTrackTable[inStyle][2] - TBTrackTable[inStyle][4] - TBTrackTable[inStyle][6]) :
		(TBTrackTable[inStyle][3] - TBTrackTable[inStyle][5] - TBTrackTable[inStyle][7]);
	
	if (outCanBeLit)
		*outCanBeLit = (TBThumbTable[inStyle][2] != 0) ? 1 : 0;
}

void XPGetTrackMetrics(
				int				inX1,
				int				inY1,
				int				inX2,
				int				inY2,
				int				inMin,
				int				inMax,
				int				inValue,
				XPTrackStyle	inTrackStyle,
				int *			outIsVertical,
				int *			outDownBtnSize,
				int *			outDownPageSize,
				int *			outThumbSize,
				int *			outUpPageSize,
				int *			outUpBtnSize)
{
	int width = inX2 - inX1;
	int height = inY2 - inY1;
	
	int	vertical = (height > width);
	int thumbSize = TBVertical[inTrackStyle] ? 
		(TBThumbTable[inTrackStyle][3] - TBThumbTable[inTrackStyle][5] - TBThumbTable[inTrackStyle][7]) : 
		(TBThumbTable[inTrackStyle][2] - TBThumbTable[inTrackStyle][4] - TBThumbTable[inTrackStyle][6]);
	int	upBtnSize = TBVertical[inTrackStyle] ? 
		(TBPageUpTable[inTrackStyle][3] - TBPageUpTable[inTrackStyle][5] - TBPageUpTable[inTrackStyle][7]) : 
		(TBPageUpTable[inTrackStyle][2] - TBPageUpTable[inTrackStyle][4] - TBPageUpTable[inTrackStyle][6]);
	int	downBtnSize = TBVertical[inTrackStyle] ? 
		(TBPageDownTable[inTrackStyle][3] - TBPageDownTable[inTrackStyle][5] - TBPageDownTable[inTrackStyle][7]) : 
		(TBPageDownTable[inTrackStyle][2] - TBPageDownTable[inTrackStyle][4] - TBPageDownTable[inTrackStyle][6]);
	
	int	remainingTrack = (vertical ? height : width) - upBtnSize - downBtnSize - thumbSize;
	
	int rangeDif = inMax - inMin;
	int thumbOffset = rangeDif ? (round((float) remainingTrack * float(inValue - inMin) / (float) rangeDif)) : 0;
	int thumbDec = thumbSize / 2;
	int thumbInc = thumbSize - thumbDec;
	int	pageUp = remainingTrack - thumbOffset;	// Remaining track is split by thumb offset...
	int pageDown = thumbOffset;

	if (outIsVertical	)	*outIsVertical	= vertical;
	if (outDownBtnSize	)   *outDownBtnSize	= downBtnSize;
	if (outDownPageSize	)   *outDownPageSize= pageDown;
	if (outThumbSize	)   *outThumbSize	= thumbSize;
	if (outUpPageSize	)   *outUpPageSize	= pageUp;
	if (outUpBtnSize	)   *outUpBtnSize	= upBtnSize;
}


/************************ QUAD DRAWING CODE *****************************/

void DrawTextureCentered(
						int inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight)
{
	int dx = (inWidth - inTextureWidth) / 2;
	int dy = (inHeight - inTextureHeight) / 2;
	DrawTextureStretched(inXOffset + dx, inYOffset + dy, inTextureWidth, inTextureHeight, inXTextureOffset, inYTextureOffset, inTextureWidth, inTextureHeight);	
}

void DrawTextureCenteredRotated(
						int inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight)
{
	int dx = (inWidth - inTextureHeight) / 2;
	int dy = (inHeight - inTextureWidth) / 2;
	DrawTextureStretchedRotated(inXOffset + dx, inYOffset + dy, inTextureHeight, inTextureWidth, inXTextureOffset, inYTextureOffset, inTextureWidth, inTextureHeight);	
}



void DrawTextureStretched(int inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight)
{
	glBegin(GL_QUADS);

	glTexCoord2f((float) inXTextureOffset / 512.0, (float) (inYTextureOffset + inTextureHeight) / 512.0);
	glVertex2i(inXOffset, inYOffset + inHeight);

	glTexCoord2f((float) (inXTextureOffset + inTextureWidth) / 512.0, (float) (inYTextureOffset + inTextureHeight) / 512.0);
	glVertex2i(inXOffset + inWidth, inYOffset + inHeight);

	glTexCoord2f((float) (inXTextureOffset + inTextureWidth) / 512.0, (float) inYTextureOffset / 512.0);
	glVertex2i(inXOffset + inWidth, inYOffset);

	glTexCoord2f((float) inXTextureOffset / 512.0, (float) inYTextureOffset / 512.0);
	glVertex2i(inXOffset, inYOffset);

	glEnd();
}


/// This function just rotates the texture by 90 degrees
void DrawTextureStretchedRotated(int	inXOffset,
						int inYOffset,
						int inWidth,
						int inHeight,
						int inXTextureOffset,
						int inYTextureOffset,
						int inTextureWidth,
						int inTextureHeight)
{
	glBegin(GL_QUADS);

	glTexCoord2f((float) inXTextureOffset / 512.0, (float) inYTextureOffset / 512.0);
	glVertex2i(inXOffset, inYOffset + inHeight);

	glTexCoord2f((float) inXTextureOffset / 512.0, (float) (inYTextureOffset + inTextureHeight) / 512.0);
	glVertex2i(inXOffset + inWidth, inYOffset + inHeight);

	glTexCoord2f((float) (inXTextureOffset + inTextureWidth) / 512.0, (float) (inYTextureOffset + inTextureHeight) / 512.0);
	glVertex2i(inXOffset + inWidth, inYOffset);

	glTexCoord2f((float) (inXTextureOffset + inTextureWidth) / 512.0, (float) inYTextureOffset / 512.0);
	glVertex2i(inXOffset, inYOffset);

	glEnd();
}


/// This routine basically segments the texture into nine sections
/// The corner sections are drawn once and the middle sections are repeated as required

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
						int yScalable)
{
	if (!xScalable && (inWidth != inTextureWidth))
	{
		inXOffset += ((inWidth - inTextureWidth) / 2);
		inWidth = inTextureWidth;
	}
	
	if (!yScalable && (inHeight != inTextureHeight))
	{
		inYOffset += ((inHeight - inTextureHeight) / 2);
		inHeight = inTextureHeight;
	}

	// Back to life...
	// Here's the deal with the rounding: we need our vertical slices to be multiples of 2 for windows
	// so that the pinstriping works.  Yes, we can be one pixel short, this is ok, windows should be
	// a multiple of two anyway...
	
	int	xSliceSize =  gHackSliceX ? gHackSliceX : (inTextureWidth / 3); // & (~1); // No need to mess around horizontally!
	int ySliceSize =  (inTextureHeight / 3) & (~1);
	int xSliceCount = (xScalable && (inWidth != inTextureWidth)) ? (inWidth / xSliceSize) : 1;
	int ySliceCount = (yScalable && (inHeight != inTextureHeight)) ? (inHeight / ySliceSize) : 1;
	int leftSlop = (inWidth - (xSliceCount * xSliceSize)) / 2;
	int	texLeftSlop = (inTextureWidth - (xSliceSize * 3)) / 2;
	int bottomSlop = ((inHeight - (ySliceCount * ySliceSize)) / 2) & (~1);
	int texBottomSlop = ((inTextureHeight - (ySliceSize * 3)) / 2) & (~1);
	int rightSlop = inWidth - (xSliceCount * xSliceSize) - leftSlop;
	int texRightSlop = inTextureWidth - (xSliceSize * 3) - texLeftSlop;
	int topSlop = (inHeight - (ySliceCount * ySliceSize) - bottomSlop) & (~1);
	int texTopSlop = (inTextureHeight - (3 * ySliceSize) - texBottomSlop) & (~1);
		
	int	xDraw = inXOffset;
	int yDraw = inYOffset;
	int xTex = inXTextureOffset;
	int yTex = inYTextureOffset;	
	
	for (int y = 0; y < ySliceCount; ++y)
	{
		int thisYSliceSize = ySliceSize;
		if (y == 0)
			thisYSliceSize += bottomSlop;
		if (y == (ySliceCount-1))
			thisYSliceSize += topSlop;
		
		for (int x = 0; x < xSliceCount; ++x)
		{
			int	thisXSliceSize = xSliceSize;
			if (x == 0)
				thisXSliceSize += leftSlop;
			if (x == (xSliceCount-1))
				thisXSliceSize += rightSlop;
			
			DrawTextureStretched(xDraw, yDraw, thisXSliceSize, thisYSliceSize, xTex, yTex, thisXSliceSize, thisYSliceSize);
			
			xDraw += thisXSliceSize;
			if (x == 0)
				xTex += (xSliceSize+texLeftSlop);
			if (x == (xSliceCount-2))
				xTex += (xSliceSize + texRightSlop - rightSlop);
		}
		
		xDraw = inXOffset;
		yDraw += thisYSliceSize;
		xTex = inXTextureOffset;
		if (y == 0)
			yTex += (ySliceSize+texBottomSlop);
		if (y == (ySliceCount-2))
			yTex += (ySliceSize + texTopSlop - topSlop);
	}
}


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
						int yScalable)
{
	if (!yScalable && (inWidth != inTextureHeight))
	{
		inXOffset += ((inWidth - inTextureHeight) / 2);
		inWidth = inTextureHeight;
	}
	
	if (!xScalable && (inHeight != inTextureWidth))
	{
		inYOffset += ((inHeight - inTextureWidth) / 2);
		inHeight = inTextureWidth;
	}


	// For reference: in this processing, we will view the texture coordinates as the ones that are rotated...
	// so if we are going to draw a vertical scroll bar horizontally, the number of x slices comes from the
	// texture's height.
	int	xSliceSize =  inTextureHeight / 3;
	int ySliceSize =  inTextureWidth / 3;
	int xSliceCount = (yScalable && (inWidth != inTextureHeight)) ? (inWidth / xSliceSize) : 1;
	int ySliceCount = (xScalable && (inHeight != inTextureWidth)) ? (inHeight / ySliceSize) : 1;
	int leftSlop = (inWidth - (xSliceCount * xSliceSize)) / 2;
	int	texLeftSlop = (inTextureWidth - (ySliceSize * 3)) / 2;
	int bottomSlop = (inHeight - (ySliceCount * ySliceSize)) / 2;
	int texBottomSlop = (inTextureHeight - (xSliceSize * 3)) / 2;
	int rightSlop = inWidth - (xSliceCount * xSliceSize) - leftSlop;
	int texRightSlop = inTextureWidth - (ySliceSize * 3) - texLeftSlop;
	int topSlop = inHeight - (ySliceCount * ySliceSize) - bottomSlop;
	int texTopSlop = inTextureHeight - (3 * xSliceSize) - texBottomSlop;
		
	int	xDraw = inXOffset;
	int yDraw = inYOffset;
	int xTex = inXTextureOffset+inTextureWidth;
	int yTex = inYTextureOffset;	
	
	for (int y = 0; y < ySliceCount; ++y)
	{
		int thisYSliceSize = ySliceSize;
		if (y == 0)
			thisYSliceSize += bottomSlop;
		if (y == (ySliceCount-1))
			thisYSliceSize += topSlop;
		
		for (int x = 0; x < xSliceCount; ++x)
		{
			int	thisXSliceSize = xSliceSize;
			if (x == 0)
				thisXSliceSize += leftSlop;
			if (x == (xSliceCount-1))
				thisXSliceSize += rightSlop;
			
			DrawTextureStretchedRotated(xDraw, yDraw, thisXSliceSize, thisYSliceSize, xTex - thisYSliceSize, yTex, thisYSliceSize, thisXSliceSize);
			
			xDraw += thisXSliceSize;
			if (x == 0)
				yTex += (xSliceSize+texBottomSlop);
			if (x == (xSliceCount-2))
				yTex += (xSliceSize + texTopSlop - rightSlop);
		}
		
		xDraw = inXOffset;
		yDraw += thisYSliceSize;
		yTex = inYTextureOffset;
		if (y == 0)
			xTex -= (ySliceSize+texRightSlop);
		if (y == (ySliceCount-2))
			xTex -= (ySliceSize + texLeftSlop - topSlop);
	}
}

