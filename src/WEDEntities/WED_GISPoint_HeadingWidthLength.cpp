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

#include "WED_GISPoint_HeadingWidthLength.h"

#include "WED_ToolUtils.h"
#include "IODefs.h"
#include "GISUtils.h"
#include "GISUtils.h"
#include "WED_Errors.h"

TRIVIAL_COPY(WED_GISPoint_HeadingWidthLength, WED_GISPoint_Heading)

WED_GISPoint_HeadingWidthLength::WED_GISPoint_HeadingWidthLength(WED_Archive * parent, int id) :
	WED_GISPoint_Heading(parent, id),
	width (this,"width", XML_Name("point","width"),1.0,5,1),
	length(this,"length",XML_Name("point","length"),1.0,5,1)
{
}

WED_GISPoint_HeadingWidthLength::~WED_GISPoint_HeadingWidthLength()
{
}

GISClass_t	WED_GISPoint_HeadingWidthLength::GetGISClass		(void				 ) const
{
	return gis_Point_HeadingWidthLength;
}

void			WED_GISPoint_HeadingWidthLength::GetBounds		(GISLayer_t l,   Bbox2&  bounds) const
{
	CacheBuild(cache_Spatial);
	Point2 corners[4];
	GetCorners(l,corners);
	bounds = Bbox2(corners[0],corners[1]);
	bounds += corners[2];
	bounds += corners[3];
}

bool			WED_GISPoint_HeadingWidthLength::IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	return me.overlap(bounds);
}

bool			WED_GISPoint_HeadingWidthLength::WithinBox		(GISLayer_t l,const Bbox2&  bounds) const
{
	Point2	corners[4];
	GetCorners(l,corners);
	return  bounds.contains(corners[0]) &&
			bounds.contains(corners[1]) &&
			bounds.contains(corners[2]) &&
			bounds.contains(corners[3]);
}

bool			WED_GISPoint_HeadingWidthLength::PtOnFrame		(GISLayer_t l,const Point2& p, double dist) const
{
	Point2	corners[4];
	GetCorners(l,corners);
	if (Segment2(corners[0],corners[1]).is_near(p,dist)) return true;
	if (Segment2(corners[1],corners[2]).is_near(p,dist)) return true;
	if (Segment2(corners[2],corners[3]).is_near(p,dist)) return true;
	if (Segment2(corners[3],corners[0]).is_near(p,dist)) return true;
	return false;
}

bool			WED_GISPoint_HeadingWidthLength::PtWithin		(GISLayer_t l,const Point2& p	 ) const
{
	Point2 corners[4];
	GetCorners(l,corners);
	return inside_polygon_pt(corners,corners+4,p);
}

void			WED_GISPoint_HeadingWidthLength::Rescale(GISLayer_t la,
								const Bbox2& old_bounds,			// Defines a linear remappign of coordinates we can apply.
								const Bbox2& new_bounds)
{
	Point2 corners[4];
	GetCorners(la,corners);
	for(int n = 0; n < 4; ++n)
	{
		corners[n].x_ = old_bounds.rescale_to_x(new_bounds,corners[n].x());
		corners[n].y_ = old_bounds.rescale_to_y(new_bounds,corners[n].y());
	}

	Point2	 ctr;
	double  h, l, w;
	Quad_4to1(corners, ctr, h, l, w);
	SetLocation(la,ctr);
	SetHeading(h);
	SetWidth(w);
	SetLength(l);

}

double	WED_GISPoint_HeadingWidthLength::GetWidth (void		 ) const
{
	return width.value;
}

void	WED_GISPoint_HeadingWidthLength::SetWidth (double w)
{
	if (w < 1.0) w = 1.0;
	if (w != width.value)
	{
		StateChanged();
		width.value = w;
		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);
	}
}

double	WED_GISPoint_HeadingWidthLength::GetLength(void		 ) const
{
	return length.value;
}

void	WED_GISPoint_HeadingWidthLength::SetLength(double l)
{
	if (l < 1.0) l = 1.0;
	if (l != length.value)
	{
		StateChanged();
		length.value = l;
		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);
	}
}

void	WED_GISPoint_HeadingWidthLength::GetCorners(GISLayer_t l,Point2 corners[4]) const
{
	Point2		center;
	GetLocation(l,center);

	Quad_1to4(center, GetHeading(), GetLength(), GetWidth(), corners);
}

void	WED_GISPoint_HeadingWidthLength::MoveCorner(GISLayer_t la,int corner, const Vector2& delta)
{
	Point2	corners[4];
	Point2	ends[2];

	GetCorners(la,corners);
	corners[corner] += delta;
	int swapped = corner == 1 || corner == 3;
	if (swapped)
	{
		ends[0] = corners[3];
		ends[1] = corners[1];
	} else {
		ends[0] = corners[0];
		ends[1] = corners[2];
	}
		Point2	ctr;
		double	h, l;
		double	w = GetWidth();
	Quad_diagto1(ends, w, ctr, h, l, swapped);

	SetLocation(la,ctr);
	SetLength(l);
	SetHeading(h);
}


void	WED_GISPoint_HeadingWidthLength::MoveSide(GISLayer_t la,int side, const Vector2& delta)
{
	Point2	ends[2];
	Point2	ctr;
	double h = GetHeading(), l = GetLength(), w = GetWidth();
	GetLocation(la,ctr);

	Quad_1to2(ctr, h, l, ends);

	Quad_MoveSide2(ends, w, side, delta);

	Quad_2to1(ends, ctr, h, l);

	SetWidth(w);
	SetLocation(la,ctr);
	SetLength(l);
	SetHeading(h);
}

void	WED_GISPoint_HeadingWidthLength::ResizeSide(GISLayer_t la,int side, const Vector2& delta, bool symetric)
{
	Point2	corners[4];
	Point2	ctr;
	double h, l, w;

	GetCorners(la,corners);
	Quad_ResizeSide4(corners, side, delta, symetric);
	Quad_4to1(corners, ctr, h, l, w);

	SetLocation(la,ctr);
	SetLength(l);
	SetWidth(w);
	SetHeading(h);
}

void	WED_GISPoint_HeadingWidthLength::ResizeCorner(GISLayer_t la,int corner, const Vector2& delta, bool symetric)
{
	Point2	ctr;
	double h = GetHeading(), l = GetLength(), w = GetWidth();
	GetLocation(la,ctr);

	Quad_ResizeCorner1(ctr, h, l, w, corner, delta, symetric);

	SetLocation(la,ctr);
	SetLength(l);
	SetWidth(w);

}


