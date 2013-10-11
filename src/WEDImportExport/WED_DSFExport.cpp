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
#include "WED_UIDefs.h"
#include "WED_AptIE.h"
#include "DSFLib.h"
#include "FileUtils.h"
#include "WED_Entity.h"
#include "PlatformUtils.h"
//#include "GISUtils.h"
#include "CompGeomDefs2.h"
#include "WED_Group.h"
#include "WED_Version.h"
#include "WED_ToolUtils.h"
#include "WED_TextureNode.h"
#include "ILibrarian.h"
#include "WED_ObjPlacement.h"
#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_StringPlacement.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_ExclusionZone.h"
#include "WED_EnumSystem.h"
#include "WED_Airport.h"
#include "WED_GISUtils.h"
#include "WED_Group.h"
#include "WED_Validate.h"
#include "MathUtils.h"
#include "DSF2Text.h"
#include "WED_Clipping.h"
#include "zip.h"
#include <stdarg.h>
#include <time.h>

// This is how much outside the DSF bounds we can legally go.
// Between you, me, and the wall, X-Plane 10.21 actually allows
// a full 0.5 degrees of 'extra'.  But...that's a LOT more than we
// should ever need.  So: set our safe bounds WELL inside X-Plane's.
// If we lose a bit or 2 in the DSF encoder, we won't have written
// something on the ragged edge.
#define DSF_EXTRA_1021 0.25


#if !NO_CGAL_BEZIER
#include "Bezier.h"
#endif
//#include <CGAL/Boolean_set_operations_2.h>
//#include "GISTool_Globals.h"

//---------------------------------------------------------------------------------------------------------------------------------------

int zip_printf(void * fi, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char tmp[4000];
	int l = vsprintf(tmp,fmt,args);
	
	zipWriteInFileInZip((zipFile) fi, tmp, l);
	return l;
}



//---------------------------------------------------------------------------------------------------------------------------------------

// stolen from GISUtils - I got annoyed with having to grab all of CGAL for a modulo function.
inline int	latlon_bucket(int p)
{
	if (p > 0) return (p / 10) * 10;
	else return ((-p + 9) / 10) * -10;
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
/************************************************************************************************************************************************
 * DSF EXPORT UTILS
 ************************************************************************************************************************************************/

// This is the number of sub-buckets to build in the DSF writer...the larger, the more precise the DSF, but the more likely
// some big item goes across buckets and we lose precision.
#define DSF_DIVISIONS 32

static bool g_dropped_pts = false;

struct	DSF_ResourceTable {
	DSF_ResourceTable() { for(int i = 0; i < 7; ++i) show_level_obj[i] = show_level_pol[i] = -1; }
	vector<string>				obj_defs;
	map<pair<string,int>, int>	obj_defs_idx;

	vector<string>				pol_defs;
	map<pair<string, int>, int>	pol_defs_idx;

	int show_level_obj[7];
	int show_level_pol[7];

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
};

static void swap_suffix(string& f, const char * new_suffix)
{
	string::size_type p = f.find_last_of(".");
	if (p != f.npos) f.erase(p);
	f += new_suffix;
}

static void strip_path(string& f)
{
	string::size_type p = f.find_last_of(":\\/");
	if (p != f.npos) f.erase(0,p+1);
}

void assemble_dsf_pt(double c[8], const Point2& pt, const Point2 * bez, UVMap_t * uv, const Bbox2& bounds)
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
	if(!bez && uv)
	{
		Point2		u;
		WED_MapPoint(*uv,pt,u);
		c[2] = u.x();
		c[3] = u.y();
	}
	if(bez && uv)
	{
		Point2		u, v;
		WED_MapPoint(*uv,pt,u);
		WED_MapPoint(*uv,*bez,v);
		c[4] = u.x();
		c[5] = u.y();
		c[6] = v.x();
		c[7] = v.y();
	}
}	

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
				assemble_dsf_pt(c, pts_triple[i].pt, &pts_triple[i].hi, NULL, bounds);
				if(!auto_closed || i != (pts_triple.size()-1))
				{
//					debug_mesh_line(pts_triple[i].pt,pts_triple[i].hi,1,1,1,0,1,0);
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
						int										auto_closed)						
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
			
			printf("Original pts:\n");
			for(vector<BezierPoint2p>::iterator i = pts.begin(); i != pts.end(); ++i)
				printf("%lf,%lf | %lf,%lf | %lf, %lf (%d)\n", 
						i->lo.x(), i->lo.y(),
						i->pt.x(), i->pt.y(),
						i->hi.x(), i->hi.y(),
						i->param);
			
			BezierPointSeqToTriple(pts.begin(),pts.end(),back_inserter(pts_triple));

			printf("Triple pts:\n");
			for(vector<BezierPoint2p>::iterator i = pts_triple.begin(); i != pts_triple.end(); ++i)
				printf("%lf,%lf | %lf,%lf | %lf, %lf (%d)\n", 
						i->lo.x(), i->lo.y(),
						i->pt.x(), i->pt.y(),
						i->hi.x(), i->hi.y(),
						i->param);
			
			for(int i = 0; i < pts_triple.size(); ++i)
			{
				assemble_dsf_pt_param(c, 
						pts_triple[i].pt, 
						pts_triple[i].param,
						&pts_triple[i].hi, 
						bounds);
				printf("bezier: %f %f %f   %f %f\n", c[0],c[1],c[2],c[3],c[4]);
				if(!auto_closed || i != (pts_triple.size()-1))
				{
//					debug_mesh_line(pts_triple[i].pt,pts_triple[i].hi,1,1,1,0,1,0);
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

		assemble_dsf_pt(c, i->source(), NULL, NULL, bounds);							// Start point _always_ written - it has the line type.
		cbs->AddPolygonPoint_f(c,writer);			

		if(next != end && i->target() != next->source())								// Discontinuity mid-line?  Write the end, close and open.
		{
			assemble_dsf_pt(c, i->target(), NULL, NULL, bounds);
			cbs->AddPolygonPoint_f(c,writer);			
			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
			cbs->BeginPolygon_f(idx, param, 2, writer);
			cbs->BeginPolygonWinding_f(writer);
		}
		else if(next == end && (i->target() != start->source() || !auto_closed))		// If we are ending AND we need a last point, write it.
		{																				// We need that last pt if we are not closed or if the
			assemble_dsf_pt(c, i->target(), NULL, NULL, bounds);						// closure is not part of the DSF
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
						int									auto_closed)
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

		assemble_dsf_pt(c, i->source(), NULL, NULL, bounds);
		c[2] = i->param;
		cbs->AddPolygonPoint_f(c,writer);			

		if(next != end && i->target() != next->source())
		{
			assemble_dsf_pt(c, i->target(), NULL, NULL, bounds);
			c[2] = i->param;
			cbs->AddPolygonPoint_f(c,writer);			
			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
			cbs->BeginPolygon_f(idx, param, 3, writer);
			cbs->BeginPolygonWinding_f(writer);
		}
		else if(next == end && (i->target() != start->source() || !auto_closed))
		{
			assemble_dsf_pt(c, i->target(), NULL, NULL, bounds);
			c[2] = i->param;
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
		assemble_dsf_pt(c, *i, NULL, NULL, bounds);
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
						UVMap_t *							uvmap,
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

		assemble_dsf_pt(crd, pts3[p].pt, &pts3[p].hi, uvmap, bounds);
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

void DSF_AccumPolygon(
						Polygon2&							poly,
						UVMap_t *							uvmap,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{	
	for(Polygon2::iterator v = poly.begin(); v != poly.end(); ++v)
	{
		double c[4];
		assemble_dsf_pt(c, *v, NULL, uvmap, bounds);
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


void DSF_AccumPolygonWithHolesBezier(
						vector<BezierPolygon2>&				poly,
						UVMap_t *							uvmap,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
//	cbs->BeginPolygonWinding_f(writer);
	for(vector<BezierPolygon2>::iterator w = poly.begin(); w != poly.end(); ++w)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygonBezier(*w, uvmap, bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}	
}	

void DSF_AccumPolygonWithHolesBezier(
						vector<BezierPolygon2p>&			poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
	cbs->BeginPolygonWinding_f(writer);
	for(int n = 0; n < poly.size(); ++n)
	DSF_AccumPolygonBezier(poly[n], bounds, cbs, writer);
	cbs->EndPolygonWinding_f(writer);
}	
						
void DSF_AccumPolygonWithHoles(
						vector<Polygon2>&					poly,
						UVMap_t *							uvmap,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
	for(vector<Polygon2>::iterator h = poly.begin(); h != poly.end(); ++h)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygon(*h, uvmap, bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}	

}

void DSF_AccumPolygonWithHoles(
						vector<Polygon2p>&					poly,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
	cbs->BeginPolygonWinding_f(writer);
	for(int n = 0; n < poly.size(); ++n)
		DSF_AccumPolygon(poly[n], bounds, cbs, writer);
	cbs->EndPolygonWinding_f(writer);
}

static int	DSF_HeightRangeRecursive(WED_Thing * what, double& out_msl_min, double& out_msl_max, const Bbox2& bounds)
{
	WED_ObjPlacement * obj;
	IGISEntity * ent;
	if((ent = dynamic_cast<IGISEntity *>(what)) != NULL)
	{
		Bbox2	cbounds;
		ent->GetBounds(gis_Geo, cbounds);
		if(!cbounds.overlap(bounds))
			return 0;
	}
	
	
	if((obj = dynamic_cast<WED_ObjPlacement *>(what)) != NULL)
	{
#if AIRPORT_ROUTING	
		if(obj->HasCustomMSL())
		{
			out_msl_min = out_msl_max = obj->GetCustomMSL();
			return 1;
		}
#endif		
	}
	int found = 0;
	int nn = what->CountChildren();
	for(int n = 0; n < nn; ++n)
	{
		double msl_min, msl_max;
		if (DSF_HeightRangeRecursive(what->GetNthChild(n),msl_min,msl_max, bounds))
		{
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
	}
	return found;
}

static int	DSF_ExportTileRecursive(
						WED_Thing *					what, 
						ILibrarian *				pkg, 
						const Bbox2&				cull_bounds,		// This is the area for which we are TRYING to get scenery.
						const Bbox2&				safe_bounds,		// This is the 'safe' area into which we CAN write scenery without exploding.
						DSF_ResourceTable&			io_table, 
						const DSFCallbacks_t *		cbs, 
						void *						writer,
						set<WED_Thing *>&			problem_children,
						int							show_level)
{
	int real_thingies = 0;
	
	WED_ObjPlacement * obj;
	WED_FacadePlacement * fac;
	WED_ForestPlacement * fst;
	WED_StringPlacement * str;
	WED_LinePlacement * lin;
	WED_PolygonPlacement * pol;
	WED_DrapedOrthophoto * orth;
	WED_ExclusionZone * xcl;

	int idx;
	string r;
	Point2	p;
	WED_Entity * ent = dynamic_cast<WED_Entity *>(what);
	if (!ent) return 0;
	if (ent->GetHidden()) return 0;
	
	IGISEntity * e = dynamic_cast<IGISEntity *>(what);
	
		Bbox2	ent_box;
		e->GetBounds(gis_Geo,ent_box);
	
	if(!ent_box.overlap(cull_bounds))
		return 0;
		
	Point2	centroid = ent_box.centroid();
	bool centroid_ob = false;
	if(centroid.x() < cull_bounds.xmin() ||
	   centroid.y() < cull_bounds.ymin() ||
	   centroid.x() >=cull_bounds.xmax() ||
	   centroid.y() >=cull_bounds.ymax())
	{
		centroid_ob = true;
	}


	if((xcl = dynamic_cast<WED_ExclusionZone *>(what)) != NULL)
	if(show_level == 6)
	{
		set<int> xtypes;
		xcl->GetExclusions(xtypes);
		Point2 minp, maxp;
		xcl->GetMin()->GetLocation(gis_Geo,minp);
		xcl->GetMax()->GetLocation(gis_Geo,maxp);
		
		if(minp.x_ > maxp.x_)	swap(minp.x_, maxp.x_);
		if(minp.y_ > maxp.y_)	swap(minp.y_, maxp.y_);
		
		for(set<int>::iterator xt = xtypes.begin(); xt != xtypes.end(); ++xt)
		{
			const char * pname = NULL;
			switch(*xt) {
			case exclude_Obj:	pname = "sim/exclude_obj";	break;
			case exclude_Fac:	pname = "sim/exclude_fac";	break;
			case exclude_For:	pname = "sim/exclude_for";	break;
			case exclude_Bch:	pname = "sim/exclude_bch";	break;
			case exclude_Net:	pname = "sim/exclude_net";	break;

			case exclude_Lin:	pname = "sim/exclude_lin";	break;
			case exclude_Pol:	pname = "sim/exclude_pol";	break;
			case exclude_Str:	pname = "sim/exclude_str";	break;
			}
			if(pname)
			{
				char valbuf[512];
				sprintf(valbuf,"%.6lf/%.6lf/%.6lf/%.6lf",minp.x(),minp.y(),maxp.x(),maxp.y());
				++real_thingies;
				cbs->AcceptProperty_f(pname, valbuf, writer);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------------------
	// OBJECT EXPORTER
	//------------------------------------------------------------------------------------------------------------	

	if((obj = dynamic_cast<WED_ObjPlacement *>(what)) != NULL)
	if(show_level == obj->GetShowLevel())
	{
		obj->GetResource(r);
		idx = io_table.accum_obj(r,show_level);
		obj->GetLocation(gis_Geo,p);
		if(cull_bounds.contains(p))
		{
			double xy[3] = { p.x(), p.y(), 0.0 };
			float heading = obj->GetHeading();
			while(heading < 0) heading += 360.0;
			while(heading >= 360.0) heading -= 360.0;
			++real_thingies;
			#if AIRPORT_ROUTING
			if(obj->HasCustomMSL())
			{
				xy[2] = obj->GetCustomMSL();
				cbs->AddObjectAbsolute_f(idx, xy, heading, writer);
			}
			else
			#endif
				cbs->AddObject_f(idx, xy, heading, writer);
		}
	}
	
	//------------------------------------------------------------------------------------------------------------
	// FACADE EXPORTER
	//------------------------------------------------------------------------------------------------------------
	
	if((fac = dynamic_cast<WED_FacadePlacement *>(what)) != NULL)
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
					DSF_AccumPolygonWithHolesBezier(*i, NULL, safe_bounds, cbs, writer);
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
					DSF_AccumPolygonWithHoles(*i, NULL, safe_bounds, cbs, writer);
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
					vector<Segment2p>	chain;
				
					WED_VectorForPointSequence(seq,chain);
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
	
	//------------------------------------------------------------------------------------------------------------
	// FOREST EXPORTER
	//------------------------------------------------------------------------------------------------------------

	if((fst = dynamic_cast<WED_ForestPlacement *>(what)) != NULL)
	if(show_level == 6)
	{
		fst->GetResource(r);
		idx = io_table.accum_pol(r,show_level);

		DebugAssert(!WED_HasBezierPol(fst));
		int param = intlim(fst->GetDensity() * 255.0,0,255) + fst->GetFillMode() * 256;
		switch(fst->GetFillMode()) {
		case dsf_fill_area:
			{
				vector<Polygon2>	fst_area;
				Assert(WED_PolygonWithHolesForPolygon(fst,fst_area));

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
					DSF_AccumPolygonWithHoles(*i, NULL, safe_bounds, cbs, writer);
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
					clip_segments(chain, cull_bounds);

					++real_thingies;
					DSF_AccumChain(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, param, 0);
				}
			}
			break;
		case dsf_fill_points:
			for(int h = -1; h < fst->GetNumHoles(); ++h)
			{
				IGISPointSequence * seq = (h == -1) ? fst->GetOuterRing() : fst->GetNthHole(h);
				vector<Point2>	pts;
				
				for(int p = 0; p < seq->GetNumPoints(); ++p)
				{
					Point2 x;
					seq->GetNthPoint(p)->GetLocation(gis_Geo,x);
					if(safe_bounds.contains(x))
					{
						pts.push_back(x);
					}
				}
				if(!pts.empty())
				{
					++real_thingies;
					DSF_AccumPts(pts.begin(),pts.end(), safe_bounds, cbs,writer, idx, param);
				}
			}
			break;
		}		
	}
	
	//------------------------------------------------------------------------------------------------------------
	// OBJ STRING EXPORTER
	//------------------------------------------------------------------------------------------------------------
	
	
	if((str = dynamic_cast<WED_StringPlacement *>(what)) != NULL)
	if(show_level == 6)
	{
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
				DSF_AccumChainBezier(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, str->GetSpacing(), 0);
			}
		}
		else
		{		
			vector<Segment2>	chain;
		
			WED_VectorForPointSequence(str,chain);
			clip_segments(chain, cull_bounds);
			if(!chain.empty())
			{
				++real_thingies;
				DSF_AccumChain(chain.begin(),chain.end(), safe_bounds, cbs,writer, idx, str->GetSpacing(), 0);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------------------
	// OBJ LINE EXPORTER
	//------------------------------------------------------------------------------------------------------------

	if((lin = dynamic_cast<WED_LinePlacement *>(what)) != NULL)
	if(show_level == 6)
	{
		lin->GetResource(r);
		idx = io_table.accum_pol(r,show_level);
		bool bez = WED_HasBezierSeq(lin);

		if(bez)
		{
			vector<Bezier2>	chain;
		
			WED_BezierVectorForPointSequence(lin,chain);

			clip_segments(chain, cull_bounds);

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
		else
		{		
			vector<Segment2>	chain;
		
			WED_VectorForPointSequence(lin,chain);
			clip_segments(chain, cull_bounds);

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

	//------------------------------------------------------------------------------------------------------------
	// DRAPED POLYGON
	//------------------------------------------------------------------------------------------------------------

	if((pol = dynamic_cast<WED_PolygonPlacement *>(what)) != NULL)
	if(show_level == 6)
	{
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
				cbs->BeginPolygon_f(idx,pol->GetHeading(),bez ? 4 : 2,writer);
				DSF_AccumPolygonWithHolesBezier(*i, NULL, safe_bounds, cbs, writer);
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
				cbs->BeginPolygon_f(idx,pol->GetHeading(),bez ? 4 : 2,writer);
				DSF_AccumPolygonWithHoles(*i, NULL, safe_bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------------------
	// UV-MAPPED DRAPED POLYGON
	//------------------------------------------------------------------------------------------------------------
	
	if((orth = dynamic_cast<WED_DrapedOrthophoto *>(what)) != NULL)
	if(show_level == 6)
	{
		orth->GetResource(r);
		idx = io_table.accum_pol(r,show_level);
		bool bez = WED_HasBezierPol(orth);

		UVMap_t	uv;
		WED_MakeUVMap(orth,uv);

		if(bez)
		{
			vector<BezierPolygon2>			orth_area;
			vector<vector<BezierPolygon2> >	orth_cuts;
			WED_BezierPolygonWithHolesForPolygon(orth, orth_area);

			if(!clip_polygon(orth_area,orth_cuts,cull_bounds))
			{
				problem_children.insert(what);
				orth_cuts.clear();				
			}

			for(vector<vector<BezierPolygon2> >::iterator i = orth_cuts.begin(); i != orth_cuts.end(); ++i)
			{
				++real_thingies;
				cbs->BeginPolygon_f(idx,65535,bez ? 8 : 4,writer);
				DSF_AccumPolygonWithHolesBezier(*i, &uv, safe_bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}
		}
		else
		{
			vector<Polygon2>			orth_area;
			vector<vector<Polygon2> >	orth_cuts;
			Assert(WED_PolygonWithHolesForPolygon(orth,orth_area));

			if(!clip_polygon(orth_area,orth_cuts,cull_bounds))
			{
				problem_children.insert(what);
				orth_cuts.clear();
			}
			
			for(vector<vector<Polygon2> >::iterator i = orth_cuts.begin(); i != orth_cuts.end(); ++i)
			{
				++real_thingies;
				cbs->BeginPolygon_f(idx,65535,bez ? 8 : 4,writer);
				DSF_AccumPolygonWithHoles(*i, &uv, safe_bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}
		}
	}


	//------------------------------------------------------------------------------------------------------------
	// RECURSION
	//------------------------------------------------------------------------------------------------------------


	int cc = what->CountChildren();
	for (int c = 0; c < cc; ++c)
		real_thingies += DSF_ExportTileRecursive(what->GetNthChild(c), pkg, cull_bounds, safe_bounds, io_table, cbs, writer, problem_children,show_level);
	return real_thingies;	
}

static void DSF_ExportTile(WED_Group * base, ILibrarian * pkg, int x, int y, set <WED_Thing *>& problem_children)
{
	void *			writer;
	DSFCallbacks_t	cbs;
	char	prop_buf[256];

	double msl_min, msl_max;
	
	Bbox2	cull(x-1,y-1,x+2,y+2);
	
	if(DSF_HeightRangeRecursive(base,msl_min,msl_max, cull))
	{
		msl_min = floor(msl_min);
		msl_max = ceil(msl_max);
		if(msl_min == msl_max)
			msl_max = msl_min + 1.0;
	}
	else
		msl_min = -32768.0, msl_max = 32767.0;
	

	writer = DSFCreateWriter(x,y,x+1,y+1, msl_min,msl_max,DSF_DIVISIONS);
	DSFGetWriterCallbacks(&cbs);

	sprintf(prop_buf, "%d", (int) x  );		cbs.AcceptProperty_f("sim/west", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) x+1);		cbs.AcceptProperty_f("sim/east", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) y+1);		cbs.AcceptProperty_f("sim/north", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) y  );		cbs.AcceptProperty_f("sim/south", prop_buf, writer);
	cbs.AcceptProperty_f("sim/planet", "earth", writer);
	cbs.AcceptProperty_f("sim/creation_agent", "WorldEditor" WED_VERSION_STRING, writer);
	cbs.AcceptProperty_f("laminar/internal_revision", "0", writer);
	cbs.AcceptProperty_f("sim/overlay", "1", writer);

	DSF_ResourceTable	rsrc;

	Bbox2	cull_bounds(x,y,x+1,y+1);
	Bbox2	safe_bounds(cull_bounds);
	if(gExportTarget == wet_xplane_1021)
		safe_bounds.expand(DSF_EXTRA_1021);
	
	
	int entities = 0;
	for(int show_level = 6; show_level >= 1; --show_level)	
		entities += DSF_ExportTileRecursive(base, pkg, cull_bounds, safe_bounds, rsrc, &cbs, writer,problem_children,show_level);

	for(vector<string>::iterator s = rsrc.obj_defs.begin(); s != rsrc.obj_defs.end(); ++s)
		cbs.AcceptObjectDef_f(s->c_str(), writer);

	for(vector<string>::iterator s = rsrc.pol_defs.begin(); s != rsrc.pol_defs.end(); ++s)
		cbs.AcceptPolygonDef_f(s->c_str(), writer);
		
	for(int i = 1; i <= 6; ++i)
	{
		char buf[20];
		if(rsrc.show_level_obj[i] != -1)
		{
			sprintf(buf,"%d/%d",i,rsrc.show_level_obj[i]);
			cbs.AcceptProperty_f("sim/require_agpoint", buf, writer);
			cbs.AcceptProperty_f("sim/require_object", buf, writer);
		}
		if(rsrc.show_level_pol[i] != -1)
		{
			sprintf(buf,"%d/%d",i,rsrc.show_level_pol[i]);
			cbs.AcceptProperty_f("sim/require_facade", buf, writer);
		}
	}		

	char	rel_dir [512];
	char	rel_path[512];
	string full_dir, full_path;
	sprintf(rel_dir ,"Earth nav data" DIR_STR "%+03d%+04d",							  latlon_bucket(y), latlon_bucket(x)	  );
	sprintf(rel_path,"Earth nav data" DIR_STR "%+03d%+04d" DIR_STR "%+03d%+04d.dsf",  latlon_bucket(y), latlon_bucket(x), y, x);
	full_path = rel_path;
	full_dir  = rel_dir ;
	pkg->LookupPath(full_dir );
	pkg->LookupPath(full_path);
	if(entities)	// empty DSF?  Don't write a empty file, makes a mess!
	{
		FILE_make_dir_exist(full_dir.c_str());
		DSFWriteToFile(full_path.c_str(), writer);
	}
	DSFDestroyWriter(writer);
}

void DSF_Export(WED_Group * base, ILibrarian * package, set<WED_Thing *>& problem_children)
{
	g_dropped_pts = false;
	Bbox2	wrl_bounds;
	base->GetBounds(gis_Geo,wrl_bounds);
	int tile_west  = floor(wrl_bounds.p1.x());
	int tile_east  = ceil (wrl_bounds.p2.x());
	int tile_south = floor(wrl_bounds.p1.y());
	int tile_north = ceil (wrl_bounds.p2.y());

	for (int y = tile_south; y < tile_north; ++y)
	for (int x = tile_west ; x < tile_east ; ++x)
	{
		DSF_ExportTile(base, package, x, y, problem_children);
	}

	if(g_dropped_pts)
		DoUserAlert("Warning: you have bezier curves that cross a DSF tile boundary.  X-Plane 9 cannot handle this case.  To fix this, only use non-curved polygons to cross a tile boundary.");		
}

int DSF_ExportOneAirportOverlayRecursive(IResolver * resolver, WED_Thing  * who, zipFile archive, set<WED_Thing *>& problem_children)
{
	if(who->GetClass() == WED_Airport::sClass)
	{
		WED_Airport * apt = dynamic_cast<WED_Airport *>(who);
		if(apt->GetHidden())
			return 1;

		//----------------------------------------------------------------------------------------------------

		ILibrarian * pkg = WED_GetLibrarian(resolver);
		string icao;
		apt->GetICAO(icao);

		icao += ".txt";
			
		zip_fileinfo	fi = { 0 };
		//http://unix.stackexchange.com/questions/14705/the-zip-formats-external-file-attribute
		fi.external_fa = 0100777 << 16;

		time_t		t;			
		time(&t);
		struct tm * our_time = localtime(&t);
		
		if(our_time)
		{
			fi.tmz_date.tm_sec  = our_time->tm_sec ;
			fi.tmz_date.tm_min  = our_time->tm_min ;
			fi.tmz_date.tm_hour = our_time->tm_hour;
			fi.tmz_date.tm_mday = our_time->tm_mday;
			fi.tmz_date.tm_mon  = our_time->tm_mon ;
			fi.tmz_date.tm_year = our_time->tm_year + 1900;
		}
			
		if(zipOpenNewFileInZip (archive,icao.c_str(),
						&fi,		// mod dates, etc??
						NULL,0,
						NULL,0,
						NULL,		// comment
						Z_DEFLATED,
						Z_DEFAULT_COMPRESSION) != 0)
		{
			string msg = string("Unable to write to zip file.");
			return 0;
		}
		
		DSFCallbacks_t	cbs;
		DSF2Text_CreateWriterCallbacks(&cbs);
		print_funcs_s pf;
		pf.print_func = (int (*)(void *,const char *,...)) zip_printf;
		pf.ref = archive;
		
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
		
		int entities = 0;
		for(int show_level = 6; show_level >= 1; --show_level)	
			entities += DSF_ExportTileRecursive(who, pkg, cull_bounds, safe_bounds, rsrc, &cbs, writer,problem_children,show_level);

		for(vector<string>::iterator s = rsrc.obj_defs.begin(); s != rsrc.obj_defs.end(); ++s)
			cbs.AcceptObjectDef_f(s->c_str(), writer);

		for(vector<string>::iterator s = rsrc.pol_defs.begin(); s != rsrc.pol_defs.end(); ++s)
			cbs.AcceptPolygonDef_f(s->c_str(), writer);
			
		for(int i = 1; i <= 6; ++i)
		{
			char buf[20];
			if(rsrc.show_level_obj[i] != -1)
			{
				sprintf(buf,"%d/%d",i,rsrc.show_level_obj[i]);
				cbs.AcceptProperty_f("sim/require_agpoint", buf, writer);
				cbs.AcceptProperty_f("sim/require_object", buf, writer);
			}
			if(rsrc.show_level_pol[i] != -1)
			{
				sprintf(buf,"%d/%d",i,rsrc.show_level_pol[i]);
				cbs.AcceptProperty_f("sim/require_facade", buf, writer);
			}
		}		
	
		zipCloseFileInZip(archive);

		//-------------------------------------------------------------------

		apt->GetICAO(icao);

		icao += ".dat";

		if(zipOpenNewFileInZip (archive, icao.c_str(),
						&fi,		// mod dates, etc??
						NULL,0,
						NULL,0,
						NULL,		// comment
						Z_DEFLATED,
						Z_DEFAULT_COMPRESSION) != 0)
		{
			string msg = string("Unable to write to zip file.");
			return 0;
		}
		
		WED_AptExport(apt, zip_printf, archive);
		zipCloseFileInZip(archive);
		


		return 1;
	}
	else if(who->GetClass() == WED_Group::sClass)
	{
		WED_Group * g = dynamic_cast<WED_Group *>(who);
		if(g && g->GetHidden())
			return 1;
			
		int n = who->CountChildren();
		for(int i = 0; i < n; ++i)
		{
			WED_Thing * child = who->GetNthChild(i);
			if(DSF_ExportOneAirportOverlayRecursive(resolver, child, archive,problem_children) == 0)
			{
				// err on export?  bail out of recursion.
				return 0;
			}
		}
		return 1;
	} 
	else
	{
		problem_children.insert(who);
		DoUserAlert("You cannot export this scenery pack for Robin because some DSF overlay elements are not inside an airport.");
		return 0;
	}
}



int		WED_CanExportPack(IResolver * resolver)
{
	return 1;
}

void	WED_DoExportPack(IResolver * resolver)
{
	// Just don't ever export if we are invalid.  Avoid the case where we write junk to a file!
	if(!WED_ValidateApt(resolver))
		return;

	ILibrarian * l = WED_GetLibrarian(resolver);
	WED_Thing * w = WED_GetWorld(resolver);
	set<WED_Thing *>	problem_children;
	DSF_Export(dynamic_cast<WED_Group *>(w), l,problem_children);

	string	apt = "Earth nav data" DIR_STR "apt.dat";
	string	apt_dir = "Earth nav data";
	l->LookupPath(apt);
	l->LookupPath(apt_dir);

	FILE_make_dir_exist(apt_dir.c_str());
	WED_AptExport(w, apt.c_str());
	if(!problem_children.empty())
	{
		DoUserAlert("One or more objects could not exported - check for self intersecting polygons and closed-ring facades crossing DFS boundaries.");
		ISelection * sel = WED_GetSelect(resolver);
		(*problem_children.begin())->StartOperation("Select broken items.");
		sel->Clear();
		for(set<WED_Thing*>::iterator p = problem_children.begin(); p != problem_children.end(); ++p)
			sel->Insert(*p);
		(*problem_children.begin())->CommitOperation();		
	}
}

void	WED_DoExportRobin(IResolver * resolver)
{
	WED_Export_Target old_target = gExportTarget;
	gExportTarget = wet_robin;

	if(!WED_ValidateApt(resolver))
	{
		gExportTarget = old_target;
		return;
	}

	char path[1024];
	strcpy(path,"");
	if (GetFilePathFromUser(getFile_Save,"Save for Global Airports...", "Export...", FILE_DIALOG_EXPORT_ROBIN, path, sizeof(path)))
	{	
		if(strlen(path) < 4 ||
			strcmp(path+strlen(path)-4,".zip") != 0)
			strncat(path,".zip",sizeof(path)-strlen(path)-1);
		WED_Thing * w = WED_GetWorld(resolver);
		set<WED_Thing *>	problem_children;
		
		zipFile archive = zipOpen(path, 0);
		if(archive == NULL)
		{
			DoUserAlert("Unable to open zip archive.");
			gExportTarget = old_target;
			return;
		}
	
		if(DSF_ExportOneAirportOverlayRecursive(resolver, w, archive, problem_children))
		{
			// success.
		}
		zipClose(archive, NULL);

		if(!problem_children.empty())
		{
			DoUserAlert("One or more objects could not exported - check for self intersecting polygons and closed-ring facades crossing DFS boundaries.");
			ISelection * sel = WED_GetSelect(resolver);
			(*problem_children.begin())->StartOperation("Select broken items.");
			sel->Clear();
			for(set<WED_Thing*>::iterator p = problem_children.begin(); p != problem_children.end(); ++p)
				sel->Insert(*p);
			(*problem_children.begin())->CommitOperation();		
		}
	}
	gExportTarget = old_target;

}

