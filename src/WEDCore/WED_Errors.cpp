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
