#include "WED_Windsock.h"

DEFINE_PERSISTENT(WED_Windsock)
START_CASTING(WED_Windsock)
INHERITS_FROM(WED_GISPoint)
END_CASTING

WED_Windsock::WED_Windsock(WED_Archive * a, int i) : WED_GISPoint(a,i),
	lit(this,"Lit","WED_windsocks", "lit",0)
{
}

WED_Windsock::~WED_Windsock()
{
}

void	WED_Windsock::SetLit(int l)
{
	lit = l;
}
