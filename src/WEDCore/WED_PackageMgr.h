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

#ifndef WED_PackageMgr_H
#define WED_PackageMgr_H

#include "GUI_Broadcaster.h"

class WED_PackageMgr : public GUI_Broadcaster {
public:

 	 WED_PackageMgr(const char *		in_xplane_folder);
	~WED_PackageMgr();

	bool		HasSystemFolder(void) const;
	bool		GetXPlaneFolder(string& root) const;

	void		SetXPlaneFolder(const string& root);

	int			CountCustomPackages(void) const;
	void		GetNthCustomPackageName(int n, string& package) const;
	void		GetNthCustomPackagePath(int n, string& package) const;

	int			CountPackages(void) const;
	void		GetNthPackageName(int n, string& package) const;
	/*Get the a package's path by passing in a number and the name of said package,
	changes the string passed in into the real physical filepath.*/
	void		GetNthPackagePath(int n, string& package) const;
	
	bool		IsPackageDefault(int n) const;
	bool		IsPackagePublicItems(int n) const;        // library has at least one public item declared in it
	void		HasPublicItems(int n);

	void		RenameCustomPackage(int n, const string& new_name);

	int			CreateNewCustomPackage(void);
	void		Rescan(void);

	string		ComputePath(const string& package, const string& rel_file) const;
	string		ReducePath(const string& package, const string& full_file) const;

private:

	string			system_path;
	bool			system_exists;
	vector<string>	custom_package_names;
	vector<string>	global_package_names;
	vector<string>	default_package_names;
	
	vector<bool>	custom_package_hasPublicItems;
	vector<bool>	global_package_hasPublicItems;
	vector<bool>	default_package_hasPublicItems;

};

extern WED_PackageMgr *		gPackageMgr;

#endif /* WED_PackageMgr_H */
