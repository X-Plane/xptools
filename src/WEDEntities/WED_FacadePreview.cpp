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

#include "XObjReadWrite.h"
#include "ObjConvert.h"
#include "CompGeomDefs2.h"
#include "MathUtils.h"

#include "WED_FacadePreview.h"

bool WED_MakeFacadePreview(fac_info_t& info, vector<wall_map_t> wall, 
	string wall_tex, float tex_size[2],	string roof_tex,  float roof_scale[2])
{
	int floors = 1;  // # floors in the way the Obj8 is constructed. Not the floor of the building/facade represents.

	// normalize and sanitize all texture coordinates
	for (vector<wall_map_t>::iterator i = wall.begin(); i != wall.end(); ++i)
	{
		for (int j=0; j<4; ++j)
		{
			i->vert[j]/=tex_size[1];
			i->hori[j]/=tex_size[0];
		}
		if (i->vert[1] < 0.001)
//			wall.erase(i);
			printf("Wall no bottom\n");
		
		if (i->vert[2] < 0.001) i->vert[2] = i->vert[1];
		else floors = 2;
		if (i->vert[3] < 0.001) i->vert[3] = i->vert[2];
		else floors = 2;
		
		if (i->hori[1] == i->hori[2])  // left but no center section defined. Happens for walls intended to be very short only
			i->hori[1] = i->hori[0];   // use left segment also for centers. Its not what XP does, but yeah ...

		fflush(stdout);
	}
	
	// populate all wall sections, if .fac does not set them excplicitly
	if (wall.empty())
		return true;
	
	// fills a XObj8-structure for library preview
	if (info.version < 900)             // can't handle type 2 facades, yet
	{
		XObj8 *obj = new XObj8;
	//	obj->indices.clear();
		
		XObjCmd8 cmd;
		
		obj->texture = wall_tex;

		int quads = 0;       // total number of quads generated
		int quads_fl;        // total number of quads for each floor
		
		float pt[8];
		pt[3] = 0.0;    	// normal vector is don't care, so have it point up;
		pt[4] = 1.0;
		pt[5] = 0.0;

		pt[1] = 0.0;        // height coordinate
		for (int level=0; level<floors; ++level)
		{
			quads_fl=0;
			for (int fl = 0; fl < 2; ++fl)
			{
				pt[0] = 0.0;        // left front corner
				pt[2] = 0.0;

				if (fl) pt[1] += (wall[0].vert[2+level]-wall[0].vert[level])*wall[0].scale_y;   // height of each floor

				for (int i=0; i<2; ++i)
					for (int j=0; j<2; ++j)
					{	
						if (!info.ring && i && j) break;      // open facades (fences etc) drawn with 3 sides only
						
						// wall selection. We want to show off as many different walls as practical
						int w = 2*i+j;
						if (w >= wall.size()) w = 0;   // show default wall if we run out of varieties
						
						pt[7] = wall[w].vert[2*fl+level];
						
						int sects = 2;    // how many sections do we need to drawfor this wall
						float total_wall_len = (wall[w].hori[3]-wall[w].hori[0])*wall[0].scale_x;
						
						if ( total_wall_len < 10.0 ) sects = 1+10.0/total_wall_len;  // make really short walls bit longer
						
						for (int k=0; k<sects; k++)
						{
							float s1 = wall[w].hori[min(k,1)];
							float s2 = wall[w].hori[2+(k==sects-1)];
							float dx = (s2-s1)*wall[0].scale_x;
							
							// "VT "
							pt[6] = s1;
							obj->geo_tri.append(pt);
							pt[2-2*j] += (1-2*i) * dx;
							pt[6] = s2;
							obj->geo_tri.append(pt);
							if (fl) quads_fl++;
						}
					}
			}
			quads += quads_fl;
		}
		// set dimension
		obj->geo_tri.get_minmax(obj->xyz_min,obj->xyz_max);

		// "IDX "
		int seq[6] = {0, 1, 2*quads_fl, 1, 2*quads_fl+1, 2*quads_fl};
		for (int i = 0; i < 6*quads; ++i)
			obj->indices.push_back(2*(i/6)+seq[i%6]);

		// "ATTR_LOD"
		obj->lods.push_back(XObjLOD8());
		obj->lods.back().lod_near = 0;
		obj->lods.back().lod_far  = 1000;

		// "ATTR_no_cull"
		cmd.cmd = attr_NoCull;
		obj->lods.back().cmds.push_back(cmd);

		// "TRIS ";
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = 0;
		cmd.idx_count  = 6*quads;
		obj->lods.back().cmds.push_back(cmd);

		info.preview = obj;
		// only problem is that the texture path contain spaces -> obj reader can not read that
		// but still valuable for checking the values/structure
	}
//		XObj8Write(mLibrary->CreateLocalResourcePath("forest_preview.obj").c_str(), *obj);
}

