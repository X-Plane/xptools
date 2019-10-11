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
#include "WED_Version.h"

#include "MemFileUtils.h"
#include "XObjReadWrite.h"
#include "ObjConvert.h"
#include "FileUtils.h"
#include "WED_PackageMgr.h"
#include "CompGeomDefs2.h"
#include "MathUtils.h"

/* Resouce Manager Theory of operation:
	It provides access to all properties/details of any art asset referenced in WED.	Normally these art assets are 
	identified by a virtual path (vpath). This is how all non-local assets are indexed in the RegMgr's databases.
   As the library manager also know about all art assets local to a scenery - these can also be referenced by
   the vpath - althought this is rather a real path, relative to the scenery directory here.
   
   Additionally - some art assets like .agp, .fac and .str can also reference other .obj assets in their definitions.
   These can be either vpaths or real path's relative to the art assets location. These are loadable by the 
   GetObjRelative, only. It takes the art asset name of the referencing asset plus the resouce same of the 
   objects referenced. Then it uses the LibraryMgr to see if that object matches any existing vpath. If not, 
   it attempts to load the object by calculating the absolute path and load it under that name.
   
   This causes currenly two issues:
   
   E.g. some agp references a resource via a relative path (e.g. ../objects/xxx.obj) is stored in duplicate in WED.
   As the ResMgr would not knwo there is also one or more vpath's referencing the same objects. It causes some 
   duplication, but no further ill effects. It could be avoided by translating such relative paths at art asset read-in time,
   using the libMgr to identify any relative path that is pointing to the same absolute path as any existing vapth and
   then replace the relative path by the vpath.
   
   E.g. some agp references a resource as above, but that relative path is identical to an existing local art asset.
   E.g. objects/xxx.obj. As the LibMgr also recognizes this as as valid vpath - it will resolve it to the local object, 
   rather than return "not found" and let it fall back to a path relative to the .agp's location.
   
   A possible fix for this is to reverse the search order for such item, i.e. first see if the object specified can be 
   found at a path relative to the referencing art assets location. If not - ask the lib Mgr to resolve a path for it.
*/

static void process_texture_path(const string& path_of_obj, string& path_of_tex)
{
	string parent;

	parent = FILE_get_dir_name(path_of_obj) + FILE_get_dir_name(path_of_tex)
					+ FILE_get_file_name_wo_extensions(path_of_tex);

	path_of_tex = parent + ".dds";          // no need to also check for .DDS, filename case desense will take care of it
	if(FILE_exists(path_of_tex.c_str()))  return;
	path_of_tex = parent + ".png";
	if(FILE_exists(path_of_tex.c_str()))  return;
	path_of_tex = parent + ".bmp";
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
	for(auto i : mObj)
		for(auto j : i.second)
			delete j;
	for(auto i : mFor)
		delete i.second;
						
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

XObj8 * WED_ResourceMgr::LoadObj(const string& abspath)
{

//printf("LoadObj '%s' - ",abspath.c_str());

	XObj8 * new_obj = new XObj8;
	if(!XObj8Read(abspath.c_str(),*new_obj))
	{
		XObj obj7;
		if(XObjRead(abspath.c_str(),obj7))
		{
			Obj7ToObj8(obj7,*new_obj);
		}
		else
		{
			delete new_obj;
//printf("NULL\n"); fflush(stdout);
		
			return nullptr;
		}
	}

	if (new_obj->texture.length() > 0) 	process_texture_path(abspath, new_obj->texture);
	if (new_obj->texture_draped.length() > 0)
	{
		process_texture_path(abspath, new_obj->texture_draped);
		if(new_obj->texture.length() == 0)
			new_obj->texture = new_obj->texture_draped;
	}
	else
		new_obj->texture_draped = new_obj->texture;

//printf("GOT it !\n"); fflush(stdout);
	return new_obj;
}

bool	WED_ResourceMgr::GetObjRelative(const string& obj_path, const string& parent_path, XObj8 const *& obj)
{
/* This is ised to resolve objects referenced inside other non-obj assets like .agp, .fac or .str
   These can be either vpaths or paths relative to the art assets location.
   If it a vpath - its got to be know to th library manager.
*/
	//printf("GetObjRel '%s', '%s'\n", obj_path.c_str(), parent_path.c_str());
	if(mLibrary->GetResourcePath(obj_path).size())
	{
		if(GetObj(obj_path,obj))
		{
			//printf("GetObjRel GotObj via vpath '%s'\n", obj_path.c_str());
			return true;
		}
	}
	/* Try if its a valid relative path */
	string apath = FILE_get_dir_name(mLibrary->GetResourcePath(parent_path)) + obj_path;
	auto i = mObj.find(apath);
	if(i != mObj.end())
	{
		obj = i->second.front();
		return true;
	}

//printf("GetObjRel trying via abspath '%s'\n", apath.c_str());
	XObj8 * new_obj = LoadObj(apath);
	if(!new_obj) return false;

	mObj[apath].push_back(new_obj);  // store the thing under its absolute path name
	obj = new_obj;
	return true;
}

bool	WED_ResourceMgr::GetObj(const string& vpath, XObj8 const *& obj, int variant)
{
	if(toupper(vpath[vpath.size()-3]) != 'O') return false;   // save time by not trying to load .agp's

//printf("GetObj %s' V=%d\n", path.c_str(), variant);
	auto i = mObj.find(vpath);
	int first_needed = 0;
	if(i != mObj.end())
	{
		if(variant < i->second.size())
		{
			obj = i->second[variant];
			return true;
		}
		else
			first_needed = i->second.size();
	}

	DebugAssert(variant < mLibrary->GetNumVariants(vpath));
	
//printf("GetObj trying to load '%s' V=%d/%d\n", path.c_str(), variant, n_variants);
	for (int v = first_needed; v <= variant; ++v)  // load only the variants we need but don't have yet
	{
		string p = mLibrary->GetResourcePath(vpath,v);
//	if (!p.size()) p = mLibrary->CreateLocalResourcePath(path);
		{
			XObj8 * new_obj = LoadObj(p);
			if(new_obj)
			{
				mObj[vpath].push_back(new_obj);
				obj = new_obj;
			}
			else
			{
	//			obj = nullptr;
				return false;
			}
		}
	}
	return true;
}

bool 	WED_ResourceMgr::SetPolUV(const string& path, Bbox2 box)
{
	auto i = mPol.find(path);
	if(i != mPol.end())
	{
		i->second.mUVBox = box;
		return true;
	}
	else
		return false;
}

bool	WED_ResourceMgr::GetLin(const string& path, lin_info_t const *& info)
{
	auto i = mLin.find(path);
	if(i != mLin.end())
	{
		info = &i->second;
		return true;
	}
	
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
	
	lin_info_t * out_info = new lin_info_t;

	out_info->base_tex.clear();
	out_info->scale_s=100;
	out_info->scale_t=100;
	float tex_width = 1024;
	out_info->s1.clear();
	out_info->sm.clear();
	out_info->s2.clear();
	out_info->rgb[0] = 0.75;   // taxi line yellow
	out_info->rgb[1] = 0.6;
	out_info->rgb[2] = 0.15;

	while(!MFS_done(&s))
	{
		if (MFS_string_match(&s,"TEXTURE", false))
		{
			MFS_string(&s,&out_info->base_tex);
		}
		else if (MFS_string_match(&s,"SCALE", false))
		{
			out_info->scale_s = MFS_double(&s);
			out_info->scale_t = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"TEX_WIDTH", false))
		{
			tex_width = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"PREVIEW_RGB", false))
		{
			out_info->rgb[0] = MFS_double(&s);
			out_info->rgb[1] = MFS_double(&s);
			out_info->rgb[2] = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"S_OFFSET", false))
		{
			float ly = MFS_double(&s);
			float s1 = MFS_double(&s);
			float sm = MFS_double(&s);
			float s2 = MFS_double(&s);
			if (s2 > s1 && s2 > sm)
			{
				out_info->s1.push_back(s1/tex_width);
				out_info->sm.push_back(sm/tex_width);
				out_info->s2.push_back(s2/tex_width);
			}
		}
		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(lin);
	
	if (out_info->s1.size() < 1) 
		return false;

	out_info->eff_width = out_info->scale_s * ( out_info->s2[0] - out_info->s1[0] - 4 / tex_width ); // assume 2 transparent pixels on each side

	process_texture_path(p,out_info->base_tex);
	mLin[path] = *out_info;
	info = out_info;
	
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

bool	WED_ResourceMgr::GetStr(const string& path, str_info_t const *& info)
{
	auto i = mStr.find(path);
	if(i != mStr.end())
	{
		info = &i->second;
		return true;
	}

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
	
	str_info_t * out_info = new str_info_t;

	out_info->offset = 0.0;
	out_info->rotation = 0.0;

	while(!MFS_done(&s))
	{
		if (MFS_string_match(&s,"OFFSET", false))
		{
			out_info->offset = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"OBJECT", false))
		{
			out_info->rotation = MFS_double(&s);    // we dont randomize the headings, but if a rotation is specified, we obey that
			int ignore = MFS_double(&s);
			string obj_res;
			MFS_string(&s,&obj_res);
printf("str res path '%s'\n",obj_res.c_str());
			clean_rpath(obj_res);
//			obj_res = FILE_get_dir_name(p) + obj_res;
//			FILE_case_correct( (char *) obj_res.c_str());
//printf("str case corr '%s'\n",obj_res.c_str());
			out_info->objs.push_back(obj_res);
		}
		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(str);
	mStr[path] = *out_info;
	info = out_info;
	return true;
}


bool	WED_ResourceMgr::GetPol(const string& path, pol_info_t const*& info)
{
	auto i = mPol.find(path);
	if(i != mPol.end())
	{
		info = &i->second;
		return true;
	}
	
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * file = MemFile_Open(p.c_str());
	if(!file) return false;

	MFScanner	s;
	MFS_init(&s, file);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"DRAPED_POLYGON",NULL))
	{
		MemFile_Close(file);
		return false;
	}

	pol_info_t * pol = new pol_info_t;
	
	pol->mSubBoxes.clear();
	pol->mUVBox = Bbox2();

	pol->base_tex.clear();
	pol->hasDecal=false;
	pol->proj_s=1000;
	pol->proj_t=1000;
	pol->kill_alpha=false;
	pol->wrap=false;

	while(!MFS_done(&s))
	{
		if (MFS_string_match(&s,"TEXTURE", false))
		{
			MFS_string(&s,&pol->base_tex);
			pol->wrap=true;
		}
		else if (MFS_string_match(&s,"SCALE", false))
		{
			pol->proj_s = MFS_double(&s);
			pol->proj_t = MFS_double(&s);
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
				pol->mSubBoxes.push_back(Bbox2(s1,t1,s2,t2));
			}
		}
		else if (MFS_string_match(&s,"TEXTURE_NOWRAP", false))
		{
			MFS_string(&s,&pol->base_tex);
			pol->wrap=false;
		}
		else if (MFS_string_match(&s,"DECAL_LIB", true))
		{
			pol->hasDecal=true;
		}
		else if (MFS_string_match(&s,"NO_ALPHA", true))
		{
			pol->kill_alpha=true;
		}
		else if (MFS_string_match(&s,"LAYER_GROUP",false))
		{
			MFS_string(&s,&pol->group);
			pol->group_offset = MFS_int(&s);
		}

		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(file);

	process_texture_path(p,pol->base_tex);
	mPol[path] = *pol;
	info = pol;
	
	return true;
}

void WED_ResourceMgr::WritePol(const string& abspath, const pol_info_t& out_info)
{
	FILE * fi = fopen(abspath.c_str(), "w");
	if(!fi)	return;
	fprintf(fi,"A\n850\nDRAPED_POLYGON\n\n");
	fprintf(fi,"# Created by WED " WED_VERSION_STRING "\n");
	fprintf(fi,out_info.wrap ? "TEXTURE %s\n" : "TEXTURE_NOWRAP %s\n", out_info.base_tex.c_str());
	fprintf(fi,"SCALE %.1lf %.1lf\n",out_info.proj_s,out_info.proj_t);
	fprintf(fi,"LOAD_CENTER %lf %lf %.1f %d", out_info.latitude, out_info.longitude,out_info.height_Meters,out_info.ddsHeight_Pxls);
	if(out_info.kill_alpha)
		fprintf(fi,"NO_ALPHA\n");
	if(!out_info.group.empty())
		fprintf(fi,"LAYER_GROUP %s %d\n",out_info.group.c_str(), out_info.group_offset);
//	if(has_decal)
//		fprintf(fi,"DECAL_LIB lib/g10/decals/grass_and_stony_dirt_1.dcl");
	fclose(fi);
	gPackageMgr->Rescan(true);  // a full rescan of LibraryMgr can take a LOT of time on large systems. Find a way to only add/update this one polygon.
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

#define FAIL(s) { printf("%s: %s\n",vpath.c_str(),s); return false; }

bool	WED_ResourceMgr::GetFac(const string& vpath, fac_info_t const *& info, int variant)
{
	auto i = mFac.find(vpath);
	int first_needed = 0;
	if(i != mFac.end())
	{
		if(variant < i->second.size())
		{
			info = i->second[variant];
			return true;
		}
		else
			first_needed = i->second.size();
	}
	
	DebugAssert(variant < mLibrary->GetNumVariants(vpath));

	for(int v = first_needed; v <=  variant; ++v)
	{
		string p = mLibrary->GetResourcePath(vpath, v);

		MFMemFile * file = MemFile_Open(p.c_str());
		if(!file) return false;
		
		fac_info_t * fac = new fac_info_t;
		
		MFScanner	s;
		MFS_init(&s, file);

		int vers, versions[] = { 800, 1000, 0 };
		if((vers = MFS_xplane_header(&s,versions,"FACADE",NULL)) == 0)
		{
			MemFile_Close(file);
			return false;
		}
		else
			fac->is_new = (vers == 1000);
			
		xflt scale_s = 1.0f, scale_t = 1.0f;
		bool roof_section = false;
		bool not_nearest_lod = false;
		REN_facade_template_t * tpl = NULL;

		while(!MFS_done(&s))
		{
			if (MFS_string_match(&s,"LOD", false))
			{
				not_nearest_lod = (MFS_double(&s) > 0.1);   // skip all info on the far out LOD's
			}
			else if(not_nearest_lod)
			{	
				MFS_string_eol(&s,NULL);
				continue;	
			}
			else if (MFS_string_match(&s,"SHADER_ROOF", true))
			{
				roof_section = true;
//				fac->has_roof = true;
			}
			else if (MFS_string_match(&s,"SHADER_WALL", true))
			{
				roof_section = false;
			}
			else if (MFS_string_match(&s,"TEXTURE", false))
			{
				string tex;
				MFS_string(&s,&tex);
				clean_rpath(tex);

				if (roof_section)
					fac->roof_tex = tex;
				else
					fac->wall_tex = tex;
			}
			else if (MFS_string_match(&s,"FACADE_SCRAPER",false))
			{
				REN_facade_scraper_t scr;
				scr.min_agl = MFS_double(&s);
				scr.max_agl = MFS_double(&s);
				scr.step_agl = MFS_double(&s);
				scr.floors = MFS_double(&s);
				fac->scrapers.push_back(scr);
			}
			else if (MFS_string_match(&s,"FACADE_SCRAPER_MODEL",false))
			{
				REN_facade_scraper_t::tower_t choice;
				if(fac->scrapers.empty())
					FAIL("Cannot have FACADE_SCRAPER_MODEL without FACADE_SCRAPER.")
				string file;
				MFS_string(&s,&file);
				clean_rpath(file);
				choice.base_obj = file;
				if (choice.base_obj.empty())
					FAIL("Could not load base OBJ for FACADE_SCRAPER_MODEL")
				MFS_string(&s,&file);
				if(file != "-")
				{
					clean_rpath(file);
					choice.towr_obj = file;
					if (choice.towr_obj.empty())
						FAIL("Could not load tower OBJ for FACADE_SCRAPER_MODEL")
				}
				/* skip scanning the pins for now
				while(m.TXT_has_word())
					choice.pins.push_back(m.TXT_flt_scan());
				if(choice.pins.size() % 1)
				{
					FAIL("Odd numberof pins")
				}	*/
				fac->scrapers.back().choices.push_back(choice);
			}
			else if (MFS_string_match(&s,"FACADE_SCRAPER_MODEL_OFFSET",false))
			{
				if(fac->scrapers.empty())
					FAIL("Cannot have FACADE_SCRAPER_MODEL_OFFSET without FACADE_SCRAPER.")
				REN_facade_scraper_t::tower_t choice;
				string file;
				choice.base_xzr[0] = MFS_double(&s);
				choice.base_xzr[1] = MFS_double(&s);
				choice.base_xzr[2] = MFS_double(&s);
				MFS_string(&s,&file);
				clean_rpath(file);
				MFS_int(&s); MFS_int(&s);  // skip showlevel restrictions
				choice.base_obj = file;
				if (choice.base_obj.empty())
					FAIL("Could not load base OBJ for FACADE_SCRAPER_MODEL")

				choice.towr_xzr[0] = MFS_double(&s);
				choice.towr_xzr[1] = MFS_double(&s);
				choice.towr_xzr[2] = MFS_double(&s);
				MFS_string(&s,&file);
				MFS_int(&s); MFS_int(&s);  // skip showlevel restrictions
				if(file != "-")
				{
					clean_rpath(file);
					choice.towr_obj = file;
				}
/*				while(m.TXT_has_word())
					choice.pins.push_back(m.TXT_flt_scan());
				if(choice.pins.size() % 1)
				{
					FAIL("Odd numberof pins")
				} */
				fac->scrapers.back().choices.push_back(choice);
			}
// scraper pad command is not implemented
			else if (MFS_string_match(&s,"WALL",false))
			{
				roof_section = false;

				double min_width = MFS_double(&s);
				double max_width = MFS_double(&s);

				if(!fac->is_new)
				{
					fac->walls.push_back(FacadeWall_t());
					fac->wallName.push_back(string("#") + to_string(fac->walls.size()));
				}
				else
				{
					fac->floors.back().walls.push_back(REN_facade_wall_t());
					fac->floors.back().walls.back().filters.push_back(REN_facade_wall_filter_t());
					fac->floors.back().walls.back().filters.back().min_width = min_width;
					fac->floors.back().walls.back().filters.back().max_width = max_width;
					fac->floors.back().walls.back().filters.back().min_heading = MFS_double(&s);
					fac->floors.back().walls.back().filters.back().max_heading = MFS_double(&s);
					if(fac->floors.size() == 1)
					{
						string buf;	MFS_string(&s,&buf);
						if(!buf.empty()) 
							fac->wallName.push_back(buf);
						else
							fac->wallName.push_back(string("#") + to_string(fac->floors.back().walls.size()));
					}
				}
				char c[64];
				snprintf(c, 64, "w=%.0f to %.0f%c", min_width / (gIsFeet ? 0.3048 : 1.0 ), max_width / (gIsFeet ? 0.3048 : 1.0 ), gIsFeet ? '\'' : 'm') ;
				fac->wallUse.push_back(c);
			} 
			else if (MFS_string_match(&s,"RING", false))
			{
				fac->is_ring = MFS_int(&s) > 0;
			}
			else if (MFS_string_match(&s,"TWO_SIDED", false))
			{
				fac->two_sided = MFS_int(&s) > 0;
			}
			else if(!fac->is_new)  // type 1 facades
			{
				if (MFS_string_match(&s,"SCALE", false))
				{
					fac->walls.back().x_scale = MFS_double(&s);
					fac->walls.back().y_scale = MFS_double(&s);
					
					if(fac->walls.back().x_scale < 0.01 || fac->walls.back().y_scale < 0.01)
						printf("facade has a scale less than 1 cm per texture. This is probably a bad facade.\n");
				}
				else if (MFS_string_match(&s,"ROOF_SLOPE", false))
				{
					fac->walls.back().roof_slope = MFS_double(&s);
					
					if(fac->walls.back().roof_slope >= 90.0 || 
						fac->walls.back().roof_slope <= -90.0)
					{
						fac->tex_correct_slope = true;
					}
					string buf;	MFS_string(&s,&buf);
					
					if(buf == "SLANT")
					{
						fac->tex_correct_slope = true;
					}
				}
				else if (MFS_string_match(&s,"BOTTOM",false))
				{
					float f1 = MFS_double(&s) * scale_t;
					float f2 = MFS_double(&s) * scale_t;
					fac->walls.back().t_floors.push_back(pair<float, float>(f1,f2));
					++fac->walls.back().bottom; 
				} 
				else if (MFS_string_match(&s,"MIDDLE",false))
				{
					float f1 = MFS_double(&s) * scale_t;
					float f2 = MFS_double(&s) * scale_t;
					fac->walls.back().t_floors.push_back(pair<float, float>(f1,f2));
					++fac->walls.back().middle; 
				} 
				else if (MFS_string_match(&s,"TOP",false))
				{
					float f1 = MFS_double(&s) * scale_t;
					float f2 = MFS_double(&s) * scale_t;
					fac->walls.back().t_floors.push_back(pair<float, float>(f1,f2));
					++fac->walls.back().top; 
				} 
				else if (MFS_string_match(&s,"LEFT",false))
				{
					float f1 = MFS_double(&s) * scale_s;
					float f2 = MFS_double(&s) * scale_s;
					fac->walls.back().s_panels.push_back(pair<float, float>(f1,f2));
					++fac->walls.back().left; 
				} 
				else if (MFS_string_match(&s,"CENTER",false))
				{
					float f1 = MFS_double(&s) * scale_s;
					float f2 = MFS_double(&s) * scale_s;
					fac->walls.back().s_panels.push_back(pair<float, float>(f1,f2));
					++fac->walls.back().center; 
				} 
				else if (MFS_string_match(&s,"RIGHT",false))
				{
					float f1 = MFS_double(&s) * scale_s;
					float f2 = MFS_double(&s) * scale_s;
					fac->walls.back().s_panels.push_back(pair<float, float>(f1,f2));
					++fac->walls.back().right; 
				} 
				else if (MFS_string_match(&s,"ROOF", false))
				{
					fac->roof_s.push_back(MFS_double(&s) * scale_s);
					fac->roof_t.push_back(MFS_double(&s) * scale_t);
					fac->has_roof = true;
				}
				else if (MFS_string_match(&s,"ROOF_SCALE", false))
				{
					fac->roof_st[0] = MFS_double(&s) * scale_s;
					fac->roof_st[1] = MFS_double(&s) * scale_t;
					xflt s_ctr = MFS_double(&s) * scale_s;
					xflt t_ctr = MFS_double(&s) * scale_t;
					fac->roof_st[2] = MFS_double(&s) * scale_s;
					fac->roof_st[3] = MFS_double(&s) * scale_t;
					xflt rsx = MFS_double(&s);
					xflt rsy = MFS_double(&s);
					xflt s_rat = (s_ctr - fac->roof_st[0]) / (fac->roof_st[2] - fac->roof_st[0]);  // fraction of tex below/left center point
					xflt t_rat = (t_ctr - fac->roof_st[1]) / (fac->roof_st[3] - fac->roof_st[1]);
					fac->roof_ab[0] = -rsx * s_rat;          // number of meters that are below/left of center point, always negative
					fac->roof_ab[1] = -rsy * t_rat;
					fac->roof_ab[2] = fac->roof_ab[0] + rsx;    // number of meters that are above/right of center point
					fac->roof_ab[3] = fac->roof_ab[1] + rsy;
					fac->has_roof = true;
				}
				else if (MFS_string_match(&s,"BASEMENT_DEPTH", false))
				{
					fac->walls.back().basement = MFS_double(&s) * scale_t;
				}
				else if (MFS_string_match(&s,"TEX_SIZE",false))
				{
					scale_s = MFS_double(&s); if(scale_s) scale_s = 1.0f / scale_s;
					scale_t = MFS_double(&s); if(scale_t) scale_t = 1.0f / scale_t;
				}
				else if (MFS_string_match(&s,"FLOORS_MIN", false))
				{
					fac->min_floors = MFS_double(&s);
				}
				else if (MFS_string_match(&s,"FLOORS_MAX", false))
				{
					fac->max_floors = MFS_double(&s);
				}
				else if (MFS_string_match(&s,"DOUBLED", false))
				{
					fac->doubled = MFS_int(&s) > 0;
				}

			}
			else  // type 2 facades
			{
				if(MFS_string_match(&s,"OBJ", false))
				{
					string file;
					MFS_string(&s,&file);
					clean_rpath(file);
					fac->objs.push_back(file);
				}
				else if(MFS_string_match(&s,"FLOOR", false))
				{
					fac->floors.push_back(REN_facade_floor_t());
					fac->floors.back().roof_surface = 0;
					MFS_string(&s,&fac->floors.back().name);
				}
				else if(MFS_string_match(&s,"SEGMENT", false))
				{
					fac->floors.back().templates.push_back(REN_facade_template_t());
					tpl = &fac->floors.back().templates.back();
				}
				else if(MFS_string_match(&s,"SEGMENT_CURVED", false))
				{
//					fac->floors.back().templates_curved.push_back(REN_facade_template_t());
//					tpl = &fac->floors.back().templates_curved.back();
					tpl = NULL;
				}
				else if(tpl && MFS_string_match(&s,"MESH", false))
				{
					tpl->meshes.push_back(REN_facade_template_t::mesh());
					MFS_double(&s);
					MFS_double(&s);
					MFS_double(&s);
					int num_vert = MFS_double(&s);
					int num_idx = MFS_double(&s);
					tpl->meshes.back().xyz.reserve(3*num_vert);
					tpl->meshes.back().uv.reserve(2*num_vert);
					tpl->meshes.back().idx.reserve(num_idx);
				}
				else if(tpl && MFS_string_match(&s,"VERTEX", false))
				{
					tpl->meshes.back().xyz.push_back(MFS_double(&s));
					tpl->meshes.back().xyz.push_back(MFS_double(&s));
					tpl->meshes.back().xyz.push_back(MFS_double(&s));
					MFS_double(&s);
					MFS_double(&s);
					MFS_double(&s);
					tpl->meshes.back().uv.push_back(MFS_double(&s));
					tpl->meshes.back().uv.push_back(MFS_double(&s));
				}
				else if(tpl && MFS_string_match(&s,"IDX", false))
				{
					while(MFS_has_word(&s))
						tpl->meshes.back().idx.push_back(MFS_int(&s));
				}
				else if(tpl && (MFS_string_match(&s,"ATTACH_DRAPED", false) || MFS_string_match(&s,"ATTACH_GRADED", false)))
				{
					tpl->objs.push_back(REN_facade_template_t::obj());
					tpl->objs.back().idx =MFS_int(&s);
					tpl->objs.back().xyzr[0] =MFS_double(&s);
					tpl->objs.back().xyzr[1] =MFS_double(&s);
					tpl->objs.back().xyzr[2] =MFS_double(&s);
					tpl->objs.back().xyzr[3] =MFS_double(&s);
				}
				else if(MFS_string_match(&s,"SPELLING", false))
				{
					fac->floors.back().walls.back().spellings.push_back(UTL_spelling_t());
					while(MFS_has_word(&s))
						fac->floors.back().walls.back().spellings.back().indices.push_back(MFS_int(&s));
				}
				else if(MFS_string_match(&s,"ROOF_HEIGHT", false))
				{
					while(MFS_has_word(&s))
						fac->floors.back().roofs.push_back(REN_facade_roof_t(MFS_double(&s)));
					fac->has_roof = true;
				}
				else if(MFS_string_match(&s,"ROOF_SCALE", false))
				{
					fac->roof_scale_s = MFS_double(&s);
					fac->roof_scale_t = MFS_double(&s);
					if (fac->roof_scale_t == 0.0) fac->roof_scale_t = fac->roof_scale_s;
				}
				else if(MFS_string_match(&s,"NO_ROOF_MESH", true))
				{
					fac->noroofmesh = true;
				}
				else if(MFS_string_match(&s,"NO_WALL_MESH", true))
				{
					fac->nowallmesh = true;
				}
				else if(MFS_string_match(&s,"ROOF_OBJ", false))
				{
					xint idx = MFS_int(&s);
					xflt s_coord = MFS_double(&s) / fac->roof_scale_s;
					xflt t_coord = MFS_double(&s) / fac->roof_scale_t;
					// xint show_lo = MFS_int(&s);
					// xint show_hi = MFS_int(&s);
					REN_facade_roof_t::robj o = { s_coord, t_coord, 0.0, idx };
					if(fac->floors.empty() || fac->floors.back().roofs.empty())
						FAIL("This facade uses a roof object that is not inside a roof.")
					else
						fac->floors.back().roofs.back().roof_objs.push_back(o);
				}
				else if(MFS_string_match(&s,"ROOF_OBJ_HEADING", false))
				{
					xint idx = MFS_int(&s);
					xflt s_coord = MFS_double(&s) / fac->roof_scale_s;
					xflt t_coord = MFS_double(&s) / fac->roof_scale_t;
					xflt r = MFS_double(&s);
					// xint show_lo = MFS_int(&s);
					// xint show_hi = MFS_int(&s);
					REN_facade_roof_t::robj o = { s_coord, t_coord, r, idx };
					if(fac->floors.empty() || fac->floors.back().roofs.empty())
						FAIL("This facade uses a roof object that is not inside a roof.")
					else
						fac->floors.back().roofs.back().roof_objs.push_back(o);
				}
			}
			MFS_string_eol(&s,NULL);
		}
		MemFile_Close(file);
		
//printf("f=%ld, t=%ld w=%ld\n",fac->floors.size(), fac->floors.back().templates.size(),	fac->floors.back().walls.size());

		if(fac->is_new)
		{
			vector<int> heights;
			for(auto& f : fac->floors)
			{
				for(auto& t : f.templates)
				{
					xflt xyz_min[3] = {  9.9e9,  9.9e9,  9.9e9 };
					xflt xyz_max[3] = { -9.9e9, -9.9e9, -9.9e9 };

					for(auto m : t.meshes)
						for(int i = 0; i < m.xyz.size(); i +=3 )
						{
							xflt * p = &m.xyz[i];
							xyz_min[0] = min(xyz_min[0], p[0]);
							xyz_max[0] = max(xyz_max[0], p[0]);
//							xyz_min[1] = min(xyz_min[1], p[1]);
//							xyz_max[1] = max(xyz_max[1], p[1]);
							xyz_min[2] = min(xyz_min[2], p[2]);
							xyz_max[2] = max(xyz_max[2], p[2]);
						}
					t.bounds[0] = xyz_max[0]- xyz_min[0];   // to IF ring=0 objects that aren't true verical fences, like jetways
					t.bounds[1] = xyz_max[0];               // to ID walls that protrude outwards from roofs
					t.bounds[2] = xyz_max[2] - xyz_min[2];  // used all thoughout to scale segment widths

					// normalize z-direction coordinates
					for(auto& m : t.meshes)
						for(int i = 0; i < m.xyz.size(); i +=3 )
							m.xyz[i+2] = interp(xyz_min[2], 0.0, xyz_max[2], 1.0, m.xyz[i+2])-1;
				}

				for(auto& w : f.walls)
				{
					for(auto& s : w.spellings)
					{
						s.total = 0.0f;
						for(auto b : s.indices)
						{
							dev_assert(intrange(b,0,f.templates.size()-1));
							s.total += f.templates[b].bounds[2];
							s.widths.push_back(f.templates[b].bounds[2]);
						}
					}
					sort(w.spellings.begin(), w.spellings.end());
				}
			}
			if(fac->noroofmesh) fac->has_roof = false;
		}
		process_texture_path(p,fac->wall_tex);
		process_texture_path(p,fac->roof_tex);
		
		height_desc_for_facade(*fac, fac->h_range);

		mFac[vpath].push_back(fac);
		info = fac;
	}
	return true;
}

inline void	do_rotate(int n, float& io_x, float& io_y)
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

#define TPR 6           // # of trees shown in a row

struct tree_t {
	float s,t,w,y; 		// texture coordinates of tree
	float o;            // offset of tree center line (where the quads inersect)
	float pct;          // relative occurence percentage for this tree
	float hmin,hmax;    // height range for this tree in meters
	int q;				// number of quads the tree is constructed of
};

bool	WED_ResourceMgr::GetFor(const string& path, XObj8 const *& obj)
{
	auto i = mFor.find(path);
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
	XObj8 * new_obj = new XObj8;
	XObjCmd8 cmd;

	new_obj->texture = tex;
	process_texture_path(p, new_obj->texture);
	
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

			new_obj->geo_tri.append(pt);
			pt[0] = t_x + x*(1.0-tree[i].o/tree[i].w);
			pt[2] = t_y + z*(1.0-tree[i].o/tree[i].w);
			pt[6] = (tree[i].s+tree[i].w)/scale_x;
			new_obj->geo_tri.append(pt);
			pt[1] = t_h;
			pt[7] = (tree[i].t+tree[i].y)/scale_y;
			new_obj->geo_tri.append(pt);
			pt[0] = t_x - x*(tree[i].o/tree[i].w);
			pt[2] = t_y - z*(tree[i].o/tree[i].w);
			pt[6] = tree[i].s/scale_x;
			new_obj->geo_tri.append(pt);
		}
	}
	// set dimension
	new_obj->geo_tri.get_minmax(new_obj->xyz_min,new_obj->xyz_max);

	// "IDX "
	int seq[6] = {0,1,2,0,2,3};
	for (int i = 0; i < 6*quads; ++i)
		new_obj->indices.push_back(4*(i/6)+seq[i%6]);

	// "ATTR_LOD"
	new_obj->lods.push_back(XObjLOD8());
	new_obj->lods.back().lod_near = 0;
	new_obj->lods.back().lod_far  = 1000;

	// "ATTR_no_cull"
	cmd.cmd = attr_NoCull;
	new_obj->lods.back().cmds.push_back(cmd);

	// "TRIS ";
	cmd.cmd = obj8_Tris;
	cmd.idx_offset = 0;
	cmd.idx_count  = 6*quads;
	new_obj->lods.back().cmds.push_back(cmd);

	mFor[path] = new_obj;
    // only problem is that the texture path contain spaces -> obj reader can not read that
    // but still valuable for checking the values/structure
//    XObj8Write(mLibrary->CreateLocalResourcePath("forest_preview.obj").c_str(), *obj);

	obj = new_obj;
	return true;
}

bool	WED_ResourceMgr::GetAGP(const string& path, agp_t& out_info)
{
	auto i = mAGP.find(path);
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
				process_texture_path(p,out_info.mesh_tex);
			}
			else
			{
				out_info.base_tex = tex;
				process_texture_path(p,out_info.base_tex);
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
			out_info.objs.back().z = 0.0;
			out_info.objs.back().name = obj_paths[MFS_int(&s)];
			out_info.objs.back().show_lo = MFS_int(&s);
			out_info.objs.back().show_hi = MFS_int(&s);
		}
		else if(MFS_string_match(&s,"OBJ_DELTA",false))
		{
			out_info.objs.push_back(agp_t::obj());
			out_info.objs.back().x = MFS_double(&s) * tex_s * tex_x;
			out_info.objs.back().y = MFS_double(&s) * tex_t * tex_y;
			out_info.objs.back().r = MFS_double(&s);
			out_info.objs.back().z = MFS_double(&s);
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

	mAGP[path] = out_info;
	return true;
}

#if ROAD_EDITING
bool	WED_ResourceMgr::GetRoad(const string& path, road_info_t& out_info)
{
	auto i = mRoad.find(path);
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