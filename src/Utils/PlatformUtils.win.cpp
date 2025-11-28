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

string GetApplicationPath()
{
	WCHAR utf16_path_buf[MAX_PATH];
	if (GetModuleFileNameW(NULL, utf16_path_buf, MAX_PATH))
	{
		return convert_utf16_to_str(utf16_path_buf);
	}
	else
	{
		return "";
	}
}

string GetCacheFolder()
{
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

string GetTempFilesFolder()
{
	WCHAR wc_temp_path[MAX_PATH] = { 0 };
	int result = GetTempPathW(MAX_PATH, wc_temp_path);
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
					int					inBufSize,
					const char*			initialPath)
{
	OPENFILENAMEW	ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);

	WCHAR file_name[MAX_PATH] = { 0 };
	ofn.lpstrFile = file_name;
	ofn.nMaxFile = MAX_PATH;

	ofn.lpstrTitle = convert_str_to_utf16(inPrompt).c_str();

	string_utf16 dummy_path;
	if (initialPath) // hack to overcome windows remembering and overriding an initial path asked for previously
	{
		string_utf16 dummy2 = convert_str_to_utf16(initialPath).c_str();
		if (initialPath[strlen(initialPath) - 1] != '\\')
			dummy2 += L"\\";
		dummy_path = dummy2;
		dummy2 += L"select_file(s)";
		memcpy(file_name, dummy2.c_str(), dummy2.size() * 2 + 1);

		char tmp[16];
		sprintf(tmp, "%08x", (unsigned int)time(0));  // ask for a different, non-existing file EVERY. DARN. TIME.
		dummy_path += convert_str_to_utf16(tmp);       // this causes windows to use the 1st fallback - lpstrFile
		ofn.lpstrInitialDir = dummy_path.c_str();
	}

	BOOL result;
	switch(inType) {
	case getFile_Open:
	case getFile_Save:
	{
		ofn.lpstrFilter = L"All Files\000*.*\000";
		ofn.nFilterIndex = 1;
		
		result = (inType == getFile_Open) ? GetOpenFileNameW(&ofn) : GetSaveFileNameW(&ofn);
		strncpy(outFileName, convert_utf16_to_str(file_name).c_str(), inBufSize);
		return (result) ? 1 : 0;
	}
	case getFile_PickFolder:
	{
		BROWSEINFOW	bif = { 0 };
		bif.hwndOwner = NULL;
		bif.pidlRoot = NULL;
		bif.pszDisplayName = NULL;
		bif.ulFlags = 0;
		bif.lpfn = NULL;
		bif.lParam = NULL;
		LPITEMIDLIST items = SHBrowseForFolderW(&bif);
		if (items == NULL) return 0;

		if (result = SHGetPathFromIDListW(items, file_name))
		{
			strcpy(outFileName, convert_utf16_to_str(file_name).c_str());
		}
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(items);
			imalloc->Release();
		}
		return result;
	}
	}
	return 0;
}

char *	GetMultiFilePathFromUser(
					const char * 		inPrompt,
					const char *		inAction,
					int					inID,
					const char *		initialPath)
{
	OPENFILENAMEW	ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	WCHAR * buf = (WCHAR *) malloc(1024 * 1024);
	buf[0] = 0;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = 1024 * 1024;

	ofn.lpstrTitle = convert_str_to_utf16(inPrompt).c_str();
	ofn.lpstrFilter = L"All Files\0*.*\0\0\0";
	ofn.nFilterIndex = 1;

	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrFileTitle = NULL;	// Don't want output that delivers file names w/out path

	string_utf16 dummy_path;
	if (initialPath) // hack to overcome windows remembering and overriding an initial path asked for previously
	{
		string_utf16 dummy2 = convert_str_to_utf16(initialPath).c_str();
		if (initialPath[strlen(initialPath) - 1] != '\\')
			dummy2 += L"\\";
		dummy_path = dummy2;
		dummy2 += L"select_file(s)";
		memcpy(buf, dummy2.c_str(), dummy2.size() * 2 + 1);

		char tmp[16];
		sprintf(tmp, "%08x", (unsigned int) time(0));  // ask for a different, non-existing file EVERY. DARN. TIME.
		dummy_path += convert_str_to_utf16(tmp);       // this causes windows to use the 1st fallback - lpstrFile
		ofn.lpstrInitialDir = dummy_path.c_str();
	}

	if(GetOpenFileNameW(&ofn))
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
	LOG_MSG("I/Alert %s\n",inMsg);
	HWND thisWin = GetForegroundWindow();
	MessageBoxW(thisWin, convert_str_to_utf16(inMsg).c_str(), L"Alert", MB_OK | MB_ICONWARNING | MB_TOPMOST |MB_TASKMODAL);
}

const char *yes_text, *no_text, *cancel_text;

LRESULT CALLBACK ConfirmMessageProc(int message, WPARAM wParam, LPARAM lParam)
{
	if (message== HCBT_ACTIVATE)
	{
		if (yes_text)
		{
			SetDlgItemTextA((HWND)wParam, IDYES, yes_text);
			SendMessage(GetDlgItem((HWND)wParam, IDYES), TB_SETBUTTONSIZE, 0, MAKELPARAM(80, 30));
		}
		if(no_text)     SetDlgItemTextA((HWND) wParam, IDNO, no_text);
		if(cancel_text) SetDlgItemTextA((HWND) wParam, IDCANCEL, cancel_text);
	}
	return FALSE;
}

int		ConfirmMessage(const char* inMsg, const char* proceedBtn, const char* cancelBtn, const char* optionBtn)
{
	yes_text = proceedBtn;
	if (optionBtn)
	{
		no_text = optionBtn;
		cancel_text = cancelBtn;
	}
	else
	{
		no_text = cancelBtn;
		cancel_text = nullptr;
	}

	HHOOK hook = SetWindowsHookEx(WH_CBT, ConfirmMessageProc, NULL, GetCurrentThreadId());
	int result = MessageBoxW(GetForegroundWindow(),
						convert_str_to_utf16(inMsg).c_str(),
						L"WED",
						MB_TASKMODAL |          // works most of the time even with no HWND
						MB_TOPMOST |            // we really need to prevent this popup to go behind another WED window
						(optionBtn ? MB_YESNOCANCEL : MB_YESNO) |
						MB_ICONQUESTION |
						MB_DEFBUTTON1);
	UnhookWindowsHookEx(hook);

	if (result == IDYES) 
		return 1;    // proceedBtn
	if (optionBtn && result == IDNO)
		return 2;    // optionBtn
	return 0;        // cancelBtn
}


int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	HWND thisWin = GetForegroundWindow();
	int result = MessageBoxW(
			thisWin,
			convert_str_to_utf16(inMessage2).c_str(),
			convert_str_to_utf16(inMessage1).c_str(),
			MB_YESNOCANCEL | MB_TOPMOST | MB_TASKMODAL |
			MB_ICONEXCLAMATION);
	switch(result) {
	case IDYES:		return close_Save;
	case IDNO:		return close_Discard;
	default:		return close_Cancel;
	}
}
