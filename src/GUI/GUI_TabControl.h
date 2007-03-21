#ifndef GUI_TABCONTROL_H
#define GUI_TABCONTROL_H

#include "GUI_Control.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

class	GUI_TabControl : public GUI_Control {
public:

						 GUI_TabControl();
	virtual				~GUI_TabControl();

	virtual	void		SetDescriptor(const string& inDesc);
	virtual	void		Draw(GUI_GraphState * state);
	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	
	virtual	void		SetValue(float inValue);
	
private:

		vector<string>		mItems;
		vector<int>			mWidths;
		
		int					mTrackBtn;
		int					mHilite;
	
};	



#endif