#ifndef GUI_TOOLBAR_H
#define GUI_TOOLBAR_H

#include "GUI_Control.h"

class	GUI_ToolBar : public GUI_Control {
public:

			 		 GUI_ToolBar(int h, int v, const char * in_resource);
	virtual			~GUI_ToolBar();
	
			void	SizeToBitmap(void);
	
	virtual	void	SetValue(float inValue);


	virtual	void	Draw(GUI_GraphState * state);
	
	virtual	int		MouseDown(int x, int y, int button);
	virtual	void	MouseDrag(int x, int y, int button);
	virtual	void	MouseUp  (int x, int y, int button);

private:

	string		mResource;
	int			mH;
	int			mV;
	
};

#endif