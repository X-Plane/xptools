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

#ifndef GUI_SCROLLERPANE_H
#define GUI_SCROLLERPANE_H

/*

	GUI_ScrollerPane - THEORY OF OPERATION
	
	GUI_ScrollerPane is a pane that manages scrollable content.

	No assumptions are made about the content - it doesn't have to be
	a sub-pane; it's simply an aspect of some object.  The object
	returns the logical vs physical bounds (any units) and takes 
	scrolling commands.  ScrollH and ScrollV take absolute positive
	units measured from the logical to physical lower left corners -
	thus when the lower left corner of the logical doc is visible, the
	scroll is 0,0.  Vertical scroll could be negative if we want a top-
	aligned document and the visible rect is taller than the logical ret.

	This class does not delete contents on destruction so that if contents
	are a sub-pane we don't have a double-delete.

	SLAVING
	
	It is possible to slave another scroller to this one - basically it makes
	our scroll-bars drive the slave.  However:
	- child auto-slaving with a "contents changed" message to the master does
	  not scroll slaves, so two slaved text fields are a bad idea...auto-scroll
	  won't propagate.
	- Master-slave is not bidirectional.

*/


#include "GUI_Pane.h"
#include "GUI_Listener.h"
#include "GUI_Broadcaster.h"

class	GUI_ScrollBar;

class	GUI_ScrollerPaneContent : public GUI_Broadcaster {
public:

	virtual	void	GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4])=0;	
	
	// Offset is from the min side of the total bounds to the min side of the visible bounds.
	// so 0 means lower left of the total bounds is visible.
	
	virtual	void	ScrollH(float xOffset)=0;
	virtual	void	ScrollV(float yOffset)=0;

};

class	GUI_ScrollerPane : public GUI_Pane, public GUI_Listener {
public:

					 GUI_ScrollerPane(int inHScroll, int inVScroll);
	virtual			~GUI_ScrollerPane();

			void	AttachSlaveH(GUI_ScrollerPane *inSlaveH);
			void	AttachSlaveV(GUI_ScrollerPane *inSlaveV);

			void	PositionInContentArea(GUI_Pane * inPane);		// Sticks between scrollbars
			void	PositionSidePane(GUI_Pane * pane);
			void	PositionHeaderPane(GUI_Pane * pane);
			void	SetContent(GUI_ScrollerPaneContent * inPane);

			void	ContentGeometryChanged(void);					// Fix scrollbars, we changed!

			void	SetImage(const char * image_res);

	virtual	void	Draw(GUI_GraphState * g);
			
	// From GUI_Pane
	virtual void	SetBounds(int x1, int y1, int x2, int y2);
	virtual void	SetBounds(int inBounds[4]);
	virtual	int		ScrollWheel(int x, int y, int dist, int axis);

	// From GUI_Listener
	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);



private:

		vector<GUI_ScrollerPane*>	mSlaveH;
		vector<GUI_ScrollerPane*>	mSlaveV;

			void	CalibrateSBs(void);

	GUI_ScrollBar *					mScrollH;
	GUI_ScrollBar *					mScrollV;
	GUI_ScrollerPaneContent *		mContent;
	bool							mCalibrating;
	string							mImage;
	bool							mCalibrateDirty;

};
	

#endif
