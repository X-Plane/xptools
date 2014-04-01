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

#include "WED_GatewayExport.h"
#include "WED_DSFExport.h"
#include "WED_Globals.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "WED_ToolUtils.h"
#include "WED_UIDefs.h"
#include "WED_Validate.h"
#include "WED_Thing.h"
#include "WED_ToolUtils.h"
#include "WED_Airport.h"
#include "ILibrarian.h"
#include "WED_SceneryPackExport.h"
#include "WED_AptIE.h"

int	WED_CanExportRobin(IResolver * resolver)
{
	return WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass) != NULL;
}

void	WED_DoExportRobin(IResolver * resolver)
{
	WED_Airport * apt = SAFE_CAST(WED_Airport,WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass));

	if(!apt) return;
	
	string icao;
	apt->GetICAO(icao);
	
	WED_Export_Target old_target = gExportTarget;
	gExportTarget = wet_robin;

	if(!WED_ValidateApt(resolver, apt))
	{
		gExportTarget = old_target;
		return;
	}

	ILibrarian * lib = WED_GetLibrarian(resolver);

	string temp_folder("tempXXXXXX");
	lib->LookupPath(temp_folder);
	vector<char> temp_chars(temp_folder.begin(),temp_folder.end());

	if(!mktemp(&temp_chars[0]))
	{
		gExportTarget = old_target;
		return;
	}
	string targ_folder(temp_chars.begin(),temp_chars.end());
	targ_folder += DIR_STR;

	string targ_folder_zip = icao + "_gateway_upload.zip";
	lib->LookupPath(targ_folder_zip);
	if(FILE_exists(targ_folder_zip.c_str()))
	{
		FILE_delete_file(targ_folder_zip.c_str(), 0);
	}

	DebugAssert(!FILE_exists(targ_folder.c_str()));

	FILE_make_dir_exist(targ_folder.c_str());

	set<WED_Thing *> problem_children;

	if(DSF_ExportAirportOverlay(resolver, apt, targ_folder, problem_children))
	{
		// success.	
	}
	
	string apt_path = targ_folder + icao + ".dat";
	WED_AptExport(apt, apt_path.c_str());

	string preview_folder = targ_folder + icao + "_Scenery_Pack/";
	string preview_zip = targ_folder + icao + "_Scenery_Pack.zip";

	gExportTarget = wet_xplane_1021;


	WED_ExportPackToPath(apt, resolver, preview_folder, problem_children);
	
	FILE_compress_dir(preview_folder, preview_zip, icao + "_Scenery_Pack/");
	
	FILE_delete_dir_recursive(preview_folder);

	FILE_compress_dir(targ_folder, targ_folder_zip, string());

	int r = FILE_delete_dir_recursive(targ_folder);

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

	gExportTarget = old_target;

}

