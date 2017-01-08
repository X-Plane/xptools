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

#include "WED_Taxiway.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
DEFINE_PERSISTENT(WED_Taxiway)
TRIVIAL_COPY(WED_Taxiway, WED_GISPolygon)

WED_Taxiway::WED_Taxiway(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	surface(this,   "Surface",          XML_Name("taxiway",	"surface"), Surface_Type, surf_Concrete),
	roughness(this, "Roughness",        XML_Name("taxiway",	"roughness"),0.25,4,2),
	heading(this,   "Texture Heading",  XML_Name("taxiway",	"heading"), 0,6,2),
	lines(this,     "Line Attributes",  XML_Name("",""),"Line Attributes", 1),
	lights(this,    "Light Attributes", XML_Name("",""),"Light Attributes", 1)
{
}

WED_Taxiway::~WED_Taxiway()
{
}

void		WED_Taxiway::SetSurface(int s)
{
		surface = s;
}

void		WED_Taxiway::SetRoughness(double r)
{
		roughness = r;
}

double		WED_Taxiway::GetHeading(void) const
{
	return heading.value;
}

void		WED_Taxiway::SetHeading(double h)
{
		heading = h;
}

int			WED_Taxiway::GetSurface(void) const
{
	return surface.value;
}

void		WED_Taxiway::Import(const AptTaxiway_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	surface = ENUM_Import(Surface_Type, x.surface_code);
	if (surface == -1)
	{
		print_func(ref,"Error importing taxiway: surface code %d is illegal (not a member of type %s).\n", x.surface_code, DOMAIN_Desc(surface.domain));
		surface = surf_Concrete;
	}

	roughness = x.roughness_ratio;
	heading = x.heading;
	SetName(x.name);
}

void		WED_Taxiway::Export(		 AptTaxiway_t& x) const
{
	x.surface_code = ENUM_Export(surface.value);
	x.roughness_ratio = roughness;
	x.heading = heading;
	GetName(x.name);
}

void	WED_Taxiway::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	WED_GISPolygon::GetNthPropertyDict(n, dict);
	if(n == PropertyItemNumber(&surface) && surface.value != surf_Water)
	{
		dict.erase(surf_Water);
	}
}
