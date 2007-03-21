#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "GUI_Control.h"

enum {

	btn_Push,
	btn_Check,
	btn_Radio

};

class	GUI_Button : public GUI_Control {
public:
						 GUI_Button();
	virtual				~GUI_Button();

			void		SetStyle(int style);

	virtual	void		SetValue(float inValue);

	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	void		Draw(GUI_GraphState * state);

private:

		int		mStyle;
		int		mHilite;
		
};

#endif
