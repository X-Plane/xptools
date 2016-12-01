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
#include "GUI_Unicode.h"
#include "PlatformUtils.h"
#include <shlobj.h>
#include <stdio.h>

string GetApplicationPath(char * pathBuf, int sz)
{
	WCHAR utf16_path_buf[MAX_PATH];
	if (GetModuleFileNameW(NULL, utf16_path_buf, sz))
	{
		return convert_utf16_to_str(utf16_path_buf);
	}
	else
	{
		return "";
	}
}

string GetCacheFolder(char cache_path[], int sz)
{
	assert(sz == MAX_PATH);

	if(sz != MAX_PATH)
	{
		return "";
	}

	WCHAR wc_cache_path[MAX_PATH];
	HRESULT res = SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wc_cache_path);
	if (SUCCEEDED(res))
	{
		return convert_utf16_to_str(wc_cache_path);
	}
	else
	{
		return "";
	}
}

string GetTempFilesFolder(char temp_path[], int sz)
{
	assert(sz == MAX_PATH);
	if(sz > MAX_PATH)
	{
		return "";
	}

	WCHAR wc_temp_path[MAX_PATH] = { 0 };
	int result = GetTempPathW(sz, wc_temp_path);
	if (result > wcslen(wc_temp_path) || result == 0)
	{
		return "";
	}

	return convert_utf16_to_str(wc_temp_path);
}

int		GetFilePathFromUser(
					int					inType,
					const char * 		inPrompt,
					const char *		inAction,
					int					inID,
					char *				outFileName,
					int					inBufSize)
{
	BROWSEINFOW	bif = { 0 };
	OPENFILENAMEW	ofn = { 0 };

	WCHAR file_name[MAX_PATH] = { 0 };
	ofn.lpstrFile = file_name;

	ofn.lpstrTitle = convert_str_to_utf16(inPrompt).c_str();

	BOOL result;
	switch(inType) {
	case getFile_Open:
	case getFile_Save:
	{
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFilter = L"All Files\000*.*\000";
		ofn.nFilterIndex = 1;	// Start with .acf files
		
		
		ofn.lpstrFile = file_name;
		if (inType != getFile_Save)
			outFileName[0] = 0;		// No initialization for open.
		ofn.nMaxFile = inBufSize;		// Guess string length?
		ofn.lpstrFileTitle = NULL;	// Don't want file name w/out path

		result = (inType == getFile_Open) ? GetOpenFileNameW(&ofn) : GetSaveFileNameW(&ofn);
		
		strcpy(outFileName, convert_utf16_to_str(file_name).c_str());
		return (result) ? 1 : 0;
	}
	case getFile_OpenImages:
	{
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFilter = L"GeoTiff (*.tif)\0*.tif\0PNG (*.png)\0*.png\0JPEG (*.jpg, *.jpeg)\0*.jpg;*.jpeg\0BMP (*.bmp)\0*.bmp\0DDS (*.dds)\0*.dds\0\0\0";
		ofn.nFilterIndex = 1;	// Start with .acf files

		if (inType != getFile_Save)
			outFileName[0] = 0;		// No initialization for open.
		ofn.nMaxFile = inBufSize;		// Guess string length?
		ofn.lpstrFileTitle = NULL;	// Don't want file name w/out path

		result = (inType == getFile_OpenImages) ? GetOpenFileNameW(&ofn) : GetSaveFileNameW(&ofn);
		return (result) ? 1 : 0;
	}
	case getFile_PickFolder:
	{
		bif.hwndOwner = NULL;
		bif.pidlRoot = NULL;
		bif.pszDisplayName = NULL;
		bif.ulFlags = 0;
		bif.lpfn = NULL;
		bif.lParam = NULL;
		LPITEMIDLIST items = SHBrowseForFolderW(&bif);
		if (items == NULL) return 0;
		result = 0;

		if (SHGetPathFromIDListW(items, file_name))
		{
			result = 1;
			strcpy(outFileName, convert_utf16_to_str(file_name).c_str());
		}
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(items);
			imalloc->Release();
		}
		return result ? 1 : 0;
	}
	}
	return 0;
}


char *	GetMultiFilePathFromUser(
					const char * 		inPrompt,
					const char *		inAction,
					int					inID)
{
	OPENFILENAMEW	ofn = { 0 };
	BOOL result;
	WCHAR * buf = (WCHAR *) malloc(1024 * 1024);

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"All Files\000*.*\000";
	ofn.nFilterIndex = 1;	// Start with .acf files
	ofn.lpstrFile = buf;
	buf[0] = 0;		// No initialization for open.
	ofn.nMaxFile = 1024 * 1024;		// Guess string length?
	ofn.lpstrFileTitle = NULL;	// Don't want file name w/out path
	ofn.lpstrTitle = convert_str_to_utf16(inPrompt).c_str();
	ofn.Flags =  OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	result = GetOpenFileNameW(&ofn);
	if(result)
	{
		vector<string>	files;
		string_utf16 path(buf);
		WCHAR* fptr = buf+path.size()+1;

		// One-file case: we get one complete fully qualified file path, full stop.
		if(*fptr == 0)
		{
			files.push_back(convert_utf16_to_str(path));
		}
		else
		// Multi-file path - we got the dir once in "path" and now we get the null-terminated list of file names.
		while(*fptr)
		{
			files.push_back(convert_utf16_to_str(path + L"\\" + fptr));
			fptr += (wcslen(fptr) + 1);
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
		free(buf);
		return ret;
	}
	else
	{
		free(buf);
		return NULL;
	}
}




void	DoUserAlert(const char * inMsg)
{
	MessageBoxW(NULL, convert_str_to_utf16(inMsg).c_str(), L"Alert", MB_OK + MB_ICONWARNING);
}

int		ConfirmMessage(const char * inMsg, const char * proceedBtn, const char * cancelBtn)
{
	int result = MessageBoxW(
						NULL,				// No Parent HWND
						convert_str_to_utf16(inMsg).c_str(),
						L"X-Plane 10",	// Dialog caption
//						MB_OKCANCEL +
						MB_YESNO +
//						MB_ICONWARNING +
						MB_USERICON +
						MB_DEFBUTTON1);

	return (result == IDOK || result == IDYES) ? 1 : 0;
}

int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	int result = MessageBoxW(
			NULL,
			convert_str_to_utf16(inMessage2).c_str(),
			convert_str_to_utf16(inMessage1).c_str(),
			MB_YESNOCANCEL +
			MB_ICONEXCLAMATION);
	switch(result) {
	case IDCANCEL:	return close_Cancel;
	case IDYES:		return close_Save;
	case IDNO:		return close_Discard;
	default:		return close_Cancel;
	}
}
