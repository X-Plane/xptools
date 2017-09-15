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

#include <sstream>
#include <fstream>

#include "FileUtils.h"
#include "GUI_Application.h"
#include "GUI_FormWindow.h"
#include "GUI_Resources.h"
#include "GUI_Timer.h"
#include "PlatformUtils.h"
#include "RAII_Classes.h"
#include "WED_FileCache.h"
#include "WED_MetaDataDefaults.h"
#include "WED_Url.h"
#include "curl/curl.h"
#include "WED_Airport.h"
#include "ILibrarian.h"
#include "WED_ToolUtils.h"

/**
 * @return true if we succeeded; false if there was a catastrophic error about which we alerted the user
 */
static bool fill_cache_request_for_metadata_csv(WED_file_cache_request * out_cache_req)
{
	string cert;
	if(!GUI_GetTempResourcePath("gateway.crt", cert))
	{
		DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
		return false;
	}

	out_cache_req->in_cert = cert;
	out_cache_req->in_domain = cache_domain_metadata_csv;
	out_cache_req->in_folder_prefix = "scenery_packs";
	out_cache_req->in_url = WED_URL_AIRPORT_METADATA_CSV;
	return true;
}

//--WED_UpdateMetadataDialog--------------------------------------------------
static int update_bounds_default[4] = { 0, 0, 500, 500 };

enum update_dialog_stage
{
	update_dialog_download_airport_metadata,
	update_dialog_waiting, //Waiting for the user to choose to continue or not
	update_dialog_done //Error or not
};

class WED_UpdateMetadataDialog : public GUI_FormWindow, public GUI_Timer
{
public:
	WED_UpdateMetadataDialog(WED_Airport * apt, IResolver * resolver);

	//When pressed, opens up the developer blog post about the Gateway 
	//virtual void AuxiliaryAction();

	//When pressed, attempts to submit current airport to the gateway
	virtual void Submit();

	//When pressed, form window is closed
	virtual void Cancel();

	//Called each time WED's timer is fired, checks download progress
	virtual	void TimerFired(void);

	static const string& GetAirportMetadataCSVPath();

private:
	//Used for downloading the airport metadata defaults
	RAII_CurlHandle*        mAirportMetadataCURLHandle;

	//Where the airport metadata csv file was ultimately downloaded to
	static string           mAirportMetadataCSVPath;

	//The airport we are attempting to update
	WED_Airport*            mApt;

	//Cache request for requesting airport metadata csv
	WED_file_cache_request  mCacheRequest;

	//The chosen airport's chosen ICAO
	string                  mChosenICAO;

	//The phase of the state in the dialog box
	update_dialog_stage     mPhase;
	IResolver*              mResolver;

	void StartCSVDownload();
};

//Static member definitions
string WED_UpdateMetadataDialog::mAirportMetadataCSVPath = "";

const string& WED_UpdateMetadataDialog::GetAirportMetadataCSVPath()
{
	return mAirportMetadataCSVPath;
}

int WED_CanUpdateMetadata(IResolver * resolver)
{
	if(WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass) == NULL)
	{
		return 0;
	}

	return 1;
}

void WED_DoUpdateMetadata(IResolver * resolver)
{
	WED_Airport * apt = SAFE_CAST(WED_Airport,WED_HasSingleSelectionOfType(resolver, WED_Airport::sClass));

	if(!apt)
		return;

	new WED_UpdateMetadataDialog(apt, resolver);
}

static string InterpretNetworkError(curl_http_get_file* curl)
{
	int err = curl->get_error();
	bool bad_net = curl->is_net_fail();

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
		curl->get_error_data(errdat);
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
	{
		ss << "Upload failed due to unknown error: " << err << ".";
	}

	return ss.str();
}

WED_UpdateMetadataDialog::WED_UpdateMetadataDialog(WED_Airport * apt, IResolver * resolver) : 
	GUI_FormWindow(gApplication, "Update Airport Metadata", 450, 150),
	mAirportMetadataCURLHandle(NULL),
	mApt(apt),
	mPhase(update_dialog_download_airport_metadata),
	mResolver(resolver)
{	
	this->Reset("", "", "Cancel", true);
	
	apt->GetICAO(mChosenICAO);

	this->AddLabel("Downloading airport metadata defaults...");
	StartCSVDownload();
}

void WED_UpdateMetadataDialog::StartCSVDownload()
{
	const bool cache_filled_successfully = fill_cache_request_for_metadata_csv(&mCacheRequest);
	if(!cache_filled_successfully)
		AsyncDestroy();
	Start(0.1);
}

void WED_UpdateMetadataDialog::Cancel()
{
	this->AsyncDestroy();
}

void WED_UpdateMetadataDialog::Submit()
{
	if(mPhase == update_dialog_done)
	{
		this->AsyncDestroy();
	}
	else if(mPhase == update_dialog_waiting)
	{
		DebugAssert(mApt);
		string icao;
		mApt->GetICAO(icao);
		
		mApt->StartOperation((string("Update " + icao + "'s Metadata").c_str()));
		mApt->StateChanged();
		bool success = fill_in_airport_metadata_defaults(*mApt, mAirportMetadataCSVPath);
		if(success == false)
		{
			mApt->AbortCommand();
			this->Reset("","","Exit",true);
			this->AddLabel("Could not find metadata, check Airport ID or if airport is supported.");
		}
		else
		{
			mApt->CommitCommand();

			this->Reset("","OK","",true);
			this->AddLabel("Metadata applied successfully!");
		}
		mPhase = update_dialog_done;
	}
}

void WED_UpdateMetadataDialog::TimerFired()
{
	if(mPhase == update_dialog_download_airport_metadata)
	{
		WED_file_cache_response res = WED_file_cache_request_file(mCacheRequest);
		if(res.out_status != cache_status_downloading)
		{
			Stop();
			
			if(res.out_status == cache_status_available)
			{
				WED_UpdateMetadataDialog::mAirportMetadataCSVPath = res.out_path;
				mPhase = update_dialog_waiting;
				this->Reset("","OK","Cancel", true);
				
				string icao;
				mApt->GetICAO(icao);
				this->AddLabel("Update metadata for the airport " + icao + "?");
				this->AddLabel("(Adds new metadata if available, will not overwrite existing values)");
			}
			else if(res.out_status == cache_status_error)
			{
				mPhase = update_dialog_done;
				this->Reset("","","Exit",true);
				this->AddLabel(InterpretNetworkError(&this->mAirportMetadataCURLHandle->get_curl_handle()));
			}
		}
		return;
	}
}
//---------------------------------------------------------------------------//
static CSVParser::CSVTable s_csv = CSVParser::CSVTable(CSVParser::CSVTable::CSVHeader(), vector<CSVParser::CSVTable::CSVRow>());
void	WED_DoInvisibleUpdateMetadata(WED_Airport * apt)
{
	if(s_csv.GetRows().empty())
	{
		WED_file_cache_request  cache_request;
		const bool cache_filled_successfully = fill_cache_request_for_metadata_csv(&cache_request);
		if(cache_filled_successfully)
		{
			WED_file_cache_response res = WED_file_cache_request_file(cache_request);
			while(res.out_status == cache_status_downloading)
			{
				// Synchronously download the file (or grab it from disk)
				res = WED_file_cache_request_file(cache_request);
			}

			if(res.out_status == cache_status_available)
			{
				std::ifstream t(res.out_path.c_str());
				if(!t.bad())
				{
					s_csv = CSVParser(',', string((istreambuf_iterator<char>(t)), istreambuf_iterator<char>())).ParseCSV();
					bool success = fill_in_airport_metadata_defaults(*apt, s_csv);
				}
				t.close();
			}
			else if(res.out_status == cache_status_error)
			{
				printf("Failed to auto-update airport metadata due to cache error\n");
			}
		}
	}
	else
	{
		bool success = fill_in_airport_metadata_defaults(*apt, s_csv);
	}
}

