#ifndef WED_MAPTOOLNEW_H
#define WED_MAPTOOLNEW_H

#include "WED_MapLayer.h"
#include "WED_PropertyHelper.h"
#include "IPropertyObject.h"
class	GUI_Pane;

class	WED_MapToolNew : public WED_MapLayer, public WED_PropertyHelper {
public:

						 WED_MapToolNew(const char * tool_name, GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_MapToolNew();

	virtual	int			HandleClickDown(int inX, int inY, int inButton)=0;
	virtual	void		HandleClickDrag(int inX, int inY, int inButton)=0;
	virtual	void		HandleClickUp  (int inX, int inY, int inButton)=0;
							
	virtual	const char *	GetStatusText(void)=0;

	virtual	void		PropEditCallback(int before);

			const char *	GetToolName(void) const;
			
private:

	string tool_name;
	
};

#endif