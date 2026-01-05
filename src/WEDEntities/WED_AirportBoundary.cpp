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

#include "WED_AirportBoundary.h"

#include "AptDefs.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_AirportBoundary)
TRIVIAL_COPY(WED_AirportBoundary, WED_GISPolygon)

WED_AirportBoundary::WED_AirportBoundary(WED_Archive * a, int i) : WED_GISPolygon(a, i),
lines(this, PROP_Name("Line Attributes", XML_Name("", "")), "Line Attributes", 1),
lights(this, PROP_Name("Light Attributes", XML_Name("", "")), "Light Attributes", 1)
#if HAS_BDY_TYPES
, types(this, PROP_Name("Type", XML_Name("boundary", "type")), BoundaryType, 0)
{
	DOMAIN_Members(BoundaryType, types);
}
#else
{
}
#endif

WED_AirportBoundary::~WED_AirportBoundary()
{
}

void WED_AirportBoundary::Import(const AptBoundary_t& x, void (*print_func)(void*, const char*, ...), void* ref)
{
	SetName(x.name);
#if HAS_BDY_TYPES
	// todo: deal with types
	DOMAIN_Members(BoundaryType, types);
#endif
}

void WED_AirportBoundary::Export(		 AptBoundary_t& x) const
{
	GetName(x.name);
	// if(types.value.empty()) DOMAIN_Members(BoundaryType, types);
	// todo:  how to spec in apt.dat ? new props, new row codes ...
}

#if HAS_BDY_TYPES
set<int> WED_AirportBoundary::GetType() const
{
	return types.value;
}
#endif