#ifndef WED_MENUS_H
#define WED_MENUS_H

#include "GUI_Application.h"
#include "GUI_Menus.h"

extern	GUI_Menu	test1;
extern	GUI_Menu	sub1;

enum {

	// File Menu
	wed_NewPackage = GUI_APP_MENUS,
	wed_OpenPackage,
	wed_ChangeSystem,
	wed_Validate,
	wed_ImportApt,
	wed_ExportApt,
	wed_ExportDSF,
	// Edit Menu,
	wed_Group,
	wed_Ungroup,
	wed_Crop,
	wed_Split,
	wed_Reverse,
	wed_MoveFirst,
	wed_MovePrev,
	wed_MoveNext,
	wed_MoveLast,
	// Pavement menu
	wed_Pavement0,
	wed_Pavement25,
	wed_Pavement50,
	wed_Pavement75,
	wed_Pavement100,
	// view menu
	wed_ZoomWorld,
	wed_ZoomAll,
	wed_ZoomSelection,
	wed_UnitFeet,
	wed_UnitMeters,
	wed_ToggleLines,	
	wed_ToggleVertices,
	wed_PickOverlay,
//	wed_ToggleOverlay,
	wed_ToggleWorldMap,
	wed_ToggleTerraserver,
	wed_RestorePanes,
	// Select Menu
	wed_SelectParent,
	wed_SelectChild,
	wed_SelectVertex,
	wed_SelectPoly,
	// Airport Menu
	wed_CreateApt,
	wed_EditApt,
	wed_AddATCFreq,
	// Help Menu
	wed_HelpScenery	
};

class	GUI_Application;

void WED_MakeMenus(GUI_Application * inApp);

#endif
