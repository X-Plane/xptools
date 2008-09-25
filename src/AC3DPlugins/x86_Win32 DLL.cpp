/*
 *  Copyright © 1997-2002 Metrowerks Corporation.  All Rights Reserved.
 *
 *  Questions and comments to:
 *       <mailto:support@metrowerks.com>
 *       <http://www.metrowerks.com/>
 */

/*	This is a basic Win32 DLL. */

/* 	Defining the following macro before including <windows.h>
	can save you minutes of build time. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

/*	Declare our exports, defining IMPEXP only inside the DLL */
#define IMPEXP	__declspec(dllexport)
#include "x86_Win32 DLL.h"

/*	This DLL user entry point function is only required when your application
	has special initialization or termination requirements.  Otherwise
	the MSL runtime provides the appropriate startup code.  */
BOOL WINAPI DllMain ( HINSTANCE hInst, DWORD wDataSeg, LPVOID lpvReserved );

BOOL WINAPI DllMain( HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved )
{
	char buf[512];
	char exename[256];
	char dllname[256];

	GetModuleFileName(0L, exename, sizeof(exename));
	GetModuleFileName(hInst, dllname, sizeof(dllname));

	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			snprintf(buf, sizeof(buf), "Process\n'%s'\nattaching to\n'%s'...", exename, dllname);
			MessageBox ( GetFocus(), buf, "Notice", MB_OK|MB_SYSTEMMODAL );
			return 1;

		case DLL_PROCESS_DETACH:
			snprintf(buf, sizeof(buf), "Process\n'%s'\ndetaching from\n'%s'...", exename, dllname);
			MessageBox ( GetFocus(), buf, "Notice", MB_OK|MB_SYSTEMMODAL );
			return 1;

		default:
			return 1;
   	}
   	return 0;
}

IMPEXP int RectFrame(HDC hdc, int x1, int y1, int x2, int y2, int t)
{
	Rectangle(hdc, x1, y1, x2, y2);
	Rectangle(hdc, x1+t, y1+t, x2-t, y2-t);
	return 1;
}

IMPEXP int EllipseFrame(HDC hdc, int x1, int y1, int x2, int y2, int t)
{
	Ellipse(hdc, x1, y1, x2, y2);
	Ellipse(hdc, x1+t, y1+t, x2-t, y2-t);
	return 1;
}
