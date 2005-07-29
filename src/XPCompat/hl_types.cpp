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
#include "hl_types.h"
#if APL && !defined(__MACHO__)
#include <Files.h>
#endif

#include "PlatformUtils.h"

void  __MACIBM_alert(xint close_to,	string C_string1,			// these guys are of unknown length because
									string C_string2,			// we are always sending in string literals
									string C_string3,			// of completely varying lengths!
									string C_string4,xint type,	// therefore we can NOT dimension the pointers here!
									const xchr * file,xint line)
{
	static char bigbuf[4096];
	
	sprintf(bigbuf, "%s\n%s\n%s\n%s\n%s, %d.",
		C_string1.c_str(),
		C_string2.c_str(),
		C_string3.c_str(),
		C_string4.c_str(),
		file, line);
	DoUserAlert(bigbuf);
	exit(1);

}
