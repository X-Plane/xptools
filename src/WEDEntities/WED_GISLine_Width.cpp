#include "WED_GISLine_Width.h"
#include "GISUtils.h"
#include "XESConstants.h"
#include "WED_ToolUtils.h"


WED_GISLine_Width::WED_GISLine_Width(WED_Archive * parent, int id) : 
	WED_GISLine(parent, id),
	width(this,"width","GIS_lines_heading", "width", 50.0,6,2)
{
}

WED_GISLine_Width::~WED_GISLine_Width()
{
}

GISClass_t		WED_GISLine_Width::GetGISClass		(void				 ) const
{
	return gis_Line_Width;
}


void			WED_GISLine_Width::GetBounds		(	   Bbox2&  bounds) const
{
	CacheBuild();
	Point2 corners[4];
	GetCorners(corners);
	bounds = Bbox2(corners[0],corners[1]);
	bounds += corners[2];
	bounds += corners[3];
}

bool			WED_GISLine_Width::IntersectsBox	(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);
	return me.overlap(bounds);
}

bool			WED_GISLine_Width::WithinBox		(const Bbox2&  bounds) const
{
	Point2	corners[4];
	GetCorners(corners);
	return  bounds.contains(corners[0]) &&
			bounds.contains(corners[1]) &&
			bounds.contains(corners[2]) &&
			bounds.contains(corners[3]);
}

bool			WED_GISLine_Width::PtOnFrame		(const Point2& p, double dist) const
{
	Point2	corners[4];
	GetCorners(corners);
	if (Segment2(corners[0],corners[1]).is_near(p,dist)) return true;
	if (Segment2(corners[1],corners[2]).is_near(p,dist)) return true;
	if (Segment2(corners[2],corners[3]).is_near(p,dist)) return true;
	if (Segment2(corners[3],corners[0]).is_near(p,dist)) return true;
	return false;
}

bool			WED_GISLine_Width::PtWithin		(const Point2& p	 ) const
{
	Point2 corners[4];
	GetCorners(corners);	
	return inside_polygon_pt(corners,corners+4,p);
}

void			WED_GISLine_Width::Rescale(
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
	
	Point2	 ends[2];
	double  w;
	Quad_4to2(corners, ends, w);
	GetSource()->SetLocation(ends[0]);
	GetTarget()->SetLocation(ends[1]);
	SetWidth(w);
	
}






double	WED_GISLine_Width::GetWidth (void		 ) const
{
	return width.value;
}

void	WED_GISLine_Width::SetWidth (double w)
{
	if (w < 0.0) w = 0.0;
	if (w != width.value)
	{
		StateChanged();
		width.value = w;
	}
}

void	WED_GISLine_Width::GetCorners(Point2 corners[4]) const
{
	Point2		ends[2];
	GetSource()->GetLocation(ends[0]);
	GetTarget()->GetLocation(ends[1]);
	
	Quad_2to4(ends, GetWidth(), corners);
}

void	WED_GISLine_Width::MoveCorner(int corner, const Vector2& delta)
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
	Quad_1to2(ctr, h, l, ends);

	GetSource()->SetLocation(ends[0]);
	GetTarget()->SetLocation(ends[1]);
}	


void	WED_GISLine_Width::MoveSide(int side, const Vector2& delta)
{
	Point2	ends[2];
	GetSource()->GetLocation(ends[0]);
	GetTarget()->GetLocation(ends[1]);
	double w = GetWidth();
	
	Quad_MoveSide2(ends, w, side, delta);

	SetWidth(w);
	GetSource()->SetLocation(ends[0]);
	GetTarget()->SetLocation(ends[1]);
}

void	WED_GISLine_Width::ResizeSide(int side, const Vector2& delta, bool symetric)
{
	Point2	ends[2], corners[4];
	double	width;
	
	GetCorners(corners);
	Quad_ResizeSide4(corners, side, delta, symetric);
	Quad_4to2(corners, ends, width);

	GetSource()->SetLocation(ends[0]);
	GetTarget()->SetLocation(ends[1]);
	SetWidth(width);	
}

void	WED_GISLine_Width::ResizeCorner(int corner, const Vector2& delta, bool symetric)
{
	Point2	ctr, ends[2];
	double	w = GetWidth(), h, l;
	GetSource()->GetLocation(ends[0]);
	GetTarget()->GetLocation(ends[1]);
	Quad_2to1(ends, ctr, h, l);
	Quad_ResizeCorner1(ctr, h, l, w, corner, delta, symetric);
	Quad_1to2(ctr, h, l, ends);
	GetSource()->SetLocation(ends[0]);
	GetTarget()->SetLocation(ends[1]);
	SetWidth(w);	
	
}

