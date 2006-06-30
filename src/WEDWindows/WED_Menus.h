#ifndef WED_MENUS_H
#define WED_MENUS_H

#include "GUI_Menus.h"

enum {

	// File Menu
	wed_NewPackage = GUI_APP_MENUS,
	wed_OpenPackage

};

class	GUI_Application;

void WED_MakeMenus(GUI_Application * inApp);

#endif
