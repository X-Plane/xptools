#include "WED_RampPosition.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_RampPosition)

WED_RampPosition::WED_RampPosition(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i)
{
}

WED_RampPosition::~WED_RampPosition()
{
}

void	WED_RampPosition::Import(const AptGate_t& x)
{
	SetLocation(x.location);
	SetHeading(x.heading);
	SetName(x.name);
}

void	WED_RampPosition::Export(		 AptGate_t& x) const
{
	GetLocation(x.location);
	x.heading = GetHeading();
	GetName(x.name);
}
