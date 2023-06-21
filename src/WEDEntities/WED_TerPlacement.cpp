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

#include "WED_TerPlacement.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_TerPlacement)
TRIVIAL_COPY(WED_TerPlacement,WED_GISPolygon)

WED_TerPlacement::WED_TerPlacement(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	resource  (this, PROP_Name("Resource",       XML_Name("ter_placement", "Resource")), ""),
	derez(this, PROP_Name("Derez Factor", XML_Name("ter_placement", "derez")), 1, 3, 1, "x"),
	skirt(this, PROP_Name("Skirt Depth", XML_Name("ter_placement", "skirt")), 1, 3, 0),
	show_level(this, PROP_Name("Show with",      XML_Name("ter_placement", "show_level")), ShowLevel, show_Level1),
	has_msl   (this, PROP_Name("Elevation Mode", XML_Name("ter_placement", "custom_msl")), TerElevationType, obj_setMSL),
	msl       (this, PROP_Name("Elevation",      XML_Name("ter_placement", "msl")), 0, 5, 2)
{
}

WED_TerPlacement::~WED_TerPlacement()
{
}

void	WED_TerPlacement::GetResource(string& r) const
{
	r = resource.value;
}

void	WED_TerPlacement::SetResource(const string& r)
{
	resource = r;
}

int		WED_TerPlacement::GetShowLevel(void) const
{
	return ENUM_Export(show_level.value);
}

int		WED_TerPlacement::GetMSLType(void) const
{
	return ENUM_Export(has_msl.value);
}

double	WED_TerPlacement::GetCustomMSL(void) const
{
	return msl.value;
}

double WED_TerPlacement::GetSamplingFactor(void) const
{
	return derez.value;
}

double WED_TerPlacement::GetSkirtDepth(void) const
{
	return skirt.value;
}

