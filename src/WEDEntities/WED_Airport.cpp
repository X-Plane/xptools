#include "WED_Airport.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "XESConstants.h"

DEFINE_PERSISTENT(WED_Airport)
TRIVIAL_COPY(WED_Airport, WED_GISComposite)

WED_Airport::WED_Airport(WED_Archive * a, int i) : WED_GISComposite(a,i),
	airport_type	(this, "Type",				"WED_airport",	"kind",			Airport_Type, type_Airport),
	elevation		(this, "Field Elevation",	"WED_airport",	"elevation",	0,6,1),
	has_atc			(this, "Has ATC",			"WED_airport",	"has_atc",		1),
	icao			(this, "ICAO Identifier",	"WED_airport",	"icao",			"xxxx")
{
}

WED_Airport::~WED_Airport()
{
}

void	WED_Airport::GetICAO(string& i) const
{
	i = icao.value;
}

int		WED_Airport::GetAirportType(void) const
{
	return airport_type.value;
}

void		WED_Airport::SetAirportType(int x) { airport_type = x; }
void		WED_Airport::SetElevation(double x) { elevation = x; }
void		WED_Airport::SetHasATC(int x) { has_atc= x; }
void		WED_Airport::SetICAO(const string& x) { icao = x; }


void		WED_Airport::Import(const AptInfo_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	airport_type = ENUM_Import(Airport_Type, info.kind_code);
	if (airport_type == -1)
	{
		print_func(ref,"Error importing airport: airport type code %d is illegal (not a member of type %s).\n", info.kind_code, DOMAIN_Fetch(airport_type.domain));
		airport_type = type_Airport;
	}	
	
	elevation = info.elevation_ft * FT_TO_MTR;
	has_atc = info.has_atc_twr;
	icao = info.icao;
	SetName(info.name);
}

void		WED_Airport::Export(AptInfo_t& info) const
{
	info.kind_code = ENUM_Export(airport_type);
	info.elevation_ft = elevation.value * MTR_TO_FT;
	info.has_atc_twr = has_atc;
	info.icao = icao.value;
	GetName(info.name);
	info.default_buildings = 0;	// deprecated field.  Not supported in x-plane so not supported in WED!
	
	Bbox2	bounds;
	GetBounds(bounds);
	info.tower.location = info.beacon.location = Segment2(bounds.p1,bounds.p2).midpoint();
	info.tower.draw_obj = -1;
	info.tower.height_ft = 50.0;
	info.beacon.color_code = apt_beacon_none;	
}

