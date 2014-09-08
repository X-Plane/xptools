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

#include "GUI_FormWindow.h"
#include "GUI_Timer.h"
#include "WED_FilterBar.h"
#include "GUI_Table.h"
#include "GUI_TextTable.h"
#include "GUI_Application.h"

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
imp_dialog_start,
imp_dialog_get_ICAO,
imp_dialog_get_versions,
imp_dialog_finish
};


//Our private class for the import dialog
class WED_GatewayImportDialog : public GUI_FormWindow, public GUI_Timer
{
public:
	WED_GatewayImportDialog(IResolver * resolver);
	
	/*At step get_ICAO, submit moves onto scenery
	At step get_scenery_versions, submit import operation 
	and start the process of importing the blob into WED*/
	virtual void Submit();

	//At set get_ICAO, backs out of the whole process
	//At step get_scenery_versions, takes one back to get_ICAO
	virtual void Cancel();
	
	//For checking on the curl_http download and displaying a progress bar
	virtual	void TimerFired(void);

private:
	import_dialog_stages mPhase;//Our simple stage counter for our simple fsm

	IResolver *			 mResolver;
	
	//Our curl handle we'll be using to get the json files, note the s
	curl_http_get_file * mCurl;
	
	//The large airport json file, obtained after imp_dialog_get_start
	Json::Value				mAirportsGET;

	//The smaller scenery json file, obtained after imp_dialog_get_versions
	Json::Value				mSceneryGET;

	//GUI dialog box memebers
	WED_FilterBar *			mFilter;

	GUI_ScrollerPane *		mScroller;
	GUI_Table *				mTable;
	GUI_Header *			mHeader;

	GUI_TextTable			mTextTable;
	GUI_TextTableHeader		mTextTableHeader;
};

//--Implemation of WED_GateWayImportDialog class---------------
WED_GatewayImportDialog::WED_GatewayImportDialog(IResolver * resolver) :
	//Form Window
	GUI_FormWindow(gApplication, "Import Scenery from Gateway", 500, 400),
	mTextTable(this,100,0),
	mResolver(resolver),
	mPhase(imp_dialog_start),
	mCurl(NULL)
{
	/*mFilter = new WED_FilterBar(this,kMsg_FilterChanged,0,"Filter:","",NULL,false);
	mFilter->Show();
	mFilter->SetSticky(1,0,1,1);
	mFilter->SetParent(packer);
	mFilter->AddListener(this);*/

	this->Show();
}

void WED_GatewayImportDialog::Submit()
{
	return;
}

void WED_GatewayImportDialog::Cancel()
{

}

void WED_GatewayImportDialog::TimerFired()
{
}
//-------------------------------------------------------------

int	WED_CanImportFromGateway(IResolver * resolver)
{
	return 1;
}

void WED_DoImportFromGateway(IResolver * resolver)
{
	new WED_GatewayImportDialog(resolver);
	return;

	//Makes the url "https://gatewayapi.x-plane.com:3001/apiv1/airports"
	string url = WED_URL_GATEWAY_API "airports";

	//Get Certification
	string cert;
	if(!GUI_GetTempResourcePath("gateway.crt", cert))
	{
		DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
		return;
	}
	
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
		return;
	}

	//Our array of all the airports
	Json::Value airportICAOs(Json::arrayValue);
	//Devrived from the root's "airports" value
	airportICAOs = root["airports"];
	
	//our collection of ICAO's to show the user
	vector<string> outICAOs;

	//loop through the whole array
	for (int i = 0; i < airportICAOs.size(); i++)
	{
		//Get the current scenery object
		Json::Value tmp(Json::objectValue);
		tmp = airportICAOs.operator[](i);

		//Add the current scenery object's airport code
		outICAOs.push_back(tmp["AirportCode"].asString());

		//Optionally print it out
		cout << outICAOs.back() << endl;
	}
}