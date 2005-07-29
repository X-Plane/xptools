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
#include "InstallerRun.h"
#include "PlatformUtils.h"
#include "InstallerScript.h"
#include <string.h>
#include <stdio.h>

/*

	TODO:
		Test installer (as opposed to updater) from CDs.
		Test installer (as opposed to updater) on Windows.
		Test updating function in updater
		Add more error checking and other useful stuff to the installer script to make it pretty.
		Add a way to name and put in a good place the dir that the system makes.

	TODO:
		(I think these are done and just need to be double-checked.
		ANCHOR ON THE MAIN FILE - confirm the main file isn't totally bogus.
		ERROR CHECKING
			do on installerprocs.cpp and installerrun/buildinstaller
			- Check result codes on all ops
			- Report all errors to log/UI.
			- halt on errs??

	(Stretch)
		NICER LOOKING UI
		IGNORE TRIVIAL DIFFERENCES - this will limit the size of patches.
 */

#if IBM
int PASCAL WinMain(HINSTANCE hinst,HINSTANCE hprevinst,LPSTR cmdline,int windowstyle)
#else
int main(void)
#endif
{
	char	fpath[1024];
#if APL
	FILE * script = fopen("installer_mac.txt", "r");
#else
	FILE * script = fopen("installer_win.txt", "r");
#endif	
	if (script != NULL)
	{
		strcpy(fpath, "X-System 800");
		if (GetFilePathFromUser(getFile_Save,"Please Pick A Location To Install X-Plane 8","Install",0, fpath))
		{
			try {
				RunScript(script, fpath);
			} catch(...) {
				DoUserAlert("An unknown error occured.");		
			}	
		}
		fclose(script);
	} else {
		
		fpath[0] = 0;
		if (GetFilePathFromUser(getFile_PickFolder,"Please pick the X-System Folder","Update",0, fpath))
		{
			try {
				RunInstaller(fpath);
				DoUserAlert("The update was successful!");		
			} catch(...) {
				DoUserAlert("An unknown error occured.");		
			}
		}
	}
	return 1;
}
