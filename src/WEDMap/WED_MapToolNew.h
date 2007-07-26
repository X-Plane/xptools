#ifndef WED_MAPTOOLNEW_H
#define WED_MAPTOOLNEW_H

#include "CompGeomDefs2.h"
#include "GUI_Defs.h"
#include "WED_MapLayer.h"
#include "WED_PropertyHelper.h"
#include "IPropertyObject.h"
class	GUI_Pane;

class	WED_MapToolNew : public WED_MapLayer, public WED_PropertyHelper {
public:

						 		 WED_MapToolNew(const char * tool_name, GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual						~WED_MapToolNew();

	virtual	int					HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers)=0;
	virtual	void				HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers)=0;
	virtual	void				HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers)=0;
	virtual	int					HandleKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  )=0;
	virtual	void				KillOperation(bool mouse_is_down)=0;		// Called when another tool is picked.  If a shape is half built, ABORT!
							
	virtual	const char *		GetStatusText(void)=0;

	virtual	void				PropEditCallback(int before);
	virtual	int					CountSubs(void);
	virtual	IPropertyObject *	GetNthSub(int n);

			const char *		GetToolName(void) const;


			bool				GetAnchor1(Point2& a);
			bool				GetAnchor2(Point2& a);
			bool				GetDistance(double& d);
			bool				GetHeading(double& h);

protected:
			
			void				SetAnchor1(const Point2& a);
			void				SetAnchor2(const Point2& a);
			void				SetDistance(double d);
			void				SetHeading(double h);

			void				ClearAnchor1(void);
			void				ClearAnchor2(void);
			void				ClearDistance(void);
			void				ClearHeading(void);

private:

	bool		has_anchor1;		Point2	anchor1;
	bool		has_anchor2;		Point2	anchor2;
	bool		has_distance;		double	distance;
	bool		has_heading;		double	heading;

	string	tool_name;
	
};

#endif