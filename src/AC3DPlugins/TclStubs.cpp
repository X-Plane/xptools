/*
 * Copyright (c) 2008, Laminar Research.
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

#include "TclStubs.h"
#if APL || LIN
#include <dlfcn.h>
#include <stdlib.h>
#endif

TCL_stubs tcl_stubs = { 0 };

const char * TCL_init_stubs(void)
{
	int lost = 0;
#if IBM
	HMODULE	hmod = GetModuleHandle(L"tcl86t");
	if(hmod == NULL) return "Could not locate tcl8.6";
#endif
#if APL || LIN
#define TCL_PROC(ret,name,args) \
	tcl_stubs.name = (ret (*) args) dlsym(RTLD_DEFAULT,#name); \
	if(tcl_stubs.name == NULL) ++lost;
#else
#define TCL_PROC(ret,name,args) \
	tcl_stubs.name = (ret (*) args) GetProcAddress(hmod,#name); \
	if(tcl_stubs.name == NULL) ++lost;
#endif
	TCL_PROCS
#undef TCL_PROC
	if (lost != 0) return "Could not locate all TCL functions.";
	return NULL;
}
