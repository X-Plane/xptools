/*
 * Copyright (c) 2004, Laminar Research.
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

 #define __DEBUGGING__
#if DEV
//Adds the abilty to bring up a console for debuging
#include <stdio.h>
#endif

#include "WED_AboutBox.h"
// Stuff we need to init
//#include "XESInit.h"
#include "WED_Document.h"
#include "WED_Assert.h"
//#include "DEMTables.h"
#include "WED_AboutBox.h"
#include "WED_StartWindow.h"
//#include "ObjTables.h"
#include "GUI_Clipboard.h"
#include "WED_Package.h"
#include "GUI_Resources.h"
#include "WED_Application.h"
#include "WED_PackageMgr.h"
#include "GUI_Pane.h"
#include "GUI_Fonts.h"
#include "GUI_Window.h"
#include "GUI_Prefs.h"

#include "WED_Menus.h"

#include "GUI_ScrollerPane.h"
#include "GUI_TextField.h"
#include "GUI_Splitter.h"

#include "WED_FileCache.h"

#define	REGISTER_LIST	\
	_R(WED_Airport) \
	_R(WED_AirportBeacon) \
	_R(WED_AirportBoundary) \
	_R(WED_AirportChain) \
	_R(WED_Ring) \
	_R(WED_AirportNode) \
	_R(WED_AirportSign) \
	_R(WED_Group) \
	_R(WED_Helipad) \
	_R(WED_KeyObjects) \
	_R(WED_LightFixture) \
	_R(WED_ObjPlacement) \
	_R(WED_RampPosition) \
	_R(WED_Root) \
	_R(WED_Runway) \
	_R(WED_RunwayNode) \
	_R(WED_Sealane) \
	_R(WED_Select) \
	_R(WED_Taxiway) \
	_R(WED_TowerViewpoint) \
	_R(WED_Windsock) \
	_R(WED_ATCFrequency) \
	_R(WED_TextureNode) \
	_R(WED_TextureBezierNode) \
	_R(WED_OverlayImage) \
	_R(WED_SimpleBoundaryNode) \
	_R(WED_SimpleBezierBoundaryNode) \
	_R(WED_LinePlacement) \
	_R(WED_StringPlacement) \
	_R(WED_ForestPlacement) \
	_R(WED_FacadePlacement) \
	_R(WED_PolygonPlacement) \
	_R(WED_DrapedOrthophoto) \
	_R(WED_ExclusionZone) \
	_R(WED_ForestRing) \
	_R(WED_FacadeRing) \
	_R(WED_FacadeNode)

#define REGISTER_LIST_ATC \
	_R(WED_TaxiRoute) \
	_R(WED_TaxiRouteNode) \
	_R(WED_ATCFlow) \
	_R(WED_ATCTimeRule) \
	_R(WED_ATCWindRule) \
	_R(WED_ATCRunwayUse) \
	_R(WED_RoadEdge)

#define _R(x)	extern void x##_Register();
REGISTER_LIST
#if AIRPORT_ROUTING
REGISTER_LIST_ATC
#endif
#undef _R

#include "WED_EnumSystem.h"

#if IBM
HINSTANCE gInstance = NULL;
#endif
#if LIN
#include "initializer.h"
#endif


#if IBM
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char * argv[])
#endif
{
#if IBM
	gInstance = hInstance;
	SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
#endif
	GUI_MemoryHog::InstallNewHandler();
	GUI_InitClipboard();
#if LIN
	Initializer linit(&argc, &argv, false);
	WED_Application	app(argc, argv);
#else
	WED_Application	app;
#endif
	WED_PackageMgr	pMgr(NULL);

	#if IBM && DEV
		//Creates a console window to use as a debuging place and starts it minimized
		if(AllocConsole())
		{
				freopen("CONOUT$", "w",stdout);
				SetConsoleTitle(L"Debug Window");
				ShowWindowAsync(GetConsoleWindow(),SW_SHOWNORMAL);
				ShowWindowAsync(GetConsoleWindow(),SW_SHOWMINNOACTIVE);
		}
	#endif

	// Ben says: the about box is actually integral to WED's operation.  WED uses a series of shared OGL contexts to hold
	// our textures, and WED cannot handle the textures being thrown away and needing a reload.  So logically we must have
	// at least one shared context so that the textures are not purged.
	// This means one window must always be in existence.  That window is the about box...which stays hidden but allocated to
	// sustain OpenGL.

	WED_AboutBox * about = new WED_AboutBox(&app);
	WED_MakeMenus(&app);
	#if LIN
	//mroe: resize after update the menubar
	about->Resize(about->centralWidget()->width(),about->centralWidget()->height());
	#endif
	WED_StartWindow * start = new WED_StartWindow(&app);

	start->Show();

	start->ShowMessage("Initializing...");
//	XESInit();

	start->ShowMessage("Reading Prefs...");
	GUI_Prefs_Read("WED");
	WED_Document::ReadGlobalPrefs();

	start->ShowMessage("Scanning X-System Folder...");
	pMgr.SetXPlaneFolder(GUI_GetPrefString("packages","xsystem",""));

	start->ShowMessage("Initializing WED File Cache");
	WED_file_cache_init();
//	start->ShowMessage("Loading DEM tables...");
//	LoadDEMTables();
//	start->ShowMessage("Loading OBJ tables...");
//	LoadObjTables();

	start->ShowMessage("Loading ENUM system...");
	WED_AssertInit();
	ENUM_Init();

	start->ShowMessage("Registering classes...");
	#define _R(x)	x##_Register();
	REGISTER_LIST
	#if AIRPORT_ROUTING
	REGISTER_LIST_ATC
	#endif
	#undef _R

	for (int y = 0; y < 2; ++y)
	for (int x = 0; x < 4; ++x)
	{
		char	fname[30];
		sprintf(fname,"earth_%d%d.jpg",x+1,y+1);
		int	tex_id = GUI_GetTextureResource(fname, 0, NULL);
		sprintf(fname,"Loading earth_%d%d.jpg",x+1,y+1);
		start->ShowMessage(fname);
	}

	app.SetAbout(about);

	start->ShowMessage(string());
	setlocale(LC_ALL,"C");
	#if LIN
	//TODO:mroe: maybe we can set this to LC_ALL for all other OS's .
	//In the case of linux we must , since standard C locale is not utf-8	
	setlocale(LC_CTYPE,"en_US.UTF-8");
	//mroe: for now , every CString holding a filepath must be wrapped by QString::fromUtf8() to convert to QString.	
	//(mainly in the dialogs and the window caption to show it to the user)
	//Setting this global for the app could be the better solution.
	//QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	#endif
	app.Run();
    // we're out of eventloop here, deleting windows on linux implies sending messages to them,
    // so this would fail.
	#if !LIN
	    delete about;
	#endif

	GUI_MemoryHog::RemoveNewHandler();

	string xsys;
	pMgr.GetXPlaneFolder(xsys);
	GUI_SetPrefString("packages","xsystem",xsys.c_str());

	WED_Document::WriteGlobalPrefs();
	GUI_Prefs_Write("WED");

	return 0;
}
