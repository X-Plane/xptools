#ifndef GUI_SCROLLBAR_H
#define GUI_SCROLLBAR_H

#include "GUI_Control.h"

class GUI_ScrollBar : public GUI_Control {
public:
						 GUI_ScrollBar();
						~GUI_ScrollBar();

	// From GUI_Pane
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp(int x, int y, int button);
	virtual	void		Draw(GUI_GraphState * state);

	// From GUI_Control
	virtual	void	SetValue(float inValue);
	virtual	void	SetMin(float inMin);
	virtual	void	SetMax(float inMax);
	virtual	void	SetPageSize(float inPageSize);

private:

		int		mClickPart;
		int		mInPart;
		float	mSlop;

};

#endif
