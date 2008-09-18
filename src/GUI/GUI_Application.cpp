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

#include "GUI_Application.h"
#include "AssertUtils.h"
#include "GUI_Menus.h"
#include "XWin.h"
#include "GUI_Window.h"
#define __DEBUGGING__

GUI_Application *	gApplication = NULL;


#if IBM
HACCEL			gAccel = NULL;
vector<ACCEL>	gAccelTable;
#endif

#if APL
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#include "XUtils.h"


static void	NukeAmpersand(string& ioString)
{
	string::size_type loc;
	while ((loc = ioString.find('&')) != ioString.npos)
	{
		ioString.erase(loc,1);
	}
}

pascal OSErr GUI_Application::HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
	GUI_Application * me = reinterpret_cast<GUI_Application *>(handlerRefcon);
	
	string	fpath;
	vector<string>	files;


	AEDescList	inDocList = { 0 };
	OSErr err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &inDocList);
	if (err) return err;
	
	SInt32		numDocs;
	err = ::AECountItems(&inDocList, &numDocs);
	if (err) goto puke;

		// Loop through all items in the list
			// Extract descriptor for the document
			// Coerce descriptor data into a FSSpec
			// Tell Program object to open or print document


	for (SInt32 i = 1; i <= numDocs; i++) {
		AEKeyword	theKey;
		DescType	theType;
		FSSpec		theFileSpec;
		Size		theSize;
		err = ::AEGetNthPtr(&inDocList, i, typeFSS, &theKey, &theType,
							(Ptr) &theFileSpec, sizeof(FSSpec), &theSize);
		if (err) goto puke;
		FSSpec_2_String(theFileSpec, fpath);
		files.push_back(fpath);
	}
	me->OpenFiles(files);

puke:
	AEDisposeDesc(&inDocList);
	return noErr;
}


pascal OSStatus GUI_Application::MacEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	GUI_Application * me = reinterpret_cast<GUI_Application *>(inUserData);

	HICommand 	cmd;
	OSStatus	status;
	MenuRef		amenu;

	UInt32	clss = ::GetEventClass(inEvent);	
	UInt32	kind = ::GetEventKind(inEvent);
	switch(clss) {
	case kEventClassCommand:
		switch(kind) {
		case kEventCommandProcess:
			{
				status = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(cmd), NULL, &cmd);
				if (status != noErr) return status;

				switch(cmd.commandID) {
				case kHICommandQuit:
					if (me->DispatchHandleCommand(gui_Quit))			return noErr;
					else												return eventNotHandledErr;					
				case kHICommandAbout:
					if (me->DispatchHandleCommand(gui_About))			return noErr;
					else												return eventNotHandledErr;
				case kHICommandPreferences:
					if (me->DispatchHandleCommand(gui_Prefs))			return noErr;
					else												return eventNotHandledErr;
				default:
					if (me->DispatchHandleCommand(cmd.commandID))		return noErr;
					else												return eventNotHandledErr;
				}
			}
		default:
			return eventNotHandledErr;
		}
	case kEventClassMenu:
		switch(kind) {
		case kEventMenuEnableItems:
			{
				status = GetEventParameter(inEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof(amenu), NULL, &amenu);
				if (status != noErr) return status;
					
				if (me->mMenus.count(amenu) == 0) 
					return eventNotHandledErr;
			
				int item_count = ::CountMenuItems(amenu);
				
				for (int n = 1; n <= item_count; ++n)
				{
					MenuCommand	id;
					GetMenuItemCommandID(amenu, n, &id);

					if (id == kHICommandQuit)			id = gui_Quit;
					if (id == kHICommandAbout)			id = gui_About;
					if (id == kHICommandPreferences)	id = gui_Prefs;
					
					if (id != 0)
					{
						string	ioName;
						int		ioCheck = 0;
						if (me->DispatchCanHandleCommand(id, ioName, ioCheck))
							EnableMenuItem(amenu, n);
						else
							DisableMenuItem(amenu, n);

						if (!ioName.empty())
						{
							NukeAmpersand(ioName);
							CFStringRef	cfstr = CFStringCreateWithCString(kCFAllocatorDefault, ioName.c_str(), kCFStringEncodingMacRoman);
							SetMenuItemTextWithCFString(amenu, n, cfstr);
							CFRelease(cfstr);
						}
						
						::CheckMenuItem(amenu, n, ioCheck > 0);
					}
				}
								
				return noErr;
			}
		default:
			return eventNotHandledErr;
		}
	default:
		return eventNotHandledErr;
	}
}


#endif

#if IBM
void	RegisterAccel(const ACCEL& inAccel)
{
	gAccelTable.push_back(inAccel);
}

static	void		BuildAccels(void)
{
	gAccel = CreateAcceleratorTable(&*gAccelTable.begin(), gAccelTable.size());
}
#endif


GUI_Application::GUI_Application() : GUI_Commander(NULL)
{
	DebugAssert(gApplication == NULL);
	gApplication = this;
	mDone = false;
#if APL

		IBNibRef	nib = NULL;
	OSStatus err = CreateNibReference(CFSTR("GUI"), &nib);
	if (err == 0)
		err = SetMenuBarFromNib(nib, CFSTR("MenuBar"));
	EnableMenuCommand(NULL, kHICommandAbout);
	EnableMenuCommand(NULL, kHICommandPreferences);


	mMacEventHandlerUPP = NewEventHandlerUPP(MacEventHandler);
	mHandleOpenDocUPP = NewAEEventHandlerUPP(HandleOpenDoc);

	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, mHandleOpenDocUPP, reinterpret_cast<long>(this), FALSE);
		
	EventTypeSpec menu_events[] = {
		kEventClassCommand,			kEventCommandProcess,
		kEventClassMenu,			kEventMenuEnableItems };
		
	InstallEventHandler(GetApplicationEventTarget(), mMacEventHandlerUPP, GetEventTypeCount(menu_events), menu_events, reinterpret_cast<void *>(this), &mMacEventHandlerRef);
	
#endif
#if IBM
	// Note: GetModuleHandle(NULL) returns the process instance/module handle which
	// is what we want UNLESS this code is put in a DLL, which would need some re-evaluation.

	XWin::RegisterClass(GetModuleHandle(NULL));
	if(OleInitialize(NULL) != S_OK)
	{
#if ERROR_CHECK
		uh oh
#endif
	}
	InitCommonControls(); 
#endif

#if LIN
    Visual*  a_defVisual = 0;
    int a_defDepth = 0;
    int a_screenNumber = 0;

    display = XOpenDisplay(NULL);
    if (!display)
        throw "failed to open the default display (:0).";

    a_screenNumber = DefaultScreen(display);
    a_defVisual = DefaultVisual(display, a_screenNumber);
    if (!a_defVisual)
        throw "invalid visual.";
    a_defDepth  = DefaultDepth(display, a_screenNumber);
    XWin::RegisterClass(display, a_screenNumber, a_defDepth, a_defVisual);
#endif

}

GUI_Application::~GUI_Application()
{
	DebugAssert(gApplication == this);
	gApplication = NULL;

}

void			GUI_Application::Run(void)
{
#if APL
	RunApplicationEventLoop();		
#endif
#if IBM

	MSG msg;

	BuildAccels();
	while (!mDone && GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, gAccel, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
#endif
#if LIN
    XEvent xevent;

    while (!mDone)
    {
        XNextEvent(display, &xevent);
        XWin::WinEventHandler((XAnyEvent*)&xevent, (int*)&mDone);
        if (mDone) break;
    }
    XCloseDisplay(display);
#endif
}

void			GUI_Application::Quit(void)
{
	mDone = true;
#if APL
	QuitApplicationEventLoop();
#endif
}

GUI_Menu			GUI_Application::GetMenuBar(void)
{
	#if APL
		return NULL;
	#elif IBM
		HWND hwnd = GUI_Window::AnyHWND();
		if (hwnd == NULL) return NULL;
		HMENU mbar = ::GetMenu(hwnd);
		if (mbar != NULL) return mbar;
		mbar = ::CreateMenu();
		::SetMenu(hwnd, mbar);
		return mbar;
	#else
		#warning add linux menubar initialization here
	#endif
}

GUI_Menu		GUI_Application::GetPopupContainer(void)
{
	#if APL
		return (GUI_Menu) -1;
	#elif IBM
		return NULL;
	#else
		return 0;
	#endif
}

GUI_Menu	GUI_Application::CreateMenu(const char * inTitle, const GUI_MenuItem_t items[], GUI_Menu	parent, int parentItem)
{

#if APL
	static MenuID	gIDs = 1000;
#endif
#if IBM
	static int		gIDs = 1000;
#endif

#if APL
	MenuRef	new_menu;
	::CreateNewMenu(gIDs++, kMenuAttrAutoDisable, &new_menu);
	if (parent != GetPopupContainer())
		::MacInsertMenu(new_menu, (parent == NULL) ? 0 : kInsertHierarchicalMenu);
	
	string	title(inTitle);
	NukeAmpersand(title);
	CFStringRef	cfstr = CFStringCreateWithCString(kCFAllocatorDefault, title.c_str(), kCFStringEncodingMacRoman);
	::SetMenuTitleWithCFString(new_menu, cfstr);
	CFRelease(cfstr);
	
	if (new_menu && parent != GetPopupContainer())
	{
		::SetMenuItemHierarchicalID((MenuRef) parent, parentItem + 1, ::GetMenuID(new_menu));
	}

	RebuildMenu(new_menu, items);
#endif

#if IBM

	HMENU	new_menu;

	if (parent == GetPopupContainer())	new_menu = CreatePopupMenu();
	else								new_menu = ::CreateMenu();
	
	if (parent)
	{
		MENUITEMINFO	mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.hSubMenu = (HMENU) new_menu;
		mif.fType = MFT_STRING;
		mif.dwTypeData = const_cast<char *>(inTitle);
		mif.fMask = (parent == GetPopupContainer()) ? MIIM_TYPE : (MIIM_TYPE | MIIM_SUBMENU);

		if (parent == GetMenuBar())
		{
			InsertMenuItem((HMENU) parent, -1, true, &mif);
		} else {
			SetMenuItemInfo((HMENU) parent, parentItem, true, &mif);	
		}		
	}
#endif
#if LIN
    #warning add menu creation code for linux here
    void* new_menu = 0;
#endif

	RebuildMenu(new_menu, items);

	mMenus.insert(new_menu);

#if IBM
	if (parent)
		DrawMenuBar(GUI_Window::AnyHWND());
#endif
	return new_menu;	
}                                    	

void	GUI_Application::RebuildMenu(GUI_Menu new_menu, const GUI_MenuItem_t	items[])
{
	#if APL
		if (CountMenuItems((MenuRef) new_menu) > 0)
			DeleteMenuItems((MenuRef) new_menu,1,CountMenuItems((MenuRef) new_menu));

		int n = 0;
		while (items[n].name)
		{
			string	itemname(items[n].name);
			NukeAmpersand(itemname);
			CFStringRef cfstr = CFStringCreateWithCString(kCFAllocatorDefault, itemname.c_str(), kCFStringEncodingMacRoman);
			::AppendMenuItemTextWithCFString((MenuRef) new_menu, cfstr, (itemname=="-" ? kMenuItemAttrSeparator : 0), items[n].cmd, NULL );
			CFRelease(cfstr);
			
			switch(items[n].key) {
			case GUI_KEY_UP:		SetMenuItemKeyGlyph((MenuRef) new_menu,n+1, kMenuUpArrowGlyph);		break;
			case GUI_KEY_DOWN:		SetMenuItemKeyGlyph((MenuRef) new_menu,n+1, kMenuDownArrowGlyph);	break;
			case GUI_KEY_RIGHT:		SetMenuItemKeyGlyph((MenuRef) new_menu,n+1, kMenuRightArrowGlyph);	break;
			case GUI_KEY_LEFT:		SetMenuItemKeyGlyph((MenuRef) new_menu,n+1, kMenuLeftArrowGlyph);	break;
			case GUI_KEY_DELETE:	SetMenuItemKeyGlyph((MenuRef) new_menu,n+1, kMenuDeleteLeftGlyph);	break;
			case GUI_KEY_RETURN:	SetMenuItemKeyGlyph((MenuRef) new_menu,n+1, kMenuReturnGlyph);		break;
			default:				::SetItemCmd((MenuRef) new_menu, n+1, items[n].key);				break;
			}

			::SetMenuItemModifiers((MenuRef) new_menu, n+1,
					((items[n].flags & gui_ShiftFlag) ? kMenuShiftModifier : 0) +
					((items[n].flags & gui_OptionAltFlag) ? kMenuOptionModifier : 0) +
					((items[n].flags & gui_ControlFlag) ? 0 : kMenuNoCommandModifier));		

			::CheckMenuItem((MenuRef) new_menu,n+1,items[n].checked);
					
			++n;
		}		
	#elif IBM
		while (GetMenuItemCount((HMENU) new_menu) > 0)
			if (RemoveMenu((HMENU) new_menu, 0, MF_BYPOSITION) == 0) break;


		int n = 0;
		while (items[n].name)
		{
			string	itemname(items[n].name);
			if(items[n].key != 0)
			{
				ACCEL accel = { 0 };
				accel.fVirt = FVIRTKEY;
				accel.cmd = items[n].cmd;				
				itemname += "\t";
				if (items[n].flags & gui_ControlFlag)	{itemname += "Ctrl+";	accel.fVirt |= FCONTROL;	}
				if (items[n].flags & gui_ShiftFlag)		{itemname += "Shift+";	accel.fVirt |= FSHIFT;		}
				if (items[n].flags & gui_OptionAltFlag) {itemname += "Alt+";	accel.fVirt |= FALT;		}
				char key_cstr[2] = { items[n].key, 0 };
				switch(items[n].key)
				{
					case GUI_KEY_UP:		itemname += "Up";		accel.key = VK_UP;		break;
					case GUI_KEY_DOWN:		itemname += "Down";		accel.key = VK_DOWN;	break;
					case GUI_KEY_RIGHT:		itemname += "Right";	accel.key = VK_RIGHT;	break;
					case GUI_KEY_LEFT:		itemname += "Left";		accel.key = VK_LEFT;	break;
					case GUI_KEY_DELETE:	itemname += "Del";		accel.key = VK_BACK;	break;
					case GUI_KEY_RETURN:	itemname += "Return";	accel.key = VK_RETURN;	break;
					default:				itemname += key_cstr;	accel.key = VkKeyScan(items[n].key) & 0xFF;	break;
				}
				RegisterAccel(accel);
			}

			MENUITEMINFO mif = { 0 };
			mif.cbSize = sizeof(mif);
			mif.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
			mif.fType = (itemname=="-") ? MFT_SEPARATOR : MFT_STRING;
			mif.fState = (items[n].checked) ? (MFS_CHECKED | MFS_ENABLED) : MFS_ENABLED;
			mif.wID = items[n].cmd;
			mif.dwItemData = items[n].cmd;
			mif.dwTypeData = const_cast<char*>(itemname.c_str());
			int err = InsertMenuItem((HMENU) new_menu, -1, true, &mif);

			++n;
		}		

	#else
		#warning add menurebuild code for linux here
	#endif
}

int			GUI_Application::HandleCommand(int command)
{
	switch(command) {
	case gui_About: this->AboutBox(); return 1;
	case gui_Prefs: this->Preferences(); return 1;
	case gui_Quit: if (this->CanQuit()) this->Quit(); return 1;
	default: return 0;
	}
}

int			GUI_Application::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case gui_About: return 1;
	case gui_Prefs: return 1;
	case gui_Quit: return 1;
	default: return 0;
	}
}
