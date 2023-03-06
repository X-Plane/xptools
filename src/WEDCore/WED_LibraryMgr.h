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

#ifndef WED_LibraryMgr_H
#define WED_LibraryMgr_H

#include "GUI_Broadcaster.h"
#include "GUI_Listener.h"
#include "IBase.h"

enum res_type {
	res_None,
	res_Directory,
	res_Object,
	res_Facade,
	res_Forest,
	res_String,
	res_Line,
	res_Autogen,
	res_Polygon
#if ROAD_EDITING
	,res_Road
#endif
};

// "Virtual" package numbers...package mgr IDs packages as 0-based index.  These meta-constants are used for filtering in special ways.
// They can be passed to GetResourceChildren.  Non-negative inputs mean take the Nth pack.
enum {
	pack_Local		= -1,			// Return only files in the users's currently open pack.
	pack_Library	= -2,			// Return only library items.
	pack_All		= -3,			// Return local files and the entire library.
	pack_Default	= -4,			// Return only library items that come from the default scenery packs that x-plane ships with.
	pack_New		= -5
	};

enum res_status {
	status_Private		= 0,		// Intentionally SORTED so that the most EXPOSED status is the HIGHEST number!
	status_Deprecated	= 1,		// fully deprecated - invisible in hierarchy and validation failure for gateway.
	// Order matters - everything < semi-deprecated is going to fail validation
	status_SemiDeprecated		= 2,		// half-deprecated items, visibility of deprecated, validates as public
	// Order matters - everything >= public is going to be public
	status_Public		= 3,
	status_New			= 4
};

void WED_clean_vpath(string& s);      // always uses "/"
void WED_clean_rpath(string& s);      // uses OS specific DIR_CHAR

class WED_LibraryMgr : public GUI_Broadcaster, public GUI_Listener, public virtual IBase {
public:

				 WED_LibraryMgr(const string& local_package);
				~WED_LibraryMgr();


	//Returns "My Package" of .../Custom Scenery/My Package
	//Combine with WED_PackageMgr::ComputePath to save a file in the package dir
	string		GetLocalPackage() const;

	string		GetResourceParent(const string& r);
	void		GetResourceChildren(const string& r, int filter_package, vector<string>& children, bool no_dirs = false);	// Pass empty resource to get roots
	res_type	GetResourceType(const string& r) const;
	string		GetResourcePath(const string& r, int variant = 0);

				// This returns true if the resource whose virtual path is "r" comes from the default library that x-plane ships with.
	bool		IsResourceDefault(const string& r) const;
	bool		IsResourceLocal(const string& r) const;
	bool		IsResourceLibrary(const string& r) const;
	bool		IsResourceDeprecatedOrPrivate(const string& r) const;
	bool		IsSeasonal(const string& r) const;
	bool		IsRegional(const string& r) const;

	string		CreateLocalResourcePath(const string& r);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

				// This returns true if the package number 'package_number' adds at least one item to the library.
	bool		DoesPackHaveLibraryItems(int package_number) const;
				// indicates if art asset exported by multiple exports, i.e. has randomized appearance
	int			GetNumVariants(const string& r) const;
				// gets the vpath of the resource used to dar apt.dat liner markings
	bool		GetLineVpath(int lt, string& vpath);
				// look up apt.dat surface types and matching public .pol vpaths
				// returns if there is a public polygon equivalent or not.
	            // even if there isnt - may still return a vpath if there is at least a public surface
	bool		GetSurfVpath(int surf, string& vpath);
	int			GetSurfEnum(const string& vpath);

private:

	void			Rescan();
	void			RescanLines();
	void			RescanSurfaces();
	void			AccumResource(const string& path, int package, const string& real_path, bool is_default,
						res_status status, bool is_backup = false, bool is_seasonal = false, bool is_regional = false);
	static	bool	AccumLocalFile(const char * fileName, bool isDir, void * ref);

	struct	res_info_t {
		set<int>	packages;       // points out if same items is exported by multiple libraries
		vector<string> real_paths;  // holds all the variants causeed by multiple EXPORTS commands
		unsigned    res_type : 4, status : 4, is_backup : 1, is_default : 1, has_seasons : 1, has_regions : 1;
	};

	typedef map<string,res_info_t>	res_map_t;
//	typedef unordered_map<string,res_info_t>	res_map_t;   // 1% faster for KATL full map display, but breaks library list
	res_map_t			res_table;

	string				local_package;
	map<int, string>	default_lines;        // list of art assets for sim default lines
	map<int, pair<string, bool> >	default_surfaces;

	friend class WED_JWFacades;
};

#endif /* WED_LibraryMgr_H */
