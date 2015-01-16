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

#include "WED_ResourceMgr.h"
#include "WED_Messages.h"
#include "WED_LibraryMgr.h"
#include "MemFileUtils.h"
#include "XObjReadWrite.h"
#include "XObjDefs.h"
#include "ObjConvert.h"
#include "FileUtils.h"
#include "WED_PackageMgr.h"
#include "CompGeomDefs2.h"

static void process_texture_path(const string& path_of_obj, string& path_of_tex)
{
	string root(path_of_tex);
	string parent(path_of_obj);

	string::size_type pp = parent.find_last_of("\\:/");
	if(pp == parent.npos) parent.clear();
	else				  parent.erase(pp+1);

	pp = root.find_last_of(".");
	if(pp != root.npos) root.erase(pp);
											path_of_tex = parent + root + ".dds";
	if(!FILE_exists(path_of_tex.c_str()))	path_of_tex = parent + root + ".png";
	if(!FILE_exists(path_of_tex.c_str()))	path_of_tex = parent + root + ".bmp";
}

WED_ResourceMgr::WED_ResourceMgr(WED_LibraryMgr * in_library) : mLibrary(in_library)
{
}

WED_ResourceMgr::~WED_ResourceMgr()
{
	Purge();

}

void	WED_ResourceMgr::Purge(void)
{
	for(map<string, XObj8 *>::iterator i = mObj.begin(); i != mObj.end(); ++i)
		delete i->second;
	mPol.clear();
	mObj.clear();
}

bool	WED_ResourceMgr::GetObjRelative(const string& obj_path, const string& parent_path, XObj8 *& obj)
{
	if(GetObj(obj_path,obj))
		return true;
	string lib_key = parent_path + string("\n") + obj_path;
	if(GetObj(lib_key,obj))
		return true;
	string full_parent = mLibrary->GetResourcePath(parent_path);
	string::size_type s = full_parent.find_last_of("\\/:");
	if(s == full_parent.npos) full_parent.clear(); else full_parent.erase(s+1);
	string p = full_parent + obj_path;

	obj = new XObj8;
	if(!XObj8Read(p.c_str(),*obj))
	{
		XObj obj7;
		if(XObjRead(p.c_str(),obj7))
		{
			Obj7ToObj8(obj7,*obj);
		}
		else
		{
			delete obj;
			obj = NULL;
			return false;
		}
	}

	process_texture_path(p,obj->texture);
	process_texture_path(p,obj->texture_draped);

	mObj[lib_key] = obj;
	return true;
	
	
}

bool	WED_ResourceMgr::GetObj(const string& path, XObj8 *& obj)
{
	map<string,XObj8 *>::iterator i = mObj.find(path);
	if(i != mObj.end())
	{
		obj = i->second;
		return true;
	}

	string p = mLibrary->GetResourcePath(path);
	obj = new XObj8;
	if(!XObj8Read(p.c_str(),*obj))
	{
		XObj obj7;
		if(XObjRead(p.c_str(),obj7))
		{
			Obj7ToObj8(obj7,*obj);
		}
		else
		{
			delete obj;
			obj = NULL;
			return false;
		}
	}

	process_texture_path(p,obj->texture);
	process_texture_path(p,obj->texture_draped);

	mObj[path] = obj;
	return true;
}

bool	WED_ResourceMgr::GetPol(const string& path, pol_info_t& out_info)
{
	map<string,pol_info_t>::iterator i = mPol.find(path);
	if(i != mPol.end())
	{
		out_info = i->second;
		return true;
	}

	string p = mLibrary->GetResourcePath(path);
	MFMemFile * pol = MemFile_Open(p.c_str());
	if(!pol) return false;


	MFScanner	s;
	MFS_init(&s, pol);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"DRAPED_POLYGON",NULL))
	{
		MemFile_Close(pol);
		return false;
	}

	out_info.base_tex.clear();
	out_info.proj_s=1000;
	out_info.proj_t=1000;
	out_info.kill_alpha=false;
	out_info.wrap=false;

	while(!MFS_done(&s))
	{
		// TEXTURE <texname>
		if (MFS_string_match(&s,"TEXTURE", false))
		{
			MFS_string(&s,&out_info.base_tex);
			out_info.wrap=true;
		}
		// SCALE <ms> <mt>
		else if (MFS_string_match(&s,"SCALE", false))
		{
			out_info.proj_s = MFS_double(&s);
			out_info.proj_t = MFS_double(&s);
		}
		// TEXTURE_NOWRAP <texname>
		else if (MFS_string_match(&s,"TEXTURE_NOWRAP", false))
		{
			MFS_string(&s,&out_info.base_tex);
			out_info.wrap=false;
		}
		// NO_ALPHA
		else if (MFS_string_match(&s,"NO_ALPHA", true))
		{
			out_info.kill_alpha=true;
		}
		// LAYER_GROUP
		else if (MFS_string_match(&s,"LAYER_GROUP",false))
		{
			MFS_string(&s,&out_info.group);
			out_info.group_offset = MFS_int(&s);
		}

		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(pol);

	process_texture_path(p,out_info.base_tex);

	mPol[path] = out_info;
	return true;
}

void WED_ResourceMgr::MakePol(const string& path, const pol_info_t& out_info)
{
	map<string,pol_info_t>::iterator i = mPol.find(path);
	if(i != mPol.end())
	{
		return;
	}

	string p = mLibrary->CreateLocalResourcePath(path);
	//Makes sure that the file will end in .pol
	string pol = "pol";
	p.replace(p.length()-3,3,pol);
	FILE * fi = fopen(p.c_str(), "w");
	if(!fi)	return;
	fprintf(fi,"A\n850\nDRAPED_POLYGON\n\n");
	
	fprintf(fi,out_info.wrap ? "TEXTURE %s\n" : "TEXTURE_NOWRAP %s\n", out_info.base_tex.c_str());
	fprintf(fi,"SCALE %lf %lf\n",out_info.proj_s,out_info.proj_t);
		/*float		latitude;
	float		longitude;
	double		height_Meters;
	int			ddsHeight_Pxls;*/
	fprintf(fi,"LOAD_CENTER %lf %lf %f %d", out_info.latitude, out_info.longitude,out_info.height_Meters,out_info.ddsHeight_Pxls);
	if(out_info.kill_alpha)
		fprintf(fi,"NO_ALPHA\n");
	fclose(fi);	
	gPackageMgr->Rescan();
}



void	WED_ResourceMgr::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
	if (inMsg == msg_SystemFolderChanged ||
		inMsg == msg_SystemFolderUpdated)
	{
		Purge();
	}
}

bool	WED_ResourceMgr::GetFac(const string& path, fac_info_t& out_info)
{
	out_info.ring = true;
	out_info.roof = true;
	out_info.modern = true;
	
	map<string,fac_info_t>::iterator i = mFac.find(path);
	if(i != mFac.end())
	{
		out_info = i->second;
		return true;
	}

	string p = mLibrary->GetResourcePath(path);
	MFMemFile * fac = MemFile_Open(p.c_str());
	if(!fac) return false;


	MFScanner	s;
	MFS_init(&s, fac);

	int versions[] = { 800,900,1000, 0 };
	int v;
	if((v=MFS_xplane_header(&s,versions,"FACADE",NULL)) == 0)
	{
		MemFile_Close(fac);
		return false;
	}
	out_info.modern = v > 800;

	out_info.ring = true;
	out_info.roof = false;

	while(!MFS_done(&s))
	{
		// RING
		if (MFS_string_match(&s,"RING", false))
		{
			out_info.ring = MFS_int(&s) > 0;
		}
		// ROOF
		else if (MFS_string_match(&s,"ROOF", false))
		{
			out_info.roof = true;
		}
		else if (MFS_string_match(&s,"ROOF_SCALE", false))
		{
			out_info.roof = true;
		}
		// ROOF_HEIGHT
		else if (MFS_string_match(&s,"ROOF_HEIGHT", false))
		{
			out_info.roof = true;
		}
		// WALL min max min max name
		else if (MFS_string_match(&s,"WALL",false))
		{
			MFS_double(&s);
			MFS_double(&s);
			MFS_double(&s);
			MFS_double(&s);
			out_info.walls.push_back(string());
			MFS_string(&s,&out_info.walls.back());
		} 
		else if(MFS_string_match(&s,"FLOOR",false))
		{
			out_info.walls.clear();
		}

		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(fac);

	mFac[path] = out_info;
	return true;
}

inline void	do_rotate(int n, double& io_x, double& io_y)
{
	Vector2 v(io_x,io_y);
	while(n > 0)
	{
		v = v.perpendicular_ccw();
		--n;
	}
	io_x = v.dx;
	io_y = v.dy;
}


#if AIRPORT_ROUTING
bool	WED_ResourceMgr::GetAGP(const string& path, agp_t& out_info)
{
	map<string,agp_t>::iterator i = mAGP.find(path);
	if(i != mAGP.end())
	{
		out_info = i->second;
		return true;
	}
	
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * agp = MemFile_Open(p.c_str());
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

	while(!MFS_done(&s))
	{
		if(MFS_string_match(&s,"TEXTURE",false))
		{
			MFS_string(&s,&out_info.base_tex);
			process_texture_path(p,out_info.base_tex);
			
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
	}
	
	MemFile_Close(agp);

	mAGP[path] = out_info;
	return true;
}
	
	
#endif