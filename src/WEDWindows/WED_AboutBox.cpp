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

#include "WED_AboutBox.h"
#include "GUI_DrawUtils.h"
#include "GUI_Fonts.h"
#include "WED_Colors.h"
#include "WED_Globals.h"
#include "GUI_Resources.h"
#include "BitmapUtils.h"
#include "WED_Version.h"

static int aboutbox_bounds[4] = { 0, 0, 512, 384};

WED_AboutBox::WED_AboutBox(GUI_Commander * cmdr) : GUI_Window("About WED", xwin_style_movable | xwin_style_centered, aboutbox_bounds, cmdr)
{
}

WED_AboutBox::~WED_AboutBox()
{
}

bool	WED_AboutBox::Closed(void)
{
	Hide();
	Stop();
    return false;
}

void		WED_AboutBox::Draw(GUI_GraphState * state)
{
	int bounds[4];
	int tile_sel[4] = { 0, 0, 1, 1 };
	GUI_Pane::GetBounds(bounds);
	GUI_DrawStretched(state, "about.png", bounds, tile_sel);

	int font = gFontSize > 14 ? font_UI_Small : font_UI_Basic;

	float f = GUI_GetLineHeight(font);
	float * color = WED_Color_RGBA(wed_Table_Text);

	const char * main_text[] = {
		"WorldEditor " WED_VERSION_STRING_SHORT,
		"© Copyright 2007-2019, Laminar Research",
		"",
		"This software is available under an open",
		"license. For more info visit",
		"http://developer.x-plane.com/code/",
		"",
		"OSM data available under CC-BY-SA 2.0",
		"license by the OpenStreetMap foundation",
		"http://www.openstreetmap.org/copyright",
		"",
		"The World Imagery is provided by ESRI",
		"For more detail see 'Permitted Uses' in the",
		"Help menu or visit http://www.esri.com",
		0 };

	int n = 0;
	while(main_text[n])
	{
		GUI_FontDraw(state, font, color, bounds[0] * 0.6 + bounds[2] * 0.4, bounds[3] - 30 - f*n, main_text[n]);
		++n;
	}

	const char * info = "WorldEditor " WED_VERSION_STRING ", compiled on " __DATE__ " " __TIME__;

	GUI_FontDraw(state, font, color,
		(bounds[0] + bounds[2]) * 0.5,
		bounds[1] * 0.7 + bounds[3] * 0.3,
		info, align_Center);

	// Don't like how you have been acredited (or NOT acredited)?  FIX IT!  Change the code
	// here, submit a patch, or file a bug report.

	const char * credits[] = {
		//Sorted alphabetically, no exceptions
		"Thanks to Ben Supnik, Ted Greene, Janos Laube, Christiano Maggi,",
		"Michael Minnhaar, Mathias Roedel, Tyler Young, Martin Boehme",
		"and everyone else who has contributed to WorldEditor's development.",
		0 };
	n = 0;
	while(credits[n])
	{
		GUI_FontDraw(
			state,font,color,
			(bounds[0] + bounds[2]) * 0.5,
			bounds[1] * 0.8 + bounds[3] * 0.2 - n * f,
			credits[n],align_Center);
		++n;
	}

}

int			WED_AboutBox::MouseDown(int x, int y, int button)
{
	return 1;
}

void		WED_AboutBox::MouseUp  (int x, int y, int button)
{
	Stop();
	Hide();
}

void		WED_AboutBox::TimerFired(void)
{
	Stop();
	Hide();
}
