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

#ifndef GUI_CONTROL_H
#define GUI_CONTROL_H

#include "GUI_Pane.h"
#include "GUI_Broadcaster.h"

class GUI_Control : public GUI_Pane, public GUI_Broadcaster {
public:

			 		 GUI_Control();
	virtual			~GUI_Control();

			void	SetMsg(intptr_t msg, intptr_t param);
			float	GetValue(void) const;
			float	GetMin(void) const;
			float	GetMax(void) const;
			float	GetPageSize(void) const;

	virtual	void	SetValue(float inValue);
	virtual	void	SetMin(float inMin);
	virtual	void	SetMax(float inMax);
	virtual	void	SetPageSize(float inPageSize);

	// Overrides from GUI_Pane.

private:

		float		mValue;
		float		mMin;
		float		mMax;
		float		mPageSize;
		intptr_t	mMsg;
		intptr_t	mParam;
};

#endif /* GUI_CONTROL_H */
