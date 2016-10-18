/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_PolygonPlacement.h"

DEFINE_PERSISTENT(WED_PolygonPlacement)
TRIVIAL_COPY(WED_PolygonPlacement,WED_GISPolygon)

WED_PolygonPlacement::WED_PolygonPlacement(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	heading(this,"Heading",  XML_Name("polygon_placement","heading"),10.0,3,1),
	resource(this,"Resource",XML_Name("polygon_placement","resource"),"")
{
}

WED_PolygonPlacement::~WED_PolygonPlacement()
{
}

double WED_PolygonPlacement::GetHeading(void) const
{
	return heading.value;
}

void WED_PolygonPlacement::SetHeading(double h)
{
	heading = h;
}

void		WED_PolygonPlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_PolygonPlacement::SetResource(const string& r)
{
	resource = r;
}

