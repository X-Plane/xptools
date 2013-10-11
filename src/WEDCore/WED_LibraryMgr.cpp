/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_LibraryMgr.h"
#include "WED_PackageMgr.h"
#include "WED_Messages.h"
#include "XUtils.h"
#include "AssertUtils.h"
#include "PlatformUtils.h"
#include "MemFileUtils.h"

static void clean_vpath(string& s)
{
	for(string::size_type p = 0; p < s.size(); ++p)
	if(s[p] == '\\' || s[p] == ':' || s[p] == '/')
		s[p] = '/';
}

static void clean_rpath(string& s)
{
	for(string::size_type p = 0; p < s.size(); ++p)
	if(s[p] == '\\' || s[p] == ':' || s[p] == '/')
		s[p] = DIR_CHAR;
}


static void split_path(const string& i, string& p, string& f)
{
	string::size_type n=i.rfind('/');
	if(n==i.npos)
	{
		f=i;
		p.clear();
	}else{
		p=i.substr(0,n);
		f=i.substr(n+1);
	}
}

static int is_direct_parent(const string& parent, const string& child)
{
	if(parent.empty()) return child.find('/',1) == child.npos;

	if((parent.size()+1) >= child.size())							return false;	// Not a child if parent is longer than child - remember we need '/' too.
	if(strncasecmp(parent.c_str(),child.c_str(),parent.size()) != 0)return false;	// Not a child if doesn't contain parent path
	if(child[parent.size()] != '/')									return false;	// Not a child if parent name has gunk after it
	if(child.find('/',parent.size()+1) != child.npos)				return false;	// Not a child if child contains subdirs beyond parent
																	return true;
}

WED_LibraryMgr::WED_LibraryMgr(const string& ilocal_package) : local_package(ilocal_package)
{
	DebugAssert(gPackageMgr != NULL);
	gPackageMgr->AddListener(this);
	Rescan();
}

WED_LibraryMgr::~WED_LibraryMgr()
{
}

string		WED_LibraryMgr::GetResourceParent(const string& r)
{
	string p,f;
	split_path(r,p,f);
	return p;
}

void		WED_LibraryMgr::GetResourceChildren(const string& r, int filter_package, vector<string>& children)
{
	children.clear();
	res_map_t::iterator me = r.empty() ? res_table.begin() : res_table.find(r);
	if(me == res_table.end())						return;
	if(me->second.res_type != res_Directory) 		return;

	if (!r.empty())
		++me;

	while(me != res_table.end())
	{
		if(me->first.size() < r.size())								break;
		if(strncasecmp(me->first.c_str(),r.c_str(),r.size()) != 0)	break;
		// Ben says: in WED 1.3 we'll get more clever about this and optionally show library innards.  But for now, just hide our privates.
		if(me->second.status == status_Public)
		if(is_direct_parent(r,me->first))
		{
			bool want_it = true;
			switch(filter_package) {
			case pack_Library:		want_it = me->second.packages.size() > 1 || !me->second.packages.count(pack_Local);	// Lib if we are in two packs or we are NOT in local.  (We are always SOMEWHERE)
			case pack_All:			break;
			case pack_Local:
			default:				want_it = me->second.packages.count(filter_package);
			}
			if(want_it)
			{
				children.push_back(me->first);
			}
		}
		++me;
	}
}

int			WED_LibraryMgr::GetResourceType(const string& r)
{
	res_map_t::iterator me = res_table.find(r);
	if (me==res_table.end()) return res_None;
	return me->second.res_type;
}

string		WED_LibraryMgr::GetResourcePath(const string& r)
{
	string fixed(r);
	for(string::size_type p = 0; p < fixed.size(); ++p)
	if(fixed[p] == ':' || fixed[p] == '\\')
		fixed[p] = '/';
	res_map_t::iterator me = res_table.find(fixed);
	if (me==res_table.end()) return string();
	return me->second.real_path;
}

bool	WED_LibraryMgr::IsResourceDefault(const string& r)
{
	string fixed(r);
	for(string::size_type p = 0; p < fixed.size(); ++p)
	if(fixed[p] == ':' || fixed[p] == '\\')
		fixed[p] = '/';
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return me->second.is_default;	
}


string		WED_LibraryMgr::CreateLocalResourcePath(const string& r)
{
	return gPackageMgr->ComputePath(local_package, r);
}

void	WED_LibraryMgr::ReceiveMessage(
						GUI_Broadcaster *		inSrc,
						intptr_t				inMsg,
						intptr_t				inParam)
{
	if (inMsg == msg_SystemFolderChanged ||
		inMsg == msg_SystemFolderUpdated)
	{
		Rescan();
	}
}

struct local_scan_t {
	string	partial;
	string	full;
	WED_LibraryMgr * who;
};

void		WED_LibraryMgr::Rescan()
{
	res_table.clear();

	int np = gPackageMgr->CountPackages();
	for(int p = 0; p < np; ++p)
	{
		string pack_base;
		gPackageMgr->GetNthPackagePath(p,pack_base);
		pack_base += DIR_STR "Library.txt";

		bool is_default_pack = gPackageMgr->IsPackageDefault(p);

		MFMemFile * lib = MemFile_Open(pack_base.c_str());
		if(lib)
		{
			gPackageMgr->GetNthPackagePath(p,pack_base);
			
			MFScanner	s;
			MFS_init(&s, lib);

			int cur_status = status_Public;

			int lib_version[] = { 800, 0 };
			if(MFS_xplane_header(&s,lib_version,"LIBRARY",NULL))
			while(!MFS_done(&s))
			{
				string vpath, rpath;

				if(MFS_string_match(&s,"PUBLIC",true))		cur_status = status_Public;
				if(MFS_string_match(&s,"PRIVATE",true))		cur_status = status_Private;
				if(MFS_string_match(&s,"DEPRECATED",true))	cur_status = status_Deprecated;

				if(MFS_string_match(&s,"EXPORT",false))
				{
					MFS_string(&s,&vpath);
					MFS_string(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);
					rpath=pack_base+DIR_STR+rpath;
					AccumResource(vpath, p, rpath,false,is_default_pack, cur_status);
				}

				if(MFS_string_match(&s,"EXPORT_EXTEND",false))
				{
					MFS_string(&s,&vpath);
					MFS_string(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);
					rpath=pack_base+DIR_STR+rpath;
					AccumResource(vpath, p, rpath,false,is_default_pack, cur_status);
				}

				if(MFS_string_match(&s,"EXPORT_EXCLUDE",false))
				{
					MFS_string(&s,&vpath);
					MFS_string(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);
					rpath=pack_base+DIR_STR+rpath;
					AccumResource(vpath, p, rpath,false,is_default_pack, cur_status);
				}

				if(MFS_string_match(&s,"EXPORT_BACKUP",false))
				{
					MFS_string(&s,&vpath);
					MFS_string(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);
					rpath=pack_base+DIR_STR+rpath;
					AccumResource(vpath, p, rpath,true,is_default_pack, cur_status);
				}

				if(MFS_string_match(&s,"EXPORT_RATIO",false))
				{
					MFS_int(&s);
					MFS_string(&s,&vpath);
					MFS_string(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);
					rpath=pack_base+DIR_STR+rpath;
					AccumResource(vpath, p, rpath,false,is_default_pack, cur_status);
				}
				MFS_string_eol(&s,NULL);
			}

			MemFile_Close(lib);
		}
	}

	string package_base;
	package_base=gPackageMgr->ComputePath(local_package,"");
	if(!package_base.empty())
	{
		package_base.erase(package_base.length()-1);

		local_scan_t info;
		info.who = this;
		info.full = package_base;
		MF_IterateDirectory(package_base.c_str(), AccumLocalFile, reinterpret_cast<void*>(&info));
	}

	BroadcastMessage(msg_LibraryChanged,0);
}

void WED_LibraryMgr::AccumResource(const string& path, int package, const string& rpath, bool is_backup, bool is_default, int status)
{
	int								rt = res_None;
	if(HasExtNoCase(path, ".obj"))	rt = res_Object;
	if(HasExtNoCase(path, ".agp"))	rt = res_Object;
	if(HasExtNoCase(path, ".fac"))	rt = res_Facade;
	if(HasExtNoCase(path, ".for"))	rt = res_Forest;
	if(HasExtNoCase(path, ".str"))	rt = res_String;
	if(HasExtNoCase(path, ".ags"))	rt = res_Polygon;
	if(HasExtNoCase(path, ".lin"))	rt = res_Line;
	if(HasExtNoCase(path, ".pol"))	rt = res_Polygon;
	if(HasExtNoCase(path, ".agb"))	rt = res_Polygon;
	if(rt == res_None) return;

	string p(path);
	while(!p.empty())
	{
		res_map_t::iterator i = res_table.find(p);
		if(i == res_table.end())
		{
			res_info_t new_info;
			new_info.status = status;
			new_info.res_type = rt;
			new_info.packages.insert(package);
			new_info.real_path = rpath;
			new_info.is_backup = is_backup;
			new_info.is_default = is_default;
			res_table.insert(res_map_t::value_type(p,new_info));
		}
		else
		{
			DebugAssert(i->second.res_type == rt);
			i->second.packages.insert(package);
			i->second.status = max(i->second.status, status);	// upgrade status if we just found a public version!
			if(i->second.is_backup && !is_backup)
			{
				i->second.is_backup = false;
				i->second.real_path = rpath;
			}
			if(is_default && !i->second.is_default)
				i->second.is_default = true;
		}

		string par, f;
		split_path(p,par,f);
		p = par;
		rt = res_Directory;
	}
}

bool WED_LibraryMgr::AccumLocalFile(const char * filename, bool is_dir, void * ref)
{
	local_scan_t * info = reinterpret_cast<local_scan_t *>(ref);
	if(is_dir)
	{
		if(strcmp(filename,".") != 0 &&
		   strcmp(filename,"..") != 0)
		{
			local_scan_t sub_info;
			sub_info.who = info->who;
			sub_info.partial = info->partial + "/" + filename;
			sub_info.full = info->full + DIR_STR + filename;
			MF_IterateDirectory(sub_info.full.c_str(), AccumLocalFile, reinterpret_cast<void*>(&sub_info));
		}
	}
	else
	{
		string r = info->partial + "/" + filename;
		string f = info->full + DIR_STR + filename;
		r.erase(0,1);
		info->who->AccumResource(r, pack_Local, f,false,false, status_Public);
	}
	return false;
}
