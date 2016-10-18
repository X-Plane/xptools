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

#include "WED_CreatePolygonTool.h"
#include "WED_AirportChain.h"
#include "WED_Taxiway.h"
#include "IResolver.h"
#include "GISUtils.h"
#include "ISelection.h"
#include "WED_AirportNode.h"
#include "WED_FacadeNode.h"
#include "WED_ToolUtils.h"
#include "WED_EnumSystem.h"
#include "WED_AirportBoundary.h"
#include "WED_Airport.h"
#include "WED_Ring.h"
#include "WED_FacadePlacement.h"
#include "WED_ForestRing.h"
#include "WED_ForestPlacement.h"
#include "WED_FacadeRing.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_LinePlacement.h"
#include "WED_StringPlacement.h"
#include "WED_SimpleBezierBoundaryNode.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_TextureNode.h"
#include "WED_TextureBezierNode.h"
#include "WED_ResourceMgr.h"
#include "MathUtils.h"

static const char * kCreateCmds[] = { "Taxiway", "Boundary", "Marking", "Hole", "Facade", "Forest", "String", "Line", "Polygon" };

static const int kIsAirport[] = { 1, 1, 1,  0,     0, 0,  0, 0, 0 };
static const int kRequireClosed[] = { 1, 1, 0, 1,    1, 1, 0, 0, 1 };
static const int kAllowCurved[] = { 1, 0, 1, 1,    1, 0,  1, 1, 1 };

string stripped_resource(const string& r)
{
	string n(r);
	string::size_type p = n.find_last_of("/\\:");
	if(p != n.npos) n.erase(0,p+1);
	return n;
}

WED_CreatePolygonTool::WED_CreatePolygonTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									CreateTool_t		tool) :
	WED_CreateToolBase(tool_name,host, zoomer, resolver,archive,
	kRequireClosed[tool] ? 3 : 2,	// min pts
	99999999,						// max pts
	kAllowCurved[tool],			// curve allowed
	0,							// curve required?
	1,							// close allowed
	kRequireClosed[tool]),		// close required?
	mType(tool),
		mPavement(tool    == create_Taxi ? this : NULL,"Pavement",XML_Name("",""),Surface_Type,surf_Concrete),
		mRoughness(tool   == create_Taxi ? this : NULL,"Roughness",XML_Name("",""),0.25,4,2),
		mHeading(tool     == create_Taxi || tool == create_Polygon ? this : NULL,"Heading",XML_Name("",""),0,5,2),
		mMarkings(tool    <= create_Hole ? this : NULL,".Markings", XML_Name("",""), LinearFeature, 0),
		mMarkingsLines(tool <= create_Hole ? this : NULL,"Markings", XML_Name("",""), ".Markings",line_SolidYellow,line_BWideBrokenDouble, 1),
		mMarkingsLights(tool <= create_Hole ? this : NULL,"Lights", XML_Name("",""), ".Markings",line_TaxiCenter,line_BoundaryEdge, 1),

		mResource(tool >  create_Hole ? this : NULL, "Resource", XML_Name("",""), ""),
		mHeight(tool   == create_Facade ? this : NULL, "Height", XML_Name("",""), 10.0, 4, 2),
		mDensity(tool  == create_Forest ? this : NULL, "Density", XML_Name("",""), 1.0, 3, 2),
		mSpacing(tool  == create_String ? this : NULL, "Spacing", XML_Name("",""), 5.0, 3, 1),
		
		mUVMap(tool    == create_Polygon ? this : NULL, "Use Texture Map", XML_Name("",""), 0)
{
	mPavement.value = surf_Concrete;
}

WED_CreatePolygonTool::~WED_CreatePolygonTool()
{
}

void	WED_CreatePolygonTool::AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed)
{
	char buf[256];

	int idx;
	WED_Thing * host = GetHost(idx);
	if (host == NULL) return;

	string cname = string("Create ") + kCreateCmds[mType];

	GetArchive()->StartCommand(cname.c_str());

	ISelection *	sel = WED_GetSelect(GetResolver());
	if (mType != create_Hole)
	sel->Clear();

	int is_bezier = mType != create_Forest && mType != create_Boundary;
	int is_apt = mType <= create_Hole;
	int is_poly = mType != create_Hole && mType != create_String && mType != create_Line;
	int is_texed = 0;
	int is_forest = mType == create_Forest;
	int is_facade = mType == create_Facade;
	
	if(mType == create_Polygon)
	{
		is_texed = mUVMap.value;
		/*
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(GetResolver());
		pol_info_t i;
		if(rmgr->GetPol(mResource.value, i))
		{
			is_texed = !i.wrap;
		}
		*/
	}
	
	if(mType == create_Hole)
	{
		DebugAssert(host->CountChildren() > 0);
		WED_Thing * old_outer_ring = host->GetNthChild(0);
		DebugAssert(old_outer_ring->CountChildren() > 0);
		is_apt = dynamic_cast<WED_AirportChain*>(old_outer_ring) != NULL;
		is_bezier = dynamic_cast<IGISPoint_Bezier*>(old_outer_ring->GetNthChild(0)) != NULL;
		is_poly = true;
		is_texed = dynamic_cast<WED_TextureNode*>(old_outer_ring->GetNthChild(0)) != NULL ||
				   dynamic_cast<WED_TextureBezierNode*>(old_outer_ring->GetNthChild(0)) != NULL;
		is_forest = dynamic_cast<WED_ForestRing*>(old_outer_ring) != NULL;
		is_facade = dynamic_cast<WED_FacadeRing*>(old_outer_ring) != NULL;
	}

	WED_AirportChain *	apt_ring=NULL;
	WED_Thing *			outer_ring;

		 if(is_apt)					outer_ring = apt_ring = WED_AirportChain::CreateTyped(GetArchive());
	else if(is_forest)				outer_ring			  = WED_ForestRing::CreateTyped(GetArchive());
	else if(is_facade)				outer_ring			  = WED_FacadeRing::CreateTyped(GetArchive());
	else if(is_poly)				outer_ring			  = WED_Ring::CreateTyped(GetArchive());

	static int n = 0;
	++n;

	bool is_ccw = (mType == create_Marks || mType == create_String || mType == create_Line) ? true : is_ccw_polygon_pt(pts.begin(), pts.end());
	if (mType == create_Hole) is_ccw = !is_ccw;

	switch(mType) {
	case create_Taxi:
		{
			WED_Taxiway *		tway = WED_Taxiway::CreateTyped(GetArchive());
			outer_ring->SetParent(tway,0);
			tway->SetParent(host, idx);

			sprintf(buf,"New Taxiway %d",n);
			tway->SetName(buf);
			sprintf(buf,"Taxiway %d Outer Ring",n);
			outer_ring->SetName(buf);

			tway->SetRoughness(mRoughness.value);
			tway->SetHeading(mHeading.value);
			tway->SetSurface(mPavement.value);

			sel->Select(tway);
		}
		break;
	case create_Boundary:
		{
			WED_AirportBoundary *		bwy = WED_AirportBoundary::CreateTyped(GetArchive());
			outer_ring->SetParent(bwy,0);
			bwy->SetParent(host, idx);

			sprintf(buf,"Airport Boundary %d",n);
			bwy->SetName(buf);
			sprintf(buf,"Airport Boundary %d Outer Ring",n);
			outer_ring->SetName(buf);

			sel->Select(bwy);

		}
		break;
	case create_Marks:
		{
			outer_ring->SetParent(host, idx);
			sprintf(buf,"Linear Feature %d",n);
			outer_ring->SetName(buf);

			if (mType != create_Hole)
				sel->Select(outer_ring);
		}
		break;
	case create_Hole:
		{
			outer_ring->SetParent(host, host->CountChildren());
			sprintf(buf,"Linear Feature %d",n);
			outer_ring->SetName(buf);

			if (mType != create_Hole)
				sel->Select(outer_ring);
		}
		break;
	case create_Facade:
		{
			WED_FacadePlacement * fac = WED_FacadePlacement::CreateTyped(GetArchive());
			outer_ring->SetParent(fac,0);
			fac->SetParent(host,idx);
			sprintf(buf,"Facade %d",n);
			fac->SetName(stripped_resource(mResource.value));
			sprintf(buf,"Facade %d outer ring",n);
			outer_ring->SetName(buf);
			sel->Select(fac);
			fac->SetResource(mResource.value);
			fac->SetHeight(mHeight.value);
		}
		break;
	case create_Forest:
		{
			WED_ForestPlacement * fst = WED_ForestPlacement::CreateTyped(GetArchive());
			outer_ring->SetParent(fst,0);
			fst->SetParent(host,idx);
			sprintf(buf,"Forest %d",n);
			fst->SetName(stripped_resource(mResource.value));
			sprintf(buf,"Forest %d outer ring",n);
			outer_ring->SetName(buf);
			sel->Select(fst);
			fst->SetResource(mResource.value);
			fst->SetDensity(mDensity.value);
		}
		break;
	case create_String:
		{
			WED_StringPlacement * str = WED_StringPlacement::CreateTyped(GetArchive());
			outer_ring = str;
			str->SetParent(host,idx);
			sprintf(buf,"String %d",n);
			str->SetName(stripped_resource(mResource.value));
			sel->Select(str);
			str->SetClosed(closed);
			str->SetResource(mResource.value);
			str->SetSpacing(mSpacing.value);
		}
		break;
	case create_Line:
		{
			WED_LinePlacement * lin = WED_LinePlacement::CreateTyped(GetArchive());
			outer_ring = lin;
			lin->SetParent(host,idx);
			sprintf(buf,"Line %d",n);
			lin->SetName(stripped_resource(mResource.value));
			sel->Select(lin);
			lin->SetClosed(closed);
			lin->SetResource(mResource.value);
		}
		break;
	case create_Polygon:
		if(is_texed)
		{
			WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(GetArchive());
			outer_ring->SetParent(dpol,0);
			dpol->SetParent(host,idx);
			sprintf(buf,"Orthophoto %d",n);
			dpol->SetName(stripped_resource(mResource.value));
			sprintf(buf,"Polygon %d outer ring",n);
			outer_ring->SetName(buf);
			sel->Select(dpol);
			dpol->SetResource(mResource.value);
		}
		else
		{
			WED_PolygonPlacement * pol = WED_PolygonPlacement::CreateTyped(GetArchive());
			outer_ring->SetParent(pol,0);
			pol->SetParent(host,idx);
			sprintf(buf,"Polygon %d",n);
			pol->SetName(stripped_resource(mResource.value));
			sprintf(buf,"Polygon %d outer ring",n);
			outer_ring->SetName(buf);
			sel->Select(pol);
			pol->SetResource(mResource.value);
			pol->SetHeading(mHeading.value);
		}
		break;
	}

	if(apt_ring)
		apt_ring->SetClosed(closed);

	double	lonmax=-9.9e9;
	double	latmax=-9.9e9;
	double	lonmin= 9.9e9;
	double	latmin= 9.9e9;
	bool	has_any_dirs = false;
	for(int n = 0; n < pts.size(); ++n)
	{
		lonmax=max(lonmax,pts[n].x());
		lonmin=min(lonmin,pts[n].x());
		latmax=max(latmax,pts[n].y());
		latmin=min(latmin,pts[n].y());
		if(has_dirs[n])
		{
			has_any_dirs=true;

			lonmax=max(lonmax,dirs_hi[n].x());
			lonmin=min(lonmin,dirs_hi[n].x());
			latmax=max(latmax,dirs_hi[n].y());
			latmin=min(latmin,dirs_hi[n].y());

			lonmax=max(lonmax,dirs_lo[n].x());
			lonmin=min(lonmin,dirs_lo[n].x());
			latmax=max(latmax,dirs_lo[n].y());
			latmin=min(latmin,dirs_lo[n].y());
		}
	}

	const float hard_coded_s[4] = { 0.0, 1.0, 1.0, 0.0 };
	const float hard_coded_t[4] = { 0.0, 0.0, 1.0, 1.0 };

	for(int n = 0; n < pts.size(); ++n)
	{
		int idx = is_ccw ? n : pts.size()-n-1;
		
		WED_AirportNode *		anode=NULL;
		WED_GISPoint_Bezier *	bnode=NULL;
		WED_GISPoint *			node=NULL;
		WED_TextureNode *		tnode=NULL;
		WED_TextureBezierNode *	tbnode=NULL;

		if(is_apt)							node = bnode = anode = WED_AirportNode::CreateTyped(GetArchive());
		else if(is_facade)					node = bnode = WED_FacadeNode::CreateTyped(GetArchive());
		else if (is_bezier && is_texed)		node = bnode = tbnode = WED_TextureBezierNode::CreateTyped(GetArchive());
		else if (is_bezier)					node = bnode = WED_SimpleBezierBoundaryNode::CreateTyped(GetArchive());
		else if (is_texed)					node = tnode = WED_TextureNode::CreateTyped(GetArchive());
		else								node = WED_SimpleBoundaryNode::CreateTyped(GetArchive());

		node->SetLocation(gis_Geo,pts[idx]);
		Point2 st(			interp(lonmin,0.0,lonmax,1.0,pts[idx].x()),
							interp(latmin,0.0,latmax,1.0,pts[idx].y()));
		if(pts.size() == 4 && !has_any_dirs)	{st.x_ = hard_coded_s[n];
												 st.y_ = hard_coded_t[n];}
							
		if(tnode)			tnode->SetLocation(gis_UV,st);
		if(tbnode)			tbnode->SetLocation(gis_UV,st);

		if(bnode)
		{
			if (!has_dirs[idx])
			{
				bnode->DeleteHandleHi();
				bnode->DeleteHandleLo();
				if(tbnode)
				{
					tbnode->SetControlHandleLo(gis_UV,st);
					tbnode->SetControlHandleHi(gis_UV,st);
				}
			}
			else
			{
				bnode->SetSplit(has_split[idx]);
				if (is_ccw)
				{
					bnode->SetControlHandleHi(gis_Geo,dirs_hi[idx]);
					bnode->SetControlHandleLo(gis_Geo,dirs_lo[idx]);
					if(tbnode)
					{
						tbnode->SetControlHandleHi(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_hi[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_hi[idx].y())));
						tbnode->SetControlHandleLo(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_lo[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_lo[idx].y())));
					}
				} else {
					bnode->SetControlHandleHi(gis_Geo,dirs_lo[idx]);
					bnode->SetControlHandleLo(gis_Geo,dirs_hi[idx]);
					if(tbnode)
					{
						tbnode->SetControlHandleHi(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_lo[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_lo[idx].y())));
						tbnode->SetControlHandleLo(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_hi[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_hi[idx].y())));
					}
				}
			}
		}
		node->SetParent(outer_ring, n);
		if(anode)
			anode->SetAttributes(mMarkings.value);
		sprintf(buf,"Node %d",n+1);
		node->SetName(buf);
	}

	GetArchive()->CommitCommand();

}

const char *	WED_CreatePolygonTool::GetStatusText(void)
{
	static char buf[256];
	int n;
	if (GetHost(n) == NULL)
	{
		if (mType == create_Hole)
			sprintf(buf,"You must selet a polygon before you can insert a hole into it.  Facades cannot have interior holes.");
		else
			sprintf(buf,"You must create an airport before you can add a %s.",kCreateCmds[mType]);
		return buf;
	}
	return NULL;
}

bool		WED_CreatePolygonTool::CanCreateNow(void)
{
	int n;
	return GetHost(n) != NULL;
}

WED_Thing *		WED_CreatePolygonTool::GetHost(int& idx)
{
	if (mType == create_Hole)
	{
		ISelection * sel = WED_GetSelect(GetResolver());
		if (sel->GetSelectionCount() != 1) return NULL;
		WED_GISPolygon * igp = dynamic_cast<WED_GISPolygon *>(sel->GetNthSelection(0));
		if(!igp) 
			return NULL;
		// A few polygons do NOT get holes: facades, um...that's it for now.
		if(igp->GetClass() == WED_FacadePlacement::sClass)
			return NULL;
		return igp;
	} else
		return WED_GetCreateHost(GetResolver(), kIsAirport[mType], true, idx);
}


void		WED_CreatePolygonTool::SetResource(const string& r)
{
	mResource.value = r;
}

void	WED_CreatePolygonTool::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	WED_CreateToolBase::GetNthPropertyDict(n, dict);
	if(n == PropertyItemNumber(&mPavement) && mPavement.value != surf_Water)
	{
		dict.erase(surf_Water);
	}
}

