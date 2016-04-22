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

#include "XDefs.h"
#include "WED_GatewayExport.h"

#if HAS_GATEWAY

// MSVC insanity: XWin must come in before ANY part of the IOperation interface or Ben's stupid #define to get file/line numbers on ops hoses the MS headers!
#include "GUI_FormWindow.h"
#include "WED_MetaDataDefaults.h"

#include "PlatformUtils.h"
#include "FileUtils.h"
#include "curl_http.h"

#include "WED_DSFExport.h"
#include "WED_Globals.h"
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
#include "GUI_Help.h"
#include "WED_Messages.h"
#include "WED_ATCFlow.h"
#include "WED_TaxiRoute.h"
#include "WED_ObjPlacement.h"
#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_ExclusionZone.h"
#include "WED_StringPlacement.h"
#include "GUI_Timer.h"
#include "GUI_Resources.h"
#include "WED_Version.h"
#include "WED_Url.h"
#include <errno.h>
#include <sstream>

#include <curl/curl.h>
#include <json/json.h>
#if IBM
#include <io.h>
#endif

// set this to one to leave the master zip blobs on disk for later examination
#define KEEP_UPLOAD_MASTER_ZIP 0

// write out lots of airports as .json files on disk - allows multi-select, no user interaction, for bulk import.
#define BULK_SPLAT_IO 0

// set this to 1 to write JSONs for multiple exports - for later examination
#define SPLAT_CURL_IO 0

#define ATC_FLOW_TAG       1
#define ATC_TAXI_ROUTE_TAG 2

// Curse C++98 for not having this.
template<typename T>
static std::string to_string(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

/*

		progress bar
		multi-line text?
		


	todo:	field types for password, no edit, multi-line comments
			make the form nicer
			error checking on upload
			progress and cancel on upload
			
			download from gateway?
 */

static int gateway_bounds_default[4] = { 0, 0, 500, 500 };

static string saved_uname;
static string saved_passwd;
static string saved_comment;

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
const char * k_atc_flow_class[] = { WED_ATCFlow::sClass, 0 };
const char * k_atc_taxi_route_class[] = { WED_TaxiRoute::sClass, 0 };

// In apt.dat, taxi route lines are 1200 through 1204
static bool has_atc_taxi_route(WED_Airport * who) { return has_any_of_class(who, k_atc_taxi_route_class); }
// In apt.dat, flow lines are 1100 and 1101
static bool has_atc_flow(WED_Airport * who)       { return has_any_of_class(who, k_atc_flow_class); }

// We are intentionally IGNORING lin/pol/str and exclusion zones...this is 3-d in the 'user' sense
// of the term, like, do things stick up.
const char * k_3d_classes[] = {
	WED_ObjPlacement::sClass,
	WED_FacadePlacement::sClass,
	WED_ForestPlacement::sClass,
	0
};

const char * k_dsf_classes[] = {
	WED_ObjPlacement::sClass,
	WED_FacadePlacement::sClass,
	WED_ForestPlacement::sClass,
	WED_ExclusionZone::sClass,
	WED_DrapedOrthophoto::sClass,
	WED_LinePlacement::sClass,
	WED_StringPlacement::sClass,
	WED_PolygonPlacement::sClass,
	0
};

static bool has_3d(WED_Airport * who) { return has_any_of_class(who, k_3d_classes); }

static bool has_dsf(WED_Airport * who) { return has_any_of_class(who, k_dsf_classes); }

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


int	WED_CanExportToGateway(IResolver * resolver)
{
	#if BULK_SPLAT_IO
	ISelection * sel = WED_GetSelect(resolver);	
	return sel->IterateSelectionAnd(Iterate_IsClass, (void *) WED_Airport::sClass);

	#else
	return WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass) != NULL;
	#endif	
}

class	WED_GatewayExportDialog : public GUI_FormWindow, public GUI_Timer {
public:

	WED_GatewayExportDialog(WED_Airport * apt, IResolver * resolver);
	virtual void Submit();
	virtual void Cancel();
	virtual	void TimerFired(void);
	
private:

	int						mPhase;
	IResolver *				mResolver;	
	curl_http_get_file *	mCurl;
	vector<char>			mResponse;
	WED_Airport *			mApt;
	set<WED_Thing *>		mProblemChildren;
	string					mParID;

};


int	Iterate_JSON_One_Airport(ISelectable * what, void * ref)
{
	IResolver * resolver = (IResolver *) ref;	
	WED_Airport * apt = SAFE_CAST(WED_Airport,what);
	if(!apt)
		return 0;			
	WED_GatewayExportDialog * dlg = new WED_GatewayExportDialog(apt, resolver);
	dlg->Submit();
	#if !BULK_SPLAT_IO
	dlg->Hide();
	#endif
	return 1;
}

void	WED_DoExportToGateway(IResolver * resolver)
{
	#if BULK_SPLAT_IO

		ISelection * sel = WED_GetSelect(resolver);	
		if(!sel->IterateSelectionAnd(Iterate_IsClass, (void *) WED_Airport::sClass))
		{
			DoUserAlert("Please select only airports to do a bulk JSON export.");
			return;
		}
		
		if(WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass))
		{
			WED_Airport * apt = SAFE_CAST(WED_Airport,WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass));

			if(!apt)
				return;
				
			new WED_GatewayExportDialog(apt, resolver);
		}
		else
		{
			sel->IterateSelectionAnd(Iterate_JSON_One_Airport, resolver);
		}
		
	#else
		
		WED_Airport * apt = SAFE_CAST(WED_Airport,WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass));
		//fill_in_meta_data_defaults(*apt);

		if(!apt)
			return;
			
		new WED_GatewayExportDialog(apt, resolver);

	#endif
}

WED_GatewayExportDialog::WED_GatewayExportDialog(WED_Airport * apt, IResolver * resolver) : 
	GUI_FormWindow(gApplication, "Airport Scenery Gateway", 500, 400),
	mResolver(resolver),
	mPhase(0),
	mCurl(NULL),
	mApt(apt)	
{	
	this->Reset("Upload", "Cancel");
				
	string icao, name;
	apt->GetICAO(icao);
	apt->GetName(name);	

	char par_id[32];
	sprintf(par_id,"%d", apt->GetSceneryID());
	
	
	
	this->AddFieldNoEdit(gw_icao,icao,name);
	this->AddField(gw_username,"User Name",saved_uname);
	this->AddField(gw_password,"Password",saved_passwd,ft_password);
	this->AddField(gw_comments,"Comments",saved_comment,ft_big);
	if(apt->GetSceneryID() >= 0)
		mParID = par_id;
	else
		mParID = "";
}

void WED_GatewayExportDialog::Cancel()
{
	if(mPhase == 2)
		GUI_LaunchURL(WED_URL_UPLOAD_OK);
		
	this->AsyncDestroy();
}


void WED_GatewayExportDialog::Submit()
{
	if(mPhase == 0)
	{
		DebugAssert(mApt);
		WED_Airport * apt = mApt;
		
		string apt_name = this->GetField(gw_icao);
		string act_name;
		apt->GetName(act_name);
		DebugAssert(act_name == apt_name);
		
		string comment = this->GetField(gw_comments);
		
//		string::size_type p = 0;
//		while((p=comment.find("\r", p)) != comment.npos)
//		{
//			comment.insert(p+1,"\n");
//			p += 2;
//		}
					
		saved_comment = comment;
		string uname = this->GetField(gw_username);
		saved_uname = uname;
		string pwd = this->GetField(gw_password);	
		saved_passwd = pwd;
		string parid = mParID;
		
		string icao;
		apt->GetICAO(icao);
		
		WED_Export_Target old_target = gExportTarget;
		gExportTarget = wet_gateway;

		if(!WED_ValidateApt(mResolver, apt))
		{
			gExportTarget = old_target;
			return;
		}

		ILibrarian * lib = WED_GetLibrarian(mResolver);

		string temp_folder("tempXXXXXX");
		lib->LookupPath(temp_folder);
		vector<char> temp_chars(temp_folder.begin(),temp_folder.end());
		temp_chars.push_back(0);

		if(!mktemp(&temp_chars[0]))
		{
			gExportTarget = old_target;
			return;
		}
		temp_chars.pop_back();
		string targ_folder(temp_chars.begin(),temp_chars.end());
		targ_folder += DIR_STR;

		printf("Dest: %s\n", targ_folder.c_str());

		string targ_folder_zip = icao + "_gateway_upload.zip";
		lib->LookupPath(targ_folder_zip);
		if(FILE_exists(targ_folder_zip.c_str()))
		{
			FILE_delete_file(targ_folder_zip.c_str(), 0);
		}

		DebugAssert(!FILE_exists(targ_folder.c_str()));

		FILE_make_dir_exist(targ_folder.c_str());

		if(has_dsf(apt))
		if(DSF_ExportAirportOverlay(mResolver, apt, targ_folder, mProblemChildren))
		{
			// success.	
		}
		
		string apt_path = targ_folder + icao + ".dat";
		WED_AptExport(apt, apt_path.c_str());

		string preview_folder = targ_folder + icao + "_Scenery_Pack" + DIR_STR;
		string preview_zip = targ_folder + icao + "_Scenery_Pack.zip";

		gExportTarget = wet_xplane_1021;


		WED_ExportPackToPath(apt, mResolver, preview_folder, mProblemChildren);
		
		FILE * readme = fopen((preview_folder+"README.txt").c_str(),"w");
		if(readme)
		{
			fprintf(readme,"-------------------------------------------------------------------------------\n");
			fprintf(readme, "%s (%s)\n", apt_name.c_str(), icao.c_str());
			fprintf(readme,"-------------------------------------------------------------------------------\n\n");
			fprintf(readme,"This scenery pack was downloaded from the X-Plane Scenery Gateway: \n");
			fprintf(readme,"\n");
			fprintf(readme,"    http://gateway.x-plane.com/\n");
			fprintf(readme,"\n");
			fprintf(readme,"Airport: %s (%s)\n\nUploaded by: %s.\n", apt_name.c_str(), icao.c_str(), uname.c_str());
			fprintf(readme,"\n");
			fprintf(readme,"Authors Comments:\n\n%s\n\n", comment.c_str());
			fprintf(readme,"Installation Instructions:\n");
			fprintf(readme,"\n");
			fprintf(readme,"To install this scenery, drag this entire folder into X-Plane's Custom Scenery\n");
			fprintf(readme,"folder and re-start X-Plane.\n");
			fprintf(readme,"\n");
			fprintf(readme,"The scenery packs shared via the X-Plane Scenery Gateway are free software; you\n");
			fprintf(readme,"can redistribute it and/or modify it under the terms of the GNU General Public\n");
			fprintf(readme,"License as published by the Free Software Foundation; either version 2 of the\n");
			fprintf(readme,"License, or (at your option) any later version.  See the included COPYING file\n");
			fprintf(readme,"for complete terms.\n");
			fclose(readme);
		}
		
		FILE * gpl = fopen((preview_folder+"COPYING").c_str(),"w");
		if(gpl)
		{
			GUI_Resource gpl_res = GUI_LoadResource("COPYING");
			if(gpl_res)
			{
				const char * b = GUI_GetResourceBegin(gpl_res);
				const char * e = GUI_GetResourceEnd(gpl_res);

				fwrite(b,1,e-b,gpl);

				GUI_UnloadResource(gpl_res);
				fclose(gpl);
			}
		}
		
		
		
		FILE_compress_dir(preview_folder, preview_zip, icao + "_Scenery_Pack/");
		
		FILE_delete_dir_recursive(preview_folder);

		FILE_compress_dir(targ_folder, targ_folder_zip, string());

		int r = FILE_delete_dir_recursive(targ_folder);
		
		string uu64;
		file_to_uu64(targ_folder_zip, uu64);
		#if !KEEP_UPLOAD_MASTER_ZIP
		FILE_delete_file(targ_folder_zip.c_str(),0);
		#endif
		
		Json::Value		scenery;
		
		string features;
		if(has_atc_flow(apt))
			features += "," + to_string(ATC_FLOW_TAG);
		if(has_atc_taxi_route(apt))
			features += "," + to_string(ATC_TAXI_ROUTE_TAG);
		if(!features.empty())
			features.erase(features.begin());
		
		scenery["userId"] = uname;
		scenery["password"] = pwd;
		if(parid.empty())
			scenery["parentId"] = Json::Value();
		else
			scenery["parentId"] = parid;
		scenery["icao"] = icao;
		scenery["aptName"] = apt_name;
		scenery["type"] = has_3d(apt) ? "3D" : "2D";
		scenery["artistComments"] = comment;
		scenery["features"] = features;
		scenery["masterZipBlob"] = uu64;
		scenery["clientVersion"] = WED_VERSION_NUMERIC;
			
		Json::Value req;
		req["scenery"] = scenery;
		
		string reqstr=req.toStyledString();
		
		printf("%s\n",reqstr.c_str());
		
		#if BULK_SPLAT_IO || SPLAT_CURL_IO
			// This code exists to service the initial upload of the gateway...
			// it dumps out a JSON upload object for each airport.
			string p = icao;
			p + icao;
			p += ".json";
			lib->LookupPath(p);		
			FILE * fi = fopen(p.c_str(),"wb");
			fprintf(fi,"%s", reqstr.c_str());
			fclose(fi);
		#endif
		#if BULK_SPLAT_IO
			delete this;
			return;
		#endif

		string cert;

		if(!GUI_GetTempResourcePath("gateway.crt", cert))
		{
			DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
			this->AsyncDestroy();
			return;
		}
		string url = WED_URL_GATEWAY_API "scenery";

		curl_http_get_file * auth_req = new curl_http_get_file(
			url.c_str(),
			NULL,&reqstr,&mResponse,cert);
	
		mCurl = auth_req;
		
		mPhase = 1;
		
		this->Reset("", "");
		this->AddLabel("Uploading airport to Gateway.");
		this->AddLabel("This could take up to one minute.");

		gExportTarget = old_target;
		
		Start(1.0);
	}
	
	if(mPhase == 2)
	{
		this->AsyncDestroy();
	}
}

void WED_GatewayExportDialog::TimerFired()
{
	if(mCurl->is_done())
	{
		Stop();
		
		string good_msg, bad_msg;
		
		mPhase = 2;
		if(mCurl->is_ok())
		{
			char * start = &mResponse[0];
			char * end = start + mResponse.size();

			Json::Reader reader;	
			Json::Value response;
		
			if(reader.parse(start, end, response))
			{
				if(response.isMember("sceneryId"))
				{		
					int new_id = response["sceneryId"].asInt();
					mApt->StartOperation("Successful submition to gateway.");
					mApt->SetSceneryID(new_id);
					mApt->CommitOperation();
					
					good_msg = "Your airport has been successfully uploaded and will be visible to all users on\nthe gateway once a moderator approves it.";
				}
				else
				{
					bad_msg = "I was not able to confirm that your scenery was uploaded.\n(No scenery ID was returned.)";
				}				
			}
			else
				bad_msg = "I was not able to confirm that your scenery was uploaded.\n(JSON was malformed.)";
		}
		else
		{
			int err = mCurl->get_error();
			bool bad_net = mCurl->is_net_fail();

			stringstream ss;
			
			if(err <= CURL_LAST)
			{
				string msg = curl_easy_strerror((CURLcode) err);
				ss << "Upload failed: " << msg << ". (" << err << ")";
				
				if(bad_net) ss << "\n(Please check your internet connectivity.)";					
			}
			else if(err >= 100)
			{
				ss << "Upload failed.  The server returned error " << err << ".";
				
				vector<char>	errdat;
				mCurl->get_error_data(errdat);
				bool is_ok = !errdat.empty();
				for(vector<char>::iterator i = errdat.begin(); i != errdat.end(); ++i)
				if(!isprint(*i))
				{
					is_ok = false;
					break;
				}
				
				if(is_ok)
				{
					string errmsg = string(errdat.begin(),errdat.end());
					ss << "\n" << errmsg;
				}
			}
			else
				ss << "Upload failed due to unknown error: " << err << ".";

			bad_msg = ss.str();			
			
		}
		
		if(!good_msg.empty())
		{
			this->Reset("OK","Learn More...");
			this->AddLabel(good_msg);
		}
		else
		{
			this->Reset("","Cancel");
			this->AddLabel(bad_msg);			
		}
		
		delete mCurl;
		mCurl = NULL;
		
		
	//	FILE * json = fopen("/Users/bsupnik/Desktop/json.txt","w");
	//	fprintf(json,"%s",reqstr.c_str());	
	//	fclose(json);
		

		if(!mProblemChildren.empty())
		{
			DoUserAlert("One or more objects could not exported - check for self intersecting polygons and closed-ring facades crossing DFS boundaries.");
			ISelection * sel = WED_GetSelect(mResolver);
			(*mProblemChildren.begin())->StartOperation("Select broken items.");
			sel->Clear();
			for(set<WED_Thing*>::iterator p = mProblemChildren.begin(); p != mProblemChildren.end(); ++p)
				sel->Insert(*p);
			(*mProblemChildren.begin())->CommitOperation();		
		}

	}

}

#endif /* HAS_GATEWAY */