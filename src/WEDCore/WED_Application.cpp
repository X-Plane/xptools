#include "WED_Application.h"
#include "WED_Package.h"
#include "WED_PackageWindow.h"
#include "PlatformUtils.h"
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



WED_Application::WED_Application()
{
}

WED_Application::~WED_Application()
{
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
	case wed_NewPackage:
		if (GetFilePathFromUser(getFile_Save, "Please name your new scenery package", "Create", FILE_DIALOG_NEW_PROJECT, buf, sizeof(buf)))
		{
			try {
				WED_Package * pack = new WED_Package(buf, true);
				WED_PackageWindow * wind = new WED_PackageWindow(LastPart(buf), pack_bounds, this, pack);
			} catch (...) {
				DoUserAlert("An error occurred");
				#if !DEV
				fix this					
				#endif
			}
		}
		return 1;
	case wed_OpenPackage:
		if (GetFilePathFromUser(getFile_PickFolder, "Please pick your scenery package", "Open", FILE_DIALOG_OPEN_PROJECT, buf, sizeof(buf) ))
		{
			try {
				WED_Package * pack = new WED_Package(buf, false);
				WED_PackageWindow * wind = new WED_PackageWindow(LastPart(buf), pack_bounds, this, pack);
			} catch (...) {
				DoUserAlert("An error occurred");
				#if !DEV
				fix this					
				#endif
			}
		}
		return 1;
	default:
		return 0;
	}
}

int		WED_Application::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) { 
	case wed_NewPackage:
	case wed_OpenPackage:	return 1;
	default:				return 0;
	}
}
