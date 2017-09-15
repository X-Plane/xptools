//
//  WED_TruckParkingLocation.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 9/4/16.
//
//

#include "WED_TruckParkingLocation.h"

#include "WED_TruckParkingLocation.h"
#include "AptDefs.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_TruckParkingLocation)
TRIVIAL_COPY(WED_TruckParkingLocation, WED_GISPoint_Heading)

WED_TruckParkingLocation::WED_TruckParkingLocation(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i),
	truck_type    (this, "Truck Type",             XML_Name("truck_parking_spot","type"   ),    ATCServiceTruckType, apt_truck_fuel_prop),
	number_of_cars(this, "Number of Baggage Cars", XML_Name("truck_parking_spot","number_of_cars"), 3, 1)
{
}

WED_TruckParkingLocation::~WED_TruckParkingLocation()
{
}

void	WED_TruckParkingLocation::SetTruckType(int truckType) { truck_type = truckType; }
int		WED_TruckParkingLocation::GetTruckType(void) const { return truck_type.value; }

void	WED_TruckParkingLocation::SetNumberOfCars(int numberOfCars) { number_of_cars = numberOfCars; }
int		WED_TruckParkingLocation::GetNumberOfCars(void) const { return number_of_cars.value; }

void	WED_TruckParkingLocation::Import(const AptTruckParking_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(gis_Geo, x.location);
	SetHeading(x.heading);
	SetName(x.name);
	int tt = ENUM_Import(ATCServiceTruckType, x.parking_type);
	if(tt == -1)
	{
		print_func(ref,"Illegal truck type: %d\n", x.parking_type);
		truck_type = apt_truck_fuel_prop;
	}
	else
		truck_type = tt;
	number_of_cars = x.train_car_count;
	
}

void	WED_TruckParkingLocation::Export(		 AptTruckParking_t& x) const
{
	GetName(x.name);
	GetLocation(gis_Geo,x.location);
	x.heading = GetHeading();
	
	x.train_car_count = number_of_cars.value;
	
	x.parking_type = ENUM_Export(truck_type.value);
	
}


void		WED_TruckParkingLocation::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	if (truck_type.value != atc_ServiceTruck_Baggage_Train && n == PropertyItemNumber(&number_of_cars))
	{
		info.prop_name = "."; //Hardcoded "Do not display" name
	}
	else
	{
		WED_GISPoint_Heading::GetNthPropertyInfo(n, info);
	}
}