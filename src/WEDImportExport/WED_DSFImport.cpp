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

#include "WED_DSFImport.h"

#include "DSFLib.h"
#include "DSF2Text.h"
#include "PlatformUtils.h"
#include "FileUtils.h"
#include "STLUtils.h"

#include "WED_AptIE.h"
#include "WED_EnumSystem.h"
#include "WED_GISUtils.h"
#include "WED_MetadataUpdate.h"
#include "WED_SimpleBoundaryNode.h"

#include "WED_Airport.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_ExclusionZone.h"
#include "WED_FacadePlacement.h"
#include "WED_FacadeRing.h"
#include "WED_FacadeNode.h"
#include "WED_ForestPlacement.h"
#include "WED_ForestRing.h"
#include "WED_Group.h"
#include "WED_StringPlacement.h"
#include "WED_AutogenPlacement.h"
#include "WED_AutogenNode.h"
#include "WED_LinePlacement.h"
#include "WED_ObjPlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_Ring.h"
#include "WED_TextureBezierNode.h"
#include "WED_TextureNode.h"
#include "WED_SimpleBezierBoundaryNode.h"
#include "WED_SimpleBoundaryNode.h"
#if ROAD_EDITING
#include "WED_RoadNode.h"
#include "WED_RoadEdge.h"
#endif

#include <sstream>

#define NO_FOR 0
#define NO_FAC 0
#define NO_LIN 0
#define NO_STR 0
#define NO_POL 0
#define NO_OBJ 0
#define NO_NET !ROAD_EDITING
#define NO_EXC 0

static void debug_it(const vector<BezierPoint2>& pts)
{
	for(int n = 0; n < pts.size(); ++n)
		printf("%d) %lf,%lf (%lf,%lf/%lf,%lf)\n",
			n,
			pts[n].pt.x(),
			pts[n].pt.y(),
			pts[n].lo.x() - pts[n].pt.x(),
			pts[n].lo.y() - pts[n].pt.y(),
			pts[n].hi.x() - pts[n].pt.x(),
			pts[n].hi.y() - pts[n].pt.y());
}

static bool end_match(const string& str, const char suf[4])
{
	if(str.size() >= 4)
		return memcmp(str.c_str() + str.size() - 4, suf, 4) == 0;
	else
		return false;
}

enum dsf_import_category {
	dsf_cat_exclusion = 0,
	dsf_cat_objects,
	dsf_cat_facades,
	dsf_cat_forests,
	dsf_cat_lines,
	dsf_cat_strings,
	dsf_cat_autogen,
	dsf_cat_orthophoto,
	dsf_cat_draped_poly,
	dsf_cat_roads,
	dsf_cat_terrain,
	dsf_cat_terrain_fx,
	dsf_cat_pavement_fx,
	dsf_cat_DIM
};

static const char * k_dsf_cat_names[dsf_cat_DIM] = {
	"Exclusion Zones",
	"Objects",
	"Facades",
	"Forests",
	"Lines",
	"Strings",
	"Autogen",
	"Orthophotos",
	"Draped Polygons",
	"Roads",
	"Terrain",
	"Terrain FX",
	"Pavement FX"
};

//ToDo:mroe: partial DSF import implemented . By bound and category , checking the bounds for roadnets only yet
class	DSF_Importer {
public:

	DSF_Importer() : is_overlay(false), dsf_cat_filter(dsf_filter_all), filter_on(false) ,is_in_bounds(false)
	{
		for(int n = 0; n < 7; ++n)
			req_level_obj[n] = req_level_agp[n] = req_level_fac[n] = -1;
		for(int n = 0; n < dsf_cat_DIM; ++n)
			bucket_parents[n] = NULL;
	}

	int					req_level_obj[7];
	int					req_level_agp[7];
	int					req_level_fac[7];

	struct	tbl_entry {
		string				vpath;
		string				name;
		dsf_import_category	cat;

		tbl_entry(const char * c) : vpath(c)
		{
			size_t pos = vpath.find_last_of('/');
			if(pos == string::npos)
				name = vpath;
			else
				name = c + pos + 1;

			cat = dsf_cat_objects;
			if(vpath.find("/terrain_FX/") != string::npos)
				cat = dsf_cat_terrain_fx;
			else if(vpath.find("/pavement_FX/") != string::npos)
				cat = dsf_cat_pavement_fx;
		};
	};

	vector<tbl_entry>	obj_table;
	vector<tbl_entry>	pol_table;
	vector<string>		net_table;

	WED_Thing *			master_parent;
	WED_Thing *			bucket_parents[dsf_cat_DIM];
	WED_Archive *		archive;

	vector<BezierPoint2>pts,uvs;
	vector<int>			walls;
	WED_Thing *			ring;
	WED_Thing *			poly;
	bool				want_uv;
	bool				want_bezier;
	bool				want_wall;
	int 				dsf_cat_filter;       // categories, e.g. .for, .obj
	vector<string>		dsf_AptID_filter;     // Airport ID's
	vector<bool>		filter_table;     	  // Airport ID IDX
	bool				filter_on;            // global switch to filter out stuff
	vector<Bbox2>		cull_bounds;
	int					is_in_bounds;
	int					autogen_rings;
	int					autogen_spelling;
	bool 				is_overlay;

	bool isInBounds(const Segment2& in_seg)
	{
		for(const auto& b : cull_bounds )
		{
			if(b.contains(in_seg.p1) || b.contains(in_seg.p2)) return true;
		}
		return false;
	}

	bool isInBounds(const Point2& in_pnt)
	{
		for(const auto& b : cull_bounds )
		{
			if(b.contains(in_pnt)) return true;
		}
		return false;
	}

	WED_Thing * get_cat_parent(dsf_import_category cat)
	{
		if(bucket_parents[cat] == NULL)
		{
			bucket_parents[cat] = WED_Group::CreateTyped(archive);
			bucket_parents[cat]->SetName(k_dsf_cat_names[cat]);
		}
		return bucket_parents[cat];
	}

	vector<WED_ExclusionZone *> accum_exclusions;

	vector<pair<Point2, int> >	accum_road;
	pair<int, int>		accum_road_type;

#if !NO_NET
	typedef map<pair<int, int>, WED_RoadNode *> road_node_map_t;
	road_node_map_t road_nodes;
	unsigned int        road_start_ID;
#endif

	int GetShowForFacID(int id)
	{
		for(int l = 1; l <= 6; ++l)
		if(req_level_fac[l] != -1)
		if(req_level_fac[l] <= id)
			return l;
		return 6;
	}

	int GetShowForObjID(int id)
	{
		for(int l = 1; l <= 6; ++l)
		if(req_level_obj[l] != -1)
		if(req_level_obj[l] <= id)
			return l;
		return 6;
	}

	void handle_req_obj(const char * str)
	{
		int level, id;
		if(sscanf(str,"%d/%d",&level,&id) == 2)
		for(int l = level; l <= 6; ++l)
		if(req_level_obj[l] == -1 || req_level_obj[l] > id)
			req_level_obj[l] = id;
	}

	void handle_req_agp(const char * str)
	{
		int level, id;
		if(sscanf(str,"%d/%d",&level,&id) == 2)
		for(int l = level; l <= 6; ++l)
		if(req_level_agp[l] == -1 || req_level_agp[l] > id)
			req_level_agp[l] = id;
	}

	void handle_req_fac(const char * str)
	{
		int level, id;
		if(sscanf(str,"%d/%d",&level,&id) == 2)
		for(int l = level; l <= 6; ++l)
		if(req_level_fac[l] == -1 || req_level_fac[l] > id)
			req_level_fac[l] = id;
	}

	void make_exclusion(const char * ex, int k)
	{
		Bbox2 b;
		if(sscanf(ex,"%lf/%lf/%lf/%lf",&b.p1.x_, &b.p1.y_, &b.p2.x_, &b.p2.y_) == 4)
		{
			for(const auto& z : accum_exclusions)
			{
				Bbox2 b_new;
				z->GetMin()->GetLocation(gis_Geo, b_new.p1);
				z->GetMax()->GetLocation(gis_Geo, b_new.p2);
				if(b_new == b)
				{
					set<int> s;
					z->GetExclusions(s);
					s.insert(k);
					z->SetExclusions(s);
					return;
				}
			}

			WED_ExclusionZone * z = WED_ExclusionZone::CreateTyped(archive);
			z->SetName("Exclusion Zone");
			z->SetParent(get_cat_parent(dsf_cat_exclusion),get_cat_parent(dsf_cat_exclusion)->CountChildren());
			set<int> s;
			s.insert(k);
			z->SetExclusions(s);

			WED_SimpleBoundaryNode * p1 = WED_SimpleBoundaryNode::CreateTyped(archive);
			WED_SimpleBoundaryNode * p2 = WED_SimpleBoundaryNode::CreateTyped(archive);
			p1->SetParent(z,0);
			p2->SetParent(z,1);
			p1->SetName("e1");
			p2->SetName("e2");
			p1->SetLocation(gis_Geo,b.p1);
			p2->SetLocation(gis_Geo,b.p2);

			accum_exclusions.push_back(z);
		}
	}

	static bool NextPass(int finished_pass_index, void * inRef)
	{
		return true;
	}

	static int	AcceptTerrainDef(const char * inPartialPath, void * inRef)
	{
		return 1;
	}

	static int	AcceptObjectDef(const char * inPartialPath, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->obj_table.push_back(inPartialPath);
		return 1;
	}

	static int	AcceptPolygonDef(const char * inPartialPath, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->pol_table.push_back(inPartialPath);
		return 1;
	}

	static int	AcceptNetworkDef(const char * inPartialPath, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->net_table.push_back(inPartialPath);
		return 1;
	}

	static int AcceptRasterDef(const char * inPartalPath, void * inRef)
	{
		return 1;
	}

	static void	AcceptProperty(const char * inProp, const char * inValue, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;

		if(strcmp(inProp, "sim/require_object") == 0)	me->handle_req_obj(inValue);
		if(strcmp(inProp, "sim/require_agpoint") == 0)	me->handle_req_agp(inValue);
		if(strcmp(inProp, "sim/require_facade") == 0)	me->handle_req_fac(inValue);
		if(strcmp(inProp, "sim/filter/aptid") == 0)
		{
			me->filter_table.push_back(false);
			if(find(me->dsf_AptID_filter.begin(), me->dsf_AptID_filter.end(), inValue) !=  me->dsf_AptID_filter.end())
				me->filter_table.back() = true;
		}

#if !NO_EXC
		if(strcmp(inProp, "sim/exclude_obj") == 0)	me->make_exclusion(inValue, exclude_Obj);
		if(strcmp(inProp, "sim/exclude_fac") == 0)	me->make_exclusion(inValue, exclude_Fac);
		if(strcmp(inProp, "sim/exclude_for") == 0)	me->make_exclusion(inValue, exclude_For);
		if(strcmp(inProp, "sim/exclude_bch") == 0)	me->make_exclusion(inValue, exclude_Bch);
		if(strcmp(inProp, "sim/exclude_net") == 0)	me->make_exclusion(inValue, exclude_Net);
		if(strcmp(inProp, "sim/exclude_lin") == 0)	me->make_exclusion(inValue, exclude_Lin);
		if(strcmp(inProp, "sim/exclude_pol") == 0)	me->make_exclusion(inValue, exclude_Pol);
		if(strcmp(inProp, "sim/exclude_str") == 0)	me->make_exclusion(inValue, exclude_Str);
#endif
		if(strcmp(inProp, "sim/overlay") == 0 && atoi(inValue) == 1)
			me->is_overlay = true;
	}

	static void	BeginPatch(
					unsigned int	inTerrainType,
					double 			inNearLOD,
					double 			inFarLOD,
					unsigned char	inFlags,
					int				inCoordDepth,
					void *			inRef)
	{
	}

	static void	BeginPrimitive(
					int				inType,
					void *			inRef)
	{
	}

	static void	AddPatchVertex(
					double			inCoordinates[],
					void *			inRef)
	{
	}

	static void	EndPrimitive(
					void *			inRef)
	{
	}

	static void	EndPatch(
					void *			inRef)
	{
	}

	static void	AddObjectWithMode(
					unsigned int	inObjectType,
					double			inCoordinates[4],
					obj_elev_mode	inMode,
					void *			inRef)
	{
#if !NO_OBJ
		DSF_Importer * me = (DSF_Importer *) inRef;
		if(me->dsf_cat_filter & dsf_filter_objects)
		{
			WED_ObjPlacement * obj = WED_ObjPlacement::CreateTyped(me->archive);
			obj->SetResource(me->obj_table[inObjectType].vpath);
			obj->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
			if (inMode == obj_ModeDraped)
				obj->SetDefaultMSL();
			else
				obj->SetCustomMSL(inCoordinates[3], inMode == obj_ModeAGL);
//			static_cast<WED_GISPoint_Heading*>(obj)->WED_GISPoint_Heading::SetHeading(inCoordinates[2]); // avoid loading .obj definition to check for fixed heading flag
			obj->WED_GISPoint_Heading::SetHeading(inCoordinates[2]); // avoid loading .obj definition to check for fixed heading flag
			obj->SetName(me->obj_table[inObjectType].name);
			dsf_import_category cat = me->obj_table[inObjectType].cat;
			obj->SetParent(me->get_cat_parent(cat),me->get_cat_parent(cat)->CountChildren());
			obj->SetShowLevel(me->GetShowForObjID(inObjectType));
		}
#endif
	}

	static void	BeginSegment(
					unsigned int	inNetworkType,
					unsigned int	inNetworkSubtype,
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef)
	{
#if !NO_NET
		DSF_Importer * me = (DSF_Importer *) inRef;
		if(me->dsf_cat_filter & dsf_filter_roads && !me->filter_on)
		{
			DebugAssert(me->accum_road.empty());

			me->road_start_ID = (unsigned int) inCoordinates[3];
			me->accum_road_type = make_pair(inNetworkType, inNetworkSubtype);
			me->accum_road.push_back(make_pair(Point2(inCoordinates[0], inCoordinates[1]), int(inCoordinates[2])));
		}
#endif
	}

	static void	AddSegmentShapePoint(
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef)
	{
#if !NO_NET
		DSF_Importer * me = (DSF_Importer *) inRef;
		if(me->dsf_cat_filter & dsf_filter_roads && !me->filter_on)
			me->accum_road.push_back(make_pair(Point2(inCoordinates[0], inCoordinates[1]), int(inCoordinates[2])));
#endif
	}

	static void	EndSegment(
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef)
	{
#if !NO_NET
		DSF_Importer * me = (DSF_Importer *) inRef;
		if(!(me->dsf_cat_filter & dsf_filter_roads) || me->filter_on) return;

		if (!me->cull_bounds.empty())
		{
			bool is_excluded = me->isInBounds(Point2(inCoordinates[0], inCoordinates[1]));
			for (const auto& pt : me->accum_road)
			{
				if(is_excluded)
					break;
				is_excluded |= me->isInBounds(pt.first);
			}
			if (!is_excluded)
			{
				me->accum_road.clear();
				return;
			}
		}
		DebugAssert(me->accum_road.size() > 0);

		unsigned int inNetworkType = me->accum_road_type.first;
		unsigned int inStartNodeID = me->road_start_ID;

		road_node_map_t::iterator sn = me->road_nodes.find(make_pair(inNetworkType, inStartNodeID));
		WED_RoadNode * road_start;
		if(sn == me->road_nodes.end())
		{
			WED_RoadNode * start_node = WED_RoadNode::CreateTyped(me->archive);
			start_node->SetParent(me->get_cat_parent(dsf_cat_roads),me->get_cat_parent(dsf_cat_roads)->CountChildren());
			stringstream ss;
			ss << inStartNodeID;
			start_node->SetLocation(gis_Geo, Point2(me->accum_road[0].first));
			start_node->SetName(ss.str());
			me->road_nodes[make_pair(inNetworkType, inStartNodeID)] = start_node;
			road_start = start_node;
		}
		else
			road_start = sn->second;

		unsigned int inEndNodeID = inCoordinates[3];
		road_node_map_t::iterator en = me->road_nodes.find(make_pair(me->accum_road_type.first, inEndNodeID));
		WED_RoadNode * road_end;
		if(en == me->road_nodes.end())
		{
			WED_RoadNode * end_node = WED_RoadNode::CreateTyped(me->archive);
			end_node->SetParent(me->get_cat_parent(dsf_cat_roads),me->get_cat_parent(dsf_cat_roads)->CountChildren());
			stringstream ss;
			ss << inEndNodeID;
			end_node->SetLocation(gis_Geo, Point2(inCoordinates[0], inCoordinates[1]));
			end_node->SetName(ss.str());
			me->road_nodes[make_pair(me->accum_road_type.first, inEndNodeID)] = end_node;
			road_end = end_node;
		}
		else
			road_end = en->second;

		me->accum_road.push_back(make_pair(Point2(inCoordinates[0], inCoordinates[1]), int(inCoordinates[2])));

		DebugAssert(me->accum_road.size() > 1);

		int start_level = me->accum_road[0].second;
		int end_level = inCoordinates[2];
		int curr_level = start_level;
		me->accum_road.front().second = 0;
		me->accum_road.back().second = 0;

		int last = me->accum_road.size()-1;
		int s = 0;
		int side = 0;

		WED_RoadEdge * edge = WED_RoadEdge::CreateTyped(me->archive);
		edge->SetParent(me->get_cat_parent(dsf_cat_roads), me->get_cat_parent(dsf_cat_roads)->CountChildren());
		edge->SetResource(me->net_table[me->accum_road_type.first]);
		edge->SetSubtype(me->accum_road_type.second);
		edge->SetStartLayer(start_level);
		edge->SetEndLayer(end_level);
		edge->AddSource(road_start, 0);

		while(s < last)
		{
			int n = s+1;
			while(n < me->accum_road.size() && me->accum_road[n].second == 1)
				++n;
			DebugAssert(n <= last);
			int span = n - s;
			DebugAssert(span > 0);
			DebugAssert(span < 4);
			if(n == last)
			{
				edge->AddSource(road_end, 1);
				edge->SetEndLayer(end_level);
			}
			else
			{
				WED_SimpleBezierBoundaryNode * shape_node = WED_SimpleBezierBoundaryNode::CreateTyped(me->archive);
				shape_node->SetName("shape point");
				shape_node->SetParent(edge,edge->CountChildren());
			}

			if(span == 1)
			{
				Segment2 path(me->accum_road[s].first, me->accum_road[n].first);
				edge->SetSide(gis_Geo, path, side);
			}
			else if(span == 2)
			{
				Point2 p1 = me->accum_road[s].first;
				Point2 p2 = me->accum_road[s+2].first;
				Point2	c = me->accum_road[s+1].first;

				Point2 c1 = Point2(
								p1.x() + (c.x() - p1.x()) * 2.0 / 3.0,
								p1.y() + (c.y() - p1.y()) * 2.0 / 3.0);
				Point2 c2 = Point2(
								p2.x() + (c.x() - p2.x()) * 2.0 / 3.0,
								p2.y() + (c.y() - p2.y()) * 2.0 / 3.0);

				Bezier2 path(p1,c1,c2,p2);
				edge->SetSideBezier(gis_Geo, path, side);
			}
			else if(span == 3)
			{
				Bezier2 path(me->accum_road[s].first,
							me->accum_road[s+1].first,
							me->accum_road[s+2].first,
							me->accum_road[s+3].first);
				edge->SetSideBezier(gis_Geo, path, side);
			}

			++side;
			s = n;
		}

		me->accum_road.clear();
#endif
	}

	static void	BeginPolygon(
					unsigned int	inPolygonType,
					unsigned short	inParam,
					int				inCoordDepth,
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		string r  = me->pol_table[inPolygonType].vpath;

		me->poly = NULL;
		me->ring = NULL;
		me->want_uv = false;
		me->want_wall = false;
		me->autogen_rings = 0;
		me->autogen_spelling = -1;

		dsf_import_category cat = dsf_cat_objects;

		me->is_in_bounds = me->cull_bounds.empty();

#if !NO_FAC
		if( me->dsf_cat_filter & dsf_filter_facades && end_match(r,".fac" ))
		{
			// Ben says: .fac must be 2-coord for v9.  But...maybe for v10 we allow curved facades?
			me->want_bezier=(inCoordDepth >= 4);
			me->want_wall = (inCoordDepth == 3 || inCoordDepth == 5);
			WED_FacadePlacement * fac = WED_FacadePlacement::CreateTyped(me->archive);
			fac->SetCustomWalls(me->want_wall);
			me->poly = fac;
			me->ring = NULL;
			fac->SetHeight(inParam);
			fac->SetResource(r);
			fac->SetShowLevel(me->GetShowForFacID(inPolygonType));
			cat = dsf_cat_facades;
		}
#endif

#if !NO_FOR
		else if(me->dsf_cat_filter & dsf_filter_forests && end_match(r,".for"))
		{
			me->want_bezier=false;
			WED_ForestPlacement * forst = WED_ForestPlacement::CreateTyped(me->archive);
			me->poly = forst;
			me->ring = NULL;
			forst->SetDensity((inParam % 256) / 255.0);
			forst->SetFillMode(inParam / 256);
			forst->SetResource(r);
			cat = dsf_cat_forests;
		}
#endif

#if !NO_LIN
		else if( me->dsf_cat_filter & dsf_filter_lines && end_match(r,".lin"))
		{
			me->want_bezier=inCoordDepth == 4;
			WED_LinePlacement * lin = WED_LinePlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = lin;
			lin->SetClosed(inParam);
			lin->SetResource(r);
			cat = dsf_cat_lines;
		}
#endif

#if !NO_STR
		else if(me->dsf_cat_filter & dsf_filter_strings && end_match(r,".str"))
		{
			me->want_bezier=inCoordDepth == 4;
			WED_StringPlacement * str = WED_StringPlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = str;
			str->SetSpacing(inParam);
			str->SetResource(r);
			cat = dsf_cat_strings;
		}
#endif

#if !NO_AG
		else if(me->dsf_cat_filter & dsf_filter_autogen && (end_match(r, ".ags") || end_match(r, ".agb")))
		{
			me->want_bezier=false;
			WED_AutogenPlacement * ags = WED_AutogenPlacement::CreateTyped(me->archive);
			me->poly = ags;
			me->ring = NULL;
			ags->SetResource(r);
			ags->SetHeight(((inParam >> 8) & 255) * 4);
			if(ags->IsAGBlock())
			{
				me->autogen_rings = -1;
				ags->SetSpelling(inParam & 255);
			}
			else
				me->autogen_rings = inParam & 255;
			cat = dsf_cat_autogen;
		}
#endif

#if !NO_POL
		else if(me->dsf_cat_filter & dsf_filter_draped_poly && (end_match(r,".pol") || end_match(r,".agb")))
		{
			me->want_uv=inParam == 65535;
			me->want_bezier=me->want_uv ? (inCoordDepth == 8) : (inCoordDepth == 4);
			std::replace(r.begin(), r.end(), '\\', '/');  // DSF created with older windows WED used backslash for local orthophoto paths
			if(me->want_uv)
			{
				WED_DrapedOrthophoto * orth = WED_DrapedOrthophoto::CreateTyped(me->archive);
				me->poly = orth;
				orth->SetResource(r);
				cat = dsf_cat_orthophoto;
			}
			else
			{
				WED_PolygonPlacement * pol = WED_PolygonPlacement::CreateTyped(me->archive);
				me->poly = pol;
				pol->SetHeading( 1.0/128.0 * (int) (inParam / 360) + inParam % 360 );
				pol->SetResource(r);
				cat = dsf_cat_draped_poly;
			}
			me->ring = NULL;
		}
#endif
		if(me->pol_table[inPolygonType].cat != dsf_cat_objects)
			cat = me->pol_table[inPolygonType].cat;

		if(me->poly)
		{
			me->poly->SetParent(me->get_cat_parent(cat),me->get_cat_parent(cat)->CountChildren());
			me->poly->SetName(me->pol_table[inPolygonType].name);
		}
		if(me->ring)
		{
			me->ring->SetParent(me->get_cat_parent(cat),me->get_cat_parent(cat)->CountChildren());
			me->ring->SetName(me->pol_table[inPolygonType].name);
		}
	}

	static void	BeginPolygonWinding(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->pts.clear();
		me->walls.clear();
		me->uvs.clear();
		if(me->poly != NULL)
		{
			if(me->poly->GetClass() == WED_ForestPlacement::sClass)
				me->ring = WED_ForestRing::CreateTyped(me->archive);
			else if(me->poly->GetClass() == WED_FacadePlacement::sClass)
				me->ring = WED_FacadeRing::CreateTyped(me->archive);
			else
				me->ring = WED_Ring::CreateTyped(me->archive);
			me->ring->SetParent(me->poly,me->poly->CountChildren());
			me->ring->SetName("Ring");
		}
	}

	static void	AddPolygonPoint(
					double *		inCoordinates,
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		WED_Thing * node;
		if(me->want_wall && me->want_bezier)
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.hi.x_ = inCoordinates[3];
			p.hi.y_ = inCoordinates[4];
			p.lo = p.pt + Vector2(p.hi, p.pt);
			me->pts.push_back(p);
			me->walls.push_back(inCoordinates[2]);
		}
		else if(me->want_wall)
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.lo = p.hi = p.pt;
			me->pts.push_back(p);
			me->walls.push_back(inCoordinates[2]);
		}
		else if(me->want_uv && me->want_bezier)
		{
			BezierPoint2	p, u;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.hi.x_ = inCoordinates[2];
			p.hi.y_ = inCoordinates[3];
			p.lo = p.pt + Vector2(p.hi, p.pt);

			u.pt.x_ = inCoordinates[4];
			u.pt.y_ = inCoordinates[5];
			u.hi.x_ = inCoordinates[6];
			u.hi.y_ = inCoordinates[7];
			u.lo = u.pt + Vector2(u.hi, u.pt);

			me->pts.push_back(p);
			me->uvs.push_back(u);
		}
		else if (me->want_uv)
		{
			BezierPoint2	p, u;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.lo = p.hi = p.pt;

			u.pt.x_ = inCoordinates[2];
			u.pt.y_ = inCoordinates[3];
			u.lo = u.hi = u.pt;
			me->pts.push_back(p);
			me->uvs.push_back(u);
		}
		else if (me->want_bezier)
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.hi.x_ = inCoordinates[2];
			p.hi.y_ = inCoordinates[3];
			p.lo = p.pt + Vector2(p.hi, p.pt);
			me->pts.push_back(p);
		}
		else
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.lo = p.hi = p.pt;
			me->pts.push_back(p);
		}

		if(!me->is_in_bounds)
		{
			Point2 p;
			p.x_ = inCoordinates[0];
			p.y_ = inCoordinates[1];
			me->is_in_bounds = me->isInBounds(p);
		}
	}

	static void	EndPolygonWinding(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;

		if(me->ring == NULL)
			return;

		if(me->want_bezier)
		{
			vector<BezierPoint2>	pc, uc;
//			debug_it(me->pts);
//			debug_it(me->uvs);

			BezierPointSeqFromTriple(me->pts.begin(),me->pts.end(), back_inserter(pc));
			me->pts.swap(pc);

			if(me->want_wall)
			{
				vector<int> wc;
				auto w = me->walls.begin();
				for(auto p : me->pts)
				{
					wc.push_back(*w++);
					if(p.is_split())
					{
						w++;
						if(p.has_lo() && p.has_hi()) w++;
					}
				}
				me->walls.swap(wc);
			}

			if(me->want_uv)
			{
				BezierPointSeqFromTriple(me->uvs.begin(),me->uvs.end(), back_inserter(uc));
				me->uvs.swap(uc);
			}

			if(me->pts.front().pt == me->pts.back().pt)
			{
				me->pts.front().lo = me->pts.back().lo;
				me->pts.pop_back();
				if(me->want_uv)
				{
					me->uvs.front().lo = me->uvs.back().lo;
					me->uvs.pop_back();
				}

				WED_StringPlacement * str = dynamic_cast<WED_StringPlacement*>(me->ring);
				if(str)
					str->SetClosed(1);
			}

//			debug_it(me->pts);
//			debug_it(me->uvs);

		}

		int i = me->ring->CountChildren();

		for(int n = 0; n < me->pts.size(); ++n)
		{
			WED_Thing * node;
			if(me->want_uv && me->want_bezier)
			{
				WED_TextureBezierNode * tb = WED_TextureBezierNode::CreateTyped(me->archive);
				node=tb;
				tb->SetBezierLocation(gis_Geo,me->pts[n]);
				tb->SetBezierLocation(gis_UV,me->uvs[n]);
			}
			else if (me->want_uv)
			{
				WED_TextureNode * t = WED_TextureNode::CreateTyped(me->archive);
				node=t;
				t->SetLocation(gis_Geo,me->pts[n].pt);
				t->SetLocation(gis_UV,me->uvs[n].pt);
			}
			else if (me->ring && me->ring->GetClass() == WED_FacadeRing::sClass)
			{
				WED_FacadeNode * b = WED_FacadeNode::CreateTyped(me->archive);
				node=b;
				b->SetBezierLocation(gis_Geo,me->pts[n]);
				if(me->want_wall)
					b->SetWallType(me->walls[n]);
			}
			else if (me->autogen_rings != 0)
			{
				WED_AutogenNode * a = WED_AutogenNode::CreateTyped(me->archive);
				node=a;
				a->SetLocation(gis_Geo, me->pts[n].pt);
				a->SetSpawning(me->autogen_rings > 0);
			}
			else if (me->want_bezier)
			{
				WED_SimpleBezierBoundaryNode * b = WED_SimpleBezierBoundaryNode::CreateTyped(me->archive);
				node=b;
				b->SetBezierLocation(gis_Geo,me->pts[n]);
			}
			else
			{
				WED_SimpleBoundaryNode * nd = WED_SimpleBoundaryNode::CreateTyped(me->archive);
				node=nd;
				nd->SetLocation(gis_Geo,me->pts[n].pt);
			}
			node->SetParent(me->ring,me->ring->CountChildren());
			char c[32];
			snprintf(c,32,"Point %d/%d", i, n);
			node->SetName(c);
		}

		if (me->autogen_rings > 0)
		{
			--me->autogen_rings;
			if(me->autogen_rings == 0)
				me->autogen_rings = -1;
		}

		if (me->poly != NULL)
			me->ring = NULL;
	}

	static void	EndPolygon(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;

		if(me->poly && me->poly->GetClass() == WED_AutogenPlacement::sClass)
		{
			// make a single winding out of all connected windings
			int n_wdg = me->poly->CountChildren() - 1;
			vector<WED_Thing *> wdgs;
			for(int n = 0; n < n_wdg; ++n)
			{
				wdgs.push_back(me->poly->GetNthChild(1));
				wdgs.back()->SetParent(NULL,0);
			}

			if(auto ags = static_cast<WED_AutogenPlacement *>(me->poly))
			{
				//Bbox2 ags_bounds;
				//ags->GetBounds(gis_Geo, ags_bounds);
				//if(!areas_bounds_overlap(ags_bounds)) // this isn't strictly right ... as the points might not be inside the box, but all around it.
				if (!me->is_in_bounds)
				{
					wdgs.push_back(me->poly->GetNthChild(0));
					wdgs.back()->SetParent(NULL, 0);
					for (auto w : wdgs)
					{
						for (int n = w->CountChildren(); n > 0 ; --n)
						{
							auto c = w->GetNthChild(0);
							c->SetParent(NULL, 0);
							c->Delete();
						}
						w->Delete();
					}
					me->poly->SetParent(NULL, 0);
					me->poly->Delete();
					me->poly = NULL;
					return;
				}
			}

			me->ring = me->poly->GetNthChild(0);
			int n_pts = me->ring->CountChildren();
			Point2 first_pt, last_pt;
			dynamic_cast<IGISPoint *>(me->ring->GetNthChild(0))      ->GetLocation(gis_Geo, first_pt);
			dynamic_cast<IGISPoint *>(me->ring->GetNthChild(n_pts-1))->GetLocation(gis_Geo, last_pt);

			while(n_wdg)
			{
				for(auto w = wdgs.begin(); w != wdgs.end(); w++)
				{
					Point2 pt;
					n_pts = (*w)->CountChildren();
					dynamic_cast<IGISPoint *>((*w)->GetNthChild(0))->GetLocation(gis_Geo, pt);
					if(pt == last_pt)
					{
						// insert this contour after existing data
						int last_n = me->ring->CountChildren();
						--last_n;                                        // remove duplicated last point of existing sequence
						WED_Thing * t = me->ring->GetNthChild(last_n);
						t->SetParent(NULL,0);
						t->Delete();
						for(int n = 0; n < n_pts; ++n)
							(*w)->GetNthChild(0)->SetParent(me->ring, n + last_n);
						(*w)->Delete();
						w = wdgs.erase(w);
					    dynamic_cast<IGISPoint *>(me->ring->GetNthChild(last_n + n_pts - 1))->GetLocation(gis_Geo, last_pt);
						break;
					}
					dynamic_cast<IGISPoint *>((*w)->GetNthChild(n_pts-1))->GetLocation(gis_Geo, pt);
					if(pt == first_pt)
					{
						// insert this contour before existing data
						--n_pts;                                       // remove duplicated last point of contour to be added
						WED_Thing * t = (*w)->GetNthChild(n_pts);
						t->SetParent(NULL,0);
						t->Delete();
						for(int n = 0; n < n_pts; ++n)
							(*w)->GetNthChild(0)->SetParent(me->ring, n);
						(*w)->Delete();
						w = wdgs.erase(w);
					    dynamic_cast<IGISPoint *>(me->ring->GetNthChild(0))->GetLocation(gis_Geo, first_pt);
						break;
					}
				}
				-- n_wdg;
			}
			if(first_pt == last_pt)
            {
                WED_Thing *t = me->ring->GetNthChild(me->ring->CountChildren()-1);
                t->SetParent(NULL,0);
                t->Delete();
            }
            else
            {
                LOG_MSG("E/DSFi Polygon outer ring not closed\n");
            }
			// in case some wdg is a real hole, i.e. has no point in common with the outer winding at all - thes are left now
			for(auto w : wdgs)
				w->SetParent(me->poly,1);
		}
		me->poly = NULL;
	}

	static void AddRasterData(
					DSFRasterHeader_t *	header,
					void *				data,
					void *				inRef)
	{
	}

	static void SetFilter(int filterId, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		if(filterId >= 0 && filterId < me->filter_table.size())
			me->filter_on = me->filter_table[filterId];
		else
			me->filter_on = false;
	}

	int do_import_dsf(const char * file_name, WED_Thing * base)
	{
		master_parent = base;
		archive = master_parent->GetArchive();

		DSFCallbacks_t cb = {	NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptRasterDef, AcceptProperty,
								BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
								AddObjectWithMode,
								BeginSegment, AddSegmentShapePoint, EndSegment,
								BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon, AddRasterData, SetFilter, nullptr };

		LOG_MSG("I/DSF Importing binary DSF from %s\n",file_name);
		int res = DSFReadFile(file_name, malloc, free, &cb, NULL, this);

		for(int i = 0; i < dsf_cat_DIM; ++i)
		if(bucket_parents[i])
			bucket_parents[i]->SetParent(master_parent, master_parent->CountChildren());

		return res;
	}

	int do_import_txt(const char * file_name, WED_Thing * base)
	{
		master_parent = base;
		archive = master_parent->GetArchive();

		DSFCallbacks_t cb = {	NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptRasterDef, AcceptProperty,
								BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
								AddObjectWithMode,
								BeginSegment, AddSegmentShapePoint, EndSegment,
								BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon, AddRasterData, SetFilter, nullptr };

		LOG_MSG("I/DSF Importing text DSF from %s\n",file_name);
		int ok = Text2DSFWithWriter(file_name, &cb, this);

		for(int i = 0; i < dsf_cat_DIM; ++i)
		if(bucket_parents[i])
			bucket_parents[i]->SetParent(master_parent, master_parent->CountChildren());

		return ok != 0 ? dsf_ErrOK : dsf_ErrCouldNotReadFile;
	}
};

int DSF_Import(const char * path, WED_Thing * base)
{
	DSF_Importer importer;
	int res = importer.do_import_dsf(path, base);
	if(res == dsf_ErrOK && !importer.is_overlay)
		DoUserAlert("WED is a Overlay Scenery Editor,\nbut the DSF to be imported is Base Mesh Scenery.\n"
		            "All scenery elements only valid for base mesh scenery will be ignored.\n");
	return res;
}

int DSF_Import_Partial(const char * path, WED_Thing * base, int inCatFilter, const vector<Bbox2>& inBounds, const vector<string>& inAptFilter)
{
	DSF_Importer importer;

	importer.cull_bounds = inBounds;
	importer.dsf_cat_filter = inCatFilter;
	importer.dsf_AptID_filter = inAptFilter;

	// do not warn about importing non-overlay DSF's when being this explicit about what to import.
	// e.g. importing roads only is almost certainly always done from base mesh DSFF's.
	return importer.do_import_dsf(path, base);
}

int WED_ImportText(const char * path, WED_Thing * base)
{
	DSF_Importer importer;
	int res = importer.do_import_txt(path, base);

	if(res == dsf_ErrOK && !importer.is_overlay)
		DoUserAlert("WED is a Overlay Scenery Editor,\nbut the DSF to be imported is Base Mesh Scenery.\n"
		            "All scenery elements only valid for base mesh scenery will be ignored.\n");
	return res;
}

#if WED
	// code that uses GUI or Res/Lib/PkgMgr fuctions
	// i.e. stuff that isn't desireable in command-line applications re-using WED code

#include "WED_ToolUtils.h"
#include "WED_UIDefs.h"
#include "WED_PackageMgr.h"

int		WED_CanImportRoads(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if(!sel->IterateSelectionAnd(Iterate_IsClass,(void*) WED_ExclusionZone::sClass)) return 0;

	vector<WED_Thing *> things;
	sel->IterateSelectionOr(Iterate_CollectThings, &things);

	for(auto t : things)
	{
		WED_ExclusionZone * excl = dynamic_cast<WED_ExclusionZone *>(t);
		if(excl == nullptr) return 0;
		set<int> excl_types;
		excl->GetExclusions(excl_types);
		if(excl_types.count(exclude_Net)) return 1;
	}
	return 0;
}

void add_all_global_DSF(const Bbox2& bb, set<string>& matching_dsf)
{
	pair<int, int> glob_scn = gPackageMgr->GlobalPackages();

	for (int lon = floor(bb.xmin()); lon < ceil(bb.xmax()); lon++)
		for (int lat = floor(bb.ymin()); lat < ceil(bb.ymax()); lat++)
			for (int pkg = glob_scn.first; pkg <= glob_scn.second; pkg++)
			{
				string path;
				gPackageMgr->GetNthPackagePath(pkg, path);
				string dirname(FILE_get_file_name(path));
				if (dirname.find("Demo Area") != string::npos || dirname.find("Global Scenery") != string::npos)
				{
					char buf[256];
					snprintf(buf, sizeof(buf), "%s" DIR_STR "Earth nav data" DIR_STR "%+03d%+04d" DIR_STR "%+03d%+04d.dsf", path.c_str(),
						lat > 0 ? (lat / 10) * 10 : ((-lat + 9) / 10) * -10, lon > 0 ? (lon / 10) * 10 : ((-lon + 9) / 10) * -10, lat, lon);
					if (FILE_exists(buf))
					{
						matching_dsf.insert(buf);
						break;
					}
				}
			}
}

void	WED_DoImportRoads(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if(!sel->IterateSelectionAnd(Iterate_IsClass,(void*) WED_ExclusionZone::sClass)) return ;

	vector<WED_Thing *> things;
	sel->IterateSelectionOr(Iterate_CollectThings, &things);

	vector<Bbox2> excl_bounds;
	int dsf_filters = 0;
	for(auto t : things)
	{
		WED_ExclusionZone * excl = dynamic_cast<WED_ExclusionZone *>(t);
		if(excl == nullptr) return;
		set<int> excl_types;
		excl->GetExclusions(excl_types);
		if(excl_types.count(exclude_Net)) dsf_filters |= dsf_filter_roads;
		if(excl_types.count(exclude_Obj)) dsf_filters |= dsf_filter_autogen;
//		if(excl_types.count(exclude_For)) dsf_filters |= dsf_filter_forests;   // general polygon import does not (yet) support partial import
		Bbox2 b;
		excl->GetBounds(gis_Geo,b);
		excl_bounds.push_back(b);
	}

	set<string> matching_dsf;

	for (const auto& bb : excl_bounds)
		add_all_global_DSF(bb, matching_dsf);

	if(matching_dsf.size())
	{
		WED_Thing * wrl = WED_GetWorld(resolver);
		wrl->StartOperation("Import Roads");
		for(auto& path : matching_dsf)
		{
			WED_Group * g = WED_Group::CreateTyped(wrl->GetArchive());
			g->SetName(path);
			g->SetParent(wrl,wrl->CountChildren());
			int result = DSF_Import_Partial(path.c_str(), g, dsf_filters, excl_bounds);
			if(result != dsf_ErrOK)
			{
				string msg = string("The file '") + path + string("' could not be imported as a DSF:\n")
							+ dsfErrorMessages[result];
				DoUserAlert(msg.c_str());
				wrl->AbortOperation();
				return;
			}
		}
		wrl->CommitOperation();
	}
}

int		WED_CanImportDSF(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	WED_Thing * wrl = WED_GetWorld(resolver);
	return 1;
}

void	WED_DoImportDSF(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);

	char * path = GetMultiFilePathFromUser("Import DSF file...", "Import", FILE_DIALOG_IMPORT_DSF);
	if(path)
	{
		char * free_me = path;

		wrl->StartOperation("Import DSF");

		while(*path)
		{
			WED_Group * g = WED_Group::CreateTyped(wrl->GetArchive());
			g->SetName(path);
			g->SetParent(wrl,wrl->CountChildren());
			int result = DSF_Import(path,g);
			if(result != dsf_ErrOK)
			{
				string msg = string("The file '") + path + string("' could not be imported as a DSF:\n")
							+ dsfErrorMessages[result];
				DoUserAlert(msg.c_str());
				wrl->AbortOperation();
				free(free_me);
				return;
			}

			path = path + strlen(path) + 1;
		}
		wrl->CommitOperation();
		free(free_me);
	}
}


static WED_Thing * find_airport_by_icao_recursive(const string& icao, WED_Thing * who)
{
	if(WED_Airport::sClass == who->GetClass())
	{
		WED_Airport * apt = dynamic_cast<WED_Airport *>(who);
		DebugAssert(apt);
		string aicao;
		apt->GetICAO(aicao);

		if(aicao == icao)
			return apt;
		else
			return NULL;
	}
	else
	{
		int n, nn = who->CountChildren();
		for(n = 0; n < nn; ++n)
		{
			WED_Thing * found_it = find_airport_by_icao_recursive(icao, who->GetNthChild(n));
			if(found_it) return found_it;
		}
	}
	return NULL;
}

#endif // #if WED
