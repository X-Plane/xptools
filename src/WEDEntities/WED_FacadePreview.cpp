/*
 * Copyright (c) 2017, Laminar Research.
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

#include "XObjReadWrite.h"
#include "ObjConvert.h"
#include "CompGeomDefs2.h"
#include "MathUtils.h"
#include "XESConstants.h"

#include "WED_ResourceMgr.h"
#include "WED_FacadePreview.h"

//void print_wall(const REN_facade_wall_t& w)
//{
//	printf("%f %f %f %f\n", w.min_width,w.max_width, w.min_heading,w.max_heading);
//	for(vector<UTL_spelling_t>::const_iterator s = w.spellings.begin(); s != w.spellings.end(); ++s)
//	{
//		printf("%zd ",distance(w.spellings.begin(),s));
//		for(vector<xbyt>::const_iterator b = s->indices.begin(); b != s->indices.end(); ++b)
//			printf(" %d", *b);
//		printf("\n");
//	}	
//}
inline bool in_heading_in_range(xflt h, xflt h_min, xflt h_max)
{
	if (h_min == h_max) return true;
	
	h = fltwrap(h,0,360);
	h_min = fltwrap(h_min,0,360);
	h_max = fltwrap(h_max,0,360);
	
	if (h_min < h_max) return (h_min <= h && h < h_max);
					   return (h_min <= h || h < h_max);
}

bool	REN_facade_wall_filter_t::is_ok(xflt len, xflt rel_hdg) const
{
	return fltrange(len,min_width, max_width) && in_heading_in_range(rel_hdg, min_heading, max_heading);		
}

bool	REN_facade_wall_filters_t::is_ok(xflt len, xflt rel_hdg) const
{
	for(vector<REN_facade_wall_filter_t>::const_iterator i = filters.begin(); i != filters.end(); ++i)
		if(i->is_ok(len,rel_hdg))
			return true;
	return false;
}

FacadeWall_t::FacadeWall_t() :
	x_scale(0.0),y_scale(0.0),
	roof_slope(0.0),
	left(0), center(0), right(0),	
	bottom(0), middle(0), top(0),
	basement(0.0)	
{}

bool WED_MakeFacadePreview(fac_info_t& info, double fac_height, double fac_width)
{
	printf("New preview for h/w %.1lf %.1lf\n", fac_height, fac_width);
	// sanitize wall choices ?
	if (info.walls.empty())	return false;
	
	// fills a XObj8-structure for library preview
	if (!info.is_new)         // can't handle type 2 facades, yet
	{
//		for(auto p : info.previews)
//			delete(p);
		XObj8 *obj;
		if (info.previews.size())
		{
			obj = info.previews[0];
			obj->indices.clear();
			obj->geo_tri.clear(8);
			obj->lods.clear();
		}
		else
			obj = new XObj8;
			
		XObjCmd8 cmd;
		
		obj->texture = info.wall_tex;

		int quads = 0;    // total number of quads for each floor
		
		float pt[8] = { 0.0 }; // 0,1,2 = x,z,y   3,4,5 = normals  6,7 = s,t
		pt[4] = 1.0;    	// normal vector is a don't care, so have it point straight up
		
		int num_walls = info.is_ring ? 4 : 3;
		
		for (int w = 0; w < num_walls; ++w)
			{	
				Vector2 dir(-cos(w*90.0*DEG_TO_RAD),sin(w*90.0*DEG_TO_RAD));
				
				const FacadeWall_t * thisWall = &info.walls[intmin2(info.walls.size()-1,w)];
				
				float lPan_totW = 0.0;
				int lPanEnd = -1;
				do
				{
					lPanEnd++;
					lPan_totW += (thisWall->s_panels[lPanEnd].second - thisWall->s_panels[lPanEnd].first) * thisWall->x_scale;
					if (lPan_totW > fac_width / 2.0) break;
				}
				while(lPanEnd < thisWall->left + thisWall->center -1);

				float rPan_totW = 0.0;
				int rPanStart = thisWall->s_panels.size();
				do
				{
					rPanStart--;
					rPan_totW += (thisWall->s_panels[rPanStart].second - thisWall->s_panels[rPanStart].first) * thisWall->x_scale;
					if (rPan_totW > fac_width - lPan_totW) break;
				}
				while(rPanStart > thisWall->left);

				float cPan_totW = (thisWall->s_panels[thisWall->left + thisWall->center -1].second - thisWall->s_panels[thisWall->left].first) * thisWall->x_scale;


												
				float total_hgt = (thisWall->t_floors.back().second - thisWall->t_floors.front().first) * thisWall->y_scale;
				int bLevEnd = thisWall->bottom + thisWall->middle -1;
				int tLevStart = thisWall->top;
				
// printf("want %5.1f left %5.1f sects %d left %5.1f\n",want_len, len_left, sects, exact-sects);

/*  Point locations 1 - 2
						  |   |
						  0 - 3    */
				{
					pt[6] = thisWall->s_panels[0].first; pt[7] = thisWall->t_floors[0].first;
					obj->geo_tri.append(pt);
					pt[1] += total_hgt;
					pt[7] = thisWall->t_floors[bLevEnd].second;
					obj->geo_tri.append(pt);
					pt[0] += dir.x() * lPan_totW; pt[2] += dir.y() * lPan_totW;
					pt[6] = thisWall->s_panels[lPanEnd].second;
					obj->geo_tri.append(pt);
					pt[1] -= total_hgt;
					pt[7] = thisWall->t_floors[0].first;
					obj->geo_tri.append(pt);
					quads++;
				}
				if(fac_width - lPan_totW - rPan_totW > cPan_totW)
				{
					pt[6] = thisWall->s_panels[rPanStart].first;
					obj->geo_tri.append(pt);
					pt[1] += total_hgt;
					pt[7] = thisWall->t_floors[bLevEnd].second;
					obj->geo_tri.append(pt);
					pt[0] += dir.x() * cPan_totW; pt[2] += dir.y() * cPan_totW;
					pt[6] = thisWall->s_panels[lPanEnd].second;
					obj->geo_tri.append(pt);
					pt[1] -= total_hgt;
					pt[7] = thisWall->t_floors[0].first;
					obj->geo_tri.append(pt);
					quads++;
				}
				//if(fac_width > lPan_totW + rPan_totW)
				{
					pt[6] = thisWall->s_panels[rPanStart].first; pt[7] = thisWall->t_floors[0].first;
					obj->geo_tri.append(pt);
					pt[1] += total_hgt;
					pt[7] = thisWall->t_floors[bLevEnd].second;
					obj->geo_tri.append(pt);
					pt[0] += dir.x() * rPan_totW; pt[2] += dir.y() * rPan_totW;
					pt[6] = thisWall->s_panels.back().second;
					obj->geo_tri.append(pt);
					pt[1] -= total_hgt;
					pt[7] = thisWall->t_floors[0].first;
					obj->geo_tri.append(pt);
					quads++;
				}
				
			}
			
		int seq[6] = {0, 1, 2, 0, 2, 3};
		for (int i = 0; i < 6*quads; ++i)
			obj->indices.push_back(4*(i/6)+seq[i%6]);
	
		obj->geo_tri.get_minmax(obj->xyz_min,obj->xyz_max);
	
		obj->lods.push_back(XObjLOD8());
		obj->lods.back().lod_near = 0;
		obj->lods.back().lod_far  = 1000;

		cmd.cmd = attr_NoCull;
		obj->lods.back().cmds.push_back(cmd);

		cmd.cmd = obj8_Tris;
		cmd.idx_offset = 0;
		cmd.idx_count  = obj->indices.size();
		obj->lods.back().cmds.push_back(cmd);

		info.previews.push_back(obj);
		
#if 0
		if (info.has_roof)
		{ 
			XObj8 *r_obj = new XObj8;
			r_obj->texture = info.roof_tex;
			
			quads=1;
			pt[1] = obj->xyz_max[1]; // height
			
			for (int i = 0; i<2; ++i)
				for (int j = 0; j<2; ++j)
				{
					pt[0] = i ? obj->xyz_min[0] : obj->xyz_max[0];
					pt[2] = j ? obj->xyz_min[2] : obj->xyz_max[2];
					
					pt[6] = roof_uv[2*i];
					pt[7] = roof_uv[1+2*j];
//	  printf("%s roof_uv = %8.3f %8.3f\n",wall_tex.c_str()+40,pt[6],pt[7]);

					r_obj->geo_tri.append(pt);
				}
				
			r_obj->geo_tri.get_minmax(r_obj->xyz_min,r_obj->xyz_max);
			
			// "IDX "
			int seq[6] = {0, 1, 2*quads, 1, 2*quads+1, 2*quads};
			for (int i = 0; i < 6*quads; ++i)
				r_obj->indices.push_back(2*(i/6)+seq[i%6]);

			// "ATTR_LOD"
			r_obj->lods.push_back(XObjLOD8());
			r_obj->lods.back().lod_near = 0;
			r_obj->lods.back().lod_far  = 1000;

			// "TRIS ";
			cmd.cmd = obj8_Tris;
			cmd.idx_offset = 0;
			cmd.idx_count  = r_obj->indices.size();
			r_obj->lods.back().cmds.push_back(cmd);

			info.previews.push_back(r_obj);
		}
		return true;
#endif
	}
	else
		return false; // todo: deal with type 2 facades here
}
