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

/************************************************************************************************************************************************************************
 * PLATFORM AND COMPILER CONTROL
 ************************************************************************************************************************************************************************/


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

/************************************************************************************************************************************************************************
 * GLOBAL FEATURE CONTROL
 ************************************************************************************************************************************************************************/

/*
	This is sort of a hack: we can turn off and on some global flags here to try experimental features that we might otherwise not want.
*/

// I am beta testing CGAL's polygon simplifier (which is a freaking AWESOME package btw) but can't check the code in until they release it.  So this
// #ifs out code that depends on the module.
#define	CGAL_BETA_SIMPLIFIER 0

// This define controls the inclusion of experimental next-gen features for ATC.  I have added them as I need to generate experimental data.
// READ MY LIPS: DO NOT SET THIS TO 1.
// The file formats for ATC are not even REMOTELY close to being finished...if you set this to 1 and compile WED, you will create a WED that will make:
// - Bogus output apt.dat files.
// - Bogus earth.wed files that won't work with either future WEDs or the current WED.

// So...you will end up wasting a lot of time and lose all your data.  DO NOT SET THIS TO 1.  CONSIDER YOURSELF WARNED!
#define AIRPORT_ROUTING 1

// Road-grid editor - NOT even remotely done yet, leave this off, dude.
#define ROAD_EDITING 0


// mroe : -- really early stage of dev , do not change.
#define WITHNWLINK 0

// Terraserver is always borked - I am killing it for now - will add back if it ever becomes useful.
// Really we need a better tile service.
#define WANT_TERRASEVER 0

// These turn on the features to import the global apt databaes for the purpose of building a final scenery pack
// from the gateway
#define GATEWAY_IMPORT_FEATURES 0

// Set this to 1 to replace vector with a version that checks bounds.  Usually only used to catch fugly bugs.
#define SAFE_VECTORS 0

// This enables jpeg 2k support for image import.
#define USE_GEOJPEG2K 1

#define XUTILS_EXCLUDE_MAC_CRAP 1

#include "MemUtils.h"

/************************************************************************************************************************************************************************
 * STL AND OTHER GLOBAL INCLUDES THAT WE LIKE ENOUGH TO HAVE EVERYWHERE
 ************************************************************************************************************************************************************************/

#ifdef __cplusplus

#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>

#if SAFE_VECTORS && DEV

	// This goo hacks vector to bounds check ALL array accesses...not fast, but a nice way to catch stupid out of bounds conditions.
	namespace std
	{
		template <class T, class Allocator = allocator<T> >
		class __dev_vector : public vector<T, Allocator>
		{
			public:
				typedef vector<T,Allocator>					base_type;
				typedef typename base_type::size_type		size_type;
				typedef typename base_type::reference		reference;
				typedef typename base_type::const_reference	const_reference;

				explicit __dev_vector(									const Allocator& a = Allocator()) : base_type(		  a	){}
				explicit __dev_vector(size_type n, const T& value = T(),const Allocator& a = Allocator()) : base_type(n,value,a	){}

				template <class InputIterator>
					__dev_vector(InputIterator first, InputIterator last,const Allocator& a = Allocator()) : base_type(first,last, a){}
					__dev_vector(const __dev_vector& x													 ) : base_type(x			){}

				inline 	     reference operator[](size_type n)		 {assert(n>=0 && n<base_type::size()); return base_type::operator[](n);}
				inline const_reference operator[](size_type n) const {assert(n>=0 && n<base_type::size()); return base_type::operator[](n);}
		};
	}
	#define vector __dev_vector

#endif

using namespace std;

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#include <stddef.h>
#include <stdint.h>

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

	#if APL || LIN || MINGW_BUILD
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
#define WINDOWS_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#endif

#if APL
#define __ASSERTMACROS__
#endif

/************************************************************************************************************************************************************************
 * CGAL ADAPTER MACROS.
 ************************************************************************************************************************************************************************/

/*
	The original theory was that we would use macros to let the airport code run in either double-precision or CGAL-precision math.  But in practice this is probably
	not going to happen.  A few months ago as I was moving to CGAL 3.3 I thought this could be handy.  But the structure that is emerging now is a conversion from
	IEEE to CGAL at the point where we need to do "precise" operations (like polygon intersections).  The storage of airports (and such) in IEEE usually is a flag
	to the programmer that the data has not been validated.

	That's all a long way of saying: someday these will go away when I get around to it.  /ben
*/

#define CGAL2DOUBLE(x)		(x)
#define POINT2				Point2
#define SEGMENT2			Segment2
#define VECTOR2				Vector2
#define CGAL_midpoint(a,b)	Segment2(a,b).midpoint()


#endif
