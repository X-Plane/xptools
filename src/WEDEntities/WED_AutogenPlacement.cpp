/*
 * Copyright (c) 2020, Laminar Research.
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

#include "WED_AutogenPlacement.h"

DEFINE_PERSISTENT(WED_AutogenPlacement)
TRIVIAL_COPY(WED_AutogenPlacement,WED_GISPolygon)

WED_AutogenPlacement::WED_AutogenPlacement(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	height  (this,PROP_Name("Height",          XML_Name("autogen_placement","height")),10.0,3,1),
	spelling(this,PROP_Name("Spelling",        XML_Name("autogen_placement","spelling")),0,3),
	resource(this,PROP_Name("Resource",        XML_Name("autogen_placement","resource")), "")
{
}

WED_AutogenPlacement::~WED_AutogenPlacement()
{
}

void	WED_AutogenPlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

bool	WED_AutogenPlacement::IsAGBlock(void) const
{
	return resource.value.back() == 'b';
}

void	WED_AutogenPlacement::SetResource(const string& r)
{
	resource = r;
}

double	WED_AutogenPlacement::GetHeight(void) const
{
	return height.value;
}

void	WED_AutogenPlacement::SetHeight(double h)
{
	height = h;
}

int 	WED_AutogenPlacement::GetSpelling(void) const
{
	return spelling.value;
}

void	WED_AutogenPlacement::SetSpelling(int s)
{
	spelling = s;
}

void	WED_AutogenPlacement::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	if (!IsAGBlock() && n == PropertyItemNumber(&spelling))
		info.prop_name = "."; // Do not show elevation property if its not relevant
	else
		WED_Thing::GetNthPropertyInfo(n, info);
}
