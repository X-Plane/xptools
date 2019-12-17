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

#if APL // this is rather obsolete. Its all intel now ....
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

/***********************************************************************************************************************************************
 * GLOBAL FEATURE CONTROL
 ***********************************************************************************************************************************************/

/*
	This is sort of a hack: we can turn off and on some global flags here to try experimental features that we might otherwise not want.
*/

// I am beta testing CGAL's polygon simplifier (which is a freaking AWESOME package btw) but can't check the code in until they release it.  So this
// #ifs out code that depends on the module.
#define	CGAL_BETA_SIMPLIFIER 0

// Road-grid editor - NOT even remotely done yet, leave this off, dude.
#define ROAD_EDITING 0

// mroe : -- really early stage of dev , do not change.
#define WITHNWLINK 0

// These turn on the features to import the global apt databaes for the purpose of building a final scenery pack
// from the gateway.  You don't need this.
#define GATEWAY_IMPORT_FEATURES 0

// After running ATC Runway Validation, show the hitboxes used for hot zone tests
// 0 = never, 1 = only those causing a violation, 2 = always show all
#define DEBUG_VIS_LINES 1

// Set this to 1 to replace vector with a version that checks bounds.  Usually only used to catch fugly bugs.
#define SAFE_VECTORS 0

// This enables gateway communication.  You can turn this off if you don't have a working CURL/SSL.
#define HAS_GATEWAY 1

// This enables curved ATC taxiways - feature is NOT done yet or offical so, like, don't use it.
#define HAS_CURVED_ATC_ROUTE 0

// Load DDS textures directly into GPU w/o de- & re-compressing
#define LOAD_DDS_DIRECT 1

// This enables direct import of 7z compressed dsf's.
#define USE_7Z 1

// This is a big hack that is no more used much ... 
// WED entities are culled based on a bounding rect - and objects now know their worst case bounding box, based on their visualization, too.
// Just in case this boundary determination fails - this is the size in dergree's thats is used as their bounding box.

#define GLOBAL_WED_ART_ASSET_FUDGE_FACTOR 0.1

// Causes DSFLib to output stats about encoding quality.
#define DSF_WRITE_STATS 0

// Set this to 1 to crank up the mesh to ludicrous speed...
#define HD_MESH 0
#define UHD_MESH 1

#if WANT_NED_MALLOC
	#include "MemUtils.h"
#endif

/*******************************************************************************************************************************************
 * STL AND OTHER GLOBAL INCLUDES THAT WE LIKE ENOUGH TO HAVE EVERYWHERE
 *******************************************************************************************************************************************/

#if defined(_MSC_VER)
	#define _CRT_SECURE_NO_WARNINGS
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#ifdef __cplusplus
		#define _USE_MATH_DEFINES
	#endif
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
	#define __func__ __FUNCTION__
	#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
 
#ifdef __cplusplus
	#include <vector>
	#include <string>
	#include <map>
	#include <set>
	#include <algorithm>
	#include <iterator>
	
	#include <unordered_map>
	#define hash_map      unordered_map
	#define hash_multimap unordered_multimap
	#define HASH_MAP_NAMESPACE_START namespace std {
	#define HASH_MAP_NAMESPACE_END }
	#define HASH_PARENT(x,y)
	#define _MSL_THROW throw()

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

#if IBM // OS specific file handling hacks
	#define WINDOWS_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <windows.h>

	#if (WED || OBJVIEW)
	#define SUPPORT_UNICODE 1
	#endif

	#if SUPPORT_UNICODE
		// This is to convert filenames from UTF-8 to UTF16/wchar_t under windows, see in FileUtils.cpp
		#define fopen x_fopen
		#ifdef __cplusplus
			extern "C" FILE* x_fopen(const char * _Filename, const char * _Mode);
		#else
			extern FILE* x_fopen(const char * _Filename, const char * _Mode);
		#endif

		#if __cplusplus
			// This class is a replacement for ofstreams. X-Plane uses ofstreams in various places
			// but it always initializes them with std::strings which have UTF-8 characters jammed
			// into them. On windows however, we need UTF-16 characters in wide format for our file
			// paths. We want the conversion to be automatic and in one place so this was easier than
			// modifying the client code in numerous places.
			#include <fstream>
			#define ofstream x_ofstream
			class x_ofstream : public basic_ostream<char, char_traits<char>> 
			{
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
#elif LIN
	// This is to put an case-insensitive fopen in place, see in FileUtils.cpp
	#ifdef __cplusplus
		extern "C" FILE* x_fopen(const char * _Filename, const char * _Mode);
	#else
		extern FILE* x_fopen(const char * _Filename, const char * _Mode);
	#endif
	#define fopen(_Filename,_Mode) x_fopen(_Filename, _Mode)
#elif APL
	//no fopen magic of any kind needed
	#define __ASSERTMACROS__
#endif // OS specific file handling hacks

/****************************************************************************************************************************************************************
 * CGAL ADAPTER MACROS.
 ****************************************************************************************************************************************************************/

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
