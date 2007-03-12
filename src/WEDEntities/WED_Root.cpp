#include "WED_Root.h"

DEFINE_PERSISTENT(WED_Root)
START_CASTING(WED_Root)
INHERITS_FROM(WED_Thing)
END_CASTING

WED_Root::WED_Root(WED_Archive * a, int i) : WED_Thing(a, i)
{
}

WED_Root::~WED_Root()
{
}
