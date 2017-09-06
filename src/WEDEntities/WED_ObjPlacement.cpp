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

#include "WED_ObjPlacement.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_ObjPlacement)
TRIVIAL_COPY(WED_ObjPlacement,WED_GISPoint_Heading)

WED_ObjPlacement::WED_ObjPlacement(WED_Archive * a, int i) : 
	WED_GISPoint_Heading(a,i),
#if AIRPORT_ROUTING
	has_msl(this,"Set MSL", XML_Name("obj_placement","custom_msl"),0),
	msl(this,"MSL",         XML_Name("obj_placement","msl"), 0, 5,3),
#endif
	resource(this,"Resource",   XML_Name("obj_placement","resource"),""),
	show_level(this,"Show with",XML_Name("obj_placement","show_level"),ShowLevel, show_Level1)

{
}

WED_ObjPlacement::~WED_ObjPlacement()
{
}

void		WED_ObjPlacement::SetShowLevel(int sl)
{
	show_level = ENUM_Import(ShowLevel,sl);
}

int			WED_ObjPlacement::GetShowLevel(void) const
{
	return ENUM_Export(show_level.value);
}

void		WED_ObjPlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_ObjPlacement::SetResource(const string& r)
{
	resource = r;
}

bool		WED_ObjPlacement::Cull(const Bbox2& b) const
{
	Point2	my_loc;
	GetLocation(gis_Geo,my_loc);

	// 20 km - if your OBJ is bigger than that, you are REALLY doing it wrong.
	// In fact, if your OBJ is bigger than 5 km you're doing it wrong.
	Bbox2	my_bounds(my_loc - Vector2(GLOBAL_WED_ART_ASSET_FUDGE_FACTOR,GLOBAL_WED_ART_ASSET_FUDGE_FACTOR), 
					  my_loc + Vector2(GLOBAL_WED_ART_ASSET_FUDGE_FACTOR, GLOBAL_WED_ART_ASSET_FUDGE_FACTOR));	
	return b.overlap(my_bounds);
}

#if AIRPORT_ROUTING
bool		WED_ObjPlacement::HasCustomMSL(void) const
{
	return has_msl.value;
}

double		WED_ObjPlacement::GetCustomMSL(void) const
{
	return msl.value;
}

void		WED_ObjPlacement::SetCustomMSL(double in_msl)
{
	has_msl = 1;
	msl = in_msl;
}

void		WED_ObjPlacement::SetDefaultMSL(void)
{
	has_msl = 0;
}

#endif
