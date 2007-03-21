#ifndef GUI_POPUPBUTTON_H
#define GUI_POPUPBUTTON_H

#include "GUI_Control.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

class	GUI_PopupButton : public GUI_Control {
public:

						 GUI_PopupButton();
	virtual				~GUI_PopupButton();

	virtual	void		SetDescriptor(const string& inDesc);
	virtual	void		Draw(GUI_GraphState * state);
	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	
	virtual	void		SetValue(float inValue);
	
private:

		vector<string>		mItems;
	
};	

#endif