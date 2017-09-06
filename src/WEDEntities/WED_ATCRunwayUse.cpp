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

#include "WED_ATCRunwayUse.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_ToolUtils.h"

#if AIRPORT_ROUTING


DEFINE_PERSISTENT(WED_ATCRunwayUse)
TRIVIAL_COPY(WED_ATCRunwayUse,WED_Thing)

WED_ATCRunwayUse::WED_ATCRunwayUse(WED_Archive * a, int i) :
	WED_Thing(a,i),
	rwy(this,"Runway",                                XML_Name("runway_use","rwy"),	      ATCRunwayOneway, atc_4L),
	dep_frq(this,"Departure Frequency",               XML_Name("runway_use","dep_frq"),   133.0, 6, 2),
	traffic(this,"Traffic Type",                      XML_Name("runway_use","traffic"),   ATCTrafficType, 0),
	operations(this,"Operations",                     XML_Name("runway_use","operations"),ATCOperationType, 0),
	dep_heading_min(this,"Legal On-Course hdg (min)", XML_Name("runway_use","dep_min"),	0, 3),
	dep_heading_max(this,"Legal On-Course hdg (max)", XML_Name("runway_use","dep_max"),	0, 3),
	vec_heading_min(this,"ATC Assigned hdg (min)",    XML_Name("runway_use","ini_min"),	0, 3),
	vec_heading_max(this,"ATC Assigned hdg (max)",    XML_Name("runway_use","ini_max"),	0, 3)
{
}

WED_ATCRunwayUse::~WED_ATCRunwayUse()
{
}

void	WED_ATCRunwayUse::SetRunway(int r)
{
	rwy.value = r;
}

int WED_ATCRunwayUse::GetRunway(void) const
{
	return rwy.value;
}

bool	WED_ATCRunwayUse::HasArrivals(void) const
{
	return operations.value.count(atc_Arrivals);
}

bool	WED_ATCRunwayUse::HasDepartures(void) const
{
	return operations.value.count(atc_Departures);
}

void	WED_ATCRunwayUse::Import(const AptRunwayRule_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(info.name);
	int rwy_int = ENUM_LookupDesc(ATCRunwayOneway, info.runway.c_str());
	if(rwy_int == -1)
	{
		print_func(ref,"Illegal runway %s\n",info.runway.c_str());
		rwy_int = atc_Runway_None;
	}
	rwy = rwy_int;
	ENUM_ImportSet(operations.domain,info.operations,operations.value);
	ENUM_ImportSet(traffic.domain,info.equipment,traffic.value);
	dep_frq.AssignFrom10Khz(info.dep_freq);
	dep_heading_min = info.dep_heading_lo;
	dep_heading_max = info.dep_heading_hi;
	vec_heading_min = info.ini_heading_lo;
	vec_heading_max = info.ini_heading_hi;
}

void	WED_ATCRunwayUse::Export(		 AptRunwayRule_t& info) const
{
	GetName(info.name);
	info.runway = ENUM_Desc(rwy.value);
	info.operations = ENUM_ExportSet(operations.value);
	info.equipment = ENUM_ExportSet(traffic.value);
	info.dep_freq = dep_frq.GetAs10Khz();
	info.dep_heading_lo = dep_heading_min;
	info.dep_heading_hi = dep_heading_max;
	info.ini_heading_lo = vec_heading_min;
	info.ini_heading_hi = vec_heading_max;
}

void	WED_ATCRunwayUse::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	dict.clear();
	if(n == PropertyItemNumber(&rwy))
	{
		const WED_Airport * airport = WED_GetParentAirport(this);
		if(airport)
		{
			PropertyDict_t full;
			WED_Thing::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysOneway(airport, legal);
			legal.insert(rwy.value);

			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,f->second));
		}
	}
	else
		WED_Thing::GetNthPropertyDict(n,dict);			
}


#endif

