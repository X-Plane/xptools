#include "WED_GISPoint_HeadingWidthLength.h"

#include "IODefs.h"
#include "SQLUtils.h"
#include "GISUtils.h"
#include "WED_Errors.h"



WED_GISPoint_HeadingWidthLength::WED_GISPoint_HeadingWidthLength(WED_Archive * parent, int id) :
	WED_GISPoint_Heading(parent, id),
	width(this,"width","GIS_points_headingwidthlength", "width",1.0),
	length(this,"length","GIS_points_headingwidthlength", "length",1.0)
{
}

WED_GISPoint_HeadingWidthLength::~WED_GISPoint_HeadingWidthLength()
{
}

GISClass_t	WED_GISPoint_HeadingWidthLength::GetGISClass		(void				 ) const
{
	return gis_Point_HeadingWidthLength;
}

bool			WED_GISPoint_HeadingWidthLength::PtWithin		(const Point2& p	 ) const
{
	Point2 corners[4];
	GetCorners(corners);	
	return inside_polygon_pt(corners,corners+4,p);
}

double	WED_GISPoint_HeadingWidthLength::GetWidth (void		 ) const
{
	return width.value;
}

void	WED_GISPoint_HeadingWidthLength::SetWidth (double w)
{
	if (w != width.value)
	{
		StateChanged();
		width.value = w;
	}
}

double	WED_GISPoint_HeadingWidthLength::GetLength(void		 ) const
{
	return length.value;
}

void	WED_GISPoint_HeadingWidthLength::SetLength(double l)
{
	if (l != length.value)
	{
		StateChanged();
		length.value = l;
	}
}

void	WED_GISPoint_HeadingWidthLength::GetCorners(Point2 corners[4]) const
{
	Vector2		dir;
	Point2		center;
	GetLocation(center);
	
	NorthHeading2VectorMeters(center, center, GetHeading(),dir);	
	dir.normalize();
	Vector2 right(dir.perpendicular_cw());
	
	Point2	zero(0,0);
	
	corners[0] = zero - dir * GetLength() * 0.5 - right * GetWidth() * 0.5;
	corners[1] = zero + dir * GetLength() * 0.5 - right * GetWidth() * 0.5;
	corners[2] = zero + dir * GetLength() * 0.5 + right * GetWidth() * 0.5;
	corners[3] = zero - dir * GetLength() * 0.5 + right * GetWidth() * 0.5;
	
	MetersToLLE(center, 4, corners);
}
