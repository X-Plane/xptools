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
#include "WED_Group.h"
#include "WED_ExclusionZone.h"
#include "WED_EnumSystem.h"
#include "WED_ObjPlacement.h"
#include "WED_ToolUtils.h"
#include "PlatformUtils.h"
#include "WED_UIDefs.h"
#include "WED_SimpleBoundaryNode.h"

#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_StringPlacement.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_Ring.h"

#include "WED_TextureBezierNode.h"
#include "WED_TextureNode.h"
#include "WED_SimpleBezierBoundaryNode.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_ForestRing.h"
#include "WED_FacadeRing.h"
#include "WED_FacadeNode.h"
#include "DSF2Text.h"
#include "WED_GISUtils.h"
#include "STLUtils.h"
#include "WED_AptIE.h"
#include "WED_Airport.h"
#if ROAD_EDITING
#include "WED_RoadNode.h"
#include "WED_RoadEdge.h"
#endif
#include "WED_MetadataUpdate.h"

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

inline bool end_match(const char * str, const char * suf)
{
	int ls = strlen(suf);
	int lstr = strlen(str);
	if(lstr > ls)
	{
		return strcmp(str+lstr-ls,suf) == 0;
	}
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
	dsf_cat_orthophoto,
	dsf_cat_draped_poly,
	dsf_cat_roads,
	dsf_cat_DIM
};

static const char * k_dsf_cat_names[dsf_cat_DIM] = { 
	"Exclusion Zones",
	"Objects",
	"Facades",
	"Forests",
	"Lines",
	"Strings",
	"Orthophotos",
	"Draped Polygons",
	"Roads"
};

class	DSF_Importer {
public:

	DSF_Importer() { 
		for(int n = 0; n < 7; ++n) 
			req_level_obj[n] = req_level_agp[n] = req_level_fac[n] = -1; 
		for(int n = 0; n < dsf_cat_DIM; ++n)
			bucket_parents[n] = NULL;
	}

	int					req_level_obj[7];
	int					req_level_agp[7];
	int					req_level_fac[7];

	vector<string>		obj_table;
	vector<string>		pol_table;
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
	
	WED_Thing * get_cat_parent(dsf_import_category cat)
	{
		if(bucket_parents[cat] == NULL)
		{
			bucket_parents[cat] = WED_Group::CreateTyped(archive);
			bucket_parents[cat]->SetName(k_dsf_cat_names[cat]);
		}
		return bucket_parents[cat];
	}

	vector<pair<Point2, int> >	accum_road;
	pair<int, int>		accum_road_type;

#if !NO_NET
	typedef map<pair<int, int>, WED_RoadNode *> road_node_map_t;
	road_node_map_t road_nodes;
	WED_RoadNode *		road_start;
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
		float b[4];
		if(sscanf(ex,"%f/%f/%f/%f",b,b+1,b+2,b+3)==4)
		{
			WED_ExclusionZone * z = WED_ExclusionZone::CreateTyped(archive);
			z->SetName("Exclusion Zone");
			z->SetParent(get_cat_parent(dsf_cat_exclusion),get_cat_parent(dsf_cat_exclusion)->CountChildren());
			set<int> s;
			s.insert(k);
			z->SetExclusions(s);

			WED_SimpleBoundaryNode * p1 =WED_SimpleBoundaryNode::CreateTyped(archive);
			WED_SimpleBoundaryNode * p2 =WED_SimpleBoundaryNode::CreateTyped(archive);
			p1->SetParent(z,0);
			p2->SetParent(z,1);
			p1->SetName("e1");
			p2->SetName("e2");
			p1->SetLocation(gis_Geo,Point2(b[0],b[1]));
			p2->SetLocation(gis_Geo,Point2(b[2],b[3]));
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
		
		if(strcmp(inProp, "sim/require_object") == 0)	me->handle_req_obj(inValue);
		if(strcmp(inProp, "sim/require_agpoint") == 0)	me->handle_req_agp(inValue);
		if(strcmp(inProp, "sim/require_facade") == 0)	me->handle_req_fac(inValue);
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

	static void	AddObject(
					unsigned int	inObjectType,
					double			inCoordinates[4],
					int				inCoordDepth,
					void *			inRef)
	{
#if !NO_OBJ
		DSF_Importer * me = (DSF_Importer *) inRef;
		WED_ObjPlacement * obj = WED_ObjPlacement::CreateTyped(me->archive);
		obj->SetResource(me->obj_table[inObjectType]);
		obj->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
		#if AIRPORT_ROUTING
		if(inCoordDepth == 4)
			obj->SetCustomMSL(inCoordinates[3]);
		else
			obj->SetDefaultMSL();
		#endif
		obj->SetHeading(inCoordinates[2]);
		obj->SetName(me->obj_table[inObjectType]);
		obj->SetParent(me->get_cat_parent(dsf_cat_objects),me->get_cat_parent(dsf_cat_objects)->CountChildren());
		obj->SetShowLevel(me->GetShowForObjID(inObjectType));
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
		DebugAssert(me->accum_road.empty());
		unsigned int inStartNodeID = (unsigned int) inCoordinates[3];
		
		road_node_map_t::iterator rn = me->road_nodes.find(make_pair(inNetworkType, inStartNodeID));
		if(rn == me->road_nodes.end())
		{
			WED_RoadNode * start_node = WED_RoadNode::CreateTyped(me->archive);
			start_node->SetParent(me->get_cat_parent(dsf_cat_roads),me->get_cat_parent(dsf_cat_roads)->CountChildren());
			stringstream ss;
			ss << inStartNodeID;
			start_node->SetLocation(gis_Geo, Point2(inCoordinates[0], inCoordinates[1]));
			start_node->SetName(ss.str());
			me->road_nodes[make_pair(inNetworkType, inStartNodeID)] = start_node;
			me->road_start = start_node;
		}
		else
			me->road_start = rn->second;
		
		me->accum_road_type = make_pair(inNetworkType, inNetworkSubtype);
		me->accum_road.push_back(make_pair(Point2(inCoordinates[0], inCoordinates[1]), int(inCoordinates[2])));
#endif
	}

	static void	AddSegmentShapePoint(
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef)
	{
#if !NO_NET
		DSF_Importer * me = (DSF_Importer *) inRef;
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
		
		unsigned int inEndNodeID = inCoordinates[3];
		
		road_node_map_t::iterator rn = me->road_nodes.find(make_pair(me->accum_road_type.first, inEndNodeID));
		WED_RoadNode * road_end;
		if(rn == me->road_nodes.end())
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
			road_end = rn->second;

		me->accum_road.push_back(make_pair(Point2(inCoordinates[0], inCoordinates[1]), int(inCoordinates[2])));

		DebugAssert(me->accum_road.size() > 1);
		
		int start_level = me->accum_road[0].second;
		int end_level = inCoordinates[2];
		me->accum_road.front().second = 0;
		me->accum_road.back().second = 0;
		
		int last = me->accum_road.size()-1;
		int s = 0;
		WED_RoadNode * last_node = me->road_start;
		while(s < last)
		{
			int n = s+1;
			while(n < me->accum_road.size() && me->accum_road[n].second == 1)
				++n;
			DebugAssert(n <= last);
			int span = n - s;
			DebugAssert(span > 0);
			DebugAssert(span < 4);
			WED_RoadEdge * edge = WED_RoadEdge::CreateTyped(me->archive);
			edge->SetParent(me->get_cat_parent(dsf_cat_roads), me->get_cat_parent(dsf_cat_roads)->CountChildren());
			edge->SetResource(me->net_table[me->accum_road_type.first]);
			edge->SetSubtype(me->accum_road_type.second);
			edge->SetStartLayer(start_level);
			edge->SetEndLayer(end_level);
			edge->AddSource(last_node, 0);
			if(n == last)
			{
				edge->AddSource(road_end, 1);
			}
			else
			{
				WED_RoadNode * shape = WED_RoadNode::CreateTyped(me->archive);
				shape->SetName("shape point");
				shape->SetLocation(gis_Geo, me->accum_road[n].first);
				shape->SetParent(me->get_cat_parent(dsf_cat_roads), me->get_cat_parent(dsf_cat_roads)->CountChildren());
				edge->AddSource(shape, 1);
				last_node = shape;
			}
			
			if(span == 1)
			{
				Segment2 path(me->accum_road[s].first, me->accum_road[n].first);
				edge->SetSide(gis_Geo, path);
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
				edge->SetSideBezier(gis_Geo, path);
			}
			else if(span == 3)
			{
				Bezier2 path(me->accum_road[s].first,
							me->accum_road[s+1].first,
							me->accum_road[s+2].first,
							me->accum_road[s+3].first);
				edge->SetSideBezier(gis_Geo, path);
			}
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
		string r  = me->pol_table[inPolygonType];
		
		me->poly = NULL;
		me->ring = NULL;

		dsf_import_category cat = dsf_cat_objects;
		
#if !NO_FAC
		
		if(end_match(r.c_str(),".fac"))
		{
			// Ben says: .fac must be 2-coord for v9.  But...maybe for v10 we allow curved facades?
			me->want_uv=false;
			me->want_bezier=(inCoordDepth >= 4);
			me->want_wall = (inCoordDepth == 3 || inCoordDepth == 5);
			WED_FacadePlacement * fac = WED_FacadePlacement::CreateTyped(me->archive);
			#if AIRPORT_ROUTING
			fac->SetCustomWalls(me->want_wall);
			#endif
			me->poly = fac;
			me->ring = NULL;
			fac->SetHeight(inParam);
			fac->SetResource(r);
			fac->SetShowLevel(me->GetShowForFacID(inPolygonType));
			cat = dsf_cat_facades;
		}
#endif

#if !NO_FOR
		if(end_match(r.c_str(),".for"))
		{
			me->want_uv=false;
			me->want_bezier=false;
			me->want_wall=false;
			WED_ForestPlacement * forst = WED_ForestPlacement::CreateTyped(me->archive);
			me->poly = forst;
			me->ring = NULL;
			forst->SetDensity((inParam % 256) / 255.0);
			#if AIRPORT_ROUTING
			forst->SetFillMode(inParam / 256);
			#endif
			forst->SetResource(r);
			cat = dsf_cat_forests;
		}
#endif

#if !NO_LIN
		if(end_match(r.c_str(),".lin"))
		{
			me->want_uv=false;
			me->want_bezier=inCoordDepth == 4;
			me->want_wall=false;
			WED_LinePlacement * lin = WED_LinePlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = lin;
			lin->SetClosed(inParam);
			lin->SetResource(r);
			cat = dsf_cat_lines;
		}
#endif

#if !NO_STR
		if(end_match(r.c_str(),".str") || end_match(r.c_str(),".ags"))
		{
			me->want_uv=false;
			me->want_bezier=inCoordDepth == 4;
			me->want_wall=false;
			WED_StringPlacement * str = WED_StringPlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = str;
			str->SetSpacing(inParam);
			str->SetResource(r);
			cat = dsf_cat_strings;
		}
#endif

#if !NO_POL
		if(end_match(r.c_str(),".pol") || end_match(r.c_str(),".agb"))
		{
			me->want_uv=inParam == 65535;
			me->want_bezier=me->want_uv ? (inCoordDepth == 8) : (inCoordDepth == 4);
			me->want_wall=false;
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
				pol->SetHeading(inParam);
				pol->SetResource(r);
				cat = dsf_cat_draped_poly;
			}
			me->ring = NULL;
		}
#endif

		if(me->poly)
		{
			me->poly->SetParent(me->get_cat_parent(cat),me->get_cat_parent(cat)->CountChildren());
			me->poly->SetName(r);
		}
		if(me->ring)
		{
			me->ring->SetParent(me->get_cat_parent(cat),me->get_cat_parent(cat)->CountChildren());
			me->ring->SetName(r);
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
				#if AIRPORT_ROUTING
				if(me->want_wall)
					b->SetWallType(me->walls[n]);
				#endif	
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
			node->SetName("Point");
		}

		
		if (me->poly != NULL)
			me->ring = NULL;
	}

	static void	EndPolygon(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
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
	}

	int do_import_dsf(const char * file_name, WED_Thing * base)
	{
		master_parent = base;
		archive = master_parent->GetArchive();

		DSFCallbacks_t cb = {	NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptRasterDef, AcceptProperty,
								BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
								AddObject,
								BeginSegment, AddSegmentShapePoint, EndSegment,
								BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon, AddRasterData, SetFilter };

		int res = DSFReadFile(file_name, malloc, free, &cb, NULL, this);
		
		for(int i = 0; i < dsf_cat_DIM; ++i)
		if(bucket_parents[i])
			bucket_parents[i]->SetParent(master_parent, master_parent->CountChildren());
		
		return res;
	}

	void do_import_txt(const char * file_name, WED_Thing * base)
	{
		master_parent = base;
		archive = master_parent->GetArchive();

		DSFCallbacks_t cb = {	NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptRasterDef, AcceptProperty,
								BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
								AddObject,
								BeginSegment, AddSegmentShapePoint, EndSegment,
								BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon, AddRasterData, SetFilter };

		int ok = Text2DSFWithWriter(file_name, &cb, this);
		
		for(int i = 0; i < dsf_cat_DIM; ++i)
		if(bucket_parents[i])
			bucket_parents[i]->SetParent(master_parent, master_parent->CountChildren());
		

//		int res = DSFReadFile(file_name, &cb, NULL, this);
//		if(res != 0)
//			printf("DSF Error: %d\n", res);
	}


};


int DSF_Import(const char * path, WED_Group * base)
{
	DSF_Importer importer;
	return importer.do_import_dsf(path, base);
}

int		WED_CanImportDSF(IResolver * resolver)
{
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

void WED_DoImportText(const char * path, WED_Thing * base)
{
	DSF_Importer importer;
	importer.do_import_txt(path, base);
}


#if GATEWAY_IMPORT_FEATURES
const string get_airport_id_from_gateway_file_path(const char * file_path)
{
	string tname(file_path);
	string::size_type p = tname.find_last_of("\\/");
	if(p != tname.npos)
		tname = tname.substr(p+1);
	p = tname.find_last_of(".");
	if(p != tname.npos)
		tname = tname.substr(0,p);
	return tname;
}
WED_Thing * get_airport_from_gateway_file_path(const char * file_path, WED_Thing * wrl)
{
	return find_airport_by_icao_recursive(get_airport_id_from_gateway_file_path(file_path), wrl);
}


//This is from an older method of importing things which involved manually getting the files from the hard drive
void	WED_DoImportDSFText(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);

	char * paths = GetMultiFilePathFromUser("Import DSF file...", "Import", FILE_DIALOG_IMPORT_DSF);
	if(paths)
	{
		char * free_me = paths;
		
		wrl->StartOperation("Import DSF");
		
		while(*paths)
		{
			if(strstr(paths,".dat"))
			{			
				WED_ImportOneAptFile(paths,wrl,NULL);
				WED_DoInvisibleUpdateMetadata(SAFE_CAST(WED_Airport, get_airport_from_gateway_file_path(paths, wrl)));
			}
			paths = paths + strlen(paths) + 1;
		}
		
		paths = free_me;

		while(*paths)
		{
			if(!strstr(paths,".dat"))
			{
				WED_Thing * g = get_airport_from_gateway_file_path(paths, wrl);
				if(g == NULL)
				{
					g = WED_Group::CreateTyped(wrl->GetArchive());
					g->SetName(paths);
					g->SetParent(wrl,wrl->CountChildren());
				}
		//		DSF_Import(path,g);
				DSF_Importer importer;
				importer.do_import_txt(paths, g);
			}	
			paths = paths + strlen(paths) + 1;
		}
		
		wrl->CommitOperation();
		free(free_me);
	}
}
#endif
