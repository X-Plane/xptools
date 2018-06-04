//
//  WED_Line_Selectorr.cpp
//
//  Created by Michael Minnhaar  5/25/18.
//
//

#ifndef WED_Line_Selector_h
#define WED_Line_Selector_h

#include "GUI_Pane.h"
#include "GUI_TextTable.h"
#include "GUI_Commander.h"

class GUI_GraphState;


class WED_Line_Selector : public GUI_Pane, public GUI_Commander {
public:
						WED_Line_Selector(GUI_Commander * parent);

	virtual	void		Draw(GUI_GraphState * state);

	virtual	int			MouseDown(int x, int y, int button);
	
	virtual	int			GetCursor(int x, int y) { return gui_Cursor_Arrow; } // prevents cursor being affected by elements in underlyig windows

			bool		SetSelection(set<int> choice);
			void		SetChoices(const vector<GUI_MenuItem_t> * dict);
			void		GetSelection(int& choices);
			
protected:

	virtual	int			AcceptTakeFocus();
	virtual	int			AcceptLoseFocus(int force);
	virtual	int			HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);

private:

	vector<GUI_MenuItem_t>	mDict;
	int 					mChoice;

};

#endif /* WED_Line_Selector_h */
