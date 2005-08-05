#ifndef __X86_WIN32_DLL_H__
#define __X86_WIN32_DLL_H__

/*	This header shows the recommended method of 
	implementing a DLL interface.  */
	
/*	This macro is defined by the DLL sources itself when
	it is built, to export the following functions.  
	When a client uses the DLL, this test changes the 
	macro so it will import the functions.  */

#ifndef IMPEXP
#define IMPEXP	__declspec(dllimport)
#endif

IMPEXP int RectFrame(HDC hdc, int x1, int y1, int x2, int y2, int t);
IMPEXP int EllipseFrame(HDC hdc, int x1, int y1, int x2, int y2, int t);

#endif /*#ifndef __X86_WIN32_DLL_H__*/
