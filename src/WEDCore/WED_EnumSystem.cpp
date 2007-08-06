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

#include "WED_EnumSystem.h"
#include "AssertUtils.h"
#include "AptDefs.h"
#include "WED_Errors.h"
#include "SQLUtils.h"
#include <string>
using std::string;

#define ENUM_DOMAIN(D,H)		last_d = DOMAIN_Create(#D,H);
#define ENUM(V,H,E)		ENUM_Create(last_d, #V,H,E);

struct enum_Info {
	int		domain;
	string	name;
	string	desc;
	int		export_value;
};

struct domain_Info {
	int	enum_begin;		// First enum
	int enum_end;		// Last enum + 1
};

static vector<enum_Info>				sEnums;				// For each enum N, string, 
static map<string, int>					sEnumsReverse;
static map<int,domain_Info>				sDomains;

bool				DOMAIN_Validate(int domain)
{
	return (domain >= 0 && domain < sEnums.size() && sEnums[domain].domain == -1);
}

const char *		DOMAIN_Fetch(int domain)
{
	if (!DOMAIN_Validate(domain)) return NULL;
	return sEnums[domain].name.c_str();
}

const char *		DOMAIN_Desc(int domain)
{
	if (!DOMAIN_Validate(domain)) return NULL;
	return sEnums[domain].desc.c_str();
}

int					DOMAIN_Lookup(const char * domain)
{
	string d(domain);
	map<string, int>::iterator i = sEnumsReverse.find(d);
	if (i == sEnumsReverse.end())	return -1;
	if (DOMAIN_Validate(i->second)) return i->second;
									return -1;
}


int					DOMAIN_Create(const char * domain, const char * desc)
{
	string d(domain);
	map<string,int>::iterator i = sEnumsReverse.find(d);
	if (i != sEnumsReverse.end())
	{
		if (!DOMAIN_Validate(i->second))
			AssertPrintf("Error: domain %s is actually an enum.", domain);
			
		return i->second;
	}
	
	enum_Info e;
	e.domain = -1;
	e.name = d;
	e.desc = desc;
	e.export_value = -1;

	int idx = sEnums.size();
//	printf("Creating domain %s as %d\n", domain, idx);
	
	sEnums.push_back(e);
	sEnumsReverse[d] = idx;
	domain_Info di = { -1, -1 };
	sDomains[idx] = di;
	return idx;
}

void				DOMAIN_Members(int domain, vector<int>& members)
{
	members.clear();
	if (!DOMAIN_Validate(domain)) return;

	domain_Info * d = &sDomains[domain];
	
	for (int e = d->enum_begin; e != d->enum_end; ++e)
	if (sEnums[e].domain == domain)
		members.push_back(e);
}

void				DOMAIN_Members(int domain, set<int>& members)
{
	members.clear();
	if (!DOMAIN_Validate(domain)) return;

	domain_Info * d = &sDomains[domain];
	
	for (int e = d->enum_begin; e != d->enum_end; ++e)
	if (sEnums[e].domain == domain)
		members.insert(e);
}

void				DOMAIN_Members(int domain, map<int, string>& members)
{
	members.clear();
	if (!DOMAIN_Validate(domain)) return;

	domain_Info * d = &sDomains[domain];
	
	for (int e = d->enum_begin; e != d->enum_end; ++e)
	if (sEnums[e].domain == domain)
		members.insert(map<int,string>::value_type(e,sEnums[e].desc));
}


int					ENUM_Create(int domain, const char * value, const char * desc, int export_value)
{
	if (!DOMAIN_Validate(domain))
		AssertPrintf("Error: illegal domain %d registering %s\n",domain, value);
	string v(value);
	map<string,int>::iterator i = sEnumsReverse.find(v);
	int idx = sEnums.size();
	if (i != sEnumsReverse.end())
	{
		idx = i->second;
		Assert(sEnums[idx].domain == domain);
		Assert(sEnums[idx].export_value == export_value);
		return idx;
	}
	
	if (sDomains[domain].enum_begin == -1)
	{
		sDomains[domain].enum_begin = idx;
		sDomains[domain].enum_end = idx+1;
	} else {
		sDomains[domain].enum_begin = min(sDomains[domain].enum_begin,idx);
		sDomains[domain].enum_end   = max(sDomains[domain].enum_end, idx+1);
	}
	
//	printf("Creating new enum %s value=%d in domain %d (%s)\n", value, idx, domain, sEnums[domain].name.c_str());
	
	enum_Info e;
	e.domain = domain;
	e.name = v;
	e.desc = desc;
	e.export_value = export_value;
	
	sEnums.push_back(e);
	sEnumsReverse[v] = idx;
	
	return idx;	
}


bool				ENUM_Validate(int value)
{
	return (value >= 0 && value < sEnums.size() && sEnums[value].domain != -1);
}

const char *		ENUM_Fetch(int value)
{
	if (!ENUM_Validate(value)) return NULL;
	return sEnums[value].name.c_str();
}

int					ENUM_Export(int value)
{
	if (!ENUM_Validate(value)) return -1;
	return sEnums[value].export_value;
}

int					ENUM_Lookup(const char * value)
{
	string v(value);
	map<string, int>::iterator i = sEnumsReverse.find(v);
	if (i == sEnumsReverse.end())	return -1;
	if (ENUM_Validate(i->second))	return i->second;
									return -1;
}

int					ENUM_Domain(int value)
{
	if (!ENUM_Validate(value)) return -1;
	return sEnums[value].domain;
}

const char * ENUM_Desc(int value)
{
	if (!ENUM_Validate(value)) return NULL;
	return sEnums[value].desc.c_str();
}

int					ENUM_Import(int domain, int export_value)
{
	if (!DOMAIN_Validate(domain)) return -1;
	
	domain_Info * d = &sDomains[domain];
	
	for (int e = d->enum_begin; e != d->enum_end; ++e)
	if (sEnums[e].domain == domain)
	if (sEnums[e].export_value == export_value)
		return e;
	
	return -1;
}



void	ENUM_Init(void)
{
	int last_d = -1;
	#include "WED_Enums.h"
}

#undef ENUM_DOMAIN
#undef ENUM

void		ENUM_write(sqlite3 * db)
{
	int err;
	{
		sql_command	clear_table(db,"DELETE FROM WED_enum_system WHERE 1;",NULL);
		err = clear_table.simple_exec();
		if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}
	
	{
		sql_command add_item(db,"INSERT INTO WED_enum_system VALUES(@i,@n,@s,@d,@e);","@i,@n,@s,@d,@e");
		for(int i = 0; i < sEnums.size(); ++i)
		{
			sql_row5<int,string,string,int,int>	r;
		
			r.a = i;
			r.b = sEnums[i].name;
			r.c = sEnums[i].desc;
			r.d = sEnums[i].domain;
			r.e = sEnums[i].export_value;
//			printf("Writing: %d,%s,%d ",r.a,r.b.c_str(),r.c);
			err = add_item.simple_exec(r);
//			printf(" result = %d\n", err);
			if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
		}
	}	
}

void		ENUM_read (sqlite3 * db, enum_map_t& out_map)
{	
	int err;
	out_map.clear();
	
	{
		sql_command	sel_domains(db,"SELECT value,name,desc FROM WED_enum_system where domain = -1;",NULL);

		sql_row3<int,string, string>	drec;
		sel_domains.begin();

		while((err = sel_domains.get_row(drec)) == SQLITE_ROW)
		{
			int real_d = DOMAIN_Create(drec.b.c_str(),drec.c.c_str());
			out_map[drec.a] = real_d;

		}
		if (err != SQLITE_DONE)	
			WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}
	
	{
		sql_command	sel_vals(db,"SELECT value,name,desc,domain,export FROM WED_enum_system where domain != -1;",NULL);
		sql_row5<int,string, string, int, int>	erec;
		sel_vals.begin();

		while((err = sel_vals.get_row(erec)) == SQLITE_ROW)
		{
			DebugAssert(out_map.count(erec.d) > 0);
			int real_e = ENUM_Create(out_map[erec.d], erec.b.c_str(), erec.c.c_str(), erec.e);
			out_map[erec.a] = real_e;

		}
		if (err != SQLITE_DONE)	
			WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}
}

