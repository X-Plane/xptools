
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

#include "WED_Assert.h"
#include "AssertUtils.h"
#include "PlatformUtils.h"
#include "FileUtils.h"

static char gAssertBuf[512];

void WED_AssertHandler_f(const char * condition, const char * file, int line)
{
	snprintf(gAssertBuf, 512, "WED has hit an error '%s' due to a bug. Please report on gatewaybugs.x-plane.com "
	                          "and attach the file %sWED_Log.txt", condition, FILE_get_dir_name(GetApplicationPath()).c_str());
	DoUserAlert(gAssertBuf);
	throw wed_assert_fail_exception(gAssertBuf, file, line);
}

void	WED_AssertInit(void)
{
	InstallDebugAssertHandler(WED_AssertHandler_f);
	InstallAssertHandler(WED_AssertHandler_f);
}
