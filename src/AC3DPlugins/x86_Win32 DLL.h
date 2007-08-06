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
