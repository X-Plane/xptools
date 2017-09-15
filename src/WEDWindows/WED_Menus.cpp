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
#include "WED_MetaDataKeys.h"
#include "GUI_Application.h"
#if APL
#include "ObjCUtils.h"
#endif

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
{	"Target X-Plane Version",0,		0,								0,	0					},
{	"-",					0,		0,								0,	0					},
{	"&Import apt.dat...",	'I',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_ImportApt		},
{	"Import DS&F...",		0,		0,								0,	wed_ImportDSF		},
{	"Import Ortho&photo...", 0,		0,								0,	wed_ImportOrtho		},
#if HAS_GATEWAY
{	"Import from Airport Scenery Gateway...",0,0,				0,	wed_ImportGateway	},
#endif
#if GATEWAY_IMPORT_FEATURES
{	"Import Airport Scenery Gateway Extracts...",0,0,				0,	wed_ImportGatewayExtract },
#endif
{	"-",					0,		0,								0,	0					},
{	"&Export apt.dat...",	'S',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_ExportApt		},
{	"Export Scenery Pac&k",	'B',	gui_ControlFlag,				0,	wed_ExportPack		},
#if HAS_GATEWAY
{	"Export to Airport Scenery Gateway...",0,	0,							0,	wed_ExportToGateway	},
#endif
#if IBM || LIN
{	"-",					0,		0,								0,	0					},
{	"&Preferences...",		0,		0,								0,	gui_Prefs			},
{	"-",					0,		0,								0,	0					},
{	"E&xit",				0,		0,								0,	gui_Quit			},
#endif
{	NULL,					0,		0,								0,	0					},
};

static const GUI_MenuItem_t kExportTargetMenu[] = {
{	"X-Plane 9.70",			0,		0,								0,	wed_Export900		},
{	"X-Plane 10.00",		0,		0,								0,	wed_Export1000		},
{	"X-Plane 10.21",		0,		0,								0,	wed_Export1021,		},
{	"X-Plane 10.50",		0,		0,								0,	wed_Export1050,		},
{	"X-Plane 11.00",		0,		0,								0,	wed_Export1100,		},
{	"Airport Scenery Gateway",0,	0,								0,	wed_ExportGateway	},
{	NULL,					0,		0,								0,	0					}
};


static const GUI_MenuItem_t	kEditMenu[] = {
{	"&Undo",				'Z',	gui_ControlFlag,				0,	gui_Undo		},
{	"&Redo",				'Z',	gui_ControlFlag+gui_ShiftFlag,	0,	gui_Redo		},
{	"-",					0,  	0,								0,	0				},
{	"Cu&t",					'X',	gui_ControlFlag,				0,	gui_Cut			},
{	"&Copy",				'C',	gui_ControlFlag,				0,	gui_Copy		},
{	"&Paste",				'V',	gui_ControlFlag,				0,	gui_Paste		},
{	"Cl&ear",				0,		0,								0,	gui_Clear		},	// we could use GUI_KEY_DELETE but having del as cmd key screws up text fields.
{	"&Duplicate",			'D',	gui_ControlFlag+gui_ShiftFlag,	0,	gui_Duplicate	},
{	"-",					0,  	0,								0,	0				},
{	"&Group",				'G',	gui_ControlFlag,				0,	wed_Group		},
{	"U&ngroup",				'G'	,	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Ungroup		},
{	"-",					0,  	0,								0,	0				},
{	"Spl&it",				'E',	gui_ControlFlag,				0,	wed_Split		},
{	"A&lign",				'L',	gui_ControlFlag,				0,	wed_Align		},
{	"&Orthogonalize",		'Q',	gui_ControlFlag,				0,	wed_Orthogonalize },
{	"Make Regular Poly",	'Q',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_RegularPoly },
#if AIRPORT_ROUTING
{	"Merge",				'M',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Merge		},
#endif
{	"Rever&se",				'R',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_Reverse		},
{	"Rotate",				'R',	gui_ControlFlag,				0,	wed_Rotate		},
{	"Cr&op Unselected",		0,		0,								0,	wed_Crop		},
//{	"Make Draped Pol&ygons",0,		0,								0,	wed_Overlay		},
#if AIRPORT_ROUTING
//{	"Make Routing",			0,		0,								0,	wed_MakeRouting },
#endif
{	"-",					0,  	0,								0,	0				},
{	"Move &First",			'[',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_MoveFirst	},
{	"&Move Up",				'[',	gui_ControlFlag,				0,	wed_MovePrev	},
{	"Move Do&wn",			']',	gui_ControlFlag,				0,	wed_MoveNext	},
{	"Move &Last",			']',	gui_ControlFlag+gui_ShiftFlag,	0,	wed_MoveLast	},
{	"Explode Special Agps",   0,    0,                              0,  wed_BreakApartSpecialAgps  },
{	"Replace Vehicle Objects", 0,	0,								0,  wed_ReplaceVehicleObj	},
{	NULL,					0,		0,								0,	0				},
};

static const GUI_MenuItem_t kViewMenu[] = {
{	"Zoom Worl&d",				'/',gui_ControlFlag+gui_ShiftFlag,			0,	wed_ZoomWorld		},	// This conflicts with a Mac key strkoe but zoom world is not THAT useful
{	"&Zoom Package",			'/',gui_ControlFlag+gui_OptionAltFlag,		0,	wed_ZoomAll			},
{	"Zoom &Selection",			'/',gui_ControlFlag,						0,	wed_ZoomSelection	},	// simple cmd-slash for MOST imoprtant zoom command!
{	"-",						0,	0,										0,	0					},
{	"&Feet",					0,	0,										0,	wed_UnitFeet		},
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
{	"To&ggle Preview",			0,	0,										0,	wed_TogglePreview	},
#if WANT_TERRASEVER
{	"Toggle &Terraserver",		0,	0,										0,	wed_ToggleTerraserver },
#endif
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
{	"Select Conn&ected",			0,			0,							0,	wed_SelectConnected },
#if AIRPORT_ROUTING
{	"-",						0,			0,							0,	0					},
{	"Select &Degenerate Edges",	0,			0,							0,	wed_SelectZeroLength },
{	"Select Do&uble Nodes",		0,			0,							0,	wed_SelectDoubles	},
{	"Select Crossing Ed&ges",	0,			0,							0,	wed_SelectCrossing	},
#endif
{	"-",						0,			0,							0,	0					},
{	"Select Local Ob&jects",		0,			0,							0,	wed_SelectLocalObjects },
{	"Select L&ibrary Objects",	0,			0,							0,	wed_SelectLibraryObjects },
{	"Select &Laminar Library Objects",0,		0,							0,	wed_SelectDefaultObjects },
{	"Select &Third Party Library Objects",0,	0,							0,	wed_SelectThirdPartyObjects },
{	"Select &Missing Objects",	0,			0,							0,	wed_SelectMissingObjects },
{	NULL,						0,			0,							0,	0					},
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
{	"Add &Metadata",			0,		0,										0, 0 },
{	"Update Metadata",			0,		0,										0, wed_UpdateMetadata},
{	"No Airport Selected",		'E',	gui_ControlFlag+gui_ShiftFlag,			0, wed_EditApt	},
{	"-",							0,		0,									0,	0			},
{	"Upgrade Ramps",				0,		0,									0,	wed_UpgradeRamps},
{	"Auto Rename Runways",			0,		0,									0,	wed_RenameRwys},
{	NULL,						0,		0,										0, 0,				}
};

// end-begin?  YES!  Since begin is before the beginnined and end is AFTER the end this gives us ONE extra slot.
// We NEED that slot to be the null terminator for the menu list.
static GUI_MenuItem_t kAddMetaDataMenu[wed_AddMetaDataEnd-wed_AddMetaDataBegin] = { 0 };

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

	GUI_Menu export_target_menu = inApp->CreateMenu(
		"Export Target", kExportTargetMenu, file_menu, 9);

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
		"&Airport", kAirportMenu, inApp->GetMenuBar(), 0);
	
	for (KeyEnum key_enum = wed_AddMetaDataBegin + 1; key_enum < wed_AddMetaDataEnd; ++key_enum)
	{
		int index = key_enum - wed_AddMetaDataBegin - 1;
		GUI_MenuItem_t menu_item = { META_KeyDisplayText(key_enum).c_str(), 0, 0, 0, key_enum };
		kAddMetaDataMenu[index] = menu_item;
	}

	GUI_Menu	airport_add_meta_data_menu = inApp->CreateMenu(
		"Add &Meta Data", kAddMetaDataMenu, airport_menu, 6);//This hardcoded 6 is a reference to
															 //kAirportMenu[6]
	GUI_Menu	help_menu = inApp->CreateMenu(
		"&Help", kHelpMenu, inApp->GetMenuBar(), 0);
}
