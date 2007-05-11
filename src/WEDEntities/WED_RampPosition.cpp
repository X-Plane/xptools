#include "WED_RampPosition.h"

DEFINE_PERSISTENT(WED_RampPosition)

WED_RampPosition::WED_RampPosition(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i)
{
}

WED_RampPosition::~WED_RampPosition()
{
}
