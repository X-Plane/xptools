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

#include "WED_FacadePlacement.h"
#include "WED_ResourceMgr.h"
#include "WED_EnumSystem.h"
#include "WED_ToolUtils.h"
#include "WED_FacadeNode.h"

DEFINE_PERSISTENT(WED_FacadePlacement)
TRIVIAL_COPY(WED_FacadePlacement,WED_GISPolygon)

WED_FacadePlacement::WED_FacadePlacement(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	height    (this,PROP_Name("Height",    XML_Name("facade_placement","height")),10,3,0,"m"),
	pick_walls(this,PROP_Name("Pick Walls",XML_Name("facade_placement","pick_walls")),0),
	resource  (this,PROP_Name("Resource",  XML_Name("facade_placement","resource")),""),
	show_level(this,PROP_Name("Show with", XML_Name("facade_placement","show_level")),ShowLevel, show_Level1)
{
}

WED_FacadePlacement::~WED_FacadePlacement()
{
}

double WED_FacadePlacement::GetHeight(void) const
{
	return height.value;
}

void WED_FacadePlacement::SetHeight(double h)
{
	height = h;
}

void		WED_FacadePlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_FacadePlacement::SetResource(const string& r)
{
	resource = r;
}

void		WED_FacadePlacement::SetShowLevel(int sl)
{
	show_level = ENUM_Import(ShowLevel,sl);
}

int			WED_FacadePlacement::GetShowLevel(void) const
{
	return ENUM_Export(show_level.value);
}

const fac_info_t * WED_FacadePlacement::GetFacInfo(void) const
{
	IResolver * r = GetArchive()->GetResolver();
	if(r)
	{
		WED_ResourceMgr* rr = WED_GetResourceMgr(r);
		if(rr)
		{
			const fac_info_t * f;
			if(rr->GetFac(resource.value,f))
				return f;
		}
	}
	return nullptr;
}

WED_FacadePlacement::TopoMode		WED_FacadePlacement::GetTopoMode(void) const 
{
	const fac_info_t * f = GetFacInfo();
	if(f)
	{
		if(f->has_roof)
			return topo_Area;   // ring and roof
		else
		{
			if(f->is_ring)
				return topo_Ring;   // ring only: no roof
			else
				return topo_Chain;  // no ring, no roof
		}
	}
	return topo_Area;
}

int		WED_FacadePlacement::GetNumWallChoices(void) const
{
	const fac_info_t * f = GetFacInfo();
	if(f)
		return  f->wallName.size();

	return 0;
}


bool		WED_FacadePlacement::HasLayer		(GISLayer_t layer							  ) const
{
	if(layer == gis_Param)	return pick_walls.value;
	return					WED_GISPolygon::HasLayer(layer);
}

void		WED_FacadePlacement::SetCustomWalls(bool has) 
{
	pick_walls = (has ? 1 : 0);
}

bool		WED_FacadePlacement::HasCustomWalls(void) const 
{
		return pick_walls.value; 
}
