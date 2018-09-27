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
#include "WED_FacadePreview.h"
#include "WED_Messages.h"
#include "WED_LibraryMgr.h"
#include "WED_Globals.h"

#include "Agp.h"
#include "MemFileUtils.h"
#include "XObjReadWrite.h"
#include "ObjConvert.h"
#include "FileUtils.h"
#include "WED_PackageMgr.h"
#include "CompGeomDefs2.h"
#include "MathUtils.h"

WED_ResourceMgr::WED_ResourceMgr(WED_LibraryMgr * in_library) : mLibrary(in_library)
{
}

WED_ResourceMgr::~WED_ResourceMgr()
{
	Purge();
}

void	WED_ResourceMgr::Purge(void)
{
	for(map<string, vector<XObj8 *> >::iterator i = mObj.begin(); i != mObj.end(); ++i)
		for(vector<XObj8 *>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			delete *j;
	for(map<string, XObj8 *>::iterator i = mFor.begin(); i != mFor.end(); ++i)
		delete i->second;
	for(map<string, vector<fac_info_t> >::iterator i = mFac.begin(); i != mFac.end(); ++i)
		for(vector<fac_info_t>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			for(vector<XObj8 *>::iterator k = j->previews.begin(); k != j->previews.end(); ++k)
				delete *k;
	for(map<string, str_info_t>::iterator i = mStr.begin(); i != mStr.end(); ++i)
		for(vector<XObj8 *>::iterator j = i->second.previews.begin(); j != i->second.previews.end(); ++j)
			delete *j;
						
	mPol.clear();
	mLin.clear();
	mObj.clear();
	mFor.clear();
	mFac.clear();
	mStr.clear();
}

int		WED_ResourceMgr::GetNumVariants(const string& path)
{
	return mLibrary->GetNumVariants(path);
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

	if (obj->texture.length() > 0) 	FILE_process_texture_path(p,obj->texture);
	if (obj->texture_draped.length() > 0)
	{
		FILE_process_texture_path(p,obj->texture_draped);
		if(obj->texture.length() == 0)
			obj->texture = obj->texture_draped;
	}
	else
		obj->texture_draped = obj->texture;

	mObj[lib_key].push_back(obj);
	
	return true;
}

bool	WED_ResourceMgr::GetObj(const string& path, XObj8 *& obj, int variant)
{
	map<string,vector<XObj8 *> >::iterator i = mObj.find(path);

	if(i != mObj.end())
	{
		DebugAssert(variant < i->second.size());
		obj = i->second[variant];
		return true;
	}
		
	int n_variants = mLibrary->GetNumVariants(path);
	for (int v = 0; v < n_variants; ++v)
	{
		string p = mLibrary->GetResourcePath(path,v);
//	if (!p.size()) p = mLibrary->CreateLocalResourcePath(path);
	
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
		if (obj->texture.length() > 0) 	FILE_process_texture_path(p,obj->texture);
		if (obj->texture_draped.length() > 0)
		{
			FILE_process_texture_path(p,obj->texture_draped);
			if(obj->texture.length() == 0)
				obj->texture = obj->texture_draped;
		}
		else
			obj->texture_draped = obj->texture;

		mObj[path].push_back(obj);
	}

	return true;
}

bool 	WED_ResourceMgr::SetPolUV(const string& path, Bbox2 box)
{
	map<string,pol_info_t>::iterator i = mPol.find(path);
	if(i != mPol.end())
	{
		i->second.mUVBox = box;
		return true;
	}
	else
		return false;
}

bool	WED_ResourceMgr::GetLin(const string& path, lin_info_t& out_info)
{
	map<string,lin_info_t>::iterator i = mLin.find(path);
	if(i != mLin.end())
	{
		out_info = i->second;
		return true;
	}

	out_info.base_tex.clear();
	out_info.scale_s=100;
	out_info.scale_t=100;
	float tex_width = 1024;
	out_info.s1.clear();
	out_info.sm.clear();
	out_info.s2.clear();
	out_info.rgb[0] = 0.75;   // taxi line yellow
	out_info.rgb[1] = 0.6;
	out_info.rgb[2] = 0.15;

	string p = mLibrary->GetResourcePath(path);
	MFMemFile * lin = MemFile_Open(p.c_str());
	if(!lin) return false;

	MFScanner	s;
	MFS_init(&s, lin);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"LINE_PAINT",NULL))
	{
		MemFile_Close(lin);
		return false;
	}

	while(!MFS_done(&s))
	{
		if (MFS_string_match(&s,"TEXTURE", false))
		{
			MFS_string(&s,&out_info.base_tex);
		}
		else if (MFS_string_match(&s,"SCALE", false))
		{
			out_info.scale_s = MFS_double(&s);
			out_info.scale_t = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"TEX_WIDTH", false))
		{
			tex_width = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"PREVIEW_RGB", false))
		{
			out_info.rgb[0] = MFS_double(&s);
			out_info.rgb[1] = MFS_double(&s);
			out_info.rgb[2] = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"S_OFFSET", false))
		{
			float ly = MFS_double(&s);
			float s1 = MFS_double(&s);
			float sm = MFS_double(&s);
			float s2 = MFS_double(&s);
			if (s2 > s1 && s2 > sm)
			{
				out_info.s1.push_back(s1/tex_width);
				out_info.sm.push_back(sm/tex_width);
				out_info.s2.push_back(s2/tex_width);
			}
		}
		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(lin);
	
	if (out_info.s1.size() < 1) 
		return false;

	out_info.eff_width = out_info.scale_s * ( out_info.s2[0] - out_info.s1[0] - 4 / tex_width ); // assume 2 transparent pixels on each side

	FILE_process_texture_path(p,out_info.base_tex);
	mLin[path] = out_info;
	
	return true;
}

#if IBM
#define DIR_CHAR '\\'
#define DIR_STR "\\"
#else
#define DIR_CHAR '/'
#define DIR_STR "/"
#endif

static void clean_rpath(string& s)
{
	for(string::size_type p = 0; p < s.size(); ++p)
		if(s[p] == '\\' || s[p] == ':' || s[p] == '/')
			s[p] = DIR_CHAR;
}

bool	WED_ResourceMgr::GetStr(const string& path, str_info_t& out_info)
{
	map<string,str_info_t>::iterator i = mStr.find(path);
	if(i != mStr.end())
	{
		out_info = i->second;
		return true;
	}

	out_info.offset = 0.0;
	out_info.rotation = 0.0;
	out_info.previews.clear();

	string p = mLibrary->GetResourcePath(path);
	MFMemFile * str = MemFile_Open(p.c_str());
	if(!str) return false;

	MFScanner	s;
	MFS_init(&s, str);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"OBJECT_STRING",NULL))
	{
		MemFile_Close(str);
		return false;
	}

	while(!MFS_done(&s))
	{
		if (MFS_string_match(&s,"OFFSET", false))
		{
			out_info.offset = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"OBJECT", false))
		{
			out_info.rotation = MFS_double(&s);
			int ignore = MFS_double(&s);
			string obj_res;
			MFS_string(&s,&obj_res);
			
			clean_rpath(obj_res);
			obj_res= FILE_get_dir_name(p) + obj_res;
			FILE_case_correct( (char *) obj_res.c_str()); 

			XObj8 * obj = new XObj8;
			if(!XObj8Read(obj_res.c_str(),*obj))
			{
				XObj obj7;
				if(XObjRead(obj_res.c_str(),obj7))
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
			if (obj->texture.length() > 0) 	FILE_process_texture_path(p,obj->texture);
			if (obj->texture_draped.length() > 0)
			{
				FILE_process_texture_path(p,obj->texture_draped);
				if(obj->texture.length() == 0)
					obj->texture = obj->texture_draped;
			}
			else
				obj->texture_draped = obj->texture;

			out_info.previews.push_back(obj);
		}
		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(str);
	mStr[path] = out_info;
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
	
	out_info.mSubBoxes.clear();
	out_info.mUVBox = Bbox2();

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
	out_info.hasDecal=false;
	out_info.proj_s=1000;
	out_info.proj_t=1000;
	out_info.kill_alpha=false;
	out_info.wrap=false;

	while(!MFS_done(&s))
	{
		if (MFS_string_match(&s,"TEXTURE", false))
		{
			MFS_string(&s,&out_info.base_tex);
			out_info.wrap=true;
		}
		else if (MFS_string_match(&s,"SCALE", false))
		{
			out_info.proj_s = MFS_double(&s);
			out_info.proj_t = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"#subtex", false)) 
		{
			float s1 = MFS_double(&s);
			float t1 = MFS_double(&s);
			float s2 = MFS_double(&s);
			float t2 = MFS_double(&s);
			if (s2 > s1 && t2 > t1)
			{
//		printf("read subtex\n");
				out_info.mSubBoxes.push_back(Bbox2(s1,t1,s2,t2));
			}
		}
		else if (MFS_string_match(&s,"TEXTURE_NOWRAP", false))
		{
			MFS_string(&s,&out_info.base_tex);
			out_info.wrap=false;
		}
		else if (MFS_string_match(&s,"DECAL_LIB", true))
		{
			out_info.hasDecal=true;
		}
		else if (MFS_string_match(&s,"NO_ALPHA", true))
		{
			out_info.kill_alpha=true;
		}
		else if (MFS_string_match(&s,"LAYER_GROUP",false))
		{
			MFS_string(&s,&out_info.group);
			out_info.group_offset = MFS_int(&s);
		}

		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(pol);

	FILE_process_texture_path(p,out_info.base_tex);
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
	int pp = p.find_last_of(".");
	p = p.substr(0,pp+1) + "pol";
	
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

bool	WED_ResourceMgr::GetFac(const string& path, fac_info_t& out_info, int variant)
{
	map<string,vector<fac_info_t> >::iterator i = mFac.find(path);
	if(i != mFac.end())
	{
//printf("OLD FAC p=%s, v=%d, nv=%d\n",path.c_str(),variant, (int) i->second.size());
		DebugAssert(variant < i->second.size());
		out_info = i->second[variant];
		return true;
	}

	int n_variants = mLibrary->GetNumVariants(path);
	
//printf("NEW FAC p=%s, v=%d\n",path.c_str(),n_variants);

	for(int v = 0; v < n_variants; ++v)
	{
		string p = mLibrary->GetResourcePath(path, v);

		MFMemFile * fac = MemFile_Open(p.c_str());
		if(!fac) return false;

		MFScanner	s;
		MFS_init(&s, fac);

		int versions[] = { 800,900,1000, 0 };
		if((out_info.version = MFS_xplane_header(&s,versions,"FACADE",NULL)) == 0)
		{
			MemFile_Close(fac);
			return false;
		}
		out_info.ring = true;
		out_info.roof = false;
		out_info.floors_min = 1.0;
		out_info.floors_max = 9999.0;
		out_info.previews.clear();

		// these dont need to be public	
		out_info.roof_slope = 0.0;
		out_info.roof_height = 0.0;
		out_info.scale_x = 20.0;
		out_info.scale_y = 20.0;


		vector <wall_map_t> wall;
		
		string		wall_tex, roof_tex;
		float 		roof_uv[4]  = { 0.0, 0.0, 1.0, 1.0 };
		float 		tex_size_x = 1024.0, tex_size_y = 1024.0;

		bool roof_section = false;
		bool no_roof_mesh = false;
		 
		while(!MFS_done(&s))
		{
			if (MFS_string_match(&s,"RING", false))
			{
				out_info.ring = MFS_int(&s) > 0;
			}
			else if (MFS_string_match(&s,"ROOF", false))
			{
				roof_section = true;
				out_info.roof = true;
	//			roof_uv[2] = MFS_double(&s);
	//			roof_uv[3] = MFS_double(&s);
			}
			else if (MFS_string_match(&s,"NO_ROOF_MESH", false))
			{
				no_roof_mesh = true;
			}
			else if (MFS_string_match(&s,"ROOF_SCALE", false))
			{
				out_info.roof = true;
				roof_uv[0] = MFS_double(&s)/tex_size_x;
				roof_uv[1] = MFS_double(&s)/tex_size_y;
				MFS_double(&s);
				MFS_double(&s);
				roof_uv[2] = MFS_double(&s)/tex_size_x;
				roof_uv[3] = MFS_double(&s)/tex_size_y;
			}
			else if (MFS_string_match(&s,"FLOORS_MIN", false))
			{
				out_info.floors_min = MFS_double(&s);
			}
			else if (MFS_string_match(&s,"FLOORS_MAX", false))
			{
				out_info.floors_max = MFS_double(&s);
			}
			else if (MFS_string_match(&s,"ROOF_HEIGHT", false))
			{
				out_info.roof = true;
				out_info.roof_height = MFS_double(&s);
			}
			else if (MFS_string_match(&s,"ROOF_SLOPE", false))
			{
				out_info.roof = true;
				out_info.roof_slope = MFS_double(&s);
			}
			else if (MFS_string_match(&s,"SHADER_ROOF", false))
			{
				roof_section = true;
			}
			else if (MFS_string_match(&s,"SHADER_WALL", false))
			{
				roof_section = false;
			}
			else if (MFS_string_match(&s,"WALL",false))
			{
				string buf;
				struct wall_map_t z;
				
				roof_section = false;
				z.min_w = MFS_double(&s);
				z.max_w = MFS_double(&s);
				MFS_double(&s);
				MFS_double(&s);
				MFS_string(&s,&buf);
				
				wall.push_back(z);

				char c[100];
				if (buf.empty())                 // make sure all wall types have some readable name
				{   
					sprintf(c,"#%i", (int) out_info.walls.size()); 
					buf = c;
				}
				out_info.walls.push_back(buf);

				sprintf(c, "w=%.0f%c to %.0f%c", z.min_w / (gIsFeet ? 0.3048 : 1.0 ), gIsFeet ? '\'' : 'm', z.max_w  / (gIsFeet ? 0.3048 : 1.0 ), gIsFeet ? '\'' : 'm') ;
				buf = c;
				out_info.w_use.push_back(buf);
			} 
			else if(MFS_string_match(&s,"FLOOR",false))
			{
				out_info.walls.clear();
			}
			else if (MFS_string_match(&s,"TEXTURE", false))
			{
				if (roof_section)
					MFS_string(&s,&roof_tex);
				else
					MFS_string(&s,&wall_tex);
			}
			else if (MFS_string_match(&s,"SCALE", false))
			{
				out_info.scale_x = MFS_double(&s);
				out_info.scale_y = MFS_double(&s);
			}
			else if (MFS_string_match(&s,"TEX_SIZE",false))
			{
				tex_size_x = MFS_double(&s);
				tex_size_y = MFS_double(&s);
			} 
			else if (MFS_string_match(&s,"BOTTOM",false))
			{
				float x = MFS_double(&s)/tex_size_y;
				if(wall.back().vert[1] == 0.0)
					wall.back().vert[0] = x;
				wall.back().vert[1] = MFS_double(&s)/tex_size_y;
			} 
			else if (MFS_string_match(&s,"MIDDLE",false))
			{
				float x = MFS_double(&s)/tex_size_y;
				wall.back().vert[2] = MFS_double(&s)/tex_size_y;
				if(wall.back().vert[1] == 0.0)
				{
	//printf("%s no bot but mid\n",p.c_str());
					wall.back().vert[0] = x;
					wall.back().vert[1] = wall.back().vert[2];
				}
			} 
			else if (MFS_string_match(&s,"TOP",false))
			{
				MFS_double(&s);
				wall.back().vert[3]  = MFS_double(&s)/tex_size_y;
			} 
			else if (MFS_string_match(&s,"LEFT",false))
			{
				float x = MFS_double(&s)/tex_size_x;
				if(wall.back().hori[1] == 0.0)
				{
	//printf("%s first left\n",p.c_str());
					wall.back().hori[0] = x;
				}
				wall.back().hori[1] = MFS_double(&s)/tex_size_x;
			} 
			else if (MFS_string_match(&s,"CENTER",false))
			{
				float x = MFS_double(&s)/tex_size_x;
				if(wall.back().hori[1] == 0.0)
				{
	//printf("%s no left but center\n",p.c_str());
					wall.back().hori[1] = x;
					wall.back().hori[0] = x;
				}
				wall.back().hori[2] = MFS_double(&s)/tex_size_x;
			} 
			else if (MFS_string_match(&s,"RIGHT",false))
			{
				MFS_double(&s);
				wall.back().hori[3] = MFS_double(&s)/tex_size_x;
			} 

			MFS_string_eol(&s,NULL);
		}
		MemFile_Close(fac);

		FILE_process_texture_path(p,wall_tex);
		FILE_process_texture_path(p,roof_tex);
		WED_MakeFacadePreview(out_info, wall, wall_tex, roof_uv, roof_tex);

		if (no_roof_mesh) out_info.roof = false;
		mFac[path].push_back(out_info);
	}
	
	out_info = mFac[path].front();

	return true;
}

#define TPR 6           // # of trees shown in a row

struct tree_t {
	float s,t,w,y; 		// texture coordinates of tree
	float o;            // offset of tree center line (where the quads inersect)
	float pct;          // relative occurence percentage for this tree
	float hmin,hmax;    // height range for this tree in meters
	int q;				// number of quads the tree is constructed of
};

bool	WED_ResourceMgr::GetFor(const string& path,  XObj8 *& obj)
{
	map<string,XObj8 *>::iterator i = mFor.find(path);
	if(i != mFor.end())
	{
		obj = i->second;
		return true;
	}
	
	string p = mLibrary->GetResourcePath(path);
	
	MFMemFile * fi = MemFile_Open(p.c_str());
	if(!fi) return false;

	MFScanner	s;
	MFS_init(&s, fi);

	int versions[] = { 800,900,1000, 0 };
	if((MFS_xplane_header(&s,versions,"FOREST",NULL)) == 0)
	{
		MemFile_Close(fi);
		return false;
	}

	vector <tree_t> tree;
	float scale_x=256, scale_y=256, space_x=30, space_y=30, rand_x=0, rand_y=0;
	string tex;

	while(!MFS_done(&s))
	{
		if(MFS_string_match(&s,"TEXTURE",false))
		{
			MFS_string(&s,&tex);
		}
		else if (MFS_string_match(&s,"SCALE_X", false))
		{
			scale_x = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"SCALE_Y", false))
		{
			scale_y = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"SPACING", false))
		{
			space_x = MFS_double(&s);
			space_y = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"RANDOM", false))
		{
			rand_x = MFS_double(&s);
			rand_y = MFS_double(&s);
		}		
		else if (MFS_string_match(&s,"TREE", false))
		{
			tree_t t;
			t.s    = MFS_double(&s);
			t.t    = MFS_double(&s);
			t.w    = MFS_double(&s);
			t.y    = MFS_double(&s);
			t.o    = MFS_double(&s);
			t.pct  = MFS_double(&s);
			t.hmin = MFS_double(&s);
			t.hmax = MFS_double(&s);
			t.q    = MFS_int(&s);

			if (fabs(t.w) > 0.001 && t.y > 0.001 )   // there are some .for with zero size tree's in XP10 and OpensceneryX uses negative widths ...
				tree.push_back(t);
		}	
		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(fi);

	// now we have one of each tree. Like on the ark. Or maybe half that :)
	// expand that to full forest of TPS * TPS trees, populated with all the varieties there are
	int varieties =  tree.size();
	vector <tree_t> treev = tree;
	tree.clear();
	
	if (varieties < 1) return false;

#if 0		// truely random tree choice, taken into account each tree's relative percentage
			// it works, but not so perfect for a forest with a relatively small number of tree's
			// e.g. a 36 tree forest with one tree ocurring at 3.5% may have either 0, 1 or 2 of that kind
					
	for (int i=0; i<TPR*TPR; ++i)
	{
		int species = 0;
		float prob = (100.0*rand())/RAND_MAX;
		for (species=varieties-1; species>0; --species)
			if (prob < treev[species].pct)
				break;
			else
				prob-=treev[species].pct;
		// if the pct for all tree's don't add up too 100% - species #0 will make up for it. 
		// XP seems to do the same.
		
		tree.push_back(treev[species]);
	}
#else
	int species[TPR*TPR] = {};
	
	for (int i=varieties-1; i>0; --i)
		for (int j=0; j<round(treev[i].pct/100.0*TPR*TPR); ++j)
		{
			int cnt=10;     // needed in case the tree percentages add up to more than 100%
			do
			{
				int where = ((float) TPR*TPR*rand())/RAND_MAX;
				if(where >= 0 && where < TPR*TPR && !species[where]) 
				{ 
					species[where] = i;
					break;
				}
			} while (--cnt);
		}
	for (int i=0; i<TPR*TPR; ++i)
		tree.push_back(treev[species[i]]);
#endif		
	
	// fills a XObj8-structure for library preview
	obj = new XObj8;
	XObjCmd8 cmd;

	obj->texture = tex;
	FILE_process_texture_path(p, obj->texture);
	
	int quads=0;

	// "VT "
	for (int i = 0; i < tree.size(); ++i)
	{
		float t_h = tree[i].hmin + ((float) rand())/RAND_MAX * (tree[i].hmax-tree[i].hmin);
		float t_w = t_h * tree[i].w/tree[i].y;                                 // full width of tree
		float t_x = (i % TPR) * space_x + rand_x*((2.0*rand())/RAND_MAX-1.0);  // tree position
		float t_y = (i / TPR) * space_y + rand_y*((2.0*rand())/RAND_MAX-1.0);
		float rot_r = ((float) rand())/RAND_MAX;

		for (int j=0; j<tree[i].q; ++j)
		{
			float rot = M_PI*(rot_r+j/(float) tree[i].q);        // tree rotation
			float x = t_w * sin(rot);
			float z = t_w * cos(rot);

			quads++;
			
			float pt[8];
			pt[3] = 0.0;
			pt[4] = 1.0;
			pt[5] = 0.0;
			
			pt[0] = t_x - x*(tree[i].o/tree[i].w);
			pt[1] = 0.0;
			pt[2] = t_y - z*(tree[i].o/tree[i].w);
			pt[6] = tree[i].s/scale_x;
			pt[7] = tree[i].t/scale_y;

			obj->geo_tri.append(pt);
			pt[0] = t_x + x*(1.0-tree[i].o/tree[i].w);
			pt[2] = t_y + z*(1.0-tree[i].o/tree[i].w);
			pt[6] = (tree[i].s+tree[i].w)/scale_x;
			obj->geo_tri.append(pt);
			pt[1] = t_h;
			pt[7] = (tree[i].t+tree[i].y)/scale_y;
			obj->geo_tri.append(pt);
			pt[0] = t_x - x*(tree[i].o/tree[i].w);
			pt[2] = t_y - z*(tree[i].o/tree[i].w);
			pt[6] = tree[i].s/scale_x;
			obj->geo_tri.append(pt);
		}
	}
	// set dimension
	obj->geo_tri.get_minmax(obj->xyz_min,obj->xyz_max);

	// "IDX "
	int seq[6] = {0,1,2,0,2,3};
	for (int i = 0; i < 6*quads; ++i)
		obj->indices.push_back(4*(i/6)+seq[i%6]);

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

	mFor[path] = obj;
    // only problem is that the texture path contain spaces -> obj reader can not read that
    // but still valuable for checking the values/structure
//    XObj8Write(mLibrary->CreateLocalResourcePath("forest_preview.obj").c_str(), *obj);

	return true;
}

#if AIRPORT_ROUTING
bool	WED_ResourceMgr::GetAGP(const string& path, agp_t& out_info)
{
	map<string, agp_t>::iterator i = mAGP.find(path);
	if(i != mAGP.end())
	{
		out_info = i->second;
		return true;
	}

	const bool loaded = ForceLoadAGP(mLibrary->GetResourcePath(path), out_info);
	if(loaded)
	{
		mAGP[path] = out_info;
	}
	return loaded;
}

bool	WED_ResourceMgr::GetRoad(const string& path, road_info_t& out_info)
{
	map<string,road_info_t>::iterator i = mRoad.find(path);
	if(i != mRoad.end())
	{
		out_info = i->second;
		return true;
	}
	
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * road = MemFile_Open(p.c_str());
	if(!road) return false;

	MFScanner	s;
	MFS_init(&s, road);

	int versions[] = { 800, 0 };
	int v;
	if((v=MFS_xplane_header(&s,versions,"ROADS",NULL)) == 0)
	{
		MemFile_Close(road);
		return false;
	}
	
	string last_name;
	
	while(!MFS_done(&s))
	{
		if(MFS_string_match(&s,"#VROAD",false))
		{
			MFS_string(&s,&last_name);
		}
		if(MFS_string_match(&s, "ROAD_DRAPED", 0))
		{
			MFS_int(&s);		// draping mode
			int id = MFS_int(&s);
			out_info.vroad_types[id] = last_name;
		}
		MFS_string_eol(&s, NULL);
	}
	
	MemFile_Close(road);
	mRoad[path] = out_info;
	return true;
}
	
#endif
