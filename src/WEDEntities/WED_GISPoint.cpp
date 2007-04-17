#include "WED_GISPoint.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

START_CASTING(WED_GISPoint)
IMPLEMENTS_INTERFACE(IGISEntity)
IMPLEMENTS_INTERFACE(IGISPoint)
INHERITS_FROM(WED_Entity)
END_CASTING

WED_GISPoint::WED_GISPoint(WED_Archive * parent, int id) :
	WED_Entity(parent, id), 
	latitude(this,"latitude","GIS_points","latitude",0.0),
	longitude(this,"longitude","GIS_points","longitude",0.0)
{
}

WED_GISPoint::~WED_GISPoint()
{
}

GISClass_t		WED_GISPoint::GetGISClass		(void				 ) const
{
	return gis_Point;
}

void			WED_GISPoint::GetBounds		(	   Bbox2&  bounds) const
{
	bounds = Bbox2(Point2(longitude.value,latitude.value));
}

//int				WED_GISPoint::IntersectsBox	(const Bbox2&  bounds) const
//{
//	return bounds.contains(Point2(longitude.value,latitude.value));
//}

bool				WED_GISPoint::WithinBox		(const Bbox2&  bounds) const
{
	return bounds.contains(Point2(longitude.value,latitude.value));
}

bool				WED_GISPoint::PtWithin		(const Point2& p	 ) const
{
	return false;
}

bool				WED_GISPoint::PtOnFrame		(const Point2& p, double dist) const
{
	return p.squared_distance(Point2(longitude.value,latitude.value)) < (dist*dist);
}

void			WED_GISPoint::Rescale			(const Bbox2& old_bounds, const Bbox2& new_bounds)
{
	if (old_bounds != new_bounds)
	{
		StateChanged();
		longitude.value = old_bounds.rescale_to_x(new_bounds,longitude.value);
		latitude.value  = old_bounds.rescale_to_y(new_bounds,latitude.value );
	}
}

void	WED_GISPoint::GetLocation(      Point2& p) const
{
	p.x = longitude.value;
	p.y = latitude.value;
}

void	WED_GISPoint::SetLocation(const Point2& p)
{
	if (p.x != longitude.value || p.y != latitude.value)
	{
		StateChanged();
		longitude.value = p.x;
		latitude.value = p.y;
	}
}

