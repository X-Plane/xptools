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

#include "GUI_Application.h"
#include "GUI_Window.h"

#include "GUI_Packer.h"
#include "GUI_Timer.h"
#include "WED_FilterBar.h"
#include "GUI_Button.h"
#include "WED_Messages.h"
#include "base64.h"

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

//Decides where you want to test getting the JSON info from
//Recomend using test from local for speed
#define TEST_FROM_SERVER 1
#define LOCAL_JSON "C:\\airports.txt"
#define GUESS_AIRPORTS_SIZE 34151

#if !TEST_FROM_SERVER
#include <fstream>
#endif

enum import_dialog_stages
{
imp_dialog_choose_ICAO,
imp_dialog_choose_versions,
imp_dialog_finish
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
private:
	friend class WED_AppMain;
	

	int mPhase;//Our simple stage counter for our simple fsm

	IResolver *			 mResolver;
	
	//Our curl handle we'll be using to get the json files, note the s
	curl_http_get_file * mCurl;

	//The JSON values are saved so we don't have to redownload them everytime

	//The large airport json file, cached so when we go back we don't need to redownload
	static Json::Value		mAirportsGET;

	//The smaller scenery json file, obtained after hitting next
	Json::Value				mSceneryGET;

//--GUI parts
	
	//--For the whole dialog box
	WED_FilterBar	 *		mFilter;
	GUI_Packer *			mPacker;
	
	GUI_Pane *				mButtonHolder;
	GUI_Button *			mNextButton;
	GUI_Button *			mBackButton;

	static int				import_bounds_default[4];

	//--ICAO Table
	GUI_ScrollerPane *		mICAO_Scroller;
	GUI_Table *				mICAO_Table;
	GUI_Header *			mICAO_Header;

	GUI_TextTable			mICAO_TextTable;
	GUI_TextTableHeader		mICAO_TextTableHeader;
	
		//--ICAO Table Provider/Geometry
		WED_AptTable			mICAO_AptProvider;
		AptVector				mICAO_Apts;

		//Extracts the Code from the constructor
		void MakeICAOTable(int bounds[4]);
		
		//Downloads the json file specified by current stage of the program
		void DownloadICAOJSON();
		//----------------------//
	//----------------------//

	//--Versions Table----------

	GUI_ScrollerPane *		mVersions_Scroller;
	GUI_Table *				mVersions_Table;
	GUI_Header *			mVersions_Header;

	GUI_TextTable			mVersions_TextTable;
	GUI_TextTableHeader		mVersions_TextTableHeader;
	
		//--Versions Table Provider/Geometry
		WED_VerTable			mVersions_VerProvider;
		VerVector				mVersions_Vers;

		//Extracts the Code from the constructor
		void MakeVersionsTable(int bounds[4]);

		//Downloads the json file specified by current stage of the program
		void DownloadVersionsJSON();
		//---------------------//
	//--------------------------//
		
//----------------------//
	
};
int WED_GatewayImportDialog::import_bounds_default[4] = { 0, 0, 500, 500 };
Json::Value WED_GatewayImportDialog::mAirportsGET = Json::Value(Json::arrayValue);

//TODO put as member
	//a buffer of chars to be filled, reserve a huge amount of space for it cause we'll need it
	vector<char> rawJSONBuf = vector<char>(8000000);
	
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
	if(resolver)
	resolver->AddListener(this);

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

	
	MakeICAOTable(bounds);
	
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

	mPacker->PackPane(mICAO_Scroller,gui_Pack_Center);

	mICAO_Scroller->PositionHeaderPane(mICAO_Header);


	DownloadICAOJSON();
	Start(1.0);
}

WED_GatewayImportDialog::~WED_GatewayImportDialog()
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
		DownloadVersionsJSON();
		break;
	case imp_dialog_choose_versions:
		this->AsyncDestroy();//?
		break;
	case imp_dialog_finish:
		this->AsyncDestroy();//?
		break;
	}
	mPhase++;
	return;
}

void WED_GatewayImportDialog::Back()
{
	switch(mPhase)
	{
	case imp_dialog_choose_ICAO:
		this->AsyncDestroy();
		break;
	case imp_dialog_choose_versions:
		break;
	case imp_dialog_finish:

		break;
	}
	mPhase--;
}

void WED_GatewayImportDialog::TimerFired()
{

	if(
#if TEST_FROM_SERVER
		mCurl->is_done()
#else
		true
#endif
		)
	{
		Stop();
		
		string good_msg, bad_msg;
		
		if(
#if TEST_FROM_SERVER
		mCurl->is_done()
#else
		true
#endif
		)
		{
			//create a string from the vector of chars
			string rawJSONString = string(rawJSONBuf.begin(),rawJSONBuf.end());
			
			#if !TEST_FROM_SERVER
				std::ifstream is(LOCAL_JSON);     // open file
	
				while (is.good())          // loop while extraction from file is possible
				{
					char c = is.get();       // get character from file
					if (is.good())
					{
						rawJSONString += c;
					}
				}
				is.close();
			#endif
			
			Json::Reader reader;
			bool success = reader.parse(rawJSONString,mAirportsGET);
	
			//Check for errors
			if(success == false)
			{
				DoUserAlert("Airports list is invalid data, ending dialog box");
				this->AsyncDestroy();
				return;
			}

			//Devrived from the root's "airports" value
			mAirportsGET = mAirportsGET["airports"];

			//loop through the whole array
			for (int i = 0; i < mAirportsGET.size(); i++)
			{
				//Get the current scenery object
				Json::Value tmp(Json::objectValue);
				tmp = mAirportsGET.operator[](i);//Yes, you need the verbose operator[] form, yes it's dumb

				AptInfo_t cur_airport;
				cur_airport.icao = tmp["AirportCode"].asString();
				cur_airport.name = tmp["AirportName"].asString();

				//Add the current scenery object's airport code
				mICAO_Apts.push_back(cur_airport);
			}

			mICAO_AptProvider.AptVectorChanged();
		}
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


void WED_GatewayImportDialog::MakeICAOTable(int bounds[4])
{
	mICAO_AptProvider.SetFilter(mFilter->GetText());//This requires mApts to be full
	
	mICAO_Scroller = new GUI_ScrollerPane(0,1);
	mICAO_Scroller->SetParent(this);
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
	mICAO_Header->SetParent(this);
	mICAO_Header->Show();
	mICAO_Header->SetSticky(1,0,1,1);
	mICAO_Header->SetTable(mICAO_Table);


					mICAO_TextTableHeader.AddListener(mICAO_Header);		// Header listens to text table to know when to refresh on col resize
					mICAO_TextTableHeader.AddListener(mICAO_Table);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mICAO_TextTable.AddListener(mICAO_Table);				// Table listens to text table to know when content changes in a resizing way
					mICAO_AptProvider.AddListener(mICAO_Table);			// Table listens to actual property content to know when data itself changes

	mPacker->PackPane(mFilter,gui_Pack_Top);
	mPacker->PackPane(mICAO_Header,gui_Pack_Top);
}

void WED_GatewayImportDialog::DownloadICAOJSON()
{
	if(mAirportsGET.size() == 0)
	{
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

	#if TEST_FROM_SERVER
		//Get it from the server
		mCurl = new curl_http_get_file(url,&rawJSONBuf,cert);
	#endif
	}
	else
	{
		for (int i = 0; i < mAirportsGET.size(); i++)
		{
			//Get the current scenery object
			Json::Value tmp(Json::objectValue);
			tmp = mAirportsGET.operator[](i);//Yes, you need the verbose operator[] form, yes it's dumb

			AptInfo_t cur_airport;
			cur_airport.icao = tmp["AirportCode"].asString();
			cur_airport.name = tmp["AirportName"].asString();

			//Add the current scenery object's airport code
			mICAO_Apts.push_back(cur_airport);

			//Optionally print it out
		}

		mICAO_AptProvider.AptVectorChanged();
	}
}

//Extracts the Code from the constructor
void WED_GatewayImportDialog::MakeVersionsTable(int bounds[4])
{
	mVersions_VerProvider.SetFilter(mFilter->GetText());//This requires mApts to be full
	
	mVersions_Scroller = new GUI_ScrollerPane(1,1);
	mVersions_Scroller->SetParent(this);
	mVersions_Scroller->Hide();
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
	mVersions_Header->SetParent(this);
	mVersions_Header->Show();
	mVersions_Header->SetSticky(1,0,1,1);
	mVersions_Header->SetTable(mVersions_Table);


					mVersions_TextTableHeader.AddListener(mVersions_Header);		// Header listens to text table to know when to refresh on col resize
					mVersions_TextTableHeader.AddListener(mVersions_Table);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mVersions_TextTable.AddListener(mVersions_Table);				// Table listens to text table to know when content changes in a resizing way
					mVersions_VerProvider.AddListener(mVersions_Table);			// Table listens to actual property content to know when data itself changes

	mPacker->PackPane(mFilter,gui_Pack_Top);
	mPacker->PackPane(mVersions_Header,gui_Pack_Top);
}

extern "C" void decode( const char * startP, const char * endP, char * destP, char ** out);

//Downloads the json file specified by current stage of the program
void WED_GatewayImportDialog::DownloadVersionsJSON()
{
	//Two steps,
	//1.Get the airport from the current selection, then get its sceneryid from mAirportsGET
	//2.pull from the sever with https://gatewayapi.x-plane.com:3001/apiv1/scenery/XXXX
	
	//index of the current selection
	set<int> out_selection;
	mICAO_AptProvider.GetSelection(out_selection);
	
	//Current airport selected
	AptInfo_t current_apt = mICAO_Apts.at(*out_selection.begin());
	
	string ICAOid;
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

	//a buffer of chars to be filled, reserve a huge amount of space for it cause we'll need it
	vector<char> rawJSONBuf = vector<char>();
	
	//Get it from the server
	curl_http_get_file file = curl_http_get_file(url,&rawJSONBuf,cert);

	//Lock up everything until the file is finished
	while(file.is_done() == false);

	//create a string from the vector of chars
	string rawJSONString = string(rawJSONBuf.begin(),rawJSONBuf.end());

	//Now that we have our rawJSONString we'll be turning it into a JSON object
	Json::Value root(Json::objectValue);
	
	Json::Reader reader;
	bool success = reader.parse(rawJSONString,root);
	
	//Check for errors
	if(success == false)
	{
		DoUserAlert("Airports list is invalid data, ending dialog box");
		this->AsyncDestroy();
		return;
	}
	Json::Value airport = root["airport"];
	Json::Value sceneryArray = Json::Value(Json::arrayValue);
	sceneryArray = airport["scenery"];

	//TODO - Needs to work for n number of scenery packs
	Json::Value arrValue1 = *(sceneryArray.begin());
	Json::Value v2 = arrValue1["sceneryId"];
	
	string sceneryId = v2.toStyledString();
	cout << sceneryId << endl;
	
	//Makes the url "https://gatewayapi.x-plane.com:3001/apiv1/scenery/XXXX"
	url.clear();
	url = WED_URL_GATEWAY_API;
	url += "scenery/" + sceneryId;

	//a buffer of chars to be filled, reserve a huge amount of space for it cause we'll need it
	rawJSONBuf.clear();
	
	//Get it from the server
	curl_http_get_file file2 = curl_http_get_file(url,&rawJSONBuf,cert);

	//Lock up everything until the file is finished
	while(file2.is_done() == false);

	//create a string from the vector of chars
	rawJSONString.clear();
	rawJSONString = string(rawJSONBuf.begin(),rawJSONBuf.end());

	//Now that we have our rawJSONString we'll be turning it into a JSON object
	root.clear();
	root = Json::Value(Json::objectValue);
	
	bool was_success = reader.parse(rawJSONString,root);

	string zipString = root["scenery"]["masterZipBlob"].asString();
	vector<char> outString = vector<char>(zipString.length());

	/*string	b64;
		result->GetContents(b64);
		vector<char>	abuf;
		abuf.resize(b64.length());		// Post-B64 decoding is always at least smaller than in b64.
		char * inp = &*abuf.begin();
		char *	outP;*/
		//decode(&*b64.begin(), &*b64.end(), inp, &outP);
	char * outP;
	decode(&*zipString.begin(),&*zipString.end(),&*outString.begin(),&outP);
	//base64_decode(zipBlob,outString);
	
	string filePath = "C:\\Users\\Ted\\Desktop\\" + ICAOid + ".zip";
	FILE * f = fopen(filePath.c_str(),"wb");
	for (int i = 0; i < outString.size(); i++)
	{
		char c = outString[i];
		fprintf(f,"%c",c);
	}

	fclose(f);
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