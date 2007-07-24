#include "WED_Root.h"

DEFINE_PERSISTENT(WED_Root)
TRIVIAL_COPY(WED_Root, WED_Thing)

WED_Root::WED_Root(WED_Archive * a, int i) : WED_Thing(a, i)
{
}

WED_Root::~WED_Root()
{
}
