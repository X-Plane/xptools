/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_CreateEdgeTool.h"
#include "WED_ToolUtils.h"
#include "WED_TaxiRouteNode.h"
#include "WED_RoadNode.h"
#include "WED_TaxiRoute.h"
#include "WED_RoadEdge.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_MapZoomerNew.h"
#include "WED_ResourceMgr.h"
#include "WED_GISUtils.h"
#include "WED_HierarchyUtils.h"
#include "WED_EnumSystem.h"
#include "STLUtils.h"
#include <sstream>

#include "WED_GroupCommands.h"
#if AIRPORT_ROUTING

static const char * kCreateCmds[] = { "Taxiway Route Line", "Road" };
static const int kIsAirport[] = { 1, 0 };

static bool is_edge_curved(CreateEdge_t tool_type)
{
	#if ROAD_EDITING
		if(tool_type == create_Road)
			return true;
	#endif
	#if HAS_CURVED_ATC_ROUTE
		return true;
	#endif
	
	return false;
}

WED_CreateEdgeTool::WED_CreateEdgeTool(
					const char *		tool_name,
					GUI_Pane *			host,
					WED_MapZoomerNew *	zoomer,
					IResolver *			resolver,
					WED_Archive *		archive,
					CreateEdge_t		tool) :
	WED_CreateToolBase(tool_name, host, zoomer, resolver, archive,
	2,						// min pts,
	99999999,				// max pts - yes, I am a hack.
	is_edge_curved(tool),	// curve allowed?
	0,						// curve required?
	1,						// close allowed?
	0),						// close required
	mType(tool),
	mVehicleClass(tool == create_TaxiRoute ? this : NULL,"Allowed Vehicles",XML_Name("",""), ATCVehicleClass, atc_Vehicle_Aircraft),
	mName(this, "Name",                                           XML_Name("",""), "N"),
	mOneway(tool == create_TaxiRoute ? this : NULL, "Oneway",     XML_Name("",""), 1),
	mRunway(tool == create_TaxiRoute ? this : NULL, "Runway",     XML_Name("",""), ATCRunwayTwoway, atc_rwy_None),
	mHotDepart(tool == create_TaxiRoute ? this : NULL, "Departure", XML_Name("",""), ATCRunwayOneway,false),
	mHotArrive(tool == create_TaxiRoute ? this : NULL, "Arrival", XML_Name("",""), ATCRunwayOneway,false),
	mHotILS(tool == create_TaxiRoute ? this : NULL, "ILS",        XML_Name("",""), ATCRunwayOneway,false),
	mWidth(tool == create_TaxiRoute ? this : NULL, "Size",        XML_Name("",""), ATCIcaoWidth, width_E),
#if ROAD_EDITING
	mLayer(tool == create_Road ? this : NULL, "Layer",         XML_Name("",""), 0, 2),
	mSubtype(tool == create_Road ? this : NULL, "Type",        XML_Name("",""), 1, 3),
	mResource(tool == create_Road ? this : NULL, "Resource",   XML_Name("",""), "lib/g10/roads.net"),
#endif
	mSlop(this, "Slop",                                        XML_Name("",""), 10, 2)
{
}

WED_CreateEdgeTool::~WED_CreateEdgeTool()
{
}

struct sort_by_seg_rat {
	sort_by_seg_rat(const Point2& i) : a(i) { }
	Point2	a;
	bool operator()(const pair<IGISPointSequence *, Point2>& p1, const pair<IGISPointSequence *, Point2>& p2) const {
		return a.squared_distance(p1.second) < a.squared_distance(p2.second);
	}
	bool operator()(const Point2& p1, const Point2& p2) const {
		return a.squared_distance(p1) < a.squared_distance(p2);
	}
};

template <class A, class B>
struct compare_second {
	bool operator()(const pair<A,B>& lhs, const pair<A,B>& rhs) const {
		return lhs.second == rhs.second; }
};

static void SortSplits(const Segment2& s, vector<Point2>& splits)
{
	sort(splits.begin(), splits.end(), sort_by_seg_rat(s.p1));
	
	// Nuke dupe pts.  A hack?  NO!  Intentional.  When we GIS-iterate through our hierarchy
	// we pick up all our graph end pts many times - once as nodes, and once as the points making
	// up the pt sequences that is the edges.
	splits.erase(unique(splits.begin(),splits.end()),splits.end());
}

static void SortSplits(const Segment2& s, vector<pair<IGISPointSequence *, Point2 > >& splits)
{
	sort(splits.begin(), splits.end(), sort_by_seg_rat(s.p1));
	splits.erase(unique(splits.begin(),splits.end(), compare_second<IGISPointSequence*,Point2>()),splits.end());
}



split_edge_info_t cast_WED_GISEdge_to_split_edge_info_t(WED_GISEdge* edge, bool active)
{
	DebugAssert(edge != NULL);
	return split_edge_info_t(edge, active);
}

void		WED_CreateEdgeTool::AcceptPath(
			const vector<Point2>&	in_pts,
			const vector<Point2>&	in_dirs_lo,
			const vector<Point2>&	in_dirs_hi,
			const vector<int>		in_has_dirs,
			const vector<int>		in_has_split,
			int						closed)
{
	vector<Point2>	pts(in_pts);
	vector<Point2>	dirs_lo(in_dirs_lo);
	vector<Point2>	dirs_hi(in_dirs_hi);
	vector<int>		has_dirs(in_has_dirs);
	vector<int>		has_split(in_has_split);
	
	int idx;
	WED_Thing * host_for_parent = GetHost(idx);
	if (host_for_parent == NULL) return;
	
	WED_Thing * host_for_merging = WED_GetContainerForHost(GetResolver(), host_for_parent, kIsAirport[mType], idx);

	string cname = string("Create ") + kCreateCmds[mType];

	GetArchive()->StartCommand(cname.c_str());

	ISelection *	sel = WED_GetSelect(GetResolver());
	sel->Clear();
	double frame_dist = fabs(GetZoomer()->YPixelToLat(mSlop.value)-GetZoomer()->YPixelToLat(0));

	const char * edge_class = WED_TaxiRoute::sClass;
#if ROAD_EDITING
	if(mType == create_Road)
		edge_class = WED_RoadEdge::sClass;
#endif
	/************************************************************************************************
	 * FIRST SNAPPING PASS - NODE TO NODE
	 ************************************************************************************************/
	
	// For each node we want to add, we are going to find a nearby existing node - and if we find one, we
	// lock our location to theirs.  This "direct hit" will get consoldiated during create.  (By moving our
	// path first, we don't get false intersections when the user meant to hit end to end.)
	for(int p = 0; p < pts.size(); ++p)
	{
		double	dist=frame_dist*frame_dist;
		WED_Thing * who = NULL;
		FindNear(host_for_merging, NULL, edge_class, pts[p],who,dist);
		if (who != NULL)
		{
			IGISPoint * pp = dynamic_cast<IGISPoint *>(who);
			if(pp)
				pp->GetLocation(gis_Geo,pts[p]);
		}	
	}
	
	/************************************************************************************************
	 * SECOND SNAPPING PASS - LOCK NEW PTS TO EXISTING EDGES
	 ************************************************************************************************/
	// Next: we need to see if our ndoes go near existing by existing edges...in that case,
	// split the edges and snap us over.
	for (int p = 0; p < pts.size(); ++p)
	{
		double dist=frame_dist*frame_dist;
		IGISPointSequence * seq = NULL;
		FindNearP2S(host_for_merging, NULL, edge_class,pts[p], seq, dist);
		if(seq)
			seq->SplitSide(pts[p], 0.001);		
	}
	
	/************************************************************************************************
	 *
	 ************************************************************************************************/
	
	vector<WED_GISEdge*> tool_created_edges;
	WED_GISEdge *	new_edge = NULL;
	WED_TaxiRoute *	tr = NULL;
#if ROAD_EDITING
	WED_RoadEdge * er = NULL;
#endif
	static int n = 0;
	int stop = closed ? pts.size() : pts.size()-1;
	int start = 0;

	WED_GISPoint * c;
	WED_Thing * src = NULL, * dst = NULL;
	double	dist=frame_dist*frame_dist;
	if(src == NULL)	
		FindNear(host_for_merging, NULL, edge_class,pts[start % pts.size()],src,dist);
	if(src == NULL)
	{
#if ROAD_EDITING
		src = c = (mType == create_TaxiRoute) ? (WED_GISPoint *) WED_TaxiRouteNode::CreateTyped(GetArchive()) : (WED_GISPoint *) WED_RoadNode::CreateTyped(GetArchive());
#else
		src = c = (WED_GISPoint *)WED_TaxiRouteNode::CreateTyped(GetArchive());
#endif
		src->SetParent(host_for_parent,idx);
		src->SetName(mName.value + "_start");
		c->SetLocation(gis_Geo,pts[0]);
	}

	int p = start + 1;
	while(p <= stop)
	{
		int sp = p - 1;
		int dp = p % pts.size();

		switch(mType) {
		case create_TaxiRoute:
			new_edge = tr = WED_TaxiRoute::CreateTyped(GetArchive());
			tr->SetOneway(mOneway.value);			
			tr->SetRunway(mRunway.value);
			tr->SetVehicleClass(mVehicleClass.value);
			tr->SetHotDepart(mHotDepart.value);
			tr->SetHotArrive(mHotArrive.value);
			tr->SetHotILS(mHotILS.value);
			tr->SetName(mName);
			tr->SetWidth(mWidth.value);
			break;
#if ROAD_EDITING
		case create_Road:
			new_edge = er = WED_RoadEdge::CreateTyped(GetArchive());
			er->SetSubtype(mSubtype.value);
			er->SetStartLayer(mLayer.value);
			er->SetEndLayer(mLayer.value);
			er->SetName(mName);
			er->SetResource(mResource.value);
			break;
#endif
		}
	
		new_edge->AddSource(src,0);
		dst = NULL;
		
		dist=frame_dist*frame_dist;
		FindNear(host_for_merging, NULL, edge_class,pts[dp],dst,dist);
		if(dst == NULL)
		{
			switch(mType) {
			case create_TaxiRoute:
				dst = c = WED_TaxiRouteNode::CreateTyped(GetArchive());
				break;
#if ROAD_EDITING
			case create_Road:
				dst = c = WED_RoadNode::CreateTyped(GetArchive());
				break;
#endif
			}
			dst->SetParent(host_for_parent,idx);
			dst->SetName(mName.value+"_stop");
			c->SetLocation(gis_Geo,pts[dp]);
		}		
		new_edge->AddSource(dst,1);
		
		if(has_dirs[sp])
		{
			if(has_dirs[dp])
			{
				new_edge->SetSideBezier(gis_Geo,Bezier2(in_pts[sp],dirs_hi[sp],dirs_lo[dp],in_pts[dp]));
			}
			else
			{
				new_edge->SetSideBezier(gis_Geo,Bezier2(in_pts[sp],dirs_hi[sp],in_pts[dp],in_pts[dp]));
			}
		}
		else
		{
			if(has_dirs[dp])
			{
				new_edge->SetSideBezier(gis_Geo,Bezier2(in_pts[sp],in_pts[sp],dirs_lo[dp],in_pts[dp]));
			}
		}
		// Do this last - half-built edge inserted the world destabilizes accessors.
		new_edge->SetParent(host_for_parent,idx);
		tool_created_edges.push_back(new_edge);
		sel->Insert(new_edge);
//		printf("Added edge %d  from 0x%08x to 0x%08x\n", p, src, dst);
		src = dst;
		++p;
	}

	//Collect edges in the current airport
	vector<WED_GISEdge*> all_edges;
	CollectRecursive(host_for_parent, back_inserter(all_edges));

	//filter them for just the crossing ones
	set<WED_GISEdge*> crossing_edges = do_select_crossing(all_edges);

	//convert, and run split!
	vector<split_edge_info_t> edges_to_split;
	
	for(set<WED_GISEdge*>::iterator e = crossing_edges.begin(); e != crossing_edges.end(); ++e)
		edges_to_split.push_back(cast_WED_GISEdge_to_split_edge_info_t(*e, find(tool_created_edges.begin(), tool_created_edges.end(), *e) != tool_created_edges.end()));
	
	edge_to_child_edges_map_t new_pieces = run_split_on_edges(edges_to_split);
	
	//For all the tool_created_edges that were split
	for(vector<WED_GISEdge*>::iterator itr = tool_created_edges.begin();
		itr != tool_created_edges.end() && new_pieces.size() > 0;
		++itr)
	{
		//Save the children as selected
		edge_to_child_edges_map_t::mapped_type& edge_map_entry = new_pieces[*itr];

		//Select only the new pieces
		set<ISelectable*> iselectable_new_pieces(edge_map_entry.begin(), edge_map_entry.end());
		sel->Insert(iselectable_new_pieces);
	}

	GetArchive()->CommitCommand();
}

bool		WED_CreateEdgeTool::CanCreateNow(void)
{
	int n;
	return GetHost(n) != NULL;
}

WED_Thing *	WED_CreateEdgeTool::GetHost(int& idx)
{
		return WED_GetCreateHost(GetResolver(), kIsAirport[mType], true, idx);
}

const char *		WED_CreateEdgeTool::GetStatusText(void)
{
	static char buf[256];
	int n;
	if (GetHost(n) == NULL)
	{
			sprintf(buf,"You must create an airport before you can add a %s.",kCreateCmds[mType]);
		return buf;
	}
	return NULL;
}

void WED_CreateEdgeTool::FindNear(WED_Thing * host, IGISEntity * ent, const char * filter, const Point2& loc, WED_Thing *& out_thing, double& out_dsq)
{
	IGISEntity * e = ent ? ent : dynamic_cast<IGISEntity*>(host);
	WED_Thing * t = host ? host : dynamic_cast<WED_Thing *>(ent);
	WED_Entity * et = t ? dynamic_cast<WED_Entity *>(t) : NULL;
	if(!IsVisibleNow(et))	return;
	if(IsLockedNow(et))		return;
	if(e && t)
	{
		Point2	l;
		IGISPoint * p;
		IGISPointSequence * ps;
		IGISComposite * c;
	
		switch(e->GetGISClass()) {
		case gis_Point:
		case gis_Point_Bezier:
		case gis_Point_Heading:
		case gis_Point_HeadingWidthLength:			
			if((p = dynamic_cast<IGISPoint *>(e)) != NULL)
			{
				p->GetLocation(gis_Geo,l);
				double my_dist = Segment2(loc,l).squared_length();
				if(my_dist < out_dsq)
				{
					out_thing = t;
					out_dsq = my_dist;
				}
			}
			break;
		case gis_PointSequence:
		case gis_Line:
		case gis_Line_Width:
		case gis_Ring:
		case gis_Edge:
		case gis_Chain:
			if(filter == NULL || filter == t->GetClass())
			if((ps = dynamic_cast<IGISPointSequence*>(e)) != NULL)
			{
				for(int n = 0; n < ps->GetNumPoints(); ++n)
					FindNear(NULL,ps->GetNthPoint(n), filter, loc, out_thing, out_dsq);
			}
			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					FindNear(NULL,c->GetNthEntity(n), filter, loc, out_thing, out_dsq);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			FindNear(host->GetNthChild(n), NULL, filter, loc, out_thing, out_dsq);
	}
}

void WED_CreateEdgeTool::FindNearP2S(WED_Thing * host, IGISEntity * ent, const char * filter, const Point2& loc, IGISPointSequence *& out_thing, double& out_dsq)
{
	IGISEntity * e = ent ? ent : dynamic_cast<IGISEntity*>(host);
	WED_Thing * t = host ? host : dynamic_cast<WED_Thing *>(ent);
	WED_Entity * et = t ? dynamic_cast<WED_Entity *>(t) : NULL;
	if(!IsVisibleNow(et))	return;
	if(IsLockedNow(et))		return;
	if(e && t)
	{
		Point2	l;
		IGISPoint * p;
		IGISPointSequence * ps;
		IGISComposite * c;
	
		switch(e->GetGISClass()) {
		case gis_PointSequence:
		case gis_Line:
		case gis_Line_Width:
		case gis_Ring:
		case gis_Edge:
		case gis_Chain:
			if(filter == NULL || t->GetClass() == filter)
			if((ps = dynamic_cast<IGISPointSequence*>(e)) != NULL)
			{
				int ns = ps->GetNumSides();
				for(int n = 0; n < ns; ++n)
				{
					Bezier2 b;
					Segment2 s;
					if(ps->GetSide(gis_Geo,n,s,b))
					{
						if(loc != b.p1 && loc != b.p2)
						{								
//							double d = b.squared_distance(loc);
//							if(d < out_dsq)
//							{
//								out_dsq = d;
//								out_thing = ps;
//							}
						}
					
					}
					else					
					{
						if(loc != s.p1 && loc != s.p2)
						{
							double d = s.squared_distance(loc);
							if(d < out_dsq)
							{
								out_dsq = d;
								out_thing = ps;
							}
						}
					}
				}
			}
			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					FindNearP2S(NULL,c->GetNthEntity(n), filter, loc, out_thing, out_dsq);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			FindNearP2S(host->GetNthChild(n), NULL, filter, loc, out_thing, out_dsq);
	}
}



void	WED_CreateEdgeTool::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	dict.clear();

#if ROAD_EDITING
	if(n == PropertyItemNumber(&mSubtype))
	{
		road_info_t r;
		if(get_valid_road_info(&r))
		{
			for(map<int,string>::iterator i = r.vroad_types.begin(); i != r.vroad_types.end(); ++i)
			{
				dict[i->first] = make_pair(i->second, true);
			}
			return;
		}
	}
	else if(n == PropertyItemNumber(&mRunway))
#else
	if (n == PropertyItemNumber(&mRunway))
#endif
	{
		WED_Airport * airport = WED_GetCurrentAirport(GetResolver());
		if(airport)
		{
			PropertyDict_t full;
			WED_CreateToolBase::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysTwoway(airport, legal);
			legal.insert(mRunway.value);
			legal.insert(atc_rwy_None);
			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,f->second));
		}
	}
	else if (n == PropertyItemNumber(&mHotDepart) ||
			 n == PropertyItemNumber(&mHotArrive) ||
			 n == PropertyItemNumber(&mHotILS))
	{
		WED_Airport * airport = WED_GetCurrentAirport(GetResolver());
		if(airport)
		{
			PropertyDict_t full;
			WED_CreateToolBase::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysOneway(airport, legal);
			PropertyVal_t val;
			this->GetNthProperty(n,val);
			DebugAssert(val.prop_kind == prop_EnumSet);
			copy(val.set_val.begin(),val.set_val.end(),set_inserter(legal));
			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,f->second));
		}
	}
	else
		WED_CreateToolBase::GetNthPropertyDict(n,dict);			
}

void		WED_CreateEdgeTool::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	WED_CreateToolBase::GetNthPropertyInfo(n, info);
#if ROAD_EDITING
	if(n == PropertyItemNumber(&mSubtype))
	{
		if(get_valid_road_info(NULL))
		{
			info.prop_kind = prop_Enum;
			return;
		}
	}
#endif

	//Ensures only the relevant properties are shown with atc_Vehicles_Ground_Trucks selected
	PropertyVal_t prop;
	mVehicleClass.GetProperty(prop);

	if (prop.int_val == atc_Vehicle_Ground_Trucks)
	{
		if (n == PropertyItemNumber(&mRunway)    ||
			n == PropertyItemNumber(&mHotDepart) ||
			n == PropertyItemNumber(&mHotArrive) ||
			n == PropertyItemNumber(&mHotILS)    ||
			n == PropertyItemNumber(&mWidth))
		{
			//"." is the special hardcoded "disable me" string, see IPropertyObject.h
			info.prop_name = ".";
			info.can_edit = false;
			info.can_delete = false;
		}
	}
}

void		WED_CreateEdgeTool::GetNthProperty(int n, PropertyVal_t& val) const
{
	WED_CreateToolBase::GetNthProperty(n, val);
#if ROAD_EDITING
	if(n == PropertyItemNumber(&mSubtype))
	{
		if(get_valid_road_info(NULL))
		{
			val.prop_kind = prop_Enum;
		}
	}
#endif
}

void		WED_CreateEdgeTool::SetNthProperty(int n, const PropertyVal_t& val)
{
#if ROAD_EDITING
	if(n == PropertyItemNumber(&mSubtype))
	{
		if(get_valid_road_info(NULL))
		{
			PropertyVal_t v(val);
			v.prop_kind = prop_Int;
			WED_CreateToolBase::SetNthProperty(n, v);
			return;
		}
	}
#endif
	WED_CreateToolBase::SetNthProperty(n, val);
}


void		WED_CreateEdgeTool::GetNthPropertyDictItem(int n, int e, string& item) const
{
#if ROAD_EDITING
	if(n == PropertyItemNumber(&mSubtype))
	{
		road_info_t r;
		if(get_valid_road_info(&r))
		{
			map<int,string>::iterator i = r.vroad_types.find(mSubtype.value);
			if (i != r.vroad_types.end())
			{
				item = i->second;
				return;
			}
			else
			{
				if (mSubtype.value == 1)
				{
					item = "None";
				}
				else
				{
					stringstream ss;
					ss << mSubtype.value;
					item = ss.str();
				}
				return;
			}
		}
	}
#endif
	WED_CreateToolBase::GetNthPropertyDictItem(n, e, item);
}


#if ROAD_EDITING
bool		WED_CreateEdgeTool::get_valid_road_info(road_info_t * optional_info) const
{
	road_info_t temp;
	road_info_t * i = optional_info ? optional_info : &temp;
	
	IResolver * resolver;
	WED_ResourceMgr * mgr;
	if((mgr = WED_GetResourceMgr(GetResolver())) != NULL)
	{
		road_info_t r;
		if(mgr->GetRoad(mResource.value, *i))
		if(i->vroad_types.size() > 0)
			return true;
	}
	return false;
}
#endif

#endif
