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
	has_msl(this,PROP_Name("Elevation Mode", XML_Name("obj_placement","custom_msl")), ObjElevationType, 0),
	msl    (this,PROP_Name("Elevation",     XML_Name("obj_placement","msl")), 0, 5, 3),
	resource  (this,PROP_Name("Resource",  XML_Name("obj_placement","resource")),""),
	show_level(this,PROP_Name("Show with", XML_Name("obj_placement","show_level")), ShowLevel, show_Level1),
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

void		WED_ObjPlacement::SetHeading(double h)
{
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(GetArchive()->GetResolver());
	if(rmgr)
	{
		const XObj8 * o;
//			int n = GetNumVariants(resource.value);   // no need to cycle through these - only the first variant is used for preview
		if(rmgr->GetObj(resource.value,o))
			if(o->fixed_heading >= 0.0)
				h = o->fixed_heading;
	}
	WED_GISPoint_Heading::SetHeading(h);
}

void	WED_ObjPlacement::Rotate(GISLayer_t l,const Point2& center, double angle)
{
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(GetArchive()->GetResolver());
	if(rmgr)
	{
		const XObj8 * o;
		if(rmgr->GetObj(resource.value,o))
			if(o->fixed_heading >= 0.0)
			{
				WED_GISPoint::Rotate(l,center,angle);
				return;
			}
	}
	WED_GISPoint_Heading::Rotate(l,center,angle);
}

double 	WED_ObjPlacement::GetVisibleDeg(void) const
{
	return visibleWithinDeg; // one note - 
}

bool		WED_ObjPlacement::Cull(const Bbox2& b) const
{

	// caching the objects dimension here for off-display culling in the map view. Its disregarding object rotation
	// and any lattitude dependency in the conversion, so the value will be choosen sufficiently pessimistic.

	if(visibleWithinDeg < 0.0)                            // so we only do this once for each object, ever
	{
		float * f = (float *) &visibleWithinDeg;          // tricking the compiler, breaking all rules. But Cull() must be const ...
		*f = GLOBAL_WED_ART_ASSET_FUDGE_FACTOR;           // the old, brain-dead visibility rule of thumb
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(GetArchive()->GetResolver());
		if(rmgr)
		{
			const XObj8 * o;
			agp_t agp;
			Point2	my_loc;
			GetLocation(gis_Geo,my_loc);
			double mtr_to_lon = MTR_TO_DEG_LAT / cos(my_loc.y() * DEG_TO_RAD);

//			int n = GetNumVariants(resource.value);   // no need to cycle through these - only the first variant is used for preview
			if(rmgr->GetObj(resource.value,o))
			{
				const XObj8 * o;
				agp_t agp;
				Point2	my_loc;
				GetLocation(gis_Geo,my_loc);
				double mtr_to_lon = MTR_TO_DEG_LAT / cos(my_loc.y() * DEG_TO_RAD);

	//			int n = GetNumVariants(resource.value);   // no need to cycle through these - only the first variant is used for preview
				if(rmgr->GetObj(resource.value,o))
				{
					*f =  pythag(max(fabs(o->xyz_max[0]), fabs(o->xyz_min[0])),max(fabs(o->xyz_max[2]), fabs(o->xyz_min[2]))) * 1.2 * mtr_to_lon;
				}
				else if(rmgr->GetAGP(resource.value,agp))
				{
					float min_xy[2] = {  999,  999 };
					float max_xy[2] = { -999, -999 };
					for(int n = 0; n < agp.tile.size(); n += 4)
					{
						min_xy[0] = min(min_xy[0], agp.tile[n]);
						max_xy[0] = max(max_xy[0], agp.tile[n]);
						min_xy[1] = min(min_xy[1], agp.tile[n+1]);
						max_xy[1] = max(max_xy[1], agp.tile[n+1]);
					}
					// we simplyfy a bit here, as we dont want to loop though where in the tile a given object is placed and how its rotated
               // so we guesstimate the objet is placed near the center of all tiles, but could be rotated any way.
               // Catches objects much larger than the tile, e.g. the long_row hangars in the airport library
					float cent_x = (min_xy[0] + max_xy[0]) * 0.5;
					float cent_y = (min_xy[1] + max_xy[1]) * 0.5;

					for(auto obj : agp.objs)
					{
						const XObj8 * agp_o;
						if (rmgr->GetObjRelative(obj.name, resource.value, agp_o))
						{
							float obj_rad = pythag(max(fabs(agp_o->xyz_max[0]),fabs(agp_o->xyz_min[0])), max(fabs(agp_o->xyz_max[2]), fabs(agp_o->xyz_min[2])));

							min_xy[0] = min(cent_x - obj_rad, min_xy[0]);
							max_xy[0] = max(cent_x + obj_rad, max_xy[0]);
							min_xy[1] = min(cent_y - obj_rad, min_xy[1]);
							max_xy[1] = max(cent_y + obj_rad, max_xy[1]);
						}
					}
					*f =  pythag(max(fabs(max_xy[0]), fabs(min_xy[0])),max(fabs(max_xy[1]), fabs(min_xy[1]))) * 1.2 * mtr_to_lon;
				}
			}
		}
	}

// This woould be a radical approach to culling - drop ALL visible items, like preview/structure/handles/highlight if its too small.
// Thois will make items completely invisible, but keeps thm selectable. Very similar as the "too small to go in" apprach where whole
// groups or airport drop out of view.
// Results in only a verw cases in users surprises - as large group of isolated items (like forests drawn as individual tree objects).
// This practive of drawing trees comoes a popular "bad paractive" for gateway airports now - which result in sceneries that do not scale 
// very well with rendering settings.
// This concept could be enhances by passing an additional parameter into the Cull() function to allow items to cull themselves differently
// based on structure/preview or even tool views.
//
//	if(visibleWithinDeg < b.yspan() * 0.002) return false;      // cull objects by size as well. Makes structure/preview and handles all disappear

	Point2	my_loc;
	GetLocation(gis_Geo,my_loc);

	Bbox2	my_bounds(my_loc - Vector2(visibleWithinDeg,visibleWithinDeg),
					  my_loc + Vector2(visibleWithinDeg,visibleWithinDeg));

	return b.overlap(my_bounds);
}

int		WED_ObjPlacement::HasCustomMSL(void) const
{
	return has_msl.value - obj_setToGround;
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


// the enum names for "Ground Level" and "set_MSL" are kept as "0" and "1" for xml backward compatibility.
// Translate those names here to keep this from confusing users

void	WED_ObjPlacement::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	WED_Thing::GetNthPropertyDict(n,dict);
	if(n == PropertyItemNumber(&has_msl))
	{
		dict[obj_setToGround] = make_pair("on Ground",true);
		dict[obj_setMSL] = make_pair("set_MSL",true);
	}
}

void	WED_ObjPlacement::GetNthPropertyDictItem(int n, int e, string& item) const
{
	if(n == PropertyItemNumber(&has_msl) && e <= obj_setMSL)
	{
		item = (e == obj_setToGround ? "on Ground" : "set_MSL");
	}
	else
		WED_Thing::GetNthPropertyDictItem(n,e,item);
}

void	WED_ObjPlacement::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	if (has_msl.value == obj_setToGround && n == PropertyItemNumber(&msl))
	{
		info.prop_name = "."; // Do not show elevation property if its not relevant
	}
	else
		WED_Thing::GetNthPropertyInfo(n, info);
}

