#include "WED_PackageWindow.h"
#include "WED_Package.h"
#include "WED_Menus.h"
#include "WED_Messages.h"
#include "WED_PackageStatusPane.h"

#include "GUI_ScrollerPane.h"
#include "GUI_SimpleScroller.h"

WED_PackageWindow::WED_PackageWindow(
				 		const char * 	inTitle, 
				 		int 			inBounds[4],
				 		GUI_Commander * inCommander,
				 		WED_Package *	inPackage) :
	GUI_Window(inTitle, inBounds, inCommander),
	mPackage(inPackage)
{
	inPackage->AddListener(this);

	GUI_ScrollerPane * scroller = new GUI_ScrollerPane(1,1);
	scroller->SetParent(this);
	scroller->SetBounds(0,16,inBounds[2] - inBounds[0], inBounds[3] - inBounds[1]);
	scroller->SetSticky(1,1,1,1);
	scroller->Show();
	
	GUI_SimpleScroller * contents = new GUI_SimpleScroller;
	contents->SetParent(scroller);
	contents->Show();
	scroller->SetContent(contents);
	scroller->PositionInContentArea(contents);
	
	WED_PackageStatusPane * p = new WED_PackageStatusPane(inPackage, inCommander);
	p->SetParent(contents);
	p->SetBounds(0,0,360 * 8, 180 * 8);
	p->Show();
	
	scroller->ContentGeometryChanged();
}

WED_PackageWindow::~WED_PackageWindow()
{
	printf("Package window (empty dtor.)\n");
}
	
int	WED_PackageWindow::KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	return 0;
}

int	WED_PackageWindow::HandleCommand(int command)
{
	switch(command) {
	case gui_Close:
	#if !DEV
	not safe!
	#endif
		delete mPackage;
		return 1;
	default:	
		return 0;
	}
}

int	WED_PackageWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case gui_Close:	return 1;
	default:		return 0;
	}
}

void	WED_PackageWindow::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	if (inSrc == mPackage && inMsg == msg_PackageDestroyed)
		delete this;
#if !DEV
	THIS IS NOT SAFE IN THE FUTURE - WE NEED GARBAGE COLLECTION!
#endif	
}

bool	WED_PackageWindow::Closed(void)
{
	delete mPackage;
	return false;
}
