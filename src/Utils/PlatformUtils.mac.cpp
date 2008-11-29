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
#include "PlatformUtils.h"


#if __MWERKS__
	#if defined(__MACH__)
		#define _STDINT_H_
	#endif
	#include <Carbon.h>
#else
	#define _STDINT_H_
	#include <Carbon/Carbon.h>
#endif
#include <string.h>

static	OSErr		FSSpecToPathName(const FSSpec * inFileSpec, char * outPathname, int in_buf_size);
static	OSErr		FSRefToPathName(const FSRef * inFileSpec, char * outPathname, int in_buf_size);


/* Get FilePathFromUser puts up a nav services dialog box and converts the results
   to a C string path. */

pascal void event_proc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUD)
{
}

const char * GetApplicationPath(char * pathBuf, int pathLen)
{
/*	ProcessInfoRec		pir;
	FSSpec				spec;
	Str255				pStr;
	ProcessSerialNumber	psn = { 0, kCurrentProcess };
	pir.processInfoLength 	= sizeof(pir);
	pir.processAppSpec 		= &spec;
	pir.processName			= pStr;
	GetProcessInformation(&psn, &pir);
	OSErr err = FSSpecToPathName(&spec, pathBuf, sizeof(pathBuf));
	if (err != noErr)
		return NULL;
*/
	CFURLRef	main_url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	CFStringRef	main_path = CFURLCopyFileSystemPath(main_url, kCFURLPOSIXPathStyle);
	CFStringGetCString(main_path,pathBuf,pathLen,kCFStringEncodingMacRoman);
	CFRelease(main_url);
	CFRelease(main_path);
	return pathBuf;
}

int		GetFilePathFromUser(
					int					inType,
					const char * 		inPrompt,
					const char *		inAction,
					int					inID,
					char * 				outFileName,
					int					inBufSize)
{
		OSErr						err;
		NavDialogCreationOptions	options;
		FSRef						fileSpec;
		NavDialogRef				dialog = NULL;
		NavUserAction				action;
		NavEventUPP					eventUPP = NULL;
		
	err = NavGetDefaultDialogCreationOptions(&options);	
	if (err != noErr) goto bail;

	if (inType == getFile_Save)
		options.saveFileName = CFStringCreateWithCString(kCFAllocatorDefault,outFileName,kCFStringEncodingMacRoman);

	options.message = CFStringCreateWithCString(kCFAllocatorDefault,inPrompt,kCFStringEncodingMacRoman);
	options.actionButtonLabel = CFStringCreateWithCString(kCFAllocatorDefault,inAction,kCFStringEncodingMacRoman);
	options.optionFlags &= ~kNavAllowMultipleFiles;
	options.optionFlags &= ~kNavAllowStationery	;
	options.optionFlags |=  kNavAllFilesInPopup	;
	options.preferenceKey = inID;

	eventUPP = NewNavEventUPP(event_proc);
	
	switch(inType) {
	case getFile_Open:
		err = NavCreateGetFileDialog(&options, NULL, eventUPP, NULL, NULL, NULL, &dialog);
		if (err != noErr) goto bail;
		break;
	case getFile_Save:
		err = NavCreatePutFileDialog(&options, 0, 0, eventUPP, NULL, &dialog);
		if (err != noErr) goto bail;
		break;
	case getFile_PickFolder:
		err = NavCreateChooseFolderDialog(&options, eventUPP, NULL, NULL, &dialog);
		if (err != noErr) goto bail;
	}

	err = NavDialogRun(dialog);
	if (err != noErr) goto bail;

	CFRelease(options.message);
	CFRelease(options.actionButtonLabel);
	if(options.saveFileName) CFRelease(options.saveFileName);
	
	action = NavDialogGetUserAction(dialog);
	if (action !=kNavUserActionCancel && action != kNavUserActionNone)
	{
		NavReplyRecord	reply;
		err = NavDialogGetReply(dialog, &reply);
		if (err != noErr) goto bail;

		err = AEGetNthPtr(&reply.selection, 1, typeFSRef, NULL, NULL, &fileSpec, sizeof(fileSpec), NULL);
		if (err != noErr)
			goto bail;

		err = FSRefToPathName(&fileSpec, outFileName, inBufSize);
		if (err != noErr)
			goto bail;

		NavDisposeReply(&reply);

		if (inType == getFile_Save)
		{
			CFStringRef str = NavDialogGetSaveFileName(dialog);

			strcat(outFileName,DIR_STR);
			int p = strlen(outFileName);
			int len = CFStringGetLength(str);
			int got = CFStringGetBytes(str, CFRangeMake(0, len), kCFStringEncodingMacRoman, 0, 0, (UInt8*)outFileName+p, len, NULL);
			outFileName[p+len] = 0;
		}

	}


	NavDialogDispose(dialog);
	dialog = NULL;

	DisposeNavEventUPP(eventUPP);
	return (action !=kNavUserActionCancel && action != kNavUserActionNone);

bail:
	if(eventUPP)	DisposeNavEventUPP(eventUPP);			
	if(dialog)		NavDialogDispose(dialog);
	return 0;


}

void	DoUserAlert(const char * inMsg)
{
	Str255	p1;
	size_t	sl;

	sl = strlen(inMsg);
	if (sl > 255)
		sl = 255;

	p1[0] = sl;
	memcpy(p1+1, inMsg, sl);

	StandardAlert(kAlertStopAlert, p1, "\p", NULL, NULL);
}

void	ShowProgressMessage(const char * inMsg, float * progress)
{
	static WindowRef	wind = NULL;
	Rect		windBounds = { 0, 0, 250, 500 };
	if (wind == NULL)
	{
		if (CreateNewWindow(kMovableAlertWindowClass, kWindowStandardHandlerAttribute, &windBounds, &wind) != noErr) return;
		if (wind == NULL) return;
		RepositionWindow(wind, NULL,kWindowCenterOnMainScreen);
		ShowWindow(wind);
	}

	SetPortWindowPort(wind);
	CFStringRef ref = CFStringCreateWithCString(NULL, inMsg, kCFStringEncodingMacRoman);
	EraseRect(&windBounds);
	InsetRect(&windBounds, 20, 15);
	DrawThemeTextBox(ref, kThemeSystemFont, kThemeStateActive, true, &windBounds, teJustLeft, NULL);
	CFRelease(ref);

	if (progress)
	{
		float p = *progress;
		ThemeTrackDrawInfo	info;
		info.kind = (p >= 0.0) ? kThemeMediumProgressBar : kThemeMediumIndeterminateBar;
		SetRect(&info.bounds, 20, 210, 480, 230);
		info.min = 0;
		info.max = (p >= 0.0) ? 1000.0 : 0.0;
		info.value = (p >= 0.0) ? (p * 1000.0) : 0;
		info.reserved = 0;
		info.attributes = kThemeTrackHorizontal;
		info.enableState = kThemeTrackActive;
		info.filler1 = 0;
		static UInt8 nPhase = 0;
		info.trackInfo.progress.phase = nPhase;
		nPhase++;
		DrawThemeTrack(&info, NULL, NULL, 0);
	}
	QDFlushPortBuffer(GetWindowPort(wind), NULL);
}

int		ConfirmMessage(const char * inMsg, const char * proceedBtn, const char * cancelBtn)
{
	Str255					pStr, proStr, clcStr;
	AlertStdAlertParamRec	params;
	short					itemHit;

	pStr[0] = strlen(inMsg);
	memcpy(pStr+1,inMsg,pStr[0]);
	proStr[0] = strlen(proceedBtn);
	memcpy(proStr+1, proceedBtn, proStr[0]);
	clcStr[0] = strlen(cancelBtn);
	memcpy(clcStr+1, cancelBtn, clcStr[0]);

	params.movable = false;
	params.helpButton = false;
	params.filterProc = NULL;
	params.defaultText = proStr;
	params.cancelText = clcStr;
	params.otherText = NULL;
	params.defaultButton = 1;
	params.cancelButton = 2;
	params.position = kWindowDefaultPosition;

	StandardAlert(kAlertCautionAlert, pStr, "\p", &params, &itemHit);

	return (itemHit == 1);
}

/*
 * FSSpecToPathName
 *
 * This routine builds a full path from a file spec by recursing up the directory
 * tree to the route, prepending each directory name.
 *
 */

OSErr	FSSpecToPathName(const FSSpec * inFileSpec, char * outPathname, int buf_size)
{
	FSRef ref;
	OSErr err = FSpMakeFSRef(inFileSpec, &ref);
	if (err != noErr) return err;
	return FSRefToPathName(&ref, outPathname, buf_size);
}

OSErr	FSRefToPathName(const FSRef * inFileRef, char * outPathname, int in_buf_size)
{
	CFURLRef url = CFURLCreateFromFSRef(kCFAllocatorDefault, inFileRef);
	if (url == NULL)	return -1;

	CFURLPathStyle st = kCFURLPOSIXPathStyle;
	#if defined(__MWERKS__)
	st = kCFURLHFSPathStyle;
	#endif
	CFStringRef	str = CFURLCopyFileSystemPath(url, st);
	CFRelease(url);
	if (str == NULL)	return -1;

	CFIndex		len = CFStringGetLength(str);
	CFIndex		got = CFStringGetBytes(str, CFRangeMake(0, len), kCFStringEncodingMacRoman, 0, 0, (UInt8*)outPathname, in_buf_size-1, NULL);
	outPathname[got] = 0;
	CFRelease(str);
	return noErr;
}


int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	OSErr	err;
	Str255	pstr1, pstr2;
	SInt16	item;
	AlertStdAlertParamRec	rec;

	pstr1[0] = strlen(inMessage1);
	pstr2[0] = strlen(inMessage2);
	memcpy(pstr1+1,inMessage1,pstr1[0]);
	memcpy(pstr2+1,inMessage2,pstr2[0]);

	rec.movable = false;
	rec.helpButton = false;
	rec.filterProc = NULL;
	rec.defaultText = (ConstStringPtr) kAlertDefaultOKText;
	rec.cancelText = (ConstStringPtr) kAlertDefaultCancelText;
	rec.otherText = (ConstStringPtr)  kAlertDefaultOtherText;
	rec.defaultButton = kAlertStdAlertOKButton;
	rec.cancelButton = kAlertStdAlertCancelButton;
	rec.position = kWindowDefaultPosition;

	err = StandardAlert(
					kAlertCautionAlert,
					pstr1,
					pstr2,
					&rec,
					&item);

	if (err != noErr) return close_Cancel;
	switch(item) {
	case kAlertStdAlertOKButton: 		return close_Save;
	case kAlertStdAlertCancelButton: 	return close_Cancel;
	case kAlertStdAlertOtherButton: 	return close_Discard;
	default: return close_Cancel;
	}
}
