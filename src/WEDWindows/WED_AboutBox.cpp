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
#include "GUI_Resources.h"
#include "BitmapUtils.h"
#include "WED_Version.h"

static int *	SizeOfPng(const char * png)
{
	static int bounds[4];
	bounds[0] = 0; bounds[1] = 0;
	GUI_GetImageResourceSize(png, bounds+2);
	return bounds;	
}

WED_AboutBox::WED_AboutBox(GUI_Commander * cmdr) : GUI_Window("About WED", xwin_style_movable | xwin_style_centered, SizeOfPng("about.png"), cmdr)
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
	GUI_DrawCentered(state, "about.png", bounds, 0, 0, tile_sel, NULL, NULL);
	
	char buf[1024];
	
	sprintf(buf,"WED " WED_VERSION_STRING ", compiled on "__DATE__" "__TIME__);
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	
	float f = GUI_GetLineHeight(font_UI_Basic);
	
	GUI_FontDrawScaled(state, font_UI_Basic, color, 
		bounds[0],
		bounds[0] * 0.75 + bounds[2] * 0.25 - f * 0.5,
		bounds[2],
		bounds[0] * 0.75 + bounds[2] * 0.25 + f * 0.5,
		buf,buf+strlen(buf), align_Center);
	
}

int			WED_AboutBox::MouseDown(int x, int y, int button)
{
	return 1;
}

void		WED_AboutBox::MouseUp  (int x, int y, int button)
{
	Stop();
#if !IBM
	Hide();
#endif
}

void		WED_AboutBox::TimerFired(void)
{
	Stop();
#if !IBM
	Hide();
#endif
}
