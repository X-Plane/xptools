#include "WED_Application.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "GUI_Resources.h"
#include "PlatformUtils.h"
#include "GUI_Help.h"
#include "WED_UIDefs.h"
#include "WED_Menus.h"
#include "WED_PackageMgr.h"

WED_Application::WED_Application() : mAboutBox(NULL)
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
		GUI_LaunchURL("http://scenery.x-plane.com/");
		return 1;
	case wed_HelpManual:
		{
			string path;
			if (GUI_GetTempResourcePath("WEDManual.pdf",path))
			{
				GUI_LaunchURL(path.c_str());
			}
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
	case wed_HelpScenery:
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