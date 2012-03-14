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
{	"Import DS&F...",		0,		0,								0,	wed_ImportDSF		},
{	"&Export apt.dat...",	'S',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_ExportApt		},
{	"Export Scenery Pac&k",	'B',	gui_ControlFlag,				0,	wed_ExportPack		},
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
#if AIRPORT_ROUTING
{	"Merge",				'M',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Merge		},
#endif
{	"Rever&se",				'R',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Reverse		},
{	"Rotate",				'R',	gui_ControlFlag,				0,	wed_Rotate		},
{	"Cr&op Unselected",		0,		0,								0,	wed_Crop		},
{	"Make Draped Pol&ygons",0,		0,								0,	wed_Overlay		},
{	"Error-Check Polygons",	0,		0,								0,	wed_CheckPolys	},
#if AIRPORT_ROUTING
{	"Make Routing",			0,		0,								0,	wed_MakeRouting },
#endif
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
{	"Pavement Transparenc&y",	0,	0,										0,	0					},
{	"&Object Density",			0,	0,										0,	0					},
{	"-",						0,	0,										0,	0					},
{	"&Pick Overlay Image...",	0,	0,										0,	wed_PickOverlay		},
//{	"Toggle &Overlay Image",	0,	0,										0,	wed_ToggleOverlay	},
{	"Toggle &World Map",		0,	0,										0,	wed_ToggleWorldMap	},
{	"Toggle Previe&w",			0,	0,										0,	wed_TogglePreview	},
{	"Toggle &Terraserver",		0,	0,										0,	wed_ToggleTerraserver },
#if WITHNWLINK
{	"Toggle LiveMode",		    0,	0,										0,	wed_ToggleLiveView },
#endif
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

static const GUI_MenuItem_t kObjDensityMenu[] = {
{	"&1 Default",				'1',	gui_ControlFlag,		0,	wed_ObjDensity1		},
{	"&2 A Lot",					'2',	gui_ControlFlag,		0,	wed_ObjDensity2		},
{	"&3 Tons",					'3',	gui_ControlFlag,		0,	wed_ObjDensity3		},
{	"&4 Mega Tons",				'4',	gui_ControlFlag,		0,	wed_ObjDensity4		},
{	"&5 Too Many",				'5',	gui_ControlFlag,		0,	wed_ObjDensity5		},
{	"&6 Totally Insane",		'6',	gui_ControlFlag,		0,	wed_ObjDensity6		},
{	NULL,						0,		gui_ControlFlag,		0,	0					}
};

static const GUI_MenuItem_t kSelectMenu[] = {
{	"Select &All",		'A',			gui_ControlFlag,				0,	gui_SelectAll		},
{	"Select &None",		'D',			gui_ControlFlag,				0,	gui_SelectNone		},
{	"-",				0,				0,								0,	0					},
{	"Select &Parent",	GUI_KEY_UP,		gui_ControlFlag,				0,	wed_SelectParent	},
{	"Select &Children",	GUI_KEY_DOWN,	gui_ControlFlag,				0,	wed_SelectChild		},
{	"Select P&olygon",	GUI_KEY_UP,		gui_ControlFlag+gui_ShiftFlag,	0,	wed_SelectPoly		},
{	"Select &Vertices",	GUI_KEY_DOWN,	gui_ControlFlag+gui_ShiftFlag,	0,	wed_SelectVertex	},
#if AIRPORT_ROUTING
{	"-",				0,				0,								0,	0					},
{	"Select Zero-Length Edges",	0,		0,								0,	wed_SelectZeroLength },
{	"Select Double Nodes",	0,			0,								0,	wed_SelectDoubles	},
{	"Select Crossing Edges",	0,		0,								0,	wed_SelectCrossing	},
#endif
{	NULL,				0,				0,								0,	0					},
};

static const GUI_MenuItem_t kAirportMenu[] = {
{	"&Create Airport",			'A',	gui_ControlFlag+gui_ShiftFlag,			0, wed_CreateApt },
{	"Create ATC &Frequency",	'F',	gui_ControlFlag,						0, wed_AddATCFreq },
#if AIRPORT_ROUTING
{	"Create Airport Flow",		0,		0,										0, wed_AddATCFlow },
{	"Create Runway Use",		0,		0,										0, wed_AddATCRunwayUse },
{	"Create Runway Time Rule",	0,		0,										0, wed_AddATCTimeRule },
{	"Create Runway Wind rule",	0,		0,										0, wed_AddATCWindRule },

#endif
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

/*
	Ben says: what Matthias and Janos have done here warrants a little bit of explanation.
	Basically the GUI menu port isn't quite opaque to client code yet.  Unlike Win32,
	the Qt Menus used on Linux cannot be shared by multiple windows, thus we cannot
	really have a global set of menu resources (simulating the mac) that are installed
	in every window.

	So...right now we have a temporary call-out from the window code to re-generate
	a copy of the client-specific menu content.  This effectively means we have
	duplicate copies of the menu bar, one per window, which is what we want (since closing
	a window releases the qt menu bar.).

	So in the long term we probably need to do something like this:

	1. menu creation is a virtual function called in app object - guarantees the menu
	bar is established once before any windows are made.  (We know this because windows
	need the app as a commander so app is always created before any windows.)  Without
	this we have to broadcast out menu bar changes all over the place.

	2. rebuild menu item API becomes private.  In practice it is only used in
	implementations, so exposing it to client code implies we can revise menus when
	most clients still don't need this.

	(Note that clients CAN revise menus by changing the ioName param in a command
	evaluation callback).

	3. Qt app menu code persists the menus, and clones out a copy when a window is made.

*/

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
		
	GUI_Menu	objd_menu = inApp->CreateMenu(
		"&Object Density", kObjDensityMenu, view_menu, 10);

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
