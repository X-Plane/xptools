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

// Stuff we need to init
#include "XWidgetApp.h"
#include "XESInit.h"
#include "WED_ProcessingCmds.h"
#include "WED_FileCommands.h"
#include "WED_Document.h"
#include "WED_SpecialCommands.h"
#include "WED_MapView.h"
#include "WED_PrefsDialog.h"
#include "WED_Assert.h"
#include "DEMTables.h"
#include "ObjTables.h"
#include <CGAL/assertions.h>
#include "WED_Package.h"

#include "GUI_Pane.h"
#include "GUI_Fonts.h"
#include "XPLMGraphics.h"
#include "GUI_Window.h"

#include "XPWidgets.h"
#include "XPWidgetDialogs.h"

#if APL
#include "SIOUX.h"
#endif


// This stuff is only needed for the hack open .elv code.
#include "AptElev.h"
#include "WED_Globals.h"
#include "ParamDefs.h"

#include "GUI_Application.h"

class	WED_App : public GUI_Application {
public:
	virtual ~WED_App() { }
	virtual	void	OpenFiles(const vector<string>& inFiles) { }
};

class	GUI_Hack : public GUI_Pane {
public:

	GUI_Hack() { n = 0; p = 0; }

	virtual void		Draw(GUI_GraphState * state)
	{
		int b[4];
		GetBounds(b);
		state->SetState(0,0,0,  0, 0,   0, 0);
		glColor3f(0,1,0);
		if (this == GetFocus()) glColor3f(1,1,0);
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
	virtual int			MouseDown(int ix, int iy, int button) { x = ix; y = iy; b = button; p = 1; Refresh(); if (GetFocus() == this) this->LoseFocus(0); else this->TakeFocus(); return 1; }
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

#if 0
void	import_tiger_repository(const string& rt)
{
	string root(rt);
	int	fnum = -1;
	if ((root.length() > 3) &&
		(root.substr(root.length()-4)==".RT1" ||
		 root.substr(root.length()-4)==".rt1"))
	{
		root = root.substr(0, root.length()-4);
		fnum = atoi(root.c_str() + root.length() - 5);
		root.erase(root.rfind('/'));
	}
	if ((root.length() > 3) &&
		(root.substr(root.length()-4)==".zip" ||
		 root.substr(root.length()-4)==".ZIP"))
	{
		fnum = atoi(root.c_str() + root.length() - 9);
	}
	
	if (fnum == -1)
	{
		printf("Could not identify file %s as a TIGER file.\n", root.c_str());
	} else {

		MFFileSet * fs = FileSet_Open(root.c_str());
		if (fs)
		{	
			printf("Reading %s/TGR%05d.RT1\n", root.c_str(), fnum);
			TIGER_LoadRT1(fs, fnum);
			printf("Reading %s/TGR%05d.RT2\n", root.c_str(), fnum);
			TIGER_LoadRT2(fs, fnum);
			printf("Reading %s/TGR%05d.RTP\n", root.c_str(), fnum);
			TIGER_LoadRTP(fs, fnum);
			printf("Reading %s/TGR%05d.RTI\n", root.c_str(), fnum);
			TIGER_LoadRTI(fs, fnum);
			printf("Reading %s/TGR%05d.RT7\n", root.c_str(), fnum);
			TIGER_LoadRT7(fs, fnum);
			printf("Reading %s/TGR%05d.RT8\n", root.c_str(), fnum);
			TIGER_LoadRT8(fs, fnum);
							
			FileSet_Close(fs);
		} else 
			printf("Could not open %s as a file set.\n", root.c_str());
	}
}
#endif

//void	XGrindInit(string& outName)
int main(int argc, const char * argv[])
{
#if APL
//	SIOUXSettings.stubmode = true;
	SIOUXSettings.standalone = false;
	SIOUXSettings.setupmenus = false;
	SIOUXSettings.autocloseonquit = false;
	SIOUXSettings.asktosaveonclose = false;
#endif

	WED_App	app;

	gFailure = CGAL::set_error_handler(cgal_failure);
	XESInit();

	WED_LoadPrefs();
	
	LoadDEMTables();
	LoadObjTables();
	
	gPackage = new WED_Package("Master:code:XPTools:SceneryTools:TestPackage1", true);
	WED_Document * doc = gPackage->NewTile(-72, 42);
	
//	int w, h;
//	XPLMGetScreenSize(&w, &h);
	RegisterFileCommands();
//	WED_MapView *	map_view = new WED_MapView(20, h - 20, w - 20, 20, 1, NULL, doc);
	RegisterProcessingCommands();
	RegisterSpecialCommands();
	
	WED_AssertInit();
	
	XPInitDefaultMargins();
	
	{
	int wb[4] = { 100, 100, 500, 500 };
	GUI_Window * test_win = new GUI_Window("Test", wb);
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
	GUI_Window * test_win = new GUI_Window("Test", wb);
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
	}
//	test_win->Show();

	app.Run();
}


bool	XGrindCanQuit(void)
{
	return true;
}

void	XGrindDone(void)
{
	WED_SavePrefs();
	delete gPackage;
}