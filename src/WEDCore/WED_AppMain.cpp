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
#include "ObjTables.h"
#include <CGAL/assertions.h>
#include "GUI_Clipboard.h"
#include "WED_Package.h"
#include "WED_Application.h"

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



class	GUI_Hack : public GUI_Pane {
public:

	GUI_Hack() { n = 0; p = 0; }

	virtual void		Draw(GUI_GraphState * state)
	{
		int b[4];
		GetBounds(b);
		state->SetState(0,0,0,  0, 0,   0, 0);
		glColor3f(0,1,0);
//		if (this == GetFocus()) glColor3f(1,1,0);
		glBegin(GL_LINE_LOOP);
		glVertex2i(b[0], b[1]);
		glVertex2i(b[0], b[3]);
		glVertex2i(b[2], b[3]);
		glVertex2i(b[2], b[1]);
		glEnd();
		
		if (p)
		{
			glPointSize(5);
			glBegin(GL_POINTS);
			glVertex2i(x,y);
			glEnd();
			glPointSize(1);
		}
		
		float c[3] = { 1, 0, 1 };
		char buf[50];
		sprintf(buf,"V=%d %s\n", n, str.c_str());
		GUI_FontDrawScaled(state, font_UI_Basic, c, b[0], b[1], b[2], b[3], buf, buf + strlen(buf), (n < 0) ? align_Left : ((n == 0) ? align_Center: align_Right));
	}
	virtual int			MouseDown(int ix, int iy, int button) { x = ix; y = iy; b = button; p = 1; Refresh(); return 1; }
	virtual void		MouseDrag(int ix, int iy, int button) { x = ix; y = iy; b = button; Refresh(); }
	virtual void		MouseUp(int ix, int iy, int button) { x = ix; y = iy; b = button; p = 0; Refresh(); }
	virtual int			ScrollWheel(int x, int y, int dist, int axis) { if (axis == 0) n += dist; Refresh(); return 1; }
	virtual int			KeyPress(char c, int v, int f) { if (!(f & gui_UpFlag)) str += c; Refresh(); return 1; }
	
	virtual int	AcceptTakeFocus(void) { return 1; }
	
	string str;
	int n, p, x, y, b;
};	


CGAL::Failure_function	gFailure = NULL;
void	cgal_failure(const char* a, const char* b, const char* c, int d, const char* e)
{
	if (gFailure)
		(*gFailure)(a, b, c, d, e);
	throw a;
}


void	XGrindDragStart(int x, int y)
{
}

void	XGrindDragOver(int x, int y)
{
}

#if 0
void	XGrindDragLeave(void)
{
}

void	XGrindFiles(const vector<string>& fileList, int x, int y)
{
	for (vector<string>::const_iterator i = fileList.begin();
		i != fileList.end(); ++i)
	{
		if (i->find(".elv") != string::npos)		
		{
//			BuildDifferentialDegree(i->c_str(), gDocument->gDem[dem_Elevation].mWest, gDocument->gDem[dem_Elevation].mSouth, 1201, 1201, gDocument->gDem[dem_Elevation], false);
//			BuildDifferentialDegree(i->c_str(), gDocument->gDem[dem_Elevation].mWest, gDocument->gDem[dem_Elevation].mSouth, 241, 241, gDocument->gDem[dem_UrbanDensity], true);
			continue;
		}

#if !DEV
restore this
#endif
//		WED_FileOpen(*i);
	}
}
#endif

//void	XGrindInit(string& outName)
int main(int argc, const char * argv[])
{
	GUI_InitClipboard();
	WED_Application	app;

	WED_MakeMenus(&app);
	
	gFailure = CGAL::set_error_handler(cgal_failure);
	XESInit();

//	WED_LoadPrefs();
	
	LoadDEMTables();
	LoadObjTables();
	
//	gPackage = new WED_Package("Macintosh HD:code:XPTools:SceneryTools:TestPackage1", true);
//	WED_Document * doc = gPackage->NewTile(-72, 42);
	
//	int w, h;
//	XPLMGetScreenSize(&w, &h);
//	RegisterFileCommands();
//	WED_MapView *	map_view = new WED_MapView(20, h - 20, w - 20, 20, 1, NULL, doc);
//	RegisterProcessingCommands();
//	RegisterSpecialCommands();
	
	WED_AssertInit();
	ENUM_Init();
	
	#define _R(x)	x##_Register();
	REGISTER_LIST
	#undef _R

	
//	XPInitDefaultMargins();
/*
	int wb[4] = { 100, 100, 500, 500 };
	int wc[4] = { 0, 0, 400, 400 };
	GUI_Window * test_win = new GUI_Window("Test", wb, &app);

	GUI_Splitter * splitter = new GUI_Splitter(gui_Split_Vertical);
	splitter->SetParent(test_win);
	splitter->Show();
	splitter->SetBounds(wc);
	splitter->SetSticky(1,1,1,1);

	GUI_ScrollerPane * sp1 = new GUI_ScrollerPane(1, 1);
	GUI_TextField * tf1 = new GUI_TextField(true, test_win);
	tf1->SetParent(sp1);
	sp1->SetParent(splitter);
	sp1->PositionInContentArea(tf1);
	sp1->SetContent(tf1);
	sp1->Show();
	tf1->Show();
	sp1->SetSticky(1,1,1,1);
	tf1->SetWidth(700);


	GUI_ScrollerPane * sp2 = new GUI_ScrollerPane(0, 1);
	GUI_TextField * tf2 = new GUI_TextField(true, test_win);
	tf2->SetParent(sp2);
	sp2->SetParent(splitter);
	sp2->PositionInContentArea(tf2);
	sp2->SetContent(tf2);
	sp2->Show();
	tf2->Show();
	sp2->SetSticky(1,0,1,1);
	tf2->SetWidth(700);

	sp1->AttachSlaveH(sp2);

	splitter->AlignContents();
	test_win->Show();

	const char * str = "Thsi is the very model of a modern major general.\nFor information vegetable animal and mineral.  I know the kings of england and I quote the "
		"something historical from marithon to waterloo in order catregoeryical!";
	tf1->DoReplaceText(0,0,str,str+strlen(str));
	tf1->SetSelection(20, 24);
*/
	/*
	{
	int wb[4] = { 100, 100, 500, 500 };
	GUI_Window * test_win = new GUI_Window("Test", wb, &app);
	GUI_Hack * h1 = new GUI_Hack;
	GUI_Hack * h2 = new GUI_Hack;
	GUI_Hack * h3 = new GUI_Hack;
	
	h1->SetParent(test_win);
	h1->SetBounds(10, 10, 390, 25);
	h1->SetSticky(1, 1, 1, 0);
	h1->Show();

	h2->SetParent(test_win);
	h2->SetBounds(10, 30, 390, 370);
	h2->SetSticky(1, 1, 1, 1);
	h2->Show();

	h3->SetParent(test_win);
	h3->SetBounds(10, 375, 390, 390);
	h3->SetSticky(1, 0, 1, 1);
	h3->Show();

	float c[4] = { 0.4, 0.4, 0.4, 0.0 };
	test_win->SetClearSpecs(true, false, c);
	
	test_win->SetBounds(80, 80, 500, 500);
	test_win->SetDescriptor("w1.\n");
	}
	{
	int wb[4] = { 100, 100, 500, 500 };
	GUI_Window * test_win = new GUI_Window("Test", wb, &app);
	GUI_Hack * h1 = new GUI_Hack;
	GUI_Hack * h2 = new GUI_Hack;
	GUI_Hack * h3 = new GUI_Hack;
	
	h1->SetParent(test_win);
	h1->SetBounds(10, 10, 25, 390);
	h1->SetSticky(1, 1, 0, 1);
	h1->Show();

	h2->SetParent(test_win);
	h2->SetBounds(30, 10, 370, 390);
	h2->SetSticky(1, 1, 1, 1);
	h2->Show();

	h3->SetParent(test_win);
	h3->SetBounds(375, 10, 390, 390);
	h3->SetSticky(0, 1, 1, 1);
	h3->Show();

	float c[4] = { 0.4, 0.4, 0.4, 0.0 };
	test_win->SetClearSpecs(true, false, c);
	
	test_win->SetBounds(80, 80, 500, 500);
	test_win->SetDescriptor("w2.\n");
	}*/
//	test_win->Show();


	app.Run();
}

#if 0
bool	XGrindCanQuit(void)
{
	return true;
}

void	XGrindDone(void)
{
	WED_SavePrefs();
	delete gPackage;
}
#endif
