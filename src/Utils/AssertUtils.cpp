#include "AssertUtils.h"
#include <stdio.h>
#include <exception>
#include <stdarg.h>
using std::exception;

class assert_fail_exception : public exception {
public:
	assert_fail_exception(const char * c, const char * f, int l) _MSL_THROW 
		: c_(c), f_(f), l_(l) {}
	assert_fail_exception(const assert_fail_exception& x) _MSL_THROW
		: c_(x.c_), f_(x.f_), l_(x.l_) {}
	assert_fail_exception& operator=(const assert_fail_exception& x) _MSL_THROW { 
		c_ = x.c_; f_ = x.f_; l_ = x.l_; return *this; }
	virtual ~assert_fail_exception() _MSL_THROW {};
	virtual const char* what() const _MSL_THROW { return c_; };
	const char * c_;
	const char * f_;
		  int    l_;
};


static	void	DefaultDebugAssert(const char * cond, const char * file, int line)
{
	printf("Debug Assert Failed: %s (%s, %d.)\n", cond, file, line);
	throw assert_fail_exception(cond,file, line);
}

static	void	DefaultAssert(const char * cond, const char * file, int line)
{
	printf("Assert Failed: %s (%s, %d.)\n", cond, file, line);
	throw assert_fail_exception(cond,file, line);
}

static	AssertHandler_f		sAssertHandler = DefaultAssert;
static	AssertHandler_f		sDebugAssertHandler = DefaultDebugAssert;

void	AssertPrintf(const char * fmt, ...)
{
	va_list	arg;
	va_start(arg, fmt);
	static char buf[4096];	
	vsnprintf(buf, sizeof(buf), fmt, arg);

	__AssertHandler(buf, __FILE__, __LINE__);
}


void __DebugAssertHandler(const char * c, const char * f, int l)
{
	if (sDebugAssertHandler)	sDebugAssertHandler(c, f, l);
	else 						DefaultDebugAssert(c,f,l);
}

void __AssertHandler(const char * c, const char * f, int l)
{
	if (sAssertHandler)	sAssertHandler(c, f, l);
	else 				DefaultAssert(c,f,l);
}

typedef void (* AssertHandler_f)(const char * condition, const char * file, int line);

AssertHandler_f		InstallDebugAssertHandler(AssertHandler_f f)
{
	swap(sDebugAssertHandler, f);
	return f;
}
	
AssertHandler_f		InstallAssertHandler(AssertHandler_f f)
{
	swap(sAssertHandler, f);
	return f;
}

#pragma mark -

bool	sTestInteractive = false;

void	TEST_SetInteractive(bool interactive)
{
	sTestInteractive = interactive;
}

bool	TEST_Handler(const char * c, const char * f, int l)
{
	printf("Test failed: %s\n(%s, %d.)\n", c, f, l);
	if (sTestInteractive)
	{
		return true;
	}
	return false;
}
