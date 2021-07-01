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
#include "WED_FacadeRing.h"
#include "WED_FacadeNode.h"
#include "WED_ResourceMgr.h"
#include "WED_EnumSystem.h"
#include "WED_ToolUtils.h"
#include "AptDefs.h"
#include "GISUtils.h"
#include "XESConstants.h"

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
#if WED
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
#endif
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

int		WED_FacadePlacement::GetType(void) const
{
	const fac_info_t * f = GetFacInfo();
	if(f)
	{
		if(f->is_new) return 2;
		else         return 1;
	}
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

bool		WED_FacadePlacement::IsJetway(int * cabin, int * tunnel) const
{
	const fac_info_t * f = GetFacInfo();
	if (f->tunnels.size())
	{
		if (cabin) *cabin = f->cabin_idx;
		if (tunnel) *tunnel = f->tunnels.front().idx;
		return true;
	}
	else
		return false;
}


bool		WED_FacadePlacement::HasDockingCabin(void) const
{
	if (gExportTarget < wet_xplane_1200) return false;

	const fac_info_t * f = GetFacInfo();
	if (f->tunnels.empty()) return false;

	auto ps = GetOuterRing();
	int n_pts = ps->GetNumPoints();
	if (n_pts >= 3)
	{
		Point2 pt;
		if (auto end = ps->GetNthPoint(n_pts - 1))
		{
			end->GetLocation(gis_Param, pt);
			if (pt.x() < 39.0)          // WED 2.0-2.4 define enums for walls 0-39
			{
				if (auto cabin = ps->GetNthPoint(n_pts - 2))
				{
					cabin->GetLocation(gis_Param, pt);
					if (pt.x() == f->cabin_idx)
					{
						if (auto tunnel = ps->GetNthPoint(n_pts - 3))
						{
							tunnel->GetLocation(gis_Param, pt);
							for(auto t : f->tunnels)
								if (pt.x() == t.idx)
									return true;
						}
					}
				}
			}
		}
	}
	return false;
}

void		WED_FacadePlacement::ExportJetway(Jetway_t& jetway)
{
	auto ps = GetOuterRing();
	int n_pts = ps->GetNumPoints();
	if (n_pts >= 3)
	{
		auto tunnel = ps->GetNthPoint(n_pts - 3);
		auto cabin = ps->GetNthPoint(n_pts - 2);
		auto c_dir = ps->GetNthPoint(n_pts - 1);

		const fac_info_t * f = GetFacInfo();
		if (f->tunnels.size())
		{
			Point2 loc, cabin_loc, type;
			tunnel->GetLocation(gis_Geo, loc);
			jetway.location = loc;

			cabin->GetLocation(gis_Geo, cabin_loc);
			jetway.install_heading = VectorDegs2NorthHeading(loc, loc, Vector2(loc, cabin_loc));
			jetway.parked_tunnel_length = LonLatDistMeters(loc, cabin_loc);
			jetway.parked_tunnel_angle = jetway.install_heading;

			tunnel->GetLocation(gis_Param, type);
			for (auto t : f->tunnels)
			{
				if (type.x() == t.idx)
				{
					jetway.size_code = t.size_code;
					break;
				}
			}

			c_dir->GetLocation(gis_Geo, loc);
//			jetway.parked_cab_angle = jetway.install_heading - VectorDegs2NorthHeading(cabin_loc, cabin_loc, Vector2(cabin_loc, loc));
			jetway.parked_cab_angle = VectorDegs2NorthHeading(cabin_loc, cabin_loc, Vector2(cabin_loc, loc));

			jetway.style_code = f->style_code;
		}
	}
}

void		WED_FacadePlacement::ImportJetway(const Jetway_t& apt_data, void(*print_func)(void *, const char *, ...), void * ref)
{
	WED_FacadeRing * ring;
	if (GetNumEntities() == 0)         // in case this is already a complete facade - switch to appending only three new nodes
	{
		SetCustomWalls(1);
		SetHeight(1);
		SetShowLevel(1);

		switch (apt_data.style_code)
		{
			case 0: SetResource("lib/airport/Ramp_Equipment/Jetways/Jetway_1_glass.fac"); break;
			case 1: SetResource("lib/airport/Ramp_Equipment/Jetways/Jetway_1_solid.fac"); break;
			case 2: SetResource("lib/airport/Ramp_Equipment/Jetways/Jetway_2_glass.fac"); break;
			case 3: SetResource("lib/airport/Ramp_Equipment/Jetways/Jetway_1_solid.fac"); break;
		}
		ring = WED_FacadeRing::CreateTyped(GetArchive());
		ring->SetParent(this, 0);
		ring->SetName("Ring");
	}
	else
		ring = dynamic_cast<WED_FacadeRing *>(GetNthEntity(0));

	const fac_info_t * f = GetFacInfo();

	WED_FacadeNode * p_tunnel;
	if (ring->GetNumEntities() == 0)
	{
		p_tunnel = WED_FacadeNode::CreateTyped(GetArchive());
		p_tunnel->SetParent(ring, 0);
		p_tunnel->SetName("Tunnel Node");
	}
	else
		p_tunnel = dynamic_cast<WED_FacadeNode *>(GetNthEntity(0));
	if(f && f->tunnels.size())
		p_tunnel->SetWallType(f->tunnels[apt_data.size_code].idx);
	else
		p_tunnel->SetWallType(4 + apt_data.size_code); // best guess - thats what the initial facades used
	p_tunnel->SetLocation(gis_Geo, apt_data.location); // in case we append to a facade - this forces the existing last node to be exactly where the
	                                                   // jetway starts. Maybe check if that is at least somewhere "close" and abort if not ?

	auto p_cabin = WED_FacadeNode::CreateTyped(GetArchive());
	p_cabin->SetParent(ring, p_tunnel->GetMyPosition() + 1);
	p_cabin->SetName("Cabin Node");
	if(f && f->tunnels.size())
		p_cabin->SetWallType(f->cabin_idx);
	else
		p_cabin->SetWallType(3);                      // best guess - thats what the initial facades used
	Vector2 dir;
	NorthHeading2VectorDegs(apt_data.location, apt_data.location, apt_data.install_heading + apt_data.parked_tunnel_angle, dir);
	Point2 pt(apt_data.location);
	pt += dir * apt_data.parked_tunnel_length * MTR_TO_DEG_LAT;
	p_cabin->SetLocation(gis_Geo, pt);

	auto p_end = WED_FacadeNode::CreateTyped(GetArchive());
	p_end->SetParent(ring, p_cabin->GetMyPosition() + 1);
	p_end->SetName("End Node");
	p_end->SetWallType(0);
	NorthHeading2VectorDegs(apt_data.location, apt_data.location, apt_data.install_heading + apt_data.parked_tunnel_angle - apt_data.parked_cab_angle, dir);
	pt += dir * 10.0 * MTR_TO_DEG_LAT;
	p_end->SetLocation(gis_Geo, pt);
}
