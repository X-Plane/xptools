#include "WED_Sealane.h"

DEFINE_PERSISTENT(WED_Sealane)
START_CASTING(WED_Sealane)
INHERITS_FROM(WED_GISLine_Width)
END_CASTING

WED_Sealane::WED_Sealane(WED_Archive * a, int i) : WED_GISLine_Width(a,i),
	buoys(this,"Show Buoys", "WED_sealane","buoys",1),
	id1(this,"Identifier 1", "WED_sealane", "id1", "4"),
	id2(this,"Identifier 2", "WED_sealane", "id2", "4")
{
}

WED_Sealane::~WED_Sealane()
{
}

void	WED_Sealane::SetBuoys(int x)
{
	buoys = x;
}
