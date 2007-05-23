#include "WED_Menus.h"
#include "GUI_Application.h"

GUI_Menu	test1 = 0;
GUI_Menu	sub1 = 0;

static const GUI_MenuItem_t	kFileMenu[] = {
{	"New Package...",	'N',	gui_ControlFlag,	0,	wed_NewPackage		},
{	"Open Package...",	'O',	gui_ControlFlag,	0,	wed_OpenPackage		},
{	"Close",			'W',	gui_ControlFlag,	0,	gui_Close			},
{	NULL,				0,		0,					0,	0					},
};

static const GUI_MenuItem_t	kEditMenu[] = {
{	"Undo",				'Z',	gui_ControlFlag,				0,	gui_Undo		},
{	"Redo",				'Z',	gui_ControlFlag+gui_ShiftFlag,	0,	gui_Redo		},
{	"-",				0,  	0,								0,	0				},
{	"Cut",				'X',	gui_ControlFlag,				0,	gui_Cut			},
{	"Copy",				'C',	gui_ControlFlag,				0,	gui_Copy		},
{	"Paste",			'V',	gui_ControlFlag,				0,	gui_Paste		},
{	"Clear",			0,		0,								0,	gui_Clear		},	// we could use GUI_KEY_DELETE but having del as cmd key screws up text fields.
{	"-",				0,  	0,								0,	0				},
{	"Group",			'G',	gui_ControlFlag,				0,	wed_Group		},
{	"Ungroup",			'G'	,	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Ungroup		},
{	"-",				0,  	0,								0,	0				},
{	"Move First",		'[',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_MoveFirst	},
{	"Move Up",			'[',	gui_ControlFlag,				0,	wed_MovePrev	},
{	"Move Down",		']',	gui_ControlFlag,				0,	wed_MoveNext	},
{	"Move Last",		']',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_MoveLast	},
{	NULL,				0,		0,								0,	0				},
};

static const GUI_MenuItem_t kViewMenu[] = {
{	"Pick Overlay Image...",	0,	0,							0,	wed_PickOverlay		},
{	"Toggle Overlay Image",		0,	0,							0,	wed_ToggleOverlay	},
{	"Toggle Terraserver",		0,	0,							0,	wed_ToggleTerraserver },
{	NULL,						0,	0,							0,	0					},
};

static const GUI_MenuItem_t kSelectMenu[] = {
{	"Select All",		'A',			gui_ControlFlag,				0,	gui_SelectAll		},
{	"Select None",		'D',			gui_ControlFlag,				0,	gui_SelectNone		},
{	"-",				0,				0,								0,	0					},
{	"Select Parent",	GUI_KEY_UP,		gui_ControlFlag,				0,	wed_SelectParent	},
{	"Select Children",	GUI_KEY_DOWN,	gui_ControlFlag,				0,	wed_SelectChild		},
{	"Select Polygon",	GUI_KEY_UP,		gui_ControlFlag+gui_ShiftFlag,	0,	wed_SelectPoly		},
{	"Select Vertices",	GUI_KEY_DOWN,	gui_ControlFlag+gui_ShiftFlag,	0,	wed_SelectVertex	},
{	NULL,				0,				0,								0,	0					},
};

static const GUI_MenuItem_t kAirportMenu[] = {
{	"Create Airport",			'A',	gui_ControlFlag+gui_ShiftFlag,			0, wed_CreateApt },
{	"No Airport Selected",		'E',	gui_ControlFlag+gui_ShiftFlag,			0, wed_EditApt },
{	NULL,						0,		0,										0, 0,				}
};



void WED_MakeMenus(GUI_Application * inApp)
{
	GUI_Menu file_menu = inApp->CreateMenu(
		"File", kFileMenu, inApp->GetMenuBar(), 0);		

	GUI_Menu edit_menu = inApp->CreateMenu(
		"Edit", kEditMenu, inApp->GetMenuBar(), 0);		

	GUI_Menu  view_menu = inApp->CreateMenu(
		"View", kViewMenu, inApp->GetMenuBar(), 0);		
		
	GUI_Menu  sel_menu = inApp->CreateMenu(
		"Select", kSelectMenu, inApp->GetMenuBar(), 0);

	GUI_Menu	airpor_menu = inApp->CreateMenu(
		"Airport", kAirportMenu, inApp->GetMenuBar(),0);	
}
