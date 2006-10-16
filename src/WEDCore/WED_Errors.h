#ifndef WED_ERRORS_H
#define WED_ERRORS_H

#include <exception>

using std::exception;

#define	WED_ThrowOSErr(x) \
	if ((x) != 0)	throw wed_error_exception(x, __FILE__, __LINE__)

#define WED_ThrowMessage(x) \
					throw wed_error_exception((x), __FILE__, __LINE__)

#define WED_ThrowMessageIf(x, y) \
	if ((x)) throw wed_error_exception((y), __FILE__, __LINE__)

#define WED_ThrowPrintf(fmt, ...) \
 		throw wed_error_exception(__FILE__, __LINE__, fmt, __VA_ARGS__)

#ifndef _MSL_THROW
#define _MSL_THROW throw()
#endif

class wed_error_exception : public exception {
public:
	wed_error_exception(int os_error_code, const char * file, int line) _MSL_THROW ;
	wed_error_exception(const char * inMessage, const char * file, int line) _MSL_THROW ;
	wed_error_exception(const char * file, int line, const char * fmt, ...) _MSL_THROW ;
	wed_error_exception(const wed_error_exception& rhs) _MSL_THROW ;
	
	wed_error_exception& operator=(const wed_error_exception& rhs);
	
    virtual const char* what () const _MSL_THROW;

private:

	wed_error_exception() _MSL_THROW ;

	const char *		mFile;
	int					mLine;	
	char				mMsg[1024];

};

const char *	WED_StringForOSError(int code);
void			WED_ReportExceptionUI(const exception& what, const char * inFmt, ...);

#endif
