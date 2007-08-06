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

#include "WED_Errors.h"
#include <stdio.h>
#include "PlatformUtils.h"
#include <stdarg.h>

wed_error_exception::wed_error_exception(int os_error_code, const char * file, int line) _MSL_THROW
{
	snprintf(mMsg, sizeof(mMsg), "OS Error %d (%s)", os_error_code, WED_StringForOSError(os_error_code));
	mFile = file;
	mLine = line;
}

wed_error_exception::wed_error_exception(const char * inMessage, const char * file, int line) _MSL_THROW
{
	mFile = file;
	mLine = line;
	strncpy(mMsg, inMessage, sizeof(mMsg)-1);
	mMsg[sizeof(mMsg)-1] = 0;
}

wed_error_exception::wed_error_exception(const char * file, int line, const char * fmt, ...) _MSL_THROW
{
	mFile = file;
	mLine = line;
	va_list	arg;
	va_start(arg, fmt);
	vsnprintf(mMsg, sizeof(mMsg), fmt, arg);
	
}

wed_error_exception::wed_error_exception(const wed_error_exception& rhs) _MSL_THROW
{
	strncpy(mMsg, rhs.mMsg, sizeof(mMsg));
	mFile = rhs.mFile;
	mLine = rhs.mLine;
}
	
wed_error_exception& wed_error_exception::operator=(const wed_error_exception& rhs)
{
	strncpy(mMsg, rhs.mMsg, sizeof(mMsg));
	mFile = rhs.mFile;
	mLine = rhs.mLine;
	return *this;
}
	
const char* wed_error_exception::what () const _MSL_THROW
{
	return mMsg;
}

const char *	WED_StringForOSError(int code)
{
	return "unknown error";
}

void			WED_ReportExceptionUI(const exception& what, const char * fmt, ...)
{
	char	msg[2048];
	va_list	arg;
	va_start(arg, fmt);
	vsnprintf(msg, sizeof(msg), fmt, arg);
	
	strcat(msg, what.what());
	
	DoUserAlert(msg);
}
