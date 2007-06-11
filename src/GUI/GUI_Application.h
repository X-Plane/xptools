#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#if APL
#define __DEBUGGING__
#include <Carbon/Carbon.h>
#endif

#include "GUI_Commander.h"

/*
	WINDOWS WARNING: MENUS
	
	Windows has the following limitations on the menu system:
	
	1. App menus cannot be created dynamically on the fly because:
	
	- They are replicated as each window is made, but existing windows will not receive the new additions.
	- Accelerators are only set up at startup (so all menus must be set up before app-run.
	
*/

class	GUI_Application : public GUI_Commander {
public:
					 GUI_Application();
	virtual			~GUI_Application();

	// APPLICATION API	
	void			Run(void);	
	void			Quit(void);

	// MENU API
	GUI_Menu		GetMenuBar(void);
	GUI_Menu		GetPopupContainer(void);
	
	GUI_Menu		CreateMenu(const char * inTitle, const GUI_MenuItem_t	items[], GUI_Menu parent, int parent_item);
	void			RebuildMenu(GUI_Menu menu, const GUI_MenuItem_t	items[]);

	virtual	void	AboutBox(void)=0;
	virtual	void	Preferences(void)=0;
	virtual	bool	CanQuit(void)=0;
	virtual	void	OpenFiles(const vector<string>& inFiles)=0;

	// From GUI_Commander - app never refuses focus!
	virtual	int				AcceptTakeFocus(void) 	{ return 1; }
	virtual	int				HandleCommand(int command);
	virtual	int				CanHandleCommand(int command, string& ioName, int& ioCheck);


private:

#if APL

	EventHandlerUPP		mMacEventHandlerUPP;
	EventHandlerUPP		mSiouxSnifferUPP;
	EventHandlerRef		mMacEventHandlerRef;
	EventHandlerRef		mSiouxSnifferRef;
	AEEventHandlerUPP	mHandleOpenDocUPP;
	
	static pascal OSStatus	MacEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSStatus 	SiouxSniffer(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSErr 	HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);


#endif

	set<GUI_Menu>		mMenus;
	bool				mDone;

};

extern	GUI_Application *	gApplication;

#endif


