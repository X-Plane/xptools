#ifndef WED_MAPPANE_H
#define WED_MAPPANE_H

#include "GUI_Pane.h"
#include "WED_MapZoomerNew.h"

class	WED_MapPane : public GUI_Pane, public WED_MapZoomerNew {
public:
		
						 WED_MapPane();
						~WED_MapPane();

	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);

	virtual	void		Draw(GUI_GraphState * state);
	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	int			ScrollWheel(int x, int y, int dist, int axis);


};


#endif
