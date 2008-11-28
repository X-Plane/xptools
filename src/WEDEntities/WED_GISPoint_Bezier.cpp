/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "WED_GISPoint_Bezier.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"
#include "GISUtils.h"

TRIVIAL_COPY(WED_GISPoint_Bezier, WED_GISPoint)


// NOTE: control lat/lon are positive vectors FROM the origin pt!


WED_GISPoint_Bezier::WED_GISPoint_Bezier(WED_Archive * parent, int id) :
	WED_GISPoint(parent, id),
	is_split(this,"Split", "GIS_points_bezier","split",0),
	ctrl_lat_lo(this,"control_latitude_lo" ,"GIS_points_bezier","ctrl_latitude_lo" ,0.0,10,6),
	ctrl_lon_lo(this,"control_longitude_lo","GIS_points_bezier","ctrl_longitude_lo",0.0,11,6),
	ctrl_lat_hi(this,"control_latitude_hi" ,"GIS_points_bezier","ctrl_latitude_hi" ,0.0,10,6),
	ctrl_lon_hi(this,"control_longitude_hi","GIS_points_bezier","ctrl_longitude_hi",0.0,11,6)
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
	p.x_ += ctrl_lon_lo.value;
	p.y_ += ctrl_lat_lo.value;
	return (ctrl_lon_lo.value != 0.0 || ctrl_lat_lo.value != 0.0);
}

bool	WED_GISPoint_Bezier::GetControlHandleHi (      Point2& p) const
{
	GetLocation(p);
	p.x_ += ctrl_lon_hi.value;
	p.y_ += ctrl_lat_hi.value;
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
		CacheInval();
		CacheBuild();
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
		CacheInval();
		CacheBuild();
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
		CacheInval();
		CacheBuild();
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
		CacheInval();
		CacheBuild();
	}
}

void	WED_GISPoint_Bezier::SetSplit		   (bool split)
{
	if (split != (is_split.value != 0))
	{
		StateChanged();
		is_split.value = split;
		CacheInval();
		CacheBuild();
	}
}

void WED_GISPoint_Bezier::Reverse(void)
{
	StateChanged();
	CacheInval();
	CacheBuild();
	swap(ctrl_lat_lo.value, ctrl_lat_hi.value);
	swap(ctrl_lon_lo.value, ctrl_lon_hi.value);

}



void			WED_GISPoint_Bezier::Rotate			(const Point2& ctr, double a)
{
	if (a != 0.0)
	{
		Point2 p;
		GetLocation(p);
		StateChanged();
	
		Point2	pt_old_lo(p.x() + ctrl_lon_lo.value, p.y() + ctrl_lat_lo.value);
		Point2	pt_old_hi(p.x() + ctrl_lon_hi.value, p.y() + ctrl_lat_hi.value);
		Vector2	v_old_lo = VectorLLToMeters(ctr, Vector2(ctr,pt_old_lo));
		Vector2	v_old_hi = VectorLLToMeters(ctr, Vector2(ctr,pt_old_hi));
		double old_len_lo = sqrt(v_old_lo.squared_length());
		double old_len_hi = sqrt(v_old_hi.squared_length());

		double old_ang_lo = VectorMeters2NorthHeading(ctr,ctr,v_old_lo);
		double old_ang_hi = VectorMeters2NorthHeading(ctr,ctr,v_old_hi);
		Vector2	v_new_lo;
		Vector2	v_new_hi;

		NorthHeading2VectorMeters(ctr, ctr, old_ang_lo + a, v_new_lo);
		NorthHeading2VectorMeters(ctr, ctr, old_ang_hi + a, v_new_hi);
		v_new_lo.normalize();
		v_new_hi.normalize();
		v_new_lo *= old_len_lo;
		v_new_hi *= old_len_hi;

		v_new_lo = VectorMetersToLL(ctr,v_new_lo);
		v_new_hi = VectorMetersToLL(ctr,v_new_hi);

		WED_GISPoint::Rotate(ctr,a);
		GetLocation(p);

		ctrl_lon_lo.value = ctr.x() + v_new_lo.dx - p.x();
		ctrl_lon_hi.value = ctr.x() + v_new_hi.dx - p.x();
		ctrl_lat_lo.value = ctr.y() + v_new_lo.dy - p.y();
		ctrl_lat_hi.value = ctr.y() + v_new_hi.dy - p.y();
		
	}
}

