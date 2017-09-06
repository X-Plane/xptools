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
	status_Deprecated	= 1,
	status_Yellow		= 2,		// half-deprecated items, visibility of deprecated, validates as public
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
	string		GetResourcePath(const string& r);
	
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

private:

	void			Rescan();
	void			AccumResource(const string& path, int package, const string& real_path, bool is_backup, bool is_default, int status);
	static	bool	AccumLocalFile(const char * fileName, bool isDir, void * ref);

	struct	res_info_t {
		int			res_type;
		set<int>	packages;
		string		real_path;
		bool		is_backup;
		bool		is_default;
		int			status;
	};

	struct compare_str_no_case {
		bool operator()(const string& lhs, const string& rhs) const {
			return strcasecmp(lhs.c_str(),rhs.c_str()) < 0;
		}
	};

	typedef map<string,res_info_t,compare_str_no_case>	res_map_t;
	res_map_t						res_table;

	string							local_package;

};

#endif /* WED_LibraryMgr_H */
