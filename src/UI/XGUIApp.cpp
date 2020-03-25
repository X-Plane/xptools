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
#include "XGUIApp.h"
#include "XWin.h"
#include "XUtils.h"
#include "PlatformUtils.h"
#include "ObjCUtils.h"

class	XGrinderWin;

#if IBM
HINSTANCE	gInstance = NULL;
#endif

void	XGrinder_Quit(void)
{
	exit(0);
}

#if APL
int		main(int argc, char ** argv)
{
	init_ns_stuff();

	XGrindInit(argc,argv);

	vector<string> files;

	for(int n = 1; n < argc; ++n)
		files.push_back(argv[n]);
	if(!files.empty())
		XGrindFiles(files);

	run_app();
	return 0;
}
#endif

#if IBM
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	gInstance = hInstance;

	MSG msg;
//	HACCEL hAccelTable;

	XWin::RegisterClass(hInstance);
	if(OleInitialize(NULL) != S_OK)
		return FALSE;

	XGrindInit();

	while (GetMessage(&msg, NULL, 0, 0))
	{
//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
//		}
	}

	OleUninitialize();
	return msg.wParam;
}
#endif


#pragma mark -


#if LIN
int main(int argc, char* argv[])
{
	XGrindInit(argc,argv);
	setlocale(LC_ALL,"C");
	return Fl::run();
}
#endif

