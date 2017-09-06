//
//  WED_TruckDestination.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 9/4/16.
//
//

#include "WED_TruckDestination.h"
#include "AptDefs.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_TruckDestination)
TRIVIAL_COPY(WED_TruckDestination, WED_GISPoint_Heading)

WED_TruckDestination::WED_TruckDestination(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i),
	truck_types		(this, "Truck Types",   XML_Name("truck_destination","types"), ATCServiceTruckType, 0)
{
}

WED_TruckDestination::~WED_TruckDestination()
{
}

void	WED_TruckDestination::SetTruckTypes(const set<int>& truckTypes) { truck_types = truckTypes; }
void	WED_TruckDestination::GetTruckTypes(set<int>& truckTypes) const { truckTypes = truck_types.value; }


void	WED_TruckDestination::Import(const AptTruckDestination_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(gis_Geo, x.location);
	SetHeading(x.heading);
	SetName(x.name);
	
	set<int>	tt;
	
	for(set<int>::const_iterator i = x.truck_types.begin(); i != x.truck_types.end(); ++i)
	{
		int t = ENUM_Import(ATCServiceTruckType, *i);
		if(t == -1)
		{
			print_func(ref,"Illegal truck type: %d\n", *i);
		}
		else
			tt.insert(t);
	}
	truck_types = tt;
}

void	WED_TruckDestination::Export(		 AptTruckDestination_t& x) const
{
	GetName(x.name);
	GetLocation(gis_Geo,x.location);
	x.heading = GetHeading();
	
	x.truck_types.clear();
	for(set<int>::const_iterator i = truck_types.value.begin(); i != truck_types.value.end(); ++i)
		x.truck_types.insert(ENUM_Export(*i));
}
