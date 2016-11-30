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

#ifndef GUI_Unicode_H
#define GUI_Unicode_H

#include <assert.h>

/*

	UNICODE - THEORY

	Unicode is a character system that uses much larger numbers (32-bit ints) to define characters - this means that we have enough numbers to represent just about
	every character in the universe - including tens of thousands of Asian characters.  The advantage of unicode is that (since we don't use one value for two chars)
	we can have text in multiple languages at once.  Without unicode the number 195 might be "ekratka" in Russian and "U with umlat" in German...but a user can easily
	have EITHER in their folder names...by using unicode we ensure we never get the language wrong.

	There are three encodings we care about:

	UTF8	- unicode chars go in between one and four bytes...higher number unicode chars take more bytes in a row.  This is the default encoding for Linux,
			  OS X, and X-Plane.  UTF8 is set up so that the ASCII chars (0-127) are simply the one-byte values 0-127 when encoded (that is, ASCII doesn't change) and
			  no character that uses multiple bytes contains a valid UTF8 character inside it.  (So for example, one byte of a chinese char won't happen to be the dir char.)

			  The only tricky thing about UTF8 is that when we are editing text, when the user hits delete, we have to anlayze the unicode char to find out how many bytes
			  we have to delete.

			  UTF8 uses some of the bits of the chars to indicate how mutliple bytes become one char...some combinations of high bits are illegal (because they represent
			  bogus control bits).  Thus given a bunch of bytes, we can try to determine whether they are really UTF8 (e.g. the control bits make sense) or not.

	UTF16	- unicode chars go in between one and two 16-bit ints (shorts) depending on the value - most chars are just one short.  No valid two-short char contains a valid
			  one-short char.  (This is just like UTF8).  This is the default encoding for Windows, but requires a conversion, as ASCII ends up with a bunch of zeros in between
			  each char.  We will only use UTF16 when talking directly to Windowws.

	UTF32	- unicode chars go in one uint32_t.  Since an uint32_t is frickin huge we never need more than one.  No one uses UTF32 in practice because it is too wasteful of memory.
			  But we will use UTF32 when we have to encode a single unicode character to make things simple.
*/

typedef	unsigned char		UTF8;													// Definitions for chars in each of the 3 encoding formats.
#if IBM
typedef wchar_t UTF16;
#else
typedef	unsigned short		UTF16;													// We will use our normal string for UTF8.  We do not make a
#endif
typedef unsigned int		UTF32;													// string for UTF32 because it is only used for 1 char at a time.

typedef basic_string<UTF16> string_utf16;

#if IBM
string_utf16 convert_str_to_utf16(const string& str);
string       convert_utf16_to_str(const string_utf16& str_utf16);
#endif

// UTF 8 routines to move through a string one unicode char at a time - could be any number of bytes.
inline		 UTF8 *		UTF8_align(		 UTF8 * string);
inline const UTF8 *		UTF8_align(const UTF8 * string);

inline		 UTF8 *		UTF8_prev(		 UTF8 * string);
inline const UTF8 *		UTF8_prev(const UTF8 * string);

inline		 UTF8 *		UTF8_next(		 UTF8 * string);
inline const UTF8 *		UTF8_next(const UTF8 * string);

inline bool				UTF8_IsValid(const UTF8 * start, const UTF8 * end);					// Is this a valid UTF8 string?  UTF8 has string bit-patterns; often we can tell if we are UTF8 or not!
inline bool				UTF8_IsValid(const string& utf8_str);								// Just for convenience.
inline const UTF8 *		UTF8_ValidRange(const UTF8 * s, const UTF8 * e);					// Return a ptr within [s,e) that is the first inval char.  Thus [s,ret) is the longest valid range starting at S.
inline const UTF8 *		UTF8_InvalidRange(const UTF8 * s, const UTF8 * e);					// Return a ptr within [s,e) that is the first val char.  Thus [s,ret) is the longest invalid range starting at S.

// UTF 8 and 16 encode and decode routines

inline UTF32			UTF8_decode(const UTF8 * chars);									// Decode UTF8 - chars must contain enough chars to make one valid unicode char.
inline int				UTF8_encode(UTF32 inChar, UTF8 outChars[4]);						// Encode char into buf, return number of bytes was filled.

inline const UTF16 *	UTF16_decode(const UTF16 * chars, UTF32& result);					// UTF 16 decoder.  Advance one unicode char, return UTF32 result.
inline int				UTF16_encode(UTF32 inChar, UTF16 outChars[2]);						// UTF 16 encoder.  Fill buffer, return how many UTF16s was filled.

// String-based UTF 16...this is handy for dealing with file paths on Windows.
void	string_utf_8_to_16(const string& input, string_utf16& output);				// convert between UTF8 and 16 in STL strings.
void	string_utf_16_to_8(const string_utf16& input, string& output);

#if APL
// Mac-specific: convert a system-script character to UTF32.  The system script might be macRoman or ISO-latin-1 or who-knows-what!
UTF32 script_to_utf32(int c);
#endif

/********************************************************************************************************************************************************************************
 * INLINE FUNCTIONS - KEEP IN THE HEADER FOR SPEED WHEN PROCESSING STRINGS!
 ********************************************************************************************************************************************************************************/

// Align: any time we have a non-lead character, back up.  We'll hit the root eventually.
inline UTF8 *	UTF8_align(		 UTF8 * string)
{
	while((*string & 0xC0) == 0x80)
		--string;
	return string;
}

inline const UTF8 *	UTF8_align(const UTF8 * string)
{
	while((*string & 0xC0) == 0x80)
		--string;
	return string;
}

// Prev.  Align, back up, then back up beyond lead chars.
inline UTF8 *	UTF8_prev(		 UTF8 * string)
{
	string = UTF8_align(string);
	--string;
	while((*string & 0xC0) == 0x80)
		--string;
	return string;
}

inline const UTF8 *	UTF8_prev(const UTF8 * string)
{
	string = UTF8_align(string);
	--string;
	while((*string & 0xC0) == 0x80)
		--string;
	return string;
}

// Next.  Align, go forward, then skip lead chars.
inline UTF8 *	UTF8_next(		 UTF8 * string)
{
	string = UTF8_align(string);
	++string;
	while((*string & 0xC0) == 0x80)
		++string;
	return string;
}

inline const UTF8 *	UTF8_next(const UTF8 * string)
{
	string = UTF8_align(string);
	++string;
	while((*string & 0xC0) == 0x80)
		++string;
	return string;
}


inline bool			UTF8_IsValid(const UTF8 * s, const UTF8 * e)
{
	while(s < e)
	{
		if ((*s & 0x80) == 0)			{ ++s; continue;	}			// ASCII case - advance one and done
		int extra_bytes = 0;											// 3 lead bit patters; calc extra bytes
		     if((*s & 0xE0) == 0xC0)	{ extra_bytes = 1; }			// 110x xxxx
		else if((*s & 0xF0) == 0xE0)	{ extra_bytes = 2; }			// 1111 0xxx
		else if((*s & 0xF8) == 0xF0)	{ extra_bytes = 3; }			// 1111 10xx
		else return false;												// Lead bytes don't match anything legit - not UTF8
		++s;
		while(extra_bytes--)
		{
			if(s >= e)					return false;					// End of string without adequate bytes for possible trailing.
			if ((*s++ & 0xC0) != 0x80)	return false;					// For each trailing byte, if not 10xxxxxx, fail
		}
	}
	return true;
}

inline const UTF8 *		UTF8_ValidRange(const UTF8 * s, const UTF8 * e)
{
	// For every valid UTF8 start, we are going to advance over that char
	// IF it is valid.  Once we hit a badly formed UTF8 char _or_ a
	// badly starting char (e.g. 10xxxxxx up front) we bail.
	while(s < e && ((*s & 0xC0) != 0x80))
	{
		const UTF8 * nc = s;
		++nc;
		while(nc < e && ((*nc & 0xC0) == 0x80))
			++nc;		
		if(!UTF8_IsValid(s,nc))
			return s;			
		s = nc;
	}
	return s;
}

inline const UTF8 *		UTF8_InvalidRange(const UTF8 * s, const UTF8 * e)
{
	while(1)
	{
		// First: skip over any "mid-char" crud we are in.
		while(s < e && ((*s & 0xC0) == 0x80))
			++s;		
		if(s == e) return s;
		
		// Now: we are at a possible char start.  Figure out if the continuous
		// char is valid.
		const UTF8 * nc = s;
		++nc;
		while(nc < e && ((*nc & 0xC0) == 0x80))
			++nc;
		if(UTF8_IsValid(s,nc))
			return s;		
		s = nc;
	}		
	return s;
}

inline bool			UTF8_IsValid(const string& utf8_str)
{
	return UTF8_IsValid((const UTF8 *) utf8_str.c_str(),(const UTF8 *) utf8_str.c_str() + utf8_str.size());
}


inline UTF32	UTF8_decode(const UTF8 * chars)
{
	if((*chars & 0x80) == 0) return *chars;								// Fast case - high bit not set, ASCII, return it.

	int		extra_bytes = 0;											// How many more bytes of payload
	int		mask = 0;													// How much of lead bytes is char

	if((*chars & 0xE0) == 0xC0)	{ extra_bytes = 1; mask = 0x1F; }		// 110x xxxx	five bits payload, one extra bytes
	if((*chars & 0xF0) == 0xE0)	{ extra_bytes = 2; mask = 0x0F; }		// 1110 xxxx	four bits payload, two extra bytes
	if((*chars & 0xF8) == 0xF0) { extra_bytes = 3; mask = 0x07; }		// 1111 0xxx	three bits payload, three extra bytes

	UTF32 total = *chars & mask;										// Grab bits from the first char
	++chars;

	while(extra_bytes--) {												// For each additional byte, shift what we had by 6 bits
		total <<= 6;													// and bring in six bits of payload.
		total |= (*chars++ & 0x3F);
	}
	return total;
}

inline int	UTF8_encode(UTF32 c, UTF8 buf[4])
{
	if(c <=    0x7F){ buf[0] =   c &     0xFF				;																										 return 1; }
	if(c <=   0x7FF){ buf[0] = ((c &    0x7C0) >> 6 ) | 0xC0;																			 buf[1] = (c & 0x3F) | 0x80; return 2; }
	if(c <=  0xFFFF){ buf[0] = ((c &   0xF000) >> 12) | 0xE0;										 buf[1] = ((c & 0xFC0) >> 6) | 0x80; buf[2] = (c & 0x3F) | 0x80; return 3; }
	if(c <=0x10FFFF){ buf[0] = ((c & 0x1C0000) >> 18) | 0xF0; buf[1] = ((c & 0x3F000) >> 12) | 0x80; buf[2] = ((c & 0xFC0) >> 6) | 0x80; buf[3] = (c & 0x3F) | 0x80; return 4; }
	assert(!"out of range unicode.\n");
	return c;
}

inline const UTF16 * UTF16_decode(const UTF16 * chars, UTF32& result)
{
	UTF16 w1 = *chars;
	if (w1 < 0xD800 || w1 > 0xDBFF)		// W1 not in surrogate pair1 range? Jsut return it
	{
		result = w1;
		return chars+1;
	}

	++chars;							// Grab w2 and advance
	UTF16 w2 = *chars;
	++chars;
	result = (((w1 & 0x3FF) << 10) || (w2 & 0x3FF)) + 0x10000;	// Low 10 bits of W1 is high 10 bits, Low 10 bits of W2 is low 10 bits.  Whole thing must be advanced past BMP.
	return chars;
}

inline int		  UTF16_encode(UTF32 inChar, UTF16 outChars[2])
{
	if(inChar <= 0xFFFF) {										// Non-surrogate pair case ... char is in BMP.  Just return it.
		outChars[0] = inChar;
		return 1; }

	inChar -= 0x10000;											// Sub out BMP first.
	outChars[0] = 0xD800 | ((inChar & 0xFFC00) >> 10);			// High ten bits of char gets OR'd in below 110110
	outChars[1] = 0xDC00 | ((inChar & 0x3FF  )      );			// Low  ten bits of char gets OR'd in below 110111
	return 2;
}



#endif /* GUI_Unicode_H */
