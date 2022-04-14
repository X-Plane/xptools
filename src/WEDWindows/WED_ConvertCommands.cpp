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
#include "WED_ConvertCommands.h"
#include "WED_GroupCommands.h"

#include "ISelection.h"

#include "AssertUtils.h"
#include "PlatformUtils.h"
#include "WED_EnumSystem.h"
#include "WED_LibraryMgr.h"
#include "WED_ToolUtils.h"

#include "WED_AirportChain.h"
#include "WED_AirportNode.h"
#include "WED_ForestPlacement.h"
#include "WED_ForestRing.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_Ring.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_SimpleBezierBoundaryNode.h"
#include "WED_StringPlacement.h"
#include "WED_Taxiway.h"
#include "WED_ObjPlacement.h"



int		WED_CanConvertTo(IResolver * resolver, const char* DstClass)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0)
		return 0;

	for (size_t i = 0; i < sel->GetSelectionCount(); ++i)
	{
		auto t = dynamic_cast<WED_Thing*>(sel->GetNthSelection(i));
		const char* SrcClass = t->GetClass();

		if (SrcClass == DstClass)	return 0;

		// Must be one of the types we can convert from
		if (SrcClass != WED_PolygonPlacement::sClass &&
			SrcClass != WED_Taxiway::sClass &&
			SrcClass != WED_AirportChain::sClass &&
			SrcClass != WED_LinePlacement::sClass &&
			SrcClass != WED_StringPlacement::sClass &&
			!(SrcClass == WED_ObjPlacement::sClass && DstClass == WED_ForestPlacement::sClass && sel->GetSelectionCount() >= 3)) return 0;

//		if (DstClass == WED_StringPlacement::sClass && SrcClass != WED_AirportChain::sClass) return 0;

		// Parent must not be a WED_GISPolygon (which can happen if it's a WED_AirportChain)
		if (dynamic_cast<WED_GISPolygon*>(t->GetParent()))	return 0;

		bool dstIsPolygon = DstClass == WED_PolygonPlacement::sClass || DstClass == WED_Taxiway::sClass;
		auto chain = dynamic_cast<WED_GISChain*>(t);
		if (chain  && dstIsPolygon)
			if (!chain->IsClosed() || chain->GetNumPoints() < 3) return 0;
	}
	return 1;
}

// Gets all the WED_GISChains in 't', including 't' itself if it is a WED_GISChain.
static void get_chains(WED_Thing * t, vector<WED_GISChain*>& chains)
{
	if (!t)
		return;

	WED_GISChain * c = dynamic_cast<WED_GISChain*>(t);
	if (c)
	{
		chains.push_back(c);
		return;
	}

	for (int child = 0; child < t->CountChildren(); ++child)
	{
		c = dynamic_cast<WED_GISChain*>(t->GetNthChild(child));
		if (c)
			chains.push_back(c);
	}
}

// Moves Bezier points from 'src' to 'dst'. Converts between WED_AirportNode and WED_SimpleBezierBoundaryNode
// as necessitated by the type of 'dst'. Nodes may be removed from 'src' (if no conversion is necessary)
// but are not guaranteed to be.
static void move_points(WED_Thing * src, WED_Thing * dst)
{
	// Collect points from 'src' (before we start removing any childern)
	vector<WED_GISPoint*> points;
	for (int i = 0; i < src->CountChildren(); ++i)
	{
		WED_GISPoint * p = dynamic_cast<WED_GISPoint*>(src->GetNthChild(i));
		if (p)
			points.push_back(p);
	}

	bool want_apt_nodes = (dynamic_cast<WED_AirportChain*>(dst) != NULL);

	for (int i = 0; i < points.size(); ++i)
	{
		bool have_apt_node = (dynamic_cast<WED_AirportNode*>(points[i]) != NULL);
		if (have_apt_node == want_apt_nodes)
		{
			points[i]->SetParent(dst, i);
		}
		else
		{
			WED_GISPoint_Bezier * src_bezier = dynamic_cast<WED_GISPoint_Bezier*>(points[i]);
			WED_GISPoint * dst_node = NULL;
			WED_GISPoint_Bezier * dst_bezier = NULL;
			if (want_apt_nodes)
			{
				dst_bezier = WED_AirportNode::CreateTyped(dst->GetArchive());
				dst_node = dst_bezier;
			}
			else if (src_bezier)
			{
				dst_bezier = WED_SimpleBezierBoundaryNode::CreateTyped(dst->GetArchive());
				dst_node = dst_bezier;
			}
			else
				dst_node = WED_SimpleBoundaryNode::CreateTyped(dst->GetArchive());
			string name;
			points[i]->GetName(name);
			dst_node->SetName(name);
			if (src_bezier && dst_bezier)
			{
				BezierPoint2 location;
				src_bezier->GetBezierLocation(gis_Geo, location);
				dst_bezier->SetBezierLocation(gis_Geo, location);
			}
			else
			{
				Point2 location;
				points[i]->GetLocation(gis_Geo, location);
				dst_node->SetLocation(gis_Geo, location);
			}
			dst_node->SetParent(dst, i);
		}
	}
}

static bool is_ccw(WED_GISChain* c)
{
	vector<Point2> points(c->GetNumPoints());
	for (int i = 0; i < c->GetNumPoints(); ++i)
		c->GetNthPoint(i)->GetLocation(gis_Geo, points[i]);
	return is_ccw_polygon_pt(points.begin(), points.end());
}

// Adds copies of the given chains to 'dst'; creates chains of the appropriate type for 'dst'. The source chains are not
// deleted or reparented, but nodes may be removed from them.
static void add_chains(WED_Thing * dst, const vector<WED_GISChain*>& chains)
{
	for (int i = 0; i < chains.size(); ++i)
	{
		WED_GISChain * dst_chain;
		if (dst->GetClass() == WED_Taxiway::sClass)
		{
			WED_AirportChain *ac = WED_AirportChain::CreateTyped(dst->GetArchive());
			ac->SetClosed(true);
			dst_chain = ac;
		}
		else
			dst_chain = WED_Ring::CreateTyped(dst->GetArchive());
		move_points(chains[i], dst_chain);
		if (is_ccw(dst_chain) == (i > 0))
			dst_chain->Reverse(gis_Geo);
		dst_chain->SetParent(dst, i);
	}
}

static void copy_heading(WED_Thing * src, WED_Thing * dst)
{
	int src_prop = src->FindProperty("Heading");
	if (src_prop == -1)
		src_prop = src->FindProperty("Texture Heading");
	int dst_prop = dst->FindProperty("Heading");
	if (dst_prop == -1)
		dst_prop = dst->FindProperty("Texture Heading");
	if (src_prop != -1 && dst_prop != -1)
	{
		PropertyVal_t val;
		src->GetNthProperty(src_prop, val);
		dst->SetNthProperty(dst_prop, val);
	}
}

typedef pair<int, vector<int> > style_t;

static style_t get_style(WED_Thing * t)
{
	int surf_type;
	vector<int> line_style;

	if (t->GetClass() == WED_Taxiway::sClass)
	{
		auto twy = static_cast<WED_Taxiway*>(t);
		surf_type = twy->GetSurface();

		string resource;
		twy->GetResource(resource);
		auto dollar_pos = resource.find('$');
		line_style.push_back(ENUM_Export(ENUM_LookupDesc(LinearFeature, resource.substr(0, dollar_pos).c_str())));
		if (dollar_pos != string::npos)
			line_style.push_back(ENUM_Export(ENUM_LookupDesc(LinearFeature, resource.substr(dollar_pos + 2).c_str())));
	}
	else if(t->GetClass() == WED_PolygonPlacement::sClass)
	{
		string resource;
		static_cast<WED_PolygonPlacement*>(t)->GetResource(resource);

		surf_type = WED_GetLibraryMgr(t->GetArchive()->GetResolver())->GetSurfEnum(resource);
		if (surf_type < 0)
		{
			if (resource.find("concrete") != string::npos)
				surf_type = surf_Concrete;
			else
				surf_type = surf_Asphalt;
		}
	}
	else if (t->GetClass() == WED_LinePlacement::sClass)
	{
		string resource;
		static_cast<WED_LinePlacement*>(t)->GetResource(resource);
		if(resource.substr(0,strlen("lib/airport/lines/")) == "lib/airport/lines/")
		{
			resource.erase(0,strlen("lib/airport/lines/"));
			int linetype;
			if(sscanf(resource.c_str(),"%d",&linetype) == 1)
				line_style.push_back(linetype);
		}
	}
	else if(t->GetClass() == WED_AirportChain::sClass)
	{
		// return value only if whl chain same style ... need to break up multi-linestyle chains beforehand

		string resource;
		static_cast<WED_AirportChain*>(t)->GetResource(resource);
		auto dollar_pos = resource.find('$');
		line_style.push_back(ENUM_Export(ENUM_LookupDesc(LinearFeature, resource.substr(0,dollar_pos).c_str())));
		if(dollar_pos != string::npos)
			line_style.push_back(ENUM_Export(ENUM_LookupDesc(LinearFeature, resource.substr(dollar_pos+2).c_str())));
	}
	if (t->GetClass() == WED_StringPlacement::sClass)
	{
		string resource;
		static_cast<WED_LinePlacement*>(t)->GetResource(resource);
		if (resource.substr(0, strlen("lib/airport/lights/slow/")) == "lib/airport/lights/slow/")
		{
			resource.erase(0, strlen("lib/airport/lights/slow/"));
			int linetype;
			if (sscanf(resource.c_str(), "%d", &linetype) == 1)
				line_style.push_back(linetype);
		}
	}
	return make_pair(surf_type, line_style);
}

static void set_style(WED_Thing * t, style_t style, WED_LibraryMgr * lmgr)
{
	if(t->GetClass() == WED_Taxiway::sClass)
	{
		auto taxiway = static_cast<WED_Taxiway*>(t);
		if(ENUM_Domain(style.first) == Surface_Type)
			taxiway->SetSurface(style.first);
		else
			taxiway->SetSurface(surf_Asphalt);
	}
	if (t->GetClass() == WED_PolygonPlacement::sClass)
	{
		string resource;
		if (!lmgr->GetSurfVpath(style.first, resource))
			resource = "lib/airport/pavement/asphalt_3D.pol";              // default to default asphalt
		static_cast<WED_PolygonPlacement*>(t)->SetResource(resource);
	}
	if (t->GetClass() == WED_LinePlacement::sClass)
	{
		string vpath;
		if(style.second.size())
			if (lmgr->GetLineVpath(style.second.front() < 100 ? style.second.front() : style.second.back(), vpath))
				static_cast<WED_LinePlacement*>(t)->SetResource(vpath);
	}
	if (t->GetClass() == WED_AirportChain::sClass)
	{
		set<int> attr;
		for (auto a : style.second)
		{
				attr.insert(ENUM_Import(LinearFeature, a));
		}
		auto chain = static_cast<WED_AirportChain*>(t);
		for (int i = 0; i < chain->GetNumEntities(); i++)
		{
			if (auto node = dynamic_cast<WED_AirportNode*>(chain->GetNthEntity(i)))
				node->SetAttributes(attr);
		}
	}
	if (t->GetClass() == WED_StringPlacement::sClass)
	{
		string vpath;
		if (style.second.size())
			if (lmgr->GetLineVpath(style.second.front() >= 100 ? style.second.front() : style.second.back(), vpath))
				static_cast<WED_StringPlacement*>(t)->SetResource(vpath);
	}
}

static void set_closed(WED_Thing * t, bool closed)
{
	if (t->GetClass() == WED_AirportChain::sClass)
		static_cast<WED_AirportChain*>(t)->SetClosed(closed);         // IsClosed is virtual, SetClosed() not ? Why ????
	if (t->GetClass() == WED_LinePlacement::sClass)
		static_cast<WED_LinePlacement*>(t)->SetClosed(closed);
	if (t->GetClass() == WED_StringPlacement::sClass)
		static_cast<WED_StringPlacement*>(t)->SetClosed(closed);
}

static bool needs_apt(WED_Thing* t)
{
	if (t->GetClass() == WED_Taxiway::sClass ||
		t->GetClass() == WED_AirportChain::sClass)
		return 1;
	else
		return 0;
}

void	WED_DoConvertTo(IResolver * resolver, CreateThingFunc create)
{
	auto wrl  = WED_GetWorld(resolver);
	auto sel  = WED_GetSelect(resolver);
	auto lmgr = WED_GetLibraryMgr(resolver);
	IOperation* op = dynamic_cast<IOperation*>(sel);
	op->StartOperation((string("Convert to ") /* + dst->HumanReadableType() */ ).c_str());

	set<WED_Thing*> to_delete;

	for (size_t i = 0; i < sel->GetSelectionCount(); ++i)
	{
		WED_Thing* src = dynamic_cast<WED_Thing*>(sel->GetNthSelection(i));
		vector<WED_GISChain*> chains;
		get_chains(src, chains);
		if (chains.empty())
		{
			DoUserAlert("No chains");
			op->AbortOperation();
			return;
		}


		WED_Thing* dst = create(wrl->GetArchive());
		bool dst_is_polygon = dynamic_cast<WED_GISPolygon*>(dst) != NULL;

		if (dst_is_polygon)
		{
			add_chains(dst, chains);

			string name;
			src->GetName(name);
			dst->SetName(name);

			copy_heading(src, dst);

			set_style(dst, get_style(src), lmgr);

			sel->Insert(dst);
			dst->SetParent(src->GetParent(), src->GetMyPosition() + 1);
		}
		else
		{
			for (int i = 0; i < chains.size(); ++i)
			{
				if(i > 0) dst = create(wrl->GetArchive());

				string name;
				src->GetName(name);
				dst->SetName(name);

				set_closed(dst, chains[i]->IsClosed());

				move_points(chains[i], dst);

				set_style(dst, get_style(src), lmgr);

				sel->Insert(dst);
				dst->SetParent(src->GetParent(), src->GetMyPosition() + 1 + i);
			}
		}

		sel->Erase(src);
		src->SetParent(NULL, 0);

		to_delete.insert(src);
	}

	WED_AddChildrenRecursive(to_delete);
	WED_RecursiveDelete(to_delete);

	op->CommitOperation();
}

void	WED_DoConvertToForest(IResolver* resolver)
{
	auto wrl = WED_GetWorld(resolver);
	auto sel = WED_GetSelect(resolver);
	IOperation* op = dynamic_cast<IOperation*>(sel);
	set<WED_Thing*> to_delete;

	auto where = dynamic_cast<WED_Thing*>(sel->GetNthSelection(0));
	if(!where)
		return;

	op->StartOperation("Convert to Forest Points");
	auto fst = WED_ForestPlacement::CreateTyped(wrl->GetArchive());
	fst->SetParent(where->GetParent(), where->GetMyPosition());
	fst->SetDensity(1.0);
	fst->SetResource("lib/vegetation/trees/deciduous/maple_medium.for");
	fst->SetName("maple_medium.for");
	fst->SetFillMode(dsf_fill_points);

	auto rng = WED_ForestRing::CreateTyped(wrl->GetArchive());
	rng->SetParent(fst, 0);
	rng->SetName("Forest Boundary");

	for (size_t i = 0; i < sel->GetSelectionCount(); ++i)
	{
		if (auto src = dynamic_cast<WED_ObjPlacement*>(sel->GetNthSelection(i)))
		{
			auto dst = WED_SimpleBoundaryNode::CreateTyped(wrl->GetArchive());
			dst->SetParent(rng, i);
			Point2 pt;
			src->GetLocation(gis_Geo, pt);
			dst->SetLocation(gis_Geo, pt);
			dst->SetName(string("Tree ") + to_string(i));
			to_delete.insert(src);
		}
	}

	if (rng->CountChildren() >= 3)
	{
		WED_RecursiveDelete(to_delete);
		sel->Clear();
		sel->Insert(fst);
		op->CommitOperation();
	}
	else
		op->AbortOperation();
}
