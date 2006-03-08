#include "GUI_Application.h"

#if IBM
HACCEL			gAccel = NULL;
vector<ACCEL>	gAccelTable;
#endif


#if APL
#include <Carbon.h>
#include "XUtils.h"
static pascal OSErr HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
#include <sioux.h>

pascal OSStatus SiouxSniffer(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	EventRecord	rec;
	if (ConvertEventRefToEventRecord(inEvent, &rec) && SIOUXHandleOneEvent(&rec))
		return noErr;
	return eventNotHandledErr;
}

pascal OSErr HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
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
	((GUI_Application *)handlerRefcon)->OpenFiles(files);

puke:
	AEDisposeDesc(&inDocList);
	return noErr;
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


GUI_Application::GUI_Application()
{
	mDone = false;
#if APL
	SetMenuBar(GetNewMBar(128));

	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
		NewAEEventHandlerUPP(HandleOpenDoc), (long) this, FALSE);

	EventTypeSpec events[] = {
		kEventClassMouse,			kEventMouseDown,
		kEventClassMouse,			kEventMouseUp,
		kEventClassMouse,			kEventMouseMoved,
		kEventClassMouse,			kEventMouseDragged,
		kEventClassKeyboard,		kEventRawKeyDown,
		kEventClassKeyboard,		kEventRawKeyRepeat,
		kEventClassKeyboard,		kEventRawKeyUp,
		kEventClassKeyboard,		kEventRawKeyModifiersChanged,
		kEventClassApplication,		kEventAppActivated,
		kEventClassApplication,		kEventAppDeactivated,
		kEventClassApplication,		kEventAppFrontSwitched,
		kEventClassWindow,			kEventWindowUpdate,
		kEventClassWindow,			kEventWindowActivated,
		kEventClassWindow,			kEventWindowDeactivated,
		kEventClassWindow,			kEventWindowShowing,
		kEventClassWindow,			kEventWindowHiding,
		kEventClassWindow,			kEventWindowShown,
		kEventClassWindow,			kEventWindowHidden,
		kEventClassWindow,			kEventWindowBoundsChanged,
		kEventClassWindow,			kEventWindowResizeCompleted,
		kEventClassWindow,			kEventWindowDragCompleted,
		kEventClassWindow,			kEventWindowClickDragRgn,
		kEventClassWindow,			kEventWindowClickResizeRgn,
		kEventClassWindow,			kEventWindowClickCollapseRgn,
		kEventClassWindow,			kEventWindowClickCloseRgn,
		kEventClassWindow,			kEventWindowClickZoomRgn,
		kEventClassWindow,			kEventWindowClickContentRgn,
		kEventClassWindow,			kEventWindowClickStructureRgn,
		kEventClassWindow,			kEventWindowCursorChange,
		kEventClassWindow,			kEventWindowClose,
		kEventClassWindow,			kEventWindowExpand,
		kEventClassWindow,			kEventWindowZoom,
		kEventClassWindow,			kEventWindowHandleContentClick };

	InstallEventHandler(GetEventDispatcherTarget(), 
		NewEventHandlerUPP(SiouxSniffer),GetEventTypeCount(events), events, NULL, NULL);	
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
