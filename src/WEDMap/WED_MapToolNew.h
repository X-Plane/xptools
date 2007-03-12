#ifndef WED_MAPTOOLNEW_H
#define WED_MAPTOOLNEW_H

#include "WED_MapLayer.h"

class	WED_MapToolNew : public WED_MapLayer {
public:

						 WED_MapToolNew(WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_MapToolNew();

	virtual	int			HandleClickDown(int inX, int inY, int inButton)=0;
	virtual	void		HandleClickDrag(int inX, int inY, int inButton)=0;
	virtual	void		HandleClickUp  (int inX, int inY, int inButton)=0;
							
	virtual int			GetNumProperties(void)=0;
	virtual	void		GetNthPropertyName(int, string&)=0;
	virtual	double		GetNthPropertyValue(int)=0;
	virtual	void		SetNthPropertyValue(int, double)=0;
	
	virtual	int			GetNumButtons(void)=0;
	virtual	void		GetNthButtonName(int, string&)=0;
	virtual	void		NthButtonPressed(int)=0;
	
	virtual	char *		GetStatusText(void)=0;

};

#endif