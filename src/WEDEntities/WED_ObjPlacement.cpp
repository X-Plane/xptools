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
#include "WED_ResourceMgr.h"
#include "WED_ToolUtils.h"
#include "XObjDefs.h"
#include "MathUtils.h"
#include "CompGeomDefs2.h"
#include "XESConstants.h"

DEFINE_PERSISTENT(WED_ObjPlacement)
TRIVIAL_COPY(WED_ObjPlacement,WED_GISPoint_Heading)

WED_ObjPlacement::WED_ObjPlacement(WED_Archive * a, int i) : 
	WED_GISPoint_Heading(a,i),
#if AIRPORT_ROUTING
	has_msl(this,PROP_Name("Set MSL", XML_Name("obj_placement","custom_msl")),0),
	msl    (this,PROP_Name("MSL",     XML_Name("obj_placement","msl")), 0, 5,3),
#endif
	resource  (this,PROP_Name("Resource",  XML_Name("obj_placement","resource")),""),
	show_level(this,PROP_Name("Show with", XML_Name("obj_placement","show_level")),ShowLevel, show_Level1),
	visibleWithinDeg(-1.0)
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
	visibleWithinDeg = -1.0; // force re-evaluation when object is changed
}

bool		WED_ObjPlacement::Cull(const Bbox2& b) const
{
	Point2	my_loc;
	GetLocation(gis_Geo,my_loc);

	// caching the objects dimension here for off-display culling in the map view. Its disregarding object rotation
	// and any lattitude dependency in the conversion, so the value will be choosen sufficiently pessimistic.

	if(visibleWithinDeg < 0.0)                            // so we only do this once for each object, ever
	{
		float * f = (float *) &visibleWithinDeg;          // tricking the compiler, breaking all rules. But Cull() must be const ...
		*f = GLOBAL_WED_ART_ASSET_FUDGE_FACTOR;           // the old, brain-dead visibility rule of thumb
		IResolver * res = GetArchive()->GetResolver();
		if(res)
		{
			WED_ResourceMgr * rmgr = WED_GetResourceMgr(res);
			if(rmgr)
			{
				XObj8 * o;
				agp_t agp;
	//			int n = GetNumVariants(resource.value);   // no need to cycle through these - only the first variant is used for preview
				if(rmgr->GetObj(resource.value,o))
				{
					double x_max = max(fabs(o->xyz_max[0]), fabs(o->xyz_min[0]));
					double y_max = max(fabs(o->xyz_max[2]), fabs(o->xyz_min[2]));
					*f = pythag(x_max,y_max) * MTR_TO_DEG_LAT * 3.0;      // pessimistic at least up to 70 deg lattitude
				}
				else if(rmgr->GetAGP(resource.value,agp))
				{
					double x_max = 0.0;
					double y_max = 0.0;
					for(int n = 0; n < agp.tile.size(); n += 4)
					{
						x_max = max(x_max,fabs(agp.tile[n]));
						y_max = max(y_max,fabs(agp.tile[n+1]));
					}
					*f = pythag(x_max,y_max) * MTR_TO_DEG_LAT * 3.0;      // pessimistic at least up to 70 deg lattitude
				}
			}
		}
	}

	Bbox2	my_bounds(my_loc - Vector2(visibleWithinDeg,visibleWithinDeg), 
					  my_loc + Vector2(visibleWithinDeg,visibleWithinDeg));

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
