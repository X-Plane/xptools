#include "WED_AirportBeacon.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_AirportBeacon)
START_CASTING(WED_AirportBeacon)
INHERITS_FROM(WED_GISPoint)
END_CASTING

WED_AirportBeacon::WED_AirportBeacon(WED_Archive * a, int i) : WED_GISPoint(a,i),
	kind(this,"Type", "WED_beacons", "type", Airport_Beacon, beacon_Airport)
{
}

WED_AirportBeacon::~WED_AirportBeacon()
{
}
