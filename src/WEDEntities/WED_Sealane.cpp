#include "WED_Sealane.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_Sealane)

WED_Sealane::WED_Sealane(WED_Archive * a, int i) : WED_GISLine_Width(a,i),
	buoys(this,"Show Buoys", "WED_sealane","buoys",1)
{
}

WED_Sealane::~WED_Sealane()
{
}

void	WED_Sealane::SetBuoys(int x)
{
	buoys = x;
}

void	WED_Sealane::Import(const AptSealane_t& x)
{
	GetSource()->SetLocation(x.ends.p1  );
	GetTarget()->SetLocation(x.ends.p2  );
				 SetWidth	(x.width_mtr);
	string	full = x.id[0] + string("/") + x.id[1];
	SetName(full);
	buoys = x.has_buoys;
}
					


void	WED_Sealane::Export(		 AptSealane_t& x) const
{
	GetSource()->GetLocation(x.ends.p1  );
	GetTarget()->GetLocation(x.ends.p2  );
							 x.width_mtr = GetWidth();
	string	full;
	GetName(full);
	string::size_type p = full.find('/');
	if (p == full.npos)
	{
		x.id[0] = full;
		x.id[1] = "xxx";
	}
	else
	{
		x.id[0] = full.substr(0,p);
		x.id[1] = full.substr(p+1);		
	}
	x.has_buoys = buoys;
}
