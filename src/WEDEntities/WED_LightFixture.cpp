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

#include "WED_LightFixture.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_LightFixture)
TRIVIAL_COPY(WED_LightFixture, WED_GISPoint_Heading)


WED_LightFixture::WED_LightFixture(WED_Archive * a, int i) : WED_GISPoint_Heading(a, i),
	light_type(this, "Type", XML_Name("light_fixture","type"),Light_Fixt, light_VASI),
	angle(this,"Angle",      XML_Name("light_fixture","angle"),3.0,4,2)
{
}

WED_LightFixture::~WED_LightFixture()
{
}

void	WED_LightFixture::SetLightType(int x)
{
	light_type = x;
}

void	WED_LightFixture::SetAngle(double x)
{
	angle = x;
}

void	WED_LightFixture::Import(const AptLight_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(gis_Geo,x.location);
	SetHeading(x.heading <= 180.0 ? x.heading + 180.0 : x.heading - 180.0);
	angle = x.angle;
	light_type = ENUM_Import(Light_Fixt, x.light_code);
	if (light_type == -1)
	{
		print_func(ref,"Error importing light fixture: light type code %d is illegal (not a member of type %s).\n", x.light_code, DOMAIN_Desc(light_type.domain));
		light_type = light_VASI;
	}

	SetName(x.name);
}

void	WED_LightFixture::Export(		 AptLight_t& x) const
{
	GetLocation(gis_Geo,x.location);
	x.heading = GetHeading();
	x.heading += 180.0;
	if (x.heading > 360.0) x.heading -= 360.0;

	x.angle = angle;
	x.light_code = ENUM_Export(light_type.value);
	GetName(x.name);
}
