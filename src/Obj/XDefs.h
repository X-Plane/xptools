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


#include <vector>
#include <string>
#include <map>
#include <set>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#define SUPPORT_STL

#if __MWERKS__							// metrowerks compiler
	#include <hash_map>
	using namespace std;				// DEC THIS TO GET THE NEW IOS FUNCTIONS IN fstream, iomanip, and string, which are all new, unlike the old fstream.h, iomanip.h, and string.h
	using Metrowerks::hash_map;			// Pull hash map into the global domain too!
	using Metrowerks::hash_multimap;
	#define HASH_MAP_NAMESPACE Metrowerks
#endif
#if __GNUC__							// gnuc is the x-code compiler
	#if APL
		#include <ext/hash_map>
		#include <ext/hash_fun.h>

		namespace __gnu_cxx {
			template<>
			struct hash<std::string>
			{
				size_t
				operator()(const std::string& __s) const
				{ return __stl_hash_string(__s.c_str()); }
			};
		}
	#else
		#include <hash_map>
	#endif
	using namespace	std;				// DEC THIS TO GET THE NEW IOS FUNCTIONS IN fstream, iomanip, and string, which are all new, unlike the old fstream.h, iomanip.h, and string.h
	using namespace __gnu_cxx;			// DEC THIS TO GET THE NEW IOS FUNCTIONS IN fstream, iomanip, and string, which are all new, unlike the old fstream.h, iomanip.h, and string.h
	using __gnu_cxx::hash_map;
	using __gnu_cxx::hash;
	#define HASH_MAP_NAMESPACE __gnu_cxx
#endif


#endif
