#include "WED_AirportBeacon.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_Errors.h"

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

void	WED_AirportBeacon::Import(const AptBeacon_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(x.location);
	kind = ENUM_Import(Airport_Beacon, x.color_code);
	if (kind == -1)
	{
		print_func(ref,"Error importing airport beacon: beacon color code %d is illegal (not a member of type %s).\n", x.color_code, DOMAIN_Fetch(kind.domain));
		kind = beacon_Airport;
	}
	SetName(x.name);
}

void	WED_AirportBeacon::Export(		 AptBeacon_t& x) const
{
	GetLocation(x.location);
	x.color_code = ENUM_Export(kind.value);
	GetName(x.name);
}
