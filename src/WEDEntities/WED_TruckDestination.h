//
//  WED_TruckDestination.h
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 9/4/16.
//
//

#ifndef WED_TruckDestination_h
#define WED_TruckDestination_h

#include "WED_GISPoint_Heading.h"

struct	AptTruckDestination_t;

class	WED_TruckDestination : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_TruckDestination)

public:

	void	SetTruckTypes(const set<int>& truckTypes);
	void	GetTruckTypes(set<int>& truckTypes) const;

	void	Import(const AptTruckDestination_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptTruckDestination_t& x) const;

	virtual const char *	HumanReadableType(void) const { return "Truck Destination"; }

private:

	WED_PropIntEnumSet			truck_types;

};



#endif /* WED_TruckDestination_h */
