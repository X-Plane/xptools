/*
 * Copyright (c) 2004, Laminar Research.
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
#include "XWidgetApp.h"
#include "XUtils.h"
#include "XPWidgetWin.h"

class	XGrinderWin;

#if IBM
HINSTANCE	gInstance = NULL;
HACCEL		gAccel = NULL;
vector<ACCEL>	gAccelTable;
#endif


#if APL
static pascal OSErr HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
//#include <sioux.h>
#endif

void	XGrinder_Quit(void)
{
	exit(0);
}

#if APL
/*
pascal OSStatus SiouxSniffer(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	EventRecord	rec;
	if (ConvertEventRefToEventRecord(inEvent, &rec) && SIOUXHandleOneEvent(&rec))
		return noErr;
	return eventNotHandledErr;
}
*/

int		main(int argc, char ** argv)
{
	SetMenuBar(GetNewMBar(128));

	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
		NewAEEventHandlerUPP(HandleOpenDoc), 0, FALSE);

	gWidgetWin = new XPWidgetWin();

	gWidgetWin->SetGLContext();
	string name;
	XGrindInit(name);

	gWidgetWin->SetTitle(name.c_str());

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



//	InstallEventHandler(GetEventDispatcherTarget(),
//		NewEventHandlerUPP(SiouxSniffer),GetEventTypeCount(events), events, NULL, NULL);

	RunApplicationEventLoop();

	XGrindDone();

	return 0;
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


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	gInstance = hInstance;

	MSG msg;
//	HACCEL hAccelTable;

	XWin::RegisterClass(hInstance);
	if(OleInitialize(NULL) != S_OK)
		return FALSE;

	gWidgetWin = new XPWidgetWin();

	gWidgetWin->SetGLContext();
	string name;
	XGrindInit(name);

	BuildAccels();

	gWidgetWin->SetTitle(name.c_str());

	gWidgetWin->DrawMenuBar();

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, gAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	XGrindDone();

	OleUninitialize();
	return msg.wParam;
}
#endif


void XGrinder_SetTitle(const char * t)
{
	if (gWidgetWin)
		gWidgetWin->SetTitle(t);
}


#pragma mark -

#if APL
pascal OSErr HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
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
		FSRef		theFileSpec;
		Size		theSize;
		err = ::AEGetNthPtr(&inDocList, i, typeFSRef, &theKey, &theType,
							(Ptr) &theFileSpec, sizeof(FSRef), &theSize);
		if (err) goto puke;
		UInt8 buf[2048];
		if(FSRefMakePath(&theFileSpec, buf, sizeof(buf)) == noErr)
		files.push_back((const char *) buf);
	}
	XGrindFiles(files, -1, -1);

puke:
	AEDisposeDesc(&inDocList);
	return noErr;
}
#endif