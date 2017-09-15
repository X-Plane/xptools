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

#include "WED_ForestPlacement.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_ForestPlacement)
TRIVIAL_COPY(WED_ForestPlacement,WED_GISPolygon)

WED_ForestPlacement::WED_ForestPlacement(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	density(this,"Density",     XML_Name("forest_placement","density"),10.0,3,1),
#if AIRPORT_ROUTING
	fill_mode(this,"Fill Mode", XML_Name("forest_placement","closed"),ForestFill, forest_Fill),
#endif	
	resource(this,"Resource",   XML_Name("forest_placement","resource"), "")
{
}

WED_ForestPlacement::~WED_ForestPlacement()
{
}

double WED_ForestPlacement::GetDensity(void) const
{
	return density.value;
}

void WED_ForestPlacement::SetDensity(double h)
{
	density = h;
}

void		WED_ForestPlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_ForestPlacement::SetResource(const string& r)
{
	resource = r;
}

int			WED_ForestPlacement::GetFillMode(void) const
{
#if AIRPORT_ROUTING
	return ENUM_Export(fill_mode.value);
#else
	return 0;
#endif
}

#if AIRPORT_ROUTING
void		WED_ForestPlacement::SetFillMode(int mode)
{
	fill_mode = ENUM_Import(ForestFill, mode);
}
#endif