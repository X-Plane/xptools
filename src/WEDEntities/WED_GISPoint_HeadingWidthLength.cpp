#include "WED_GISPoint_HeadingWidthLength.h"

#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

START_CASTING(WED_GISPoint_HeadingWidthLength)
IMPLEMENTS_INTERFACE(IGISEntity)
IMPLEMENTS_INTERFACE(IGISPoint)
IMPLEMENTS_INTERFACE(IGISPoint_Heading)
IMPLEMENTS_INTERFACE(IGISPoint_WidthLength)
INHERITS_FROM(WED_GISPoint_Heading)
END_CASTING

WED_GISPoint_HeadingWidthLength::WED_GISPoint_HeadingWidthLength(WED_Archive * parent, int id) :
	WED_GISPoint_Heading(parent, id),
	width(this,"width","GIS_points_headingwidthlength", "width",1.0),
	length(this,"length","GIS_points_headingwidthlength", "length",1.0)
{
}

WED_GISPoint_HeadingWidthLength::~WED_GISPoint_HeadingWidthLength()
{
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

