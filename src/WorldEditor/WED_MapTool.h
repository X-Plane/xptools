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
#ifndef WED_MAPTOOL_H
#define WED_MAPTOOL_H

#include "XPLMDisplay.h"

class	WED_MapZoomer;

class	WED_MapTool {
public:

					WED_MapTool(WED_MapZoomer * inZoomer);
	virtual			~WED_MapTool();

	// Mouse API - the tool can provide visual indications of what's
	// going on and also 
	virtual	void	DrawFeedbackUnderlay(
							bool				inCurrent)=0;
	virtual	void	DrawFeedbackOverlay(
							bool				inCurrent)=0;
	virtual	bool	HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX, 
							int 				inY, 
							int 				inButton)=0;
							
	// Support for some properties that can be edited.	
	virtual int		GetNumProperties(void)=0;
	virtual	void	GetNthPropertyName(int, string&)=0;
	virtual	double	GetNthPropertyValue(int)=0;
	virtual	void	SetNthPropertyValue(int, double)=0;
	
	virtual	int		GetNumButtons(void)=0;
	virtual	void	GetNthButtonName(int, string&)=0;
	virtual	void	NthButtonPressed(int)=0;
	
	virtual	char *	GetStatusText(void)=0;

protected:

	inline WED_MapZoomer *	GetZoomer(void) const { return mZoomer; }
		
private:

					WED_MapTool();

		WED_MapZoomer *		mZoomer;
	
};

#endif
