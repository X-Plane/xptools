#include "WED_GISPoint_HeadingWidthLength.h"

#include "WED_ToolUtils.h"
#include "IODefs.h"
#include "GISUtils.h"
#include "SQLUtils.h"
#include "GISUtils.h"
#include "WED_Errors.h"



WED_GISPoint_HeadingWidthLength::WED_GISPoint_HeadingWidthLength(WED_Archive * parent, int id) :
	WED_GISPoint_Heading(parent, id),
	width (this,"width", "GIS_points_headingwidthlength", "width" ,1.0,5,1),
	length(this,"length","GIS_points_headingwidthlength", "length",1.0,5,1)
{
}

WED_GISPoint_HeadingWidthLength::~WED_GISPoint_HeadingWidthLength()
{
}

GISClass_t	WED_GISPoint_HeadingWidthLength::GetGISClass		(void				 ) const
{
	return gis_Point_HeadingWidthLength;
}

void			WED_GISPoint_HeadingWidthLength::GetBounds		(	   Bbox2&  bounds) const
{
	CacheBuild();
	Point2 corners[4];
	GetCorners(corners);
	bounds = Bbox2(corners[0],corners[1]);
	bounds += corners[2];
	bounds += corners[3];
}

bool			WED_GISPoint_HeadingWidthLength::IntersectsBox	(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);
	return me.overlap(bounds);
}

bool			WED_GISPoint_HeadingWidthLength::WithinBox		(const Bbox2&  bounds) const
{
	Point2	corners[4];
	GetCorners(corners);
	return  bounds.contains(corners[0]) &&
			bounds.contains(corners[1]) &&
			bounds.contains(corners[2]) &&
			bounds.contains(corners[3]);
}

bool			WED_GISPoint_HeadingWidthLength::PtOnFrame		(const Point2& p, double dist) const
{
	Point2	corners[4];
	GetCorners(corners);
	if (Segment2(corners[0],corners[1]).is_near(p,dist)) return true;
	if (Segment2(corners[1],corners[2]).is_near(p,dist)) return true;
	if (Segment2(corners[2],corners[3]).is_near(p,dist)) return true;
	if (Segment2(corners[3],corners[0]).is_near(p,dist)) return true;
	return false;
}

bool			WED_GISPoint_HeadingWidthLength::PtWithin		(const Point2& p	 ) const
{
	Point2 corners[4];
	GetCorners(corners);	
	return inside_polygon_pt(corners,corners+4,p);
}

void			WED_GISPoint_HeadingWidthLength::Rescale(
								const Bbox2& old_bounds,			// Defines a linear remappign of coordinates we can apply.
								const Bbox2& new_bounds)
{
	Point2 corners[4];
	GetCorners(corners);
	for(int n = 0; n < 4; ++n)
	{
		corners[n].x = old_bounds.rescale_to_x(new_bounds,corners[n].x);
		corners[n].y = old_bounds.rescale_to_y(new_bounds,corners[n].y);
	}
	
	Point2	 ctr;
	double  h, l, w;
	Quad_4to1(corners, ctr, h, l, w);
	SetLocation(ctr);
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
	if (w < 0.0) w = 0.0;
	if (w != width.value)
	{
		StateChanged();
		width.value = w;
		CacheInval();
		CacheBuild();
	}
}

double	WED_GISPoint_HeadingWidthLength::GetLength(void		 ) const
{
	return length.value;
}

void	WED_GISPoint_HeadingWidthLength::SetLength(double l)
{
	if (l < 0.0) l = 0.0;
	if (l != length.value)
	{
		StateChanged();
		length.value = l;
		CacheInval();
		CacheBuild();
	}
}

void	WED_GISPoint_HeadingWidthLength::GetCorners(Point2 corners[4]) const
{
	Point2		center;
	GetLocation(center);
	
	Quad_1to4(center, GetHeading(), GetLength(), GetWidth(), corners);
}

void	WED_GISPoint_HeadingWidthLength::MoveCorner(int corner, const Vector2& delta)
{
	Point2	corners[4];
	Point2	ends[2];
	
	GetCorners(corners);
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

	SetLocation(ctr);
	SetLength(l);
	SetHeading(h);
}	


void	WED_GISPoint_HeadingWidthLength::MoveSide(int side, const Vector2& delta)
{	
	Point2	ends[2];
	Point2	ctr;
	double h = GetHeading(), l = GetLength(), w = GetWidth();	
	GetLocation(ctr);
		
	Quad_1to2(ctr, h, l, ends);

	Quad_MoveSide2(ends, w, side, delta);

	Quad_2to1(ends, ctr, h, l);
	
	SetWidth(w);
	SetLocation(ctr);
	SetLength(l);
	SetHeading(h);
}

void	WED_GISPoint_HeadingWidthLength::ResizeSide(int side, const Vector2& delta, bool symetric)
{
	Point2	corners[4];
	Point2	ctr;
	double h, l, w;
	
	GetCorners(corners);
	Quad_ResizeSide4(corners, side, delta, symetric);
	Quad_4to1(corners, ctr, h, l, w);

	SetLocation(ctr);
	SetLength(l);
	SetWidth(w);
	SetHeading(h);
}

void	WED_GISPoint_HeadingWidthLength::ResizeCorner(int corner, const Vector2& delta, bool symetric)
{
	Point2	ctr;
	double h = GetHeading(), l = GetLength(), w = GetWidth();	
	GetLocation(ctr);
	
	Quad_ResizeCorner1(ctr, h, l, w, corner, delta, symetric);
	
	SetLocation(ctr);
	SetLength(l);
	SetWidth(w);
	
}


