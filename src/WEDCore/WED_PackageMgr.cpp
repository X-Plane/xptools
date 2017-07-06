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
#include "FileUtils.h"

#define CUSTOM_PACKAGE_PATH	"Custom Scenery"
#define GLOBAL_PACKAGE_PATH	"Global Scenery"
#define DEFAULT_PACKAGE_PATH "Resources" DIR_STR "default scenery"

WED_PackageMgr * gPackageMgr = NULL;

WED_PackageMgr::WED_PackageMgr(const char *		in_xplane_folder)
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

static bool package_scan_func(const char * fileName, bool is_dir, void * ref)
{
	vector<string> * container = (vector<string> *) ref;
	if(is_dir && fileName[0] != '.') container->push_back(fileName);
	return false;
}

void		WED_PackageMgr::SetXPlaneFolder(const string& root)
{
	system_path = root;
	Rescan();
}

int			WED_PackageMgr::CountCustomPackages(void) const
{
	return custom_package_names.size();
}

void		WED_PackageMgr::GetNthCustomPackageName(int n, string& package) const
{
	package = custom_package_names[n];
}

void		WED_PackageMgr::GetNthCustomPackagePath(int n, string& package) const
{
	package = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR + custom_package_names[n];
}

int			WED_PackageMgr::CountPackages(void) const
{
	return custom_package_names.size() +
		   global_package_names.size() +
		   default_package_names.size();
}

void		WED_PackageMgr::GetNthPackageName(int n, string& package) const
{
	if (n < custom_package_names.size())	{ package = custom_package_names[n]; return; }
	n -= custom_package_names.size();

	if (n < global_package_names.size())	{ package = global_package_names[n]; return; }
	n -= global_package_names.size();

	package = default_package_names[n];
}

void		WED_PackageMgr::GetNthPackagePath(int n, string& package) const
{
	if (n < custom_package_names.size())	{ package = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR +  custom_package_names[n]; return; }
	n -= custom_package_names.size();

	if (n < global_package_names.size())	{ package = system_path + DIR_STR GLOBAL_PACKAGE_PATH DIR_STR + global_package_names[n]; return; }
	n -= global_package_names.size();

	package = system_path + DIR_STR DEFAULT_PACKAGE_PATH DIR_STR + default_package_names[n];
}

bool		WED_PackageMgr::IsPackageDefault(int n) const
{
	return n >= (custom_package_names.size() + global_package_names.size());
}

bool		WED_PackageMgr::IsPackagePublicItems(int n) const
{
	if (n < custom_package_names.size())	return custom_package_hasPublicItems[n];
	n -= custom_package_names.size();

	if (n < global_package_names.size())	return global_package_hasPublicItems[n];
	n -= global_package_names.size();

	return default_package_hasPublicItems[n];
}

void		WED_PackageMgr::HasPublicItems(int n)
{
	if (n < custom_package_names.size())	{ custom_package_hasPublicItems[n] = true; return; }
	n -= custom_package_names.size();

	if (n < global_package_names.size())	{ global_package_hasPublicItems[n] = true; return; }
	n -= global_package_names.size();

	default_package_hasPublicItems[n] = true;
}

void		WED_PackageMgr::RenameCustomPackage(int n, const string& new_name)
{
	string oldn = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR + custom_package_names[n];
	string newn = system_path + DIR_STR CUSTOM_PACKAGE_PATH DIR_STR + new_name;
	int res = FILE_rename_file(oldn.c_str(), newn.c_str());
	if (res != 0)
	{
		wed_error_exception e(res, __FILE__ , __LINE__);
		WED_ReportExceptionUI(e, "Unable to rename package %s to %s",oldn.c_str(), newn.c_str());
	} else
		custom_package_names[n] = new_name;

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
		for(int p = 0; p < custom_package_names.size(); ++p)
		if (strcasecmp(name.c_str(), custom_package_names[p].c_str()) == 0)
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

		custom_package_names.push_back(name);
		custom_package_hasPublicItems.push_back(false);  // that may not always be true, but rescan() will fix that later
		BroadcastMessage(msg_SystemFolderUpdated,0);
		return custom_package_names.size()-1;
	} while (1);
	return -1;
}

static bool CompareNoCase(const string& s1, const string& s2) { return strcasecmp(s1.c_str(), s2.c_str()) < 0; }

void		WED_PackageMgr::Rescan(void)
{
	custom_package_names.clear();
	global_package_names.clear();
	default_package_names.clear();
	
	system_exists=false;
	if (MF_GetFileType(system_path.c_str(),mf_CheckType) == mf_Directory)
	{
		string cus_dir = system_path + DIR_STR CUSTOM_PACKAGE_PATH;
		if (MF_GetFileType(cus_dir.c_str(),mf_CheckType) == mf_Directory)
		{
			system_exists=true;
			MF_IterateDirectory(cus_dir.c_str(), package_scan_func, &custom_package_names);
			sort(custom_package_names.begin(),custom_package_names.end(),CompareNoCase);
		}

		string glb_dir = system_path + DIR_STR GLOBAL_PACKAGE_PATH;
		if (MF_GetFileType(glb_dir.c_str(),mf_CheckType) == mf_Directory)
		{
			system_exists=true;
			MF_IterateDirectory(glb_dir.c_str(), package_scan_func, &global_package_names);
			sort(global_package_names.begin(),global_package_names.end(),CompareNoCase);
		}

		string def_dir = system_path + DIR_STR DEFAULT_PACKAGE_PATH;
		if (MF_GetFileType(def_dir.c_str(),mf_CheckType) == mf_Directory)
		{
			system_exists=true;
			MF_IterateDirectory(def_dir.c_str(), package_scan_func, &default_package_names);
			sort(default_package_names.begin(),default_package_names.end(),CompareNoCase);
		}
	}
	
	custom_package_hasPublicItems.clear();
	global_package_hasPublicItems.clear();
	default_package_hasPublicItems.clear();
	
	for (int i=0; i<custom_package_names.size(); ++i)
			custom_package_hasPublicItems.push_back(false);
	for (int i=0; i<global_package_names.size(); ++i)
			global_package_hasPublicItems.push_back(false);
	for (int i=0; i<default_package_names.size(); ++i)
			default_package_hasPublicItems.push_back(false);

	BroadcastMessage(msg_SystemFolderChanged,0);
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
		if(p != prefix.npos && p <= prefix.size() && p <= partial.size() && strncmp(prefix.c_str(), partial.c_str(), p) == 0)
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
