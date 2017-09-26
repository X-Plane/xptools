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
#include "WED_TaxiRouteNode.h"

#if AIRPORT_ROUTING

static void get_runway_parts(int rwy, set<int>& rwy_parts)
{
	if(rwy == atc_rwy_None)
		return;

	int rwy_int = ENUM_Export(rwy);
	int rwy_dir = rwy_int/10;
	int rwy_lane =rwy_int%10;
	if(rwy_dir < 37)
	{
		rwy_parts.insert(ENUM_Import(ATCRunwayOneway,rwy_int));
		return;
	}
	
	rwy_dir -= 36;
	int rwy_lane_recip = rwy_lane;
	if(rwy_lane_recip) rwy_lane_recip = 4 - rwy_lane_recip;
	rwy_parts.insert(ENUM_Import(ATCRunwayOneway,rwy_dir*10+rwy_lane));
	rwy_parts.insert(ENUM_Import(ATCRunwayOneway,rwy_dir*10+180+rwy_lane_recip));
	

}

DEFINE_PERSISTENT(WED_TaxiRoute)

WED_TaxiRoute::WED_TaxiRoute(WED_Archive * a, int i) : WED_GISEdge(a,i),
	vehicle_class(this,	"Allowed Vehicles",XML_Name("taxi_route","vehicle_class"),ATCVehicleClass,atc_Vehicle_Aircraft),
	oneway(this,"One-Way",                 XML_Name("taxi_route","oneway"),   1),
	runway(this,"Runway",                  XML_Name("taxi_route","runway"),   ATCRunwayTwoway, atc_rwy_None),
	hot_depart(this,"Departures",          XML_Name("departures","runway"),   ATCRunwayOneway,false),
	hot_arrive(this,"Arrivals",            XML_Name("arrivals","runway"),     ATCRunwayOneway,false),
	hot_ils(this,"ILS Precision Area",     XML_Name("ils_holds","runway"),    ATCRunwayOneway,false),
	width(this,"Size",                     XML_Name("taxi_route","width"),    ATCIcaoWidth, width_E)
{
}

void WED_TaxiRoute::CopyFrom(const WED_TaxiRoute * rhs)
{
	WED_GISEdge::CopyFrom(rhs);
	// Why is this necessary?  All WED_Things attempt to copy their guts by usign the introspection
	// interface of IPropertyObject.
	//
	// But WED_TaxiRoute LIES about its interface to make "virtual" hot zones when it is a runway -
	// this keeps the UI matching x-plane.  But it results in a bug: when we copy the taxi route,
	// those virtual hot zones are copied (via the property interface) and become real in the new object,
	// which is bad.
	// So we run the base class, then manually copy this stuff. 
	hot_depart = rhs->hot_depart;
	hot_arrive = rhs->hot_arrive;
	hot_ils = rhs->hot_ils;
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

set<int> WED_TaxiRoute::GetHotDepart()
{
	return hot_depart;
}

set<int> WED_TaxiRoute::GetHotArrive()
{
	return hot_arrive;
}

set<int> WED_TaxiRoute::GetHotILS()
{
	return hot_ils;
}


void WED_TaxiRoute::SetWidth(int w)
{
	width= w;
}

void	WED_TaxiRoute::SetVehicleClass(int in_class)
{
	vehicle_class = in_class;
}

void	WED_TaxiRoute::Import(const AptRouteEdge_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(info.name);
	vehicle_class = atc_Vehicle_Aircraft;
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
		runway = atc_rwy_None;

	width = ENUM_Import(ATCIcaoWidth, info.width);
	if(width == -1)
	{
		print_func(ref,"Illegal width: %d\n", info.width);
		width = width_E;
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

void	WED_TaxiRoute::Import(const AptServiceRoadEdge_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(info.name);
	oneway = info.oneway;
	runway = atc_rwy_None;
	vehicle_class = atc_Vehicle_Ground_Trucks;
}

void	WED_TaxiRoute::Export(AptRouteEdge_t& info, AptServiceRoadEdge_t& info2) const
{
	if(AllowTrucks())
	{
		info2.oneway = oneway.value;
		this->GetName(info2.name);
		return;
	}
	else
	{
		info.hot_depart.clear();
		info.hot_arrive.clear();
		info.hot_ils.clear();
		info.width = ENUM_Export(width.value);

		if(runway.value == atc_rwy_None)
		{
			this->GetName(info.name);
			info.runway = 0;
		}
		else
		{
			info.runway = 1;
			info.name = ENUM_Desc(runway.value);
			
			set<int>	runway_parts;
			get_runway_parts(runway.value,runway_parts);

			for(set<int>::iterator itr = runway_parts.begin(); itr != runway_parts.end(); ++itr)
			{
				info.hot_depart.insert(ENUM_Desc(*itr));
				info.hot_arrive.insert(ENUM_Desc(*itr));
				info.hot_ils.insert(ENUM_Desc(*itr));
			}
		}

		set<int>::iterator h;
		for (h = hot_depart.value.begin(); h != hot_depart.value.end(); ++h)
			info.hot_depart.insert(ENUM_Desc(*h));
		for (h = hot_arrive.value.begin(); h != hot_arrive.value.end(); ++h)
			info.hot_arrive.insert(ENUM_Desc(*h));
		for (h = hot_ils.value.begin(); h != hot_ils.value.end(); ++h)
			info.hot_ils.insert(ENUM_Desc(*h));

		info.oneway = oneway.value;
		return;
	}
}

void	WED_TaxiRoute::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	dict.clear();
	if(n == PropertyItemNumber(&runway))
	{
		const WED_Airport * airport = WED_GetParentAirport(this);
		if(airport)
		{
			PropertyDict_t full;
			WED_GISEdge::GetNthPropertyDict(n,full);			
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
		const WED_Airport * airport = WED_GetParentAirport(this);
		if(airport)
		{
			set<int>	runway_parts;
			get_runway_parts(runway.value,runway_parts);
			
		
			PropertyDict_t full;
			WED_GISEdge::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysOneway(airport, legal);
			PropertyVal_t val;
			this->GetNthProperty(n,val);
			DebugAssert(val.prop_kind == prop_EnumSet);
			copy(val.set_val.begin(),val.set_val.end(),set_inserter(legal));
			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,make_pair(f->second.first,runway_parts.count(f->first) == 0)));
		}
	}
	else
		WED_GISEdge::GetNthPropertyDict(n,dict);			
}

void		WED_TaxiRoute::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	WED_GISEdge::GetNthPropertyInfo(n, info);
	if(runway.value != atc_rwy_None)
	if(n == PropertyItemNumber(&name))
	{
		info.can_delete = false;
		info.can_edit = false;
	}

	PropertyVal_t prop;
	vehicle_class.GetProperty(prop);

	if (prop.int_val == atc_Vehicle_Ground_Trucks)
	{
		if (n == PropertyItemNumber(&runway) ||
			n == PropertyItemNumber(&hot_depart) ||
			n == PropertyItemNumber(&hot_arrive) ||
			n == PropertyItemNumber(&hot_ils) ||
			n == PropertyItemNumber(&width))
		{
			info.prop_name = ".";
			info.can_edit = false;
			info.can_delete = false;
		}
	}
}

void		WED_TaxiRoute::GetNthProperty(int n, PropertyVal_t& val) const
{
	WED_GISEdge::GetNthProperty(n, val);
	if(runway.value != atc_rwy_None)
	{
		if(n == PropertyItemNumber(&name))
		{
			val.string_val = ENUM_Desc(runway.value);		
		}
		
		if(n == PropertyItemNumber(&hot_depart) ||
		n == PropertyItemNumber(&hot_arrive) ||
		n == PropertyItemNumber(&hot_ils))
		{
			set<int>	runway_parts;
			get_runway_parts(runway.value,runway_parts);
			copy(runway_parts.begin(),runway_parts.end(),set_inserter(val.set_val));
		}
	}
}

bool	WED_TaxiRoute::IsOneway(void) const
{
	return oneway.value;
}

bool	WED_TaxiRoute::IsRunway(void) const
{
	return runway.value != atc_rwy_None;
}

bool	WED_TaxiRoute::HasHotArrival(void) const
{
	// BEN SAYS: we used to treat being a runway as being hot.  But the UI needs to distinguish between
	// "I am a runway and hot because I am a runway" and "Ia m a runway and hot for a CROSSING runway" -e.g.
	// a LAHSO marking.  So only return TRUE hotness.
	set<int>	runway_parts;
	get_runway_parts(runway.value,runway_parts);

	for(set<int>::iterator i = hot_arrive.value.begin(); i != hot_arrive.value.end(); ++i)
		if(runway_parts.count(*i) == 0)
			return true;
	return false;
}

bool	WED_TaxiRoute::HasHotDepart(void) const
{
	set<int>	runway_parts;
	get_runway_parts(runway.value,runway_parts);

	for(set<int>::iterator i = hot_depart.value.begin(); i != hot_depart.value.end(); ++i)
		if(runway_parts.count(*i) == 0)
			return true;
	return false;
}

bool	WED_TaxiRoute::HasHotILS(void) const
{
	set<int>	runway_parts;
	get_runway_parts(runway.value,runway_parts);

	for(set<int>::iterator i = hot_ils.value.begin(); i != hot_ils.value.end(); ++i)
		if(runway_parts.count(*i) == 0)
			return true;
	return false;
}

bool	WED_TaxiRoute::HasInvalidHotZones(const set<int>& legal_rwys) const
{
	set<int>::const_iterator z;

	for(z = hot_depart.value.begin(); z != hot_depart.value.end(); ++z)
	if(legal_rwys.count(*z) == 0)
		return true;

	for(z = hot_arrive.value.begin(); z != hot_arrive.value.end(); ++z)
	if(legal_rwys.count(*z) == 0)
		return true;

	for(z = hot_ils.value.begin(); z != hot_ils.value.end(); ++z)
	if(legal_rwys.count(*z) == 0)
		return true;
	
	return false;
}

int		WED_TaxiRoute::GetRunway(void) const
{
	return runway.value;
}

int		WED_TaxiRoute::GetWidth(void) const
{
	return width.value;
}

WED_Thing *		WED_TaxiRoute::CreateSplitNode()
{
	return WED_TaxiRouteNode::CreateTyped(GetArchive());
}

bool	WED_TaxiRoute::AllowAircraft(void) const
{
	return vehicle_class.value == atc_Vehicle_Aircraft;
}

bool	WED_TaxiRoute::AllowTrucks(void) const
{
	return vehicle_class.value == atc_Vehicle_Ground_Trucks;
}

bool	WED_TaxiRoute::CanBeCurved() const
{
#if HAS_CURVED_ATC_ROUTE
	return true;
#else
	return false;
#endif
}

#endif

