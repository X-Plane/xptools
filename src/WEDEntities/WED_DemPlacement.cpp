/*
 * Copyright (c) 2023, Laminar Research.
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

#include "WED_DemPlacement.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_DemPlacement)
TRIVIAL_COPY(WED_DemPlacement,WED_GISPolygon)

WED_DemPlacement::WED_DemPlacement(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	resource(this,   PROP_Name("Resource",  XML_Name("dem_placement", "Resource")), ""),
	show_level(this, PROP_Name("Show with", XML_Name("dem_placement", "show_level")), ShowLevel, show_Level1)
{
}

WED_DemPlacement::~WED_DemPlacement()
{
}

void		WED_DemPlacement::GetResource(string& r) const
{
	r = resource.value;
}

void		WED_DemPlacement::SetResource(const string& r)
{
	resource = r;
}
int			WED_DemPlacement::GetShowLevel(void) const
{
	return ENUM_Export(show_level.value);
}
