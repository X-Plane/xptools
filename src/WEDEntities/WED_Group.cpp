#include "WED_Group.h"

WED_Group::WED_Group(WED_Archive * a, int i) : WED_Thing(a,i)
{
}

WED_Group::~WED_Group()
{
}

DEFINE_PERSISTENT(WED_Group)
INHERITS_FROM(WED_Thing)
END_CASTING

