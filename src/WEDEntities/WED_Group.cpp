#include "WED_Group.h"

DEFINE_PERSISTENT(WED_Group)
TRIVIAL_COPY(WED_Group, WED_GISComposite)

WED_Group::WED_Group(WED_Archive * a, int i) : WED_GISComposite(a,i)
{
}

WED_Group::~WED_Group()
{
}


