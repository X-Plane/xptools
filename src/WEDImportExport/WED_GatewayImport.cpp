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

#include "PlatformUtils.h"
#include "GUI_Resources.h"
#include "WED_Url.h"
#include "curl_http.h"
#include <curl/curl.h>

#include <json/json.h>


#include "WED_Document.h"
#include "WED_PackageMgr.h"

#include "GUI_Application.h"
#include "GUI_Window.h"
#include "GUI_Packer.h"
#include "GUI_Timer.h"
#include "WED_FilterBar.h"
#include "GUI_Button.h"
#include "WED_Messages.h"

//--DSF/AptImport
#include "WED_AptIE.h"
#include "WED_ToolUtils.h"

#include "WED_DSFImport.h"
#include "WED_Group.h"
//---------------

//--Table Code------------
#include "GUI_Table.h"
#include "GUI_TextTable.h"
#include "WED_Colors.h"

	//--ICAO Table------------
	#include "WED_AptTable.h"
	//------------------------//
	//--Version Table---------
	#include "WED_VerTable.h"
	//------------------------//

//------------------------//

//Heuristics for how big to initialize the rawJSONbuf, since it is faster to make a big chunk than make it constantly resize itself
//These should probably change as the gateway gets bigger and the downloads get longer in average charecter length
#define AIRPORTS_GET_SIZE_GUESS 9000000
#define VERSIONS_GET_SIZE_GUESS 4000
#define VERSION_GET_SIZE_GUESS  6000

enum imp_dialog_stages
{
imp_dialog_choose_ICAO,//Let the user choose the aiport from the table. The required GET is done before hand
imp_dialog_choose_versions,//Let user pick scenery pack(s) to download. The required GET is done before hand
imp_dialog_download_specific_version,//Download scenery pack, save the contents in the right place, import to WED
imp_dialog_finish//Clean up any processes left
};

enum imp_dialog_msg
{
	filter_changed,
	click_next,
	click_back
};

//Our private class for the import dialog
class WED_GatewayImportDialog : public GUI_Window, public GUI_Listener, public GUI_Timer, public GUI_Destroyable
{
public:
	WED_GatewayImportDialog(WED_Document * resolver, GUI_Commander * cmdr);
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

#if DEV
	friend class WED_AppMain;
#endif

	int mPhase;//Our simple stage counter for our simple fsm

	WED_Document *			 mResolver;
	
	//Our curl handle we'll be using to get the json files, note the s
	curl_http_get_file * mCurl;

	void DoAirportImport(string filePath);
//--GUI parts
	
	//--For the whole dialog box
	WED_FilterBar	 *		mFilter;
	GUI_Packer *			mPacker;

	GUI_Pane *				mButtonHolder;
	GUI_Button *			mNextButton;
	GUI_Button *			mBackButton;

	static int				import_bounds_default[4];

	//--ICAO Table
	GUI_Packer *			mICAO_Packer;
	GUI_ScrollerPane *		mICAO_Scroller;
	GUI_Table *				mICAO_Table;
	GUI_Header *			mICAO_Header;

	GUI_TextTable			mICAO_TextTable;
	GUI_TextTableHeader		mICAO_TextTableHeader;
	
	//Starts the download of the json file containing the airports list
	void StartICAODownload();
	//From the downloaded JSON, fill the ICAO table
	void FillICAOFromJSON();
		//--ICAO Table Provider/Geometry
		WED_AptTable			mICAO_AptProvider;
		AptVector				mICAO_Apts;

		//Sets up the ICAO table GUI and event handlers
		void MakeICAOTable(int bounds[4]);
		//----------------------//
	//----------------------//

	//--Versions Table----------
	GUI_Packer *			mVersions_Packer;
	GUI_ScrollerPane *		mVersions_Scroller;
	GUI_Table *				mVersions_Table;
	GUI_Header *			mVersions_Header;

	GUI_TextTable			mVersions_TextTable;
	GUI_TextTableHeader		mVersions_TextTableHeader;
	
	//Starts the download of the json file to populate the versions table
	void StartVersionsDownload();

	//From the downloaded JSON, fill the versions table
	void FillVersionsFromJSON();
	//Starts the download of the specific version, aiming to get the masterZipBlob contained
	void StartSpecificVersionDownload(int id);

		//--Versions Table Provider/Geometry
		WED_VerTable			mVersions_VerProvider;
		VerVector				mVersions_Vers;

		//Sets up the Versions table GUI and event handlers
		void MakeVersionsTable(int bounds[4]);
		//---------------------//
	//--------------------------//
		
//----------------------//
	
};
int WED_GatewayImportDialog::import_bounds_default[4] = { 0, 0, 500, 500 };

//TODO, put these in better places

//a buffer of chars to be filled, reserve a huge amount of space for it cause we'll need it
vector<char> rawJSONBuf;

string ICAOid;

//--Implemation of WED_GateWayImportDialog class---------------
WED_GatewayImportDialog::WED_GatewayImportDialog(WED_Document * resolver, GUI_Commander * cmdr) :
	GUI_Window("Import from Gateway",xwin_style_visible|xwin_style_centered,import_bounds_default,cmdr),
	mResolver(resolver),
	mPhase(imp_dialog_choose_ICAO),
	mCurl(NULL),
	mICAO_AptProvider(&mICAO_Apts),
	mICAO_TextTable(this,100,0),
	mVersions_VerProvider(&mVersions_Vers),
	mVersions_TextTable(this,100,0)
{
	mResolver->AddListener(this);

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
	mFilter = new WED_FilterBar(this,filter_changed,0,"Filter:","",NULL,false);
	mFilter->Show();
	mFilter->SetSticky(1,0,1,1);
	mFilter->SetParent(mPacker);
	mFilter->AddListener(this);
	mPacker->PackPane(mFilter,gui_Pack_Top);

	//--Button Setup
		int k_reg[4] = { 0, 0, 1, 3 };
		int k_hil[4] = { 0, 1, 1, 3 };
	
		mNextButton = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
		mNextButton->SetBounds(105,5,205,GUI_GetImageResourceHeight("push_buttons.png") / 3);
		mNextButton->Show();
		mNextButton->SetSticky(0,1,1,0);
		mNextButton->SetDescriptor("Next");
		mNextButton->SetMsg(click_next,0);

		mBackButton = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
		mBackButton->SetBounds(5,5,105,GUI_GetImageResourceHeight("push_buttons.png") / 3);
		mBackButton->Show();
		mBackButton->SetSticky(1,1,0,0);
		mBackButton->SetDescriptor("Cancel");
		mBackButton->SetMsg(click_back,0);
	
		mButtonHolder = new GUI_Pane;
		mButtonHolder->SetBounds(0,0,210,GUI_GetImageResourceHeight("push_buttons.png") / 3 + 10);

		mNextButton->SetParent(mButtonHolder);
		mNextButton->AddListener(this);
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
		tableHolder->SetSticky(1,1,1,0);
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
	StartICAODownload();
}

WED_GatewayImportDialog::~WED_GatewayImportDialog()
{
	if(mCurl != NULL)
	{
		delete mCurl;
		mCurl = NULL;
	}
}

void WED_GatewayImportDialog::DoAirportImport(string filePath)
{

}

void WED_GatewayImportDialog::Next()
{
	/*From imp_choose_ICAO to imp_choose_versions (in no particular order
	* SetDescriptor for back button
	* Start Timer for downloading the VersionJSON for the ICAO selection
	* change mPhase
	* hide ICAOTable stuff
	* show VersionsTable stuff
	*/
	switch(mPhase)
	{
	case imp_dialog_choose_ICAO:
		//Going to show versions
		StartVersionsDownload();
		mICAO_Packer->Hide();
		mVersions_Packer->Show();
		/*
		mICAO_Scroller->Hide();
		mICAO_Header->Hide();
		mVersions_Scroller->Show();
		mVersions_Header->Show();*/
		break;
	case imp_dialog_choose_versions:
		StartSpecificVersionDownload(mVersions_Vers[0].sceneryId);
		break;
	case imp_dialog_download_specific_version:
		break;
	case imp_dialog_finish:

		this->AsyncDestroy();//?
		break;
	}
	mPhase++;
}

void WED_GatewayImportDialog::Back()
{
	switch(mPhase)
	{
	case imp_dialog_choose_ICAO:
		this->AsyncDestroy();
		break;
	case imp_dialog_choose_versions:
		mICAO_Packer->Show();
		mVersions_Packer->Hide();
		/*
		mICAO_Scroller->Show();
		mICAO_Header->Show();
		mVersions_Scroller->Hide();
		mVersions_Header->Hide();*/
		break;
	case imp_dialog_finish:
		break;
	}
	mPhase--;
}

extern "C" void decode( const char * startP, const char * endP, char * destP, char ** out);
void WED_GatewayImportDialog::TimerFired()
{
	if(mCurl->is_done())
	{
		Stop();

		if(mCurl->is_ok())
		{
			if(mPhase == imp_dialog_choose_ICAO)
			{
				FillICAOFromJSON();
			}		
			else if(mPhase == imp_dialog_choose_versions)
			{
				FillVersionsFromJSON();
			}
			else if(mPhase == imp_dialog_download_specific_version)
			{
				//create a string from the vector of chars
				
				string rawJSONString = string(rawJSONBuf.begin(),rawJSONBuf.end());

				//Now that we have our rawJSONString we'll be turning it into a JSON object
				
				Json::Value root = Json::Value(Json::objectValue);
				Json::Reader reader;
				bool success = reader.parse(rawJSONString,root);

				//Check for errors
				if(success == false)
				{
					DoUserAlert("Airports list is invalid data, ending dialog box");
					this->AsyncDestroy();
					return;
				}

				string zipString = root["scenery"]["masterZipBlob"].asString();
				vector<char> outString = vector<char>(zipString.length());

				char * outP;
				decode(&*zipString.begin(),&*zipString.end(),&*outString.begin(),&outP);

				
				/*string filePath;// = "C:\\Users\\Ted\\Desktop\\" + ICAOid + ".zip";
				gPackageMgr->GetXPlaneFolder(filePath);
				filePath += "\\Custom Scenery";
				*/
				//YES I KNOW THIS IS SO BAD BUT I CAN'T FIGURE IT OUT OTHERWISE!
				string filePath = mResolver->GetFilePath();

				//earth.wed.xml = 13
				filePath.erase(filePath.size()-13-1);
				filePath += "\\"+ ICAOid + ".zip";

				FILE * f = fopen(filePath.c_str(),"wb");
				for (int i = 0; i < outString.size(); i++)
				{
					char c = outString[i];
					fprintf(f,"%c",c);
				}

				fclose(f);

				//--This is where the MemFilesAPI will head into
				int breakhereandmanuallyunzip = 0;
				
				WED_Thing * wrl = WED_GetWorld(mResolver);

				//char * paths = //GetMultiFilePathFromUser("Import DSF file...", "Import", FILE_DIALOG_IMPORT_DSF);
				//Change ICAO.zip to ICAO.dat
				string aptDOTdat_path = filePath.substr(0,filePath.size()-4) + ".dat";
		
				wrl->StartOperation("Import DSF");
				WED_ImportOneAptFile(aptDOTdat_path,wrl);

				WED_Thing * //= NULL;// = find_airport_by_icao_recursive(ICAOid,wrl);
				//if(g == NULL)
				//{
					g = WED_Group::CreateTyped(wrl->GetArchive());
					g->SetName(aptDOTdat_path);
					g->SetParent(wrl,wrl->CountChildren());
				//}
				string dsf_txt_path = filePath.substr(0,filePath.size()-4) + ".txt";
				DSF_Import(dsf_txt_path.c_str(), (WED_Group *) g);
				wrl->CommitOperation();
				
				//Set it back so we can download another
				mPhase = imp_dialog_choose_versions;
			}//end if(mPhase == imp_dialog_download_specific_version
		}//end if(mCurl->is_ok())
		else if(mCurl->get_error() <= CURL_LAST)//start our error checking process
		{
			
		}
	}//end if(mCurl->is_done())
}//end WED_GatewayImportDialog::TimerFired()

void WED_GatewayImportDialog::FillICAOFromJSON()
{
	//create a string from the vector of chars
	string rawJSONString = string(rawJSONBuf.begin(),rawJSONBuf.end());

	Json::Value root;
	Json::Reader reader;
	bool success = reader.parse(rawJSONString,root);
	
	//Check for errors
	if(success == false)
	{
		DoUserAlert("Airports list is invalid data, ending dialog box");
		this->AsyncDestroy();
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

		AptInfo_t cur_airport;
		cur_airport.icao = tmp["AirportCode"].asString();
		cur_airport.name = tmp["AirportName"].asString();

		//Add the current scenery object's airport code
		mICAO_Apts.push_back(cur_airport);
	}
	mICAO_AptProvider.AptVectorChanged();
}

void WED_GatewayImportDialog::FillVersionsFromJSON()
{
	//create a string from the vector of chars
	string rawJSONString = string(rawJSONBuf.begin(),rawJSONBuf.end());

	//Now that we have our rawJSONString we'll be turning it into a JSON object
	Json::Value root(Json::objectValue);
	
	Json::Reader reader;
	bool success = reader.parse(rawJSONString,root);
	
	//Check for errors
	if(success == false)
	{
		DoUserAlert("Scenery list is invalid data, ending dialog box");
		this->AsyncDestroy();
		return;
	}
	Json::Value airport = root["airport"];
	Json::Value sceneryArray = Json::Value(Json::arrayValue);
	sceneryArray = airport["scenery"];

	//Build up the table
	for (Json::ValueIterator itr = sceneryArray.begin(); itr != sceneryArray.end(); itr++)
	{
		//TODO - actually access the scenery pack, you are currently accessing the array of scenery packs
		Json::Value curScenery = *itr;
		VerInfo_t tmp;
					
		tmp.sceneryId = curScenery.operator[]("sceneryId").asInt();
		tmp.parentId = curScenery.operator[]("parentId").asInt();
		tmp.userId = curScenery.operator[]("userId").asInt();
		tmp.userName = curScenery.operator[]("userName").asString();
		//Dates will appear as ISO8601: https://en.wikipedia.org/wiki/ISO_8601
		//For example 2014-07-31T14:34:47.000Z
		tmp.dateUploaded = curScenery.operator[]("dateUpload").asString();
		tmp.dateAccepted = curScenery.operator[]("dateAccepted").asString();
		tmp.dateApproved = curScenery.operator[]("dateApproved").asString();
		//2 for 2D =  3 for 3D
		tmp.type = curScenery.operator[]("type").asString();
		//TODO-tmp.features = curScenery.operator[]("features").asString();
		tmp.artistComments = curScenery.operator[]("artistComments").asString();
		tmp.moderatorComments = curScenery.operator[]("moderatorComments").asString();
					
		mVersions_Vers.push_back(tmp);
	}
	mVersions_VerProvider.VerVectorChanged();
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
	}
}

void WED_GatewayImportDialog::StartICAODownload()
{
	rawJSONBuf = vector<char>(AIRPORTS_GET_SIZE_GUESS);
	string url = WED_URL_GATEWAY_API;
	
	//Get Certification
	string cert;
	if(!GUI_GetTempResourcePath("gateway.crt", cert))
	{
		DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
		return;
	}

	//Makes the url "https://gatewayapi.x-plane.com:3001/apiv1/airports"
	url += "airports";

	//Get it from the server
	mCurl = new curl_http_get_file(url,&rawJSONBuf,cert);
	Start(1.0);
}

//Starts the download process
void WED_GatewayImportDialog::StartVersionsDownload()
{
	rawJSONBuf = vector<char>(VERSIONS_GET_SIZE_GUESS);
	//Two steps,
	//1.Get the airport from the current selection, then get its sceneryid from mAirportsGET

	//index of the current selection
	set<int> out_selection;
	mICAO_AptProvider.GetSelection(out_selection);
	
	//Current airport selected
	AptInfo_t current_apt = mICAO_Apts.at(*out_selection.begin());
	
	
	ICAOid = current_apt.icao;
	
	//Get Certification
	string cert;
	if(!GUI_GetTempResourcePath("gateway.crt", cert))
	{
		DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
		return;
	}
	
	string url = WED_URL_GATEWAY_API;
	//Makes the url "https://gatewayapi.x-plane.com:3001/apiv1/airport/ICAO"
	url += "airport/" + ICAOid;

	//Get it from the server
	mCurl = new curl_http_get_file(url,&rawJSONBuf,cert);
	Start(1.0);
}

void WED_GatewayImportDialog::StartSpecificVersionDownload(int id)
{
	//a buffer of chars to be filled, reserve a huge amount of space for it cause we'll need it
	rawJSONBuf = vector<char>(VERSION_GET_SIZE_GUESS);

	//Get Certification
	string cert;
	if(!GUI_GetTempResourcePath("gateway.crt", cert))
	{
		DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
		return;
	}

	string url = WED_URL_GATEWAY_API;

	//This can store up to INT_MAX
	char idStr[5] = {0};
	itoa(id,idStr,10);
	url += "scenery/" + string(idStr);
	//Get it from the server
	mCurl = new curl_http_get_file(url,&rawJSONBuf,cert);
	Start(1.0);
}

void WED_GatewayImportDialog::MakeICAOTable(int bounds[4])
{
	mICAO_AptProvider.SetFilter(mFilter->GetText());//This requires mApts to be full
	
	mICAO_Scroller = new GUI_ScrollerPane(0,1);
	mICAO_Scroller->SetParent(mICAO_Packer);
	mICAO_Scroller->Show();
	mICAO_Scroller->SetSticky(1,1,1,1);

	mICAO_TextTable.SetProvider(&mICAO_AptProvider);
	mICAO_TextTable.SetGeometry(&mICAO_AptProvider);

	
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

void WED_DoImportFromGateway(WED_Document * resolver)
{
	new WED_GatewayImportDialog(resolver,gApplication);
	return;
}