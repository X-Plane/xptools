#include "WED_Windsock.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_Windsock)

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

void	WED_Windsock::Import(const AptWindsock_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(x.location);
	SetName(x.name);
	lit = x.lit;
}

void	WED_Windsock::Export(		 AptWindsock_t& x) const
{
	GetLocation(x.location);
	GetName(x.name);
	x.lit = lit;
}
