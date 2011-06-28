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

static char gAssertBuf[65536];

static const char * trim_file(const char * p)
{
	const char * ret = p;
	while(*p)
	{
		if(*p == '\\' || *p == ':' || *p == '/')
			ret = p+1;
		++p;
	}
	return ret;
}



void WED_AssertHandler_f(const char * condition, const char * file, int line)
{
	FILE * efile = fopen("error.out", "a");
	fprintf(efile ? efile : stderr, "ASSERTION FAILED: %s (%s:%d.)\n", condition, trim_file(file), line);
	if (efile) fclose(efile);

	sprintf(gAssertBuf, "WorldEditor has hit an error due to a bug.  Please report the following to Ben:\n"
						"%s (%s:%d.)\n", condition, trim_file(file), line);

	DoUserAlert(gAssertBuf);

	throw wed_assert_fail_exception(condition, file, line);
}

void	WED_AssertInit(void)
{
	InstallDebugAssertHandler(WED_AssertHandler_f);
	InstallAssertHandler(WED_AssertHandler_f);
}
