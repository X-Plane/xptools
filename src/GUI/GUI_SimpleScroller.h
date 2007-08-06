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

#ifndef GUI_SIMPLESCROLLER_H
#define GUI_SIMPLESCROLLER_H

/*
	The simple scroller provides a scroller interface and simply moves its children to accomplish
	scrolling.  This provides a very simple way to implement scrolled views.

*/

#include "GUI_Pane.h"
#include "GUI_ScrollerPane.h"
#include "GUI_Listener.h"

class	GUI_SimpleScroller : public GUI_Pane, public GUI_ScrollerPaneContent, public GUI_Listener {
public:
					 GUI_SimpleScroller();
	virtual			~GUI_SimpleScroller();

	// From GUI_ScrollerPaneContent
	virtual	void	GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4]);	
	virtual	void	ScrollH(float xOffset);
	virtual	void	ScrollV(float yOffset);
	
	// From GUI_Pane
	virtual void	SetBounds(int x1, int y1, int x2, int y2);
	virtual void	SetBounds(int inBounds[4]);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);
private:
			void	AlignContents(void);
	
};

#endif /* GUI_SIMPLESCROLLER_H */
