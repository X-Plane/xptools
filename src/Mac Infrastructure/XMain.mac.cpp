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

#include <string>
using namespace std;
#include "PlatformUtils.h"

pascal OSErr HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);

extern	void	XGrindFiles(const vector<string>& files);
extern	void	XGrindGlutInit(void);


OSErr	RegAppleEvent(void)
{
	return AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
		NewAEEventHandlerUPP(HandleOpenDoc), 0, FALSE);

}


int main(int argc, char ** argv)
{


#if USE_GLUT

	glutInit(&argc, argv);
	XGrindGlutInit();
#else
	SetMenuBar(GetNewMBar(128));
	ShowProgressMessage("Drag a file onto the application icon in the finder to process a file.");
#endif

	OSErr err = RegAppleEvent();

	if (err != noErr)
		return err;

#if USE_GLUT
	glutMainLoop();
#else
	RunApplicationEventLoop();
#endif
	return 0;
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
	XGrindFiles(files);

puke:
	AEDisposeDesc(&inDocList);
	return noErr;
}