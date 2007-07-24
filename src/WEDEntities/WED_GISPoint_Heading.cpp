#include "WED_GISPoint_Heading.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

TRIVIAL_COPY(WED_GISPoint_Heading, WED_GISPoint)

WED_GISPoint_Heading::WED_GISPoint_Heading(WED_Archive * parent, int id) :
	WED_GISPoint(parent, id),
	heading(this,"heading","GIS_points_heading", "heading",0.0,6,2)
{
}

WED_GISPoint_Heading::~WED_GISPoint_Heading()
{
}

GISClass_t		WED_GISPoint_Heading::GetGISClass		(void				 ) const
{
	return gis_Point_Heading;
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
		CacheInval();
		CacheBuild();
	}
}

void	WED_GISPoint_Heading::Rotate			(const Point2& center, double angle)
{
	WED_GISPoint::Rotate(center,angle);
	heading = heading.value + angle;
}
