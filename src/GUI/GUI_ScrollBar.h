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

#ifndef GUI_SCROLLBAR_H
#define GUI_SCROLLBAR_H

#include "GUI_Control.h"
#include "GUI_Timer.h"

class GUI_ScrollBar : public GUI_Control, public GUI_Timer {
public:
						 GUI_ScrollBar();
						~GUI_ScrollBar();

	// From GUI_Pane
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp(int x, int y, int button);
	virtual	void		Draw(GUI_GraphState * state);

	virtual	void		TimerFired(void);

	// From GUI_Control
	virtual	void		SetValue(float inValue);
	virtual	void		SetMin(float inMin);
	virtual	void		SetMax(float inMax);
	virtual	void		SetPageSize(float inPageSize);

			int			GetMinorAxis(int vertical);

private:

		int		mClickPart;
		int		mInPart;
		float	mSlop;

};

#endif
