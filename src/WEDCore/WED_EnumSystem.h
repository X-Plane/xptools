#ifndef WED_ENUMSYSTEM_H
#define WED_ENUMSYSTEM_H

#include <vector>
#include <set>
#include <map>

using std::set;
using std::vector;
using std::map;

/*
	WED_EnumSystem - THEORY OF OPERATION
	
	Notes on enum system:
	
	All enums are non-negative ints, sequentially numbered at zero, starting with built-ins.
	
	All enums belong to domains, which define a subset of enums for pseudo-typing.
	
	All enums have string equivalents for export, same as C++ symbol if it exists.
	
	Enums may have an "export value", which is a non-shifting permanent int, used for file format I/O

*/

bool				DOMAIN_Validate(int domain);
const char *		DOMAIN_Fetch(int domain);
int					DOMAIN_Lookup(const char * domain);
int					DOMAIN_Create(const char * domain, const char * desc);
const char *		DOMAIN_Desc(int domain);

void				DOMAIN_Members(int domain, vector<int>& members);
void				DOMAIN_Members(int domain, set<int>& members);
void				DOMAIN_Members(int domain, map<int, string>& members);

bool				ENUM_Validate(int value);
const char *		ENUM_Fetch(int value);													// Return the string name of an enum
int					ENUM_Export(int value);													// Return the export value for an enum
int					ENUM_Lookup(const char * value);										// Find an enum or -1 if missing
int					ENUM_Domain(int value);													// What domain are we in?
const char *		ENUM_Desc(int value);
int					ENUM_Create(int domain, const char * value, const char * descrip, int export_value);			// Find an enum, add if needed
int					ENUM_Import(int domain, int export_value);

void				ENUM_Init(void);


#define ENUM_DOMAIN(D,H)	D,
#define ENUM(V,H,E)	V,

enum {

	#include "WED_Enums.h"

	last_enum

};

#undef ENUM_DOMAIN
#undef ENUM




#endif /* WED_ENUMSYSTEM_H */
