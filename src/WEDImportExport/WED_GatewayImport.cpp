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


#include "WED_GatewayImport.h"
#include "WED_GatewayExport.h"
#include "WED_DSFImport.h"

#if HAS_GATEWAY
#include "MemFileUtils.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "curl_http.h"
#include <curl/curl.h>
#include <json/json.h>
#include "RAII_Classes.h"

#include <sstream>
#include <thread>

#include "WED_Document.h"
#include "WED_Globals.h"
#include "WED_GroupCommands.h"
#include "WED_MapPane.h"
#include "WED_PackageMgr.h"
#include "WED_PropertyPane.h"
#include "WED_Url.h"
#include "WED_UIDefs.h"

#include "GUI_Application.h"
#include "GUI_Button.h"
#include "GUI_FilterBar.h"
#include "GUI_Label.h"
#include "GUI_Packer.h"
#include "GUI_Resources.h"
#include "GUI_Timer.h"
#include "GUI_Window.h"

#include "WED_Messages.h"
#include "WED_AptIE.h"
#include "WED_ToolUtils.h"
#include "WED_HierarchyUtils.h"
#include "WED_Airport.h"
#include "WED_ExclusionZone.h"
#include "WED_Group.h"
#include "WED_MetadataUpdate.h"
#include "WED_MetaDataDefaults.h"

//---------------

//--Table Code------------
#include "GUI_Table.h"
#include "GUI_TextTable.h"
#include "WED_Colors.h"

#define ALLOW_MULTI_IMPORT 1                       // set this if you want to allow to import multiple airports at a time
#define MULTI_MAX_FILES TYLER_MODE ? 50000 : 300   // Prevent too many files to be downloaded in one swoop - its REALLY slow

#if ALLOW_MULTI_IMPORT
	#include "WED_AptTable.h"
#else
	#include "WED_ICAOTable.h"
#endif
	//--Version Table---------
	#include "WED_VerTable.h"
	//------------------------//

//------------------------//

#include "WED_FileCache.h"

//Heuristics for how big to initialize the rawJSONbuf, since it is faster to make a big chunk than make it constantly resize itself
//These should probably change as the gateway gets bigger and the downloads get longer in average charecter length

#if DEV

#if !TEST_AT_START
//If you want to have the zip,dsf.txt, and apt.dat stay on the HDD instead of being deleted instantly
//Cannot be concievably used during TEST_AT_START mode
#define SAVE_ON_HDD 0
#endif
#endif

enum imp_dialog_stages
{
	imp_dialog_error,//for file and network errors
	imp_dialog_download_airport_metadata,
	imp_dialog_download_ICAO,
	imp_dialog_choose_ICAO,//Let the user choose the aiport from the table. The required GET is done before hand
	imp_dialog_download_versions,
	imp_dialog_choose_versions,//Let user pick scenery pack(s) to download. The required GET is done before hand
	imp_dialog_download_specific_version//Download scenery pack, save the contents in the right place, import to WED
};

enum imp_dialog_msg
{
	filter_changed,
	click_next,
	click_extra,
	click_back
};


//--Mem File Utils code for virtually handling the downloaded zip file--
//Returns an empty string if everything went well, the error message if not
string MemFileHandling(const string & zipPath, const string & filePath, const string & ICAO, bool & has_dsf)
//Scope to ensure all the MemFile stuff stays inside here
{
	//A representation of the zipfile
	MFFileSet * zipRep = FileSet_Open(zipPath.c_str());
	if(zipRep == NULL)
	{
#if DEV
		return string("Could not create memory mapped version of " + zipPath + ". Check if the file exists or if you have sufficient permissions");
#else
		return string("Could not open " + zipPath + ". Check if the file exists or if you have sufficient permissions");
#endif
	}
	int fileCount = FileSet_Count(zipRep) - 1;
	/* For all the files
	*  Get the current file inside the memory mapped zip
	*  Get the filename,
	*		if it is "ICAO.txt", mark has_dsf to true
	*  build the file path
	*  Write the file to the harddrive
	*/
	while(fileCount >= 0)
	{
		MFMemFile * currentFile = FileSet_OpenNth(zipRep,fileCount);
		const char * fileName = FileSet_GetNth(zipRep,fileCount);

#if !SAVE_ON_HDD//Because, remember we DON'T want this file normally
		if(fileName == (ICAO + "_Scenery_Pack" + ".zip"))
		{
			MemFile_Close(currentFile);
			fileCount--;
			continue;
		}
#endif
		string writeMode;
		if(fileName == (ICAO + ".txt"))
		{
			has_dsf = true;
			writeMode = "w";
		}
		else
		{
			writeMode = "wb";
		}

		RAII_FileHandle f(string(filePath + fileName), writeMode.c_str());

		if(f)
		{
			size_t write_result = fwrite(MemFile_GetBegin(currentFile),sizeof(char),MemFile_GetEnd(currentFile)-MemFile_GetBegin(currentFile),f());
			f.close();

			if(write_result != MemFile_GetEnd(currentFile)-MemFile_GetBegin(currentFile))
			{
				MemFile_Close(currentFile);
				return string("Could not create file at " + string(filePath + fileName) + ", please ensure you have enough hard drive space and permissions");
			}
			MemFile_Close(currentFile);
		}
		else
		{
			MemFile_Close(currentFile);
			return string("Could not create file at " + filePath + fileName + ", please ensure you have sufficient hard drive space and permissions");
		}
		fileCount--;
	}
	FileSet_Close(zipRep);
	return string();
}//end MemFile stuff
//---------------------------------------------------------------------------//

typedef vector<char> JSON_BUF;

//Our private class for the import dialog
class WED_GatewayImportDialog : public GUI_Window, public GUI_Listener, public GUI_Timer, public GUI_Destroyable
{
public:
	WED_GatewayImportDialog(WED_Document * resolver, WED_MapPane * pane, GUI_Commander * cmdr, WED_PropertyPane* prop_pane);
	~WED_GatewayImportDialog();

private:
	//Fired when selecting the next button
	void Next();

	//Fire when selecting the back button
	void Back();

	//For checking on the curl_http download and displaying a progress bar
	virtual	void TimerFired(void);
	virtual void ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam);

#if TEST_AT_START
	//So we can start testing stuff as soon as the program opens
	//Allows for testing of the network and the GUI
	friend class WED_AppMain;
#endif

	int					mPhase;//Our simple stage counter for our simple fsm

	WED_Document *		mResolver;
	WED_MapPane *		mMapPane;
	WED_PropertyPane *	mPropPane;

	//The cache request info struct for requesting files
	WED_file_cache_request	mCacheRequest;

	//The buffers of the specific packs downloaded at the end
	vector<string>	mSpecificBufs;

//--GUI parts

	//Changes the decoration of the GUI window's title, buttons, etc, based on the stage
	void DecorateGUIWindow(string labelDesc="");
	//--For the whole dialog box
	GUI_FilterBar	 *		mFilter;
	GUI_Packer *			mPacker;

	GUI_Pane *				mButtonHolder;
	GUI_Button *			mNextButton;
	GUI_Button *			mBackButton;
	GUI_Button *			mExtraButton;
	GUI_Label *				mLabel;

	static int				import_bounds_default[4];

	//--ICAO Table
	GUI_Packer *			mICAO_Packer;
	GUI_ScrollerPane *		mICAO_Scroller;
	GUI_Table *				mICAO_Table;
	GUI_Header *			mICAO_Header;

	GUI_TextTable			mICAO_TextTable;
	GUI_TextTableHeader		mICAO_TextTableHeader;

	//Starts the download of the airport defaults csv
	void StartCSVDownload();

	//Starts the download of the json file containing the airports list
	void StartICAODownload();

	//From the downloaded JSON, fill the ICAO table
	void FillICAOFromJSON(const string& json_string);
		//--ICAO Table Provider/Geometry
#if ALLOW_MULTI_IMPORT
		WED_AptTable			mICAO_AptProvider;
#else
		WED_ICAOTable			mICAO_AptProvider;
#endif
		AptVector				mICAO_Apts;

		//Sets up the ICAO table GUI and event handlers
		void MakeICAOTable(int bounds[4]);
		//----------------------//
	//----------------------//

	// Select Airports based on external file
	void SelectWithFile();

	//--Versions Table----------
	GUI_Packer *			mVersions_Packer;
	GUI_ScrollerPane *		mVersions_Scroller;
	GUI_Table *				mVersions_Table;
	GUI_Header *			mVersions_Header;
	//index of the current selection
	set<int>				mVersions_VersionsSelected;
	GUI_TextTable			mVersions_TextTable;
	GUI_TextTableHeader		mVersions_TextTableHeader;

	//Starts the download of the json file to populate the versions table
	bool StartVersionsDownload();

	//From the downloaded JSON, fill the versions table
	void FillVersionsFromJSON(const string& json_string);

	//Once a specific version is downloaded this method decodes and imports it into the document
	//Returns a pointer to the last imported airport
	WED_Airport * ImportSpecificVersion(const string& json_string);

	//Keeps the versions downloading until they have all been (atleast attempted to download)
	//returns false if there is nothing left in the queue
	bool NextVersionsDownload();
		//--Versions Table Provider/Geometry
		WED_VerTable			mVersions_VerProvider;
		VerVector				mVersions_Vers;

	//Sets up the Versions table GUI and event handlers
	void MakeVersionsTable(int bounds[4]);

};


int WED_GatewayImportDialog::import_bounds_default[4] = { 0, 0, 800, 500 };


//--Implemation of WED_GateWayImportDialog class-------------------------------
WED_GatewayImportDialog::WED_GatewayImportDialog(WED_Document * resolver, WED_MapPane * pane, GUI_Commander * cmdr, WED_PropertyPane * prop_pane):
	GUI_Window("Import from Gateway",xwin_style_visible|xwin_style_centered|xwin_style_resizable|xwin_style_modal,import_bounds_default,cmdr),
	mResolver(resolver),
	mMapPane(pane),
	mPropPane(prop_pane),
	mPhase(imp_dialog_download_airport_metadata),
	mICAO_AptProvider(&mICAO_Apts, gModeratorMode ? "Date Accepted" : "Locked until", gModeratorMode ? "User Name": "by Artist"),
	mICAO_TextTable(this,100,0),
	mVersions_VerProvider(&mVersions_Vers),
	mVersions_TextTable(this,100,0)
{
#if !TEST_AT_START
	mResolver->AddListener(this);
#endif
	//mPacker
	int bounds[4];
	mPacker = new GUI_Packer;
	mPacker->SetParent(this);
	mPacker->SetSticky(1,1,1,1);
	mPacker->Show();
	GUI_Pane::GetBounds(bounds);
	mPacker->SetBounds(bounds);
	mPacker->SetBkgkndImage ("gradient.png");

	//Filter
	mFilter = new GUI_FilterBar(this,filter_changed,0,"Search:","",false);
	mFilter->Show();
	mFilter->SetSticky(1,0,1,1);
	mFilter->SetParent(mPacker);
	mFilter->AddListener(this);
	mPacker->PackPane(mFilter,gui_Pack_Top);

	//--Button Setup
		int k_reg[4] = { 0, 0, 1, 3 };
		int k_hil[4] = { 0, 1, 1, 3 };

		mNextButton = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
		mNextButton->SetBounds(265,5,425,GUI_GetImageResourceHeight("push_buttons.png") / 2);
		mNextButton->SetSticky(0,1,1,0);
		mNextButton->SetMsg(click_next,0);

		mBackButton = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
		mBackButton->SetBounds(5,5,105,GUI_GetImageResourceHeight("push_buttons.png") / 2);
		mBackButton->SetSticky(1,1,0,0);
		mBackButton->SetMsg(click_back,0);

		mExtraButton = new GUI_Button("push_buttons.png", btn_Push, k_reg, k_hil, k_reg, k_hil);
		mExtraButton->SetBounds(105, 5, 265, GUI_GetImageResourceHeight("push_buttons.png") / 2);
		mExtraButton->SetSticky(.5, 1, .5, 0);
		mExtraButton->SetMsg(click_extra, 0);

		mButtonHolder = new GUI_Pane;
		mButtonHolder->SetBounds(0,0,430,GUI_GetImageResourceHeight("push_buttons.png") / 2 );

		mNextButton->SetParent(mButtonHolder);
		mNextButton->AddListener(this);
		mExtraButton->SetParent(mButtonHolder);
		mExtraButton->AddListener(this);
		mBackButton->SetParent(mButtonHolder);
		mBackButton->AddListener(this);

		mButtonHolder->SetParent(mPacker);
		mButtonHolder->Show();
		mButtonHolder->SetSticky(1,1,1,0);
		mPacker->PackPane(mButtonHolder,gui_Pack_Bottom);
	//--------------------------------//

		GUI_Pane * tableHolder = new GUI_Pane();
		tableHolder->SetBounds(0,0,0,0);//Because we are packing in the center these will be completely overriden
		tableHolder->SetParent(mPacker);
		tableHolder->Show();
		tableHolder->SetSticky(1,1,1,1);
		mPacker->PackPane(tableHolder,gui_Pack_Center);

		mICAO_Packer = new GUI_Packer;
		mICAO_Packer->SetParent(tableHolder);
		mICAO_Packer->SetSticky(1,1,1,1);
		mICAO_Packer->Show();
		tableHolder->GetBounds(bounds);
		mICAO_Packer->SetBounds(bounds);
		mICAO_Packer->SetBkgkndImage ("gradient.png");

		mVersions_Packer = new GUI_Packer;
		mVersions_Packer->SetParent(tableHolder);
		mVersions_Packer->SetSticky(1,1,1,1);
		mVersions_Packer->Hide();
		tableHolder->GetBounds(bounds);
		mVersions_Packer->SetBounds(bounds);
		mVersions_Packer->SetBkgkndImage ("gradient.png");

		MakeICAOTable(bounds);
		MakeVersionsTable(bounds);

		mLabel = new GUI_Label();
		mLabel->SetParent(this);
		mLabel->SetDescriptor("Download in Progress, Please Wait");
		mLabel->SetImplicitMultiline(true);
		int labelBounds[4] = {70,230,600,270};
		mLabel->SetBounds(labelBounds);


		mLabel->SetColors(WED_Color_RGBA(wed_Table_Text));

		mLabel->SetSticky(1,0,1,1);
		DecorateGUIWindow();

	StartCSVDownload();
}

WED_GatewayImportDialog::~WED_GatewayImportDialog()
{

}

void WED_GatewayImportDialog::Next()
{
#if ALLOW_MULTI_IMPORT
	set<int> apts;
#endif
	switch(mPhase)
	{
	case imp_dialog_error:
		this->AsyncDestroy();
		break;
	//case imp_dialog_download_airport_metadata:
		//break; no next button here
	//case imp_dialog_download_ICAO:
		//break; no next button here
	case imp_dialog_choose_ICAO:

#if ALLOW_MULTI_IMPORT
		mICAO_AptProvider.GetSelection(apts);
		if(apts.size() > 1)
		{
			int max_imports = MULTI_MAX_FILES;   // some artifical limit to prevent WED locking up for hours when selecting the whole gateway ...
			mVersions_VersionsSelected.clear();
			mVersions_Vers.clear();
			for(set<int>::iterator apt = apts.begin(); apt != apts.end(); ++apt)
			{
				VerInfo_t v;
				v.icao = mICAO_Apts.at(*apt).icao; v.sceneryId = mICAO_Apts.at(*apt).kind_code;
				if(v.sceneryId != 0) // prevent 404 error due to airport w/no scenery available, yet
				{
					mVersions_Vers.push_back(v);
					mVersions_VersionsSelected.insert(mVersions_Vers.size()-1);
				}
				if(!--max_imports)
				{
					DoUserAlert("Stopped after importing 500 airports, large gateway downloads are unsupported.");
					break;
				}
			}
			DecorateGUIWindow("Loading file(s) from hard drive, please wait...");
			NextVersionsDownload();
			mPhase = imp_dialog_download_specific_version;
		}
		else
#endif
		//Going to show versions
		if(StartVersionsDownload())
		{
			mPhase = imp_dialog_download_versions;
			mICAO_AptProvider.SelectionStart(1);
		}
		else {
			DoUserAlert("Pick an airport to import.");
		}
		break;
	//case imp_dialog_download_versions:
		//break; no next button here
	case imp_dialog_choose_versions:
		mVersions_VerProvider.GetSelection(mVersions_VersionsSelected);

		//Were we able to in the first place?
		bool able_to_start = NextVersionsDownload();
		if(able_to_start == false)
		{
			//This one stays as a user alert because we don't want to leave this window yet.
			DoUserAlert("You must select at least one item in the list");
			return;//
		}
		mPhase = imp_dialog_download_specific_version;
		break;
	//case imp_dialog_download_specific_version:
		//break; no button here
	}
	DecorateGUIWindow();
}

void WED_GatewayImportDialog::Back()
{
	switch(mPhase)
	{
	case imp_dialog_error:
		break;
	case imp_dialog_download_airport_metadata:
	case imp_dialog_download_ICAO:
	case imp_dialog_choose_ICAO:
		this->AsyncDestroy();
		break;
	case imp_dialog_download_versions:
		Stop();
		/* intentional, really! */
	case imp_dialog_choose_versions:
		mFilter->ClearFilter();				// Filter from choose version not appropriate for airports?
		mPhase = imp_dialog_choose_ICAO;
//		mVersions_VerProvider.SelectionStart(1);
		break;
	case imp_dialog_download_specific_version:
		break;
	}
	DecorateGUIWindow();//Decorate once we're in the correct place
}

static void dummyPrintf(void* ref, const char* fmt, ...) { return; }

extern "C" void decode( const char * startP, const char * endP, char * destP, char ** out);
void WED_GatewayImportDialog::TimerFired()
{
	WED_file_cache_response res = gFileCache.request_file(mCacheRequest);

	if(mPhase == imp_dialog_download_airport_metadata ||
	   mPhase == imp_dialog_download_ICAO ||
	   mPhase == imp_dialog_download_versions ||
	   mPhase >= imp_dialog_download_specific_version)
	{
		if(res.out_status != cache_status_available)
		{
			char p[30] = "";
			if(mPhase > imp_dialog_download_specific_version)
				sprintf(p," Plus %2d more files to go.",(int) mVersions_VersionsSelected.size());

			int progress = res.out_download_progress;
			char c[100];
			if(progress < 0)
				sprintf(c,"Download in Progress: %4dkB received. %s",-progress,p);
			else
				sprintf(c,"Download in Progress: %2d%% done. %s",progress,p);
			DecorateGUIWindow(c);
		}
	}

	//If we've reached a conclusion to this cache request
	if(res.out_status != cache_status_downloading)
	{
		Stop();
		mPhase++;
		DecorateGUIWindow();

		if(res.out_status == cache_status_available)
		{
			//Attempt to open the file we just downloaded
			RAII_FileHandle file(res.out_path.c_str(),"r");

			string file_contents;
			if(FILE_read_file_to_string(file(), file_contents) == 0)
			{
				file.close();

				if(mPhase == imp_dialog_download_ICAO)
				{
					StartICAODownload();

					DecorateGUIWindow("Loading file from hard drive, please wait...");
				}
				if(mPhase == imp_dialog_choose_ICAO)//We just finished downloading the ICAO list
				{
					FillICAOFromJSON(file_contents);
				}
				else if(mPhase == imp_dialog_choose_versions)//
				{
					FillVersionsFromJSON(file_contents);
				}
				else if(mPhase - 1 >= imp_dialog_download_specific_version) // -1 to counter act the mPhase++, >= for the fact we have multiple downloads
				{
					//Push back the latest buffer
					mSpecificBufs.push_back(file_contents);

					//Try to start the next download
					bool has_versions_left = NextVersionsDownload();

					//We're all done with everything!
					if(has_versions_left == false)
					{
						WED_Thing * wrl = WED_GetWorld(mResolver);
						wrl->StartOperation("Import Scenery Pack");

						WED_Airport * last_imported = NULL;

						//If it fails anywhere inside it will soon be destroyed
						for (int i = 0; i < mSpecificBufs.size(); i++)
						{
							last_imported = ImportSpecificVersion(mSpecificBufs[i]);
							//We completely abort if _anything_ goes wrong
							if(last_imported == NULL)
							{
								wrl->AbortOperation();
								this->AsyncDestroy();//All done!
								return;
							}
							add_iso3166_country_metadata(*last_imported);
							if (last_imported->GetSceneryID() < 94010)
							{
								if (ConfirmMessage("Existing X-Plane 11 Exclusion Zones and Flatten properties must be removed and re-evaluated for X-Plane 12 gateway submissions", "Delete as recommended", "Keep all") == 1)
								{
									set<WED_Thing*> ex_set;
									CollectRecursive(last_imported, inserter(ex_set, ex_set.begin()), IgnoreVisiblity, TakeAlways,
										WED_ExclusionZone::sClass, 2);
									WED_RecursiveDelete(ex_set);

									AptInfo_t apt_info;
									last_imported->Export(apt_info);
									auto it = std::find(apt_info.meta_data.begin(), apt_info.meta_data.end(), make_pair(string("flatten"), string("1")));
									if (it != apt_info.meta_data.end())
									{
										apt_info.meta_data.erase(it);
										last_imported->Import(apt_info, dummyPrintf, nullptr);
									}
								}
							}
						}

						//Set the current airport in the sense of "WED's current airport"
						WED_SetCurrentAirport(mResolver, last_imported);
						gExportTarget = wet_gateway;
						//Select the current airport in the sense of selecting something on the map pane
						ISelection * sel = WED_GetSelect(mResolver);
						sel->Clear();
						sel->Insert(last_imported);
						mMapPane->ZoomShowSel();
						wrl->CommitOperation();

						this->AsyncDestroy();//All done!
						return;
					}
				}//end if(mPhase == imp_dialog_download_specific_version)
				return;
			}
			file.close();
		}//end if(mCurl.get_curl_handle()->is_ok())
		else if(res.out_status == cache_status_error)
		{
			if(res.out_error_human != "" || res.out_error_type != cache_error_type_none)
			{
				mPhase = imp_dialog_error;
				DecorateGUIWindow(res.out_error_human);
			}
		}
	}//end if res.out_status != cache_status_downloading && ... != cache_status_not_started
}//end WED_GatewayImportDialog::TimerFired()

void WED_GatewayImportDialog::FillICAOFromJSON(const string& json_string)
{
	Json::Value root;
	Json::Reader reader;
	bool success = reader.parse(json_string,root);

	//Check for errors
	if(success == false)
	{
		mPhase = imp_dialog_error;

	#if DEV
		DecorateGUIWindow("Airports list is invalid data");
	#else
		DecorateGUIWindow("The download was corrupted, please try again");
	#endif
		return;
	}

	Json::Value mAirportsGET = Json::Value(Json::arrayValue);
	mAirportsGET.swap(root["airports"]);

	//loop through the whole array
	for (int i = 0; i < mAirportsGET.size(); i++)
	{
		//Get the current scenery object
		Json::Value tmp(Json::objectValue);
		tmp = mAirportsGET.operator[](i);//Yes, you need the verbose operator[] form. Yes it's dumb

		if(tmp["AcceptedSceneryCount"].asInt() >= 0)
		{
			AptInfo_t cur_airport;
			cur_airport.icao = tmp["AirportCode"].asString();
			cur_airport.name = tmp["AirportName"].asString();

			string code;
			Json::Value meta(Json::objectValue);
			meta.swap(tmp["metadata"]);
			if (meta.size())
			{
				code = meta["icao_code"].asString();

				string code2 = meta["faa_code"].asString();
				if(code.size() && code2.size())
				{
					if(code != code2)
						code += "," + code2;
				}
				else
					code += code2;

				code2 = meta["local_code"].asString();
				if(code.size() && code2.size())
				{
					if(code.find(code2) == string::npos)
						code += "," + code2;
				}
				else
					code += code2;
			}
			cur_airport.meta_data.push_back(make_pair("IcaoFaaLocal", code));   // pseudo-tag to support selection by ANY of these 3 tags
			cur_airport.kind_code = 0;                                          // scenery-ID of not deprecated, recommended version, if any

			if(gModeratorMode)
			{
				if (tmp["AcceptedSceneryCount"].asInt() > tmp["ApprovedSceneryCount"].asInt())
				{
					//Makes the url "https://gateway.x-plane.com/apiv1/airport/ICAO"
					mCacheRequest.in_url = WED_get_GW_api_url() + "airport/" + cur_airport.icao;
					mCacheRequest.in_domain = cache_domain_airport_versions_json;
					mCacheRequest.in_folder_prefix = "scenery_packs" DIR_STR "GatewayImport" DIR_STR + cur_airport.icao;

					WED_file_cache_response res = gFileCache.request_file(mCacheRequest);

					for (int i = 0; i < 30; ++i) // try downloading version info for 3sec. Should normally be enough.
					{
						if(res.out_status == cache_status_downloading)
						{
							this_thread::sleep_for(chrono::milliseconds(100));
							res = gFileCache.request_file(mCacheRequest);
						}
					}

					string vers_info;
					if(res.out_status == cache_status_available && FILE_read_file_to_string(res.out_path, vers_info) == 0)
					{
						string AcceptDate;
						string Artist;
						Json::Value root2;
						Json::Reader reader2;
						bool success = reader2.parse(vers_info,root2);
						if(success == false)
							break;
						Json::Value airport = root2["airport"];
						Json::Value sceneryArray = Json::Value(Json::arrayValue);
						sceneryArray = airport["scenery"];
						for (Json::ValueIterator itr = sceneryArray.begin(); itr != sceneryArray.end(); itr++)
						{
							Json::Value curScenery = *itr;
							if(curScenery["Status"].asString() == "Accepted")
							{
								AcceptDate = curScenery.operator[]("dateAccepted").asString();
								if (AcceptDate == "")
									AcceptDate = "Unknown";
								Artist = curScenery.operator[]("userName").asString();
								cur_airport.kind_code = curScenery.operator[]("sceneryId").asInt();
								break;
							}
						}
						cur_airport.meta_data.push_back(make_pair(AcceptDate.substr(0,10), Artist));
					}
					else
						cur_airport.meta_data.push_back(make_pair("Unknown", "Dnld timed out"));
				}
				else
					cur_airport.kind_code = tmp["RecommendedSceneryId"].asInt();        // mis-using that property to support multi-airport import
			}
			else
			{
				if(tmp["ExcludeSubmissions"].asInt())
					cur_airport.meta_data.push_back(make_pair("Indefinitely", "Gateway Moderator"));
				else
				{
					string reserved = tmp["checkedOutBy"].asString();
					if (!reserved.empty())
						cur_airport.meta_data.push_back(make_pair(tmp["checkOutEndDate"].asString().substr(0,10), reserved));
				}
				if(tmp["Deprecated"].asInt() == 0)
					cur_airport.kind_code = tmp["RecommendedSceneryId"].asInt();        // mis-using that property to support multi-airport import
			}
			//Add the current scenery object's airport code
			mICAO_Apts.push_back(cur_airport);
		}
	}
	mICAO_AptProvider.AptVectorChanged();
	if(gModeratorMode) mICAO_AptProvider.SelectHeaderCell(3);
}

void WED_GatewayImportDialog::FillVersionsFromJSON(const string& json_string)
{
	//Now that we have our json_string we'll be turning it into a JSON object
	Json::Value root(Json::objectValue);

	Json::Reader reader;
	bool success = reader.parse(json_string,root);

	//Check for errors
	if(success == false)
	{
		mPhase = imp_dialog_error;

	#if DEV
		DecorateGUIWindow("Scenery list is invalid data");
	#else
		DecorateGUIWindow("The download was corrupted, please try again");
	#endif
		return;
	}
	Json::Value airport = root["airport"];
	Json::Value sceneryArray = airport["scenery"];

	//Clear the previous list no matter what
	mVersions_Vers.clear();
	//Build up the table
	for (Json::ValueIterator itr = sceneryArray.begin(); itr != sceneryArray.end(); itr++)
	{
		Json::Value curScenery = *itr;
		VerInfo_t tmp;

		//!!IMPORTANT!! Use of ".operator[]" because the author of jsoncpp didn't read Scott Meyer's "Item 26: Guard against potential ambiguity"!
		tmp.sceneryId     = curScenery.operator[]("sceneryId").asInt();
		tmp.icao          = airport["icao"].asString();
		tmp.isRecommended = tmp.sceneryId == airport.operator[]("recommendedSceneryId").asInt();

		tmp.parentId = curScenery.operator[]("parentId").asInt();
		tmp.userId   = curScenery.operator[]("userId").asInt();
		tmp.userName = curScenery.operator[]("userName").asString() != "" ? curScenery.operator[]("userName").asString() : "N/A";
		//Dates will appear as ISO8601: https://en.wikipedia.org/wiki/ISO_8601
		//For example 2014-07-31T14:34:47.000Z

		//If the date string exists go with the date string, else go with a default
		tmp.dateUploaded = curScenery.operator[]("dateUpload").asString()   != "" ? curScenery.operator[]("dateUpload").asString() : "0000-00-00T00:00:00.000Z";
		tmp.dateAccepted = curScenery.operator[]("dateAccepted").asString() != "" ? curScenery.operator[]("dateAccepted").asString() : "0000-00-00T00:00:00.000Z";
		tmp.dateApproved = curScenery.operator[]("dateApproved").asString() != "" ? curScenery.operator[]("dateApproved").asString() : "0000-00-00T00:00:00.000Z";
		tmp.type   = curScenery.operator[]("type").asString();		//2 for 2D =  3 for 3D
		tmp.status = curScenery["Status"].asString();

		//!!features is not needed to read!!
		//string features_str = curScenery.operator[]("features").asString();
		//tmp.features = vector<char>(features_str.begin(), features_str.end());

		tmp.artistComments    = curScenery.operator[]("artistComments").asString()    != "" ? curScenery.operator[]("artistComments").asString() : "N/A";
		tmp.moderatorComments = curScenery.operator[]("moderatorComments").asString() != "" ? curScenery.operator[]("moderatorComments").asString() : "N/A";

		//Catches cases where acceptedSceneryCount > 0 && some pack only has uploadeded status
		if(!(tmp.dateAccepted == "0000-00-00T00:00:00.000Z" && tmp.dateApproved == "0000-00-00T00:00:00.000Z"))
		{
			mVersions_Vers.push_back(tmp);
		}
	}
	mVersions_VerProvider.VerVectorChanged();
}

void WED_GatewayImportDialog::SelectWithFile()
{
	char c[256];
	GetFilePathFromUser(getFile_Open, "Pick file with airport ID's to be selected", "Next", 0, c, sizeof(c));

	FILE * fn = fopen(c, "r");
	if (fn)
	{
		int	low_x, low_y, high_x, high_y;
		char *icao;

		mICAO_AptProvider.SelectionStart(1);
		mICAO_AptProvider.SelectGetLimits(low_x, low_y, high_x, high_y);
		while (fgets(c, sizeof(c), fn))
		{
			icao = c;
			for(int i = 0; i < sizeof(c)-12; ++i)
				if (c[i] == ' ') ++icao;
				else break;

			icao[10] = 0;
			for(int i = 0; i < 10; ++i)
				if (icao[i] < '0' || (icao[i] > '9' && icao[i] < 'A') || icao[i] > 'Z')
				{
					icao[i] = 0;
					break;
				}
			for (int i = low_y; i <= high_y; ++i)
			{
				GUI_CellContent	content;
				mICAO_AptProvider.GetCellContent(low_x, i, content);
				if (content.text_val == icao)
				{
					mICAO_AptProvider.SelectionStart(0);
					mICAO_AptProvider.SelectRange(low_x, i, high_x, i, 0);
					break;
				}
			}
		}
		fclose(fn);
	}
}

void WED_GatewayImportDialog::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	switch(inMsg)
	{
	case filter_changed:
		mICAO_AptProvider.SetFilter(mFilter->GetText());
		mVersions_VerProvider.SetFilter(mFilter->GetText());
		break;
	case click_next:
		Next();
		break;
	case msg_DocumentDestroyed:
		this->AsyncDestroy();
		break;
	case click_back:
		Back();
		break;
	case click_extra:
		SelectWithFile();
	case GUI_TABLE_CONTENT_CHANGED:
		{
			set<int> sel;
			mICAO_AptProvider.GetSelection(sel);
			mNextButton->SetDescriptor(sel.size() < 2 ? "Next" : gModeratorMode ? "Import Accepted" : "Import Recommened");
		}
	}
}

void WED_GatewayImportDialog::StartCSVDownload()
{
	mCacheRequest.in_domain = cache_domain_metadata_csv;

	mCacheRequest.in_folder_prefix = "scenery_packs";
	mCacheRequest.in_url = WED_URL_AIRPORT_METADATA_CSV;

	Start(0.1);
	mLabel->Show();
}

void WED_GatewayImportDialog::StartICAODownload()
{

	mCacheRequest.in_domain = cache_domain_airports_json;
	mCacheRequest.in_folder_prefix = "scenery_packs" DIR_STR "GatewayImport";

	//Makes the url "https://gateway.x-plane.com/apiv1/airports"
	mCacheRequest.in_url = WED_get_GW_api_url() + "airports";

	Start(0.1);
	mLabel->Show();
}

//Starts the download process
bool WED_GatewayImportDialog::StartVersionsDownload()
{
	//Two steps,
	//1.Get the airport from the current selection, then get its sceneryid from mICAO_Apts

	//index of the current selection
	set<int> out_selection;
	mICAO_AptProvider.GetSelection(out_selection);
	if(out_selection.empty())
		return false;

	//Current airport selected
	AptInfo_t current_apt = mICAO_Apts.at(*out_selection.begin());

	//Makes the url "https://gateway.x-plane.com/apiv1/airport/ICAO"
	mCacheRequest.in_url = WED_get_GW_api_url() + "airport/" + current_apt.icao;
	mCacheRequest.in_domain = cache_domain_airport_versions_json;

	stringstream ss;
	ss << "scenery_packs" << DIR_STR << "GatewayImport" << DIR_STR << current_apt.icao;
	mCacheRequest.in_folder_prefix = ss.str();

	Start(0.1);
	mLabel->Show();
	return true;
}

bool WED_GatewayImportDialog::NextVersionsDownload()
{
	if(mVersions_VersionsSelected.size() == 0)
	{
		Stop();
		DecorateGUIWindow("Importing airport(s), please wait...");
		return false;
	}
	std::set<int>::iterator index = mVersions_VersionsSelected.begin();

	//Start the download
	mCacheRequest.in_url = WED_get_GW_api_url() + "scenery/" + to_string(mVersions_Vers[*index].sceneryId);
	mCacheRequest.in_domain = cache_domain_scenery_pack;
	mCacheRequest.in_folder_prefix = string("scenery_packs" DIR_STR "GatewayImport" DIR_STR) + mVersions_Vers[*index].icao;

	if(gFileCache.request_file(mCacheRequest).out_status == cache_status_available)
	{
		if(mVersions_VersionsSelected.size() > 1)
			DecorateGUIWindow(string("Airport ") + mVersions_Vers[*index].icao + " is cached already.");
		else
			DecorateGUIWindow("Importing airport(s), please wait...");
		Start(0.005);   // process w/no delay, but not too fast so multiple timer events accumulate
	}
	else
		Start(0.1);

	mVersions_VersionsSelected.erase(index);
	mLabel->Show();
	return true;
}

WED_Airport * WED_GatewayImportDialog::ImportSpecificVersion(const string& json_string)
{
	//Now that we have our rawJSONString we'll be turning it into a JSON object
	Json::Value root = Json::Value(Json::objectValue);
	Json::Reader reader;
	bool success = reader.parse(json_string,root);

	//Check for errors
	if(success == false)
	{
		mPhase = imp_dialog_error;
#if DEV
		DecorateGUIWindow("Downloaded JSON is invalid");
#else
		DecorateGUIWindow("The download was corrupted, please try again");//User facing
#endif
		this->AsyncDestroy();
		return NULL;
	}

	string zipString = root["scenery"]["masterZipBlob"].asString();
	vector<char> outString = vector<char>(zipString.length());

	char * outP;
	decode(&*zipString.begin(),&*zipString.end(),&*outString.begin(),&outP);

	//Fixes the terrible vector padding bug by shrinking it back down to precisely the correct size
	outString.resize(outP - &*outString.begin());

#if TEST_AT_START
	//Anything beyond cannot be tested at the start or wouldn't be useful.
	return NULL;
#endif
	string filePath("");

	ILibrarian * lib = WED_GetLibrarian(mResolver);
    lib->LookupPath(filePath);

	string mICAOid = root["scenery"]["icao"].asString();
	string zipPath = filePath + mICAOid + ".zip";

	if(!outString.empty())
	{
		RAII_FileHandle f(zipPath.c_str(),"wb");
		if(f)
		{
			size_t write_result = fwrite(&outString[0], sizeof(char), outString.size(), f());

			if(write_result != outString.size())
			{
				mPhase = imp_dialog_error;
#if DEV
				stringstream ss;
				ss << write_result;

				string wr = ss.str();
				ss.str() = "";
				ss << outString.size();
				string out_s = ss.str();
				DecorateGUIWindow("Could not create file at " + zipPath + ", write_result: " + wr + ", outstring.size(): " + out_s);
#else
				DecorateGUIWindow("Could not create file at " + zipPath + ", please ensure you have enough space and sufficient permissions");
#endif
				int removeVal = FILE_delete_file(zipPath.c_str(), false);
				if(removeVal != 0)
				{
					//DoUserAlert(string("Could not remove temporary file " + zipPath + ". You may delete this file if you wish").c_str());//TODO - is this not helpful to the user?
				}
				return NULL;//Exit before we can continue
			}
		}
		else
		{
			mPhase = imp_dialog_error;
			DecorateGUIWindow("Could not create file at " + zipPath + ", please ensure you have enough space and sufficient permissions");
			return NULL;
		}
	}

	bool has_dsf = false;

	string res = MemFileHandling(zipPath,filePath,mICAOid,has_dsf);

	if(res.size() != 0)
	{
		mPhase = imp_dialog_error;
		DecorateGUIWindow(res);
		return NULL;
	}

	WED_Thing * wrl = WED_GetWorld(mResolver);

	string aptdatPath = filePath + mICAOid + ".dat";

	//The one out_apt will be the WED_Thing we'll be putting the rest of our
	//Operation inside of
	vector<WED_Airport *> out_apt;
	WED_ImportOneAptFile(aptdatPath,wrl,&out_apt);

	WED_Airport * g = NULL;

	if(out_apt.empty())
        DoUserAlert("Import failed - no valid data found !");
    else
	{
        out_apt[0]->StateChanged();
		g = out_apt[0];
		g->SetSceneryID(root["scenery"]["sceneryId"].asInt());
	}

	string dsfTextPath = filePath + mICAOid + ".txt";
	if(has_dsf && g)
	{
		WED_ImportText(dsfTextPath.c_str(), (WED_Thing *) g);
	}

	// the auto-created groups are at this point always the children of the airport. Close them all.
	// for(auto c_ID ; )
	set<int> cat_list;
	int n_child = g->CountChildren();
	for (int c = 0; c < n_child; c++)
		cat_list.insert(g->GetNthChild(c)->GetID());
	mPropPane->SetClosed(cat_list);

	if(!out_apt.empty())
		WED_DoInvisibleUpdateMetadata(out_apt[0]);  // this also sets GUI 2D/3D metadata - so do it only after dsf contents has been added

#if !SAVE_ON_HDD && !GATEWAY_IMPORT_FEATURES
	//clean up our files ICAOid.dat and potentially ICAOid.txt
	if(has_dsf)
	{
		if(FILE_delete_file(dsfTextPath.c_str(),0) != 0)
		{
			//DoUserAlert(string("Could not remove temporary file " + dsfTextPath + ". You may delete this file if you wish").c_str());//TODO - is this not helpful to the user?
		}
	}
	if(FILE_delete_file(aptdatPath.c_str(),0) != 0)
	{
		//DoUserAlert(string("Could not remove temporary file " + aptdatPath + ". You may delete this file if you wish").c_str());//TODO - is this not helpful to the user?
	}
	if(FILE_delete_file(zipPath.c_str(),0) != 0)
	{
		//DoUserAlert(string("Could not remove temporary file " + zipPath + ". You may delete this file if you wish").c_str());//TODO - is this not helpful to the user?
	}
#endif

	return g;
}

void WED_GatewayImportDialog::DecorateGUIWindow(string labelDesc)
{
	switch(mPhase)
	{
	case imp_dialog_error:
		mBackButton->Hide();
		mExtraButton->Hide();

		mNextButton->Show();
		mNextButton->SetDescriptor("Exit");

		mLabel->Show();
		mLabel->SetDescriptor(labelDesc);

		mICAO_Packer->Hide();
		mVersions_Packer->Hide();
		break;
	case imp_dialog_download_ICAO:
		mBackButton->Show();
		mBackButton->SetDescriptor("Cancel");

		mExtraButton->Hide();
		mNextButton->Hide();

		mLabel->Show();
		mLabel->SetDescriptor(labelDesc);
		mICAO_Packer->Hide();
		mVersions_Packer->Hide();
		break;
	case imp_dialog_choose_ICAO:
		mBackButton->Show();
		mBackButton->SetDescriptor("Cancel");

		mNextButton->Show();
		mNextButton->SetDescriptor("Next");

		mExtraButton->Show();
		mExtraButton->SetDescriptor("Select with File");

		mLabel->Hide();

		mICAO_Packer->Show();
		mVersions_Packer->Hide();
		break;
	case imp_dialog_download_versions:
		mBackButton->Show();
		mBackButton->SetDescriptor("Back");

		mNextButton->Hide();
		mExtraButton->Hide();

		mLabel->Show();
		mLabel->SetDescriptor(labelDesc);

		mICAO_Packer->Hide();
		mVersions_Packer->Hide();
		break;
	case imp_dialog_choose_versions:
		mFilter->ClearFilter();

		mBackButton->Show();
		mBackButton->SetDescriptor("Back");

		mNextButton->Show();
		mNextButton->SetDescriptor("Import Pack(s)");

		mExtraButton->Hide();
		mLabel->Hide();

		mICAO_Packer->Hide();
		mVersions_Packer->Show();

		break;
	case imp_dialog_download_specific_version:
	default:
		mBackButton->Hide();
		mExtraButton->Hide();
		mNextButton->Hide();

		mLabel->Show();
		mLabel->SetDescriptor(labelDesc);

		mICAO_Packer->Hide();
		mVersions_Packer->Hide();
		break;
	}
}

void WED_GatewayImportDialog::MakeICAOTable(int bounds[4])
{
	mICAO_AptProvider.SetFilter(mFilter->GetText()); //This requires mApts to be full

	mICAO_Scroller = new GUI_ScrollerPane(0,1);
	mICAO_Scroller->SetParent(mICAO_Packer);
	mICAO_Scroller->Show();
	mICAO_Scroller->SetSticky(1,1,1,1);

	mICAO_TextTable.SetProvider(&mICAO_AptProvider);
	mICAO_TextTable.SetGeometry(&mICAO_AptProvider);
	mICAO_TextTable.AddListener(this);

	mICAO_TextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_Table_SelectText),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mICAO_TextTable.SetTextFieldColors(
				WED_Color_RGBA(wed_TextField_Text),
				WED_Color_RGBA(wed_TextField_Hilite),
				WED_Color_RGBA(wed_TextField_Bkgnd),
				WED_Color_RGBA(wed_TextField_FocusRing));

	mICAO_Table = new GUI_Table(true);
	mICAO_Table->SetGeometry(&mICAO_AptProvider);
	mICAO_Table->SetContent(&mICAO_TextTable);
	mICAO_Table->SetParent(mICAO_Scroller);
	mICAO_Table->SetSticky(1,1,1,1);
	mICAO_Table->Show();

	mICAO_Scroller->PositionInContentArea(mICAO_Table);
	mICAO_Scroller->SetContent(mICAO_Table);
	mICAO_TextTable.SetParentTable(mICAO_Table);

	mICAO_TextTableHeader.SetProvider(&mICAO_AptProvider);
	mICAO_TextTableHeader.SetGeometry(&mICAO_AptProvider);

	mICAO_TextTableHeader.SetImage("header.png");
	mICAO_TextTableHeader.SetColors(
			WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

	mICAO_Header = new GUI_Header(true);

	bounds[1] = 0;
	bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;
	mICAO_Header->SetBounds(bounds);
	mICAO_Header->SetGeometry(&mICAO_AptProvider);
	mICAO_Header->SetHeader(&mICAO_TextTableHeader);
	mICAO_Header->SetParent(mICAO_Packer);
	mICAO_Header->Show();
	mICAO_Header->SetSticky(1,0,1,1);
	mICAO_Header->SetTable(mICAO_Table);

					mICAO_TextTableHeader.AddListener(mICAO_Header);		// Header listens to text table to know when to refresh on col resize
					mICAO_TextTableHeader.AddListener(mICAO_Table);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mICAO_TextTable.AddListener(mICAO_Table);				// Table listens to text table to know when content changes in a resizing way
					mICAO_AptProvider.AddListener(mICAO_Table);			// Table listens to actual property content to know when data itself changes

	//mICAO_Packer->PackPane(mFilter,gui_Pack_Top);
	mICAO_Packer->PackPane(mICAO_Header,gui_Pack_Top);

	mICAO_Packer->PackPane(mICAO_Scroller,gui_Pack_Center);
	mICAO_Scroller->PositionHeaderPane(mICAO_Header);
}

void WED_GatewayImportDialog::MakeVersionsTable(int bounds[4])
{
	mVersions_VerProvider.SetFilter(mFilter->GetText());//This requires mVers to be full?

	mVersions_Scroller = new GUI_ScrollerPane(1,1);
	mVersions_Scroller->SetParent(mVersions_Packer);
	mVersions_Scroller->Show();
	mVersions_Scroller->SetSticky(1,1,1,1);

	mVersions_TextTable.SetProvider(&mVersions_VerProvider);
	mVersions_TextTable.SetGeometry(&mVersions_VerProvider);

	mVersions_TextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_Table_SelectText),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mVersions_TextTable.SetTextFieldColors(
				WED_Color_RGBA(wed_TextField_Text),
				WED_Color_RGBA(wed_TextField_Hilite),
				WED_Color_RGBA(wed_TextField_Bkgnd),
				WED_Color_RGBA(wed_TextField_FocusRing));

	mVersions_Table = new GUI_Table(true);
	mVersions_Table->SetGeometry(&mVersions_VerProvider);
	mVersions_Table->SetContent(&mVersions_TextTable);
	mVersions_Table->SetParent(mVersions_Scroller);
	mVersions_Table->SetSticky(1,1,1,1);
	mVersions_Table->Show();
	mVersions_Scroller->PositionInContentArea(mVersions_Table);
	mVersions_Scroller->SetContent(mVersions_Table);
	mVersions_TextTable.SetParentTable(mVersions_Table);

	mVersions_TextTableHeader.SetProvider(&mVersions_VerProvider);
	mVersions_TextTableHeader.SetGeometry(&mVersions_VerProvider);

	mVersions_TextTableHeader.SetImage("header.png");
	mVersions_TextTableHeader.SetColors(
			WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

	mVersions_Header = new GUI_Header(true);

	bounds[1] = 0;
	bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;
	mVersions_Header->SetBounds(bounds);
	mVersions_Header->SetGeometry(&mVersions_VerProvider);
	mVersions_Header->SetHeader(&mVersions_TextTableHeader);
	mVersions_Header->SetParent(mVersions_Packer);

	//--IMPORTANT, gets shown with back and next---
	mVersions_Header->Show();
	//---------------------------------------------//

	mVersions_Header->SetSticky(1,0,1,1);
	mVersions_Header->SetTable(mVersions_Table);


					mVersions_TextTableHeader.AddListener(mVersions_Header);		// Header listens to text table to know when to refresh on col resize
					mVersions_TextTableHeader.AddListener(mVersions_Table);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mVersions_TextTable.AddListener(mVersions_Table);				// Table listens to text table to know when content changes in a resizing way
					mVersions_VerProvider.AddListener(mVersions_Table);			// Table listens to actual property content to know when data itself changes

	mPacker->PackPane(mVersions_Header,gui_Pack_Top);
	mPacker->PackPane(mVersions_Scroller,gui_Pack_Center);
	mVersions_Scroller->PositionHeaderPane(mVersions_Header);
}
//-------------------------------------------------------------

int	WED_CanImportFromGateway(IResolver * resolver)
{
	return 1;
}

void WED_DoImportFromGateway(WED_Document * resolver, WED_MapPane * pane, WED_PropertyPane* prop_pane)
{
	new WED_GatewayImportDialog(resolver, pane, gApplication, prop_pane);
	return;
}

#endif /* HAS_GATEWAY */
