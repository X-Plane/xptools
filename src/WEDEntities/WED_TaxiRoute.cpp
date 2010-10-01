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

#include "WED_TaxiRoute.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_ToolUtils.h"
#include "STLUtils.h"

#if AIRPORT_ROUTING

DEFINE_PERSISTENT(WED_TaxiRoute)
TRIVIAL_COPY(WED_TaxiRoute, WED_GISEdge)

WED_TaxiRoute::WED_TaxiRoute(WED_Archive * a, int i) : WED_GISEdge(a,i),
	oneway(this,"One-Way","WED_taxiroute","oneway", 1),
	runway(this,"Runway", "WED_taxiroute","runway", ATCRunwayTwoway, atc_rwy_None),
	hot_depart(this,"Departures","WED_taxiroute_depart","departures", ATCRunwayOneway,false),
	hot_arrive(this,"Arrivals","WED_taxiroute_arrive","arrivals", ATCRunwayOneway,false),
	hot_ils(this,"ILS Precision Area","WED_taxiroute_ils","ils", ATCRunwayOneway,false)
{
}

WED_TaxiRoute::~WED_TaxiRoute()
{
}

void		WED_TaxiRoute::SetOneway(int d)
{
	oneway = d;
}

void		WED_TaxiRoute::SetRunway(int r)
{
	runway = r;
}

void		WED_TaxiRoute::SetHotDepart(const set<int>& rwys)
{
	hot_depart = rwys;
}

void		WED_TaxiRoute::SetHotArrive(const set<int>& rwys)
{
	hot_arrive = rwys;
}

void		WED_TaxiRoute::SetHotILS(const set<int>& rwys)
{
	hot_ils = rwys;
}

void	WED_TaxiRoute::Import(const AptRouteEdge_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(info.name);
	oneway = info.oneway;
	if(info.runway)
	{
		int r = ENUM_LookupDesc(ATCRunwayTwoway, info.name.c_str());
		if (r == -1)
		{
			print_func(ref,"Runway name %s is illegal.\n", info.name.c_str());
			runway = atc_rwy_None;
		} else
			runway = r;
		
	}
	else
	{
		runway = atc_rwy_None;
	}

	for(set<string>::iterator h = info.hot_depart.begin(); h != info.hot_depart.end(); ++h)
	{
		int r = ENUM_LookupDesc(ATCRunwayOneway,h->c_str());
		if(r == -1)
			print_func(ref,"Runway name %s is illegal.\n", h->c_str());
		else
			hot_depart += r;
	}

	for(set<string>::iterator h = info.hot_arrive.begin(); h != info.hot_arrive.end(); ++h)
	{
		int r = ENUM_LookupDesc(ATCRunwayOneway,h->c_str());
		if(r == -1)
			print_func(ref,"Runway name %s is illegal.\n", h->c_str());
		else
			hot_arrive += r;
	}

	for(set<string>::iterator h = info.hot_ils.begin(); h != info.hot_ils.end(); ++h)
	{
		int r = ENUM_LookupDesc(ATCRunwayOneway,h->c_str());
		if(r == -1)
			print_func(ref,"Runway name %s is illegal.\n", h->c_str());
		else
			hot_ils += r;
	}

}

void	WED_TaxiRoute::Export(		 AptRouteEdge_t& info) const
{	
	if(runway.value == atc_rwy_None)
	{
		this->GetName(info.name);
		info.runway = 0;
	}
	else
	{
		info.runway = 1;
		info.name = ENUM_Desc(runway.value);
	}
	
	info.oneway = oneway.value;
	info.hot_depart.clear();
	info.hot_arrive.clear();
	info.hot_ils.clear();
	
	set<int>::iterator h;
	for (h = hot_depart.value.begin(); h != hot_depart.value.end(); ++h)
		info.hot_depart.insert(ENUM_Desc(*h));
	for (h = hot_arrive.value.begin(); h != hot_arrive.value.end(); ++h)
		info.hot_arrive.insert(ENUM_Desc(*h));
	for (h = hot_ils.value.begin(); h != hot_ils.value.end(); ++h)
		info.hot_ils.insert(ENUM_Desc(*h));


}

void	WED_TaxiRoute::GetNthPropertyDict(int n, PropertyDict_t& dict)
{
	dict.clear();
	if(n == PropertyItemNumber(&runway))
	{
		WED_Airport * airport = WED_GetParentAirport(this);
		if(airport)
		{
			PropertyDict_t full;
			WED_Thing::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysTwoway(airport, legal);
			legal.insert(runway.value);
			legal.insert(atc_rwy_None);
			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,f->second));
		}
	}
	else if (n == PropertyItemNumber(&hot_depart) ||
			 n == PropertyItemNumber(&hot_arrive) ||
			 n == PropertyItemNumber(&hot_ils))
	{
		WED_Airport * airport = WED_GetParentAirport(this);
		if(airport)
		{
			PropertyDict_t full;
			WED_Thing::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysOneway(airport, legal);
			PropertyVal_t val;
			this->GetNthProperty(n,val);
			DebugAssert(val.prop_kind == prop_EnumSet);
			copy(val.set_val.begin(),val.set_val.end(),set_inserter(legal));
			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,f->second));
		}
	}
	else
		WED_Thing::GetNthPropertyDict(n,dict);			
}

bool	WED_TaxiRoute::IsOneway(void) const
{
	return oneway.value;
}


#endif
