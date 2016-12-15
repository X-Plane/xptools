/* 
 * Copyright (c) 2016, Laminar Research.
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

#ifndef WED_FILECACHE_H
#define WED_FILECACHE_H

#include "CACHE_DomainPolicy.h"

/*
	WED_FileCache - THEORY OF OPERATION

	The file cache is a black box: clients call WED_file_cache_request_file repeatedly on a timer, passing in a request struct, receiving a response struct.
	
	- The request contains information about the URL to connect to and other data
	- The response contains error data, download progress, and (potentially) a path where the file successfully downloaded
		* After an error a url is placed on a cool down timer, preventing WED DDOS'ing the server
		* Clients can use the error information to decide whether or not to try again
	- Cached files that are too old are re-downloaded
	- A cache domain policy determines maximum age and minimum cool down periods
*/

enum CACHE_status
{
	cache_status_available,   //File available on disk
	cache_status_cooling,     //File currently in cool down mode after error
	cache_status_downloading, //File is currently download from the net
	cache_status_error        //File has had some kind of error, see CACHE_error_type
};

//What type of cache error
enum CACHE_error_type
{
    cache_error_type_none,        //No error
    cache_error_type_client_side, //Error de-termined to be client's, see curl_http.h's UTL_http_is_error_bad_net
    cache_error_type_disk_write,  //Error saving file to disk
    cache_error_type_server_side, //Error determined to be server's, likely HTTP 300 - 500's
    cache_error_type_unknown,     //Error origin could not be determined, probably WED's fault
};

struct WED_file_cache_request
{
	WED_file_cache_request();

	WED_file_cache_request(const string& cert, CACHE_domain domain, const string& folder_prefix, const string& url);

	//Our security certification
	string in_cert;
	
	//Domain policy for the file, stores information on how files should be downloaded and kept
	CACHE_domain in_domain;

	//A folder prefix to place this cached file in, no leading or trailing slash
	string in_folder_prefix;

	//The URL to request from, cached inside CACHE_CacheObject
	string in_url;
};

struct WED_file_cache_response
{
	WED_file_cache_response(float download_progress, string error_human, CACHE_error_type error_type, string path, CACHE_status status);
	
	//From a range from -1.0 (download not started), to 100.0
	float out_download_progress;

	//Human readable error string
	string out_error_human;

	//The type of error we just occured (who is to blame.) cached inside CACHE_CacheObject
	CACHE_error_type out_error_type;

	//Path to load downloaded file from, cached inside CACHE_CacheObject and file existing on disk
	string out_path;

	//Status of the cache
	CACHE_status out_status;

	bool operator==(const WED_file_cache_response& rhs) const;
	bool operator!=(const WED_file_cache_response& rhs) const;
};

class CACHE_FileCacheInitializer;

//Initialize the file cache, called once at the start of the program
void WED_file_cache_init();

//Attempts give client a file path for file, downloading said file if need be. Feedback on progress and ability is given in the form status, error codes, and status updates.
//This is intended to be called a timer until a client gets their file or sufficient indication they should stop trying.
WED_file_cache_response WED_file_cache_request_file(const WED_file_cache_request& req);

//Checks if the file is in cache without starting any downloads. in_cert member of WED_file_cache_request is ignored
//returns the file path if it exists on disk, "" if not
string WED_file_cache_file_in_cache(const WED_file_cache_request& req);

string WED_file_cache_url_to_cache_path(const WED_file_cache_request& req);
//Blocks until all previous cURL handles are finished or are forcibly stopped. Called once at the end of the program.
void WED_file_cache_shutdown();

#endif
