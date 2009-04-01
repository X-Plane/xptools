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

#ifndef RF_ASSERT_H
#define RF_ASSERT_H

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

#endif /* RF_ASSERT_H */