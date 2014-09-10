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

//--Table Code------------
#include "GUI_Table.h"
#include "GUI_TextTable.h"
#include "WED_Colors.h"

	//--ICAO Table------------
	#include "WED_AptTable.h"
	//------------------------//
	//--Version Table---------
	//------------------------//

//------------------------//

//Decides where you want to test getting the JSON info from
//Recomend using test from local for speed
#define TEST_FROM_SERVER 0
#define LOCAL_JSON "C:\\airports.txt"
#define GUESS_AIRPORTS_SIZE 34151

#if !TEST_FROM_SERVER
#include <fstream>
#endif

enum import_dialog_stages
{
imp_dialog_choose_ICAO,
imp_dialog_choose_version,
imp_dialog_finish
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

	//Downloads the json file specified by current stage of the program
	void DownloadJSON(import_dialog_stages stage);

	int mPhase;//Our simple stage counter for our simple fsm

	IResolver *			 mResolver;
	
	//Our curl handle we'll be using to get the json files, note the s
	curl_http_get_file * mCurl;

	//The JSON values are saved so we don't have to redownload them everytime

	//The large airport json file, obtained during construction, TODO - bad idea
	Json::Value				mAirportsGET;

	//The smaller scenery json file, obtained after hitting next
	Json::Value				mSceneryGET;

//--GUI parts
	
	//--For the whole dialog box
	WED_FilterBar	 *		mFilter;
	GUI_ScrollerPane *		mScroller;

	//The only thing that will change is the providers
	GUI_Table *				mTable;
	GUI_Header *			mHeader;

	GUI_TextTable			mTextTable;
	GUI_TextTableHeader		mTextTableHeader;
	
	GUI_Pane *				mButtonHolder;
	GUI_Button *			mNextButton;
	GUI_Button *			mBackButton;

	static int import_bounds_default[4];
		
		//--ICAO Table Provider/Geometry
		WED_AptTable			mAptProvider;
		AptVector				mApts;

		//Extracts the Code from the constructor
		void MakeICAOTable(int bounds[4]);
		//----------------------//

		//--Versions Table Provider/Geometry

		//---------------------//

	//----------------------//
//----------------------//
	
};
int WED_GatewayImportDialog::import_bounds_default[4] = { 0, 0, 500, 500 };

//--Implemation of WED_GateWayImportDialog class---------------
WED_GatewayImportDialog::WED_GatewayImportDialog(WED_Document * resolver, GUI_Commander * cmdr) :
	GUI_Window("Import from Gateway",xwin_style_visible|xwin_style_centered,import_bounds_default,cmdr),
	mResolver(resolver),
	mPhase(imp_dialog_choose_ICAO),
	mCurl(NULL),
	mAptProvider(&mApts),
	mTextTable(this,100,0)
{
	DownloadJSON(imp_dialog_choose_ICAO);

	resolver->AddListener(this);

	//Packer
	int bounds[4];
	GUI_Packer * packer = new GUI_Packer;
	packer->SetParent(this);
	packer->SetSticky(1,1,1,1);
	packer->Show();
	GUI_Pane::GetBounds(bounds);
	packer->SetBounds(bounds);
	packer->SetBkgkndImage ("gradient.png");

	//Filter
	mFilter = new WED_FilterBar(this,3008,0,"Filter:","",NULL,false);//TODO - kMsg_filterchanged
	mFilter->Show();
	mFilter->SetSticky(1,0,1,1);
	mFilter->SetParent(packer);
	mFilter->AddListener(this);

	mAptProvider.SetFilter(mFilter->GetText());//This requires mApts to be full
	
	mScroller = new GUI_ScrollerPane(0,1);
	mScroller->SetParent(this);
	mScroller->Show();
	mScroller->SetSticky(1,1,1,1);

	mTextTable.SetProvider(&mAptProvider);
	mTextTable.SetGeometry(&mAptProvider);

	
	mTextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_Table_SelectText),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable.SetTextFieldColors(
				WED_Color_RGBA(wed_TextField_Text),
				WED_Color_RGBA(wed_TextField_Hilite),
				WED_Color_RGBA(wed_TextField_Bkgnd),
				WED_Color_RGBA(wed_TextField_FocusRing));

	mTable = new GUI_Table(true);
	mTable->SetGeometry(&mAptProvider);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->SetSticky(1,1,1,1);
	mTable->Show();	
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mTextTable.SetParentTable(mTable);

	mTextTableHeader.SetProvider(&mAptProvider);
	mTextTableHeader.SetGeometry(&mAptProvider);

	mTextTableHeader.SetImage("header.png");
	mTextTableHeader.SetColors(
			WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

	mHeader = new GUI_Header(true);

	bounds[1] = 0;
	bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;
	mHeader->SetBounds(bounds);
	mHeader->SetGeometry(&mAptProvider);
	mHeader->SetHeader(&mTextTableHeader);
	mHeader->SetParent(this);
	mHeader->Show();
	mHeader->SetSticky(1,0,1,1);
	mHeader->SetTable(mTable);


					mTextTableHeader.AddListener(mHeader);		// Header listens to text table to know when to refresh on col resize
					mTextTableHeader.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
					mAptProvider.AddListener(mTable);			// Table listens to actual property content to know when data itself changes

	packer->PackPane(mFilter,gui_Pack_Top);
	packer->PackPane(mHeader,gui_Pack_Top);
	
	int k_reg[4] = { 0, 0, 1, 3 };
	int k_hil[4] = { 0, 1, 1, 3 };
	
	GUI_Button * okay_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	okay_btn->SetBounds(105,5,205,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	okay_btn->Show();
	okay_btn->SetSticky(0,1,1,0);
	okay_btn->SetDescriptor("Import");
	okay_btn->SetMsg(0,0);

	GUI_Button * cncl_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	cncl_btn->SetBounds(5,5,105,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	cncl_btn->Show();
	cncl_btn->SetSticky(1,1,0,0);
	cncl_btn->SetDescriptor("Cancel");
	cncl_btn->SetMsg(0,0);
	
	GUI_Pane * holder = new GUI_Pane;
	holder->SetBounds(0,0,210,GUI_GetImageResourceHeight("push_buttons.png") / 3 + 10);
	
	
	okay_btn->SetParent(holder);
	okay_btn->AddListener(this);
	cncl_btn->SetParent(holder);
	cncl_btn->AddListener(this);
	
	holder->SetParent(packer);
	holder->Show();
	holder->SetSticky(1,1,1,0);
	
	packer->PackPane(holder,gui_Pack_Bottom);

	
	packer->PackPane(mScroller,gui_Pack_Center);

	mScroller->PositionHeaderPane(mHeader);

	/*
	
	mScroller = new GUI_ScrollerPane(0,1);
	mScroller->SetParent(this);
	mScroller->Show();
	mScroller->SetSticky(1,1,1,1);

	mTextTable.SetProvider(&mAptTable);
	mTextTable.SetGeometry(&mAptTable);

	mTextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_Table_SelectText),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable.SetTextFieldColors(
				WED_Color_RGBA(wed_TextField_Text),
				WED_Color_RGBA(wed_TextField_Hilite),
				WED_Color_RGBA(wed_TextField_Bkgnd),
				WED_Color_RGBA(wed_TextField_FocusRing));

	mTable = new GUI_Table(true);
	mTable->SetGeometry(&mAptTable);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->SetSticky(1,1,1,1);
	mTable->Show();	
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mTextTable.SetParentTable(mTable);

	mTextTableHeader.SetProvider(&mAptTable);
	mTextTableHeader.SetGeometry(&mAptTable);

	mTextTableHeader.SetImage("header.png");
	mTextTableHeader.SetColors(
			WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

	mHeader = new GUI_Header(true);

	bounds[1] = 0;
	bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;
	mHeader->SetBounds(bounds);
	mHeader->SetGeometry(&mAptTable);
	mHeader->SetHeader(&mTextTableHeader);
	mHeader->SetParent(this);
	mHeader->Show();
	mHeader->SetSticky(1,0,1,1);
	mHeader->SetTable(mTable);


					mTextTableHeader.AddListener(mHeader);		// Header listens to text table to know when to refresh on col resize
					mTextTableHeader.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
					mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
					mAptTable.AddListener(mTable);			// Table listens to actual property content to know when data itself changes

	packer->PackPane(mFilter,gui_Pack_Top);
	packer->PackPane(mHeader,gui_Pack_Top);
	
	int k_reg[4] = { 0, 0, 1, 3 };
	int k_hil[4] = { 0, 1, 1, 3 };
	
	
	GUI_Pane * holder = new GUI_Pane;
	holder->SetBounds(0,0,210,GUI_GetImageResourceHeight("push_buttons.png") / 3 + 10);
	
	
	holder->SetParent(packer);
	holder->Show();
	holder->SetSticky(1,1,1,0);
	
	packer->PackPane(holder,gui_Pack_Bottom);

	
	packer->PackPane(mScroller,gui_Pack_Center);

	mScroller->PositionHeaderPane(mHeader);
	*/
}

WED_GatewayImportDialog::~WED_GatewayImportDialog()
{
}

void WED_GatewayImportDialog::Next()
{
	//if(
	mPhase++;
	return;
}

void WED_GatewayImportDialog::Back()
{

}

void WED_GatewayImportDialog::TimerFired()
{

}

void WED_GatewayImportDialog::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	/*
	switch(inMsg) {
	case kMsg_FilterChanged:	
		mAptTable.SetFilter(mFilter->GetText());
		break;
	case kMsgImport:
		DoIt();
		this->AsyncDestroy();
		break;
	case msg_DocumentDestroyed:
	case kMsgCancel:
		this->AsyncDestroy();
		break;
	}*/
}

void WED_GatewayImportDialog::DownloadJSON(import_dialog_stages stage)
{
	string url = WED_URL_GATEWAY_API;
	
	//Get Certification
	string cert;
	if(!GUI_GetTempResourcePath("gateway.crt", cert))
	{
		DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
		return;
	}

	if(stage == import_dialog_stages::imp_dialog_choose_ICAO && mAirportsGET.size() == 0)
	{
		//Makes the url "https://gatewayapi.x-plane.com:3001/apiv1/airports"
		 url += "airports";

	#if TEST_FROM_SERVER
		//a buffer of chars to be filled, reserve a huge amount of space for it cause we'll need it
		vector<char> rawJSONBuf = vector<char>(GUESS_AIRPORT_SIZE);
	
		//Get it from the server
		curl_http_get_file file = curl_http_get_file(url,&rawJSONBuf,cert);
	
	
		//Lock up everything until the file is finished
		while(file.is_done() == false);

		//create a string from the vector of chars
		string rawJSONString = string(rawJSONBuf->begin(),rawJSONBuf->end());
	#else
		std::ifstream is(LOCAL_JSON);     // open file

		string rawJSONString;
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

		//Our array of all the airports
		Json::Value mAirportsGET(Json::arrayValue);

		//Devrived from the root's "airports" value
		mAirportsGET = root["airports"];

		//loop through the whole array
		for (int i = 0; i < mAirportsGET.size(); i++)
		{
			//Get the current scenery object
			Json::Value tmp(Json::objectValue);
			tmp = mAirportsGET.operator[](i);

			AptInfo_t cur_airport;
			cur_airport.icao = tmp["AirportCode"].asString();
			cur_airport.name = tmp["AirportName"].asString();

			//Add the current scenery object's airport code
			mApts.push_back(cur_airport);

			//Optionally print it out
		}
	}
}
/*
void WED_GatewayImportDialog::MakeICAOTable(int bounds[4])
{
	//The text table chain is done in reverse order starting with the leaf and working back up


	//Text table provider
	mAptTable = WED_AptTable(&mApts);
	mAptTable.SetFilter(mFilter->GetText());
	
	//Text table
	mTextTable.SetProvider(&mAptTable);
	mTextTable.SetGeometry(&mAptTable);
	
	mTextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_Table_SelectText),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable.SetTextFieldColors(
				WED_Color_RGBA(wed_TextField_Text),
				WED_Color_RGBA(wed_TextField_Hilite),
				WED_Color_RGBA(wed_TextField_Bkgnd),
				WED_Color_RGBA(wed_TextField_FocusRing));

	mTable = new GUI_Table(true);
	
	mTable->SetSticky(1,1,1,1);
	mTable->Show();

	mTable->SetGeometry(&mAptTable);
	mTable->SetContent(&mTextTable);

	mScroller = new GUI_ScrollerPane(0,1);
	
	mScroller->SetSticky(1,1,1,1);
	mScroller->Show();

	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	
	mTable->SetParent(mScroller);
	mTextTable.SetParentTable(mTable);
	mScroller->SetParent(this);
	//TextTableHeader has a default constructor
	mTextTableHeader.SetProvider(&mAptTable);
	mTextTableHeader.SetGeometry(&mAptTable);

	mTextTableHeader.SetImage("header.png");
	mTextTableHeader.SetColors(
			WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Header_Text));

	mHeader = new GUI_Header(true);

	bounds[1] = 0;
	bounds[3] = GUI_GetImageResourceHeight("header.png") / 2;
	mHeader->SetBounds(bounds);
	mHeader->SetGeometry(&mAptTable);
	mHeader->SetHeader(&mTextTableHeader);
	mHeader->SetParent(this);
	mHeader->Show();
	mHeader->SetSticky(1,0,1,1);
	mHeader->SetTable(mTable);


	mTextTableHeader.AddListener(mHeader);		// Header listens to text table to know when to refresh on col resize
	mTextTableHeader.AddListener(mTable);		// Table listense to text table header to announce scroll changes (and refresh) on col resize
	mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
	mAptTable.AddListener(mTable);			// Table listens to actual property content to know when data itself changes
}
//-------------------------------------------------------------
*/
int	WED_CanImportFromGateway(IResolver * resolver)
{
	return 1;
}

void WED_DoImportFromGateway(WED_Document * resolver)
{
	new WED_GatewayImportDialog(resolver,gApplication);
	return;

	
}