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
#include "WED_EnumSystem.h"
#include "AssertUtils.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "MemFileUtils.h"
#include <time.h>

static void clean_vpath(string& s)
{
	for(string::size_type p = 0; p < s.size(); ++p)
		if(s[p] == '\\' || s[p] == ':')
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

bool WED_LibraryMgr::GetLineVpath(int lt, string& vpath)
{
	map<int,string>::iterator l = default_lines.find(lt);
	if(l == default_lines.end())
		return false;
	else
	{
		vpath = l->second;
		return true;
	}
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

string		WED_LibraryMgr::GetResourcePath(const string& r, int variant)
{
	string fixed(r);
	clean_vpath(fixed);
	res_map_t::iterator me = res_table.find(fixed);
	if (me==res_table.end()) return string();
	DebugAssert(variant < me->second.real_paths.size());
	return me->second.real_paths[variant];
}

bool	WED_LibraryMgr::IsResourceDefault(const string& r)
{
	string fixed(r);
	clean_vpath(fixed);
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return me->second.is_default;
}

bool	WED_LibraryMgr::IsResourceLocal(const string& r)
{
	string fixed(r);
	clean_vpath(fixed);
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return me->second.packages.count(pack_Local) && me->second.packages.size() == 1;
}

bool	WED_LibraryMgr::IsResourceLibrary(const string& r)
{
	string fixed(r);
	clean_vpath(fixed);
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return !me->second.packages.count(pack_Local) || me->second.packages.size() > 1;
}

bool	WED_LibraryMgr::IsResourceDeprecatedOrPrivate(const string& r)
{
	string fixed(r);
	clean_vpath(fixed);
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return false;
	return me->second.status < status_Yellow;                  // status "Yellow' is still deemed public wrt validation, i.e. allowed on the gateway
}

bool	WED_LibraryMgr::DoesPackHaveLibraryItems(int package)
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

int		WED_LibraryMgr::GetNumVariants(const string& r)
{
	string fixed(r);
	clean_vpath(fixed);
	res_map_t::const_iterator me = res_table.find(fixed);
	if (me==res_table.end()) return 1;
	return me->second.real_paths.size();
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
		if(gPackageMgr->IsDisabled(p)) continue;
		//the physical directory of the scenery pack
		string pack_base;
		//Get the pack's physical location
		gPackageMgr->GetNthPackagePath(p,pack_base);

		pack_base += DIR_STR "library.txt";

		bool is_default_pack = gPackageMgr->IsPackageDefault(p);

		//Connects the physical Library.txt to the virual Memory File system? (95% sure) -Ted
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
				bool is_export_backup = false;
				
				if( MFS_string_match(&s,"EXPORT",false) ||
				    MFS_string_match(&s,"EXPORT_EXTEND",false) ||
				    MFS_string_match(&s,"EXPORT_EXCLUDE",false) ||
					(is_export_backup  = MFS_string_match(&s,"EXPORT_BACKUP",false)))
				{
					MFS_string(&s,&vpath);
					MFS_string_eol(&s,&rpath);
					clean_vpath(vpath);
					clean_rpath(rpath);

					if (is_no_true_subdir_path(rpath)) break; // ignore paths that lead outside current scenery directory
					rpath=pack_base+DIR_STR+rpath;
					FILE_case_correct( (char *) rpath.c_str());  /* yeah - I know I'm overriding the 'const' protection of the c_str() here.
					   But I know this operation is never going to change the strings length, so thats OK to do.
					   And I have to case-correct the path right here, as this path later is not only used by the case insensitive MF_open()
					   but also to derive the paths to the textures referenced in those assets. And those textures are loaded with case-sensitive fopen.
					   */
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
					FILE_case_correct( (char *) rpath.c_str());  // yeah - I know I'm overriding the 'const' protection of the c_str() here.
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
					else if(MFS_string_match(&s,"SEMI_DEPRECATED",true))
						cur_status = status_Yellow;

					MFS_string_eol(&s,NULL);
				}
			}
			MemFile_Close(lib);
		}
	}
	RescanLines();

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


void WED_LibraryMgr::RescanLines()
{
	vector<int> existing_line_enums;
	DOMAIN_Members(LinearFeature, existing_line_enums);

	set<int> existing_line_types;
	for(vector<int>::iterator e = existing_line_enums.begin(); e != existing_line_enums.end(); ++e)
	{
		existing_line_types.insert(ENUM_Export(*e));
	}
	default_lines.clear();

	res_map_t::iterator m = res_table.begin();
	while(m != res_table.end() && m->first.find("lib/airport/lines/",0) == string::npos )
		++m;

	while(m != res_table.end() && m->first.find("lib/airport/lines/",0) != string::npos )
	{
		string resnam(m->first);
		resnam.erase(0,strlen("lib/airport/lines/"));

		if(resnam[0] >= '0' && resnam[0] <= '9' &&
//		   m->second.is_default &&
		   m->second.status >= status_Public && resnam.substr(resnam.size()-4) == ".lin"  )
		{
			resnam.erase(resnam.size()-4);

			// create human readable Description (also used as XML keyword) from resource file name
			int linetype;
			char nice_name[40];
			sscanf(resnam.c_str(),"%d%*c%29s",&linetype,nice_name);
			for(int i = 0; i < 30; ++i)
			{
				if(nice_name[i] == 0) break;
				if(i == 0) nice_name[0] = toupper(nice_name[0]);
				if(nice_name[i] == '_')
				{
					nice_name[i] = ' ';
					if(nice_name[i+1] != 0)
					{
						nice_name[i+1] = toupper(nice_name[i+1]);
						if(nice_name[i+2] == 0)
						{
							nice_name[i+1] = 0;
							strcat(nice_name,"(Black)");
						}
					}
				}
			}

			if(linetype > 0 && linetype < 100)
			{
				default_lines[linetype] = m->first;
				if(existing_line_types.count(linetype) == 0)
				{
					const char * icon = "line_Unknown";
					// try to find the right icon, in case the particular number wasn't yet added to the ENUMS.h
#if 0
					// that would be nice - but we can't parse the .lin statement here, the ResourceMgr isn't available
					lin_info_t linfo;
					if (rmgr->GetLin(m->first,linfo))
					{
						// determine Chroma & Hue for the preview color line
						float R = linfo.rgb[0], G = linfo.rgb[1], B = linfo.rgb[2];
						float M = fltmax3(R,G,B);
						float m = fltmin3(R,G,B);
						float C = M-m;
						if(C < 0.3)        icon = linetype < 50 ? "line_SolidWite"  : "line_BSolidWhite";
						else
						{
							float H = 0.0;
							if     (R == M) H = fmod((G-B) / C, 6.0);
							else if(G == M) H = (B-R) / C + 2.0;
							else if(B == M) H = (R-G) / C + 4.0;
							H = fltwrap(H * 60.0, 0.0, 360.0);

							if     (H > 330.0 ||
									H < 20.0)  icon = linetype < 50 ? "line_SolidRed"   : "line_BSolidRed";
							else if(H < 45.0)  icon = linetype < 50 ? "line_SolidOrange": "line_BSolidOrange";
							else if(H < 70.0)  icon = linetype < 50 ? "line_SolidYellow": "line_BSolidYellow";
							else if(H < 135.0) icon = linetype < 50 ? "line_SolidGreen" : "line_BSolidGreen";
							else               icon = linetype < 50 ? "line_SolidBlue"  : "line_BSolidBlue";
						}
					}
#else
					for(int i = 0; i < resnam.length(); ++i) resnam[i] = tolower(resnam[i]); // C11 would make this so much easier ...

					if(resnam.find("_red") != string::npos)
					{
					    if(resnam.find("_dash") != string::npos)    icon = linetype < 50 ? "line_BrokenRed" : "line_BBrokenRed";
					    else                                        icon = linetype < 50 ? "line_SolidRed"   : "line_BSolidRed";
					}
					else if(resnam.find("_orange") != string::npos) icon = linetype < 50 ? "line_SolidOrange": "line_BSolidOrange";
					else if(resnam.find("_green") != string::npos)  icon = linetype < 50 ? "line_SolidGreen" : "line_BSolidGreen";
					else if(resnam.find("_blue") != string::npos)   icon = linetype < 50 ? "line_SolidBlue"  : "line_BSolidBlue";
					else if(resnam.find("_yellow") != string::npos || resnam.find("_taxi") != string::npos || resnam.find("_hold") != string::npos)
					{
						if(resnam.find("_hold") != string::npos)
						{
							if(resnam.find("_ils") != string::npos)         icon = linetype < 50 ? "line_ILSHold"   : "line_BILSHold";
							else if(resnam.find("_double") != string::npos ||
							        resnam.find("_runway") != string::npos) icon = linetype < 50 ? "line_RunwayHold": "line_BRunwayHold";
							else if(resnam.find("_taxi") != string::npos)   icon = linetype < 50 ? "line_ILSCriticalCenter" : "line_BILSCriticalCenter";
                            else                                            icon = linetype < 50 ? "line_OtherHold" : "line_BOtherHold";
						}
						else if(resnam.find("_wide") != string::npos) icon = linetype < 50 ? "line_SolidYellowW" : "line_BSolidYellowW";
						else                                          icon = linetype < 50 ? "line_SolidYellow"  : "line_BSolidYellow";
					}
					else if(resnam.find("_white") != string::npos || resnam.find("_road") != string::npos)
					{
					    if(resnam.find("_dash") != string::npos)    icon = "line_BrokenWhite";
					    else                                        icon = linetype < 50 ? "line_SolidWhite" : "line_BSolidWhite";
					}
#endif
					ENUM_Create(LinearFeature, icon, nice_name, linetype);
					existing_line_types.insert(linetype);                      // keep track in case of erroneously supplied duplicate vpath's
				}
			}
		}
		m++;
	}

	m=res_table.begin();
	while(m != res_table.end() && m->first.find("lib/airport/lights/slow/",0) == string::npos )
		++m;

	while(m != res_table.end() && m->first.find("lib/airport/lights/slow/",0) != string::npos )
	{
		string resnam(m->first);
		resnam.erase(0,strlen("lib/airport/lights/slow/"));

		if(resnam[0] >= '0' && resnam[0] <= '9' &&
//		   m->second.is_default &&
		   m->second.status >= status_Public && resnam.substr(resnam.size()-4) == ".str"  )
		{
			resnam.erase(resnam.size()-4);

			// create human readable Description (also used as XML keyword) from resource file name
			int lighttype;
			char nice_name[60];
			sscanf(resnam.c_str(),"%d%*c%29s",&lighttype,nice_name);
			for(int i = 0; i < 30; ++i)
			{
				if(nice_name[i] == 0) break;
				if(i == 0) nice_name[0] = toupper(nice_name[0]);
				if(nice_name[i] == '_')
				{
					nice_name[i] = ' ';
					if(nice_name[i+1] != 0)
					{
						if(strcmp(nice_name+i+1,"G_uni") == 0) strcpy(nice_name+i+1,"(Unidirectional Green)");
						else if(strcmp(nice_name+i+1,"YG_uni") == 0) strcpy(nice_name+i+1,"(Unidirectional Amber/Green)");
						else nice_name[i+1] = toupper(nice_name[i+1]);
					}
				}
			}

			if(lighttype > 100 && lighttype < 200)
			{
				default_lines[lighttype] = m->first;
				if(existing_line_types.count(lighttype) == 0)
				{
					const char * icon = "line_Unknown";
					// try to find the right icon, in case the particular number wasn't yet added to the ENUMS.h
					if(resnam.find("_G_uni") != string::npos)       icon = "line_TaxiCenterUni";
					else if(resnam.find("_YG_uni") != string::npos) icon = "line_HoldShortCenterUni";

					ENUM_Create(LinearFeature, icon, nice_name, lighttype);
					existing_line_types.insert(lighttype);                      // keep track in case of erroneously supplied duplicate vpath's
				}
			}
		}
		m++;
	}
}

void WED_LibraryMgr::AccumResource(const string& path, int package, const string& rpath, bool is_backup, bool is_default, int status)
{

    // surprise: This function is called 60,300 time upon loading any scenery. Yep, XP11 has that many items in the libraries.
    // Resultingly the full path was converted to lower case 0.6 million times => 24 million calls to tolower() ... time to optimize

	string suffix;
	suffix = FILE_get_file_extension(path);

	int	rt;

	if     (suffix == "obj") rt = res_Object;
	else if(suffix == "agp") rt = res_Object;
	else if(suffix == "fac") rt = res_Facade;
	else if(suffix == "for") rt = res_Forest;
	else if(suffix == "str") rt = res_String;
	else if(suffix == "lin") rt = res_Line;
	else if(suffix == "pol") rt = res_Polygon;
// not sure we want to even list these ?
// per Ben's explanation of May 2nd 2018 - we don't, until we support all parameters for these.
//	else if(suffix == "ags") rt = res_Polygon;
//	else if(suffix == "agb") rt = res_Polygon;
#if ROAD_EDITING
	else if(suffix == "net") rt = res_Road;
#endif
	else return;

	if (package >= 0 && status >= status_Public && !is_backup) gPackageMgr->AddPublicItems(package);

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
			if(rt > res_Directory)                      // speedup/memory saver: no need to store this for directories
				new_info.real_paths.push_back(rpath);
			new_info.is_backup = is_backup;
			new_info.is_default = is_default;
			res_table.insert(res_map_t::value_type(p,new_info));
		}
		else
		{
			DebugAssert(i->second.res_type == rt);
			if(i->second.is_backup && !is_backup)
			{
				i->second.is_backup = false;
				i->second.real_paths.clear();
				i->second.packages.clear();
			}
			else if(is_backup)
				break;                                   // avoid adding backups as variants

			i->second.packages.insert(package);
			if(is_default && !i->second.is_default)
			{
				i->second.status = status;               // LR libs will always override/downgrade Custom Libs visibility
				i->second.is_default = true;             // But they can still elevate any prior LR lib's visiblity, as some do
			}
			else
				i->second.status = max(i->second.status, status);	// upgrade status if we just found a public version!
			// add only unique paths, but need to preserve first path added as first element, so deliberately not using a set<string> !
			if(rt > res_Directory)                      // speedup/memory saver: no need to store this for directories
			if(std::find(i->second.real_paths.begin(), i->second.real_paths.end(), rpath) == i->second.real_paths.end())
				i->second.real_paths.push_back(rpath);
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
