#include "GUI_Clipboard.h"

#if APL
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#elif IBM
#include <Windows.h>
#endif

bool	GUI_GetTextFromClipboard(string& outText)
{
#if IBM
		HGLOBAL   	hglb;
		LPTSTR    	lptstr;
		bool		retVal = false;

	if (IsClipboardFormatAvailable(CF_TEXT))
		return false;

	if (OpenClipboard(NULL))
		return false;

	hglb = GetClipboardData(CF_TEXT);
	if (hglb != NULL)
	{
		lptstr = (LPSTR)GlobalLock(hglb);
		if (lptstr != NULL)
		{
			outText = lptstr;
			GlobalUnlock(hglb);
			retVal = true;
		}
	}

	CloseClipboard();

	return retVal;
#elif APL
		ScrapRef	scrap;
		SInt32		byteCount = 0;

	if (::GetCurrentScrap(&scrap) != noErr)
		return false;

	OSStatus	status = ::GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &byteCount);
	if (status != noErr)
		return false;

	outText.resize(byteCount);

	return (::GetScrapFlavorData(scrap, kScrapFlavorTypeText, &byteCount, &*outText.begin() ) == noErr);
#else
	#error NOT implemented
#endif
}

bool	GUI_SetTextToClipboard(const string& inText)
{
#if IBM
		LPTSTR  lptstrCopy;
		HGLOBAL hglbCopy;

	if (OpenClipboard(NULL))
		return false;
	EmptyClipboard();

	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR) * (inText.length() + 1));
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return false;
	}

	lptstrCopy = (LPSTR)GlobalLock(hglbCopy);
	strcpy(lptstrCopy, inText.c_str());
	GlobalUnlock(hglbCopy);

	SetClipboardData(CF_TEXT, hglbCopy);
	CloseClipboard();
	return true;
#elif APL
		ScrapRef	scrap;
	if (::ClearCurrentScrap() != noErr) return false;
	if (::GetCurrentScrap(&scrap) != noErr) return false;

	return ::PutScrapFlavor( scrap, kScrapFlavorTypeText, kScrapFlavorMaskNone, inText.size(), &*inText.begin()) == noErr;
#else
	#error NOT implemented
#endif
}
