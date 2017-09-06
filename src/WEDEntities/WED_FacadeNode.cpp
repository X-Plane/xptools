/* 
 * Copyright (c) 2011, Laminar Research.
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

#include "WED_FacadeNode.h"
#include "WED_EnumSystem.h"
#include "WED_ToolUtils.h"
#include "WED_ResourceMgr.h"
#include "WED_FacadePlacement.h"

DEFINE_PERSISTENT(WED_FacadeNode)
TRIVIAL_COPY(WED_FacadeNode,WED_GISPoint_Bezier)

WED_FacadeNode::WED_FacadeNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i)
#if AIRPORT_ROUTING
	,wall_type(this,"Wall", XML_Name("facade_node","wall_type"),FacadeWall, facade_Wall0)
#endif	
{
}

WED_FacadeNode::~WED_FacadeNode()
{
}

#if AIRPORT_ROUTING

void	WED_FacadeNode::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	dict.clear();
	if(n == PropertyItemNumber(&wall_type))
	{
		IResolver * resolver;
		WED_ResourceMgr * mgr;
		WED_Thing * parent, * grand_parent;
		WED_FacadePlacement * fac;
		string		res;
		fac_info_t	info;
		if((parent = this->GetParent()) != NULL)
		if((grand_parent = parent->GetParent()) != NULL)
		if((fac = dynamic_cast<WED_FacadePlacement *>(grand_parent)) != NULL)
		if((resolver = GetArchive()->GetResolver()) != NULL)
		if((mgr = WED_GetResourceMgr(resolver)) != NULL)
		{
			fac->GetResource(res);
			if (mgr->GetFac(res,info))
			if(!info.walls.empty())
			{
				dict.clear();
				for (int n = 0; n < info.walls.size(); ++n)
					dict[n + facade_Wall0] = make_pair(info.walls[n],true);
				return;
			}
		}
	}

	WED_Thing::GetNthPropertyDict(n,dict);			
}

void		WED_FacadeNode::GetNthPropertyDictItem(int n, int e, string& item) const
{
	if(n == PropertyItemNumber(&wall_type))
	{
		int idx = e - facade_Wall0;
		IResolver * resolver;
		WED_ResourceMgr * mgr;
		WED_Thing * parent, * grand_parent;
		WED_FacadePlacement * fac;
		string		res;
		fac_info_t	info;
		if((parent = this->GetParent()) != NULL)
		if((grand_parent = parent->GetParent()) != NULL)
		if((fac = dynamic_cast<WED_FacadePlacement *>(grand_parent)) != NULL)
		if((resolver = GetArchive()->GetResolver()) != NULL)
		if((mgr = WED_GetResourceMgr(resolver)) != NULL)
		{
			fac->GetResource(res);
			if (mgr->GetFac(res,info))
			if(info.walls.size() > idx)
			{
				item = info.walls[idx];
				return;
			}
		}
	}

	WED_Thing::GetNthPropertyDictItem(n,e,item);			
}

#endif

int		WED_FacadeNode::GetWallType(void) const
{
#if AIRPORT_ROUTING
	return ENUM_Export(wall_type.value);
#else
	return 0;
#endif
}

#if AIRPORT_ROUTING

void	WED_FacadeNode::SetWallType(int wt)
{
	wall_type = ENUM_Import(FacadeWall, wt);
}

bool	WED_FacadeNode::HasLayer		(GISLayer_t layer							  ) const
{
	if(layer == gis_Param) return true;
	return WED_GISPoint_Bezier::HasLayer(layer);
}

void	WED_FacadeNode::GetLocation		   (GISLayer_t l,      Point2& p) const
{
	if(l == gis_Param) p = Point2(ENUM_Export(wall_type.value),0.0);
	else				WED_GISPoint_Bezier::GetLocation(l, p);
}

bool	WED_FacadeNode::GetControlHandleLo (GISLayer_t l,       Point2& p) const
{
	GISLayer_t rl = (l == gis_Param) ? gis_Geo : l;
	if(!WED_GISPoint_Bezier::GetControlHandleLo(rl,p)) return false;
	if (l == gis_Param) p = Point2(ENUM_Export(wall_type.value),0.0);
	return true;
}

bool	WED_FacadeNode::GetControlHandleHi (GISLayer_t l,       Point2& p) const
{
	GISLayer_t rl = (l == gis_Param) ? gis_Geo : l;
	if(!WED_GISPoint_Bezier::GetControlHandleHi(rl,p)) return false;
	if (l == gis_Param) p = Point2(ENUM_Export(wall_type.value),0.0);
	return true;
}

#endif