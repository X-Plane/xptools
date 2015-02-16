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
#include <Windows.h>
#include <Commdlg.h>
#include "PlatformUtils.h"
#include <shlobj.h>
#include <stdio.h>

const char * GetApplicationPath(char * pathBuf, int sz)
{
	if (GetModuleFileName(NULL, pathBuf, sz))
		return pathBuf;
	else
		return NULL;
}


int		GetFilePathFromUser(
					int					inType,
					const char * 		inPrompt,
					const char *		inAction,
					int					inID,
					char * 				outFileName,
					int					inBufSize)
{
		BROWSEINFO	bif = { 0 };
		OPENFILENAME	ofn = { 0 };


	BOOL result;
	switch(inType) {
	case getFile_Open:
	case getFile_Save:
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFilter = "All Files\000*.*\000";
		ofn.nFilterIndex = 1;	// Start with .acf files
		ofn.lpstrFile = outFileName;
		if (inType != getFile_Save)
			outFileName[0] = 0;		// No initialization for open.
		ofn.nMaxFile = inBufSize;		// Guess string length?
		ofn.lpstrFileTitle = NULL;	// Don't want file name w/out path
		ofn.lpstrTitle = inPrompt;
		result = (inType == getFile_Open) ? GetOpenFileName(&ofn) : GetSaveFileName(&ofn);
		return (result) ? 1 : 0;
	case getFile_OpenImages:
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFilter = "GeoTiff (*.tif)\0*.tif\0PNG (*.png)\0*.png\0JPEG (*.jpg, *.jpeg)\0*.jpg;*.jpeg\0BMP (*.bmp)\0*.bmp\0DDS (*.dds)\0*.dds\0\0\0";
		ofn.nFilterIndex = 1;	// Start with .acf files
		ofn.lpstrFile = outFileName;
		if (inType != getFile_Save)
			outFileName[0] = 0;		// No initialization for open.
		ofn.nMaxFile = inBufSize;		// Guess string length?
		ofn.lpstrFileTitle = NULL;	// Don't want file name w/out path
		ofn.lpstrTitle = inPrompt;
		result = (inType == getFile_OpenImages) ? GetOpenFileName(&ofn) : GetSaveFileName(&ofn);
		return (result) ? 1 : 0;
	case getFile_PickFolder:
		bif.hwndOwner = NULL;
		bif.pidlRoot = NULL;
		bif.pszDisplayName = NULL;
		bif.lpszTitle = inPrompt;
		bif.ulFlags = 0;
		bif.lpfn = NULL;
		bif.lParam = NULL;
		LPITEMIDLIST items = SHBrowseForFolder(&bif);
		if (items == NULL) return 0;
		result = 0;
        if (SHGetPathFromIDList (items, outFileName))
        {
        	result = 1;
//			strcat(outFileName, "\\");
		}
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( items );
            imalloc->Release ( );
        }
        return result ? 1 : 0;
	}
	return 0;
}


char *	GetMultiFilePathFromUser(
					const char * 		inPrompt,
					const char *		inAction,
					int					inID)
{
	OPENFILENAME	ofn = { 0 };
	BOOL result;
	char buf[4096];

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "All Files\000*.*\000";
	ofn.nFilterIndex = 1;	// Start with .acf files
	ofn.lpstrFile = buf;
	buf[0] = 0;		// No initialization for open.
	ofn.nMaxFile = sizeof(buf);		// Guess string length?
	ofn.lpstrFileTitle = NULL;	// Don't want file name w/out path
	ofn.lpstrTitle = inPrompt;
	ofn.Flags =  OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	result = GetOpenFileName(&ofn);
	if(result)
	{
		vector<string>	files;
		string path(buf);
		char * fptr = buf+path.size()+1;
		while(*fptr)
		{
			files.push_back(path + "\\" + fptr);
			fptr += (strlen(fptr) + 1);
		}

		int buf_size = 1;
		for(int i = 0; i < files.size(); ++i)
			buf_size += (files[i].size() + 1);
	
		char * ret = (char *) malloc(buf_size);
		char * p = ret;

		for(int i = 0; i < files.size(); ++i)
		{
			strcpy(p, files[i].c_str());
			p += (files[i].size() + 1);
		}
		*p = 0;
	
		return ret;
	}
	else
		return NULL;
}




void	DoUserAlert(const char * inMsg)
{
	MessageBox(NULL, inMsg, "Alert", MB_OK + MB_ICONWARNING);
}

int		ConfirmMessage(const char * inMsg, const char * proceedBtn, const char * cancelBtn)
{
	int result = MessageBox(
						NULL,				// No Parent HWND
						inMsg,
						"X-Plane 8",			// Dialog caption
//						MB_OKCANCEL +
						MB_YESNO +
//						MB_ICONWARNING +
						MB_USERICON +
						MB_DEFBUTTON1);

	return (result == IDOK || result == IDYES) ? 1 : 0;
}

int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	int result = MessageBox(
			NULL,
			inMessage2,
			inMessage1,
			MB_YESNOCANCEL +
			MB_ICONEXCLAMATION);
	switch(result) {
	case IDCANCEL:	return close_Cancel;
	case IDYES:		return close_Save;
	case IDNO:		return close_Discard;
	default:		return close_Cancel;
	}
}
