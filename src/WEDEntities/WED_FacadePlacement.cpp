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
	height(this,"Height",       XML_Name("facade_placement","height"),10.0,3,1),
#if AIRPORT_ROUTING
	pick_walls(this,"Pick Walls",XML_Name("facade_placement","pick_walls"),0),
#endif	
	resource(this,"Resource",   XML_Name("facade_placement","resource"),""),
	show_level(this,"Show with",XML_Name("facade_placement","show_level"),ShowLevel, show_Level1)
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


WED_FacadePlacement::TopoMode		WED_FacadePlacement::GetTopoMode(void) const 
{
	IResolver * r = GetArchive()->GetResolver();
	if(r)
	{
		WED_ResourceMgr* rr = WED_GetResourceMgr(r);
		if(rr)
		{
			fac_info_t f;
			if(rr->GetFac(resource.value,f))
			{
				if(f.roof) return topo_Area;
				return f.ring ? topo_Ring : topo_Chain;
			}
		}
	}
	return topo_Area;
}

//void		WED_FacadePlacement::GetWallChoices(vector<int>& out_walls)
//{
//	out_walls.clear();
//	if (pick_walls.value)
//	{
//		for(int h = -1; h < GetNumHoles(); ++h)
//		{
//			IGISPointSequence * s = (h == -1) ? GetOuterRing() : GetNthHole(h);
//			for(int n = 0; n < s->GetNumSides(); ++n)
//			{
//				IGISPoint * p = s->GetNthPoint(n);
//				WED_FacadeNode * n = dynamic_cast<WED_FacadeNode *>(p);
//				if(n)
//					out_walls.push_back(n->GetWallType());
//			}
//		}
//	}
//}

#if AIRPORT_ROUTING
bool		WED_FacadePlacement::HasLayer		(GISLayer_t layer							  ) const
{
	if(layer == gis_Param)	return pick_walls.value;
	return					WED_GISPolygon::HasLayer(layer);
}

void		WED_FacadePlacement::SetCustomWalls(bool has) 
{
	pick_walls = (has ? 1 : 0);
}

#endif

bool		WED_FacadePlacement::HasCustomWalls(void) const 
{
	#if AIRPORT_ROUTING
		return pick_walls.value; 
	#else
		return false;
	#endif
}
