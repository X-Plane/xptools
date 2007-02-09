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
