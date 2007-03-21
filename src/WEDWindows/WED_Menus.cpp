#include "WED_Menus.h"
#include "GUI_Application.h"

GUI_Menu	test1 = 0;
GUI_Menu	sub1 = 0;

static const GUI_MenuItem_t	kFileMenu[] = {
{	"New Package...",	'N',	gui_ControlFlag,	wed_NewPackage		},
{	"Open Package...",	'O',	gui_ControlFlag,	wed_OpenPackage		},
{	"Close",			'W',	gui_ControlFlag,	gui_Close			},
{	NULL,				0,		0,					0					},
};

static const GUI_MenuItem_t	kEditMenu[] = {
{	"Undo",				'Z',	gui_ControlFlag,				gui_Undo		},
{	"Redo",				'Z',	gui_ControlFlag+gui_ShiftFlag,	gui_Redo		},
{	"-",				0,  	0,								0				},
{	"Cut",				'X',	gui_ControlFlag,				gui_Cut			},
{	"Copy",				'C',	gui_ControlFlag,				gui_Copy		},
{	"Paste",			'V',	gui_ControlFlag,				gui_Paste		},
{	"Clear",			0,		0,								gui_Clear		},
{	"Select All",		'A',	gui_ControlFlag,				gui_SelectAll	},
{	NULL,				0,		0,								0				},
};

static const GUI_MenuItem_t kTestMenu[] = {
{	"An Item",			0,		0,								gui_Close		},
{	"Submenu",			0,		0,								0				},
{	NULL,				0,		0,								0,				}
};

static const GUI_MenuItem_t kTestSubMenu[] = {
{	"Sub 1",			0,		0,								gui_Close		},
{	"Sub 2",			0,		0,								gui_Close		},
{	NULL,				0,		0,								0,				}
};



void WED_MakeMenus(GUI_Application * inApp)
{
	GUI_Menu file_menu = inApp->CreateMenu(
		"File", kFileMenu, inApp->GetMenuBar(), 0);		

	GUI_Menu edit_menu = inApp->CreateMenu(
		"Edit", kEditMenu, inApp->GetMenuBar(), 0);		

	test1 = inApp->CreateMenu(
		"Test", kTestMenu, inApp->GetMenuBar(), 0);

	sub1 = inApp->CreateMenu(
		"Sub", kTestSubMenu, test1, 1);

}
