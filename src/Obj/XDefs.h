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

// ATC features from WED 1.2.  This used to have ae big "DO NOT USE THIS" warning but we, like, shipped it, so this should be on.
#define AIRPORT_ROUTING 1

// Road-grid editor - NOT even remotely done yet, leave this off, dude.
#define ROAD_EDITING 0

// mroe : -- really early stage of dev , do not change.
#define WITHNWLINK 0

// Terraserver is always borked - I am killing it for now - will add back if it ever becomes useful.
// Really we need a better tile service.
#define WANT_TERRASEVER 0

// These turn on the features to import the global apt databaes for the purpose of building a final scenery pack
// from the gateway.  You don't need this.
#define GATEWAY_IMPORT_FEATURES 0

// After running ATC Runway Validation, show the hitboxes used for hot zone tests
// 0 = never, 1 = only those causing a violation, 2 = always show all
#define DEBUG_VIS_LINES 1

// Set this to 1 to replace vector with a version that checks bounds.  Usually only used to catch fugly bugs.
#define SAFE_VECTORS 0

// This enables jpeg 2k support for image import.  Can be turned off (for now) if you don't have the libs.
// (As of WED 1.4b1 this is off due to stability problems.
#define USE_GEOJPEG2K 0

// This enables gateway communication.  You can turn this off if you don't have a working CURL/SSL.
#define HAS_GATEWAY 1

// This enablse curved ATC taxiways - feature is NOT done yet or offical so, like, don't use it.
#define HAS_CURVED_ATC_ROUTE 0

// This is a big hack.  WED objects have culling "built-in" based on a bounding rect - it's part of the IGIS interface.
// But this is kind of a design flaw; the actual culling depends on the -visualization-, which is applied via a map layer.
// Only the map visualization knows how big things are.
//
// The result of this deisgn flaw is that objects disappear when their origin point goes off-screen - they are culled as a point.
//
// To get aronud this, we simply declare a slop factor when culling (1) objects/AGPs and (2) groups/airports/composites (which can contain
// them).  All of these cull as "on screen" if they are near the edge but fully off screen.
//
// This factor is in degrees lat/lon, so it's somewhere between 5 and 10 km of slop.  If your object is so big that this isn't enough
// (E.g. your object is more than 10 km from its origin) then YOU ARE DOING IT WRONG.  Break up your object or use another art asset;
// X-Plane actually contains slight math errors in OBJ placement on a round world and will not handle this well.
#define GLOBAL_WED_ART_ASSET_FUDGE_FACTOR 0.1

// Causes DSFLib to output stats about encoding quality.
#define DSF_WRITE_STATS 0

// Set this to 1 to crank up the mesh to ludicrous speed...
#define HD_MESH 0
#define UHD_MESH 1

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
#include <iterator>

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

#if defined(_MSC_VER)

	#ifdef __cplusplus
	#define _USE_MATH_DEFINES
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
	#define snprintf _snprintf

#if __cplusplus
	static __inline double round(double v) { return floor(v+0.5); }
#endif

#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)

#endif

#if IBM
#define WINDOWS_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

#if (WED || OBJVIEW)
#define SUPPORT_UNICODE 1
#endif

#if __cplusplus && SUPPORT_UNICODE
#include <fstream>

#define fopen x_fopen
#define ofstream x_ofstream
extern FILE* x_fopen(const char * _Filename, const char * _Mode);
//extern FILE* x_fopen(const WCHAR* _Filename, const wchar_t * _Mode);

// This class is a replacement for ofstreams. X-Plane uses ofstreams in various places
// but it always initializes them with std::strings which have UTF-8 characters jammed
// into them. On windows however, we need UTF-16 characters in wide format for our file
// paths. We want the conversion to be automatic and in one place so this was easier than
// modifying the client code in numerous places.
class x_ofstream : public basic_ostream<char, char_traits<char>> {
public:
	typedef basic_ofstream<char, char_traits<char>> _Myt;
	typedef basic_ostream<char, char_traits<char>> _Mybase;
	typedef basic_filebuf<char, char_traits<char>> _Myfb;
	typedef basic_ios<char, char_traits<char>> _Myios;

	x_ofstream() :_Mybase(&_Filebuffer) {}
	virtual ~x_ofstream() {}

	// Implementation is in FILE_ops.cpp
	void open(const char *_Filename, ios_base::openmode _Mode = ios_base::out, int _Prot = (int)ios_base::_Openprot);
	void open(const wchar_t *_Filename, ios_base::openmode _Mode = ios_base::out, int _Prot = (int)ios_base::_Openprot);
	void close();

private:
	_Myfb _Filebuffer;
};


#endif
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
