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
#include "WED_AptIE.h"
#include "DSFLib.h"
#include "FileUtils.h"
#include "WED_Entity.h"
#include "PlatformUtils.h"
#include "GISUtils.h"
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
#include "WED_GISUtils.h"
#include "MathUtils.h"
#include "Bezier.h"
#include <CGAL/Boolean_set_operations_2.h>
#include "GISTool_Globals.h"

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
#define MAX_OUTSIDE 0.00001
static void gentle_crop(Point2& p, const Bbox2& bounds, bool& hard_crop)
{
	double x_outside = max(bounds.xmin() - p.x(), p.x() - bounds.xmax());
	double y_outside = max(bounds.ymin() - p.y(), p.y() - bounds.ymax());
	if(x_outside > MAX_OUTSIDE || y_outside > MAX_OUTSIDE)
		hard_crop = true;
	p = bounds.clamp(p);
}

/************************************************************************************************************************************************
 * BEZIER AND SEGMENT POLYGON CUTTING
 ************************************************************************************************************************************************/

// These routines cut down polygons with holes into sets of polygons with holes based on a bounding box.
// Note that one polygon can be many - for example, if the top of a U is cut from, we get two small polygons.

static void push_mono_curves(Bezier_curve_2 b, Bezier_polygon_2& out_pol)
{
	Bezier_traits_2 traits;
	Bezier_traits_2::Make_x_monotone_2	monifier(traits.make_x_monotone_2_object());
	
	vector<CGAL::Object>	mono_curves;
	
	monifier(b, back_inserter(mono_curves));
	
	for(vector<CGAL::Object>::iterator o = mono_curves.begin(); o != mono_curves.end(); ++o)
	{
		Bezier_polygon_2::X_monotone_curve_2	c;
		if(CGAL::assign(c,*o))
			out_pol.push_back(c);
		else
			Assert(!"No cast.");
	}
}


void ClipBezierPolygon(Bezier_polygon_with_holes_2& poly, const Bbox2& bounds, vector<Bezier_polygon_with_holes_2>& cuts)
{
	Bezier_polygon_2	clip;

	push_mono_curves(ben2cgal(Bezier2(bounds.bottom_side())),clip);
	push_mono_curves(ben2cgal(Bezier2(bounds.right_side())),clip);
	push_mono_curves(ben2cgal(Bezier2(bounds.top_side())),clip);
	push_mono_curves(ben2cgal(Bezier2(bounds.left_side())),clip);

	CGAL::intersection(poly, clip, back_inserter(cuts));

}

void ClipPolygon(Polygon_with_holes_2& poly, const Bbox2& bounds, vector<Polygon_with_holes_2>& cuts)
{
	Polygon_2	clip;
	clip.push_back(Point_2(bounds.xmin(), bounds.ymin()));
	clip.push_back(Point_2(bounds.xmax(), bounds.ymin()));
	clip.push_back(Point_2(bounds.xmax(), bounds.ymax()));
	clip.push_back(Point_2(bounds.xmin(), bounds.ymax()));
	
	CGAL::intersection(poly,clip,back_inserter(cuts));
}


/************************************************************************************************************************************************
 * BEZIER AND SEGMENT CLIPPING
 ************************************************************************************************************************************************/

// These routines push lists of curves that are cropped - we don't use the CGAL routines because we don't want curves induced at the crop edges;
// instead we simply cut each segment and keep some of them.
 
void	push_cuts_x(const Bezier2& b, double t1, double t2, vector<Bezier2>& out_curves, double x, bool want_right)
{
	Bezier2	sub;
	b.subcurve(sub, t1,t2);
	if(t1 > 0.0 && t1 < 1.0)
		sub.p1.x_ = x;
	if(t2 > 0.0 && t2 < 1.0)
		sub.p2.x_ = x;
		
	if(want_right)
	{
		if( sub.p1.x() < x ||
			sub.p2.x() < x ||
			sub.c1.x() < x ||
			sub.c2.x() < x) return;
	} else {
		if( sub.p1.x() > x ||
			sub.p2.x() > x ||
			sub.c1.x() > x ||
			sub.c2.x() > x) return;
	}
	out_curves.push_back(sub);
}

void push_cuts_x(const Segment2& s, vector<Segment2>& out_curves, double x, bool out_right)
{
	if(s.p1.x() <= x && s.p2.x() <= x && !out_right)			// These two cases are when we are entirely on the clip side	.
		out_curves.push_back(s);								// we can just take the whole segment.
	else if(s.p1.x() >= x && s.p2.x() >= x && out_right)
		out_curves.push_back(s);
	else if (s.p1.x() < x && s.p2.x() > x)						// Left to right segment, take the half we want.
	{
		if(out_right)		out_curves.push_back(Segment2(Point2(x,s.y_at_x(x)),s.p2));
		else				out_curves.push_back(Segment2(s.p1,Point2(x,s.y_at_x(x))));
	}
	else if (s.p2.x() < x && s.p1.x() > x)
	{
		if(out_right)		out_curves.push_back(Segment2(s.p1,Point2(x,s.y_at_x(x))));
		else				out_curves.push_back(Segment2(Point2(x,s.y_at_x(x)),s.p2));
	}	
}

void push_cuts_y(const Segment2& s, vector<Segment2>& out_curves, double y, bool out_top)
{
	if(s.p1.y() <= y && s.p2.y() <= y && !out_top)			// These two cases are when we are entirely on the clip side	.
		out_curves.push_back(s);								// we can just take the whole segment.
	else if(s.p1.y() >= y && s.p2.y() >= y && out_top)
		out_curves.push_back(s);
	else if (s.p1.y() < y && s.p2.y() > y)						// Left to right segment, take the half we want.
	{
		if(out_top)		out_curves.push_back(Segment2(Point2(s.x_at_y(y),y),s.p2));
		else			out_curves.push_back(Segment2(s.p1,Point2(s.x_at_y(y),y)));
	}
	else if (s.p2.y() < y && s.p1.y() > y)
	{
		if(out_top)		out_curves.push_back(Segment2(s.p1,Point2(s.x_at_y(y),y)));
		else			out_curves.push_back(Segment2(Point2(s.x_at_y(y),y),s.p2));
	}	
}

void push_cuts_x(const Segment2& s, vector<Bezier2>& out_curves, double x, bool out_right)
{
	if(s.p1.x() <= x && s.p2.x() <= x && !out_right)			// These two cases are when we are entirely on the clip side	.
		out_curves.push_back(s);								// we can just take the whole segment.
	else if(s.p1.x() >= x && s.p2.x() >= x && out_right)
		out_curves.push_back(s);
	else if (s.p1.x() < x && s.p2.x() > x)						// Left to right segment, take the half we want.
	{
		if(out_right)		out_curves.push_back(Segment2(Point2(x,s.y_at_x(x)),s.p2));
		else				out_curves.push_back(Segment2(s.p1,Point2(x,s.y_at_x(x))));
	}
	else if (s.p2.x() < x && s.p1.x() > x)
	{
		if(out_right)		out_curves.push_back(Segment2(s.p1,Point2(x,s.y_at_x(x))));
		else				out_curves.push_back(Segment2(Point2(x,s.y_at_x(x)),s.p2));
	}	
}

void push_cuts_y(const Segment2& s, vector<Bezier2>& out_curves, double y, bool out_top)
{
	if(s.p1.y() <= y && s.p2.y() <= y && !out_top)			// These two cases are when we are entirely on the clip side	.
		out_curves.push_back(s);								// we can just take the whole segment.
	else if(s.p1.y() >= y && s.p2.y() >= y && out_top)
		out_curves.push_back(s);
	else if (s.p1.y() < y && s.p2.y() > y)						// Left to right segment, take the half we want.
	{
		if(out_top)		out_curves.push_back(Segment2(Point2(s.x_at_y(y),y),s.p2));
		else			out_curves.push_back(Segment2(s.p1,Point2(s.x_at_y(y),y)));
	}
	else if (s.p2.y() < y && s.p1.y() > y)
	{
		if(out_top)		out_curves.push_back(Segment2(s.p1,Point2(s.x_at_y(y),y)));
		else			out_curves.push_back(Segment2(Point2(s.x_at_y(y),y),s.p2));
	}	
}



void	push_cuts_y(const Bezier2& b, double t1, double t2, vector<Bezier2>& out_curves, double y, bool want_top)
{
	Bezier2	sub;
	b.subcurve(sub, t1,t2);
	if(t1 > 0.0 && t1 < 1.0)
		sub.p1.y_ = y;
	if(t2 > 0.0 && t2 < 1.0)
		sub.p2.y_ = y;
		
	if(want_top)
	{
		if( sub.p1.y() < y ||
			sub.p2.y() < y ||
			sub.c1.y() < y ||
			sub.c2.y() < y) return;
	} else {
		if( sub.p1.y() > y ||
			sub.p2.y() > y ||
			sub.c1.y() > y ||
			sub.c2.y() > y) return;
	}
	out_curves.push_back(sub);
}

void	CropSegmentChainVertical(const vector<Segment2>& in_chain, vector<Segment2>& out_chain, double x, bool want_right)
{
	out_chain.clear();
	for(vector<Segment2>::const_iterator b = in_chain.begin(); b != in_chain.end(); ++b)
	{
		push_cuts_x(*b, out_chain, x, want_right);
	}
}

void	CropSegmentChainHorizontal(const vector<Segment2>& in_chain, vector<Segment2>& out_chain, double y, bool want_top)
{
	out_chain.clear();
	for(vector<Segment2>::const_iterator b = in_chain.begin(); b != in_chain.end(); ++b)
	{
		push_cuts_y(*b, out_chain,y, want_top);
	}
}

void	CropBezierChainVertical(const vector<Bezier2>& in_chain, vector<Bezier2>& out_chain, double x, bool want_right)
{
	out_chain.clear();
	for(vector<Bezier2>::const_iterator b = in_chain.begin(); b != in_chain.end(); ++b)
	{
		if(b->is_segment())
		{
			push_cuts_x(b->as_segment(), out_chain, x, want_right);		
		}
		else
		{
			double	t[4];

			int cuts = b->t_at_x(x,t);
			switch(cuts) {
			case 0:
				push_cuts_x(*b, 0,1,out_chain, x, want_right);	
				break;
			case 1:
				push_cuts_x(*b, 0,t[0],out_chain, x, want_right);	
				push_cuts_x(*b, t[0],1,out_chain, x, want_right);	
				break;
			case 2:
				push_cuts_x(*b, 0,t[0],out_chain, x, want_right);	
				push_cuts_x(*b, t[0],t[1],out_chain, x, want_right);	
				push_cuts_x(*b, t[1],1,out_chain, x, want_right);	
				break;
			case 3:
				push_cuts_x(*b, 0,t[0],out_chain, x, want_right);	
				push_cuts_x(*b, t[0],t[1],out_chain, x, want_right);	
				push_cuts_x(*b, t[1],t[2],out_chain, x, want_right);	
				push_cuts_x(*b, t[2],1,out_chain, x, want_right);	
				break;
			default:
				DebugAssert(!"Illegal t_at_x");
				break;
			}
		}
	}
}

void	CropBezierChainHorizontal(const vector<Bezier2>& in_chain, vector<Bezier2>& out_chain, double y, bool want_top)
{
	out_chain.clear();
	for(vector<Bezier2>::const_iterator b = in_chain.begin(); b != in_chain.end(); ++b)
	{
		if(b->is_segment())
		{
			push_cuts_y(b->as_segment(), out_chain, y, want_top);		
		}
		else
		{
			double	t[4];

			int cuts = b->t_at_y(y,t);
			switch(cuts) {
			case 0:
				push_cuts_y(*b, 0,1,out_chain, y, want_top);	
				break;
			case 1:
				push_cuts_y(*b, 0,t[0],out_chain, y, want_top);	
				push_cuts_y(*b, t[0],1,out_chain, y, want_top);	
				break;
			case 2:
				push_cuts_y(*b, 0,t[0],out_chain, y, want_top);	
				push_cuts_y(*b, t[0],t[1],out_chain, y, want_top);	
				push_cuts_y(*b, t[1],1,out_chain, y, want_top);	
				break;
			case 3:
				push_cuts_y(*b, 0,t[0],out_chain, y, want_top);	
				push_cuts_y(*b, t[0],t[1],out_chain, y, want_top);	
				push_cuts_y(*b, t[1],t[2],out_chain, y, want_top);	
				push_cuts_y(*b, t[2],1,out_chain, y, want_top);	
				break;
			default:
				DebugAssert(!"Illegal t_at_y");
				break;
			}
		}
	}
}


void CropBezierChainBox(const vector<Bezier2>& in_chain, vector<Bezier2>& out_chain, const Bbox2& box)
{
	vector<Bezier2>	t;
	CropBezierChainVertical(in_chain, t, box.xmin(), true);
	CropBezierChainVertical(t, out_chain, box.xmax(), false);

	CropBezierChainHorizontal(out_chain, t, box.ymin(), true);
	CropBezierChainHorizontal(t, out_chain, box.ymax(), false);
}

void CropSegmentChainBox(const vector<Segment2>& in_chain, vector<Segment2>& out_chain, const Bbox2& box)
{
	vector<Segment2>	t;
	CropSegmentChainVertical(in_chain, t, box.xmin(), true);
	CropSegmentChainVertical(t, out_chain, box.xmax(), false);

	CropSegmentChainHorizontal(out_chain, t, box.ymin(), true);
	CropSegmentChainHorizontal(t, out_chain, box.ymax(), false);
}

/************************************************************************************************************************************************
 * DSF EXPORT UTILS
 ************************************************************************************************************************************************/

// This is the number of sub-buckets to build in the DSF writer...the larger, the more precise the DSF, but the more likely
// some big item goes across buckets and we lose precision.
#define DSF_DIVISIONS 32

static bool g_dropped_pts = false;

struct	DSF_ResourceTable {
	vector<string>		obj_defs;
	map<string, int>	obj_defs_idx;

	vector<string>		polygon_defs;
	map<string, int>	polygon_defs_idx;

	int accum_obj(const string& f)
	{
		map<string,int>::iterator i = obj_defs_idx.find(f);
		if (i != obj_defs_idx.end()) return i->second;
		obj_defs.push_back(f);
		obj_defs_idx[f] = obj_defs.size()-1;
		return			  obj_defs.size()-1;
	}

	int accum_pol(const string& f)
	{
		map<string,int>::iterator i = polygon_defs_idx.find(f);
		if (i != polygon_defs_idx.end()) return i->second;
		polygon_defs.push_back(f);
		polygon_defs_idx[f] = polygon_defs.size()-1;
		return				  polygon_defs.size()-1;
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

static int DSF_HasBezierSeq(IGISPointSequence * ring)
{
	int np = ring->GetNumPoints();
	IGISPoint_Bezier * b;
	Point2 d;
	for(int n = 0; n < np; ++n)
	if(ring->GetNthPoint(n)->GetGISClass() == gis_Point_Bezier)
	if((b =dynamic_cast<IGISPoint_Bezier *>(ring->GetNthPoint(n))) != NULL)
	if(b->GetControlHandleHi(gis_Geo,d) || b->GetControlHandleLo(gis_Geo,d))
		return 1;
	return 0;
}

static int DSF_HasBezierPol(IGISPolygon * pol)
{
	if(DSF_HasBezierSeq(pol->GetOuterRing())) return 1;
	int hc = pol->GetNumHoles();
	for(int h = 0; h < hc; ++h)
	if(DSF_HasBezierSeq(pol->GetNthHole(h))) return 1;
	return 0;
}

void assemble_dsf_pt(double c[8], const Point_2& pt, const Point_2 * bez, UVMap_t * uv, const Bbox2& bounds)
{	
	Point2	p = cgal2ben(pt);
	gentle_crop(p, bounds, g_dropped_pts);
	c[0] = p.x();
	c[1] = p.y();

	if(bez)
	{
		Point2 b = cgal2ben(*bez);
		gentle_crop(b, bounds, g_dropped_pts);
		c[2] = b.x();
		c[3] = b.y();
	}
	if(!bez && uv)
	{
		Point_2		u;
		WED_MapPoint(*uv,pt,u);
		Point2 uu = cgal2ben(u);
		c[2] = uu.x();
		c[3] = uu.y();
	}
	if(bez && uv)
	{
		Point_2		u, v;
		WED_MapPoint(*uv,pt,u);
		WED_MapPoint(*uv,*bez,v);
		Point2 uu = cgal2ben(u);
		Point2 vv = cgal2ben(v);
		c[4] = uu.x();
		c[5] = uu.y();
		c[6] = vv.x();
		c[7] = vv.y();
	}
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
		Point_2		u;
		WED_MapPoint(*uv,ben2cgal(pt),u);
		Point2 uu = cgal2ben(u);
		c[2] = uu.x();
		c[3] = uu.y();
	}
	if(bez && uv)
	{
		Point_2		u, v;
		WED_MapPoint(*uv,ben2cgal(pt),u);
		WED_MapPoint(*uv,ben2cgal(*bez),v);
		Point2 uu = cgal2ben(u);
		Point2 vv = cgal2ben(v);
		c[4] = uu.x();
		c[5] = uu.y();
		c[6] = vv.x();
		c[7] = vv.y();
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
						int										closed)						
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
				if(!closed || i != (pts_triple.size()-1))
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
						vector<Segment2>::const_iterator	start,
						vector<Segment2>::const_iterator	end,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer,
						int									idx,
						int									param,
						int									closed)
{
	vector<Segment2>::const_iterator prev = end, next;
	for(vector<Segment2>::const_iterator i = start; i != end; ++i)
	{
		next = i;
		++next;
		double c[4];
		
		if(prev == end || prev->target() != i->source())
		{
			if(prev != end)
			{
				cbs->EndPolygonWinding_f(writer);
				cbs->EndPolygon_f(writer);
			}	
			cbs->BeginPolygon_f(idx, param, 2, writer);
			cbs->BeginPolygonWinding_f(writer);

			assemble_dsf_pt(c, i->source(), NULL, NULL, bounds);
			cbs->AddPolygonPoint_f(c,writer);			
		}

		assemble_dsf_pt(c, i->target(), NULL, NULL, bounds);
		cbs->AddPolygonPoint_f(c,writer);
		
		if(next == end)
		{
			cbs->EndPolygonWinding_f(writer);
			cbs->EndPolygon_f(writer);
		}	
		prev = i;
	}	
}

/************************************************************************************************************************************************
 * DSF EXPORT CENTRAL
 ************************************************************************************************************************************************/

void DSF_AccumPolygonBezier(
						Bezier_polygon_2&					poly,
						UVMap_t *							uvmap,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
	vector<Bezier2>	converted;
	for(Bezier_polygon_2::Curve_iterator e = poly.curves_begin(); e != poly.curves_end(); ++e)
	{		
		converted.push_back(cgal2ben(*e));
	}
	
	vector<BezierPoint2>	pts, pts3;
	
	BezierToBezierPointSeq(converted.begin(),converted.end(),back_inserter(pts));
	BezierPointSeqToTriple(pts.begin(),pts.end(),back_inserter(pts3));

//	PROBLEM: CGAL not exact matched endpoints!
//	DebugAssert(pts3.front() == pts3.back());
	pts3.pop_back();
	
	for(int p = 0; p < pts3.size(); ++p)
	{	
		double crd[8];

		assemble_dsf_pt(crd, pts3[p].pt, &pts3[p].hi, uvmap, bounds);
		cbs->AddPolygonPoint_f(crd,writer);		
	}
}						
						
void DSF_AccumPolygon(
						Polygon_2&							poly,
						UVMap_t *							uvmap,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
	for(Polygon_2::Vertex_iterator v = poly.vertices_begin(); v != poly.vertices_end(); ++v)
	{
		double c[4];
		assemble_dsf_pt(c, *v, NULL, uvmap, bounds);
		
		cbs->AddPolygonPoint_f(c,writer);
	}
}						

void DSF_AccumPolygonWithHolesBezier(
						Bezier_polygon_with_holes_2&		poly,
						UVMap_t *							uvmap,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
	cbs->BeginPolygonWinding_f(writer);
	DSF_AccumPolygonBezier(poly.outer_boundary(), uvmap, bounds, cbs, writer);
	cbs->EndPolygonWinding_f(writer);
	for(Bezier_polygon_with_holes_2::Hole_iterator h = poly.holes_begin(); h != poly.holes_end(); ++h)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygonBezier(*h, uvmap, bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}	
}	
						
void DSF_AccumPolygonWithHoles(
						Polygon_with_holes_2&				poly,
						UVMap_t *							uvmap,
						const Bbox2&						bounds,
						const DSFCallbacks_t *				cbs, 
						void *								writer)
{
	cbs->BeginPolygonWinding_f(writer);
	DSF_AccumPolygon(poly.outer_boundary(), uvmap, bounds, cbs, writer);
	cbs->EndPolygonWinding_f(writer);
	for(Polygon_with_holes_2::Hole_iterator h = poly.holes_begin(); h != poly.holes_end(); ++h)
	{
		cbs->BeginPolygonWinding_f(writer);
		DSF_AccumPolygon(*h, uvmap, bounds, cbs, writer);
		cbs->EndPolygonWinding_f(writer);
	}	

}

static void	DSF_ExportTileRecursive(WED_Thing * what, ILibrarian * pkg, const Bbox2& bounds, set<string>& io_resources, DSF_ResourceTable& io_table, const DSFCallbacks_t * cbs, void * writer)
{
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
	if (!ent) return;
	if (ent->GetHidden()) return;
	
	IGISEntity * e = dynamic_cast<IGISEntity *>(what);
	
		Bbox2	ent_box;
		e->GetBounds(gis_Geo,ent_box);
	if(!ent_box.overlap(bounds))
		return;
		
	if((xcl = dynamic_cast<WED_ExclusionZone *>(what)) != NULL)
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
				cbs->AcceptProperty_f(pname, valbuf, writer);
			}
		}
	}

	if((obj = dynamic_cast<WED_ObjPlacement *>(what)) != NULL)
	{
		obj->GetResource(r);
		idx = io_table.accum_obj(r);
		obj->GetLocation(gis_Geo,p);
		if(bounds.contains(p))
		{
			double xy[2] = { p.x(), p.y() };
			float heading = obj->GetHeading();
			while(heading < 0) heading += 360.0;
			while(heading >= 360.0) heading -= 360.0;
			cbs->AddObject_f(idx, xy, heading, writer);
		}
	}
	if((fac = dynamic_cast<WED_FacadePlacement *>(what)) != NULL)
	{
		fac->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierPol(fac);
		
		if(bez)
		{
			Bezier_polygon_with_holes_2	fac_area;
			vector<Bezier_polygon_with_holes_2> cuts;
			WED_BezierPolygonWithHolesForPolygon(fac, fac_area);
			ClipBezierPolygon(fac_area,bounds, cuts);
			for(vector<Bezier_polygon_with_holes_2>::iterator ci = cuts.begin(); ci != cuts.end(); ++ci)
			{
				cbs->BeginPolygon_f(idx,fac->GetHeight(),bez ? 4 : 2,writer);			
				DSF_AccumPolygonWithHolesBezier(*ci, NULL, bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}
		}
		else
		{
			Polygon_with_holes_2	fac_area;
			vector<Polygon_with_holes_2>	cuts;
			if(WED_PolygonWithHolesForPolygon(fac,fac_area))
			{
				ClipPolygon(fac_area, bounds,cuts);
				for(vector<Polygon_with_holes_2>::iterator ci = cuts.begin(); ci != cuts.end(); ++ci)			
				{
					cbs->BeginPolygon_f(idx,fac->GetHeight(),bez ? 4 : 2,writer);
					DSF_AccumPolygonWithHoles(*ci, NULL, bounds, cbs, writer);
					cbs->EndPolygon_f(writer);
				}
			}
			else
				printf("FUBAR.\n");
		}
	}

	if((fst = dynamic_cast<WED_ForestPlacement *>(what)) != NULL)
	{
		fst->GetResource(r);
		idx = io_table.accum_pol(r);

		DebugAssert(!DSF_HasBezierPol(fst));

		Polygon_with_holes_2	fst_area;
		vector<Polygon_with_holes_2>	cuts;
		if(WED_PolygonWithHolesForPolygon(fst,fst_area))
		{
			ClipPolygon(fst_area, bounds,cuts);
			for(vector<Polygon_with_holes_2>::iterator ci = cuts.begin(); ci != cuts.end(); ++ci)			
			{
				cbs->BeginPolygon_f(idx,fst->GetDensity() * 255.0,2,writer);
				DSF_AccumPolygonWithHoles(*ci, NULL, bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}
		}
		else
			printf("foobar poly.\n");
	}
	
	if((str = dynamic_cast<WED_StringPlacement *>(what)) != NULL)
	{
		str->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierSeq(str);

		if(bez)
		{
			vector<Bezier2>	chain, cut_chain;
		
			WED_BezierVectorForPointSequence(str,chain);
			CropBezierChainBox(chain,cut_chain,bounds);

			if(!cut_chain.empty())
				DSF_AccumChainBezier(cut_chain.begin(),cut_chain.end(), bounds, cbs,writer, idx, str->GetSpacing(), 0);
		}
		else
		{		
			vector<Segment2>	chain, cut_chain;
		
			WED_VectorForPointSequence(str,chain);
			CropSegmentChainBox(chain,cut_chain,bounds);

			if(!cut_chain.empty())
				DSF_AccumChain(cut_chain.begin(),cut_chain.end(), bounds, cbs,writer, idx, str->GetSpacing(), 0);
		}
	}

	if((lin = dynamic_cast<WED_LinePlacement *>(what)) != NULL)
	{
		lin->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierSeq(lin);

		if(bez)
		{
			vector<Bezier2>	chain, cut_chain;
		
			WED_BezierVectorForPointSequence(lin,chain);
			CropBezierChainBox(chain,cut_chain,bounds);

			if(!cut_chain.empty())
			{
				int closed = 0;
				if(one_winding(cut_chain))
				{
					if(cut_chain.front().p1 == cut_chain.back().p2 && cut_chain.size() > 1)
					{
						closed = 1;
					}
				}
				else
				{
					while(cut_chain.front().p1 == cut_chain.back().p2)
					{
						cut_chain.insert(cut_chain.begin(),cut_chain.back());
						cut_chain.pop_back();
					}
				}
				DSF_AccumChainBezier(cut_chain.begin(),cut_chain.end(), bounds, cbs,writer, idx, closed, closed);
			}
		}
		else
		{		
			vector<Segment2>	chain, cut_chain;
		
			WED_VectorForPointSequence(lin,chain);
			CropSegmentChainBox(chain,cut_chain,bounds);

			if(!cut_chain.empty())
			{
				int closed = 0;
				if(one_winding(cut_chain))
				{
					if(cut_chain.front().p1 == cut_chain.back().p2 && cut_chain.size() > 1)
					{
						closed = 1;
						cut_chain.pop_back();
					}
				}
				else
				{
					while(cut_chain.front().p1 == cut_chain.back().p2)
					{
						cut_chain.insert(cut_chain.begin(),cut_chain.back());
						cut_chain.pop_back();
					}
				}
				DSF_AccumChain(cut_chain.begin(),cut_chain.end(), bounds, cbs,writer, idx, closed, closed);
			}
		}
	}

	if((pol = dynamic_cast<WED_PolygonPlacement *>(what)) != NULL)
	{
		pol->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierPol(pol);

		if(bez)
		{
			Bezier_polygon_with_holes_2	pol_area;
			vector<Bezier_polygon_with_holes_2> cuts;
			WED_BezierPolygonWithHolesForPolygon(pol, pol_area);
			ClipBezierPolygon(pol_area,bounds, cuts);
			for(vector<Bezier_polygon_with_holes_2>::iterator ci = cuts.begin(); ci != cuts.end(); ++ci)
			{
				cbs->BeginPolygon_f(idx,pol->GetHeading(),bez ? 4 : 2,writer);
				DSF_AccumPolygonWithHolesBezier(*ci, NULL, bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}
		}
		else
		{
			Polygon_with_holes_2	pol_area;
			vector<Polygon_with_holes_2>	cuts;
			if(WED_PolygonWithHolesForPolygon(pol,pol_area))
			{
				ClipPolygon(pol_area, bounds,cuts);
				for(vector<Polygon_with_holes_2>::iterator ci = cuts.begin(); ci != cuts.end(); ++ci)			
				{
					cbs->BeginPolygon_f(idx,pol->GetHeading(),bez ? 4 : 2,writer);
					DSF_AccumPolygonWithHoles(*ci, NULL, bounds, cbs, writer);
					cbs->EndPolygon_f(writer);
				}
			}
			else
				printf("FUBAR\n");
		}
	}
	if((orth = dynamic_cast<WED_DrapedOrthophoto *>(what)) != NULL)
	{
		orth->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierPol(orth);

		UVMap_t	uv;
		WED_MakeUVMap(orth,uv);

		if(bez)
		{
			Bezier_polygon_with_holes_2	orth_area;
			vector<Bezier_polygon_with_holes_2> cuts;
			WED_BezierPolygonWithHolesForPolygon(orth, orth_area);
			ClipBezierPolygon(orth_area,bounds, cuts);
			for(vector<Bezier_polygon_with_holes_2>::iterator ci = cuts.begin(); ci != cuts.end(); ++ci)
			{
				cbs->BeginPolygon_f(idx,65535,bez ? 8 : 4,writer);
				DSF_AccumPolygonWithHolesBezier(*ci, &uv, bounds, cbs, writer);
				cbs->EndPolygon_f(writer);
			}
		}
		else
		{
			Polygon_with_holes_2	orth_area;
			vector<Polygon_with_holes_2>	cuts;
			if(WED_PolygonWithHolesForPolygon(orth,orth_area))
			{
				ClipPolygon(orth_area, bounds,cuts);
				for(vector<Polygon_with_holes_2>::iterator ci = cuts.begin(); ci != cuts.end(); ++ci)			
				{
					cbs->BeginPolygon_f(idx,65535,bez ? 8 : 4,writer);
					DSF_AccumPolygonWithHoles(*ci, &uv, bounds, cbs, writer);
					cbs->EndPolygon_f(writer);
				}
			}
			else
				printf("fubar.\n");
		}
	}

	int cc = what->CountChildren();
	for (int c = 0; c < cc; ++c)
		DSF_ExportTileRecursive(what->GetNthChild(c), pkg, bounds, io_resources, io_table, cbs, writer);
}

static void DSF_ExportTile(WED_Group * base, ILibrarian * pkg, int x, int y, set<string>& io_resources)
{
	void *			writer;
	DSFCallbacks_t	cbs;
	char	prop_buf[256];

	writer = DSFCreateWriter(x,y,x+1,y+1, -32768.0,32767.0,DSF_DIVISIONS);
	DSFGetWriterCallbacks(&cbs);

	sprintf(prop_buf, "%d", (int) x  );		cbs.AcceptProperty_f("sim/west", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) x+1);		cbs.AcceptProperty_f("sim/east", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) y+1);		cbs.AcceptProperty_f("sim/north", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) y  );		cbs.AcceptProperty_f("sim/south", prop_buf, writer);
	cbs.AcceptProperty_f("sim/planet", "earth", writer);
	cbs.AcceptProperty_f("sim/creation_agent", "WorldEditor" WED_VERSION_STRING, writer);
	cbs.AcceptProperty_f("laminar/internal_revision", "0", writer);
	cbs.AcceptProperty_f("sim/overlay", "1", writer);
	cbs.AcceptProperty_f("sim/require_object", "1/0", writer);
	cbs.AcceptProperty_f("sim/require_facade", "1/0", writer);

	DSF_ResourceTable	rsrc;

	Bbox2	clip_bounds(x,y,x+1,y+1);
	DSF_ExportTileRecursive(base, pkg, clip_bounds, io_resources, rsrc, &cbs, writer);

	for(vector<string>::iterator s = rsrc.obj_defs.begin(); s != rsrc.obj_defs.end(); ++s)
		cbs.AcceptObjectDef_f(s->c_str(), writer);

	for(vector<string>::iterator s = rsrc.polygon_defs.begin(); s != rsrc.polygon_defs.end(); ++s)
		cbs.AcceptPolygonDef_f(s->c_str(), writer);

	char	rel_dir [512];
	char	rel_path[512];
	string full_dir, full_path;
	sprintf(rel_dir ,"Earth nav data" DIR_STR "%+03d%+04d",							  latlon_bucket(y), latlon_bucket(x)	  );
	sprintf(rel_path,"Earth nav data" DIR_STR "%+03d%+04d" DIR_STR "%+03d%+04d.dsf",  latlon_bucket(y), latlon_bucket(x), y, x);
	full_path = rel_path;
	full_dir  = rel_dir ;
	pkg->LookupPath(full_dir );
	pkg->LookupPath(full_path);
	FILE_make_dir_exist(full_dir.c_str());
	DSFWriteToFile(full_path.c_str(), writer);
	DSFDestroyWriter(writer);
}

void DSF_Export(WED_Group * base, ILibrarian * package)
{
	g_dropped_pts = false;
	Bbox2	wrl_bounds;
	base->GetBounds(gis_Geo,wrl_bounds);
	int tile_west  = floor(wrl_bounds.p1.x());
	int tile_east  = ceil (wrl_bounds.p2.x());
	int tile_south = floor(wrl_bounds.p1.y());
	int tile_north = ceil (wrl_bounds.p2.y());

	set<string>	generated_resources;

	for (int y = tile_south; y < tile_north; ++y)
	for (int x = tile_west ; x < tile_east ; ++x)
	{
		DSF_ExportTile(base, package, x, y, generated_resources);
	}

	if(g_dropped_pts)
		DoUserAlert("Warning: you have bezier curves that cross a DSF tile boundary.  X-Plane 9 cannot handle this case.  To fix this, only use non-curved polygons to cross a tile boundary.");
}

int		WED_CanExportPack(IResolver * resolver)
{
	return 1;
}

void	WED_DoExportPack(IResolver * resolver)
{
	ILibrarian * l = WED_GetLibrarian(resolver);
	WED_Thing * w = WED_GetWorld(resolver);
	DSF_Export(dynamic_cast<WED_Group *>(w), l);

	string	apt = "Earth nav data" DIR_STR "apt.dat";
	l->LookupPath(apt);
	
	WED_AptExport(w, apt.c_str());
	
}

