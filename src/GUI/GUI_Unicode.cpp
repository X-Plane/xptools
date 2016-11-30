/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "GUI_Unicode.h"

string_utf8  convert_str_to_utf8(const string& str)
{
	return reinterpret_cast<const UTF8*>(str.c_str());
}

string_utf16 convert_str_to_utf16(const string& str)
{
	return convert_utf8_to_utf16(convert_str_to_utf8(str));
}

string       convert_utf8_to_str(const string_utf8& str_utf8)
{
	return reinterpret_cast<const char*>(str_utf8.c_str());
}

string_utf16 convert_utf8_to_utf16(const string_utf8&  str_utf8)
{
	string_utf16 str_utf16;
	string_utf_8_to_16(str_utf8, str_utf16);
	return str_utf16;
}

string       convert_utf16_to_str(const string_utf16& str_utf16)
{
	string_utf8 str_utf8;
	string_utf_16_to_8(str_utf16, str_utf8);
	return convert_utf8_to_str(str_utf8);
}

string_utf8 convert_utf16_to_utf8(const string_utf16&  str_utf16)
{
	string_utf8 str_utf8;
	string_utf_16_to_8(str_utf16, str_utf8);
	return str_utf8;
}

// These allocate memory - there is no point in inlining them as they will make a pile o function calls anyway.
void	string_utf_8_to_16(const string_utf8& input, string_utf16& output)
{
	output.clear();
	const UTF8 * p = input.c_str();
	const UTF8 * e = p + input.size();
	while (p < e)
	{
		UTF32 c = UTF8_decode(p);
		p = UTF8_next(p);
		UTF16 b[2];
		int n = UTF16_encode(c,b);
		output.insert(output.end(),b,b+n);
	}
}

void convert_utf_16_to_8(const string_utf16& input, string& output)
{
	string_utf8 output_utf8;
	string_utf_16_to_8(input, output_utf8);
	output = reinterpret_cast<const char*>(output_utf8.c_str());
}

void	string_utf_16_to_8(const string_utf16& input, string_utf8& output)
{
	output.clear();
	const UTF16 * p = (const UTF16 *) input.c_str();
	const UTF16 * e = p + input.size();
	while(p < e)
	{
		UTF32 c;
		p = UTF16_decode(p,c);
		UTF8 b[4];
		int n = UTF8_encode(c,b);
		output.insert(output.end(),b,b+n);
	}
}


#if APL

#include <CoreFoundation/CFString.h>

static void	CFStringToSTL(CFStringRef cfstr, string& stl_str, CFStringEncoding encoding)
{
	CFIndex blen, blen2, clen, clen2, slen = CFStringGetLength(cfstr);
	clen = CFStringGetBytes(cfstr,CFRangeMake(0, slen), encoding, 0, FALSE, NULL, 0, &blen);	
	assert(clen==slen);
	vector<UTF8>	buf(blen);
	clen2 = CFStringGetBytes(cfstr,CFRangeMake(0,slen), encoding, 0, FALSE, &*buf.begin(), blen, &blen2);	
	assert(clen==clen2);
	assert(blen==blen2);
	stl_str = string(buf.begin(),buf.end());
}

UTF32 script_to_utf32(int c)
{
	char buf[2] = { c, 0 };
	CFStringRef	s = CFStringCreateWithCString(kCFAllocatorDefault,buf,CFStringGetSystemEncoding());
	if(s== NULL) return (c < 0x80 ? c : 0);
	
	string utf8;
	CFStringToSTL(s,utf8,kCFStringEncodingUTF8);
	CFRelease(s);
	return UTF8_decode((const UTF8*)utf8.c_str());
}
#endif