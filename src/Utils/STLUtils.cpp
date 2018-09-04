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

#include "STLUtils.h"
#include <cctype>
#include <iostream>
#include <string>

bool ci_char_traits::eq(char c1, char c2)
{
	return std::toupper(c1) == std::toupper(c2);
}

bool ci_char_traits::lt(char c1, char c2)
{
	return std::toupper(c1) <  std::toupper(c2);
}

int ci_char_traits::compare(const char* s1, const char* s2, size_t n)
{
	while (n-- != 0) {
		if (std::toupper(*s1) < std::toupper(*s2)) return -1;
		if (std::toupper(*s1) > std::toupper(*s2)) return 1;
		++s1; ++s2;
	}
	return 0;
}

const char* ci_char_traits::find(const char* s, int n, char a)
{
	const int ua(std::toupper(a));
	while (n-- != 0)
	{
		if (std::toupper(*s) == ua)
			return s;
		s++;
	}
	return NULL;
}

std::ostream& operator<<(std::ostream& os, const ci_string& str)
{
	return os.write(str.data(), str.size());
}

void str_replace_all(string& s, const string& a, const string& b){
	string::size_type p=0;
	while((p=(s.find(a,p)))!=s.npos){
		s.erase(p,a.size());
		s.insert(p,b);
		p += b.size();}
}
