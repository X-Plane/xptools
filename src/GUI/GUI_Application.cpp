#include "GUI_Application.h"
#include "AssertUtils.h"
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
		ioString.erase(loc);
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
	GUI_Menu	amenu;

	UInt32	clss = ::GetEventClass(inEvent);	
	UInt32	kind = ::GetEventKind(inEvent);
	switch(clss) {
	case kEventClassCommand:
		switch(kind) {
		case kEventCommandProcess:
			{
				status = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(cmd), NULL, &cmd);
				if (status != noErr) return status;

				if (me->DispatchHandleCommand(cmd.commandID))
					return noErr;
				else
					return eventNotHandledErr;
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
					
				if (me->mMenus.count(amenu) == 0) return eventNotHandledErr;
			
				int item_count = ::CountMenuItems(amenu);
				
				for (int n = 1; n <= item_count; ++n)
				{
					MenuCommand	id;
					GetMenuItemCommandID(amenu, n, &id);
					
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
	SetMenuBar(GetNewMBar(128));

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
		return FALSE;

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

	while (!mDone && GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, gAccel, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
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
	#else
		#error not impl
	#endif
}

GUI_Menu		GUI_Application::GetPopupContainer(void)
{
	#if APL
		return (GUI_Menu) -1;
	#else
		#error not iml
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
	GUI_Menu	new_menu;
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
		::SetMenuItemHierarchicalID(parent, parentItem + 1, ::GetMenuID(new_menu));
	}

	RebuildMenu(new_menu, items);
#endif

#if IBM
	#error check this!
	GUI_Menu parent = (inParentMenu) ? 
		(((MenuInfo_t *) inParentMenu)->menu) :
		(gWidgetWin->GetMenuBar());

	pMenu->menu = CreateMenu();

	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.hSubMenu = pMenu->menu;
	mif.fType = MFT_STRING;
	mif.dwTypeData = const_cast<char *>(inName);
	mif.fMask = (inParentMenu) ? MIIM_SUBMENU : (MIIM_TYPE | MIIM_SUBMENU);

	if (inParentMenu == NULL)
	{
		InsertMenuItem(parent, -1, true, &mif);
	} else {
		SetMenuItemInfo(parent, inParentItem, true, &mif);
		
	}		
	
#endif

	mMenus.insert(new_menu);
	return new_menu;	
}                                    	

void	GUI_Application::RebuildMenu(GUI_Menu new_menu, const GUI_MenuItem_t	items[])
{
	#if APL
		if (CountMenuItems(new_menu) > 0)
			DeleteMenuItems(new_menu,1,CountMenuItems(new_menu));

		int n = 0;
		while (items[n].name)
		{
			string	itemname(items[n].name);
			NukeAmpersand(itemname);
			CFStringRef cfstr = CFStringCreateWithCString(kCFAllocatorDefault, itemname.c_str(), kCFStringEncodingMacRoman);
			::AppendMenuItemTextWithCFString(new_menu, cfstr, (itemname=="-" ? kMenuItemAttrSeparator : 0), items[n].cmd, NULL );
			CFRelease(cfstr);
			
			switch(items[n].key) {
			case GUI_KEY_UP:		SetMenuItemKeyGlyph(new_menu,n+1, kMenuUpArrowGlyph);		break;
			case GUI_KEY_DOWN:		SetMenuItemKeyGlyph(new_menu,n+1, kMenuDownArrowGlyph);		break;
			case GUI_KEY_RIGHT:		SetMenuItemKeyGlyph(new_menu,n+1, kMenuRightArrowGlyph);	break;
			case GUI_KEY_LEFT:		SetMenuItemKeyGlyph(new_menu,n+1, kMenuLeftArrowGlyph);		break;
			case GUI_KEY_DELETE:	SetMenuItemKeyGlyph(new_menu,n+1, kMenuDeleteLeftGlyph);	break;
			case GUI_KEY_RETURN:	SetMenuItemKeyGlyph(new_menu,n+1, kMenuReturnGlyph);		break;
			default:				::SetItemCmd(new_menu, n+1, items[n].key);					break;
			}

			::SetMenuItemModifiers(new_menu, n+1,
					((items[n].flags & gui_ShiftFlag) ? kMenuShiftModifier : 0) +
					((items[n].flags & gui_OptionAltFlag) ? kMenuOptionModifier : 0) +
					((items[n].flags & gui_ControlFlag) ? 0 : kMenuNoCommandModifier));		
					
			++n;
		}	
	#else
		#error not impl
	#endif
}

