#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "GUI_Control.h"

enum GUI_ButtonType {

	btn_Web,
	btn_Push,
	btn_Check,
	btn_Radio

};

class	GUI_Button : public GUI_Control {
public:
						 GUI_Button(
								const char *		in_res_name,
								GUI_ButtonType		behavior,
								int					off_regular[4],
								int					off_hilite[4],
								int					on_regular[4],
								int					on_hilite[4]);								
	virtual				~GUI_Button();

	virtual	void		SetValue(float inValue);

	virtual	int			MouseMove(int x, int y);
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	void		Draw(GUI_GraphState * state);

			void		SetHilite(int hilite);
private:

		GUI_ButtonType		mBehavior;
		string				mResource;
		int					mCellOffReg[4];
		int					mCellOffHilite[4];
		int					mCellOnReg[4];
		int					mCellOnHilite[4];
		int					mHilite;		
};

#endif
