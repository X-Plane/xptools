#include "Agp.h"
#include "MemFileUtils.h"
#include "CompGeomDefs2.h"
#include "FileUtils.h"


inline void	do_rotate(int n, double& io_x, double& io_y)
{
	Vector2 v(io_x,io_y);
	while(n > 0)
	{
		v = v.perpendicular_cw();
		--n;
	}
	io_x = v.dx;
	io_y = v.dy;
}

bool load_agp(const string &disk_path, agp_t& out_info)
{
	MFMemFile * agp = MemFile_Open(disk_path.c_str());
	if(!agp) return false;

	MFScanner	s;
	MFS_init(&s, agp);

	int versions[] = { 1000, 0 };
	int v;
	if((v=MFS_xplane_header(&s,versions,"AG_POINT",NULL)) == 0)
	{
		MemFile_Close(agp);
		return false;
	}
	
	double tex_s = 1.0, tex_t = 1.0;		// these scale from pixels to UV coords
	double tex_x = 1.0, tex_y = 1.0;		// meters for tex, x & y
	int	 rotation = 0;
	double anchor_x = 0.0, anchor_y = 0.0;
	out_info.hide_tiles = 0;
	vector<string>	obj_paths;

	bool is_mesh_shader = false;

	while(!MFS_done(&s))
	{
		if(MFS_string_match(&s,"TEXTURE",false))
		{
			string tex;
			MFS_string(&s,&tex);
			if(is_mesh_shader)
			{
				out_info.mesh_tex = tex;
				FILE_process_texture_path(disk_path,out_info.mesh_tex);
			}
			else
			{
				out_info.base_tex = tex;
				FILE_process_texture_path(disk_path,out_info.base_tex);
			}
		}
		else if(MFS_string_match(&s,"TEXTURE_SCALE",false))
		{
			tex_s = 1.0 / MFS_double(&s);
			tex_t = 1.0 / MFS_double(&s);
		}
		else if(MFS_string_match(&s,"TEXTURE_WIDTH",false))
		{
			tex_x = MFS_double(&s);
			tex_y = tex_x * tex_s / tex_t;
		}
		else if(MFS_string_match(&s,"OBJECT",false))
		{
			string p;
			MFS_string(&s,&p);
			obj_paths.push_back(p);
		}
		else if(MFS_string_match(&s,"TILE",false))
		{
			out_info.tile.resize(16);
			double s1 = MFS_double(&s);
			double t1 = MFS_double(&s);
			double s2 = MFS_double(&s);
			double t2 = MFS_double(&s);
			double x1 = s1 * tex_s * tex_x;
			double x2 = s2 * tex_s * tex_x;
			double y1 = t1 * tex_t * tex_y;
			double y2 = t2 * tex_t * tex_y;
			
			s1 *= tex_s;
			s2 *= tex_s;
			t1 *= tex_t;
			t2 *= tex_t;
			
			anchor_x = (x1 + x2) * 0.5;
			anchor_y = (y1 + y2) * 0.5;
			out_info.tile[ 0] = x1;
			out_info.tile[ 1] = y1;
			out_info.tile[ 2] = s1;
			out_info.tile[ 3] = t1;
			out_info.tile[ 4] = x2;
			out_info.tile[ 5] = y1;
			out_info.tile[ 6] = s2;
			out_info.tile[ 7] = t1;
			out_info.tile[ 8] = x2;
			out_info.tile[ 9] = y2;
			out_info.tile[10] = s2;
			out_info.tile[11] = t2;
			out_info.tile[12] = x1;
			out_info.tile[13] = y2;
			out_info.tile[14] = s1;
			out_info.tile[15] = t2;
		}
		else if(MFS_string_match(&s,"ROTATION",false))
		{
			rotation = MFS_int(&s);
		}
		else if(MFS_string_match(&s,"CROP_POLY",false))
		{
			out_info.tile.clear();
			while(MFS_has_word(&s))
			{
				double ps = MFS_double(&s);
				double pt = MFS_double(&s);
				out_info.tile.push_back(ps * tex_s * tex_x);
				out_info.tile.push_back(pt * tex_t * tex_y);
				out_info.tile.push_back(ps * tex_s);
				out_info.tile.push_back(pt * tex_t);
			}
		}
		else if(MFS_string_match(&s,"OBJ_DRAPED",false) ||
				MFS_string_match(&s,"OBJ_GRADED",false))
		{
			out_info.objs.push_back(agp_t::obj());
			out_info.objs.back().x = MFS_double(&s) * tex_s * tex_x;
			out_info.objs.back().y = MFS_double(&s) * tex_t * tex_y;
			out_info.objs.back().r = MFS_double(&s);
			out_info.objs.back().name = obj_paths[MFS_int(&s)];
			out_info.objs.back().show_lo = MFS_int(&s);
			out_info.objs.back().show_hi = MFS_int(&s);
		}
		else if(MFS_string_match(&s,"ANCHOR_PT",false))
		{
			anchor_x = MFS_double(&s) * tex_s * tex_x;
			anchor_y = MFS_double(&s) * tex_t * tex_y;
		}
		else if (MFS_string_match(&s,"HIDE_TILES",true))
		{
			out_info.hide_tiles = 1;
		}
		else if (MFS_string_match(&s,"MESH_SHADER",true))
		{
			is_mesh_shader = true;
		}
		MFS_string_eol(&s,NULL);
	}
	
	for(int n = 0; n < out_info.tile.size(); n += 4)
	{
		out_info.tile[n  ] -= anchor_x;
		out_info.tile[n+1] -= anchor_y;
		do_rotate(rotation,out_info.tile[n  ],out_info.tile[n+1]);
	}
	for(vector<agp_t::obj>::iterator o = out_info.objs.begin(); o != out_info.objs.end(); ++o)
	{
		o->x -= anchor_x;
		o->y -= anchor_y;
		do_rotate(rotation,o->x,o->y);
		o->r += 90.0 * rotation;
	}
	
	MemFile_Close(agp);
	return true;
}
