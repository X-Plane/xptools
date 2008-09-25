/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "WED_Menus.h"
#include "GUI_Application.h"

GUI_Menu	test1 = 0;
GUI_Menu	sub1 = 0;

static const GUI_MenuItem_t	kAppMenu[] = {
{ "About WED...",		0,	0,	0,	0 },
{ "-",					0,	0,	0,	0 },
{	NULL,				0,		0,					0,	0					},
};


static const GUI_MenuItem_t	kFileMenu[] = {
{	"&New Package...",		'N',	gui_ControlFlag,				0,	wed_NewPackage		},
{	"&Open Package...",		'O',	gui_ControlFlag,				0,	wed_OpenPackage		},
{	"Chan&ge X-System Folder...",0,	0,								0,	wed_ChangeSystem	},
{	"-",					0,  	0,								0,	0					},
{	"&Close",				'W',	gui_ControlFlag,				0,	gui_Close			},
{	"&Save",				'S',	gui_ControlFlag,				0,	gui_Save			},
{	"&Revert To Saved",		0,		0,								0,	gui_Revert			},
{	"-",					0,  	0,								0,	0					},
{	"&Validate",			'V',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Validate		},
{	"&Import apt.dat...",	'I',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_ImportApt		},
{	"&Export apt.dat...",	'S',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_ExportApt		},
#if 0
{	"Export &DSFs",			0,		0,								0,	wed_ExportDSF		},
#endif
#if IBM || LIN
{	"-",					0,		0,								0,	0					},
{	"&Preferences...",		0,		0,								0,	gui_Prefs			},
{	"-",					0,		0,								0,	0					},
{	"E&xit",				0,		0,								0,	gui_Quit			},
#endif
{	NULL,					0,		0,								0,	0					},
};

static const GUI_MenuItem_t	kEditMenu[] = {
{	"&Undo",				'Z',	gui_ControlFlag,				0,	gui_Undo		},
{	"&Redo",				'Z',	gui_ControlFlag+gui_ShiftFlag,	0,	gui_Redo		},
{	"-",					0,  	0,								0,	0				},
{	"Cu&t",					'X',	gui_ControlFlag,				0,	gui_Cut			},
{	"&Copy",				'C',	gui_ControlFlag,				0,	gui_Copy		},
{	"&Paste",				'V',	gui_ControlFlag,				0,	gui_Paste		},
{	"Cl&ear",				0,		0,								0,	gui_Clear		},	// we could use GUI_KEY_DELETE but having del as cmd key screws up text fields.
{	"&Duplicate",			0,		0,								0,	gui_Duplicate	},
{	"-",					0,  	0,								0,	0				},
{	"&Group",				'G',	gui_ControlFlag,				0,	wed_Group		},
{	"U&ngroup",				'G'	,	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Ungroup		},
{	"-",					0,  	0,								0,	0				},
{	"Spl&it",				'E',	gui_ControlFlag,				0,	wed_Split		},
{	"Rever&se",				0,		0,								0,	wed_Reverse		},
{	"Cr&op Unselected",		0,		0,								0,	wed_Crop		},
{	"-",					0,  	0,								0,	0				},
{	"Move &First",			'[',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_MoveFirst	},
{	"&Move Up",				'[',	gui_ControlFlag,				0,	wed_MovePrev	},
{	"Move Do&wn",			']',	gui_ControlFlag,				0,	wed_MoveNext	},
{	"Move &Last",			']',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_MoveLast	},
{	NULL,					0,		0,								0,	0				},
};

static const GUI_MenuItem_t kViewMenu[] = {
{	"Zoom Worl&d",				'/',gui_ControlFlag+gui_OptionAltFlag,		0,	wed_ZoomWorld		},
{	"&Zoom Package",			'/',gui_ControlFlag,						0,	wed_ZoomAll			},
{	"Zoom &Selection",			'/',gui_ControlFlag+gui_ShiftFlag,			0,	wed_ZoomSelection	},
{	"-",						0,	0,										0,	0					},
{	"&Feet",						0,	0,										0,	wed_UnitFeet		},
{	"&Meters",					0,	0,										0,	wed_UnitMeters		},
{	"-",						0,	0,										0,	0					},
{	"Show &Line Markings",		0,	0,										0,	wed_ToggleLines		},
{	"Show &Vertices",			0,	0,										0,	wed_ToggleVertices	},
// Janos says: no submenus yet :-(
#if !LIN
{	"Pavement Transparenc&y",	0,	0,										0,	0					},
#endif
{	"-",						0,	0,										0,	0					},
{	"&Pick Overlay Image...",	0,	0,										0,	wed_PickOverlay		},
//{	"Toggle &Overlay Image",	0,	0,										0,	wed_ToggleOverlay	},
{	"Toggle &World Map",		0,	0,										0,	wed_ToggleWorldMap	},
{	"Toggle &Terraserver",		0,	0,										0,	wed_ToggleTerraserver },
{	"-",						0,	0,										0,	0					},
{	"&Restore Frames",			0,	0,										0,	wed_RestorePanes	},
{	NULL,						0,	0,										0,	0					},
};

static const GUI_MenuItem_t kPavementMenu[] = {
{	"&None",					0,	0,							0,	wed_Pavement0		},
{	"&25%",						0,	0,							0,	wed_Pavement25		},
{	"&50%",						0,	0,							0,	wed_Pavement50		},
{	"&75%",						0,	0,							0,	wed_Pavement75		},
{	"&Solid",					0,	0,							0,	wed_Pavement100		},
{	NULL,						0,	0,							0,	0					}
};

static const GUI_MenuItem_t kSelectMenu[] = {
{	"Select &All",		'A',			gui_ControlFlag,				0,	gui_SelectAll		},
{	"Select &None",		'D',			gui_ControlFlag,				0,	gui_SelectNone		},
{	"-",				0,				0,								0,	0					},
{	"Select &Parent",	GUI_KEY_UP,		gui_ControlFlag,				0,	wed_SelectParent	},
{	"Select &Children",	GUI_KEY_DOWN,	gui_ControlFlag,				0,	wed_SelectChild		},
{	"Select P&olygon",	GUI_KEY_UP,		gui_ControlFlag+gui_ShiftFlag,	0,	wed_SelectPoly		},
{	"Select &Vertices",	GUI_KEY_DOWN,	gui_ControlFlag+gui_ShiftFlag,	0,	wed_SelectVertex	},
{	NULL,				0,				0,								0,	0					},
};

static const GUI_MenuItem_t kAirportMenu[] = {
{	"&Create Airport",			'A',	gui_ControlFlag+gui_ShiftFlag,			0, wed_CreateApt },
{	"Create ATC &Frequency",	'F',	gui_ControlFlag,						0, wed_AddATCFreq },
{	"No Airport Selected",		'E',	gui_ControlFlag+gui_ShiftFlag,			0, wed_EditApt	},
{	NULL,						0,		0,										0, 0,				}
};

static const GUI_MenuItem_t kHelpMenu[] = {
{	"&WED User's Guide",			0,	0,										0,	wed_HelpManual },
{	"-",							0,	0,										0,	0				},
{	"&X-Plane Scenery Homepage",	0,	0,										0,	wed_HelpScenery },
#if IBM || LIN
{	"-",							0,		0,									0,	0				},
{	"&About WED",					0,		0,									0,	gui_About		},
#endif
{	NULL,							0,		0,									0, 0,				}
};

void WED_MakeMenus(GUI_Application * inApp)
{
	GUI_Menu file_menu = inApp->CreateMenu(
		"&File", kFileMenu, inApp->GetMenuBar(), 0);

	GUI_Menu edit_menu = inApp->CreateMenu(
		"&Edit", kEditMenu, inApp->GetMenuBar(), 0);

	GUI_Menu  view_menu = inApp->CreateMenu(
		"&View", kViewMenu, inApp->GetMenuBar(), 0);

	GUI_Menu	pave_menu = inApp->CreateMenu(
		"Pavement T&ransparency",	kPavementMenu, view_menu, 9);

	GUI_Menu  sel_menu = inApp->CreateMenu(
		"&Select", kSelectMenu, inApp->GetMenuBar(), 0);

	GUI_Menu	airport_menu = inApp->CreateMenu(
		"&Airport", kAirportMenu, inApp->GetMenuBar(),0);

	GUI_Menu	help_menu;
#if APL
	MenuRef	win_menu;
	if (CreateStandardWindowMenu(kWindowMenuIncludeRotate,&win_menu)==noErr)
		InsertMenu(win_menu,0);

	MenuRef hp = 0;
	MenuItemIndex ind = 0;
	OSStatus err = HMGetHelpMenu(&hp,&ind);

	inApp->RebuildMenu(hp, kHelpMenu);
#else
	help_menu = inApp->CreateMenu("&Help", kHelpMenu, inApp->GetMenuBar(), 0);
#endif
}
