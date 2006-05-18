#ifndef WED_PACKAGESTATUSPANE_H
#define WED_PACKAGESTATUSPANE_H

class	WED_Package;

#include "GUI_Pane.h"

class	WED_PackageStatusPane : public GUI_Pane {
public:

					 WED_PackageStatusPane(WED_Package * inPackage);
	virtual			~WED_PackageStatusPane();
	
	virtual	void		Draw(GUI_GraphState * state);
	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp(int x, int y, int button);

private:

	WED_Package * mPackage;

};
	


#endif /* WED_PACKAGESTATUSVIEW_H */


