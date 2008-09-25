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

#ifndef GUI_TOOLBAR_H
#define GUI_TOOLBAR_H

#include "GUI_Control.h"

class	GUI_ToolBar : public GUI_Control {
public:

			 		 GUI_ToolBar(int h, int v, const char * in_resource);
	virtual			~GUI_ToolBar();

			void	SizeToBitmap(void);

			void	SetToolTips(const vector<string>& in_tips);

	virtual	void	SetValue(float inValue);


	virtual	void	Draw(GUI_GraphState * state);

	virtual	int		MouseDown(int x, int y, int button);
	virtual	void	MouseDrag(int x, int y, int button);
	virtual	void	MouseUp  (int x, int y, int button);

	virtual	int		GetHelpTip(int x, int y, int tip_bounds[4], string& tip);

private:

	vector<string>	mTips;
	string			mResource;
	int				mH;
	int				mV;

};

#endif