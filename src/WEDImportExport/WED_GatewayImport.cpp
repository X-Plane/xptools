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

int	WED_CanImportFromGateway(IResolver * resolver)
{
	//Makes the url "https://gatewayapi.x-plane.com:3001/apiv1/airports"
	//string url = WED_URL_GATEWAY_API "airports";

	vector<char> response;

	string cert;

	if(!GUI_GetTempResourcePath("gateway.crt", cert))
	{
		DoUserAlert("This copy of WED is damaged - the certificate for the X-Plane airport gateway is missing.");
		return 0;
	}
	return 1;
	//Contact the server
	/*curl_http_get_file * auth_req = new curl_http_get_file(WED_URL_GATEWAY_API "airports",NULL,NULL,&response,cert);
	
	while(auth_req->is_done() == false)
	{
		int doNothing = 0;
	}

	//If the contact happens, we are able to
	if(auth_req->get_error() == false)
	{
		return 1;
	}
	else
	{
		DoUserAlert("Could not connect to airport server");
		return 0;
	}*/
}

void WED_DoImportFromGateway(IResolver * resolver)
{

}