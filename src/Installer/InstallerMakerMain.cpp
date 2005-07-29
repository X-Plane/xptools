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
#include "BuildInstaller.h"
#include "PlatformUtils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if APL
#define Sleep(x)  sleep(x/1000)
#endif

#if IBM
int PASCAL WinMain(HINSTANCE hinst,HINSTANCE hprevinst,LPSTR cmdline,int windowstyle)
#else
int main(void)
#endif
{
	// UI TEST CODE - IGNORE
#if 0
	ShowProgressMessage("This is a test message.", NULL);
	Sleep(1000);
	float v = -1.0;
	ShowProgressMessage("This also is a message.  It is quite a bit longer than the other messages but that should be ok.", &v);
	Sleep(1000);
	ShowProgressMessage("Third message.", NULL);
	Sleep(1000);
	for (int n = 0; n < 100; ++n)
	{
		v = (float) n / (99.0);
		ShowProgressMessage("Working hard", &v);
		Sleep(10);
	}
	ShowProgressMessage("4th message.", NULL);
	Sleep(1000);
#endif
	
	char	opath[1024], npath[1024], epath[1024];
	opath[0] = npath[0] = epath[0] = 0;
	if (GetFilePathFromUser(getFile_PickFolder,"Please pick the old X-System Folder", "Choose", 0,opath))
	if (GetFilePathFromUser(getFile_PickFolder,"Please pick the new X-System Folder", "Choose", 1,npath))
	if (GetFilePathFromUser(getFile_Open,"Please pick the installer to build.", "Build", 2, epath))
	{	
		try {
			int n = BuildInstaller(opath, npath, epath);
			char	buf[1024];
			sprintf(buf, "Successfully added %d files.", n);
			if (n == 0)
				DoUserAlert("No files are different.  Installer not useful and will not work.");		
			else
				DoUserAlert(buf);
			
		} catch (...) {
			DoUserAlert("Unknown exception caught!");		
		}
	}
	
	return 1;
}
