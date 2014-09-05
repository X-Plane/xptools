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

//Decides where you want to test getting the JSON info from
//Recomend using test from local for speed
#define TEST_FROM_SERVER 0
#define LOCAL_JSON "C:\\airports.txt"
#define GUESS_AIRPORTS_SIZE 34151

#if !TEST_FROM_SERVER
#include <fstream>
#endif
int	WED_CanImportFromGateway(IResolver * resolver)
{
	return 1;
}

void WED_DoImportFromGateway(IResolver * resolver)
{
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