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

#include "WED_GISPoint_Heading.h"
#include "IODefs.h"
#include "WED_Errors.h"

TRIVIAL_COPY(WED_GISPoint_Heading, WED_GISPoint)

WED_GISPoint_Heading::WED_GISPoint_Heading(WED_Archive * parent, int id) :
	WED_GISPoint(parent, id),
	heading(this,"heading", XML_Name("point","heading"),0.0,6,2)
{
}

WED_GISPoint_Heading::~WED_GISPoint_Heading()
{
}

GISClass_t		WED_GISPoint_Heading::GetGISClass		(void				 ) const
{
	return gis_Point_Heading;
}

double	WED_GISPoint_Heading::GetHeading(void			) const
{
	return heading.value;
}

void	WED_GISPoint_Heading::SetHeading(double h)
{
	if (h != heading.value)
	{
		StateChanged();
		heading.value = h;
		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);
	}
}

void	WED_GISPoint_Heading::Rotate			(GISLayer_t l,const Point2& center, double angle)
{
	WED_GISPoint::Rotate(l,center,angle);
	if(l == gis_Geo)
	heading = heading.value + angle;
}
