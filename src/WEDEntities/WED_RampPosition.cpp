#include "WED_RampPosition.h"

DEFINE_PERSISTENT(WED_RampPosition)
START_CASTING(WED_RampPosition)
INHERITS_FROM(WED_GISPoint_Heading)
END_CASTING

WED_RampPosition::WED_RampPosition(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i)
{
}

WED_RampPosition::~WED_RampPosition()
{
}
