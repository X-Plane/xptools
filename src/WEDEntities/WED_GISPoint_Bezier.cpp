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
	is_split(this,"Split", "GIS_point_bezier","split",0),
	ctrl_lat_lo(this,"control_latitude_lo","GIS_points_bezier","ctrl_latitude_lo",0.0),
	ctrl_lon_lo(this,"control_longitude_lo","GIS_points_bezier","ctrl_longitude_lo",0.0),
	ctrl_lat_hi(this,"control_latitude_hi","GIS_points_bezier","ctrl_latitude_hi",0.0),
	ctrl_lon_hi(this,"control_longitude_hi","GIS_points_bezier","ctrl_longitude_hi",0.0)
{
}

WED_GISPoint_Bezier::~WED_GISPoint_Bezier()
{
}

GISClass_t		WED_GISPoint_Bezier::GetGISClass		(void				 ) const
{
	return gis_Point_Bezier;
}

void			WED_GISPoint_Bezier::Rescale			(const Bbox2& old_bounds, const Bbox2& new_bounds)
{
	WED_GISPoint::Rescale(old_bounds, new_bounds);
	ctrl_lon_lo.value = old_bounds.rescale_to_xv(new_bounds,ctrl_lon_lo.value);
	ctrl_lat_lo.value = old_bounds.rescale_to_yv(new_bounds,ctrl_lat_lo.value );
	ctrl_lon_hi.value = old_bounds.rescale_to_xv(new_bounds,ctrl_lon_hi.value);
	ctrl_lat_hi.value = old_bounds.rescale_to_yv(new_bounds,ctrl_lat_hi.value );
}

bool	WED_GISPoint_Bezier::GetControlHandleLo (      Point2& p) const
{
	GetLocation(p);
	p.x += ctrl_lon_lo.value;
	p.y += ctrl_lat_lo.value;
	return (ctrl_lon_lo.value != 0.0 || ctrl_lat_lo.value != 0.0);
}

bool	WED_GISPoint_Bezier::GetControlHandleHi (      Point2& p) const
{
	GetLocation(p);
	p.x += ctrl_lon_hi.value;
	p.y += ctrl_lat_hi.value;
	return (ctrl_lon_hi.value != 0.0 || ctrl_lat_hi.value != 0.0);
}

bool	WED_GISPoint_Bezier::IsSplit(void) const
{
	return is_split.value;
}



void	WED_GISPoint_Bezier::SetControlHandleLo (const Point2& p)
{
	Point2	me;
	GetLocation(me);
	Vector2	lo_vec(me,p);
	Vector2	hi_vec(ctrl_lon_hi.value,ctrl_lat_hi.value);
	if (!is_split.value)
		hi_vec = -lo_vec;
	
	if (lo_vec.dx != ctrl_lon_lo.value ||
		lo_vec.dy != ctrl_lat_lo.value ||
		hi_vec.dx != ctrl_lon_hi.value ||
		hi_vec.dy != ctrl_lat_hi.value)
	{
		StateChanged();
		ctrl_lon_lo.value = lo_vec.dx;
		ctrl_lat_lo.value = lo_vec.dy;
		ctrl_lon_hi.value = hi_vec.dx;
		ctrl_lat_hi.value = hi_vec.dy;
	}
}

void	WED_GISPoint_Bezier::SetControlHandleHi (const Point2& p)
{
	Point2	me;
	GetLocation(me);
	Vector2	hi_vec(me,p);
	Vector2	lo_vec(ctrl_lon_lo.value,ctrl_lat_lo.value);
	if (!is_split.value)
		lo_vec = -hi_vec;
	
	if (lo_vec.dx != ctrl_lon_lo.value ||
		lo_vec.dy != ctrl_lat_lo.value ||
		hi_vec.dx != ctrl_lon_hi.value ||
		hi_vec.dy != ctrl_lat_hi.value)
	{
		StateChanged();
		ctrl_lon_lo.value = lo_vec.dx;
		ctrl_lat_lo.value = lo_vec.dy;
		ctrl_lon_hi.value = hi_vec.dx;
		ctrl_lat_hi.value = hi_vec.dy;
	}
}

void	WED_GISPoint_Bezier::DeleteHandleLo	   (void)
{
	Vector2	lo_vec(0.0,0.0);
	Vector2	hi_vec(ctrl_lon_hi.value,ctrl_lat_hi.value);
	if (!is_split.value)
		hi_vec = lo_vec;
	
	if (lo_vec.dx != ctrl_lon_lo.value ||
		lo_vec.dy != ctrl_lat_lo.value ||
		hi_vec.dx != ctrl_lon_hi.value ||
		hi_vec.dy != ctrl_lat_hi.value)
	{
		StateChanged();
		ctrl_lon_lo.value = lo_vec.dx;
		ctrl_lat_lo.value = lo_vec.dy;
		ctrl_lon_hi.value = hi_vec.dx;
		ctrl_lat_hi.value = hi_vec.dy;
	}
}

void	WED_GISPoint_Bezier::DeleteHandleHi	   (void)
{
	Vector2	hi_vec(0.0,0.0);
	Vector2	lo_vec(ctrl_lon_lo.value,ctrl_lat_lo.value);
	if (!is_split.value)
		lo_vec = hi_vec;
	
	if (lo_vec.dx != ctrl_lon_lo.value ||
		lo_vec.dy != ctrl_lat_lo.value ||
		hi_vec.dx != ctrl_lon_hi.value ||
		hi_vec.dy != ctrl_lat_hi.value)
	{
		StateChanged();
		ctrl_lon_lo.value = lo_vec.dx;
		ctrl_lat_lo.value = lo_vec.dy;
		ctrl_lon_hi.value = hi_vec.dx;
		ctrl_lat_hi.value = hi_vec.dy;
	}
}

void	WED_GISPoint_Bezier::SetSplit		   (bool split)
{
	if (split != is_split.value)
	{
		StateChanged();
		is_split.value = split;
	}
}



