#ifndef WED_MENUS_H
#define WED_MENUS_H

enum {

	wed_NewPackage = 1000,
	wed_OpenPackage,
	wed_Close

};

class	GUI_Application;

void WED_MakeMenus(GUI_Application * inApp);

#endif
