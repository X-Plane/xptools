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

// checks if path includes enough '..' to possibly not be a true subdirectory of the current directory
// i.e. dir/../x  or  d/../x or ./x      are fine
//      ../x  or  dir/../../x  or ./../x  or  dir/./../../x get flagged
static bool is_no_true_subdir_path(string& s)
{
	int subdir_levels = 0;
	for(string::size_type p = 0; p < s.size(); ++p)
		if(s[p] == '\\' || s[p] == ':' || s[p] == '/')
		{
			if (p>=1 && s[p-1] != '.')
				subdir_levels++;
			else if (p>=2 && s[p-1] == '.')
			{
				if (s[p-2] == '.')
			  		subdir_levels--;
				else if (s[p-2] != '\\' && s[p-2] != ':' && s[p-2] != '/')
					subdir_levels++;
			}
			
			if (subdir_levels < 0)
				return true;
		}
	return false;
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

//Library manager constructor
WED_LibraryMgr::WED_LibraryMgr(const string& ilocal_package) : local_package(ilocal_package)
{
	DebugAssert(gPackageMgr != NULL);
	gPackageMgr->AddListener(this);
	Rescan();
}

WED_LibraryMgr::~WED_LibraryMgr()
{
}

string WED_LibraryMgr::GetLocalPackage() const
{
	return local_package;
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
		// Ben says: even in WED 1.6 we still don't show private or deprecated stuff
		if(me->second.status >= status_Public)
		if(is_direct_parent(r,me->first))
		{
			bool want_it = true;
			switch(filter_package) {
			case pack_Library:		want_it = me->second.packages.size() > 1 || !me->second.packages.count(pack_Local);	// Lib if we are in two packs or we are NOT in local.  (We are always SOMEWHERE)
			case pack_All:			break;
			case pack_Default:		want_it = me->second.is_default;	break;
			case pack_New:			want_it = me->second.status == status_New; break;
			case pack_Local:		// Since "local" is a virtal index, the search for Nth pack works for local too.
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

bool	WED_LibraryMgr::IsResourceLocal(const string& r)
{
	string fixed(r);
	for(string::size_type p = 0; p < fixed.size(); ++p)
	if(fixed[p] == ':' || fixed[p] == '\\')
		fixed[p] = '/';
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return me->second.packages.count(pack_Local) && me->second.packages.size() == 1;
}

bool	WED_LibraryMgr::IsResourceLibrary(const string& r)
{
	string fixed(r);
	for(string::size_type p = 0; p < fixed.size(); ++p)
	if(fixed[p] == ':' || fixed[p] == '\\')
		fixed[p] = '/';
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return !me->second.packages.count(pack_Local) || me->second.packages.size() > 1;
}

bool	WED_LibraryMgr::IsResourceDeprecatedOrPrivate(const string& r)
{
	string fixed(r);
	for(string::size_type p = 0; p < fixed.size(); ++p)
	if(fixed[p] == ':' || fixed[p] == '\\')
		fixed[p] = '/';
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return me->second.status < status_Public;
}

bool		WED_LibraryMgr::DoesPackHaveLibraryItems(int package)
{
	for(res_map_t::iterator i = res_table.begin(); i != res_table.end(); ++i)
		if(i->second.packages.count(package))
		{
//	The problem here is that a resource can be defined in multiple libraries,
//  some of those definitions may be deprecated or private, but others not.
//  If there is at least one public definition, the resource has status >= status_Public.
//  So its impossible to find out this way if a given library has no public items ...

// if  (i->second.status > status_Public)
//			printf("Pack %d '%s' status = %d\n",package,i->second.real_path.c_str(),i->second.status);
//			if ( i->second.status >= status_Public)	

			return true;
		}
	return false;
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
	//Clear the reasource table
	res_table.clear();

	//Number of packages?
	int np = gPackageMgr->CountPackages();

	//For the number of packages
	for(int p = 0; p < np; ++p)
	{
		//the physical directory of the scenery pack
		string pack_base;
		//Get the pack's physical location
		gPackageMgr->GetNthPackagePath(p,pack_base);
		
		//concatinate string
		pack_base += DIR_STR "library.txt";

		//
		bool is_default_pack = gPackageMgr->IsPackageDefault(p);

		//Connects the physical Library.txt to the virual Memory File system? (95% sure) -Ted
		MFMemFile * lib = MemFile_Open(pack_base.c_str());

		//If there is a lib file
		if(lib)
		{
			//Get the package again
			gPackageMgr->GetNthPackagePath(p,pack_base);
			
			//Create a memory scanner
			MFScanner	s;

			//Initialize the Memory File System
			MFS_init(&s, lib);

			int cur_status = status_Public;

			//Set the library version
			int lib_version[] = { 800, 0 };

			
			if(MFS_xplane_header(&s,lib_version,"LIBRARY",NULL))
			while(!MFS_done(&s))
			{
				string vpath, rpath;

				bool is_export_export  = MFS_string_match(&s,"EXPORT",false);
				bool is_export_extend  = MFS_string_match(&s,"EXPORT_EXTEND",false);
				bool is_export_exclude = MFS_string_match(&s,"EXPORT_EXCLUDE",false);
				bool is_export_backup  = MFS_string_match(&s,"EXPORT_BACKUP",false);

				if( is_export_export  ||
					is_export_extend  ||
					is_export_exclude ||
					is_export_backup)
				{
					MFS_string(&s,&vpath);
					MFS_string_eol(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);

					if (is_no_true_subdir_path(rpath)) break; // ignore paths that lead outside current scenery directory
					rpath=pack_base+DIR_STR+rpath;
					AccumResource(vpath, p, rpath, is_export_backup, is_default_pack, cur_status);
				}
				else if(MFS_string_match(&s,"EXPORT_RATIO",false))
				{
				    double x = MFS_double(&s);
					MFS_string(&s,&vpath);
					MFS_string_eol(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);
					if (is_no_true_subdir_path(rpath)) break; // ignore paths that lead outside current scenery directory
					rpath=pack_base+DIR_STR+rpath;
					AccumResource(vpath, p, rpath,false,is_default_pack, cur_status);
				}
				else
				{
					if(MFS_string_match(&s,"PUBLIC",true))
					{	
						cur_status = status_Public;
						
						int new_until = 0;
						new_until = MFS_int(&s);
						if (new_until > 20170101)
						{
							time_t rawtime;
							struct tm * timeinfo;
							time (&rawtime);
							timeinfo = localtime (&rawtime);							
							int now = 10000 * (timeinfo->tm_year+1900) +100*timeinfo->tm_mon + timeinfo->tm_mday;
							if (new_until >= now)
							{
								cur_status = status_New;
							}
						}
					}
					else if(MFS_string_match(&s,"PRIVATE",true))    
						cur_status = status_Private;
					else if(MFS_string_match(&s,"DEPRECATED",true)) 
						cur_status = status_Deprecated;
						
					MFS_string_eol(&s,NULL);
				}
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
#if ROAD_EDITING
	if(HasExtNoCase(path, ".net"))	rt = res_Road;
#endif
	if(rt == res_None) return;

	if (package >= 0 && status >= status_Public) gPackageMgr->HasPublicItems(package);

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
