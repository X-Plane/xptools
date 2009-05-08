/*
 * Copyright (c) 2004, Laminar Research.
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
#ifndef RF_MAPTOOL_H
#define RF_MAPTOOL_H

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#include "GUI_Defs.h"

class	RF_MapZoomer;
class	GUI_GraphState;

enum XPLMMouseStatus {
     xplm_MouseDown                           = 1,
     xplm_MouseDrag                           = 2,
     xplm_MouseUp                             = 3
};

class	RF_MapTool {
public:

					RF_MapTool(RF_MapZoomer * inZoomer);
	virtual			~RF_MapTool();

	// Mouse API - the tool can provide visual indications of what's
	// going on and also
	virtual	void	DrawFeedbackUnderlay(
							GUI_GraphState *	state,
							bool				inCurrent)=0;
	virtual	void	DrawFeedbackOverlay(
							GUI_GraphState *	state,
							bool				inCurrent)=0;
	virtual	bool	HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX,
							int 				inY,
							int 				inButton,
							GUI_KeyFlags		inModifiers)=0;

	// Support for some properties that can be edited.
	virtual int		GetNumProperties(void)=0;
	virtual	void	GetNthPropertyName(int, string&)=0;
	virtual	double	GetNthPropertyValue(int)=0;
	virtual	void	SetNthPropertyValue(int, double)=0;

	virtual	int		GetNumButtons(void)=0;
	virtual	void	GetNthButtonName(int, string&)=0;
	virtual	void	NthButtonPressed(int)=0;

	virtual	char *	GetStatusText(int x, int y)=0;

protected:

	inline RF_MapZoomer *	GetZoomer(void) const { return mZoomer; }

private:

					RF_MapTool();

		RF_MapZoomer *		mZoomer;

};

#endif
