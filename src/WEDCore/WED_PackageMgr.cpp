/*
 * Copyright (c) 2007, Laminar Research.
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

#include "WED_PackageMgr.h"
#include "PlatformUtils.h"
#include "WED_Errors.h"
#include "MemFileUtils.h"
#include "FileUtils.h"
#include "AssertUtils.h"
#include "WED_Messages.h"

#define CUSTOM_PACKAGE_PATH	"Custom Scenery"
#define GLOBAL_PACKAGE_PATH	"Global Scenery"
#define DEFAULT_PACKAGE_PATH "Resources" DIR_STR "default scenery"

WED_PackageMgr * gPackageMgr = NULL;

struct WED_PackageInfo
{
	string name;
	bool hasAnyItems;
	bool hasPublicItems;
	bool hasXML;
	bool hasAPT;
	bool isDisabled;
	WED_PackageInfo(const char * n) : name(n), hasPublicItems(false), hasAnyItems(false), hasXML(false), hasAPT(false), isDisabled(false) { }
};

WED_PackageMgr::WED_PackageMgr(const char * in_xplane_folder) : system_exists(false)
{
	DebugAssert(gPackageMgr==NULL);
	gPackageMgr=this;
	if(in_xplane_folder)
	SetXPlaneFolder(in_xplane_folder);
}

WED_PackageMgr::~WED_PackageMgr()
{
	DebugAssert(gPackageMgr==this);
	gPackageMgr=NULL;
}

bool		WED_PackageMgr::HasSystemFolder(void) const
{
	return system_exists;
}

bool		WED_PackageMgr::GetXPlaneFolder(string& root) const
{
	root = system_path;
	return system_exists;
}

bool		WED_PackageMgr::AccumAnyDir(const char * fileName, bool isDir, void * ref)
{
	if(isDir && fileName[0] != '.') 
	{
		vector<WED_PackageInfo> * container = (vector<WED_PackageInfo> *) ref;
		container->push_back(fileName);
	}
	return false;
}

static void analyze_package(const string& abspath, WED_PackageInfo& pkginfo)
{
	string fp(abspath);
	
	if(FILE_exists((fp + DIR_STR "library.txt").c_str()))
		pkginfo.hasAnyItems = true;
	if(FILE_exists((fp + DIR_STR "earth.wed.xml").c_str()))
		pkginfo.hasXML = true;
	if(FILE_exists((fp + DIR_STR "Earth nav data" + DIR_STR + "apt.dat").c_str()))
		pkginfo.hasAPT = true;
}

struct package_local_scan_t {
	string	fullpath;
	vector<WED_PackageInfo> * who;
};

bool		WED_PackageMgr::AccumLibDir(const char * fileName, bool isDir, void * ref)
{
	if(isDir && fileName[0] != '.')
	{
		package_local_scan_t * info = reinterpret_cast<package_local_scan_t *>(ref);
		if(info) 
		{
			info->who->push_back(fileName);
			analyze_package(info->fullpath + DIR_STR + fileName, info->who->back());
// printf("scan %s for %s = %d\n",fileName, f.c_str(),  info->who->back().hasPublicItems);
		}
	}
	return false;  // keep looking for more such files in this directory
}

bool		WED_PackageMgr::SetXPlaneFolder(const string& root)
{
	// Check for the presence of the two folders WED really cares about.
	string dir = root + DIR_STR + CUSTOM_PACKAGE_PATH;
	if (MF_GetFileType(dir.c_str(),mf_CheckType) != mf_Directory)
		return false;

	dir = root + DIR_STR + DEFAULT_PACKAGE_PATH;
	if (MF_GetFileType(dir.c_str(),mf_CheckType) != mf_Directory)
		return false;
	
	system_path = root;
	Rescan(true);
	return true;
}

int			WED_PackageMgr::CountCustomPackages(void) const
{
	return custom_packages.size();
}

int			WED_PackageMgr::CountPackages(void) const
{
	return custom_packages.size() +
		   global_packages.size() +
		   default_packages.size();
}

void		WED_PackageMgr::GetNthPackageName(int n, string& package) const
{
	if (n < custom_packages.size())	{ package = custom_packages[n].name; return; }
	n -= custom_packages.size();

	if (n < global_packages.size())	{ package = global_packages[n].name; return; }
	n -= global_packages.size();

	package = default_packages[n].name;
}

void		WED_PackageMgr::GetNthPackagePath(int n, string& package) const
{
	if (n < custom_packages.size())	{ package = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR +  custom_packages[n].name; return; }
	n -= custom_packages.size();

	if (n < global_packages.size())	{ package = system_path + DIR_STR GLOBAL_PACKAGE_PATH DIR_STR + global_packages[n].name; return; }
	n -= global_packages.size();

	package = system_path + DIR_STR DEFAULT_PACKAGE_PATH DIR_STR + default_packages[n].name;
}

bool		WED_PackageMgr::IsPackageDefault(int n) const
{
	return n >= (custom_packages.size() + global_packages.size());
}

bool		WED_PackageMgr::HasXML(int n) const
{
	if (n < custom_packages.size())	
		return custom_packages[n].hasXML;
	else 
		return false;
}

bool		WED_PackageMgr::HasAPT(int n) const
{
	if (n < custom_packages.size())	
		return custom_packages[n].hasAPT;
	else 
		return false;
}

bool		WED_PackageMgr::HasLibrary(int n) const
{
	if (n < custom_packages.size())	
		return custom_packages[n].hasAnyItems;
	else 
		return false;
}

bool		WED_PackageMgr::IsDisabled(int n) const
{
	if (n < custom_packages.size())	
		return custom_packages[n].isDisabled;
	else 
		return false;
}

bool		WED_PackageMgr::HasPublicItems(int n) const
{
	if (n < custom_packages.size())	return custom_packages[n].hasPublicItems;
	n -= custom_packages.size();

	if (n < global_packages.size())	return global_packages[n].hasPublicItems;
	n -= global_packages.size();

	return default_packages[n].hasPublicItems;
}

void		WED_PackageMgr::AddPublicItems(int n)
{
	if (n < custom_packages.size())	{ custom_packages[n].hasPublicItems = true; return; }
	n -= custom_packages.size();

	if (n < global_packages.size())	{ global_packages[n].hasPublicItems = true; return; }
	n -= global_packages.size();

	default_packages[n].hasPublicItems = true;
}

void		WED_PackageMgr::RenameCustomPackage(int n, const string& new_name)
{
	string oldn = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR + custom_packages[n].name;
	string newn = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR + new_name;
	int res = FILE_rename_file(oldn.c_str(), newn.c_str());
	if (res != 0)
	{
		wed_error_exception e(res, __FILE__ , __LINE__);
		WED_ReportExceptionUI(e, "Unable to rename package %s to %s",oldn.c_str(), newn.c_str());
	} else
		custom_packages[n].name = new_name;

	BroadcastMessage(msg_SystemFolderUpdated,0);
}

int WED_PackageMgr::CreateNewCustomPackage(void)
{
	int n = 0;
	string name;
	string path;
	char buf[256];
	do {
		++n;
		sprintf(buf,"Untitled %d",n);
		name = buf;
		path = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR + name;

		int found_in_our_list = 0;
		for(int p = 0; p < custom_packages.size(); ++p)
		if (strcasecmp(name.c_str(), custom_packages[n].name.c_str()) == 0)
		{
			found_in_our_list = 1;
			break;
		}
		if (found_in_our_list)
			continue;

		if (MF_GetFileType(path.c_str(), mf_CheckType) != mf_BadFile)
			continue;

		if (FILE_make_dir(path.c_str()) != 0)
		{
			DoUserAlert("ERROR: unable to create a new scenery package.  Make sure you have write access to your x-system folder.");
			return -1;
		}

		custom_packages.push_back(name.c_str());
		BroadcastMessage(msg_SystemFolderUpdated,0);
		return custom_packages.size()-1;
	} while (1);
	return -1;
}

static bool SortPackageList(const WED_PackageInfo& p1, const WED_PackageInfo& p2) 
{ 
//	if(p1.hasXML != p2.hasXML) return p1.hasXML;     // packages with earth.wed.xml come first
//	if(p1.hasAPT != p2.hasAPT) return p1.hasAPT;     // then all package with any apt.dat

	bool p1_inclAPT = p1.hasXML || p1.hasAPT;
	bool p2_inclAPT = p2.hasXML || p2.hasAPT;
	if(p1_inclAPT != p2_inclAPT) return p1_inclAPT;              // packages with any airport come first
	if(p1.hasAnyItems != p2.hasAnyItems && !(p1_inclAPT || p2_inclAPT)) return p2.hasAnyItems;  // pure Libraries come last
	return strcasecmp(p1.name.c_str(), p2.name.c_str()) < 0; 
}

void		WED_PackageMgr::Rescan(bool alwaysBroadcast)
{

	bool pkg_list_changed = alwaysBroadcast;
	system_exists=false;
	if (MF_GetFileType(system_path.c_str(),mf_CheckType) == mf_Directory)
	{
		string cus_dir = system_path + DIR_STR CUSTOM_PACKAGE_PATH;

		if (MF_GetFileType(cus_dir.c_str(),mf_CheckType) == mf_Directory)
		{
			vector<WED_PackageInfo> old_cust_packages;
			old_cust_packages.swap(custom_packages);

			system_exists=true;
			package_local_scan_t info;
			info.fullpath = cus_dir;
			info.who = &custom_packages;
			MF_IterateDirectory(cus_dir.c_str(), AccumLibDir, (void*) &info);
			
			vector<string> disabledSceneries;
			MFMemFile * ini = MemFile_Open((cus_dir + DIR_STR "scenery_packs.ini").c_str());
			if(ini)
			{
				MFScanner	s;
				MFS_init(&s, ini);
				int versions[] = { 1000, 0 };
				if(MFS_xplane_header(&s,versions,"SCENERY",NULL))
				{
					while(!MFS_done(&s))
					{
						if (MFS_string_match(&s,"SCENERY_PACK_DISABLED", false))
						{
							string dis;
							MFS_string_eol(&s,&dis);
							size_t  p_end = dis.find_last_of("\\/");
							size_t  p_beg = dis.find_last_of("\\/",p_end-1);
							disabledSceneries.push_back(dis.substr(p_beg+1,p_end-p_beg-1));
						}
						MFS_string_eol(&s,NULL);
					}
				}
				MemFile_Close(ini);
			
				for(auto dis : disabledSceneries)
					for(auto& scn : custom_packages)
						if(scn.name == dis) scn.isDisabled = true;
			}

			sort(custom_packages.begin(),custom_packages.end(),SortPackageList);

			if(old_cust_packages.size() == custom_packages.size())
			{
				auto  o = old_cust_packages.begin();
				for(auto n : custom_packages)
				{
					if(o->name != n.name) pkg_list_changed = true;
					else
					{
						if(o->isDisabled != n.isDisabled) pkg_list_changed = true;
						if(o->hasAnyItems != o->hasAnyItems) pkg_list_changed = true;
					}
					o++;
				}
			}
			else
				pkg_list_changed = true;

			if(!pkg_list_changed) custom_packages.swap(old_cust_packages);
		}

		if(pkg_list_changed)
		{
			string glb_dir = system_path + DIR_STR GLOBAL_PACKAGE_PATH;
			global_packages.clear();
			if (MF_GetFileType(glb_dir.c_str(),mf_CheckType) == mf_Directory)
			{
				system_exists=true;
				MF_IterateDirectory(glb_dir.c_str(), AccumAnyDir, &global_packages);
				sort(global_packages.begin(),global_packages.end(),SortPackageList);
			}

			string def_dir = system_path + DIR_STR DEFAULT_PACKAGE_PATH;
			default_packages.clear();
			if (MF_GetFileType(def_dir.c_str(),mf_CheckType) == mf_Directory)
			{
				system_exists=true;
				MF_IterateDirectory(def_dir.c_str(), AccumAnyDir, &default_packages);
				sort(default_packages.begin(),default_packages.end(),SortPackageList);
			}

			BroadcastMessage(msg_SystemFolderChanged,0);
		}
	}

	XPversion = "Unknown";
	string logfile_name = system_path + DIR_STR "Log.txt";
	string logfile_contents;
	if (FILE_exists(logfile_name.c_str()) &&
		FILE_read_file_to_string(logfile_name, logfile_contents) == 0)
	{
		size_t v_pos = logfile_contents.find("X-Plane");  // version string is the one behind the X-plane keyword
		if (v_pos != string::npos && v_pos < 100)
		{
			char v[16];
			sscanf(logfile_contents.c_str()+v_pos+8,"%15s",v);
			XPversion = v;
		}
	}
}

string		WED_PackageMgr::ComputePath(const string& package, const string& rel_file) const
{
	if(rel_file.size() >= 2 && rel_file[1] == ':') return rel_file;
	return system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR + package + DIR_STR + rel_file;
}

string		WED_PackageMgr::ReducePath(const string& package, const string& full_file) const
{
	string prefix = ComputePath(package,string());
	string partial(full_file);

#if IBM
	if(prefix.size() >= 2 && partial.size() >= 2 &&
		prefix[1] == ':' && partial[1] == ':' &&
		prefix[0] != partial[0]) return full_file;
#endif

	int n = 0;
	do {
		int p = prefix.find_first_of("\\/:", n);
		if(p == prefix.npos) break;
		++p;
		if(p != prefix.npos && p <= prefix.size() && p <= partial.size() && strncasecmp(prefix.c_str(), partial.c_str(), p) == 0)
			n = p;
		else
			break;
	} while(1);

	prefix.erase(0,n);
	partial.erase(0,n);

	while(!prefix.empty())
	{
		string::size_type chop = prefix.find_first_of("\\/:");
		if (chop == prefix.npos) break;
		prefix.erase(0,chop+1);
		partial = string("../") + partial;
	}
	return partial;
}

const char * WED_PackageMgr::GetXPversion() const
{
	return XPversion.c_str();
}

bool		WED_PackageMgr::IsSameXPVersion( const string& version) const
{
	return (XPversion == version);   // might have to refine that, i.e. consider various all release candidates to be equivalent ?
}
