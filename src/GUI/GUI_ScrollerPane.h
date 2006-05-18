#ifndef GUI_SCROLLERPANE_H
#define GUI_SCROLLERPANE_H

// This class manages scrollbars for you.  The behavior provides
// the scrolled content - doesn't have to be a subpane, can be anything.
// Deleting super pane does NOT delete behavhior because behavior may 
// just be an APSECT of a subclassed pane which will be deleted anyway - so
// we have to avoid the double-delete.

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

			void	PositionInContentArea(GUI_Pane * inPane);		// Sticks between scrollbars
			void	SetContent(GUI_ScrollerPaneContent * inPane);

			void	ContentGeometryChanged(void);					// Fix scrollbars, we changed!

	// From GUI_Pane
	virtual void	SetBounds(int x1, int y1, int x2, int y2);
	virtual void	SetBounds(int inBounds[4]);

	// From GUI_Listener
	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);



private:

			void	CalibrateSBs(void);

	GUI_ScrollBar *					mScrollH;
	GUI_ScrollBar *					mScrollV;
	GUI_ScrollerPaneContent *		mContent;
	bool							mCalibrating;

};
	

#endif
