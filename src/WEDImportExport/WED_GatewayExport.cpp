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
#include "GUI_Application.h"
#include "GUI_FormWindow.h"
#include "WED_Messages.h"
#include "WED_ATCFlow.h"
#include "WED_TaxiRoute.h"
#include "WED_ObjPlacement.h"
#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
/*
	todo: field types for password, no edit, multi-line
 */

static int gateway_bounds_default[4] = { 0, 0, 500, 500 };

enum {
	gw_icao = 1,
	gw_username,
	gw_password,
	gw_comments
};

static bool is_of_class(WED_Thing * who, const char ** classes)
{
	while(*classes)
	{
		if(strcmp(who->GetClass(), *classes) == 0)
			return true;
		++classes;
	}
	return false;
}

static bool has_any_of_class(WED_Thing * who, const char ** classes)
{
	if(is_of_class(who,classes) )
		return true;
	int n, nn = who->CountChildren();
	for(n = 0; n < nn; ++n)
		if(has_any_of_class(who->GetNthChild(n), classes))
			return true;
	return false;		
}

// ATC classes - the entire flow hierarchy sits below WED_ATCFlow, so we only need that.  
// Nodes are auxilary to WED_TaxiRoute, so the taxi route covers all edges.
const char * k_atc_classes[] = {
	WED_ATCFlow::sClass,
	WED_TaxiRoute::sClass,
	0
};

static bool has_atc(WED_Airport * who) { return has_any_of_class(who, k_atc_classes); }

// We are intentionally IGNORING lin/pol/str and exclusion zones...this is 3-d in the 'user' sense
// of the term, like, do things stick up.
const char * k_3d_classes[] = {
	WED_ObjPlacement::sClass,
	WED_FacadePlacement::sClass,
	WED_ForestPlacement::sClass,
	0
};

static bool has_3d(WED_Airport * who) { return has_any_of_class(who, k_3d_classes); }

//------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------


static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void encodeblock( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

static int file_to_uu64(const string& path, string& enc)
{
	enc.clear();
	FILE * fi = fopen(path.c_str(),"rb");
	if(fi == NULL)
		return errno;
		
	while(!feof(fi))
	{
		unsigned char buf[3];
		int rd = fread(buf,1,3,fi);
		if(rd <= 0)
			break;
		
		char out[5];
		
		encodeblock(buf, (unsigned char *) out, rd);
		out[4] = 0;
		enc += out;
	}
	
	fclose(fi);
	return 0;
}





//------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------


int	WED_CanExportRobin(IResolver * resolver)
{
	return WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass) != NULL;
}

static void DoGatewaySubmit(GUI_FormWindow * form, void * ref);

void	WED_DoExportRobin(IResolver * resolver)
{
	WED_Airport * apt = SAFE_CAST(WED_Airport,WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass));

	if(!apt)
		return;
		
	string icao, name;
	apt->GetICAO(icao);
	apt->GetName(name);	

	GUI_FormWindow * frm = new GUI_FormWindow(gApplication, "Gateway", "Upload","Cancel", DoGatewaySubmit, resolver);
	frm->AddField(gw_icao,icao,name);
	frm->AddField(gw_username,"User Name","ben");
	frm->AddField(gw_password,"Password","wedGuru");
	frm->AddField(gw_comments,"Comments","This is an airport.");
	frm->Show();
}

static void DoGatewaySubmit(GUI_FormWindow * form, void * ref)
{
	IResolver * resolver = (IResolver *) ref;
	
	WED_Airport * apt = SAFE_CAST(WED_Airport,WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass));
	if(!apt) return;
	
	int tag_atc = has_atc(apt);
	int tag_3d = has_3d(apt);
	
	string apt_name = form->GetField(gw_icao);
	string act_name;
	apt->GetName(act_name);
	DebugAssert(act_name == apt_name);
	
	string comment = form->GetField(gw_comments);
	string uname = form->GetField(gw_username);
	string pwd = form->GetField(gw_password);	
	
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
	
	FILE * readme = fopen((preview_folder+"README.txt").c_str(),"w");
	if(readme)
	{
		fprintf(readme,"-------------------------------------------------------------------------------\n");
		fprintf(readme, "%s (%s)\n", apt_name.c_str(), icao.c_str());
		fprintf(readme,"-------------------------------------------------------------------------------\n\n");
		fprintf(readme,"Airport: %s (%s)\n\nUploaded by: %s.\n\n", apt_name.c_str(), icao.c_str(), uname.c_str());
		fprintf(readme,"Authors Comments:\n\n%s\n\n", comment.c_str());
		fprintf(readme,"Installation Instructions:\n\n");
		fprintf(readme,"To install this scenery, drag this entire folder into X-Plane's Custom Scenery\n");
		fprintf(readme,"folder.");
		fclose(readme);
	}
	
	
	
	FILE_compress_dir(preview_folder, preview_zip, icao + "_Scenery_Pack/");
	
	FILE_delete_dir_recursive(preview_folder);

	FILE_compress_dir(targ_folder, targ_folder_zip, string());

	int r = FILE_delete_dir_recursive(targ_folder);
	
	string uu64;
	file_to_uu64(targ_folder_zip, uu64);
	
	FILE * json = fopen("/Users/bsupnik/Desktop/json.txt","w");
	
	fprintf(json,"{\"Scenery\":\n");
	fprintf(json,"{\"userId\":\"%s\"\n", uname.c_str());
	fprintf(json,",\"password\":\"%s\"\n", pwd.c_str());
	fprintf(json,",\"icao\":\"%s\"\n", icao.c_str());
	fprintf(json,",\"type\":\"%s\"\n", tag_3d ? "3D" : "2D");
	fprintf(json,",\"artistComments\":\"%s\"\n", comment.c_str());
	fprintf(json,",\"features\":\"%s\"\n", tag_atc ? "2" : "");
	fprintf(json,",\"masterZipBlob\":\"%s\"\n}\n}\n", uu64.c_str());
	
	fclose(json);
	

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

