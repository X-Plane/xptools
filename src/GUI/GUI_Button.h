#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "GUI_Control.h"

enum GUI_ButtonType {

	btn_Web,		// Hilites when mouse-over...you need to dehilite when in the parent window -- radio-friends dehilite each other.  On click, resets val to 0
	btn_Push,		// Push: hilites while mouse down and in, resets val to 0 on a click.
	btn_Check,		// Toggles itself on a click between 0 and 1.
	btn_Radio		// Sets itself to 1, sets radio friends to 0.

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

			void		SetTiles(											// You can change the tiles on the fly, effectively changing the look or behavior
								int					off_regular[4],			// of the button.
								int					off_hilite[4],
								int					on_regular[4],
								int					on_hilite[4]);										

			void		AddRadioFriend(GUI_Button * who);					// Each button knows its peers in the radio-button or web-button group.

			void		SetHilite(int hilite);								// Manually set hilite state - needed for parent to turn off web-button hilite

	virtual	void		SetValue(float inValue);

	virtual	int			MouseMove(int x, int y);
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	void		Draw(GUI_GraphState * state);

private:
		vector<GUI_Button *>	mRadios;


		GUI_ButtonType		mBehavior;
		string				mResource;
		int					mCellOffReg[4];
		int					mCellOffHilite[4];
		int					mCellOnReg[4];
		int					mCellOnHilite[4];
		int					mHilite;		
};

#endif
