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

enum {
	res_None,
	res_Directory,
	res_Object,
	res_Facade,
	res_Forest,
	res_String,
	res_Line,
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

enum {
	status_Private		= 0,		// Intentionally SORTED so that the most EXPOSED status is the HIGHEST number!
	status_Deprecated	= 1,		// fully deprecated - invisible in hierarchy and validation failure for gateway.
	// Order matters - everything LESS than yellow is going to fail validation
	status_Yellow		= 2,		// half-deprecated items, visibility of deprecated, validates as public
	// Order matters - everything GEQUAL to public is going to be public
	status_Public		= 3,
	status_New			= 4
};

class WED_LibraryMgr : public GUI_Broadcaster, public GUI_Listener, public virtual IBase {
public:

				 WED_LibraryMgr(const string& local_package);
				~WED_LibraryMgr();

				
	//Returns "My Package" of .../Custom Scenery/My Package
	//Combine with WED_PackageMgr::ComputePath to save a file in the package dir
	string		GetLocalPackage() const;

	string		GetResourceParent(const string& r);
	void		GetResourceChildren(const string& r, int filter_package, vector<string>& children);	// Pass empty resource to get roots
	int			GetResourceType(const string& r);
	string		GetResourcePath(const string& r, int variant = 0);
	
				// This returns true if the resource whose virtual path is "r" comes from the default library that x-plane ships with.
	bool		IsResourceDefault(const string& r);
	bool		IsResourceLocal(const string& r);
	bool		IsResourceLibrary(const string& r);
	bool		IsResourceDeprecatedOrPrivate(const string& r);
		
	string		CreateLocalResourcePath(const string& r);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

				// This returns true if the package number 'package_number' adds at least one item to the library.
	bool		DoesPackHaveLibraryItems(int package_number);
				// indicates if art asset exported by multiple exports, i.e. has randomized appearance
	int			GetNumVariants(const string& r);
				// get vpath for a apt.dat taxiline line type #.
	bool		GetLineVpath(int lt, string& vpath);

private:

	void			Rescan();
	void			RescanLines();
	void			AccumResource(const string& path, int package, const string& real_path, bool is_backup, bool is_default, int status);
	static	bool	AccumLocalFile(const char * fileName, bool isDir, void * ref);

	struct	res_info_t {
		int			res_type;
		set<int>	packages;       // points out if same items is exported by multiple libraries
		vector<string> real_paths;  // holds all the variants causeed by multiple EXPORTS commands
		bool		is_backup;
		bool		is_default;
		int			status;
	};

	// Not only case-insensitive, but also sort of leading zero (number) insensitive for the basename part:
	// all leading digits are treated as a number - so 3a.lin is listed after 20a.lin, aka numeric order.
	// If two start with the same number (e.g. 01, 001 and 1) - its normal lexicographic order again.
	// e.g.  0  00  01aa  1aa  009x 10aa 10bb  aa  bb

	struct compare_str_no_case {
		bool operator()(const string& lhs, const string& rhs) const {
			string::size_type pl = lhs.find_last_of('/') ;
			string::size_type pr = rhs.find_last_of('/') ;

			if(pl == pr)
			{
				if(pl == string::npos)
					pl = 0;
				else
				{
					int path_cmp = strncasecmp(lhs.c_str(),rhs.c_str(),pl);
					if(path_cmp != 0) return path_cmp < 0;
					pl += 1;
				}
				const char * l = lhs.c_str() + pl;
				const char * r = rhs.c_str() + pl;

				if((*l >= '0' && *l <= '9') || (*r >= '0' && *r <= '9'))
				{
					int int_l, int_r;
					if(sscanf(l,"%d",&int_l) > 0)
						if(sscanf(r,"%d",&int_r) > 0)
							if(int_l != int_r)
								return int_l < int_r;
				}
				return strcasecmp(l,r) < 0;
			}
			return strcasecmp(lhs.c_str(),rhs.c_str()) < 0;
		}
	};

	typedef map<string,res_info_t,compare_str_no_case>	res_map_t;
	res_map_t			res_table;

	string				local_package;
	map<int, string>	default_lines;  // list of art assets for sim default lines
};

#endif /* WED_LibraryMgr_H */
