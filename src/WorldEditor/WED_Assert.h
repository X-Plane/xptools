#ifndef WED_ASSERT_H
#define WED_ASSERT_H

#include <exception>

using std::exception;

class wed_assert_fail_exception : public exception {
public:
	wed_assert_fail_exception(const char * c, const char * f, int l) _MSL_THROW 
		: c_(c), f_(f), l_(l) {}
	wed_assert_fail_exception(const wed_assert_fail_exception& x) _MSL_THROW
		: c_(x.c_), f_(x.f_), l_(x.l_) {}
	wed_assert_fail_exception& operator=(const wed_assert_fail_exception& x) _MSL_THROW { 
		c_ = x.c_; f_ = x.f_; l_ = x.l_; return *this; }
	virtual ~wed_assert_fail_exception() _MSL_THROW {};
	virtual const char* what() const _MSL_THROW { return c_; };
	const char * c_;
	const char * f_;
		  int    l_;
};


void	WED_AssertInit(void);

#endif /* WED_ASSERT_H */