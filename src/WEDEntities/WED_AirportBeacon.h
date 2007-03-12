#ifndef WED_AIRPORTBEACON_H
#define WED_AIRPORTBEACON_H

#include "WED_GISPoint.h"

class	WED_AirportBeacon : public WED_GISPoint {

DECLARE_PERSISTENT(WED_AirportBeacon)

private:

	WED_PropIntEnum			kind;
	
};

#endif

