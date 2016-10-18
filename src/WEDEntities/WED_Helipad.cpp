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

#include "WED_Helipad.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_Helipad)
TRIVIAL_COPY(WED_Helipad, WED_GISPoint_HeadingWidthLength)

WED_Helipad::WED_Helipad(WED_Archive * a, int i) : WED_GISPoint_HeadingWidthLength(a,i),

	surface(this,"Surface",		XML_Name("helipad","surface"),	Surface_Type,surf_Concrete),
	markings(this,"Markings",	XML_Name("helipad","markings"),	Helipad_Markings,heli_Mark_Default),
	shoulder(this,"Shoulder",	XML_Name("helipad","shoulder"),	Shoulder_Type,shoulder_None),
	roughness(this,"Roughness",	XML_Name("helipad","roughness"),0.25,4,2),
	edgelights(this,"Lights",	XML_Name("helipad","lights"),	Heli_Lights,heli_Yellow)
{
}

WED_Helipad::~WED_Helipad()
{
}

void		WED_Helipad::SetSurface(int x)
{
	surface = x;
}

void		WED_Helipad::SetMarkings(int x)
{
	markings = x;
}

void		WED_Helipad::SetShoulder(int x)
{
	shoulder = x;
}

void		WED_Helipad::SetRoughness(double x)
{
	roughness = x;
}

void		WED_Helipad::SetEdgeLights(int x)
{
	edgelights = x;
}

int			WED_Helipad::GetSurface(void) const
{
	return surface.value;
}

void	WED_Helipad::Import(const AptHelipad_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(x.id);
	SetLocation(gis_Geo,x.location);
	SetHeading(x.heading);
	SetWidth(x.width_mtr);
	SetLength(x.length_mtr);
	surface = ENUM_Import(Surface_Type, x.surface_code);
	if (surface == -1)
	{
		print_func(ref,"Error importing helipad: surface code %d is illegal (not a member of type %s).\n", x.surface_code, DOMAIN_Desc(surface.domain));
		surface = surf_Concrete;
	}
	markings = ENUM_Import(Helipad_Markings, x.marking_code);
	if (markings == -1)
	{
		print_func(ref,"Error importing helipad: markings code %d is illegal (not a member of type %s).\n", x.marking_code, DOMAIN_Desc(markings.domain));
		markings = heli_Mark_Default;
	}
	shoulder = ENUM_Import(Shoulder_Type, x.shoulder_code);
	if (shoulder == -1)
	{
		print_func(ref,"Error importing helipad: shoulder code %d is illegal (not a member of type %s).\n", x.shoulder_code, DOMAIN_Desc(shoulder.domain));
		shoulder = shoulder_None;
	}
	edgelights = ENUM_Import(Heli_Lights, x.edge_light_code);
	if (edgelights == -1)
	{
		print_func(ref,"Error importing helipad: edge-lights code %d is illegal (not a member of type %s).\n", x.edge_light_code, DOMAIN_Desc(edgelights.domain));
		edgelights = heli_Yellow;
	}
	roughness = x.roughness_ratio;
}


void	WED_Helipad::Export(		 AptHelipad_t& x) const
{
	GetName(x.id);
	GetLocation(gis_Geo,x.location);
	x.heading = GetHeading();
	x.width_mtr = GetWidth();
	x.length_mtr = GetLength();
	x.surface_code = ENUM_Export(surface.value);
	x.marking_code = ENUM_Export(markings.value);
	x.shoulder_code = ENUM_Export(shoulder.value);
	x.roughness_ratio = roughness;
	x.edge_light_code = ENUM_Export(edgelights.value);

}

