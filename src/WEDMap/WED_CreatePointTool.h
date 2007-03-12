#ifndef WED_CREATEPOINTTOOL_H
#define WED_CREATEPOINTTOOL_H

#include "WED_MapToolNew.h"

class	WED_CreatePointTool : public WED_MapToolNew {
public:

						 WED_CreatePointTool(WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_CreatePointTool();

	virtual	void		DrawSelected			(int inCurrent, GUI_GraphState * g);

	virtual	int			HandleClickDown(int inX, int inY, int inButton);
	virtual	void		HandleClickDrag(int inX, int inY, int inButton);
	virtual	void		HandleClickUp  (int inX, int inY, int inButton);
							
	virtual int			GetNumProperties(void);
	virtual	void		GetNthPropertyName(int, string&);
	virtual	double		GetNthPropertyValue(int);
	virtual	void		SetNthPropertyValue(int, double);
	
	virtual	int			GetNumButtons(void);
	virtual	void		GetNthButtonName(int, string&);
	virtual	void		NthButtonPressed(int);
	
	virtual	char *		GetStatusText(void);

private:

	double	click_x;
	double	click_y;
	
	bool		clicking;

};


#endif
