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

// Stuff we need to init
#include "XESInit.h"
#include "WED_Document.h"
#include "WED_PrefsDialog.h"
#include "WED_Assert.h"
#include "DEMTables.h"
#include "WED_AboutBox.h"
#include "ObjTables.h"
#include <CGAL/assertions.h>
#include "GUI_Clipboard.h"
#include "WED_Package.h"
#include "WED_Application.h"
#include "WED_AboutBox.h"

#include "GUI_Pane.h"
#include "GUI_Fonts.h"
#include "GUI_Window.h"

#include "XPWidgets.h"
#include "XPWidgetDialogs.h"

#include "WED_Menus.h"


#include "GUI_ScrollerPane.h"
#include "GUI_Textfield.h"
#include "GUI_Splitter.h"

#define	REGISTER_LIST	\
	_R(WED_Airport) \
	_R(WED_AirportBeacon) \
	_R(WED_AirportBoundary) \
	_R(WED_AirportChain) \
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
	_R(WED_Windsock)

#define _R(x)	extern void x##_Register();
REGISTER_LIST
#undef _R

#include "WED_EnumSystem.h"

#if IBM
HINSTANCE gInstance = NULL;
#endif

CGAL::Failure_function	gFailure = NULL;
void	cgal_failure(const char* a, const char* b, const char* c, int d, const char* e)
{
	if (gFailure)
		(*gFailure)(a, b, c, d, e);
	throw a;
}

#if IBM
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, const char * argv[])
#endif
{
#if IBM
	gInstance = hInstance;
#endif
	GUI_InitClipboard();
	WED_Application	app;
	
	// Ben says: the about box is actually integral to WED's operation.  WED uses a series of shared OGL contexts to hold
	// our textures, and WED cannot handle the textures being thrown away and needing a reload.  So logically we must have
	// at least one shared context so that the textures are not purged.	
	// This means one window must always be in existence.  That window is the about box...which stays hidden but allocated to
	// sustain OpenGL.

	WED_AboutBox * about = new WED_AboutBox(&app);
	about->Start(1.0);
	about->Refresh();
	about->UpdateNow();

	WED_MakeMenus(&app);
	
	gFailure = CGAL::set_error_handler(cgal_failure);
	XESInit();

	LoadDEMTables();
	LoadObjTables();
	
	WED_AssertInit();
	ENUM_Init();
	
	#define _R(x)	x##_Register();
	REGISTER_LIST
	#undef _R
	
	app.SetAbout(about);
	app.Run();
	
	delete about;
	return 0;
}
