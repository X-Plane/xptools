//
//  WED_TruckParkingLocation.h
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 9/4/16.
//
//

#ifndef WED_TruckParkingLocation_h
#define WED_TruckParkingLocation_h

#include "WED_GISPoint_Heading.h"

struct	AptTruckParking_t;

class	WED_TruckParkingLocation : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_TruckParkingLocation)

public:

	void	SetTruckType(int truckType);
	int		GetTruckType(void) const;

	void	SetNumberOfCars(int numberOfCars);
	int		GetNumberOfCars(void) const;

	void	Import(const AptTruckParking_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptTruckParking_t& x) const;

	virtual const char *	HumanReadableType(void) const { return "Truck Parking Location"; }

private:

	WED_PropIntEnum			truck_type;
	WED_PropIntText			number_of_cars;

};



#endif /* WED_TruckParkingLocation_h */
