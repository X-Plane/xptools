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

bool WED_MakeFacadePreview(fac_info_t * info, const string& wall_tex, const string &roof_tex)
{

	// sanitize list of walls
#if 1
	if (!info.is_new) 
	{  // type 1 stuff



		return true;
	}
	else
	{  //type 2 stuff
		return true;
	}
#else
	// populate all wall sections, if .fac does not set them excplicitly
	if (walls.empty())
		return true;
	
	int want_floors = 1;      // # floors in the way the Obj8 is constructed. Not the floors of the building/facade represents.
	if (has_middles) 
		want_floors = 2;
	else
	{
		info.min_floors = -1.0;
		info.max_floors =  (walls[0].vert[2]-walls[0].vert[0]) * info.scale_y;
	}
	
	// fills a XObj8-structure for library preview
	if (!info.is_new)         // can't handle type 2 facades, yet
	{
		XObj8 *obj = new XObj8;
	//	obj->indices.clear();
		
		XObjCmd8 cmd;
		
		obj->texture = wall_tex;

		int quads;          // total number of quads for each floor
		
		float pt[8];
		pt[3] = 0.0;    	// normal vector is a don't care, so have it point straight up
		pt[4] = 1.0;
		pt[5] = 0.0;

		pt[1] = 0.0;        // initial height coordinate - todo: basements
		for (int level=0; level<want_floors; ++level)
		{
			quads=0;
			for (int fl = 0; fl < 2; ++fl)
			{
				pt[0] = 0.0;        // left front corner
				pt[2] = 0.0;

				if (fl) pt[1] += (walls[0].vert[level+2]-walls[0].vert[level])*info.scale_y;   // height of each floor

				for (int i=0; i<2; ++i)
					for (int j=0; j<2; ++j)
					{	
						if (!info.ring && i && j) break;      // open facades (fences etc) drawn with 3 sides only
						
						// wall selection. We want to show off as many different walls as practical
						int w = 2*i+j;
						if (w >= walls.size()) w = 0;   // show default wall if we run out of varieties
						
						pt[7] = walls[w].vert[2*fl+level];
						
						float len_left = want_len - (walls[w].hori[2]-walls[w].hori[0] + walls[w].hori[3]-walls[w].hori[1]) * info.scale_x;
						float exact = 2.0 + len_left / ((walls[w].hori[2]-walls[w].hori[1]) * info.scale_x);
						int sects = ceil(exact);

// printf("want %5.1f left %5.1f sects %d left %5.1f\n",want_len, len_left, sects, exact-sects);
						
						for (int k=0; k<sects; k++)
						{
							float s1 = walls[w].hori[min(k,1)];
							float s2 = walls[w].hori[2+(k==sects-1)];
							float dx = (s2-s1)*info.scale_x;
							
							// "VT "
							pt[6] = s1;
							obj->geo_tri.append(pt);
							pt[2-2*j] += (1-2*i) * dx * (1.0-(k==sects-2)*(sects-exact));
							pt[6] = s2 - (s2-s1)*(k==sects-2)*(sects-exact);
							obj->geo_tri.append(pt);
							if (fl) quads++;
						}
					}
			}
			int seq[6] = {0, 1, 2*quads, 1, 2*quads+1, 2*quads};
			for (int i = 0; i < 6*quads; ++i)
				obj->indices.push_back(2*(i/6)+seq[i%6]);
			
		}
		
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
		
		if (info.roof)
		{ 
			XObj8 *r_obj = new XObj8;
			r_obj->texture = roof_tex;
			
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
	}
	else
		return false; // todo: deal with type 2 facades here
}
#endif
