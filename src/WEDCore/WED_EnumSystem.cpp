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
static map<pair<int,string>, int>		sEnumsReverse;
static map<int,domain_Info>				sDomains;

bool				DOMAIN_Validate(int domain)
{
	return (domain >= 0 && domain < sEnums.size() && sEnums[domain].domain == -1);
}

const char *		DOMAIN_Name(int domain)
{
	if (!DOMAIN_Validate(domain)) return NULL;
	return sEnums[domain].name.c_str();
}

const char *		DOMAIN_Desc(int domain)
{
	if (!DOMAIN_Validate(domain)) return NULL;
	return sEnums[domain].desc.c_str();
}

//int					DOMAIN_LookupName(const char * domain)
//{
//	string d(domain);
//	map<string, int>::iterator i = sEnumsReverse.find(d);
//	if (i == sEnumsReverse.end())	return -1;
//	if (DOMAIN_Validate(i->second)) return i->second;
//									return -1;
//}

int					DOMAIN_LookupDesc(const char * domain)
{
	string d(domain);
	map<pair<int, string>, int>::iterator i = sEnumsReverse.find(pair<int,string>(-1,d));
	if (i == sEnumsReverse.end())	return -1;
	if (DOMAIN_Validate(i->second)) return i->second;
									return -1;
}


int					DOMAIN_Create(const char * domain, const char * desc)
{
	string d(domain);
	map<pair<int,string>,int>::iterator i = sEnumsReverse.find(pair<int,string>(-1,d));
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
	DebugAssert(sEnumsReverse.count(pair<int,string>(-1,d))==0);
	sEnumsReverse[pair<int,string>(-1,d)] = idx;
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
	map<pair<int,string>,int>::iterator i = sEnumsReverse.find(pair<int,string>(domain,desc));
	int idx = sEnums.size();
	
	if (i != sEnumsReverse.end())
	{
		idx = i->second;
		Assert(sEnums[idx].domain == domain);
//		Ben says: EXPORT values do NOT have to match - go by CURRENT export values - may be due to a bug fix!
//		Assert(sEnums[idx].export_value == export_value);
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
	DebugAssert(sEnumsReverse.count(pair<int,string>(domain,desc))==0);	
	sEnumsReverse[pair<int,string>(domain,desc)] = idx;

	return idx;
}


bool				ENUM_Validate(int value)
{
	return (value >= 0 && value < sEnums.size() && sEnums[value].domain != -1);
}

const char *		ENUM_Name(int value)
{
	if (!ENUM_Validate(value)) return NULL;
	return sEnums[value].name.c_str();
}

int					ENUM_Export(int value)
{
	if (!ENUM_Validate(value)) return -1;
	return sEnums[value].export_value;
}

int					ENUM_ExportSet(const set<int>& members)
{
	int r = 0;
	for(set<int>::const_iterator m = members.begin(); m != members.end(); ++m)
	{
		if(!ENUM_Validate(*m)) return -1;
		r |= sEnums[*m].export_value;
	}		
	return r;
}

//int					ENUM_LookupName(const char * value)
//{
//	string v(value);
//	map<string, int>::iterator i = sEnumsReverse.find(v);
//	if (i == sEnumsReverse.end())	return -1;
//	if (ENUM_Validate(i->second))	return i->second;
//									return -1;
//}

int					ENUM_LookupDesc(int domain, const char * value)
{
	string v(value);
	map<pair<int,string>, int>::iterator i = sEnumsReverse.find(pair<int,string>(domain,v));
	if (i == sEnumsReverse.end())	
	{
#if DEV
		printf("Cannot find enum '%s' in domain %d\n",value, domain);
#endif
		return -1;
	}
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

void				ENUM_ImportSet(int domain, int export_value, set<int>& vals)
{
	vals.clear();
	if (!DOMAIN_Validate(domain)) return;

	domain_Info * d = &sDomains[domain];

	for (int e = d->enum_begin; e != d->enum_end; ++e)
	if (sEnums[e].domain == domain)
	if (sEnums[e].export_value & export_value)
		vals.insert(e);
}

void	ENUM_Init(void)
{
	int last_d = -1;
	#include "WED_Enums.h"
}
