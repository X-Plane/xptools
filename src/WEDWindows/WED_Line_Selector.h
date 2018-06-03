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

//			bool		SetSelection(const string& inDesc);
			bool		SetSelection(set<int> choice);
			void		SetChoices(const vector<GUI_MenuItem_t> * dict);

			
//			void		GetSelection(string& outDesc);
			void		GetSelection(set<int>& choices);
			
			
protected:

	virtual	int			AcceptTakeFocus();
	virtual	int			AcceptLoseFocus(int force);
	virtual	int			HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);

private:

	vector<GUI_MenuItem_t>	mDict;
	int 					mChoice;

};


class	GUI_ScrollerPane;
class	GUI_Table;
class	GUI_Header;
class	GUI_TextTable;
class	GUI_TextTableHeader;
class	WED_Document;
class	GUI_FilterBar;
class	GUI_Button;

#include "GUI_Window.h"
#include "GUI_Listener.h"
#include "GUI_Destroyable.h"

#include "WED_AptTable.h"

class WED_EnumSelectDialog : public GUI_Window, public GUI_Listener, public GUI_Destroyable {

public:
	WED_EnumSelectDialog(GUI_TextTable * resolver, GUI_CellContent * info, const GUI_EnumDictionary& dict);
	~WED_EnumSelectDialog();

	virtual void ReceiveMessage(
					GUI_Broadcaster *		inSrc,
					intptr_t    			inMsg,
					intptr_t				inParam);
private:
	WED_Document *		mResolver;
	GUI_FilterBar *		mFilter;
	GUI_CellContent *	mInfo;

	// List of errors and warnings
	GUI_ScrollerPane *		mScroller;
	GUI_Table *				mTable;
	GUI_Header *			mHeader;

	GUI_TextTable			mTextTable;
	GUI_TextTableHeader		mTextTableHeader;
	
	// brazenly mis-using these for our list
	AptVector				mMsgs;
	WED_AptTable			mMsgTable;
};



#endif /* WED_Line_Selector_h */
