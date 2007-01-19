#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#if APL
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#endif

#if APL
#define GUI_Menu MenuRef
#elif IBM
#define GUI_Menu HMENU
#endif


#include "GUI_Commander.h"

struct	GUI_MenuItem_t {
	const char *	name;
	char			key;
	GUI_KeyFlags	flags;
	int				cmd;
};

class	GUI_Application : public GUI_Commander {
public:
					 GUI_Application();
	virtual			~GUI_Application();

	// APPLICATION API	
	void			Run(void);	
	void			Quit(void);

	// MENU API
	GUI_Menu		GetMenuBar(void);
	GUI_Menu		CreateMenu(const char * inTitle, const GUI_MenuItem_t	items[], GUI_Menu parent, int parent_item);
	
	virtual	void	OpenFiles(const vector<string>& inFiles)=0;

	// From GUI_Commander - app never refuses focus!
	virtual	int				AcceptTakeFocus(void) 	{ return 1; }


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

#endif


