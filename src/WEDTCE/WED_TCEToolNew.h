/*
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_TCEToolNew_H
#define WED_TCEToolNew_H

#include "WED_TCELayer.h"
#include "GUI_Defs.h"
#include "IPropertyObject.h"

class WED_TCEToolNew: public WED_TCELayer, public IPropertyObject {
public:

						 		 WED_TCEToolNew(const char * tool_name, GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual						~WED_TCEToolNew();

	virtual	int					HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers)=0;
	virtual	void				HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers)=0;
	virtual	void				HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers)=0;
	virtual	int					HandleToolKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  )=0;
	virtual	void				KillOperation(bool mouse_is_down)=0;		// Called when another tool is picked.  If a shape is half built, ABORT!

	virtual	const char *		GetStatusText(void)=0;

			const char *		GetToolName(void) const;

private:

	string	tool_name;


};

#endif /* WED_TCEToolNew_H */
