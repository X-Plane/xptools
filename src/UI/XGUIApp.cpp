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
#include "XGUIApp.h"
#include "XWin.h"
#include "XUtils.h"

class	XGrinderWin;

#if IBM
HINSTANCE	gInstance = NULL;
#endif

#if APL
static pascal OSErr HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
#endif

void	XGrinder_Quit(void)
{
	exit(0);
}

#if APL
int		main(int argc, char ** argv)
{
	SetMenuBar(GetNewMBar(128));

	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
		NewAEEventHandlerUPP(HandleOpenDoc), 0, FALSE);

	XGrindInit();
	
	RunApplicationEventLoop();		
	return 0;
}
#endif	

#if IBM
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

	XGrindInit();

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
//		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
//		}
	}

	OleUninitialize();
	return msg.wParam;
}
#endif


#pragma mark -

#if APL
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
	XGrindFiles(files);

puke:
	AEDisposeDesc(&inDocList);
	return noErr;
}
#endif
#if SOTHIS_H4X
int main(int argc, char* argv[])
{
    Display* display = NULL;
    XVisualInfo* xvisual = NULL;
    GLXFBConfig* fbConfig = NULL;
    GLXFBConfig currFbConfig;
    XEvent xevent;
    Atom wmp;
    Atom wdw;
    int haveVisual = 0;
    int nfbConfig = 0;
    int fbAttr[] = {GLX_DRAWABLE_TYPE,
                    GLX_WINDOW_BIT,
                    GLX_RENDER_TYPE,
                    GLX_RGBA_BIT,
                    GLX_DOUBLEBUFFER,
                    True,
                    GLX_STENCIL_SIZE, 8,
                    GLX_DEPTH_SIZE, 16,
                    None
                   };

    display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "failed to open the default display (:0).\n");
        return 1;
    }
    fbConfig = glXChooseFBConfig(display, DefaultScreen(display), fbAttr, &nfbConfig);
    if (fbConfig == NULL)
    {
        XCloseDisplay(display);
        fprintf(stderr, "display doesn't support current configuration. glxext not loaded?\n");
        return 1;
    }
    for (int i = 0; i < nfbConfig; i++)
    {
        xvisual = glXGetVisualFromFBConfig(display, fbConfig[i]);
        if (!xvisual) continue;
        currFbConfig = fbConfig[i];
        haveVisual = 1;
        break;
    }
    if (!haveVisual)
    {
        XCloseDisplay(display);
        fprintf(stderr, "wasn't able to find an applicable visual\n");
        return 1;
    }
    wmp = XInternAtom(display, "WM_PROTOCOLS", False);
    wdw = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XWin::RegisterClass(display, xvisual);
    XGrindInit();
    while (haveVisual)
    {
        XNextEvent(display, &xevent);
        XWin::WinEventHandler((XAnyEvent*)&xevent, &haveVisual);
        if (!haveVisual) break;
    }
    XCloseDisplay(display);
    return 0;
}
#endif // SOTHIS_H4X
