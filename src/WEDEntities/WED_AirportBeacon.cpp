#include "WED_AirportBeacon.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_AirportBeacon)

WED_AirportBeacon::WED_AirportBeacon(WED_Archive * a, int i) : WED_GISPoint(a,i),
	kind(this,"Type", "WED_beacons", "type", Airport_Beacon, beacon_Airport)
{
}

WED_AirportBeacon::~WED_AirportBeacon()
{
}

void		WED_AirportBeacon::SetKind(int k)
{
	kind = k;
}

void	WED_AirportBeacon::Import(const AptBeacon_t& x)
{
	SetLocation(x.location);
	kind = ENUM_Import(Airport_Beacon, x.color_code);
	SetName(x.name);
}

void	WED_AirportBeacon::Export(		 AptBeacon_t& x) const
{
	GetLocation(x.location);
	x.color_code = ENUM_Export(kind.value);
	GetName(x.name);
}
