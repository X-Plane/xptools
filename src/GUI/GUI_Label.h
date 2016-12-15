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

#ifndef GUI_Label_H
#define GUI_Label_H

#include "GUI_Pane.h"

class	GUI_GraphState;

class	GUI_Label : public GUI_Pane {
public:

						 GUI_Label();
	virtual				~GUI_Label();

			/* GUI_Label supports two kinds of multiline behavior,
			one is explict. Explicit (always on) means
			descriptor text with \n's in them will make a new line.
			Implicit means that if the text is about to go beyond it's boundries
			a newline is automatically inserted.
			*/
			void		SetImplicitMultiline(bool isImplicitMultiline);
			void		SetFont(int font);
			void		SetColors(float text_color[4]);

			void		SetMargins(float l, float b, float r, float t);
	virtual	void		SetDescriptor(const string& inDesc);
	virtual	void		Draw(GUI_GraphState * state);
	
private:
		bool				mIsImplicitMulti;
		int					mFont;
		float				mMargins[4];
		float				mColorText[4];

};


#endif
