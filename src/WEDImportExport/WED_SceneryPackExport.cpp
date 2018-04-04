/* 
 * Copyright (c) 2014, Laminar Research.
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

#include "WED_SceneryPackExport.h"
#include "WED_AptIE.h"
#include "WED_DSFExport.h"
#include "IResolver.h"
#include "WED_ToolUtils.h"
#include "ILibrarian.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "WED_Group.h"
#include "WED_Validate.h"
#include "WED_Document.h"


void	WED_ExportPackToPath(WED_Thing * root, IResolver * resolver, const string& in_path, set<WED_Thing *>& problem_children)
{
	int result = DSF_Export(root, resolver, in_path,problem_children);
	if (result == -1)
	{
		return;
	}

	string	apt = in_path + "Earth nav data" DIR_STR "apt.dat";
	string	apt_dir = in_path + "Earth nav data";

	FILE_make_dir_exist(apt_dir.c_str());
	WED_AptExport(root, apt.c_str());
}


int		WED_CanExportPack(IResolver * resolver)
{
	return 1;
}

void	WED_DoExportPack(WED_Document * resolver, WED_MapPane * pane)
{
	// Just don't ever export if we are invalid.  Avoid the case where we write junk to a file!
	if(!WED_ValidateApt(resolver, pane))
		return;

	ILibrarian * l = WED_GetLibrarian(resolver);
	WED_Thing * w = WED_GetWorld(resolver);
	WED_Group * g = dynamic_cast<WED_Group*>(w);
	DebugAssert(g);
	set<WED_Thing *>	problem_children;

	string pack_base;
	l->LookupPath(pack_base);

	WED_ExportPackToPath(g, resolver, pack_base, problem_children);


	if(!problem_children.empty())
	{
		DoUserAlert("One or more objects could not exported - check for self intersecting polygons and closed-ring facades crossing DFS boundaries.");
		ISelection * sel = WED_GetSelect(resolver);
		(*problem_children.begin())->StartOperation("Select broken items.");
		sel->Clear();
		for(set<WED_Thing*>::iterator p = problem_children.begin(); p != problem_children.end(); ++p)
			sel->Insert(*p);
		(*problem_children.begin())->CommitOperation();		
	}
}

