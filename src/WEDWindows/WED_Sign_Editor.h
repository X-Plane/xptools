//
//  WED_SignEditor.hpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/31/15.
//
//

#ifndef WED_Sign_Editor_h
#define WED_Sign_Editor_h

#include "WED_Sign_Parser.h"
#include "GUI_Pane.h"
#include "GUI_Commander.h"
#include "GUI_Timer.h"
#include "WED_Taxi_Sign.h"

class GUI_GraphState;

int plot_token(const sign_token& sign, int x, int y, float scale, GUI_GraphState * g);

void RenderSign(GUI_GraphState * state, int x, int y, const string& sign_text, float scale, int font_id, const float color[4]);
	
class WED_Sign_Editor : public GUI_Pane, public GUI_Commander, public GUI_Timer {
public:

						WED_Sign_Editor(GUI_Commander * parent);

	virtual	void		Draw(GUI_GraphState * state);
	virtual	int			MouseMove(int x, int y			  );
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	
	virtual	int			GetCursor(int x, int y) { 
							return 	gui_Cursor_Arrow;  // prevents cursor being affected by elements in underlyig windows
							}

	virtual	void		TimerFired(void);
			bool		SetSignText(const string& outDesc);
			void		GetSignText(string& outDesc);
protected:

	virtual	int				AcceptTakeFocus();
	virtual	int				AcceptLoseFocus(int force);
	virtual	int			HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);

private:

	void		delete_selection();
	void		replace_selection(parser_glyph_t g, parser_color_t c);
	void		selection_changed();

	int				mEditSide;
	int				mEditStart;
	int				mEditEnd;
	int				mIsDrag;
	int				mCaret;
	parser_color_t	mColor;
	sign_data		mData;
	
	
};







#endif /* WED_Sign_Editor_h */
