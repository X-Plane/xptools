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
#include "GUI_Resources.h"

int kDefaultBounds[4] = { 256, 192, 768, 576 };

WED_AboutBox::WED_AboutBox(GUI_Commander * cmdr) : GUI_Window("About WED", kDefaultBounds, cmdr)
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
