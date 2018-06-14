//
//  WED_Line_Selector.cpp
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

struct entry {
	string	name;
	bool	checked;
	int		enu;
	entry(const char * c = "") : name(c), checked(false), enu(-1) {}
};

//class WED_Line_Selector : public GUI_Pane, public GUI_Commander {
class WED_Line_Selector : public GUI_EditorInsert {
public:
					WED_Line_Selector(GUI_Commander * parent);
			void	SetChoices(const vector<GUI_MenuItem_t> * dict);

	virtual	void	Draw(GUI_GraphState * state);

	virtual	int		MouseDown(int x, int y, int button);
	virtual	int		MouseMove(int x, int y);
	
	virtual	int		GetCursor(int x, int y) { return gui_Cursor_Arrow; } // prevents cursor being affected by elements in underlyig windows

	virtual bool	SetData(const GUI_CellContent& c);
	virtual	void	GetData(GUI_CellContent& c);
	virtual	void	GetSizeHint(int * w, int * h);
			
protected:

	virtual	int		AcceptTakeFocus() { return 1; }
	virtual	int		AcceptLoseFocus(int force) { return 1; }
	virtual	int		HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);

private:

#define LINESEL_MAX_ROWS 40
	entry			mDict[LINESEL_MAX_ROWS][2];
	int				mColWidth[2];
	
	int 			mChoice;
	int				mR, mC;
	int				mRows, mCols;
};

#endif /* WED_Line_Selector_h */
