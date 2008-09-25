/*
 * Copyright (c) 2004, Laminar Research.
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
#ifndef MEMISTREAMBUF_H
#define MEMISTREAMBUF_H

#include <streambuf>

namespace std {

template <class charT, class traits>
class basic_memstreambuf
	: public basic_streambuf<charT, traits>
{
	typedef basic_streambuf<charT, traits> base;
public:
	typedef charT                     char_type;
	typedef typename traits::int_type int_type;
	typedef typename traits::pos_type pos_type;
	typedef typename traits::off_type off_type;
	typedef traits                    traits_type;

	explicit basic_memstreambuf(const char_type * inBegin, const char_type * inEnd) {
		base::setg(const_cast<char*>(inBegin), const_cast<char*>(inBegin), const_cast<char*>(inEnd)); base::setp(0, 0); }
	virtual ~basic_memstreambuf() { }

protected:

	pos_type seekoff(off_type off, ios_base::seekdir way, ios_base::openmode which)
	{
		if (which == ios_base::out) return pos_type(-1);
		off_type newoff;
		switch (way)
		{
		case ios_base::beg:
			newoff = 0;
			break;
		case ios_base::cur:
			newoff = base::gptr() - base::eback();
			break;
		case ios_base::end:
			newoff = base::egptr() - base::eback();
			break;
		default:
			return pos_type(-1);
		}
		newoff += off;
		if (newoff < 0 || newoff > (base::egptr() - base::eback()))
			return pos_type(-1);
		if (which & ios_base::in)
			base::setg(base::eback(), base::eback() + newoff, base::egptr());
		return pos_type(newoff);
	}


	pos_type seekpos(pos_type sp, ios_base::openmode which)
	{
		if (which == ios_base::out) return pos_type(-1);
		off_type off = sp;
		if (off < 0 || off > (base::egptr() - base::eback()))
			return pos_type(-1);
		if (which & ios_base::in)
			base::setg(base::eback(), base::eback() + off, base::egptr());
		return sp;
	}

};

typedef	basic_memstreambuf<char, char_traits<char> >  memstreambuf;

}

#endif

