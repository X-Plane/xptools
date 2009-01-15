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

#include "WED_GISPoint.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"
#include "GISUtils.h"

TRIVIAL_COPY(WED_GISPoint, WED_Entity)

WED_GISPoint::WED_GISPoint(WED_Archive * parent, int id) :
	WED_Entity(parent, id),
	latitude (this,"latitude" ,"GIS_points","latitude" ,0.0,10,6),
	longitude(this,"longitude","GIS_points","longitude",0.0,11,6)
{
}

WED_GISPoint::~WED_GISPoint()
{
}

GISClass_t		WED_GISPoint::GetGISClass		(void				 ) const
{
	return gis_Point;
}

const char *	WED_GISPoint::GetGISSubtype	(void				 ) const
{
	return GetClass();
}

bool			WED_GISPoint::HasUV(void) const
{
	return false;
}

void			WED_GISPoint::GetBounds		(	   Bbox2&  bounds) const
{
	bounds = Bbox2(Point2(longitude.value,latitude.value));
}

bool				WED_GISPoint::IntersectsBox	(const Bbox2&  bounds) const
{
	return bounds.contains(Point2(longitude.value,latitude.value));
}

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
		CacheInval();
		CacheBuild();
	}
}

void	WED_GISPoint::GetLocation(      Point2& p) const
{
	// Bit of a hack: a client can call this to build its own bounding box cache.
	// So re-validate OUR cache here.  (Otherwise our change of location won't
	// start a cache-inval cascade.)
	CacheBuild();

	p.x_ = longitude.value;
	p.y_ = latitude.value;
}

void	WED_GISPoint::GetUV	   (      Point2& p) const
{
	DebugAssert(!"Should not be accessing this.");
	p = Point2(0,0);
}

void	WED_GISPoint::SetUV	   (const Point2& p)
{
	DebugAssert(!"Should not be accessing this.");
}

void	WED_GISPoint::SetLocation(const Point2& p)
{
	if (p.x() != longitude.value || p.y() != latitude.value)
	{
		StateChanged();
		longitude.value = p.x();
		latitude.value = p.y();
		CacheInval();
		CacheBuild();
	}
}

void			WED_GISPoint::Rotate			(const Point2& ctr, double a)
{
	if (a != 0.0)
	{
		StateChanged();
		Point2	pt_old(longitude.value, latitude.value);
		Vector2	v_old = VectorLLToMeters(ctr,Vector2(ctr,pt_old));
		double old_len = sqrt(v_old.squared_length());

		double old_ang = VectorMeters2NorthHeading(ctr,ctr,v_old);
		Vector2	v_new;

		NorthHeading2VectorMeters(ctr, ctr, old_ang + a, v_new);
		v_new.normalize();
		v_new *= old_len;

		v_new = VectorMetersToLL(ctr,v_new);
		
		longitude.value = ctr.x() + v_new.dx;
		latitude.value = ctr.y() + v_new.dy;
		
		CacheInval();
		CacheBuild();
	}
}

