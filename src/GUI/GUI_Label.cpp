/* 
 * Copyright (c) 2014, Laminar Research.
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

#include "GUI_Label.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"


GUI_Label::GUI_Label() : GUI_Pane(),
	mFont(font_UI_Basic)
{
	mMargins[0] = mMargins[1] = mMargins[2] = mMargins[3] = 0.0f;
	mColorText[0] = 0.0;	mColorText[1] = 0.0;	mColorText[2] = 0.0;	mColorText[3] = 1.0;
}	

GUI_Label::~GUI_Label()
{
}

void		GUI_Label::SetFont(int font)
{
	mFont = font;
}

void		GUI_Label::SetColors(float text_color[4])
{
	for (int n = 0; n < 4; ++n)
		mColorText[n] = text_color[n];

}

void		GUI_Label::SetMargins(float l, float b, float r, float t)
{
	mMargins[0] = l;
	mMargins[1] = b;
	mMargins[2] = r;
	mMargins[3] = t;
}

void		GUI_Label::SetDescriptor(const string& inDesc)
{
	GUI_Pane::SetDescriptor(inDesc);
	GUI_Pane::Refresh();
}

void		GUI_Label::Draw(GUI_GraphState * state)
{
	string txt;
	GetDescriptor(txt);
	
	const char * tStart = txt.c_str();
	const char * tEnd = tStart + txt.size();
	
	int b[4];
	GetBounds(b);
	b[0] += mMargins[0];
	b[1] += mMargins[1];
	b[2] -= mMargins[2];
	b[3] -= mMargins[3];
	
	int x = b[0];
	int yy = b[1];
	
	GUI_FontDrawScaled(state, mFont, mColorText,
						x, yy, x + 10000, yy + GUI_GetLineHeight(mFont),
						tStart, tEnd, align_Left);
}
