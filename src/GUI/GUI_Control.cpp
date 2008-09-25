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

#include "GUI_Control.h"
#include "GUI_Messages.h"

GUI_Control::GUI_Control() :
	mMin(0), mMax(1), mValue(0), mPageSize(1)
{
}

GUI_Control::~GUI_Control()
{
}

float	GUI_Control::GetValue(void) const
{
	return mValue;
}

float	GUI_Control::GetMin(void) const
{
	return mMin;
}

float	GUI_Control::GetMax(void) const
{
	return mMax;
}

float	GUI_Control::GetPageSize(void) const
{
	return mPageSize;
}

void	GUI_Control::SetValue(float inValue)
{
	mValue = inValue;
	BroadcastMessage(GUI_CONTROL_VALUE_CHANGED, 0);
}

void	GUI_Control::SetMin(float inMin)
{
	mMin = inMin;
}

void	GUI_Control::SetMax(float inMax)
{
	mMax = inMax;
}

void	GUI_Control::SetPageSize(float inPageSize)
{
	mPageSize = inPageSize;
}
