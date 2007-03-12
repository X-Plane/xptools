#include "WED_Group.h"

WED_Group::WED_Group(WED_Archive * a, int i) : WED_GISComposite(a,i)
{
}

WED_Group::~WED_Group()
{
}

DEFINE_PERSISTENT(WED_Group)
START_CASTING(WED_Group)
INHERITS_FROM(WED_GISComposite)
END_CASTING

