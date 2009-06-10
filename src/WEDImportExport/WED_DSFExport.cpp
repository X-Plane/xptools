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
#include "DSFLib.h"
#include "FileUtils.h"
#include "WED_Entity.h"
#include "PlatformUtils.h"
#include "GISUtils.h"
#include "CompGeomDefs2.h"
#include "WED_Group.h"
#include "WED_Version.h"
//#include "WED_OverlayImage.h"
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

static bool g_dropped_pts = false;;

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
	if(b->GetControlHandleHi(d) || b->GetControlHandleLo(d))
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

static void DSF_AccumPtsCGAL(const Polygon_2& ring, const DSFCallbacks_t * cbs, void * writer, const Polygon_2 * tex_coord, const Bbox2& bounds)
{
	cbs->BeginPolygonWinding_f(writer);
	for (int n = 0; n < ring.size(); ++n)
	{
		double c[4] = { CGAL::to_double(ring[n].x()),
						CGAL::to_double(ring[n].y()),
						0,0};
		if(tex_coord)
		{
			c[2] = CGAL::to_double(tex_coord->vertex(n).x());
			c[3] = CGAL::to_double(tex_coord->vertex(n).y());
		}

		c[0] = doblim(c[0],bounds.xmin(),bounds.xmax());
		c[1] = doblim(c[1],bounds.ymin(),bounds.ymax());
		c[2] = doblim(c[2],0,1);
		c[3] = doblim(c[3],0,1);
		cbs->AddPolygonPoint_f(c,writer);
	}
	cbs->EndPolygonWinding_f(writer);	
}

class	DSF_LineClipper {
public:

	DSF_LineClipper(
			const DSFCallbacks_t *	icbs,
			void *					iwriter,
			const Bbox2&			iclip) : cbs(icbs), writer(iwriter), clip(iclip), alive(false), inited(false)
	{
	}
	
	~DSF_LineClipper()
	{
		if(alive)
			cbs->EndPolygonWinding_f(writer);			
	}
	
	void Accum(const Point2& c)
	{
		double coords[2];
	
		if(!inited)
		{
			last = c;
			inited=1;
			return;
		}
		
		Segment2	seg(last,c);
		
		bool in1 = clip.contains(seg.p1);
		bool in2 = clip.contains(seg.p2);
		
		if(in1 && in2)
		{
			if(!alive)
			{
				alive=true;
				cbs->BeginPolygonWinding_f(writer);
				coords[0] = last.x();
				coords[1] = last.y();
				DebugAssert(clip.contains(last));
				cbs->AddPolygonPoint_f(coords,writer);							
			}
			coords[0] = c.x(); coords[1] = c.y();
			DebugAssert(clip.contains(c));
			cbs->AddPolygonPoint_f(coords,writer);			
		}
		else if (in1)
		{
			if(!alive)
			{
				alive=true;
				cbs->BeginPolygonWinding_f(writer);
				coords[0] = last.x();
				coords[1] = last.y();
				DebugAssert(clip.contains(last));
				cbs->AddPolygonPoint_f(coords,writer);							
			}

			if (seg.clip_to(clip))
			{
				coords[0] = seg.p2.x(); coords[1] = seg.p2.y(); 
				DebugAssert(clip.contains(seg.p2));
				cbs->AddPolygonPoint_f(coords,writer);			
			}
			else
			{
				DebugAssert(!"Clipping error.");
			}
			cbs->EndPolygonWinding_f(writer);
			alive = false;			
		}
		else if (in2)
		{
			DebugAssert(!alive);
			alive = true;
			cbs->BeginPolygonWinding_f(writer);

			if (seg.clip_to(clip))
			{
				cbs->BeginPolygonWinding_f(writer);
				coords[0] = seg.p1.x(); coords[1] = seg.p1.y(); 
				DebugAssert(clip.contains(seg.p1));
				cbs->AddPolygonPoint_f(coords,writer);
				coords[0] = seg.p2.x(); coords[1] = seg.p2.y(); 
				DebugAssert(clip.contains(seg.p2));
				cbs->AddPolygonPoint_f(coords,writer);
			}
			else
			{
				DebugAssert(!"Clipping error.");
			}
		}
		else
		{
			DebugAssert(!alive);
			if (seg.clip_to(clip))
			{
				cbs->BeginPolygonWinding_f(writer);
				coords[0] = seg.p1.x(); coords[1] = seg.p1.y(); 
				DebugAssert(clip.contains(seg.p1));
				cbs->AddPolygonPoint_f(coords,writer);
				coords[0] = seg.p2.x(); coords[1] = seg.p2.y(); 
				DebugAssert(clip.contains(seg.p2));
				cbs->AddPolygonPoint_f(coords,writer);
				cbs->EndPolygonWinding_f(writer);
			}
		}
		
		last = c;
	}

private:
	const DSFCallbacks_t *	cbs;
	void *					writer;
	bool					alive;
	bool					inited;
	Bbox2					clip;
	Point2					last;
};	

static void assemble_dsf_pt(double c[8], const Point2& p, const Point2& st, const Point2& ch, const Point2& stch, bool bezier, bool tex_coord)
{	
	c[0] = p.x();
	c[1] = p.y();

	if(bezier)
	{
		c[2] = ch.x();
		c[3] = ch.y();
	}
	if(!bezier && tex_coord)
	{
		c[2] = st.x();
		c[3] = st.y();
	}
	if(bezier && tex_coord)
	{
		c[4] = st.x();
		c[5] = st.y();
		c[6] = stch.x();
		c[7] = stch.y();
	}
}	

static void DSF_AccumPts(IGISPointSequence * ring, const DSFCallbacks_t * cbs, void * writer, int bezier, int tex_coord, int double_end, const Bbox2& bounds)
{
	bool can_clip = !tex_coord && !bezier;
	DSF_LineClipper	clipper(cbs,writer,bounds);
	if(!can_clip)
		cbs->BeginPolygonWinding_f(writer);
	Point2	p,	cl,ch;
	Point2	st,	stcl,	stch;
	int np = ring->GetNumPoints();
	double c[8];
	for (int n = 0; n < (np + double_end); ++n)
	{
		IGISPoint * pt = ring->GetNthPoint(n % np);
		IGISPoint_Bezier * pth = bezier ? dynamic_cast<IGISPoint_Bezier *>(pt) : NULL;
		bool has_hi = false, has_lo = false;
		pt->GetLocation(p);
		if(tex_coord) pt->GetUV(st);
		ch = p;
		if(pth) has_hi = pth->GetControlHandleHi(ch);
		if(pth && tex_coord) pth->GetUVHi(stch);

		if(pth && pth->IsSplit())
		{
			has_lo = pth->GetControlHandleLo(cl);
			if(tex_coord) pth->GetUVLo(stcl);
			
			cl.x_ = 2.0 * p.x_ - cl.x_;
			cl.y_ = 2.0 * p.y_ - cl.y_;
			
			stcl.x_ = 2.0 * st.x_ - stcl.x_;
			stcl.y_ = 2.0 * st.y_ - stcl.y_;
			
			if(has_lo)
			{
				assemble_dsf_pt(c,p,st,cl,stcl, bezier, tex_coord);
				if(bounds.contains(Point2(c[0],c[1])))	cbs->AddPolygonPoint_f(c,writer);
				else									g_dropped_pts = true;
			}
			assemble_dsf_pt(c,p,st,p,st, bezier, tex_coord);
			if(bounds.contains(Point2(c[0],c[1])))	cbs->AddPolygonPoint_f(c,writer);
			else									g_dropped_pts = true;
			if(has_hi)
			{
				assemble_dsf_pt(c,p,st,ch,stch, bezier, tex_coord);
				if(bounds.contains(Point2(c[0],c[1])))	cbs->AddPolygonPoint_f(c,writer);
				else									g_dropped_pts = true;
			}
		} else {
			
			assemble_dsf_pt(c,p,st,ch,stch, bezier, tex_coord);

			if(can_clip)
				clipper.Accum(Point2(c[0],c[1]));
			else {
				if(bounds.contains(Point2(c[0],c[1])))		cbs->AddPolygonPoint_f(c,writer);
				else										g_dropped_pts = true;
			}
		}
	}
	if(!can_clip)
		cbs->EndPolygonWinding_f(writer);

}

static void DSF_AccumWindings(IGISPolygon * pol, const DSFCallbacks_t * cbs, void * writer, int bezier, int tex_coord, int holes, const Bbox2& bounds)
{
	if(bezier)
	{
		DSF_AccumPts(pol->GetOuterRing(), cbs,writer,bezier,tex_coord, 0, bounds);	// don't dupe first point
		int hc = holes ? pol->GetNumHoles() : 0;
		for(int h = 0; h < hc; ++h)
			DSF_AccumPts(pol->GetNthHole(h),cbs,writer,bezier,tex_coord, 0, bounds); // don't dupe first point
	} else {
		Polygon_2	outer_ring, uv_ring;
		if (!WED_PolygonForPointSequence(pol->GetOuterRing(), outer_ring, tex_coord ? &uv_ring : NULL))
			Assert(!"Found beziers in supposedly no-bez case");
		UVMap_t		uv_map;
		if(tex_coord)
			WED_MakeUVMap(outer_ring,uv_ring, uv_map);
		
		Polygon_set_2	ent_area;
		
		WED_PolygonSetForEntity(pol,ent_area);
		
		Polygon_2		clip;
		clip.push_back(Point_2(bounds.xmin(),bounds.ymin()));
		clip.push_back(Point_2(bounds.xmax(),bounds.ymin()));
		clip.push_back(Point_2(bounds.xmax(),bounds.ymax()));
		clip.push_back(Point_2(bounds.xmin(),bounds.ymax()));
		
		ent_area.intersection(clip);
		
		vector<Polygon_with_holes_2>	sub_entities;
		ent_area.polygons_with_holes(back_insert_iterator<vector<Polygon_with_holes_2> >(sub_entities));
		
		for(vector<Polygon_with_holes_2>::iterator sub_ent = sub_entities.begin(); sub_ent != sub_entities.end(); ++ sub_ent)
		{
			Polygon_with_holes_2	sub_ent_uv;
			if(tex_coord)
				if (!WED_MapPolygonWithHoles(uv_map, *sub_ent, sub_ent_uv))
					Assert(!"UV map failed.");
			
			DebugAssert(!sub_ent->is_unbounded());
			DSF_AccumPtsCGAL(sub_ent->outer_boundary(), cbs, writer, tex_coord ? &sub_ent_uv.outer_boundary() : NULL, bounds);
			
			if(holes)
			{
				if(tex_coord)
				{
					for(Polygon_with_holes_2::Hole_iterator h = sub_ent->holes_begin(); h != sub_ent->holes_end(); ++h)
						DSF_AccumPtsCGAL(*h, cbs, writer, NULL, bounds);
				}
				else
				{
					for(Polygon_with_holes_2::Hole_iterator h = sub_ent->holes_begin(), u = sub_ent_uv.holes_begin(); h != sub_ent->holes_end(); ++h, ++u)
						DSF_AccumPtsCGAL(*h, cbs, writer, &*u, bounds);
				}
			}
		}		
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
		e->GetBounds(ent_box);
	if(!ent_box.overlap(bounds))
		return;

	if((xcl = dynamic_cast<WED_ExclusionZone *>(what)) != NULL)
	{
		set<int> xtypes;
		xcl->GetExclusions(xtypes);
		Point2 minp, maxp;
		xcl->GetMin()->GetLocation(minp);
		xcl->GetMax()->GetLocation(maxp);
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
		obj->GetLocation(p);
		if(bounds.contains(p))
		{
			double xy[2] = { p.x(), p.y() };
			cbs->AddObject_f(idx, xy, obj->GetHeading(), writer);
		}
	}
	if((fac = dynamic_cast<WED_FacadePlacement *>(what)) != NULL)
	{
		fac->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierPol(fac);
		
		cbs->BeginPolygon_f(idx,fac->GetHeight(),bez ? 4 : 2,writer);
		DSF_AccumWindings(fac,cbs,writer,bez ? 1:0,0,0,bounds);	// no curves, no tex, no holes
		cbs->EndPolygon_f(writer);
	}
	if((fst = dynamic_cast<WED_ForestPlacement *>(what)) != NULL)
	{
		fst->GetResource(r);
		idx = io_table.accum_pol(r);
		cbs->BeginPolygon_f(idx,fst->GetDensity() * 255.0,2,writer);
		DSF_AccumWindings(fst,cbs,writer,0,0,1,bounds);	// no curves, no tex, yes holes
		cbs->EndPolygon_f(writer);
	}
	if((str = dynamic_cast<WED_StringPlacement *>(what)) != NULL)
	{
		str->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierSeq(str);
		cbs->BeginPolygon_f(idx,str->GetSpacing(),bez ? 4 : 2,writer);
		DSF_AccumPts(str,cbs,writer,bez,0, str->IsClosed(),bounds);	// yes curves, no tex holes, dupe end pt if closed
		cbs->EndPolygon_f(writer);
	}
	if((lin = dynamic_cast<WED_LinePlacement *>(what)) != NULL)
	{
		lin->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierSeq(lin);
		// Ben says: this is a huge freaking mess.  If a bezier is FULLY contained in the 
		
		bool no_close = bounds.overlap(ent_box) && !bounds.contains(ent_box);
		
		cbs->BeginPolygon_f(idx,!no_close && lin->IsClosed(),bez ? 4 : 2,writer);
		DSF_AccumPts(lin,cbs,writer,bez,0, no_close && lin->IsClosed(), bounds);	// yes curves, no tex holes, don't dupe end pt
		cbs->EndPolygon_f(writer);
	}
	if((pol = dynamic_cast<WED_PolygonPlacement *>(what)) != NULL)
	{
		pol->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierPol(pol);
		cbs->BeginPolygon_f(idx,pol->GetHeading(),bez ? 4 : 2,writer);
		DSF_AccumWindings(pol,cbs,writer,bez,0,1,bounds);	// yes curves, no tex, yes holes
		cbs->EndPolygon_f(writer);
	}
	if((orth = dynamic_cast<WED_DrapedOrthophoto *>(what)) != NULL)
	{
		orth->GetResource(r);
		idx = io_table.accum_pol(r);
		bool bez = DSF_HasBezierPol(orth);
		cbs->BeginPolygon_f(idx,65535,bez ? 8 : 4,writer);
		DSF_AccumWindings(orth,cbs,writer,bez,1,1,bounds);	// yes curves, no tex, yes holes
		cbs->EndPolygon_f(writer);
	}



/*	if ((img = dynamic_cast<WED_OverlayImage *>(what)) != NULL)
	{
		string img_file, pol_file;
		img->GetImage(img_file);
		pol_file = img_file;
		swap_suffix(pol_file,".pol");
		strip_path(img_file);

		int pol_idx = io_table.accum_pol(pol_file);

		IGISPointSequence * oring = img->GetOuterRing();

		cbs->BeginPolygon_f(pol_idx, 65535, 4, writer);
		cbs->BeginPolygonWinding_f(writer);

		for (int pn = oring->GetNumPoints()-1; pn >= 0; --pn)
		{
			WED_TextureNode * tn = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(pn));
			if (tn)
			{
				Point2 st, v;
				tn->GetTexCoord(st);
				tn->GetLocation(v);
				double c[4] = { v.x(), v.y(), st.x(), st.y() };
				cbs->AddPolygonPoint_f(c, writer);
			}
		}

		cbs->EndPolygonWinding_f(writer);
		cbs->EndPolygon_f(writer);

		if (io_resources.count(pol_file) == 0)
		{
			io_resources.insert(pol_file);
			string pol_path = pol_file;
			pkg->LookupPath(pol_path);
			if (!FILE_exists(pol_path.c_str()))
			{
				FILE * pfile = fopen(pol_path.c_str(), "w");
				if (pfile)
				{
					int is_apple = 1;
					#if IBM || LIN
						is_apple = 0;
					#endif
					fprintf(pfile, "%s" CRLF, is_apple ? "A" : "I");
					fprintf(pfile, "850" CRLF);
					fprintf(pfile, "DRAPED_POLYGON" CRLF CRLF);
					fprintf(pfile, "# Generated by WED " WED_VERSION_STRING CRLF);
					fprintf(pfile, "LAYER_GROUP airports -1" CRLF);
					fprintf(pfile, "TEXTURE_NOWRAP %s" CRLF, img_file.c_str());
					fprintf(pfile, "SCALE 25 25" CRLF);
					fclose(pfile);
				}
			}
		}
	}
*/

	int cc = what->CountChildren();
	for (int c = 0; c < cc; ++c)
		DSF_ExportTileRecursive(what->GetNthChild(c), pkg, bounds, io_resources, io_table, cbs, writer);
}

static void DSF_ExportTile(WED_Group * base, ILibrarian * pkg, int x, int y, set<string>& io_resources)
{
	void *			writer;
	DSFCallbacks_t	cbs;
	char	prop_buf[256];

	writer = DSFCreateWriter(x,y,x+1,y+1, 8);
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
	base->GetBounds(wrl_bounds);
	int tile_west  = floor(wrl_bounds.p1.x());
	int tile_east  = ceil (wrl_bounds.p2.x());
	int tile_south = floor(wrl_bounds.p1.y());
	int tile_north = ceil (wrl_bounds.p2.y());

	set<string>	generated_resources;

	for (int y = tile_south; y < tile_north; ++y)
	for (int x = tile_west ; x < tile_east ; ++x)
		DSF_ExportTile(base, package, x, y, generated_resources);
	if(g_dropped_pts)
		DoUserAlert("Warning: you have curved overlays that cross a DSF tile - they may not have exported correctly.  Do not use overlay elements that are curved and cross a DSF tile boundary.");
}

int		WED_CanExportDSF(IResolver * resolver)
{
	return 1;
}

void	WED_DoExportDSF(IResolver * resolver)
{
	ILibrarian * l = WED_GetLibrarian(resolver);
	WED_Thing * w = WED_GetWorld(resolver);
	DSF_Export(dynamic_cast<WED_Group *>(w), l);
}

