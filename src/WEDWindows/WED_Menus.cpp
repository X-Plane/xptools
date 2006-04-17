#include "WED_Menus.h"
#include "GUI_Application.h"

static const GUI_MenuItem_t	kFileMenu[] = {
{	"New Package...",	'N',	gui_ControlFlag,	wed_NewPackage		},
{	"Open Package...",	'O',	gui_ControlFlag,	wed_OpenPackage		},
{	"Close",			'W',	gui_ControlFlag,	wed_Close			},
{	NULL,				0,		0,					0					},
};

void WED_MakeMenus(GUI_Application * inApp)
{
	GUI_Menu file_menu = inApp->CreateMenu(
		"File", kFileMenu, inApp->GetMenuBar(), 0);		

}
