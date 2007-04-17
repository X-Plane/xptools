#include "WED_GISPoint_Bezier.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

START_CASTING(WED_GISPoint_Bezier)
IMPLEMENTS_INTERFACE(IGISEntity)
IMPLEMENTS_INTERFACE(IGISPoint)
IMPLEMENTS_INTERFACE(IGISPoint_Bezier)
INHERITS_FROM(WED_GISPoint)
END_CASTING

// NOTE: control lat/lon are positive vectors FROM the origin pt!


WED_GISPoint_Bezier::WED_GISPoint_Bezier(WED_Archive * parent, int id) :
	WED_GISPoint(parent, id),
	control_latitude(this,"control_latitude","GIS_points_bezier","ctrl_latitude",0.0),
	control_longitude(this,"control_longitude","GIS_points_bezier","ctrl_longitude",0.0)
{
}

WED_GISPoint_Bezier::~WED_GISPoint_Bezier()
{
}

GISClass_t		WED_GISPoint_Bezier::GetGISClass		(void				 ) const
{
	Point2	c,me;
	GetControlHandleHi(c);
	GetLocation(me);
	if (c == me) 	return gis_Point;
					return gis_Point_Bezier;
}

void			WED_GISPoint_Bezier::Rescale			(const Bbox2& old_bounds, const Bbox2& new_bounds)
{
	WED_GISPoint::Rescale(old_bounds, new_bounds);
	control_longitude.value = old_bounds.rescale_to_xv(new_bounds,control_longitude.value);
	control_latitude.value  = old_bounds.rescale_to_yv(new_bounds,control_latitude.value );
}

void	WED_GISPoint_Bezier::GetControlHandleLo (      Point2& p) const
{
	GetLocation(p);
	p.x -= control_longitude.value;
	p.y -= control_latitude.value;
}


void	WED_GISPoint_Bezier::SetControlHandleLo (const Point2& p)
{
	Point2 l;
	GetLocation(l);
	double	dx = l.x - p.x;
	double	dy = l.y - p.y;
	if (dx != control_longitude.value || dy != control_latitude.value)
	{
		StateChanged();
		control_longitude.value = dx;
		control_latitude.value = dy;
	}
}

void	WED_GISPoint_Bezier::GetControlHandleHi (      Point2& p) const
{
	GetLocation(p);
	p.x += control_longitude.value;
	p.y += control_latitude.value;
}

void	WED_GISPoint_Bezier::SetControlHandleHi (const Point2& p)
{
	Point2 l;
	GetLocation(l);
	double	dx = p.x - l.x;
	double	dy = p.y - l.y;
	if (dx != control_longitude.value || dy != control_latitude.value)
	{
		StateChanged();
		control_longitude.value = dx;
		control_latitude.value = dy;
	}
}


