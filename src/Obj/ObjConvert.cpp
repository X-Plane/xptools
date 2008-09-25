/*
 * Copyright (c) 2005, Laminar Research.
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
#include "ObjConvert.h"
#include "XObjDefs.h"
#include <math.h>
#include "PVRTGeometry.h"
#include "PVRTTriStrip.h"

static bool operator==(const vec_tex& lhs, const vec_tex& rhs);
bool operator==(const vec_tex& lhs, const vec_tex& rhs)
{
	return
		lhs.v[0] == rhs.v[0] &&
		lhs.v[1] == rhs.v[1] &&
		lhs.v[2] == rhs.v[2] &&
		lhs.st[0] == rhs.st[0] &&
		lhs.st[1] == rhs.st[1];
}

const int	TRI_HARD =	attr_Max  ;
const int	TRI_COCK =	attr_Max+1;

static void	special_tris_to_quads(XObj& obj)
{
	for (vector<XObjCmd>::iterator cmd = obj.cmds.begin(); cmd != obj.cmds.end(); ++cmd)
	{
		vector<XObjCmd>::iterator	cmd2 = cmd;
		++cmd2;

		if (cmd2 != obj.cmds.end())
		{
			if (cmd->cmdID == TRI_HARD && cmd2->cmdID ==TRI_HARD)
			{
				     if (cmd->st[0] == cmd2->st[1] && cmd->st[1] == cmd2->st[0]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+1,cmd2->st[2]); }
				else if (cmd->st[0] == cmd2->st[2] && cmd->st[1] == cmd2->st[1]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+1,cmd2->st[0]); }
				else if (cmd->st[0] == cmd2->st[0] && cmd->st[1] == cmd2->st[2]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+1,cmd2->st[1]); }

				else if (cmd->st[1] == cmd2->st[1] && cmd->st[2] == cmd2->st[0]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+2,cmd2->st[2]); }
				else if (cmd->st[1] == cmd2->st[2] && cmd->st[2] == cmd2->st[1]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+2,cmd2->st[0]); }
				else if (cmd->st[1] == cmd2->st[0] && cmd->st[2] == cmd2->st[2]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+2,cmd2->st[1]); }

				else if (cmd->st[2] == cmd2->st[1] && cmd->st[0] == cmd2->st[0]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+3,cmd2->st[2]); }
				else if (cmd->st[2] == cmd2->st[2] && cmd->st[0] == cmd2->st[1]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+3,cmd2->st[0]); }
				else if (cmd->st[2] == cmd2->st[0] && cmd->st[0] == cmd2->st[2]) { cmd->cmdID = obj_Quad_Hard; cmd->st.insert(cmd->st.begin()+3,cmd2->st[1]); }
				else cmd->cmdID = obj_Tri;

				if (cmd->cmdID != cmd2->cmdID)
				{
					cmd = obj.cmds.erase(cmd2);
					--cmd;
				}
			}

			else if (cmd->cmdID == TRI_COCK && cmd2->cmdID ==TRI_COCK)
			{
				     if (cmd->st[0] == cmd2->st[1] && cmd->st[1] == cmd2->st[0]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+1,cmd2->st[2]); }
				else if (cmd->st[0] == cmd2->st[2] && cmd->st[1] == cmd2->st[1]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+1,cmd2->st[0]); }
				else if (cmd->st[0] == cmd2->st[0] && cmd->st[1] == cmd2->st[2]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+1,cmd2->st[1]); }

				else if (cmd->st[1] == cmd2->st[1] && cmd->st[2] == cmd2->st[0]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+2,cmd2->st[2]); }
				else if (cmd->st[1] == cmd2->st[2] && cmd->st[2] == cmd2->st[1]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+2,cmd2->st[0]); }
				else if (cmd->st[1] == cmd2->st[0] && cmd->st[2] == cmd2->st[2]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+2,cmd2->st[1]); }

				else if (cmd->st[2] == cmd2->st[1] && cmd->st[0] == cmd2->st[0]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+3,cmd2->st[2]); }
				else if (cmd->st[2] == cmd2->st[2] && cmd->st[0] == cmd2->st[1]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+3,cmd2->st[0]); }
				else if (cmd->st[2] == cmd2->st[0] && cmd->st[0] == cmd2->st[2]) { cmd->cmdID = obj_Quad_Cockpit; cmd->st.insert(cmd->st.begin()+3,cmd2->st[1]); }
				else cmd->cmdID = obj_Tri;

				if (cmd->cmdID != cmd2->cmdID)
				{
					cmd = obj.cmds.erase(cmd2);
					--cmd;
				}

			}
		} else {
			if (cmd->cmdID == TRI_HARD || cmd->cmdID == TRI_COCK) cmd->cmdID = obj_Tri;
		}
	}
}

static int append_rgb(ObjPointPool * pool, const vec_rgb& rgb);
static int append_rgb(ObjPointPool * pool, const vec_rgb& rgb)
{
	float	dat[6] = { rgb.v[0], rgb.v[1], rgb.v[2], rgb.rgb[0], rgb.rgb[1], rgb.rgb[2] };
	return pool->append(dat);
}

static int append_st(ObjPointPool * pool, const vec_tex& st);
static int append_st(ObjPointPool * pool, const vec_tex& st)
{
	float	dat[8] = { st.v[0], st.v[1], st.v[2], 0.0, 0.0, 0.0, st.st[0], st.st[1] };
	return pool->append(dat);
}

void	Obj7ToObj8(const XObj& obj7, XObj8& obj8)
{
	obj8.texture = obj7.texture + ".png";
	obj8.texture_lit = obj7.texture + "_LIT.png";
	obj8.indices.clear();
	obj8.geo_tri.clear(8);
	obj8.geo_lines.clear(6);
	obj8.geo_lights.clear(6);
	obj8.animation.clear();
	obj8.lods.resize(1);
	obj8.lods.back().lod_near = obj8.lods.back().lod_far = 0.0;
	obj8.lods.back().cmds.clear();

	XObjCmd8	cmd8;
	int 		n;
	int			idx_base;

	bool		is_hard = false;
	bool		is_cock = false;
	bool		now_hard, now_cock;

	vector<XObjCmd>::const_iterator next;
	for (vector<XObjCmd>::const_iterator cmd = obj7.cmds.begin(); cmd != obj7.cmds.end(); ++cmd)
	{
		switch(cmd->cmdID) {
		case attr_LOD:
			if (obj8.lods.back().lod_far != 0.0)
				obj8.lods.push_back(XObjLOD8());
			obj8.lods.back().lod_near = cmd->attributes[0];
			obj8.lods.back().lod_far = cmd->attributes[1];
			is_hard = false;
			is_cock = false;
			break;
		case obj_Light:
			cmd8.cmd = obj8_Lights;
			cmd8.idx_offset = obj8.geo_lights.count();
			cmd8.idx_count = cmd->rgb.size();
			for (n = 0; n < cmd->rgb.size(); ++n)
				append_rgb(&obj8.geo_lights, cmd->rgb[n]);
			obj8.lods.back().cmds.push_back(cmd8);
			break;
		case obj_Line:
			cmd8.cmd = obj8_Lines;
			cmd8.idx_offset = obj8.indices.size();
			cmd8.idx_count = cmd->rgb.size();
			for (n = 0; n < cmd->rgb.size(); ++n)
				obj8.indices.push_back(append_rgb(&obj8.geo_lines, cmd->rgb[n]));
			obj8.lods.back().cmds.push_back(cmd8);
			break;
		case obj_Tri:
			cmd8.cmd = obj8_Tris;
			cmd8.idx_offset = obj8.indices.size();
			cmd8.idx_count = cmd->st.size();
			idx_base = obj8.geo_tri.count();
			for (n = 0; n < cmd->st.size(); ++n)
				append_st(&obj8.geo_tri, cmd->st[n]);
			for (n = 0; n < cmd->st.size(); ++n)
				obj8.indices.push_back(idx_base+n);
			obj8.lods.back().cmds.push_back(cmd8);
			break;
		case obj_Quad:
		case obj_Movie:
		case obj_Quad_Hard:
		case obj_Quad_Cockpit:
			// TODO: movies?
			now_hard = cmd->cmdID == obj_Quad_Hard;
			now_cock = cmd->cmdID == obj_Quad_Cockpit;

			if (now_hard != is_hard)
			{
				is_hard = now_hard;
				cmd8.cmd = (now_hard) ? attr_Hard : attr_No_Hard;
				obj8.lods.back().cmds.push_back(cmd8);
			}

			if (now_cock != is_cock)
			{
				is_cock = now_cock;
				cmd8.cmd = is_cock ? attr_Tex_Cockpit : attr_Tex_Normal;
				obj8.lods.back().cmds.push_back(cmd8);
			}

			cmd8.cmd = obj8_Tris;
			cmd8.idx_offset = obj8.indices.size();
			cmd8.idx_count = cmd->st.size() * 3 / 2;
			idx_base = obj8.geo_tri.count();
			for (n = 0; n < cmd->st.size(); ++n)
				append_st(&obj8.geo_tri, cmd->st[n]);
			for (n = 0; n < cmd->st.size(); n += 4)
			{
				obj8.indices.push_back(idx_base+n+0);
				obj8.indices.push_back(idx_base+n+1);
				obj8.indices.push_back(idx_base+n+2);
				obj8.indices.push_back(idx_base+n+0);
				obj8.indices.push_back(idx_base+n+2);
				obj8.indices.push_back(idx_base+n+3);
			}
			obj8.lods.back().cmds.push_back(cmd8);

			next = cmd;
			++next;
			if (next != obj7.cmds.end() && next->cmdID != obj_Quad && next->cmdID != obj_Movie && next->cmdID != obj_Quad_Cockpit && next->cmdID != obj_Quad_Hard)
			{
				now_hard = false;
				now_cock = false;

				if (now_hard != is_hard)
				{
					is_hard = now_hard;
					cmd8.cmd = (now_hard) ? attr_Hard : attr_No_Hard;
					obj8.lods.back().cmds.push_back(cmd8);
				}

				if (now_cock != is_cock)
				{
					is_cock = now_cock;
					cmd8.cmd = is_cock ? attr_Tex_Cockpit : attr_Tex_Normal;
					obj8.lods.back().cmds.push_back(cmd8);
				}
			}

			break;
		case obj_Polygon:
		case obj_Tri_Fan:
			cmd8.cmd = obj8_Tris;
			cmd8.idx_offset = obj8.indices.size();
			cmd8.idx_count = (cmd->st.size()-2)*3;
			idx_base = obj8.geo_tri.count();
			for (n = 0; n < cmd->st.size(); ++n)
				append_st(&obj8.geo_tri, cmd->st[n]);
			for (n = 2; n < cmd->st.size(); ++n)
			{
				obj8.indices.push_back(idx_base);
				obj8.indices.push_back(idx_base+n-1);
				obj8.indices.push_back(idx_base+n-0);
			}
			obj8.lods.back().cmds.push_back(cmd8);
			break;
		case obj_Quad_Strip:
			cmd8.cmd = obj8_Tris;
			cmd8.idx_offset = obj8.indices.size();
			cmd8.idx_count = (cmd->st.size()-2)*3;
			idx_base = obj8.geo_tri.count();
			for (n = 0; n < cmd->st.size(); ++n)
				append_st(&obj8.geo_tri, cmd->st[n]);
			for (n = 2; n < cmd->st.size(); n += 2)
			{
				obj8.indices.push_back(idx_base+n-2);
				obj8.indices.push_back(idx_base+n-1);
				obj8.indices.push_back(idx_base+n+1);
				obj8.indices.push_back(idx_base+n-2);
				obj8.indices.push_back(idx_base+n+1);
				obj8.indices.push_back(idx_base+n+0);
			}
			obj8.lods.back().cmds.push_back(cmd8);
			break;
		case obj_Tri_Strip:
			cmd8.cmd = obj8_Tris;
			cmd8.idx_offset = obj8.indices.size();
			cmd8.idx_count = (cmd->st.size()-2)*3;
			idx_base = obj8.geo_tri.count();
			for (n = 0; n < cmd->st.size(); ++n)
				append_st(&obj8.geo_tri, cmd->st[n]);
			for (n = 2; n < cmd->st.size(); ++n)
			{
				if (n % 2)
				{
					obj8.indices.push_back(idx_base+n-2);
					obj8.indices.push_back(idx_base+n-0);
					obj8.indices.push_back(idx_base+n-1);
				} else {
					obj8.indices.push_back(idx_base+n-2);
					obj8.indices.push_back(idx_base+n-1);
					obj8.indices.push_back(idx_base+n-0);
				}
			}
			obj8.lods.back().cmds.push_back(cmd8);
			break;

		case attr_Shade_Flat:
		case attr_Shade_Smooth:
		case attr_Ambient_RGB:
		case attr_Diffuse_RGB:
		case attr_Emission_RGB:
		case attr_Specular_RGB:
		case attr_Shiny_Rat:
		case attr_No_Depth:
		case attr_Depth:
		case attr_Reset:
		case attr_Cull:
		case attr_NoCull:
		case attr_Offset:
		case obj_Smoke_Black:
		case obj_Smoke_White:

			cmd8.cmd = cmd->cmdID;
			for (n = 0; n < cmd->attributes.size(); ++n)
				cmd8.params[n] = cmd->attributes[n];
			obj8.lods.back().cmds.push_back(cmd8);
			break;
		}
	}

	for (n = 0; n < obj8.geo_lines.count(); ++n)
	{
		float * dat = obj8.geo_lines.get(n);
		dat[3] *= 0.1;
		dat[4] *= 0.1;
		dat[5] *= 0.1;
	}

	for (n = 0; n < obj8.geo_lights.count(); ++n)
	{
		float * dat = obj8.geo_lights.get(n);
		dat[3] *= 0.1;
		dat[4] *= 0.1;
		dat[5] *= 0.1;
	}
	Obj8_ConsolidateIndexCommands(obj8);
	Obj8_CalcNormals(obj8);
}

void	Obj8_ConsolidateIndexCommands(XObj8& obj8)
{
	for (vector<XObjLOD8>::iterator lod = obj8.lods.begin(); lod != obj8.lods.end(); ++lod)
	{
		vector<XObjCmd8>::iterator cmd = lod->cmds.begin(), next;
		while (cmd != lod->cmds.end())
		{
			next = cmd;
			++next;
			if (next != lod->cmds.end())
			{
				if (next->cmd == cmd->cmd &&
					(cmd->cmd == obj8_Tris || cmd->cmd == obj8_Lines || cmd->cmd == obj8_Lights) &&
					((cmd->idx_offset + cmd->idx_count) == next->idx_offset))
				{
					cmd->idx_count += next->idx_count;
					cmd = lod->cmds.erase(next);
					--cmd;
				} else
					++cmd;
			} else
				++cmd;
		}
	}
}

void	Obj8_CalcNormals(XObj8& obj8)
{
	for (vector<XObjLOD8>::iterator lod = obj8.lods.begin(); lod != obj8.lods.end(); ++lod)
	for (vector<XObjCmd8>::iterator  cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
	if (cmd->cmd == obj8_Tris)
	{
		for (int o = 0; o < cmd->idx_count; o += 3)
		{
			int i1 = obj8.indices[cmd->idx_offset + o + 0];
			int i2 = obj8.indices[cmd->idx_offset + o + 1];
			int i3 = obj8.indices[cmd->idx_offset + o + 2];

			float p1[8], p2[8], p3[8];
			memcpy(p1, obj8.geo_tri.get(i1), sizeof (p1));
			memcpy(p2, obj8.geo_tri.get(i2), sizeof (p2));
			memcpy(p3, obj8.geo_tri.get(i3), sizeof (p3));

			float n[3], a[3], b[3];
			a[0] = p3[0] - p1[0];		b[0] = p2[0] - p1[0];
			a[1] = p3[1] - p1[1];		b[1] = p2[1] - p1[1];
			a[2] = p3[2] - p1[2];		b[2] = p2[2] - p1[2];

			n[0]= a[1]*b[2]-b[1]*a[2];
			n[1]=-a[0]*b[2]+b[0]*a[2];
			n[2]= a[0]*b[1]-b[0]*a[1];

			float len=sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);

			if(len==0.0){
				n[0]=0.0;
				n[1]=1.0;
				n[2]=0.0;}
			else{			len=1.0/len;
				n[0]*=len;
				n[1]*=len;
				n[2]*=len;}

			p1[3] = p2[3] = p3[3] = n[0];
			p1[4] = p2[4] = p3[4] = n[1];
			p1[5] = p2[5] = p3[5] = n[2];

			obj8.geo_tri.set(i1, p1);
			obj8.geo_tri.set(i2, p2);
			obj8.geo_tri.set(i3, p3);

		}
	}
}

void	Obj8ToObj7(const XObj8& obj8, XObj& obj7)
{
	obj7.texture = obj8.texture;
	if (obj7.texture.size() > 4)
		obj7.texture.erase(obj7.texture.size()-4);

	obj7.cmds.clear();

	XObjCmd	cmd7;
	vec_tex	st;
	vec_rgb rgb;
	int n;
	int tri_cmd;

	for (vector<XObjLOD8>::const_iterator lod = obj8.lods.begin(); lod != obj8.lods.end(); ++lod)
	{
		tri_cmd = obj_Tri;
		cmd7.st.clear();
		cmd7.rgb.clear();
		if (lod->lod_far != 0.0)
		{
			cmd7.cmdType = type_Attr;
			cmd7.cmdID = attr_LOD;
			cmd7.attributes.resize(2);
			cmd7.attributes[0] = lod->lod_near;
			cmd7.attributes[1] = lod->lod_far;
			obj7.cmds.push_back(cmd7);
			cmd7.attributes.clear();
		}

		for (vector<XObjCmd8>::const_iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
		{
			switch(cmd->cmd) {
			case obj8_Tris:
				cmd7.cmdType = type_Poly;
				cmd7.cmdID = tri_cmd;
				for (n = 0; n < cmd->idx_count; ++n)
				{
					const float * p = obj8.geo_tri.get(obj8.indices[cmd->idx_offset+n]);
					st.v[0] = p[0];
					st.v[1] = p[1];
					st.v[2] = p[2];
					st.st[0] = p[6];
					st.st[1] = p[7];
					cmd7.st.push_back(st);
					if ((n % 3) == 2)
					{
						obj7.cmds.push_back(cmd7);
						cmd7.st.clear();
					}
				}
				break;
			case obj8_Lines:
				cmd7.cmdType = type_PtLine;
				cmd7.cmdID = obj_Line;
				for (n = 0; n < cmd->idx_count; ++n)
				{
					const float * p = obj8.geo_lines.get(obj8.indices[cmd->idx_offset+n]);
					rgb.v[0] = p[0];
					rgb.v[1] = p[1];
					rgb.v[2] = p[2];
					rgb.rgb[0] = p[3] * 10.0;
					rgb.rgb[1] = p[4] * 10.0;
					rgb.rgb[2] = p[5] * 10.0;
					cmd7.rgb.push_back(rgb);
					if ((n % 2) == 1)
					{
						obj7.cmds.push_back(cmd7);
						cmd7.rgb.clear();
					}
				}
				break;
			case obj8_Lights:
				cmd7.cmdType = type_PtLine;
				cmd7.cmdID = obj_Light;
				for (n = 0; n < cmd->idx_count; ++n)
				{
					const float * p = obj8.geo_lights.get(cmd->idx_offset+n);
					rgb.v[0] = p[0];
					rgb.v[1] = p[1];
					rgb.v[2] = p[2];
					rgb.rgb[0] = p[3] * 10.0;
					rgb.rgb[1] = p[4] * 10.0;
					rgb.rgb[2] = p[5] * 10.0;
					cmd7.rgb.push_back(rgb);
					obj7.cmds.push_back(cmd7);
					cmd7.rgb.clear();
				}
				break;

			case attr_Hard:			tri_cmd = TRI_HARD;	break;
			case attr_No_Hard:		tri_cmd = obj_Tri;	break;
			case attr_Tex_Normal:	tri_cmd = obj_Tri;	break;
			case attr_Tex_Cockpit:	tri_cmd = TRI_COCK;	break;

			case attr_Shade_Flat:
			case attr_Shade_Smooth:
			case attr_Ambient_RGB:
			case attr_Diffuse_RGB:
			case attr_Emission_RGB:
			case attr_Specular_RGB:
			case attr_Shiny_Rat:
			case attr_No_Depth:
			case attr_Depth:
			case attr_LOD:
			case attr_Reset:
			case attr_Cull:
			case attr_NoCull:
			case attr_Offset:
				{
					cmd7.cmdID = cmd->cmd;
					cmd7.cmdType = type_Attr;
					int idx = FindIndexForCmd(cmd->cmd);
					for (n = 0; n < gCmds[idx].elem_count; ++n)
						cmd7.attributes.push_back(cmd->params[n]);
					obj7.cmds.push_back(cmd7);
					cmd7.attributes.clear();
				}
				break;
			}
		}
	}
	special_tris_to_quads(obj7);
}


bool	Obj8_Optimize(XObj8& obj8)
{
	typedef	pair<int, int>		idx_range;
	typedef vector<idx_range>	idx_range_vector;

	idx_range_vector ranges;

	for(vector<XObjLOD8>::iterator L = obj8.lods.begin(); L != obj8.lods.end(); ++L)
	for(vector<XObjCmd8>::iterator C = L->cmds.begin(); C != L->cmds.end(); ++C)
	{
		if(C->cmd == obj8_Tris)
		{
			idx_range me(C->idx_offset, C->idx_offset + C->idx_count);
			for(idx_range_vector::iterator iter = ranges.begin(); iter != ranges.end(); ++iter)
			if(iter->first < me.second &&
			   iter->second > me.first)
			{
				printf("Sorry, the IDX range [%d,%d] overlaps with [%d,%d] so we cannot optimize.\n",
					me.first, me.second, iter->first, iter->second);
				return false;
			}

			ranges.push_back(me);

		}
	}
	vector<unsigned short>	idx16;
	for(vector<int>::iterator idx_iter = obj8.indices.begin(); idx_iter != obj8.indices.end(); ++idx_iter)
	{
		if (*idx_iter > 65535)
		{
			printf("Sorry, we cannot optimize because the indices cannot be reduced to 16 bits.\n");
			return false;
		}
		idx16.push_back(*idx_iter);
	}

	for(idx_range_vector::iterator r = ranges.begin(); r != ranges.end(); ++r)
	{
		PVRTTriStripList(
			&idx16[r->first],
			(r->second - r->first) / 3);


/*
		PVRTGeometrySort(
			obj8.geo_tri.get(0),
			&idx16[r->first],
			32,
			obj8.geo_tri.count(),
			(r->second - r->first) / 3,
			obj8.geo_tri.count(),
			(r->second - r->first) / 3,
			PVRTGEOMETRY_SORT_VERTEXCACHE | PVRTGEOMETRY_SORT_IGNOREVERTS);
*/
	}

/*
	PVRTGeometrySort(
		obj8.geo_tri.get(0),
		&idx16[0],
		32,
		obj8.geo_tri.count(),
		idx16.size() / 3,
		obj8.geo_tri.count(),
		idx16.size() / 3,
		0);
*/
	obj8.indices.clear();
	obj8.indices.insert(obj8.indices.end(),idx16.begin(), idx16.end());

	return true;
}
