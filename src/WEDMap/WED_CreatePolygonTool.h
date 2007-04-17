#ifndef WED_CREATEPOLYGONTOOL_H
#define WED_CREATEPOLYGONTOOL_H

#include "WED_CreateToolBase.h"

class	WED_CreatePolygonTool : public WED_CreateToolBase {
public:

						 WED_CreatePolygonTool(
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer, 
									IResolver *			resolver);
	virtual				~WED_CreatePolygonTool();

	virtual int			GetNumProperties(void) { return 0; }
	virtual	void		GetNthPropertyName(int, string&) { }
	virtual	double		GetNthPropertyValue(int) { return 0; }
	virtual	void		SetNthPropertyValue(int, double) { }
	
	virtual	int			GetNumButtons(void) { return 0; }
	virtual	void		GetNthButtonName(int, string&) { }
	virtual	void		NthButtonPressed(int) { }
	
	virtual	char *		GetStatusText(void) { return NULL; }

protected:

	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<int>		has_dirs,
							const vector<Point2>&	dirs,
							int						closed);

};

#endif /* WED_CREATEPOLYGONTOOL_H */
