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

#include "WED_Application.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "GUI_Resources.h"
#include "PlatformUtils.h"
#include "GUI_Help.h"
#include "WED_UIDefs.h"
#include "WED_Menus.h"
#include "WED_PackageMgr.h"
#include "WED_Url.h"

#if LIN
WED_Application::WED_Application(int& argc, char* argv[]) : GUI_Application(argc, argv),
#elif APL
WED_Application::WED_Application() : GUI_Application("WEDMainMenu"),
#else
	WED_Application::WED_Application() :
#endif
		mAboutBox(NULL)
{
}

WED_Application::~WED_Application()
{
}

void	WED_Application::SetAbout(GUI_Window * about_box)
{
	mAboutBox = about_box;
}

void	WED_Application::OpenFiles(const vector<string>& inFiles)
{
}

int		WED_Application::HandleCommand(int command)
{
	char buf[1024];
	int pack_bounds[4] = { 50, 50, 550, 300 };
	buf[0] = 0;

	switch(command) {
	case wed_HelpScenery:
		// LR maintains a forwarding directory for all v10-class products 
		// so that we can restructure our content management without breaking binary
		// apps in-field.  So...this is the perma-marker for WED 1.1 scenery help.
		GUI_LaunchURL(WED_URL_HELP_SCENERY);
		return 1;
	case wed_HelpManual:
		{
			// We used to have a nice PDF published with WED, but...WED is changing fast
			// and it stops going final to have to wait for doc complete.  So let's put
			// the manual online and off we go.
//			string path;
//			if (GUI_GetTempResourcePath("WEDManual.pdf",path))
//			{
//				GUI_LaunchURL(path.c_str());
//			}
			GUI_LaunchURL(WED_URL_MANUAL);
		}
		return 1;
	default:
		return GUI_Application::HandleCommand(command);
	}
}

int		WED_Application::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case gui_Undo:		ioName = "&Undo"; return 0;
	case gui_Redo:		ioName = "&Redo"; return 0;
	case wed_HelpScenery:	return 1;
	case wed_HelpManual:	return 1;
	default:				return GUI_Application::CanHandleCommand(command, ioName, ioCheck);
	}
}

void	WED_Application::AboutBox(void)
{
	if (mAboutBox)	mAboutBox->Show();
	else			DoUserAlert("WED 1.0 by Ben Supnik.");
}

void	WED_Application::Preferences(void)
{
	DoUserAlert("WED does not yet have any preferences.");
}

bool	WED_Application::CanQuit(void)
{
//	if (ConfirmMessage("Are you sure you want to quit WED", "Quit", "Cancel"))	return true;	return false;
	return WED_Document::TryCloseAll();
}

