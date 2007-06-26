#include "WED_AirportBoundary.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_AirportBoundary)

WED_AirportBoundary::WED_AirportBoundary(WED_Archive * a, int i) : WED_GISPolygon(a,i)
{
}

WED_AirportBoundary::~WED_AirportBoundary()
{
}

void WED_AirportBoundary::Import(const AptBoundary_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(x.name);
}

void WED_AirportBoundary::Export(		 AptBoundary_t& x) const
{
	GetName(x.name);
}
