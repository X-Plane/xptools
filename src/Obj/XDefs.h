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
#ifndef XDEFS_H
#define XDEFS_H

#if __MWERKS__
	#if __MACH__
		#pragma c99 on

		#define _MSL_USING_MW_C_HEADERS 1

		#define __dest_os __mac_os_x
	#elif APL
		#define __dest_os __mac_os
	#endif

	#define __MSL_LONGLONG_SUPPORT__
#endif

#if APL || LIN
	#define CRLF "\n"
#else
	#define CRLF "\r\n"
#endif

#if APL
	#if defined(__POWERPC__)
		#define BIG 1
		#define LIL 0
	#else
		#define BIG 0
		#define LIL 1
	#endif
#elif IBM || LIN
	#define BIG 0
	#define LIL 1
#else
	#error NO PLATFORM!
#endif

#ifdef __cplusplus

#include <vector>
#include <string>
#include <map>
#include <set>

using namespace std;

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUPPORT_STL

#ifdef __cplusplus

#if __MWERKS__							// metrowerks compiler
	#include <hash_map>
	using namespace std;				// DEC THIS TO GET THE NEW IOS FUNCTIONS IN fstream, iomanip, and string, which are all new, unlike the old fstream.h, iomanip.h, and string.h
	using Metrowerks::hash_map;			// Pull hash map into the global domain too!
	using Metrowerks::hash_multimap;
	#define HASH_MAP_NAMESPACE_START namespace Metrowerks {
	#define HASH_MAP_NAMESPACE_END }

	#define HASH_PARENT(x,y) : std::unary_function<x,y>
#endif
#if __GNUC__							// gnuc is the x-code compiler

	// Some code bases like CGAL think __powerpc__ is lower case - for some reason gcc has upper, so...
//	#if defined(__POWERPC__) && !defined(__powerpc__)
//		#define __powerpc__
//	#endif

	#if APL || LIN
// TODO: replace this hack with standard conform <unordered_map>, <hash_map> will disappear in the near future
	#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
	#if GCC_VERSION >= 40300
		#include <backward/hash_fun.h>
		#include <backward/hash_map>
	#else
		#include <ext/hash_map>
		#include <ext/hash_fun.h>
	#endif

		namespace __gnu_cxx {
			template<>
			struct hash<std::string>
			{
				size_t
				operator()(const std::string& __s) const
				{ return __stl_hash_string(__s.c_str()); }
			};

			template<>
			struct hash<float>
			{
				size_t
				operator()(const float& __s) const
				{ return (size_t) __s; }
			};

		}
	#else
		#include <hash_map>
	#endif
	using namespace	std;				// DEC THIS TO GET THE NEW IOS FUNCTIONS IN fstream, iomanip, and string, which are all new, unlike the old fstream.h, iomanip.h, and string.h
	using namespace __gnu_cxx;			// DEC THIS TO GET THE NEW IOS FUNCTIONS IN fstream, iomanip, and string, which are all new, unlike the old fstream.h, iomanip.h, and string.h
	using __gnu_cxx::hash_map;
	using __gnu_cxx::hash;
	#define HASH_MAP_NAMESPACE_START namespace __gnu_cxx {
	#define HASH_MAP_NAMESPACE_END }
	#define HASH_PARENT(x,y)
	#define _MSL_THROW throw()
#endif

#endif /* __cplusplus */

#if defined(_MSC_VER) && !defined(__MWERKS__)

	#ifdef __cplusplus

		#include <hash_map>
		using namespace stdext;	// Ben says - can't entirely blame MSVC for this - hash maps are NOT stardard - a weakness of the STL that causes much grief!
		using namespace std;

#define HASH_MAP_NAMESPACE_START namespace stdext {
#define HASH_MAP_NAMESPACE_END }
#define HASH_PARENT(x,y)
#define _MSL_THROW throw()

	#endif

	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp

	#define __func__ __FUNCTION__

	#define ENOERR 0
	#define round(X) floor(X + 0.5f)
	#define snprintf _snprintf

#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)

#endif



#if IBM
#include <winsock2.h>
#include <windows.h>
#endif


#define CGAL2DOUBLE(x)		(x)
#define POINT2				Point2
#define SEGMENT2			Segment2
#define VECTOR2				Vector2
#define CGAL_midpoint(a,b)	Segment2(a,b).midpoint()


#endif
