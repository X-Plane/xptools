#include "WED_Application.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "PlatformUtils.h"
#include "GUI_Help.h"
#include "WED_UIDefs.h"
#include "WED_Menus.h"

const char * LastPart(const char * s)
{
	const char * ret = s;
	while (*s)
	{
		if (*s==':'||*s=='/'||*s=='\\') ret=s;
		++s;		
	}
	return ret;
}



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
	case wed_NewPackage:
		if (GetFilePathFromUser(getFile_Save, "Please name your new scenery package", "Create", FILE_DIALOG_NEW_PROJECT, buf, sizeof(buf)))
		{
			try {
			
				double b[4] = { -180, -90, 180, 90 };
				WED_Document * doc = new WED_Document(buf, b);				
				WED_DocumentWindow * wind = new WED_DocumentWindow(LastPart(buf), this, doc);
			} catch (...) {
				DoUserAlert("An error occurred");
				#if ERROR_HANDLING
				fix this					
				#endif
			}
		}
		return 1;
	case wed_OpenPackage:
		if (GetFilePathFromUser(getFile_PickFolder, "Please pick your scenery package", "Open", FILE_DIALOG_OPEN_PROJECT, buf, sizeof(buf) ))
		{
			try {
				double b[4] = { -180, -90, 180, 90 };
				WED_Document * doc = new WED_Document(buf, b);				
				WED_DocumentWindow * wind = new WED_DocumentWindow(LastPart(buf), this, doc);

			} catch (...) {
				DoUserAlert("An error occurred");
				#if ERROR_HANDLING
				fix this					
				#endif
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
	case wed_NewPackage:
	case wed_OpenPackage:	return 1;
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