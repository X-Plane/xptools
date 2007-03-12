#include "WED_GISPoint_Heading.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

START_CASTING(WED_GISPoint_Heading)
IMPLEMENTS_INTERFACE(IGISEntity)
IMPLEMENTS_INTERFACE(IGISPoint)
IMPLEMENTS_INTERFACE(IGISPoint_Heading)
INHERITS_FROM(WED_GISPoint)
END_CASTING


WED_GISPoint_Heading::WED_GISPoint_Heading(WED_Archive * parent, int id) :
	WED_GISPoint(parent, id),
	heading(this,"heading","GIS_points_heading", "heading",0.0)
{
}

WED_GISPoint_Heading::~WED_GISPoint_Heading()
{
}

double	WED_GISPoint_Heading::GetHeading(void			) const
{
	return heading.value;
}

void	WED_GISPoint_Heading::SetHeading(double h)
{
	if (h != heading.value)
	{
		StateChanged();
		heading.value = h;
	}
}
