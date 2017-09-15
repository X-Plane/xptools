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

#ifndef WED_ENUMSYSTEM_H
#define WED_ENUMSYSTEM_H

#include <vector>
#include <set>
#include <map>

using std::set;
using std::vector;
using std::map;

struct	sqlite3;
/*
	WED_EnumSystem - THEORY OF OPERATION

	Notes on enum system:

	All enums are non-negative ints, sequentially numbered at zero, starting with built-ins.

	All enums belong to domains, which define a subset of enums for pseudo-typing.

	All enums have string equivalents for export, same as C++ symbol if it exists.

	Enums may have an "export value", which is a non-shifting permanent int, used for file format I/O

*/

bool				DOMAIN_Validate(int domain);
const char *		DOMAIN_Name(int domain);
//int					DOMAIN_LookupName(const char * domain);
int					DOMAIN_LookupDesc(const char * domain);
int					DOMAIN_Create(const char * domain, const char * desc);
const char *		DOMAIN_Desc(int domain);

void				DOMAIN_Members(int domain, vector<int>& members);
void				DOMAIN_Members(int domain, set<int>& members);
void				DOMAIN_Members(int domain, map<int, string>& members);

bool				ENUM_Validate(int value);
const char *		ENUM_Name(int value);								// Return the string name of an enum
int					ENUM_Export(int value);								// Return the export value for an enum
int					ENUM_ExportSet(const set<int>& members);
//int					ENUM_LookupName(const char * value);			// Find an enum or -1 if missing
int					ENUM_LookupDesc(int domain, const char * value);	// Find an enum or -1 if missing
int					ENUM_Domain(int value);								// What domain are we in?
const char *		ENUM_Desc(int value);
int					ENUM_Create(int domain, const char * value, const char * descrip, int export_value);			// Find an enum, add if needed
int					ENUM_Import(int domain, int export_value);
void				ENUM_ImportSet(int domain, int export_value, set<int>& members);

void				ENUM_Init(void);

typedef	map<int,int>	enum_map_t;

#define ENUM_DOMAIN(D,H)	D,
#define ENUM(V,H,E)	V,

enum {

	#include "WED_Enums.h"

	last_enum

};

#undef ENUM_DOMAIN
#undef ENUM




#endif /* WED_ENUMSYSTEM_H */
