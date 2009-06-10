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
	
	FILE * fi = fopen(p.c_str(), "w");
	if(!fi)	return;
	fprintf(fi,"A\n850\nDRAPED_POLYGON\n\n");
	
	fprintf(fi,out_info.wrap ? "TEXTURE %s\n" : "TEXTURE_NOWRAP %s\n", out_info.base_tex.c_str());
	fprintf(fi,"SCALE %lf %lf\n",out_info.proj_s,out_info.proj_t);

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
