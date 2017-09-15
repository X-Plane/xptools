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
#include "WED_Errors.h"
#include "GISUtils.h"

TRIVIAL_COPY(WED_GISPoint, WED_Entity)

WED_GISPoint::WED_GISPoint(WED_Archive * parent, int id) :
	WED_Entity(parent, id),
	latitude (this,"latitude" ,XML_Name("point","latitude" ),0.0,13,9),
	longitude(this,"longitude",XML_Name("point","longitude"),0.0,14,9)
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

bool			WED_GISPoint::HasLayer(GISLayer_t l) const
{
	return l == gis_Geo;
}

void			WED_GISPoint::GetBounds		(GISLayer_t l,Bbox2&  bounds) const
{
	CacheBuild(cache_Spatial);
	Point2	p;
	GetLocation(l,p);
	bounds = Bbox2(p);
}

bool				WED_GISPoint::IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const
{
	Point2	p;
	GetLocation(l,p);
	return bounds.contains(p);
}

bool				WED_GISPoint::WithinBox		(GISLayer_t l,const Bbox2&  bounds) const
{
	Point2	p;
	GetLocation(l,p);
	return bounds.contains(p);
}

bool				WED_GISPoint::PtWithin		(GISLayer_t l,const Point2& p	 ) const
{
	return false;
}

bool				WED_GISPoint::PtOnFrame		(GISLayer_t l, const Point2& pt, double dist) const
{
	Point2	p;
	GetLocation(l,p);
	return p.squared_distance(pt) < (dist*dist);
}

bool WED_GISPoint::Cull(const Bbox2& b) const
{
	Bbox2	me;
	GetBounds(gis_Geo, me);
	return b.overlap(me);
}

void			WED_GISPoint::Rescale			(GISLayer_t l,const Bbox2& old_bounds, const Bbox2& new_bounds)
{
	if (old_bounds != new_bounds)
	{
		Point2 p;
		GetLocation(l,p);
		p.x_ = old_bounds.rescale_to_x(new_bounds,p.x_);
		p.y_ = old_bounds.rescale_to_y(new_bounds,p.y_);
		SetLocation(l,p);
	}
}

void	WED_GISPoint::GetLocation(GISLayer_t l,     Point2& p) const
{
	// Bit of a hack: a client can call this to build its own bounding box cache.
	// So re-validate OUR cache here.  (Otherwise our change of location won't
	// start a cache-inval cascade.)
//	CacheBuild();

	// Ben says: 9/13/09 - the above comment is from WED 1.1 and seems like it is perhaps
	// wrong.  Since we reval our cache every time we inval it (this pair invals our parents
	// just once and then remembers that we did it) we should NOT ever be in a state where our
	// own cache is invalid.  (Since we have no cache, how can it be invalid?  So we mark it as
	// valid every time we inval, so the next inval will wrok.)
	//
	// So - commenting out for now.  If the caching system goes hinky on us...perhaps that will
	// show why this was necessary?
	

	if(l == gis_Geo)
	{
		p.x_ = longitude.value;
		p.y_ = latitude.value;
	} else {
		p.x_ = p.y_ = 0.0;
	}
}

void	WED_GISPoint::SetLocation(GISLayer_t l, const Point2& p)
{
	DebugAssert(l==gis_Geo);

	if (p.x() != longitude.value || p.y() != latitude.value)
	{
		StateChanged();
		longitude.value = p.x();
		latitude.value = p.y();
		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);
	}
}

void			WED_GISPoint::Rotate			(GISLayer_t l, const Point2& ctr, double a)
{
	if (a != 0.0)
	{
		Point2	pt_old;
		GetLocation(l,pt_old);
		Vector2	v_old = VectorLLToMeters(ctr,Vector2(ctr,pt_old));
		double old_len = sqrt(v_old.squared_length());

		double old_ang = VectorMeters2NorthHeading(ctr,ctr,v_old);
		Vector2	v_new;

		NorthHeading2VectorMeters(ctr, ctr, old_ang + a, v_new);
		v_new.normalize();
		v_new *= old_len;

		v_new = VectorMetersToLL(ctr,v_new);

		SetLocation(l,ctr + v_new);

		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);
	}
}

void		WED_GISPoint::PropEditCallback(int before)
{
	WED_Entity::PropEditCallback(before);
	
	if(!before)
	{
		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);
	}
	
}
