#include "WED_Taxiway.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
DEFINE_PERSISTENT(WED_Taxiway)

WED_Taxiway::WED_Taxiway(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	surface(this,		"Surface",			"WED_taxiway",	"surface",	Surface_Type,	surf_Concrete),
	roughness(this,		"Roughness",		"WED_taxiway",	"roughness",	0.25),
	heading(this,		"Texture Heading",	"WED_taxiway",	"heading",		0)
{
}

WED_Taxiway::~WED_Taxiway()
{
}

void		WED_Taxiway::SetSurface(int s)
{
		surface = s;
}

void		WED_Taxiway::SetRoughness(double r)
{
		roughness = r;
}

void		WED_Taxiway::SetHeading(double h)
{
		heading = h;
}

int			WED_Taxiway::GetSurface(void) const
{
	return surface.value;
}

void		WED_Taxiway::Import(const AptTaxiway_t& x)
{
	surface = ENUM_Import(Surface_Type, x.surface_code);
	roughness = x.roughness_ratio;
	heading = x.heading;
	SetName(x.name);
}

void		WED_Taxiway::Export(		 AptTaxiway_t& x) const
{
	x.surface_code = ENUM_Export(surface.value);
	x.roughness_ratio = roughness;
	x.heading = heading;
	GetName(x.name);
}
