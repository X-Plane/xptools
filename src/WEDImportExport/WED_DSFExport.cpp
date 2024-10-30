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

#include "WED_DSFExport.h"
#include "WED_OrthoExport.h"

#include "WED_Entity.h"
#include "WED_Group.h"
#include "WED_TextureNode.h"
#include "WED_ObjPlacement.h"
#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_StringPlacement.h"
#include "WED_AutogenPlacement.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_TerPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_ExclusionZone.h"
#include "WED_ExclusionPoly.h"
#include "WED_RoadEdge.h"
#include "WED_Airport.h"

#include "IResolver.h"
#include "WED_HierarchyUtils.h"
#include "WED_ToolUtils.h"
#include "WED_ResourceMgr.h"

#include "PlatformUtils.h"
#include "FileUtils.h"
#include "MathUtils.h"
#include "GISUtils.h"
#include "CompGeomDefs2.h"
#include "WED_Version.h"
#include "WED_EnumSystem.h"
#include "WED_Clipping.h"
#include "DSFLib.h"
#include "DSF2Text.h"
#include "zip.h"
#include <stdarg.h>


#if DEV
#include "PerfUtils.h"
#endif

// This is how much outside the DSF bounds we can legally go.
// Between you, me, and the wall, X-Plane 10.21 actually allows
// a full 0.5 degrees of 'extra'.  But...that's a LOT more than we
// should ever need.  So: set our safe bounds WELL inside X-Plane's.
// If we lose a bit or 2 in the DSF encoder, we won't have written
// something on the ragged edge.
#define DSF_EXTRA_1021 0.25

//---------------------------------------------------------------------------------------------------------------------------------------

int zip_printf(void * fi, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char tmp[4000];
	int l = vsprintf(tmp,fmt,args);
	va_end(args);

	zipWriteInFileInZip((zipFile) fi, tmp, l);
	return l;
}

//---------------------------------------------------------------------------------------------------------------------------------------

static unsigned int encode_heading(double h)     // draped polygons have 1/128th degree roation encoded in integer headings
{
	double          wrapped = dobwrap(h, 0.0, 360.0);
	unsigned int  whole_deg = wrapped;
	double         frac_deg = wrapped - whole_deg;

	return whole_deg + 360u * (unsigned int)(128.0 * frac_deg);
}

static unsigned int encode_spacing(double s)     // object strings have 1/128th resolution encoded in integer spacing
{
	if (gExportTarget > wet_xplane_1200 && s < 433.8)
	{
		unsigned int nearest = round(s);
		double ratio = nearest == 0 ? 0.0 : s / (double) nearest;
		if (ratio > 0.98 && ratio < 1.02)        // really close to full meters, keep using plain int, best XP11 compatibility
			return nearest;
		else
			return 10000u + (unsigned int) round(128.0 * s);
	}
	else
		return s;
}

template <class __Iterator>
__Iterator find_contiguous_beziers(__Iterator b, __Iterator e)
{
	if(b == e) return e;
	__Iterator n(b);
	++n;
	while(n != e && n->p1 == b->p2)
	{
		++n;
		++b;
	}
	return n;
}

static bool one_winding(const vector<Segment2>& v)
{
	for(int n = 1; n < v.size(); ++n)
		if(v[n-1].p2 != v[n].p1)
			return false;
	return true;
}

static bool one_winding(const vector<Bezier2>& v)
{
	for(int n = 1; n < v.size(); ++n)
		if(v[n-1].p2 != v[n].p1)
			return false;
	return true;
}

// Here's the problem(s):
// 1. X-Plane won't open a DSF whose points are outside the tile boundaries.
// 2. CGAL sometimes gives us a point that is fractionally outside the DSF due to a rounding error.
// So: apply clamping, but flag when we are off by more than 1 meter.  (CGAL is significantly better
// than this.
//#define MAX_OUTSIDE 0.00001


static void gentle_crop(Point2& p, const Bbox2& bounds, bool& hard_crop)
{
	Point2 op(p);
	p = bounds.clamp(p);
	if(op != p)
		hard_crop = true;
}

template <typename T>
bool bad_match(const T& s1, const T& s2)
{
	return s1.p1 != s2.p2;
}

struct kill_zero_length_segment {

	bool operator()(const Segment2& s) const { return s.p1 == s.p2; }

};

// We normally reject zero length segments, but for the sake of grandfathered global airports, we'll instead simply remove them on export.
template<class Segment>
void remove_all_zero_length_segments(vector<Segment> &in_out_chain)
{
	in_out_chain.erase(remove_if(in_out_chain.begin(), in_out_chain.end(), kill_zero_length_segment()), in_out_chain.end());
}

/************************************************************************************************************************************************
 * ROAD PROCESSOR
 ************************************************************************************************************************************************/
#if ROAD_EDITING
class dsf_road_grid_helper {
public:

	struct node {
		vector<int>	edges;
		int			id;
	};
	struct edge {
		int start_node;
		int	end_node;
		int start_level;
		int end_level;
		int subtype;
		vector<Bezier2>	path;

		bool level_is_uniform() { return start_level == end_level; }
		int level_for_node(int n)
		{
			DebugAssert(n == start_node || n == end_node);
			if (n == start_node)return start_level;
			else				return end_level;
		}
	};

	typedef map<IGISPoint *, int>	node_index;

	node_index				m_node_index;
	vector<node>			m_nodes;
	vector<edge>			m_edges;

	void add_segment(WED_RoadEdge * e, const Bbox2& cull_bounds);

	void remove_dupes();
	void assign_ids(int& idx);
	void export_to_dsf(	int&					node_offset,
						int						net_type,
						const DSFCallbacks_t *	cbs,
						void *					writer);
};

void dsf_road_grid_helper::add_segment(WED_RoadEdge * e, const Bbox2& cull_bounds)
{
	int n = e->GetNumSides();

	IGISPoint * gp_start = e->GetNthPoint(0);
	IGISPoint * gp_end = e->GetNthPoint(n);

	Point2 sp,ep;
	gp_start->GetLocation(gis_Geo,sp);
	gp_end->GetLocation(gis_Geo,ep);

	Bezier2 b ;
	vector<Bezier2> segm;
	for(int s = 0; s < n ;++s)
	{
		if(e->GetSide(gis_Geo, s , b)) // We are a real bezier:  make always qubic
		{
			if(b.p1 == b.c1)
			{
				b.c1 = b.p1 + b.derivative(0.01);
			}
			else if(b.p2 == b.c2)
			{
				b.c2 = b.p2 - b.derivative(0.99);
			}
		}
		segm.push_back(b);
	}

	clip_segments(segm,cull_bounds);
	if(segm.empty()) return;					//the whole segment is out of the bounds or something else is wrong

	auto part_st  = segm.begin();
	auto segm_end = segm.end();

	while(part_st != segm_end)
	{
		auto part_en = find_contiguous_beziers(part_st,segm_end);
		if(part_st != part_en)
		{
			edge new_edge;
			new_edge.subtype = e->GetSubtype();

		    new_edge.path.assign(part_st,part_en);

			if(new_edge.path.front().p1 == sp)							//start point is unchanged , not cutted
			{
				new_edge.start_level = e->GetStartLayer();
				node_index::iterator si = m_node_index.find(gp_start);
				if(si == m_node_index.end())
				{
					new_edge.start_node = m_nodes.size();
					si = m_node_index.insert(make_pair(gp_start, (int) m_nodes.size())).first;
					m_nodes.push_back(node());
				}
				else
				{
					new_edge.start_node = si->second;
				}
				m_nodes[si->second].edges.push_back((int) m_edges.size());
			}
			else											//start point is new after culling , creating new own node
			{
				new_edge.start_level = e->GetEndLayer();    // ToDo: mroe: revisit , using original endlayer for every newly split node
				new_edge.start_node = m_nodes.size();
				m_nodes.push_back(node());
				m_nodes[m_nodes.size()-1].edges.push_back((int) m_edges.size());
			}

			if( new_edge.path.back().p2 == ep)							//end point is unchanged , not cutted
			{
				new_edge.end_level = e->GetEndLayer();
				node_index::iterator ei = m_node_index.find(gp_end);
				if(ei == m_node_index.end())
				{
					new_edge.end_node = m_nodes.size();
					ei = m_node_index.insert(make_pair(gp_end, (int) m_nodes.size())).first;
					m_nodes.push_back(node());
				}
				else
				{
					new_edge.end_node = ei->second;
				}
				m_nodes[ei->second].edges.push_back((int) m_edges.size());
			}
			else											//end point is new after culling , creating new own node
			{
				new_edge.end_level = e->GetEndLayer();
				new_edge.end_node = m_nodes.size();
				m_nodes.push_back(node());
				m_nodes[m_nodes.size()-1].edges.push_back((int) m_edges.size());
			}

			m_edges.push_back(new_edge);
		}

		part_st = part_en;
	}
}

void dsf_road_grid_helper::remove_dupes()
{
	// For each node, we identify a node of degree 2 with an outgoing and incoming road
	// that are exactly the same.  We merge around us and clear us out.
	for(int n = 0; n < m_nodes.size(); ++n)
	{
		if(m_nodes[n].edges.size() == 2)
		{
			int e1 = m_nodes[n].edges[0];
			int e2 = m_nodes[n].edges[1];
			if(e1 != e2)						// If we have a loop (a contour going to itself) we can't merge.
			{
				edge * ee1 = &m_edges[e1];
				edge * ee2 = &m_edges[e2];

				if(ee1->level_is_uniform() && ee2->level_is_uniform())
				if(ee1->level_for_node(n) == ee2->level_for_node(n) && ee1->subtype == ee2->subtype)	// Only merge if level and subtype matches
				{
					DebugAssert(ee1->start_node == n || ee1->end_node == n);
					DebugAssert(ee2->start_node == n || ee2->end_node == n);
					DebugAssert(ee1->start_node != n || ee1->end_node != n);
					DebugAssert(ee2->start_node != n || ee2->end_node != n);

					int incoming1 = ee1->end_node == n;
					int incoming2 = ee2->end_node == n;

					if(incoming1 != incoming2)									// Only merge if one is in and one is out - so direction isn't reversed.
					{
						if(!incoming1)
						{
							// merge 2 then 1
							DebugAssert(ee2->end_node == ee1->start_node);
							swap(ee1, ee2);
							swap(e1, e2);
						}

						// Now we have e1->e2 for sure
						// merge 1 then 2
						DebugAssert(ee1->end_node == ee2->start_node);
						if(ee1->start_node == ee2->end_node ) continue;  //we have a loop (a contour going to itself) we can't merge.

						// Find the final destination of e2, and mark it as having e1 coming in.
						node * dest = &m_nodes[ee2->end_node];
						vector<int>::iterator i = find(dest->edges.begin(), dest->edges.end(), e2);
						DebugAssert(i != dest->edges.end());
						if(i != dest->edges.end())
						{
							*i = e1;
						}

						// Now merge - ee1 get's ee2's path and destination

						ee1->path.insert(ee1->path.end(),ee2->path.begin(), ee2->path.end());
						ee1->end_node = ee2->end_node;
						ee1->end_level = ee2->end_level;

						// Mark ee2 as unused - path is clear, no connectivity
						ee2->start_node = -1;
						ee2->end_node = -1;
						ee2->path.clear();

						// Mark ourselves as dead - no adjacent edges
						m_nodes[n].edges.clear();
					}
				}
			}
		}
	}
}


void dsf_road_grid_helper::assign_ids(int& idx)
{
	for(vector<node>::iterator n = m_nodes.begin(); n != m_nodes.end(); ++n)
	{
		if(n->edges.empty())
			n->id = 0;
		else
			n->id = idx++;
	}
}

void dsf_road_grid_helper::export_to_dsf(
	int&					node_offs,
	int						net_type,
	const DSFCallbacks_t *	cbs,
	void *					writer)
{
	remove_dupes();
	assign_ids(node_offs);

	double coords[4];

	for(vector<edge>::iterator e = m_edges.begin(); e != m_edges.end(); ++e)
	if(!e->path.empty())
	{
		for(vector<Bezier2>::iterator b = e->path.begin(); b != e->path.end(); ++b)
		{
			coords[0] = b->p1.x();
			coords[1] = b->p1.y();

			if(b == e->path.begin())
			{
				coords[2] = e->start_level;
				coords[3] = m_nodes[e->start_node].id;
				cbs->BeginSegment_f(
								net_type,
								e->subtype,
								coords,
								false,
								writer);
			}
			else
			{
				coords[2] = 0;
				cbs->AddSegmentShapePoint_f(coords, false, writer);
			}

			if(b->p1 != b->c1 || b->p2 != b->c2)
			{
				coords[2] = 1;
				coords[0] = b->c1.x();
				coords[1] = b->c1.y();
				cbs->AddSegmentShapePoint_f(coords, false, writer);
				coords[0] = b->c2.x();
				coords[1] = b->c2.y();
				cbs->AddSegmentShapePoint_f(coords, false, writer);
			}
		}

		coords[2] = e->end_level;
		coords[0] = e->path.back().p2.x();
		coords[1] = e->path.back().p2.y();
		coords[3] = m_nodes[e->end_node].id;
		cbs->EndSegment_f(coords, false, writer);
	}
}
#endif


/************************************************************************************************************************************************
 * DSF EXPORT UTILS
 ************************************************************************************************************************************************/

// This is the number of sub-buckets to build in the DSF writer...the larger, the more precise the DSF, but the more likely
// some big item goes across buckets and we lose precision.
#define DSF_DIVISIONS 32

static bool g_dropped_pts = false;

struct	DSF_ResourceTable {
	DSF_ResourceTable() { for(int i = 0; i < 7; ++i) show_level_obj[i] = show_level_pol[i] = -1; cur_filter = -1;}
	vector<string>				obj_defs;
	map<pair<string,int>, int>	obj_defs_idx;

	vector<string>				pol_defs;
	map<pair<string, int>, int>	pol_defs_idx;

#if ROAD_EDITING
	vector<string>				net_defs;
	map<pair<string, int>, pair<int, dsf_road_grid_helper> > net_defs_idx;
#endif

	vector<string>				filters;
	map<string, int>			filter_idx;

	map<int, vector<pair<string, string> > > exclusions;

	int							cur_filter;

	int show_level_obj[7];
	int show_level_pol[7];

	void set_filter(int x) { cur_filter = x; }

	void accum_exclusion(const string& key, const string& value)
	{
		exclusions[cur_filter].push_back(make_pair(key,value));
	}

	int accum_obj(const string& f, int show_level)
	{
		map<pair<string,int>,int>::iterator i = obj_defs_idx.find(pair<string,int>(f,show_level));
		if (i != obj_defs_idx.end()) return i->second;
		obj_defs_idx[pair<string,int>(f,show_level)] = obj_defs.size();
		if(show_level_obj[show_level] == -1)	show_level_obj[show_level] = obj_defs.size();
		DebugAssert(show_level_obj[show_level] <= obj_defs.size());
		obj_defs.push_back(f);
		return			  obj_defs.size()-1;
	}

	int accum_pol(const string& f, int show_level)
	{
		map<pair<string,int>,int>::iterator i = pol_defs_idx.find(pair<string,int>(f,show_level));
		if (i != pol_defs_idx.end()) return i->second;
		pol_defs_idx[pair<string,int>(f,show_level)] = pol_defs.size();
		if(show_level_pol[show_level] == -1)	show_level_pol[show_level] = pol_defs.size();
		DebugAssert(show_level_pol[show_level] <= pol_defs.size());
		pol_defs.push_back(f);
		return				  pol_defs.size()-1;
	}
#if ROAD_EDITING
	dsf_road_grid_helper * accum_net(const string& f, int idx)
	{
		int ret = 0;
		auto i = net_defs_idx.find(make_pair(f, idx));
		if(i != net_defs_idx.end())
			return &i->second.second;
		else
		{
			i = net_defs_idx.begin();
			while(i != net_defs_idx.end())
			{
				if(i->first.first == f) break;
				i++;
			}
			if(i == net_defs_idx.end())
			{
				net_defs_idx[make_pair(f, idx)] = make_pair((int) net_defs.size(), dsf_road_grid_helper());
				net_defs.push_back(f);
			}
			else
			{
				net_defs_idx[make_pair(f, idx)] = make_pair(i->second.first, dsf_road_grid_helper());
			}
			return &net_defs_idx[make_pair(f, idx)].second;
		}
	}
#endif
	int accum_filter(const string& icao_filter)
	{
		string icao_filter_uc(icao_filter);      // X-Plane auto-capitalizes apt.dat entries upon reading and compares filters case-sensitive ...
		for_each(icao_filter_uc.begin(), icao_filter_uc.end(), [](char & c) { c = toupper(c); });

		map<string, int>::iterator i = filter_idx.find(icao_filter_uc);
		if (i != filter_idx.end()) return i->second;
		filter_idx[icao_filter_uc] = filters.size();
		filters.push_back(icao_filter_uc);
		return filters.size()-1;
	}

	void write_tables(DSFCallbacks_t& cbs, void * writer)
	{
		for(vector<string>::iterator s = obj_defs.begin(); s != obj_defs.end(); ++s)
			cbs.AcceptObjectDef_f(s->c_str(), writer);

		for(vector<string>::iterator s = pol_defs.begin(); s != pol_defs.end(); ++s)
			cbs.AcceptPolygonDef_f(s->c_str(), writer);

		for(vector<pair<string,string> >::iterator e = exclusions[-1].begin(); e != exclusions[-1].end(); ++e)
			cbs.AcceptProperty_f(e->first.c_str(), e->second.c_str(), writer);
#if ROAD_EDITING
		for(auto s : net_defs)
		{
			int road_node = 1;                            // number networks per network definition
			cbs.AcceptNetworkDef_f(s.c_str(), writer);
			for(auto m : net_defs_idx)
				if(m.first.first == s)
				{
					cbs.SetFilter_f(m.first.second, writer);
					m.second.second.export_to_dsf(road_node, m.second.first, &cbs, writer);
				}
		}
#endif
		int idx = 0;
		for(vector<string>::iterator s = filters.begin(); s != filters.end(); ++s, ++idx)
		{
			cbs.AcceptProperty_f("sim/filter/aptid",s->c_str(),writer);

			for(vector<pair<string,string> >::iterator e = exclusions[idx].begin(); e != exclusions[idx].end(); ++e)
				cbs.AcceptProperty_f(e->first.c_str(), e->second.c_str(), writer);
		}
		for(int i = 1; i <= 6; ++i)
		{
			char buf[20];
			if(show_level_obj[i] != -1)
			{
				sprintf(buf,"%d/%d",i,show_level_obj[i]);
				cbs.AcceptProperty_f("sim/require_agpoint", buf, writer);
				cbs.AcceptProperty_f("sim/require_object", buf, writer);
			}
			if(show_level_pol[i] != -1)
			{
				sprintf(buf,"%d/%d",i,show_level_pol[i]);
				cbs.AcceptProperty_f("sim/require_facade", buf, writer);
			}
		}

	}
};

// stacking is lon, lat, control_lon, control_lat
void assemble_dsf_pt(double c[4], const Point2& pt, const Point2 * bez, const Bbox2& bounds)
{
	Point2	p = pt;
	gentle_crop(p, bounds, g_dropped_pts);
	c[0] = p.x();
	c[1] = p.y();

	if(bez)
	{
		Point2 b = *bez;
		gentle_crop(b, bounds, g_dropped_pts);
		c[2] = b.x();
		c[3] = b.y();
	}
}

// DSF stacking is lon, lat, param, u, v
void assemble_dsf_pt_param(double c[8], const Point2& pt, int pt_param, const Point2 * bez, const Bbox2& bounds)
{
	Point2	p = pt;
	gentle_crop(p, bounds, g_dropped_pts);
	c[0] = p.x();
	c[1] = p.y();

	if(bez)
	{
		Point2 b = *bez;
		gentle_crop(b, bounds, g_dropped_pts);
		c[3] = b.x();
		c[4] = b.y();
	}
	c[2] = pt_param;
}

// stacking is either:
// lon, lat, u, v or
// lon, lat, control lon, control lat, u, v, control u, control v
void assemble_dsf_pt_uv(double c[8], const Point2& pt, const Point2& uv, const Point2 * bez, const Point2 * bez_uv, const Bbox2& bounds)
{
	Point2	p = pt;
	gentle_crop(p, bounds, g_dropped_pts);
	c[0] = p.x();
	c[1] = p.y();

	DebugAssert((bez == NULL) == (bez_uv == NULL));

	if(bez)
	{
		Point2 b = *bez;
		gentle_crop(b, bounds, g_dropped_pts);
		c[2] = b.x();
		c[3] = b.y();

		c[4] = uv.x();
		c[5] = uv.y();
		c[6] = bez_uv->x();
		c[7] = bez_uv->y();
	}
	else
	{
		c[2] = uv.x();
		c[3] = uv.y();
	}
}



static void	DSF_AccumChainBezier(
						vector<Bezier2>::const_iterator	start,
						vector<Bezier2>::const_iterator	end,
						const Bbox2&							bounds,
						const DSFCallbacks_t *					cbs,
						void *									writer,
						int										idx,
						int										param,
						int										auto_closed)
{
	vector<Bezier2>::const_iterator n = start;

	while(n != end)
	{
		vector<Bezier2>::const_iterator e = find_contiguous_beziers(n,end);
		if(n != end)
		{
			cbs->BeginPolygon_f(idx, param, 4, writer);
			cbs->BeginPolygonWinding_f(writer);

			double c[4];
			vector<BezierPoint2>	pts,pts_triple;
			BezierToBezierPointSeq(n,e,back_inserter(pts));
			BezierPointSeqToTriple(pts.begin(),pts.end(),back_inserter(pts_triple));

			for(int i = 0; i < pts_triple.size(); ++i)
			{
				assemble_dsf_pt(c, pts_triple[i].pt, &pts_triple[i].hi, bounds);
				if(!auto_closed || i != (pts_triple.size()-1))
				{
					cbs->AddPolygonPoint_f(c,writer);
				}
			}

			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
		}
		n = e;
	}

}

static void	DSF_AccumChainBezier(
						vector<Bezier2p>::const_iterator		start,
						vector<Bezier2p>::const_iterator		end,
						const Bbox2&							bounds,
						const DSFCallbacks_t *					cbs,
						void *									writer,
						int										idx,
						int										param,
						int										auto_closed,
						double									lastPt_param)
{
	vector<Bezier2p>::const_iterator n = start;

	while(n != end)
	{
		vector<Bezier2p>::const_iterator e = find_contiguous_beziers(n,end);
		if(n != end)
		{
			cbs->BeginPolygon_f(idx, param, 5, writer);
			cbs->BeginPolygonWinding_f(writer);

			double c[5];
			vector<BezierPoint2p>	pts,pts_triple;
			BezierToBezierPointSeq(n,e,back_inserter(pts));
			BezierPointSeqToTriple(pts.begin(),pts.end(),back_inserter(pts_triple));

			for(int i = 0; i < pts_triple.size(); ++i)
			{
				assemble_dsf_pt_param(c,
						pts_triple[i].pt,
						pts_triple[i].param,
						&pts_triple[i].hi,
						bounds);
				if(!auto_closed || i != (pts_triple.size()-1))
				{
					if(e == end && i == (pts_triple.size() - 1))
						c[2] = lastPt_param;
					if (e == end && i == (pts_triple.size() - 2) && pts_triple[i].pt == pts_triple[i+1].pt)
						c[2] = lastPt_param;
					cbs->AddPolygonPoint_f(c,writer);
				}
			}

			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
		}
		n = e;
	}

}


static void	DSF_AccumChain(
						vector<Segment2>::const_iterator	start,		// This is a list of segments that may or may not
						vector<Segment2>::const_iterator	end,		// be end-to-end.
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer,
						int									idx,
						int									param,
						int									auto_closed)		// If true, the entity we are writing is implicitly 'closed' in the DSF - no need to repush the first point as the last
{
	vector<Segment2>::const_iterator next;
	for(vector<Segment2>::const_iterator i = start; i != end; ++i)
	{
		next = i;
		++next;
		double c[4];

		if(i == start)																	// First segment?  Open the primitive.
		{
			cbs->BeginPolygon_f(idx, param, 2, writer);
			cbs->BeginPolygonWinding_f(writer);
		}

		assemble_dsf_pt(c, i->source(), NULL, bounds);							// Start point _always_ written - it has the line type.
		cbs->AddPolygonPoint_f(c,writer);

		if(next != end && i->target() != next->source())								// Discontinuity mid-line?  Write the end, close and open.
		{
			assemble_dsf_pt(c, i->target(), NULL, bounds);
			cbs->AddPolygonPoint_f(c,writer);
			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
			cbs->BeginPolygon_f(idx, param, 2, writer);
			cbs->BeginPolygonWinding_f(writer);
		}
		else if(next == end && (i->target() != start->source() || !auto_closed))		// If we are ending AND we need a last point, write it.
		{																				// We need that last pt if we are not closed or if the
			assemble_dsf_pt(c, i->target(), NULL, bounds);						// closure is not part of the DSF
			cbs->AddPolygonPoint_f(c,writer);
		}

		DebugAssert(!(next == end && i->target() != start->source() && auto_closed));	// If we are on the last segment but discontinuous back to the start AND we auto close, it's an error by the caller - segment lists that are auto-close must be, well, loops!

		if(next == end)																	// Always cap at end
		{
			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
		}
	}
}

static void	DSF_AccumChain(
						vector<Segment2p>::const_iterator	start,
						vector<Segment2p>::const_iterator	end,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer,
						int									idx,
						int									param,
						int									auto_closed,
						double								lastPt_param)
{
	vector<Segment2p>::const_iterator next;
	for(vector<Segment2p>::const_iterator i = start; i != end; ++i)
	{
		next = i;
		++next;
		double c[4];

		if(i == start)
		{
			cbs->BeginPolygon_f(idx, param, 3, writer);
			cbs->BeginPolygonWinding_f(writer);
		}

		assemble_dsf_pt(c, i->source(), NULL, bounds);
		c[2] = i->param;
		cbs->AddPolygonPoint_f(c,writer);

		if(next != end && i->target() != next->source())
		{
			assemble_dsf_pt(c, i->target(), NULL, bounds);
			c[2] = i->param;
			cbs->AddPolygonPoint_f(c,writer);
			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
			cbs->BeginPolygon_f(idx, param, 3, writer);
			cbs->BeginPolygonWinding_f(writer);
		}
		else if(next == end && (i->target() != start->source() || !auto_closed))
		{
			assemble_dsf_pt(c, i->target(), NULL, bounds);
			c[2] = lastPt_param;
			cbs->AddPolygonPoint_f(c,writer);
		}

		if(next == end)
		{
			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
		}
	}
}


void DSF_AccumPts(		vector<Point2>::const_iterator		begin,
						vector<Point2>::const_iterator		end,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer,
						int									idx,
						int									param)
{
	cbs->BeginPolygon_f(idx, param, 2, writer);
	cbs->BeginPolygonWinding_f(writer);
	double c[2];
	for(vector<Point2>::const_iterator i = begin; i != end; ++i)
	{
		assemble_dsf_pt(c, *i, NULL, bounds);
		cbs->AddPolygonPoint_f(c,writer);
	}

	cbs->EndPolygonWinding_f(writer);
	cbs->EndPolygon_f(writer);
}

/************************************************************************************************************************************************
 * DSF EXPORT CENTRAL
 ************************************************************************************************************************************************/

void DSF_AccumPolygonBezier(
						BezierPolygon2&						poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	vector<BezierPoint2>	pts, pts3;

	BezierToBezierPointSeq(poly.begin(),poly.end(),back_inserter(pts));
	BezierPointSeqToTriple(pts.begin(),pts.end(),back_inserter(pts3));

	DebugAssert(pts3.front() == pts3.back());
	pts3.pop_back();

	for(int p = 0; p < pts3.size(); ++p)
	{
		double crd[8];

		assemble_dsf_pt(crd, pts3[p].pt, &pts3[p].hi, bounds);
		cbs->AddPolygonPoint_f(crd,writer);
	}
}

void DSF_AccumPolygonBezier(
						const BezierPolygon2p&				poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	vector<BezierPoint2p>	pts, pts3;

	BezierToBezierPointSeq(poly.begin(),poly.end(),back_inserter(pts));
	BezierPointSeqToTriple(pts.begin(),pts.end(),back_inserter(pts3));

	pts3.pop_back();

	for(int p = 0; p < pts3.size(); ++p)
	{
		double crd[8];

		assemble_dsf_pt_param(crd, pts3[p].pt, pts3[p].param, &pts3[p].hi, bounds);
		cbs->AddPolygonPoint_f(crd,writer);
	}
}

void DSF_AccumPolygonBezier(
						const BezierPolygon2uv&				poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	vector<BezierPoint2uv>	pts, pts3;

	BezierToBezierPointSeq(poly.begin(),poly.end(),back_inserter(pts));
	BezierPointSeqToTriple(pts.begin(),pts.end(),back_inserter(pts3));

	pts3.pop_back();

	for(int p = 0; p < pts3.size(); ++p)
	{
		double crd[8];

		assemble_dsf_pt_uv(crd, pts3[p].pt, pts3[p].uv.pt, &pts3[p].hi, &pts3[p].uv.hi, bounds);
		cbs->AddPolygonPoint_f(crd,writer);
	}
}

void DSF_AccumPolygon(
						Polygon2&							poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(Polygon2::iterator v = poly.begin(); v != poly.end(); ++v)
	{
		double c[4];
		assemble_dsf_pt(c, *v, NULL, bounds);
		cbs->AddPolygonPoint_f(c,writer);
	}
}

void DSF_AccumPolygon(
						const vector<Segment2p>&			poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(vector<Segment2p>::const_iterator s = poly.begin(); s != poly.end(); ++s)
	{
		double c[4];
		assemble_dsf_pt_param(c, s->source(), s->param, NULL, bounds);
		cbs->AddPolygonPoint_f(c,writer);
	}
}

void DSF_AccumPolygon(
						const vector<Segment2uv>&			poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(vector<Segment2uv>::const_iterator s = poly.begin(); s != poly.end(); ++s)
	{
		double c[4];
		assemble_dsf_pt_uv(c, s->p1, s->uv.p1, NULL, NULL, bounds);
		cbs->AddPolygonPoint_f(c,writer);
	}
}


void DSF_AccumPolygonWithHolesBezier(
						vector<BezierPolygon2>&				poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(vector<BezierPolygon2>::iterator w = poly.begin(); w != poly.end(); ++w)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygonBezier(*w, bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}
}

void DSF_AccumPolygonWithHolesBezier(
						vector<BezierPolygon2p>&			poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(int n = 0; n < poly.size(); ++n)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygonBezier(poly[n], bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}
}

void DSF_AccumPolygonWithHolesBezier(
						vector<BezierPolygon2uv>&			poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(int n = 0; n < poly.size(); ++n)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygonBezier(poly[n], bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}
}


void DSF_AccumPolygonWithHoles(
						vector<Polygon2>&					poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(vector<Polygon2>::iterator h = poly.begin(); h != poly.end(); ++h)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygon(*h, bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}

}

void DSF_AccumPolygonWithHoles(
						vector<Polygon2p>&					poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(int n = 0; n < poly.size(); ++n)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygon(poly[n], bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}
}

void DSF_AccumPolygonWithHoles(
						vector<Polygon2uv>&					poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs,
						void *								writer)
{
	for(int n = 0; n < poly.size(); ++n)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygon(poly[n], bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}
}


// 1 = got at least 1 min/max height entity
// 0 = got entities, none affected by height
// -1 = cull
static int	DSF_HeightRangeRecursive(WED_Thing * what, double& out_msl_min, double& out_msl_max, const Bbox2& bounds)
{
	IGISEntity * ent;
	if((ent = dynamic_cast<IGISEntity *>(what)) != NULL)
	{
		Bbox2	cbounds;
		ent->GetBounds(gis_Geo, cbounds);
		if(!cbounds.overlap(bounds))
			return -1;
	}

	sClass_t c = what->GetClass();

	if(c == WED_ObjPlacement::sClass)
	{
		auto obj = static_cast<WED_ObjPlacement *>(what);
		if(obj->HasCustomMSL())
		{
			out_msl_min = out_msl_max = obj->GetCustomMSL();
			return 1;
		}
	}

	if (c == WED_TerPlacement::sClass)
	{
		auto ter = static_cast<WED_TerPlacement*>(what);
		out_msl_min = out_msl_max = ter->GetCustomMSL();
		return 1;
	}

	int found = 0;		// true if we found at least 1 min/max
	int any_inside = 0;	// true if we found ANYTHING inside at all?

	if(c == WED_Airport::sClass || c == WED_Group::sClass)
	{
		int nn = what->CountChildren();
		for(int n = 0; n < nn; ++n)
		{
			double msl_min, msl_max;
			int child_cull = DSF_HeightRangeRecursive(what->GetNthChild(n),msl_min,msl_max, bounds);
			if (child_cull == 1)
			{
				any_inside = 1;
				if(found)
				{
					out_msl_min=min(out_msl_min,msl_min);
					out_msl_max=max(out_msl_max,msl_max);
				}
				else
				{
					out_msl_min=msl_min;
					out_msl_max=msl_max;
					found=1;
				}
			}
			else if(child_cull == 0)
				any_inside = 1;
		}
	}

	if(!any_inside && ent && ent->GetGISClass() != gis_Composite)
		return 0;

	return found ? 1 : (any_inside ? 0 : -1);
}

static const char * get_exclusion_text(int i)
{
	switch (i) {
	case exclude_Obj:	return "sim/exclude_obj";
	case exclude_Fac:	return "sim/exclude_fac";
	case exclude_For:	return "sim/exclude_for";
	case exclude_Bch:	return "sim/exclude_bch";
	case exclude_Net:	return "sim/exclude_net";

	case exclude_Lin:	return "sim/exclude_lin";
	case exclude_Pol:	return "sim/exclude_pol";
	case exclude_Str:	return "sim/exclude_str";
	default: return nullptr;
	}

}


//Returns -1 for abort, or n where n > 0 for the number of
static int	DSF_ExportTileRecursive(
						WED_Thing *					what,
						IResolver *					resolver,
						const string&				pkg,
						const Bbox2&				cull_bounds,		// This is the area for which we are TRYING to get scenery.
						const Bbox2&				safe_bounds,		// This is the 'safe' area into which we CAN write scenery without exploding.
						DSF_ResourceTable&			io_table,
						const DSFCallbacks_t *		cbs,
						void *						writer,
						set<WED_Thing *>&			problem_children,
						int							show_level,
						DSF_export_info_t *			export_info )
{
	WED_Entity * ent = static_cast<WED_Entity *>(what);
	if (!ent || ent->GetHidden())
		return 0;

	IGISEntity * e = dynamic_cast<IGISEntity *>(what);
	Bbox2	ent_box;
	e->GetBounds(gis_Geo,ent_box);

	if(!ent_box.overlap(cull_bounds))
		return 0;

	Point2	centroid = ent_box.centroid();
	bool centroid_ob = centroid.x() < cull_bounds.xmin() ||	centroid.y() < cull_bounds.ymin() ||
					   centroid.x() >= cull_bounds.xmax() || centroid.y() >= cull_bounds.ymax();

	int real_thingies = 0;
	int idx;
	string r;
	WED_Airport * apt;
	sClass_t c = what->GetClass();

	//------------------------------------------------------------------------------------------------------------
	// OBJECT EXPORTER
	//------------------------------------------------------------------------------------------------------------

	if(c == WED_ObjPlacement::sClass)
	{
		auto obj = static_cast<WED_ObjPlacement *>(what);
		if(show_level == obj->GetShowLevel())
		{
			obj->GetResource(r);
			idx = io_table.accum_obj(r,show_level);
			Point2 p;
			obj->GetLocation(gis_Geo,p);
			if(cull_bounds.contains(p))
			{
				float heading = obj->GetHeading();
				while(heading < 0) heading += 360.0;
				while(heading >= 360.0) heading -= 360.0;
				++real_thingies;

				double xyrz[4] = { p.x(), p.y(), heading, 0.0 };
				if(obj->HasCustomMSL())
				{
					xyrz[3] = doblim(obj->GetCustomMSL(), -500.0, +10000.0);   // XPD-15378 rendering engine may freak out
					cbs->AddObjectWithMode_f(idx, xyrz, (obj_elev_mode) obj->HasCustomMSL(), writer);
				}
				else
					cbs->AddObjectWithMode_f(idx, xyrz, obj_ModeDraped, writer);
			}
		}
		return real_thingies;
	}

	if (c == WED_TerPlacement::sClass)
	{
		auto ter = static_cast<WED_TerPlacement*>(what);
		if (show_level == ter->GetShowLevel())
		{
			Bbox2 b;
			ter->GetBounds(gis_Geo, b);
			Point2 obj_loc = b.centroid();
			if (cull_bounds.contains(obj_loc))
			{
				if (int result = WED_ExportTerrObj(ter, resolver, pkg, r) < 0)
					return result;
				export_info->resourcesAdded = true;

				idx = io_table.accum_obj(r, show_level);
				double xyrz[4] = { obj_loc.x(), obj_loc.y(), 0.0, ter->GetCustomMSL() };
				cbs->AddObjectWithMode_f(idx, xyrz, (obj_elev_mode) ter->GetMSLType(), writer);
				++real_thingies;
			}
		}
		return real_thingies;
	}

	//------------------------------------------------------------------------------------------------------------
	// FACADE EXPORTER
	//------------------------------------------------------------------------------------------------------------

	if(c == WED_FacadePlacement::sClass)
	{
		auto fac = static_cast<WED_FacadePlacement *>(what);
		if(show_level == fac->GetShowLevel())
		{
			fac->GetResource(r);
			idx = io_table.accum_pol(r,show_level);
			bool bez = WED_HasBezierPol(fac);
			bool fac_is_auto_closed = fac->GetTopoMode() != WED_FacadePlacement::topo_Chain;

			if(fac->GetTopoMode() == WED_FacadePlacement::topo_Area && fac->HasCustomWalls())
			{
				if(bez)
				{
					vector<BezierPolygon2p>				fac_area;
					vector<vector<BezierPolygon2p> >	fac_cut;
					WED_BezierPolygonWithHolesForPolygon(fac, fac_area);

					if(gExportTarget < wet_xplane_1021)
					{
						if(!clip_polygon(fac_area,fac_cut,cull_bounds))
						{
							problem_children.insert(what);
							fac_cut.clear();
						}
					}
					else if(!centroid_ob)
						fac_cut.push_back(fac_area);

					for(vector<vector<BezierPolygon2p> >::iterator i = fac_cut.begin(); i != fac_cut.end(); ++i)
					{
						++real_thingies;
						cbs->BeginPolygon_f(idx,fac->GetHeight(),5,writer);
						DSF_AccumPolygonWithHolesBezier(*i, safe_bounds, cbs, writer);
						cbs->EndPolygon_f(writer);
					}
				}
				else
				{
					vector<Polygon2p>			fac_area;
					vector<vector<Polygon2p> >	fac_cut;
					Assert(WED_PolygonWithHolesForPolygon(fac,fac_area));

					if(gExportTarget < wet_xplane_1021)
					{
						if (!clip_polygon(fac_area,fac_cut,cull_bounds))
						{
							fac_cut.clear();
							problem_children.insert(what);
						}
					} else if(!centroid_ob)
						fac_cut.push_back(fac_area);

					for(vector<vector<Polygon2p> >::iterator i = fac_cut.begin(); i != fac_cut.end(); ++i)
					{
						++real_thingies;
						cbs->BeginPolygon_f(idx,fac->GetHeight(),3,writer);
						DSF_AccumPolygonWithHoles(*i, safe_bounds, cbs, writer);
						cbs->EndPolygon_f(writer);
					}
				}
			}
			else if(fac->GetTopoMode() == WED_FacadePlacement::topo_Area)
			{
				if(bez)
				{
					vector<BezierPolygon2> fac_area;
					vector<vector<BezierPolygon2> > fac_cut;
					WED_BezierPolygonWithHolesForPolygon(fac, fac_area);

					if(gExportTarget < wet_xplane_1021)
					{
						if(!clip_polygon(fac_area,fac_cut,cull_bounds))
						{
							problem_children.insert(what);
							fac_cut.clear();
						}
					}
					else if(!centroid_ob)
						fac_cut.push_back(fac_area);

					for(vector<vector<BezierPolygon2> >::iterator i = fac_cut.begin(); i != fac_cut.end(); ++i)
					{
						++real_thingies;
						cbs->BeginPolygon_f(idx,fac->GetHeight(),4,writer);
						DSF_AccumPolygonWithHolesBezier(*i, safe_bounds, cbs, writer);
						cbs->EndPolygon_f(writer);
					}
				}
				else
				{
					vector<Polygon2>			fac_area;
					vector<vector<Polygon2> >	fac_cut;
					Assert(WED_PolygonWithHolesForPolygon(fac,fac_area));

					if(gExportTarget < wet_xplane_1021)
					{
						if (!clip_polygon(fac_area,fac_cut,cull_bounds))
						{
							fac_cut.clear();
							problem_children.insert(what);
						}
					} else if(!centroid_ob)
						fac_cut.push_back(fac_area);

					for(vector<vector<Polygon2> >::iterator i = fac_cut.begin(); i != fac_cut.end(); ++i)
					{
						++real_thingies;
						cbs->BeginPolygon_f(idx,fac->GetHeight(),2,writer);
						DSF_AccumPolygonWithHoles(*i, safe_bounds, cbs, writer);
						cbs->EndPolygon_f(writer);
					}

				}
			}
			else if (fac->HasCustomWalls())
			{
				for(int h = -1; h < fac->GetNumHoles(); ++h)
				{
					IGISPointSequence * seq = (h == -1) ? fac->GetOuterRing() : fac->GetNthHole(h);

					if(bez)
					{
						vector<Bezier2p>	chain;

						WED_BezierVectorForPointSequence(seq,chain);
						if(gExportTarget < wet_xplane_1021)
							clip_segments(chain,cull_bounds);
						else if(centroid_ob)
							chain.clear();

						if (!chain.empty() && export_info->DockingJetways && fac->HasDockingCabin())
						{
							chain.pop_back();
							chain.pop_back();
						}

						if(!chain.empty())
						{
							++real_thingies;
							if(fac_is_auto_closed && bad_match(chain.front(),chain.back()))
								problem_children.insert(what);
							else
							{
								Point2 pt;
								seq->GetNthPoint(seq->GetNumPoints() - 1)->GetLocation(gis_Param, pt);
								DSF_AccumChainBezier(chain.cbegin(), chain.cend(), safe_bounds, cbs, writer, idx, fac->GetHeight(), fac_is_auto_closed, pt.x());
							}
						}
					}
					else
					{
						vector<Segment2p>	chain;

						WED_VectorForPointSequence(seq,chain);
						// We normally reject zero length segments, but for grandfathered global airports, we'll try to clean this.
						remove_all_zero_length_segments(chain);
						if(gExportTarget < wet_xplane_1021)
							clip_segments(chain,cull_bounds);
						else if(centroid_ob)
							chain.clear();

						if (!chain.empty() && export_info->DockingJetways && fac->HasDockingCabin())
						{
							chain.pop_back();
							chain.pop_back();
						}

						if(!chain.empty())
						{
							++real_thingies;
							if(fac_is_auto_closed && bad_match(chain.front(),chain.back()))
								problem_children.insert(what);
							else
							{
								Point2 pt;
								seq->GetNthPoint(seq->GetNumPoints() - 1)->GetLocation(gis_Param, pt);
								DSF_AccumChain(chain.cbegin(), chain.cend(), safe_bounds, cbs, writer, idx, fac->GetHeight(), fac_is_auto_closed, pt.x());
							}
						}
					}
				}
			}
			else
			{
				for(int h = -1; h < fac->GetNumHoles(); ++h)
				{
					IGISPointSequence * seq = (h == -1) ? fac->GetOuterRing() : fac->GetNthHole(h);

					if(bez)
					{
						vector<Bezier2>	chain;

						WED_BezierVectorForPointSequence(seq,chain);
						if(gExportTarget < wet_xplane_1021)
							clip_segments(chain,cull_bounds);
						else if(centroid_ob)
							chain.clear();

						if(!chain.empty())
						{
							++real_thingies;
							if(fac_is_auto_closed && bad_match(chain.front(),chain.back()))
								problem_children.insert(what);
							else
								DSF_AccumChainBezier(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, fac->GetHeight(), fac_is_auto_closed);
						}
					}
					else
					{
						vector<Segment2>	chain;

						WED_VectorForPointSequence(seq,chain);
						// We normally reject zero length segments, but for grandfathered global airports, we'll try to clean this.
						remove_all_zero_length_segments(chain);
						if(gExportTarget < wet_xplane_1021)
							clip_segments(chain,cull_bounds);
						else if(centroid_ob)
							chain.clear();
						if(!chain.empty())
						{
							++real_thingies;
							if(fac_is_auto_closed && bad_match(chain.front(),chain.back()))
								problem_children.insert(what);
							else
								DSF_AccumChain(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, fac->GetHeight(), fac_is_auto_closed);
						}
					}
				}
			}
		}
		return real_thingies;
	}

	if(show_level == 6)
	{
		//------------------------------------------------------------------------------------------------------------
		// EXCLUSION EXPORTER
		//------------------------------------------------------------------------------------------------------------

		if(c == WED_ExclusionZone::sClass)
		{
			auto xcl = static_cast<WED_ExclusionZone *>(what);
			set<int> xtypes;
			xcl->GetExclusions(xtypes);
			Point2 minp, maxp;
			xcl->GetMin()->GetLocation(gis_Geo,minp);
			xcl->GetMax()->GetLocation(gis_Geo,maxp);

			if(minp.x_ > maxp.x_)	swap(minp.x_, maxp.x_);
			if(minp.y_ > maxp.y_)	swap(minp.y_, maxp.y_);

			for(auto xt : xtypes)
			{
				if(auto pname = get_exclusion_text(xt))
				{
					char valbuf[64];
					sprintf(valbuf,"%.6lf/%.6lf/%.6lf/%.6lf",minp.x(),minp.y(),maxp.x(),maxp.y());
					++real_thingies;
					io_table.accum_exclusion(pname, valbuf);
				}
			}
			return real_thingies;
		}

		else if (c == WED_ExclusionPoly::sClass)
		{
			auto xcl = static_cast<WED_ExclusionPoly*>(what);
			set<int> xtypes;
			xcl->GetExclusions(xtypes);
			Bbox2 bounds;
			xcl->GetBounds(gis_Geo, bounds);

			for (auto xt : xtypes)
			{
				if (auto pname = get_exclusion_text(xt))
				{
					char valbuf[64];
					sprintf(valbuf, "%.6lf/%.6lf/%.6lf/%.6lf;", bounds.p1.x(), bounds.p1.y(), bounds.p2.x(), bounds.p2.y());
					++real_thingies;
					string excbuf(valbuf);

					vector<Polygon2>	xcl_area;
					Assert(WED_PolygonWithHolesForPolygon(xcl, xcl_area));

					vector<vector<Polygon2> >	xcl_clipped;
					if (!clip_polygon(xcl_area, xcl_clipped, cull_bounds))
					{
						xcl_clipped.clear();
						problem_children.insert(what);
					}
					for (const auto& pol_vec : xcl_clipped)
						for (const auto& pol : pol_vec)
							for (const auto& pt : pol)
							{
								sprintf(valbuf, "%.6lf/%.6lf,", pt.x(), pt.y());
								excbuf += valbuf;
							}

					if (excbuf.back() == ',')
						excbuf.pop_back();
					io_table.accum_exclusion(pname, excbuf);
				}
			}
			return real_thingies;
		}

		//------------------------------------------------------------------------------------------------------------
		// FOREST EXPORTER
		//------------------------------------------------------------------------------------------------------------

		else if(c == WED_ForestPlacement::sClass)
		{
			auto fst = static_cast<WED_ForestPlacement *>(what);
			fst->GetResource(r);
			idx = io_table.accum_pol(r,show_level);

			DebugAssert(!WED_HasBezierPol(fst));
			int param = intlim(fst->GetDensity() * 255.0,0,255) + fst->GetFillMode() * 256;
			switch(fst->GetFillMode()) {
			case dsf_fill_area:
				{
					vector<Polygon2>	fst_area;
					Assert(WED_PolygonWithHolesForPolygon(fst,fst_area));

					// We normally reject zero length segments, but for grandfathered global airports, we'll try to clean this.
					for(vector<Polygon2>::iterator f = fst_area.begin(); f != fst_area.end(); ++f)
						f->erase(unique(f->begin(),f->end()),f->end());

					vector<vector<Polygon2> >	fst_clipped;
					if (!clip_polygon(fst_area, fst_clipped,cull_bounds))
					{
						fst_clipped.clear();
						problem_children.insert(what);
					}

					for(vector<vector<Polygon2> >::iterator i = fst_clipped.begin(); i != fst_clipped.end(); ++i)
					{
						++real_thingies;
						cbs->BeginPolygon_f(idx,param,2,writer);
						DSF_AccumPolygonWithHoles(*i, safe_bounds, cbs, writer);
						cbs->EndPolygon_f(writer);
					}
				}
				break;
			case dsf_fill_line:
				{
					for(int h = -1; h < fst->GetNumHoles(); ++h)
					{
						IGISPointSequence * seq = (h == -1) ? fst->GetOuterRing() : fst->GetNthHole(h);
						vector<Segment2>	chain;

						WED_VectorForPointSequence(seq,chain);
						// We normally reject zero length segments, but for grandfathered global airports, we'll try to clean this.
						remove_all_zero_length_segments(chain);

						clip_segments(chain, cull_bounds);
						if(!chain.empty())
						{
							++real_thingies;
							DSF_AccumChain(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, param, 0);
						}
					}
				}
				break;
			case dsf_fill_points:
				{
					vector<Point2>	pts;

					for (int h = -1; h < fst->GetNumHoles(); ++h)             // allow holes in point mode ???
					{
						IGISPointSequence* seq = (h == -1) ? fst->GetOuterRing() : fst->GetNthHole(h);

						for (int p = 0; p < seq->GetNumPoints(); ++p)
						{
							Point2 x;
							seq->GetNthPoint(p)->GetLocation(gis_Geo, x);
							if (safe_bounds.contains(x))
							{
								pts.push_back(x);
							}
						}
					}

					if (!pts.empty())
					{
						++real_thingies;
#if !TYLER_MODE
						bool elevated = false;
						// get DEM
						Bbox2 bnds;
						double ter_msl = 0;
						fst->GetBounds(gis_Geo, bnds);
						vector<WED_TerPlacement*> ters;
						CollectRecursive(WED_GetWorld(resolver), back_inserter(ters), EntityNotHidden, TakeAlways, WED_TerPlacement::sClass);

						const dem_info_t* dem_info = nullptr;
						for(auto t : ters)
							if (t->Cull(bnds))
							{
								string dem_file;
								t->GetResource(dem_file);
								if (!(!WED_GetResourceMgr(resolver)->GetDem(dem_file, dem_info)))
									elevated = true;
								if (t->GetMSLType() == obj_ModeMSL)
									ter_msl = t->GetCustomMSL();
								break;
							}
						// get height ranges
						const for_info_t * fst_info = nullptr;
						if (!WED_GetResourceMgr(resolver)->GetFor(r, fst_info))
							elevated = false;
						if(elevated)
						{
							cbs->BeginPolygon_f(idx, param, 4 + 10, writer);   // really means 4 data planes, but signals PointPool scaling needs to suit forests with height, msl
							cbs->BeginPolygonWinding_f(writer);
							double c[4];
							for (auto& p : pts)
							{
								c[0] = p.x();
								c[1] = p.y();
								c[2] = 0.8 * fst_info->max_height;       // random (min_height, max_height);
								c[3] = dem_info->xy_nearest(p.x(), p.y()) + ter_msl;

								cbs->AddPolygonPoint_f(c, writer);
							}
							cbs->EndPolygonWinding_f(writer);
							cbs->EndPolygon_f(writer);
						}
						else
#endif
							DSF_AccumPts(pts.begin(), pts.end(), safe_bounds, cbs, writer, idx, param);
					}
				}
				break;
			}
			return real_thingies;
		}

		//------------------------------------------------------------------------------------------------------------
		// OBJ STRING EXPORTER
		//------------------------------------------------------------------------------------------------------------

		else if(c == WED_StringPlacement::sClass)
		{
			auto str = static_cast<WED_StringPlacement *>(what);
			str->GetResource(r);
			idx = io_table.accum_pol(r,show_level);
			bool bez = WED_HasBezierSeq(str);

			if(bez)
			{
				vector<Bezier2>	chain;

				WED_BezierVectorForPointSequence(str,chain);
				clip_segments(chain, cull_bounds);
				if(!chain.empty())
				{
					++real_thingies;
					DSF_AccumChainBezier(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, encode_spacing(str->GetSpacing()), 0);
				}
			}
			else
			{
				vector<Segment2>	chain;

				WED_VectorForPointSequence(str,chain);
				// We normally reject zero length segments, but for grandfathered global airports, we'll try to clean this.
				remove_all_zero_length_segments(chain);
				clip_segments(chain, cull_bounds);
				if(!chain.empty())
				{
					++real_thingies;
					DSF_AccumChain(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, encode_spacing(str->GetSpacing()), 0);
				}
			}
			return real_thingies;
		}

		//------------------------------------------------------------------------------------------------------------
		// AUTOGEN STRING EXPORTER
		//------------------------------------------------------------------------------------------------------------

		else if(c == WED_AutogenPlacement::sClass)
		{
			auto ags = static_cast<WED_AutogenPlacement *>(what);
			ags->GetResource(r);
			idx = io_table.accum_pol(r,show_level);
			bool bez = WED_HasBezierPol(ags);
			int n_spawning = 1;


			// start at contour 1, keep going until not spawning
			// write as first polygon all these, including the first not spawning point.
			// add to spawning.
			// then take last point again, keep going until spwning flips etc
			// add do spawn/nonspawn as needed

			vector<Polygon2p>			pol_area_raw;
			Assert(WED_PolygonWithHolesForPolygon(ags,pol_area_raw));

			vector<vector<Polygon2p> >	pol_cuts;
			if(!clip_polygon(pol_area_raw, pol_cuts, cull_bounds))
			{
				problem_children.insert(what);
				pol_cuts.clear();
			}

			for(const auto& cuts : pol_cuts)
			{
				bool last_pt_spawn = true;
				vector<Polygon2> 	pol_area;
				vector<Polygon2>	pol_nonspawning;
				pol_area.push_back(Polygon2());

				for(auto p2p : cuts.front())
				{
					if(last_pt_spawn)
						pol_area.back().push_back(p2p.p1);
					else
						pol_nonspawning.back().push_back(p2p.p1);

					if(p2p.param) 	// segments created during clipping coincide exactly with clipping box bounds, must be non-spawning
					{
						if( (p2p.p1.x() == p2p.p2.x() && (p2p.p1.x() == cull_bounds.xmin() || p2p.p1.x() == cull_bounds.xmax())) ||
							(p2p.p1.y() == p2p.p2.y() && (p2p.p1.y() == cull_bounds.ymin() || p2p.p1.y() == cull_bounds.ymax())) )
								p2p.param = false;
					}

					if((p2p.param != 0) != last_pt_spawn)
					{
						if(last_pt_spawn)
						{
							pol_nonspawning.push_back(Polygon2());
							pol_nonspawning.back().push_back(p2p.p1);
							last_pt_spawn = false;
						}
						else
						{
							pol_area.push_back(Polygon2());
							pol_area.back().push_back(p2p.p1);
							last_pt_spawn = true;
							n_spawning++;
						}
					}
				}
				if(last_pt_spawn)
					pol_area.back().push_back(cuts.front().front().p1);
				else
					pol_nonspawning.back().push_back(cuts.front().front().p1);

				// append non-spawning vectors at the end
				for(const auto& ns : pol_nonspawning)
					pol_area.push_back(ns);

				// all other contours are holes, so add append at end
				auto h2p = cuts.begin();
				for(++h2p; h2p != cuts.end(); ++h2p)
				{
					pol_area.push_back(Polygon2());
					for(auto p2p : *h2p)
						pol_area.back().push_back(p2p.p1);
				}

				++real_thingies;
				int para = intlim(intround(ags->GetHeight() * 0.25), 0, 255) << 8;
				if(ags->IsAGBlock())
					para += intlim(ags->GetSpelling(), 0, 255);
				else
					para += intlim(n_spawning, 0, 255);
				cbs->BeginPolygon_f(idx, para, 2, writer);
				DSF_AccumPolygonWithHoles(pol_area, safe_bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}

			return real_thingies;
		}

		//------------------------------------------------------------------------------------------------------------
		// OBJ LINE EXPORTER
		//------------------------------------------------------------------------------------------------------------

		else if(c == WED_LinePlacement::sClass)
		{
			auto lin = static_cast<WED_LinePlacement *>(what);
			lin->GetResource(r);
			idx = io_table.accum_pol(r,show_level);
			bool bez = WED_HasBezierSeq(lin);

			if(bez)
			{
				vector<Bezier2>	chain;

				WED_BezierVectorForPointSequence(lin,chain);

				clip_segments(chain, cull_bounds);
				if(!chain.empty())
				{
					int closed = 0;
					if(one_winding(chain))
					{
						if(chain.front().p1 == chain.back().p2 && chain.size() > 1)
						{
							closed = 1;
						}
					}
					else
					{
						while(chain.front().p1 == chain.back().p2)
						{
							chain.insert(chain.begin(),chain.back());
							chain.pop_back();
						}
					}
					++real_thingies;
					if(closed && bad_match(chain.front(),chain.back()))
					{
						DebugAssert(!"We should not get here - it's a logic error, not a precision error!");
						problem_children.insert(what);
					}
					else
						DSF_AccumChainBezier(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, closed, closed);
				}
			}
			else
			{
				vector<Segment2>	chain;

				WED_VectorForPointSequence(lin,chain);
				// We normally reject zero length segments, but for grandfathered global airports, we'll try to clean this.
				remove_all_zero_length_segments(chain);
				clip_segments(chain, cull_bounds);
				if(!chain.empty())
				{
					int closed = 0;
					if(one_winding(chain))
					{
						if(chain.front().p1 == chain.back().p2 && chain.size() > 1)
						{
							closed = 1;
						}
					}
					else
					{
						while(chain.front().p1 == chain.back().p2)
						{
							chain.insert(chain.begin(),chain.back());
							chain.pop_back();
						}
					}
					++real_thingies;
					if(closed && bad_match(chain.front(),chain.back()))
					{
						DebugAssert(!"We should not get here - it's a logic error, not a precision error!");
						problem_children.insert(what);
					}
					else
						DSF_AccumChain(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, closed, closed);
				}
			}
			return real_thingies;
		}
		//------------------------------------------------------------------------------------------------------------
		// DRAPED POLYGON
		//------------------------------------------------------------------------------------------------------------

		else if(c == WED_PolygonPlacement::sClass)
		{
			auto pol = static_cast<WED_PolygonPlacement *>(what);
			pol->GetResource(r);
			idx = io_table.accum_pol(r,show_level);
			bool bez = WED_HasBezierPol(pol);

			if(bez)
			{
				vector<BezierPolygon2>			pol_area;
				vector<vector<BezierPolygon2> >	pol_cuts;

				WED_BezierPolygonWithHolesForPolygon(pol, pol_area);

				if(!clip_polygon(pol_area,pol_cuts,cull_bounds))
				{
					problem_children.insert(what);
					pol_cuts.clear();
				}

				for(vector<vector<BezierPolygon2> >::iterator i = pol_cuts.begin(); i != pol_cuts.end(); ++i)
				{
					++real_thingies;
					cbs->BeginPolygon_f(idx, encode_heading(pol->GetHeading()),bez ? 4 : 2,writer);
					DSF_AccumPolygonWithHolesBezier(*i, safe_bounds, cbs, writer);
					cbs->EndPolygon_f(writer);
				}
			}
			else
			{
				vector<Polygon2> 	pol_area;
				vector<vector<Polygon2> >	pol_cuts;

				Assert(WED_PolygonWithHolesForPolygon(pol,pol_area));

				if(!clip_polygon(pol_area,pol_cuts,cull_bounds))
				{
					problem_children.insert(what);
					pol_cuts.clear();

				}
				for(vector<vector<Polygon2> >::iterator i = pol_cuts.begin(); i != pol_cuts.end(); ++i)
				{
					++real_thingies;
					cbs->BeginPolygon_f(idx, encode_heading(pol->GetHeading()),bez ? 4 : 2,writer);
					DSF_AccumPolygonWithHoles(*i, safe_bounds, cbs, writer);
					cbs->EndPolygon_f(writer);
				}
			}
			return real_thingies;
		}

		//------------------------------------------------------------------------------------------------------------
		// UV-MAPPED DRAPED POLYGON
		//------------------------------------------------------------------------------------------------------------

		else if(c == WED_DrapedOrthophoto::sClass)
		{
			auto orth = static_cast<WED_DrapedOrthophoto *>(what);
			orth->GetResource(r);
#if WED
			if (orth->IsNew())
			{
				if (auto result = WED_ExportOrtho(orth, resolver, pkg, export_info, r) < 0)
					return result;
				export_info->resourcesAdded = true;
			}
#endif
			idx = io_table.accum_pol(r,show_level);
			bool bez = WED_HasBezierPol(orth);

			if(bez)
			{
				vector<BezierPolygon2uv>			orth_area;
				vector<vector<BezierPolygon2uv> >	orth_cuts;

				WED_BezierPolygonWithHolesForPolygon(orth, orth_area);

				if(!clip_polygon(orth_area,orth_cuts,cull_bounds))
				{
					problem_children.insert(what);
					orth_cuts.clear();
				}

				for(vector<vector<BezierPolygon2uv> >::iterator i = orth_cuts.begin(); i != orth_cuts.end(); ++i)
				{
					++real_thingies;
					cbs->BeginPolygon_f(idx,65535,8,writer);
					DSF_AccumPolygonWithHolesBezier(*i, safe_bounds, cbs, writer);
					cbs->EndPolygon_f(writer);
				}
			}
			else
			{
				vector<Polygon2uv>			orth_area;
				vector<vector<Polygon2uv> >	orth_cuts;
				Assert(WED_PolygonWithHolesForPolygon(orth,orth_area));

				if(!clip_polygon(orth_area,orth_cuts,cull_bounds))
				{
					problem_children.insert(what);
					orth_cuts.clear();
				}

				for(vector<vector<Polygon2uv> >::iterator i = orth_cuts.begin(); i != orth_cuts.end(); ++i)
				{
					++real_thingies;
					cbs->BeginPolygon_f(idx,65535,4,writer);
					DSF_AccumPolygonWithHoles(*i, safe_bounds, cbs, writer);
					cbs->EndPolygon_f(writer);
				}
			}
#if WED
			if(orth->IsNew())
				what->AbortOperation(); // this will nicely undo the UV mapping rescaling we did :)
#endif
			return real_thingies;
		}

	#if ROAD_EDITING
		//------------------------------------------------------------------------------------------------------------
		// ROAD EXPORTER
		//------------------------------------------------------------------------------------------------------------

		else if (c == WED_RoadEdge::sClass)
		{
			auto roa = static_cast<WED_RoadEdge*>(what);
			string asset;
			roa->GetResource(asset);
			dsf_road_grid_helper * grid = io_table.accum_net(asset, io_table.cur_filter);
			grid->add_segment(roa,cull_bounds);
			++real_thingies;
			return real_thingies;
		}
	#endif // ROAD_EDITING
	}

	if(c == WED_Airport::sClass)
	{
		apt = static_cast<WED_Airport*>(what);
		apt->GetICAO(r);
		idx = io_table.accum_filter(r.c_str());
		cbs->SetFilter_f(idx,writer);
		io_table.set_filter(idx);
	}
	else apt = NULL;

	//------------------------------------------------------------------------------------------------------------
	// RECURSION
	//------------------------------------------------------------------------------------------------------------

	if(apt || c == WED_Group::sClass)  // only recurse if there is actually a possibility of more DSF content in there
	{
		int cc = what->CountChildren();
		for (int c = 0; c < cc; ++c)
		{
			int result = DSF_ExportTileRecursive(what->GetNthChild(c), resolver, pkg, cull_bounds, safe_bounds, io_table, cbs, writer, problem_children, show_level, export_info);
			if (result == -1)
			{
				real_thingies = -1; //Abort!
				break;
			}
			else
			{
				real_thingies += result;
			}
		}
	}

	if(apt)
	{
		cbs->SetFilter_f(-1,writer);
		io_table.set_filter(-1);
	}

	return real_thingies;
}

int DSF_ExportTile(WED_Thing * base, IResolver * resolver, const string& pkg, int x, int y, set <WED_Thing *>& problem_children, DSF_export_info_t * export_info)
{
	void *			writer;
	DSFCallbacks_t	cbs;
	char	buffer[256];

	double msl_min, msl_max;
	Bbox2	cull(x,y,x+1,y+1);

	int cull_code = DSF_HeightRangeRecursive(base,msl_min,msl_max, cull);    // also finds if tile has anything goint into it

	if(cull_code < 0)
		return 0;
	else if(cull_code > 0)
	{
		msl_min = floor(msl_min);
		msl_max = ceil(msl_max);
		if(msl_min == msl_max)
			msl_max = msl_min + 1.0;
	}
	else // cull_code == 0
		msl_min = -32768.0, msl_max = 32767.0;

	writer = DSFCreateWriter(x,y,x+1,y+1, msl_min,msl_max,DSF_DIVISIONS);
	DSFGetWriterCallbacks(&cbs);

	sprintf(buffer, "%d", (int) x  );		cbs.AcceptProperty_f("sim/west", buffer, writer);
	sprintf(buffer, "%d", (int) x+1);		cbs.AcceptProperty_f("sim/east", buffer, writer);
	sprintf(buffer, "%d", (int) y+1);		cbs.AcceptProperty_f("sim/north", buffer, writer);
	sprintf(buffer, "%d", (int) y  );		cbs.AcceptProperty_f("sim/south", buffer, writer);
	cbs.AcceptProperty_f("sim/planet", "earth", writer);
	cbs.AcceptProperty_f("sim/creation_agent", "WorldEditor" WED_VERSION_STRING, writer);
	cbs.AcceptProperty_f("laminar/internal_revision", "0", writer);
	cbs.AcceptProperty_f("sim/overlay", "1", writer);

	DSF_ResourceTable	rsrc;

	Bbox2	cull_bounds(x,y,x+1,y+1);
	Bbox2	safe_bounds(cull_bounds);
	if(gExportTarget >= wet_xplane_1021)
		safe_bounds.expand(DSF_EXTRA_1021);

	int entities = 0;
	for (int show_level = 6; show_level >= 1; --show_level)
	{
		int result = DSF_ExportTileRecursive(base, resolver, pkg, cull_bounds, safe_bounds, rsrc, &cbs, writer, problem_children, show_level, export_info);
		if (result == -1)
		{
			DSFDestroyWriter(writer);
			return -1; //Abort!
		}
		entities += result;
	}

	rsrc.write_tables(cbs,writer);

	if(entities)	// empty DSF?  Don't write a empty file, makes a mess!
	{
		snprintf(buffer, 255, "%sEarth nav data" DIR_STR "%+03d%+04d",	pkg.c_str(), latlon_bucket(y), latlon_bucket(x)	);
		FILE_make_dir_exist(buffer);

		snprintf(buffer, 255, "%sEarth nav data" DIR_STR "%+03d%+04d" DIR_STR "%+03d%+04d.dsf", pkg.c_str(), latlon_bucket(y), latlon_bucket(x), y, x);
		DSFWriteToFile(buffer, writer);

		snprintf(buffer, 255, "%+03d%+04d" DIR_STR "%+03d%+04d.dsf", latlon_bucket(y), latlon_bucket(x), y, x);
		export_info->mark_written(buffer);
	}

	/*
	// test code to make sure culling works - asserts if we false-cull.
	if(cull_code == -1)
	{
		if(entities > 0)
		{
			int cull_code = DSF_HeightRangeRecursive(base,msl_min,msl_max, cull);
		}
		Assert(entities == 0);
	}
	*/

	DSFDestroyWriter(writer);
	return entities;
}

int DSF_Export(WED_Thing * base, IResolver * resolver, const string& package, set<WED_Thing *>& problem_children)
{
#if DEV
	StElapsedTime	etime("Export time");
#endif
	g_dropped_pts = false;
	Bbox2	wrl_bounds;

	IGISEntity * ent = dynamic_cast<IGISEntity *>(base);
	DebugAssert(ent);
	if(!ent)
		return 0;

	ent->GetBounds(gis_Geo,wrl_bounds);
	int tile_west  = floor(wrl_bounds.p1.x());
	int tile_east  = ceil (wrl_bounds.p2.x());
	int tile_south = floor(wrl_bounds.p1.y());
	int tile_north = ceil (wrl_bounds.p2.y());

	int DSF_export_tile_res = 0;

	DSF_export_info_t DSF_export_info(resolver);   // We kept the last loaded orthoimage open, so it does not have to be loaded repeatedly. This gates parallel DSF exports.
	DSF_export_info.DockingJetways = gExportTarget >= wet_xplane_1200;

	for (int y = tile_south; y < tile_north; ++y)
	{
#if TYLER_MODE
		printf("Exporting DSF's at lattitude %d\n", y);
#endif
		for (int x = tile_west; x < tile_east; ++x)
		{
			DSF_export_tile_res = DSF_ExportTile(base, resolver, package, x, y, problem_children, &DSF_export_info);
			if (DSF_export_tile_res == -1) break;
		}
		if (DSF_export_tile_res == -1) break;
	}
	if (g_dropped_pts)
	{
#if WED
		DoUserAlert(
#else
		fprintf(stderr, "%s\n",
#endif
				"Warning: you have bezier curves that cross a DSF tile boundary. X-Plane 9 cannot handle this case. "
				"To fix this, only use non-curved polygons to cross a tile boundary.");
		return -1;
	}
	return 0;
}

int DSF_ExportAirportOverlay(IResolver * resolver, WED_Airport  * apt, const string& package, set<WED_Thing *>& problem_children)
{
	if(apt->GetHidden())
		return 1;
	string icao;
	apt->GetICAO(icao);
	string dsf_path = package + icao + ".txt";

	FILE * dsf = fopen(dsf_path.c_str(),"w");
	if(dsf)
	{
		DSFCallbacks_t	cbs;
		DSF2Text_CreateWriterCallbacks(&cbs);
		print_funcs_s pf;
		pf.print_func = (int (*)(void *, const char *, ...)) fprintf;
		pf.ref = dsf;

		void * writer = &pf;

		Bbox2	cull_bounds(-180,-90,180,90);
		Bbox2	safe_bounds(-180,-90,180,90);

		cbs.AcceptProperty_f("sim/west", "-180", writer);
		cbs.AcceptProperty_f("sim/east", "180", writer);
		cbs.AcceptProperty_f("sim/north", "90", writer);
		cbs.AcceptProperty_f("sim/south", "-90", writer);
		cbs.AcceptProperty_f("sim/planet", "earth", writer);
		cbs.AcceptProperty_f("sim/creation_agent", "WorldEditor" WED_VERSION_STRING, writer);
		cbs.AcceptProperty_f("laminar/internal_revision", "0", writer);
		cbs.AcceptProperty_f("sim/overlay", "1", writer);

		DSF_ResourceTable	rsrc;
		DSF_export_info_t DSF_export_info;
		DSF_export_info.DockingJetways = false; // keep jetways as facades. Then no need to mtach up apt.dat jetways and DSF facades at import from GW

		int entities = 0;
		for(int show_level = 6; show_level >= 1; --show_level)
			entities += DSF_ExportTileRecursive(apt, resolver, package, cull_bounds, safe_bounds, rsrc, &cbs, writer, problem_children, show_level, &DSF_export_info);

		Assert(DSF_export_info.orthoImg.data == NULL); //  In this type of export - orthoimages are not allowed. So this should never happen.

		rsrc.write_tables(cbs,writer);

		fclose(dsf);
		return 1;
	}
	else
		return 0;
}
