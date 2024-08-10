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
#include "FileUtils.h"
#include "WED_PackageMgr.h"
#include "CompGeomDefs2.h"
#include "MathUtils.h"
#include "BitmapUtils.h"

#include "DEMDefs.h"
#include "WED_OrthoExport.h"

#if IBM
#define DIR_CHAR '\\'
#define DIR_STR "\\"
#else
#define DIR_CHAR '/'
#define DIR_STR "/"
#endif

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
	string parent(FILE_get_dir_name(path_of_obj));
	path_of_tex = FILE_get_file_name_wo_extensions(path_of_tex);
	WED_clean_rpath(path_of_tex);

	while (path_of_tex.length() > 2 && path_of_tex[0] == '.' && path_of_tex[1] == '.')
	{
		path_of_tex.erase(0,2);
		parent.erase(parent.find_last_of(DIR_CHAR,parent.length()-2));
	}
	parent += path_of_tex;

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
	for(auto& i : mObj)
		for(auto j : i.second)
			delete j;
	mObj.clear();

	mPol.clear();
	mLin.clear();

	for (auto& i : mFor)
	{
		if(i.second.preview != nullptr) delete i.second.preview;
		if(i.second.preview_3d != nullptr) delete i.second.preview_3d;
	}
	mFor.clear();

	mFac.clear();
	mStr.clear();
	mAGP.clear();
	mDem.clear();
}

void	WED_ResourceMgr::Purge(const string& vpath)
{
	auto i = mObj.find(vpath);
	if (i != mObj.end())
	{
		for (auto j : (*i).second)
			delete j;
		mObj.erase(i);
	}
}

bool	WED_ResourceMgr::GetAllInDir(const string& vdir, vector<pair<string, int> >& vpaths)
{
	vector<string> names;
	mLibrary->GetResourceChildren(vdir, pack_All, names, true);

	for (auto& n : names)
		vpaths.push_back(make_pair(n, mLibrary->GetResourceType(n)));

	return names.size();
}

bool	WED_ResourceMgr::GetDem(const string& path, dem_info_t const*& info)
{
	auto i = mDem.find(path);
	if (i != mDem.end())
	{
		info = &i->second;
		return true;
	}
	dem_info_t* out_info = &mDem[path];

	out_info->mWidth = 0;
	out_info->mHeight = 0;

	string rpath = mLibrary->CreateLocalResourcePath(path);
	if (WED_ExtractGeoTiff(*out_info, rpath.c_str(), 0))
	{
		info = out_info;
		return true;
	}
	else
		return false;
}


XObj8 * WED_ResourceMgr::LoadObj(const string& abspath)
{
	XObj8 * new_obj = new XObj8;
	if(!XObj8Read(abspath.c_str(),*new_obj))
	{
		delete new_obj;
		return nullptr;
	}
	for (auto& l : new_obj->lods)  // balance begin/end_annimation in broken assets to fix display artefacts due to imbalanced glPush/PopMatrix()
	{
		int num_anims = 0;
		for (const auto& c : l.cmds)
		{
			if (c.cmd == anim_Begin) num_anims++;
			else if (c.cmd == anim_End) num_anims--;

		}
		while (num_anims > 0)
		{
			l.cmds.push_back(XObjCmd8());
			l.cmds.back().cmd = anim_End;
			num_anims--;
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

	return new_obj;
}

bool	WED_ResourceMgr::GetObjRelative(const string& obj_path, const string& parent_path, XObj8 const *& obj)
{
/* This is ised to resolve objects referenced inside other non-obj assets like .agp, .fac or .str
   These can be either vpaths or paths relative to the art assets location.
   If it a vpath - its got to be known to the library manager.
*/
	//printf("GetObjRel '%s', '%s'\n", obj_path.c_str(), parent_path.c_str());
	if(mLibrary->GetResourcePath(obj_path).size())
	{
		if(GetObj(obj_path,obj))
		{
			return true;
		}
	}
	/* Try if its a valid relative path */
	string apath = FILE_get_dir_name(mLibrary->GetResourcePath(parent_path)) + obj_path;

	for (auto& a : apath)
#if IBM
		if (a == '/')
#else
		if (a == '\\')
#endif
			a = DIR_CHAR;

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

	info = nullptr;
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * lin = MemFile_Open(p.c_str());
	if(!lin) return false;

	MFScanner	s;
	MFS_init(&s, lin);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"LINE_PAINT",NULL))
	{
		LOG_MSG("E/RES unsupported version or header in %s\n", p.c_str());
		MemFile_Close(lin);
		return false;
	}

	lin_info_t * out_info = &mLin[path];
	info = out_info;

	out_info->base_tex.clear();
	out_info->scale_s=100;
	out_info->scale_t=100;
	float tex_width = 1024;
	float tex_height = 1024;
	out_info->s1.clear();
	out_info->sm.clear();
	out_info->s2.clear();
	out_info->rgb[0] = out_info->rgb[1] = out_info->rgb[2] = 0.0;
	out_info->start_caps.clear();
	out_info->end_caps.clear();
	out_info->align = 0;
	out_info->hasDecal = false;

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
		else if (MFS_string_match(&s,"TEX_HEIGHT", false))
		{
			tex_height = MFS_double(&s);
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
			if (s2 > s1)
			{
				out_info->s1.push_back(s1/tex_width);
				out_info->sm.push_back(sm/tex_width);
				out_info->s2.push_back(s2/tex_width);
			}
		}
		else if (MFS_string_match(&s,"START_CAP", false))
		{
			out_info->start_caps.push_back(lin_info_t::caps());
			MFS_double(&s);
			out_info->start_caps.back().s1 = MFS_double(&s)/tex_width;
			out_info->start_caps.back().sm = MFS_double(&s)/tex_width;
			out_info->start_caps.back().s2 = MFS_double(&s)/tex_width;
			out_info->start_caps.back().t1 = MFS_double(&s)/tex_height;
			out_info->start_caps.back().t2 = MFS_double(&s)/tex_height;
		}
		else if (MFS_string_match(&s,"END_CAP", false))
		{
			out_info->end_caps.push_back(lin_info_t::caps());
			MFS_double(&s);
			out_info->end_caps.back().s1 = MFS_double(&s)/tex_width;
			out_info->end_caps.back().sm = MFS_double(&s)/tex_width;
			out_info->end_caps.back().s2 = MFS_double(&s)/tex_width;
			out_info->end_caps.back().t1 = MFS_double(&s)/tex_height;
			out_info->end_caps.back().t2 = MFS_double(&s)/tex_height;
		}
		else if (MFS_string_match(&s,"ALIGN", false))
		{
			out_info->align = MFS_double(&s);
		}
		else if (MFS_string_match(&s,"DECAL_LIB", true))
		{
			out_info->hasDecal=true;
		}
		else if (MFS_string_match(&s, "LAYER_GROUP", false))
		{
			MFS_string(&s, &out_info->group);
			out_info->group_offset = MFS_int(&s);
			if (abs(out_info->group_offset) > 5)
				LOG_MSG("E/Lin offset for LAYER_GROUP out of bounds in %s\n", p.c_str());
		}

		if (MFS_string_match(&s,"#wed_text", false))
			MFS_string_eol(&s,&out_info->description);
		else
			MFS_string_eol(&s,NULL);
	}
	MemFile_Close(lin);

	if (out_info->s1.size() < 1)
		return false;

	out_info->eff_width = out_info->scale_s * ( out_info->s2[0] - out_info->s1[0] - 2 / tex_width ); // assume one transparent pixels on each side

	process_texture_path(p,out_info->base_tex);
	return true;
}

bool	WED_ResourceMgr::GetStr(const string& path, str_info_t const *& info)
{
	auto i = mStr.find(path);
	if(i != mStr.end())
	{
		info = &i->second;
		return true;
	}

	info = nullptr;
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * str = MemFile_Open(p.c_str());
	if(!str) return false;

	MFScanner	s;
	MFS_init(&s, str);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"OBJECT_STRING",NULL))
	{
		LOG_MSG("E/RES unsupported version or header in %s\n", p.c_str());
		MemFile_Close(str);
		return false;
	}

	str_info_t * out_info = &mStr[path];
	info = out_info;

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
			WED_clean_vpath(obj_res);
			out_info->objs.push_back(obj_res);
		}

		if (MFS_string_match(&s,"#wed_text", false))
			MFS_string_eol(&s,&out_info->description);
		else
			MFS_string_eol(&s,NULL);
	}
	MemFile_Close(str);
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

	info = nullptr;
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * file = MemFile_Open(p.c_str());
	if(!file) return false;

	MFScanner	s;
	MFS_init(&s, file);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"DRAPED_POLYGON",NULL))
	{
		LOG_MSG("E/RES unsupported version or header in %s\n", p.c_str());
		MemFile_Close(file);
		return false;
	}

	pol_info_t * pol = &mPol[path];
	info = pol;

	pol->mSubBoxes.clear();
	pol->mUVBox = Bbox2();

	pol->base_tex.clear();
	pol->decal.clear();
	pol->proj_s=1000;
	pol->proj_t=1000;
	pol->kill_alpha=false;
	pol->wrap=false;

	int isTex = 0;

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
			MFS_string(&s, &pol->decal);
		}
		else if (MFS_string_match(&s,"NO_ALPHA", true))
		{
			pol->kill_alpha=true;
		}
		else if (MFS_string_match(&s,"LAYER_GROUP", false))
		{
			MFS_string(&s,&pol->group);
			pol->group_offset = MFS_int(&s);
			if(abs(pol->group_offset) > 5)
				LOG_MSG("E/Pol offset for LAYER_GROUP out of bounds in %s\n", p.c_str());
		}
		else if (MFS_string_match(&s,"SURFACE", false))
		{
			string tmp;
			MFS_string(&s,&tmp);
			if(tmp != "asphalt" && tmp != "concrete" && tmp != "grass" && tmp != "gravel" && tmp != "dirt" && tmp != "snow")
				LOG_MSG("E/Pol illegal SURFACE type in %s\n", p.c_str());
		}
		else if ((isTex = MFS_string_match(&s, "TEXTURE_TILE", false)) || MFS_string_match(&s, "RUNWAY_TILE", false))
		{
			string ctrl_tex;
			pol->tiling.tiles_x = MFS_int(&s);
			pol->tiling.tiles_y = MFS_int(&s);
			pol->tiling.pages_x = MFS_int(&s);
			pol->tiling.pages_y = MFS_int(&s);
			MFS_string(&s, &ctrl_tex);

			pol->tiling.rwy = isTex == 0;

			if (pol->tiling.tiles_x < 1 || pol->tiling.tiles_y < 1 || pol->tiling.pages_x < 1 || pol->tiling.pages_y < 1)
			{
				LOG_MSG("E/Pol impropper TEXTURE_TILE data in %s\n", p.c_str());
				pol->tiling.tiles_x = pol->tiling.tiles_y = pol->tiling.pages_x = pol->tiling.pages_y = 1;
			}
			else
			{
	        	if (pol->tiling.pages_x > 16) pol->tiling.pages_x = 16;  // we only ever preview a 3-4 multiples of a texture
				if (pol->tiling.pages_y > 16) pol->tiling.pages_y = 16;  // so we might well save ourselves some space

				pol->tiling.idx.reserve(pol->tiling.pages_x * pol->tiling.pages_y);

				if (ctrl_tex.size())
				{
					process_texture_path(p, ctrl_tex);
					ImageInfo img;
					if (LoadBitmapFromAnyFile(ctrl_tex.c_str(), &img))
					{
						LOG_MSG("E/Pol can't load control texture %s for %s\n", ctrl_tex.c_str(), p.c_str());
					}
					else
					{
						for (int y = 0; y < pol->tiling.pages_y; y++)
							for (int x = 0; x < pol->tiling.pages_x; x++)
							{
								int pixel = img.channels * (x + img.width * y);
								int red = *(img.data + pixel + (img.channels == 2 ? 0 : 2)); // img data is BGR
								int green = *(img.data + pixel + 1);
								red   *= (pol->tiling.tiles_x + 128) / 256;
								green *= (pol->tiling.tiles_y + 128) / 256;
								pol->tiling.idx.push_back(red);
								pol->tiling.idx.push_back(green);
							}
					}
					if (img.data) free(img.data);
				}
				else
					for (int x = 0; x < pol->tiling.pages_x; x++)
						for (int y = 0; y < pol->tiling.pages_y; y++)
						{
							pol->tiling.idx.push_back(pol->tiling.tiles_x * rand() / RAND_MAX);
							pol->tiling.idx.push_back(pol->tiling.tiles_y * rand() / RAND_MAX);
						}
			}
		}

		if (MFS_string_match(&s,"#wed_text", false))
			MFS_string_eol(&s,&pol->description);
		else
			MFS_string_eol(&s,NULL);
	}
	MemFile_Close(file);
	process_texture_path(p,pol->base_tex);
	return true;
}

void WED_ResourceMgr::WritePol(const string& abspath, const pol_info_t& out_info)
{
	FILE * fi = fopen(abspath.c_str(), "w");
	if(!fi)	return;
	fprintf(fi,"A\n850 Created by WED " WED_VERSION_STRING "\nDRAPED_POLYGON\n\n");
	fprintf(fi, "LOAD_CENTER %.5lf %.5lf %.1f %d\n", out_info.latitude, out_info.longitude, out_info.height_Meters, out_info.ddsHeight_Pxls);
	fprintf(fi,out_info.wrap ? "TEXTURE %s\n" : "TEXTURE_NOWRAP %s\n", out_info.base_tex.c_str());
	if (!out_info.decal.empty())
		fprintf(fi, "DECAL_LIB %s\n", out_info.decal.c_str());
	fprintf(fi,"SCALE %.1lf %.1lf\n",out_info.proj_s,out_info.proj_t);
	if(out_info.kill_alpha)
		fprintf(fi,"NO_ALPHA\n");
	if(!out_info.group.empty())
		fprintf(fi,"LAYER_GROUP %s %d\n",out_info.group.c_str(), out_info.group_offset);
	fclose(fi);
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

bool	WED_ResourceMgr::GetFac(const string& vpath, fac_info_t const *& info, int variant)
{
	auto i = mFac.find(vpath);
	int first_needed = 0;
	if(i != mFac.end())
	{
		if(variant < i->second.size())
		{
			info = &i->second[variant];
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

		MFScanner	s;
		MFS_init(&s, file);

		int vers, versions[] = { 800, 1000, 0 };
		if((vers = MFS_xplane_header(&s,versions,"FACADE",NULL)) == 0)
		{
			LOG_MSG("E/RES unsupported version or header in %s\n", p.c_str());
			MemFile_Close(file);
			return false;
		}

		mFac[vpath].push_back(fac_info_t());
		fac_info_t * fac = &mFac[vpath].back();
		info = fac;

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
					LOG_MSG("E/Fac FACADE_SCRAPER_MODEL without FACADE_SCRAPER in %s\n", p.c_str());
				string file;
				MFS_string(&s,&file);
				WED_clean_vpath(file);
				choice.base_obj = file;
				MFS_string(&s,&file);
				if(file != "-")
				{
					WED_clean_vpath(file);
					choice.towr_obj = file;
				}
				while(MFS_has_word(&s))
					choice.pins.push_back(MFS_double(&s));
				if(choice.pins.size() % 2)
					LOG_MSG("E/Fac odd numberof pin coordinates in %s\n", p.c_str());
				fac->scrapers.back().choices.push_back(choice);
			}
			else if (MFS_string_match(&s,"FACADE_SCRAPER_MODEL_OFFSET",false))
			{
				if(fac->scrapers.empty())
					LOG_MSG("E/Fac FACADE_SCRAPER_MODEL_OFFSET without FACADE_SCRAPER in %s\n", p.c_str());
				REN_facade_scraper_t::tower_t choice;
				string file;
				choice.base_xzr[0] = MFS_double(&s);
				choice.base_xzr[1] = MFS_double(&s);
				choice.base_xzr[2] = MFS_double(&s);
				MFS_string(&s,&file);
				WED_clean_vpath(file);
				MFS_int(&s); MFS_int(&s);  // skip showlevel restrictions
				choice.base_obj = file;

				choice.towr_xzr[0] = MFS_double(&s);
				choice.towr_xzr[1] = MFS_double(&s);
				choice.towr_xzr[2] = MFS_double(&s);
				MFS_string(&s,&file);
				MFS_int(&s); MFS_int(&s);  // skip showlevel restrictions
				if(file != "-")
				{
					WED_clean_vpath(file);
					choice.towr_obj = file;
				}
				while(MFS_has_word(&s))
					choice.pins.push_back(MFS_double(&s));
				if(choice.pins.size() % 2)
					LOG_MSG("E/Fac odd numberof pin coordinates in %s\n", p.c_str());
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
					MFS_double(&s);
					MFS_double(&s);
					string buf;	MFS_string(&s,&buf);
					if(!buf.empty())
						fac->wallName.push_back(buf);
					else
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
				snprintf(c, 64, "w=%.3g to %.3g%c", min_width / (gIsFeet ? 0.3048 : 1.0 ), max_width / (gIsFeet ? 0.3048 : 1.0 ), gIsFeet ? '\'' : 'm') ;
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
						LOG_MSG("E/Fac scale less than 1 cm per texture, probably bad facade. %s\n", p.c_str());
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
					WED_clean_vpath(file);
					fac->objs.push_back(file);
				}
				else if(MFS_string_match(&s,"#obj_wed",false))
				{
					string file;
					MFS_string_eol(&s,&file); s.cur -= 3;
					WED_clean_vpath(file);
					fac->objs.back() = file;      // use this one instead of the OBJ X-plane would use.
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
					tpl->meshes.back().nml.reserve(3*num_vert);
					tpl->meshes.back().uv.reserve(2*num_vert);
					tpl->meshes.back().idx.reserve(num_idx);
				}
				else if(tpl && MFS_string_match(&s,"VERTEX", false))
				{
					tpl->meshes.back().xyz.push_back(MFS_double(&s));
					tpl->meshes.back().xyz.push_back(MFS_double(&s));
					tpl->meshes.back().xyz.push_back(MFS_double(&s));
					Vector3 v;
					v.dx = MFS_double(&s);
					v.dy = MFS_double(&s);
					v.dz = MFS_double(&s);
//					v.normalize();                      // some facades are off by 1e6 - picky OGL shaders don't like that
					tpl->meshes.back().nml.push_back(v.dx);
					tpl->meshes.back().nml.push_back(v.dy);
					tpl->meshes.back().nml.push_back(v.dz);
					tpl->meshes.back().uv.push_back(MFS_double(&s));
					tpl->meshes.back().uv.push_back(MFS_double(&s));
				}
				else if(tpl && MFS_string_match(&s,"IDX", false))
				{
					while (MFS_has_word(&s))
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
					if (MFS_has_word(&s))
					{
						tpl->objs.back().show[0] = MFS_int(&s);
						tpl->objs.back().show[1] = MFS_int(&s);
					}
					else
					{
						tpl->objs.back().show[0] = 1;
						tpl->objs.back().show[1] = 1;
					}
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
				else if (MFS_string_match(&s, "TWO_SIDED_ROOF", true))
				{
					if (!fac->floors.back().roofs.empty())
						fac->floors.back().roofs.back().two_sided = true;
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
						LOG_MSG("E/Fac roof object not inside a roof in %s\n", p.c_str());
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
						LOG_MSG("E/Fac roof object not inside a roof in %s\n", p.c_str());
					else
						fac->floors.back().roofs.back().roof_objs.push_back(o);
				}
				else if (MFS_string_match(&s, "#cabin", false))
				{
					fac->cabin_idx = fac->wallName.size() - 1;
					fac->style_code = MFS_int(&s);
				}
				else if (MFS_string_match(&s, "#tunnel", false))
				{
					fac->tunnels.push_back(fac_info_t::tunnel_t());
					fac->tunnels.back().idx = fac->wallName.size() - 1;
					MFS_string(&s, &(fac->tunnels.back().obj));
					fac->tunnels.back().size_code = MFS_int(&s);
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

					for(auto& m : t.meshes)
						for(int i = 0; i < m.xyz.size(); i += 3)
							m.xyz[i + 2] -= xyz_max[2];    // right-adjust all wall segments to go from -xx to 0
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

			for(const auto& obj_nam : fac->objs)                // move this back into faacade preview code, since it costs too much time.
			{                                             // We do at times load EVERY facade just to find out which are custom jetways
				const XObj8 * o;
				fac->xobjs.push_back(nullptr);
				if(GetObjRelative(obj_nam, vpath, o))
					fac->xobjs.back() = o;
				else
					LOG_MSG("E/Fac can not load object %s in %s\n", obj_nam.c_str(), p.c_str());

			}
			if (fac->tunnels.size())
			{
				if (fac->style_code < 0)
				{
					fac->tunnels.clear();
					LOG_MSG("E/Fac %s does not have a valid #cabin tag, ignoring all #tunnel tags\n", p.c_str());
				}
				else
					for (auto& t : fac->tunnels)
					{
						if(!GetObjRelative(t.obj, vpath, t.o))
							LOG_MSG("E/Fac can not load jetway %s in %s\n", t.obj.c_str(), p.c_str());
					}
			}
		}
		process_texture_path(p,fac->wall_tex);
		if(fac->has_roof)
		{
			if(fac->roof_tex.empty())
				fac->roof_tex = fac->wall_tex;
			else
				process_texture_path(p,fac->roof_tex);
		}
		height_desc_for_facade(*fac, fac->h_range);
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

bool	WED_ResourceMgr::GetFor(const string& path, for_info_t const *& info)
{
	auto i = mFor.find(path);
	if(i != mFor.end())
	{
		info = &i->second;
		return true;
	}

	info = nullptr;
	string p = mLibrary->GetResourcePath(path);

	MFMemFile * fi = MemFile_Open(p.c_str());
	if(!fi) return false;

	MFScanner	s;
	MFS_init(&s, fi);

	int versions[] = { 800, 1000, 1200, 0 };
	if((MFS_xplane_header(&s,versions,"FOREST",NULL)) == 0)
	{
		LOG_MSG("E/RES unsupported version or header in %s\n", p.c_str());
		MemFile_Close(fi);
		return false;
	}

	for_info_t * fst = &mFor[path];
	info = fst;

	fst->has_3D = false;
	float scale_x=256, scale_y=256, space_x=30, space_y=30, rand_x=0, rand_y=0;
	string tex, tex_3d;
	bool shader_2d = true;
	fst->max_height = 0.0;
	int layer = 0;

	struct tree_mesh {
		vector<vector<float> > vert;
		vector<int> idx;
	};
	tree_mesh* this_tree_3d = nullptr;
	map<string, tree_mesh> trees_3d;
	bool is_tree2 = false;

	while(!MFS_done(&s))
	{
		if (this_tree_3d && MFS_string_match(&s, "VERTEX", false))
		{
			vector<float> v(8, 0.0);
			v[0] = MFS_double(&s);        // x,y,z
			v[1] = MFS_double(&s);
			v[2] = MFS_double(&s);
			v[3] = MFS_double(&s);        // normals
			v[4] = MFS_double(&s);
			v[5] = MFS_double(&s);
			v[6] = MFS_double(&s);        // texture u,v
			v[7] = MFS_double(&s);
			MFS_double(&s);               // dx,dy,dx annimation vectors
			MFS_double(&s);
			MFS_double(&s);
			this_tree_3d->vert.push_back(v);
		}
		else if (this_tree_3d && MFS_string_match(&s, "IDX", false))
		{
			while (MFS_has_word(&s))
				this_tree_3d->idx.push_back(MFS_int(&s));
		}
		else if (MFS_string_match(&s, "TEXTURE", false))
		{
			if (shader_2d)
				MFS_string(&s, &tex);
			else
				MFS_string(&s, &tex_3d);
		}
		else if (MFS_string_match(&s, "SCALE_X", false))
		{
			scale_x = MFS_double(&s);
		}
		else if (MFS_string_match(&s, "SCALE_Y", false))
		{
			scale_y = MFS_double(&s);
		}
		else if (MFS_string_match(&s, "SPACING", false))
		{
			space_x = MFS_double(&s);
			space_y = MFS_double(&s);
		}
		else if (MFS_string_match(&s, "RANDOM", false))
		{
			rand_x = MFS_double(&s);
			rand_y = MFS_double(&s);
		}
		else if ((is_tree2 = MFS_string_match(&s, "TREE2", false)) || MFS_string_match(&s, "TREE", false))
		{
			for_info_t::tree_t t;
			t.s = MFS_double(&s);
			t.t = MFS_double(&s);
			t.w = MFS_double(&s);
			t.h = MFS_double(&s);
			t.o = MFS_double(&s);
			t.pct = MFS_double(&s);
			t.hmin = MFS_double(&s);
			t.hmax = MFS_double(&s);
			if (fst->max_height < t.hmax) fst->max_height = t.hmax;
			if (is_tree2)                                // new optional format in XP12, per Sidney
			{
				MFS_double(&s);
				MFS_double(&s);
			}
			t.quads = MFS_int(&s);
			layer = MFS_int(&s);

			if (fabs(t.w) > 0.001 && t.h > 0.001)   // there are some .for with zero size tree's in XP10 and OpensceneryX uses negative widths ...
				fst->trees[layer].push_back(t);
		}
		else if (MFS_string_match(&s, "MESH_3D", false))
		{
			fst->has_3D = true;
			if(fst->trees[layer].back().mesh_3d.empty())
				MFS_string(&s, &fst->trees[layer].back().mesh_3d);
		}
		else if (MFS_string_match(&s, "MESH", false))
		{
			string nam;
			MFS_string(&s, &nam);
			this_tree_3d = &trees_3d[nam];
			MFS_double(&s);							 // near, far LOD
			MFS_double(&s);
			this_tree_3d->vert.reserve(MFS_int(&s));
			this_tree_3d->idx.reserve(MFS_int(&s));
		}
		else if (MFS_string_match(&s, "SHADER_2D", true))
			shader_2d = true;
		else if (MFS_string_match(&s, "SHADER_3D", true))
			shader_2d = false;

		if (MFS_string_match(&s,"#wed_text", false))
			MFS_string_eol(&s, &fst->description);
		else
			MFS_string_eol(&s, NULL);
	}
	MemFile_Close(fi);

	int quads = 0;

	// now we have one of each tree. Like on the ark. Or maybe half that :)
	// expand that to full forest of TPS * TPS trees, populated with all the varieties there are

	#define TREES_PER_ROW  6

	// fill a XObj8-structure for library preview

	int tot_varieties = 0;
	for (const auto& t_vec : fst->trees)
		tot_varieties += t_vec.second.size();
	if (tot_varieties == 0) return false;

	const int TPR = fst->trees.begin()->second.size() < 4 ? 3 : TREES_PER_ROW;

	fst->description += to_string(tot_varieties) + string(" different trees, ");
	fst->description += string("max h=") + to_string(intround(fst->max_height / (gIsFeet ? 0.3048 : 1.0))) + string(gIsFeet ? "ft" : "m");

	XObj8 *new_obj = new XObj8, *new_obj_3d = nullptr;
	XObjCmd8 cmd;
	new_obj->texture = tex;
	process_texture_path(p, new_obj->texture);
	if (fst->has_3D)
	{
		new_obj_3d = new XObj8;
		new_obj_3d->texture = tex_3d;
		process_texture_path(p, new_obj_3d->texture);
	}

	for (auto t_vec : fst->trees)
	{
		struct tree_pos {
			int species;
			float height;
			float x_off, y_off;
			float rot;
			tree_pos() : species(0) {};
		} tree_array[TREES_PER_ROW * TREES_PER_ROW];

		float total_pct = 0;
		for (const auto& t : t_vec.second)
			total_pct += t.pct;

		for (int i = t_vec.second.size() - 1; i > 0; i--)
			for (int j = 0; j < round(t_vec.second[i].pct / total_pct * TPR * TPR); ++j)
			{
				int cnt = 20;     // needed in case the tree percentages add up to more than 100%
				do
				{
					int where = ((float)TPR * TPR * rand()) / RAND_MAX;
					if (where >= 0 && where < TPR * TPR && tree_array[where].species == 0)
					{
						tree_array[where].species = i;
						break;
					}
				} while (--cnt);
			}

		// "VT "
		for (int i = 0; i < TPR * TPR; ++i)
		{
			for_info_t::tree_t* tree = &t_vec.second[tree_array[i].species];

			tree_array[i].height = tree->hmin + (tree->hmax - tree->hmin) * rand() / RAND_MAX;
			tree_array[i].x_off = (i % TPR) * space_x + rand_x * ((2.0 * rand()) / RAND_MAX - 1.0);     // tree position in our array
			tree_array[i].y_off = (i / TPR) * space_y + rand_y * ((2.0 * rand()) / RAND_MAX - 1.0);
			tree_array[i].rot = ((float)rand()) / RAND_MAX;                                      // doesn't make much sense to save this here for the 3D tree's
																								// as the 2D trees always face the user when 3D tree's are present
			float t_w = tree_array[i].height / tree->h * tree->w;

			for (int j = 0; j < tree->quads; ++j)
			{
				float rot = M_PI * (tree_array[i].rot + j / (float)tree->quads);
				float x = t_w * sinf(rot);
				float z = t_w * cosf(rot);
				quads++;

				float pt[8];
				pt[3] = 0.0;
				pt[4] = 1.0;
				pt[5] = 0.0;

				pt[0] = tree_array[i].x_off - x * (tree->o / tree->w);
				pt[1] = 0.0;
				pt[2] = tree_array[i].y_off - z * (tree->o / tree->w);
				pt[6] = tree->s / scale_x;
				pt[7] = tree->t / scale_y;

				new_obj->geo_tri.append(pt);
				pt[0] = tree_array[i].x_off + x * (1.0 - tree->o / tree->w);
				pt[2] = tree_array[i].y_off + z * (1.0 - tree->o / tree->w);
				pt[6] = (tree->s + tree->w) / scale_x;
				new_obj->geo_tri.append(pt);
				pt[1] = tree_array[i].height;
				pt[7] = (tree->t + tree->h) / scale_y;
				new_obj->geo_tri.append(pt);
				pt[0] = tree_array[i].x_off - x * (tree->o / tree->w);
				pt[2] = tree_array[i].y_off - z * (tree->o / tree->w);
				pt[6] = tree->s / scale_x;
				new_obj->geo_tri.append(pt);
			}

			if (fst->has_3D)
			{
				tree_mesh* tree_3d = &trees_3d[tree->mesh_3d];

				int i_base = new_obj_3d->geo_tri.count();
				float scale = tree_array[i].height / tree->hmin;
				float sin_rot = scale * sinf(M_PI * (tree_array[i].rot));
				float cos_rot = scale * cosf(M_PI * (tree_array[i].rot));

				for (auto v : tree_3d->vert)
				{
					auto x = v[0];
					auto z = v[2];
					v[0] = tree_array[i].x_off + x * cos_rot - z * sin_rot;
					v[1] *= scale;
					v[2] = tree_array[i].y_off + x * sin_rot + z * cos_rot;
					new_obj_3d->geo_tri.append(v.data());
				}
				for (auto i : tree_3d->idx)
					new_obj_3d->indices.push_back(i + i_base);
			}
		}
	}

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
	fst->preview = new_obj;

	if (fst->has_3D)
	{
		new_obj_3d->geo_tri.get_minmax(new_obj_3d->xyz_min, new_obj_3d->xyz_max);

		// "ATTR_LOD"
		new_obj_3d->lods.push_back(XObjLOD8());
		new_obj_3d->lods.back().lod_near = 0;
		new_obj_3d->lods.back().lod_far = 1000;

		// "TRIS ";
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = 0;
		cmd.idx_count = new_obj_3d->indices.size();
		new_obj_3d->lods.back().cmds.push_back(cmd);
		fst->preview_3d = new_obj_3d;
	}

	return true;
}

void WED_ResourceMgr::setup_tile(agp_t::tile_t * agp, int rotation, const string& path)
{
	for(int n = 0; n < agp->tile.size(); n += 4)
	{
		agp->tile[n  ] -= agp->anchor_x;
		agp->tile[n+1] -= agp->anchor_y;
		do_rotate(rotation,agp->tile[n  ],agp->tile[n+1]);
	}
	for(auto& o : agp->objs)
	{
		o.x -= agp->anchor_x;
		o.y -= agp->anchor_y;
		do_rotate(rotation, o.x, o.y);
		o.r += 90 * rotation;
	}
	for (auto& f : agp->facs)
		for(auto& l : f.locs)
		{
			float x = l.x_ - agp->anchor_x;
			float y = l.y_ - agp->anchor_y;
			do_rotate(rotation, x, y);
			l.x_ = x;
			l.y_ = -y;
		}

	agp->xyz_min[0] = agp->xyz_min[1] = agp->xyz_min[2] =  999.0;
	agp->xyz_max[0] = agp->xyz_max[1] = agp->xyz_max[2] = -999.0;

	for(int n = 0; n < agp->tile.size(); n += 4)
	{
		agp->xyz_min[0] = min(agp->xyz_min[0], agp->tile[n]);
		agp->xyz_max[0] = max(agp->xyz_max[0], agp->tile[n]);
		agp->xyz_min[2] = min(agp->xyz_min[2], agp->tile[n+1]);
		agp->xyz_max[2] = max(agp->xyz_max[2], agp->tile[n+1]);
	}

	auto o = agp->objs.begin();
	while(o != agp->objs.end())
	{
		const XObj8 * oo;
		if(GetObjRelative(o->name, path, oo))
		{
			o->obj = oo;
			if (fabs(o->r-180.0) < 45.0)  // account for rotation, very roughly only
			{
					agp->xyz_min[0] = min(agp->xyz_min[0], oo->xyz_min[0] + o->x);
					agp->xyz_max[0] = max(agp->xyz_max[0], oo->xyz_max[0] + o->x);
					agp->xyz_min[2] = min(agp->xyz_min[2], oo->xyz_min[2] + o->y);
					agp->xyz_max[2] = max(agp->xyz_max[2], oo->xyz_max[2] + o->y);
			}
			else if (fabs(o->r-90.0) < 45.0)
			{
					agp->xyz_min[0] = min(agp->xyz_min[0],-oo->xyz_min[2] + o->x);
					agp->xyz_max[0] = max(agp->xyz_max[0],-oo->xyz_max[2] + o->x);
					agp->xyz_min[2] = min(agp->xyz_min[2], oo->xyz_min[0] + o->y);
					agp->xyz_max[2] = max(agp->xyz_max[2], oo->xyz_max[0] + o->y);
			}
			else if (fabs(o->r+90.0) < 45.0)
			{
					agp->xyz_min[0] = min(agp->xyz_min[0], oo->xyz_max[2] + o->x);
					agp->xyz_max[0] = max(agp->xyz_max[0], oo->xyz_min[2] + o->x);
					agp->xyz_min[2] = min(agp->xyz_min[2],-oo->xyz_max[0] + o->y);
					agp->xyz_max[2] = max(agp->xyz_max[2],-oo->xyz_min[0] + o->y);
			}
			else
			{
					agp->xyz_min[0] = min(agp->xyz_min[0],-oo->xyz_max[0] + o->x);
					agp->xyz_max[0] = max(agp->xyz_max[0],-oo->xyz_min[0] + o->x);
					agp->xyz_min[2] = min(agp->xyz_min[2],-oo->xyz_max[2] + o->y);
					agp->xyz_max[2] = max(agp->xyz_max[2],-oo->xyz_min[2] + o->y);
			}
			agp->xyz_min[1] = min(agp->xyz_min[1], oo->xyz_min[1] + o->z);
			agp->xyz_max[1] = max(agp->xyz_max[1], oo->xyz_max[1] + o->z);
			o++;
		}
		else
		{
			o = agp->objs.erase(o);
			LOG_MSG("E/Agp can not load object %s in %s\n", o->name.c_str(), path.c_str());
		}
	}

	auto f = agp->facs.begin();
	while (f != agp->facs.end())
	{
		const fac_info_t * fac;
		if(GetFac(f->name, fac))                // doesn't take rpaths, only vpaths
		{
			f->fac = fac;
/*			for (auto& l : f->locs)
			{
				agp->xyz_min[0] = min(agp->xyz_min[0], (float) l.x());
				agp->xyz_max[0] = max(agp->xyz_max[0], (float) l.x());
				agp->xyz_min[2] = min(agp->xyz_min[2], (float) l.y());
				agp->xyz_max[2] = max(agp->xyz_max[2], (float) l.y());
			}
			agp->xyz_min[1] = min(agp->xyz_min[1], 0.0f);    // do better - figure the real height limits
			agp->xyz_max[1] = min(agp->xyz_max[1], 2.0f); */
			f++;
		}
		else
		{
			f = agp->facs.erase(f);
			LOG_MSG("E/Agp can not load facade %s in %s\n", f->name.c_str(), path.c_str());
		}
	}
}

bool	WED_ResourceMgr::GetAGP(const string& path, agp_t const *& info)
{
	auto i = mAGP.find(path);
	if(i != mAGP.end())
	{
		info = &i->second;
		return true;
	}

	string p = mLibrary->GetResourcePath(path);
	MFMemFile * file = MemFile_Open(p.c_str());
	if(!file) return false;

	MFScanner	s;
	MFS_init(&s, file);

	int versions[] = { 1000, 0 };
	int v;

//	if( (v=MFS_xplane_header(&s,versions,"AG_POINT",NULL)) == 0)
	string l1; MFS_string_eol(&s, &l1);
	v = MFS_int(&s); MFS_string_eol(&s,NULL);
	string l3; MFS_string_eol(&s, &l3);
	if((l1 != "I" && l1 != "A") || v != 1000 || (l3 != "AG_STRING" && l3 != "AG_POINT" && l3 != "AG_BLOCK"))
	{
		LOG_MSG("E/RES unsupported version or header in %s\n", p.c_str());
		MemFile_Close(file);
		return false;
	}

	agp_t * agp = &mAGP[path];
	info = agp;

	double tex_s = 1.0, tex_t = 1.0;		// these scale from pixels to UV coords
	double tex_x = 1.0, tex_y = 1.0;		// meters for tex, x & y

	double flip_y = l3 == "AG_STRING" ? -1.0 : 1.0;
	int	 rotation = 0;
	int	 last_id = -1;
	agp->hide_tiles = 0;
	vector<string>	obj_paths, fac_paths;

	bool is_mesh_shader = false;
	agp_t::tile_t * ti = nullptr;

	while(!MFS_done(&s))
	{
		if(MFS_string_match(&s,"TEXTURE",false))
		{
			string tex;
			MFS_string(&s,&tex);
			if(is_mesh_shader)
			{
				agp->mesh_tex = tex;
				process_texture_path(p,agp->mesh_tex);
			}
			else
			{
				agp->base_tex = tex;
				process_texture_path(p,agp->base_tex);
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
		else if (MFS_string_match(&s, "TEXTURE_HEIGHT", false))
		{
			tex_y = MFS_double(&s);
		}
		else if(MFS_string_match(&s,"OBJECT",false))
		{
			string p;
			MFS_string(&s,&p);
			WED_clean_vpath(p);          // cant say here yet if its a relative rpath or a vpath.
			obj_paths.push_back(p);
		}
		else if (MFS_string_match(&s,"FACADE", false))
		{
			string p;
			MFS_string(&s, &p);
			WED_clean_vpath(p);          // cant say here yet if its a relative rpath or a vpath.
			fac_paths.push_back(p);
		}
		else if(MFS_string_match(&s,"#object_wed",false))
		{
			string p;
			MFS_string_eol(&s,&p); s.cur -= 3;
			WED_clean_vpath(p);
			obj_paths.back() = p;      // use this one instead of the OBJECT X-plane would use.
		}
		else if(MFS_string_match(&s,"TILE_ID",false))
		{
			last_id = MFS_int(&s);
		}
		else if(MFS_string_match(&s,"TILE",false))
		{
			if(ti) setup_tile(ti, rotation, path);
			agp->tiles.push_back(agp_t::tile_t());
			ti = &agp->tiles.back();
			ti->id = last_id;
			double s1 = MFS_double(&s);
			double t1 = MFS_double(&s);
			double s2 = MFS_double(&s);
			double t2 = MFS_double(&s);
			double x1 = s1 * tex_s * tex_x;
			double x2 = s2 * tex_s * tex_x;
			double y1 = t1 * tex_t * tex_y;
			double y2 = t2 * tex_t * tex_y;

			s1 *= tex_s; s2 *= tex_s;
			t1 *= tex_t; t2 *= tex_t;

			ti->anchor_x = (x1 + x2) * 0.5;
			ti->anchor_y = (y1 + y2) * 0.5;
			ti->tile.resize(16);
			ti->tile[ 0] = x1;	ti->tile[ 1] = y1;
			ti->tile[ 2] = s1;	ti->tile[ 3] = t1;
			ti->tile[ 4] = x2;	ti->tile[ 5] = y1;
			ti->tile[ 6] = s2;	ti->tile[ 7] = t1;

			ti->tile[ 8] = x2; ti->tile[ 9] = y2;
			ti->tile[10] = s2; ti->tile[11] = t2;
			ti->tile[12] = x1; ti->tile[13] = y2;
			ti->tile[14] = s1; ti->tile[15] = t2;
		}
		else if(MFS_string_match(&s,"SUB_GRID",false))
		{
			agp->tiles.push_back(agp_t::tile_t());
			ti = &agp->tiles.back();
		}
		else if (MFS_string_match(&s, "CUT_H", false))
		{
			if (ti)
				ti->cut_h.push_back(MFS_double(&s) * tex_s);
		}
		else if (MFS_string_match(&s, "CUT_V", false))
		{
			if (ti)
				ti->cut_v.push_back(MFS_double(&s) * tex_t);
		}
		else if (MFS_string_match(&s, "END_CUTS", true))
		{
			if (ti)
			{
				double s1 = ti->cut_h.front();
				double t1 = ti->cut_v.front();
				double s2 = ti->cut_h.back();
				double t2 = ti->cut_v.back();
				double x1 = tex_x * s1;
				double x2 = tex_x * s2;
				double y1 = tex_y * t1;
				double y2 = tex_y * t2;

				ti->tile.resize(16);
				ti->tile[0] = x1;	ti->tile[1] = y1;
				ti->tile[2] = s1;	ti->tile[3] = t1;
				ti->tile[4] = x2;	ti->tile[5] = y1;
				ti->tile[6] = s2;	ti->tile[7] = t1;

				ti->tile[8] = x2; ti->tile[9] = y2;
				ti->tile[10] = s2; ti->tile[11] = t2;
				ti->tile[12] = x1; ti->tile[13] = y2;
				ti->tile[14] = s1; ti->tile[15] = t2;
			}
		}
		else if(MFS_string_match(&s,"ROTATION",false))
		{
			rotation = MFS_int(&s);
		}
		else if(MFS_string_match(&s,"CROP_POLY",false))
		{
			ti->tile.clear();
			while(MFS_has_word(&s))
			{
				double ps = MFS_double(&s);
				double pt = MFS_double(&s);
				ti->tile.push_back(ps * tex_s * tex_x);
				ti->tile.push_back(pt * tex_t * tex_y);
				ti->tile.push_back(ps * tex_s);
				ti->tile.push_back(pt * tex_t);
			}
		}
		else if(MFS_string_match(&s,"OBJ_DRAPED",false) ||
				MFS_string_match(&s,"OBJ_GRADED",false))
		{
			ti->objs.push_back(agp_t::obj_t());
			ti->objs.back().x = MFS_double(&s) * tex_s * tex_x;
			ti->objs.back().y = MFS_double(&s) * tex_t * tex_y;
			ti->objs.back().r = MFS_double(&s);
			ti->objs.back().z = 0.0;
			int obj_idx = MFS_int(&s);
			if(obj_idx >= 0 && obj_idx < obj_paths.size())
			{
				ti->objs.back().name = obj_paths[obj_idx];
				ti->objs.back().show_lo = MFS_int(&s);
				ti->objs.back().show_hi = MFS_int(&s);
			}
			else
			{
				ti->objs.pop_back(); // ignore instances with OOB index
				LOG_MSG("E/Agp object index out of bounds in %s\n",p.c_str());
			}
		}
		else if (MFS_string_match(&s, "OBJ_SCRAPER", false))
		{
			ti->objs.push_back(agp_t::obj_t());
			ti->objs.back().x = MFS_double(&s) * tex_s * tex_x;
			ti->objs.back().y = MFS_double(&s) * tex_t * tex_y;
			ti->objs.back().r = MFS_double(&s);
			ti->objs.back().z = 0.0;
			int obj_idx = MFS_int(&s);
			if (obj_idx >= 0 && obj_idx < obj_paths.size())
			{
				ti->objs.back().name = obj_paths[obj_idx];
				ti->objs.back().scp_min = MFS_double(&s);
				ti->objs.back().scp_max = MFS_double(&s);
				ti->objs.back().scp_step = MFS_double(&s);
				if(ti->objs.back().scp_step > 0.0) { ti->has_scp = true; agp->has_scp = true; }
			}
			else
			{
				ti->objs.pop_back(); // ignore instances with OOB index
				LOG_MSG("E/Agp object index out of bounds in %s\n",p.c_str());
			}
		}
		else if(MFS_string_match(&s,"OBJ_DELTA",false))
		{
			ti->objs.push_back(agp_t::obj_t());
			ti->objs.back().x = MFS_double(&s) * tex_s * tex_x;
			ti->objs.back().y = MFS_double(&s) * tex_t * tex_y;
			ti->objs.back().r = MFS_double(&s);
			ti->objs.back().z = MFS_double(&s);
			int obj_idx = MFS_int(&s);
			if(obj_idx >= 0 && obj_idx < obj_paths.size())
			{
				ti->objs.back().name = obj_paths[obj_idx];
				ti->objs.back().show_lo = MFS_int(&s);
				ti->objs.back().show_hi = MFS_int(&s);
			}
			else
			{
				ti->objs.pop_back(); // ignore instances with OOB index
				LOG_MSG("E/Agp object index out of bounds in %s\n",p.c_str());
			}
		}
		else if (MFS_string_match(&s, "FAC", false))
		{
			ti->facs.push_back(agp_t::fac_t());
			int fac_idx = MFS_int(&s);
			if (fac_idx >= 0 && fac_idx < fac_paths.size())
			{
				ti->facs.back().name = fac_paths[fac_idx];
				ti->facs.back().height = MFS_double(&s);
				while (MFS_has_word(&s))
				{
					Point2 p;
					p.x_ = MFS_double(&s) * tex_s * tex_x;
					p.y_ = MFS_double(&s) * tex_t * tex_y * flip_y;
					ti->facs.back().locs.push_back(p);
					ti->facs.back().walls.push_back(0);
				}
			}
			else
			{
				ti->facs.pop_back(); // ignore instances with OOB index
				LOG_MSG("E/Agp facade index out of bounds in %s\n",p.c_str());
			}
		}
		else if (MFS_string_match(&s, "FAC_WALLS", false))
		{
			ti->facs.push_back(agp_t::fac_t());
			int fac_idx = MFS_int(&s);
			if (fac_idx >= 0 && fac_idx < fac_paths.size())
			{
				ti->facs.back().name = fac_paths[fac_idx];
				ti->facs.back().height = MFS_double(&s);
				while (MFS_has_word(&s))
				{
					Point2 p;
					p.x_ = MFS_double(&s) * tex_s * tex_x;
					p.y_ = MFS_double(&s) * tex_t * tex_y * flip_y;
					ti->facs.back().locs.push_back(p);
					ti->facs.back().walls.push_back(MFS_int(&s));
				}
			}
			else
			{
				ti->facs.pop_back(); // ignore instances with OOB index
				LOG_MSG("E/Agp facade index out of bounds in %s\n",p.c_str());
			}
		}
		else if(MFS_string_match(&s,"ANCHOR_PT",false))
		{
			ti->anchor_x = MFS_double(&s) * tex_s * tex_x;
			ti->anchor_y = MFS_double(&s) * tex_t * tex_y;
		}
		else if (MFS_string_match(&s,"HIDE_TILES",true))
		{
			agp->hide_tiles = 1;
		}
		else if (MFS_string_match(&s,"MESH_SHADER",true))
		{
			is_mesh_shader = true;
		}

		if (MFS_string_match(&s,"#wed_text", false))
			MFS_string_eol(&s,&agp->description);
		else
			MFS_string_eol(&s,NULL);
	}
	for(auto& t : agp->tiles)
		setup_tile(&t, rotation, path);

	MemFile_Close(file);
	return true;
}

#if ROAD_EDITING
bool	WED_ResourceMgr::GetRoad(const string& path, const road_info_t *& out_info)
{
	auto i = mRoad.find(path);
	if(i != mRoad.end())
	{
		out_info = &i->second;
		return true;
	}

	out_info = nullptr;
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * mf = MemFile_Open(p.c_str());
	if(!mf) return false;

	MFScanner	s;
	MFS_init(&s, mf);

	int versions[] = { 800, 0 };
	int v;
	if((v=MFS_xplane_header(&s,versions,"ROADS",NULL)) == 0)
	{
		LOG_MSG("E/RES unsupported version or header in %s\n", p.c_str());
		MemFile_Close(mf);
		return false;
	}

	road_info_t * rd = &mRoad[path];
	out_info = rd;

	road_info_t::vroad_t vroad;
	int last_vroad = 0;

	road_info_t::road_t road;
	int last_road = 0;

	double max_lod = 0.0;
	double last_scale = 0.0;
	double last_center = 0.0;

	double leftmost_lane = 999;
	double rightmost_lane = -999;
	int	lane_direction = -1;
//	string last_name;

	while(!MFS_done(&s))
	{
		if(MFS_string_match(&s,"#VROAD", false))
		{
//			if (last_vroad)
//				printf("vroad %d %s referencing road %d %s\n", last_vroad, vroad.description.c_str(), vroad.rd_type, out_info.road_types.count(last_vroad) ? "(ok)" : "(unk)");

			MFS_string(&s,&vroad.description); // vroad type display name
		}
		else if(MFS_string_match(&s, "ROAD_DRAPED", false))
		{
			MFS_int(&s);
			last_vroad = MFS_int(&s);		// vroad type number
			vroad.rd_type = 0;
			MFS_double(&s);
			MFS_double(&s);

			rd->vroad_types[last_vroad] = vroad;
		}
		else if(MFS_string_match(&s, "ROAD_DRAPE_CHOICE", false))
		{
			MFS_double(&s);
			vroad.rd_type = MFS_int(&s);

			if (last_vroad && vroad.rd_type)
//				rd->vroad_types[last_vroad] = vroad;  // if we take the last road choice for previews, thats usually the bridges.
                                                      // we can assume the first segment is just the pavement and minimal sidewalks,
													  // which is OK for a 'simple' preview using only a SINGLE degment
			{
				rd->vroad_types[last_vroad] = vroad;  // the first road choice is usually the non-wet/non-graded one, with full sidewalks etc
				                                      // but often built from multiple segements - so we need to show them all to be meaningfull
				last_vroad = 0;
			}
		}
		else if(MFS_string_match(&s, "TEXTURE", false))
		{
			MFS_int(&s);       	 	     // z-index, we dont care
			string tex_name;
			MFS_string(&s,&tex_name);    // texture name
			rd->textures.push_back(tex_name);
		}
		else if(MFS_string_match(&s, "ROAD_TYPE", false))
		{
			last_road    = MFS_int(&s);   	        // road type number
			road.width   = MFS_double(&s);          // nominal width of pavement in mtr
			road.length  = MFS_double(&s);			// length for repeating texture in mtr
			road.tex_idx = MFS_int(&s);       	 	// texture index
			road.traffic_width = 0.0;
			road.oneway = true;
			// ignore the rgb definitions for map display colors, for now
			rd->road_types[last_road] = road;
			max_lod = 0.0;
			leftmost_lane = 999;
			rightmost_lane = -999;
			lane_direction = -1;

//			printf("road %d w=%.1f\n",last_road, road.width);
		}
		else if(MFS_string_match(&s, "ROAD_CENTER", false))
		{
			last_center  = MFS_double(&s);         // define lateral offset relative to vector, added to everything
		}
		else if(MFS_string_match(&s, "SCALE", false))
		{
			last_scale = MFS_double(&s);
		}
		else if(MFS_string_match(&s, "SEGMENT_DRAPED", false))
		{
			auto& r = rd->road_types[last_road];

			int tex_idx = MFS_int(&s);
			MFS_double(&s);						// min lod
			double lod = MFS_double(&s);		// max lod
		//	if (max_lod <= lod)
			{
				r.tex_idx = tex_idx;
		//		if(max_lod < lod) r.segs.clear();
				max_lod = lod;
				r.segs.push_back(road_info_t::road_t::seg_t());

				MFS_double(&s);

				r.segs.back().left   = MFS_double(&s) - last_center;
				r.segs.back().s_left = MFS_double(&s) / last_scale;
				r.segs.back().right   = MFS_double(&s) - last_center;
				r.segs.back().s_right = MFS_double(&s) / last_scale;
			}

//			printf("road %d w=%.1f\n",last_road, road.width);
		}
		else if(MFS_string_match(&s, "SEGMENT_GRADED", false))
		{
			auto& r = rd->road_types[last_road];

			int tex_idx = MFS_int(&s);
			MFS_double(&s);						// min lod
			double lod = MFS_double(&s);		// max lod
		//	if (max_lod <= lod)
			{
		//		if(max_lod < lod) r.segs.clear();
				max_lod = lod;
				r.segs.push_back(road_info_t::road_t::seg_t());

				MFS_double(&s);

				r.segs.back().left   = MFS_double(&s) - last_center;
				double h_left =        MFS_double(&s);
				r.segs.back().s_left = MFS_double(&s) / last_scale;
				r.segs.back().right   = MFS_double(&s) - last_center;
				double h_right =        MFS_double(&s);
				r.segs.back().s_right = MFS_double(&s) / last_scale;

				if(h_left != 0.0 || h_right != 0.0)
					r.segs.pop_back();                  // ignore the elevated/sloped sections for our previews
				else
					r.tex_idx = tex_idx;

			}
//			printf("road %d w=%.1f\n",last_road, road.width);
		}
		else if(MFS_string_match(&s, "OBJECT", false))
		{
			string obj_path;   MFS_string(&s, &obj_path);
			double lat_offs  = (MFS_double(&s) - 0.5) * rd->road_types[last_road].width;
			double rotation  = MFS_double(&s);
			                   MFS_int(&s);        // on ground/elevated
			double frequency = MFS_double(&s);
			double strt_offs = MFS_double(&s);

			if(frequency == 0.0)
			{
				rd->road_types[last_road].vert_objs.push_back(road_info_t::road_t::obj_t());
				auto& o = rd->road_types[last_road].vert_objs.back();
				o.path     = obj_path;
				o.lat_offs = lat_offs - last_center;
				o.rotation = rotation;
			}
		}
		else if(MFS_string_match(&s, "WIRE", false))
		{
			rd->road_types[last_road].wires.push_back(road_info_t::road_t::wire_t());
			auto& w = rd->road_types[last_road].wires.back();

			               MFS_double(&s);     // min lod
			               MFS_double(&s);     // max lod
			w.lat_offs =  (MFS_double(&s) -0.5) * rd->road_types[last_road].width;  // convert to absolute offset
			w.end_height = MFS_double(&s);
			w.droop =      MFS_double(&s);	   // 0 no droop, 1 touches ground in middle

			if(leftmost_lane > w.lat_offs)  leftmost_lane = w.lat_offs;
			if(rightmost_lane < w.lat_offs) rightmost_lane = w.lat_offs;
			double traffic_width = rightmost_lane - leftmost_lane + 4.0;
			if(traffic_width > rd->road_types[last_road].traffic_width)
				rd->road_types[last_road].traffic_width = traffic_width;
			rd->road_types[last_road].oneway = false;
		}
		else if(MFS_string_match(&s, "OBJECT_DRAPED", false))
		{
			string type;       MFS_string(&s, &type);
			if(type == "DIST")                       // there are also BEGIN and END types - placed where vectors dead-end
			{
				rd->road_types[last_road].dist_objs.push_back(road_info_t::road_t::obj_t());
				auto& o = rd->road_types[last_road].dist_objs.back();

							  MFS_string(&s, &o.path);
				o.lat_offs  = MFS_double(&s) - last_center;   // min offset
				              MFS_double(&s);   // max offset, if not same, random variation
				o.rotation  = MFS_double(&s);   // min rotation
				              MFS_double(&s);   // max rotation
				              MFS_double(&s);   // usually int, but sometimes float representing almost whole numbers plus tiny (1e-6) offsets
				              MFS_double(&s);   // same as above
				              MFS_string(&s, &type); // two floats in x.x/y.y format or one int
				              MFS_string(&s, &type); // two floats in x.x/y.y format or one int
				              MFS_int(&s);
				              MFS_int(&s);
			}
		}
		else if(MFS_string_match(&s, "OBJECT_GRADED", false))
		{
			string type;       MFS_string(&s, &type);
			if(type == "DIST")                      // placed by distance along vector
			{
				rd->road_types[last_road].dist_objs.push_back(road_info_t::road_t::obj_t());
				auto& o = rd->road_types[last_road].dist_objs.back();

							  MFS_string(&s, &o.path);
				o.lat_offs  = MFS_double(&s) - last_center;   // min offset
				              MFS_double(&s);   // max offset, if not same, random variation
				o.rotation  = MFS_double(&s);   // min rotation
				              MFS_double(&s);   // max rotation
				              MFS_double(&s);   // usually int, but sometimes float representing almost whole numbers plus tiny (1e-6) offsets
				              MFS_double(&s);   // same as above
				              MFS_string(&s, &type); // two floats in x.x/y.y format or one int
				              MFS_string(&s, &type); // same as above
				              MFS_int(&s);
				              MFS_int(&s);
			}
			else if(type == "VERT")                 // placed only at vertices, e.g. powerlines
			{
				rd->road_types[last_road].vert_objs.push_back(road_info_t::road_t::obj_t());
				auto& o = rd->road_types[last_road].vert_objs.back();

				              MFS_string(&s, &o.path);
				o.lat_offs  = MFS_double(&s) - last_center;   // min offset
				              MFS_double(&s);   // max offset, if not same, random variation
				o.rotation  = MFS_double(&s);   // min rotation
				              MFS_double(&s);   // max rotation
				              MFS_double(&s);   // usually int, but sometimes float representing almost whole numbers plus tiny (1e-6) offsets
				              MFS_double(&s);   // same as above
							  MFS_string(&s, &type); // two floats in x.x/y.y format usually all zero
				              MFS_string(&s, &type); // same as above
			}
		}
		else if(MFS_string_match(&s, "OBJECT_FREQ", false))
		{
			                   MFS_double(&s);       // first paird efinex a subrange of 0.0 to 1.0, like 0.0 1.0 or 0.3 0.7
			                   MFS_double(&s);

			                   MFS_double(&s);       // a set of two to four pairs of numbers. First in each pair is usually a power-of-2 or large whole number
			                   MFS_double(&s);       // second half of each pair is the associated probability (1 > x > 0)

			                   MFS_double(&s);       // subsequent values may be zero, but if nonzero always a dual fraction of previous value
			                   MFS_double(&s);       // all probabilites always add up to 1.0

			                   MFS_double(&s);
			                   MFS_double(&s);

			                   MFS_double(&s);
			                   MFS_double(&s);
		}
		else if(MFS_string_match(&s, "OBJECT_ALT", false))
		{
			string obj_path;   MFS_string(&s, &obj_path);
		}
		else if(MFS_string_match(&s, "CAR_DRAPED", false) || MFS_string_match(&s, "CAR_GRADED", false))
		{
			int    dir = MFS_int(&s);         // traffic direction
			double lat = MFS_double(&s);	  // traffic lateral offset

			// the width parameter for roads is pretty misleading - its the maximum width with all possible sidewalks etc.
			// we want the traffic lanes only

			if(leftmost_lane > lat)  leftmost_lane = lat;
			if(rightmost_lane < lat) rightmost_lane = lat;

			double traffic_width = rightmost_lane - leftmost_lane + 4.0;

			if(lane_direction < 0) lane_direction = dir;
			else if(lane_direction != dir) rd->road_types[last_road].oneway = false;

			if(traffic_width > rd->road_types[last_road].traffic_width)
				rd->road_types[last_road].traffic_width = traffic_width;
		}
		else if(MFS_string_match(&s, "TRAIN", true))
		{
			rd->road_types[last_road].traffic_width = 4.0;
		}
		MFS_string_eol(&s, NULL);
	}

	MemFile_Close(mf);

#if 0
	const char * dir = "/home/xplane/XP11/Custom Scenery/lin_roads/";
	bool euro = path.find("_EU") != string::npos;

	if(euro) FILE_make_dir_exist((string(dir) + "roads_EU").c_str());
	else     FILE_make_dir_exist((string(dir) + "roads").c_str());

	char buf[256];
	snprintf(buf,sizeof(buf), "%slibrary.txt", dir);
	FILE * lib_fp = fopen(buf, "a");
//	fprintf(lib_fp,"I\n800\nLIBRARY\n\n");
//	fprintf(lib_fp,"PUBLIC\n");

	for(auto& r : rd->vroad_types)
	{
		auto& rr = rd->road_types.at(r.second.rd_type);

		string nam(r.second.description);
		if(nam.substr(0,4) == "rail" || nam.substr(0,5) == "power") continue;

		for(int i = 0; i<nam.size(); i++)
			if(nam[i] == '/') nam[i] = '_';
		snprintf(buf,sizeof(buf), "%s%s/%d-%s.lin",dir, euro ? "roads_EU" : "roads", r.first, nam.c_str());

		FILE * fp = fopen(buf, "w"); if(!fp) continue;

		fprintf(fp,"I\n850\nLINE_PAINT\n\n");
		string tex(rd->textures[rr.tex_idx]);
		fprintf(fp,"TEXTURE ../%s\n", tex.c_str());
		if(tex.find("CoresResidentialDry") != string::npos)
		{
			fprintf(fp,"TEXTURE_NORMAL 1.0 ../%s\n", (tex.substr(0,tex.size() - 4) + "_NML.png").c_str());
		}

		double w = 0, x_scale; 		// get scale from widest segment - narrow segments are inaccurate due to integer pixel resolution of st coords
		for(auto s : rr.segs)
			if ( s.s_right - s.s_left > w)
			{
				w = s.s_right - s.s_left;
				x_scale = (s.left - s.right) / (s.s_left - s.s_right);
			}
		printf("%d %.2lf\n",r.first,x_scale);
		fprintf(fp,"LOD 10000\nTEX_WIDTH 2048\nSCALE %.1lf %.1lf\n", x_scale, rr.length);
		int x=0;
		for(auto s = rr.segs.rbegin(); s != rr.segs.rend(); s++)
		{
			double   center = (s->left+s->right)/2.0;           // this is where it needs to be placed, lateral offset in meters
			double s_center = (s->s_left+s->s_right)/2.0;       // this is where the center of the st coords for this segment actually are
			fprintf(fp, "S_OFFSET %d %d %d %d\n", x, intround(s->s_left*2048), intround((s_center - center / x_scale)*2048) + (center > 3.0 ? 1 : 0), intround(s->s_right*2048));
			x++;
		}
		fprintf(fp, "MIRROR\n");
		fprintf(fp, "LAYER_GROUP roads -1\n");
		if(nam.find("hwy") != string::npos)
			fprintf(fp, "DECAL_LIB lib/g10/decals/road_hwy.dcl\n");
		else
			fprintf(fp, "DECAL_LIB lib/g10/decals/road%s_dry.dcl\n", euro ? "_EU" : "_res" );
		fclose(fp);

//		fprintf(lib_fp,"EXPORT lib/g10/%s/roads/%d-%s.lin\t\t\troads%s/%d-%s.lin\n", euro ? "EU" : "US", r.first, nam.c_str(),
//		                                                                             euro ? "_EU" : "",  r.first, nam.c_str());
	}
	fclose(lib_fp);
#endif

	for(auto& t : rd->textures)
		process_texture_path(p, t);

	return true;
}

void WED_JWFacades::load(WED_LibraryMgr* lmgr, WED_ResourceMgr* rmgr)
{
	for (auto& r : lmgr->res_table)
	{
		if (r.second.res_type == res_Facade)
		{
			const fac_info_t* fac;
			if (r.second.packages.count(pack_Local))
			{
				if (rmgr->GetFac(r.first, fac))
					for (auto& t : fac->tunnels)
						mJWFacades[FILE_get_dir_name(r.first) + t.obj] = r.first; // local facades always need to point to local objects
			}
			else if (!r.second.is_default && (r.first.find("jetway") != string::npos || r.first.find("Jetway") != string::npos))
			{
				if (rmgr->GetFac(r.first, fac))
					for (auto& t : fac->tunnels)
						mJWFacades[t.obj] = r.first; // library JW facades always need to point to vpaths. can not resolve a relative rpath in an external library
			}
		}
	}
}

string WED_JWFacades::find(WED_LibraryMgr* lmgr, WED_ResourceMgr* rmgr, const string& tunnel_vpath)
{
	if (!mInitialized)
	{
		load(lmgr, rmgr);
		mInitialized = true;
	}
	auto tun = mJWFacades.find(tunnel_vpath);
	if (tun == mJWFacades.end())
		return "can not find suitable jetway facade";
	else
		return tun->second;
}

string	WED_ResourceMgr::GetJetwayVpath(const string& tunnel_vpath)
{
	return mJetways.find(mLibrary, this, tunnel_vpath);
}

#endif
