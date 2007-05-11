#include "WED_AirportBoundary.h"

DEFINE_PERSISTENT(WED_AirportBoundary)

WED_AirportBoundary::WED_AirportBoundary(WED_Archive * a, int i) : WED_GISPolygon(a,i)
{
}

WED_AirportBoundary::~WED_AirportBoundary()
{
}
