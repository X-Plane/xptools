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

/*
	WED_FileCache - THEORY OF OPERATION

	The file cache is a black box: clients call WED_file_cache_request_file repeatedly on a timer, passing in a request struct a response struct.
	The request contains information about the URL to connect to and other data.
	The response contains 0 to some error data and the file's disk path if and when on disk, after downloading at some point in life.

	If the file is on disk and not too old, a download is not started.
	
	Clients should use the error information to decide whether or not to try again.
	After an error has occured a given URL will be placed on a cool down timer, preventing a client from repeatedly pinging the server.
	The cooldown length is decided by the content_type given in the request.
	
	TODO:
	- The bit about "We keep returning the same error"
	- File catagories, lifespans, and per catagory cool down timers
	- Thourough testing, last done as of 48245211b2a977d
	- Client pfrefix folder "place all my cached objects in CACHE_FOLDER/myfolder/
	- Implement no-cache?
	- Re-write test method
	- Decide on cool down lengths
	- Integrate with WED_GatewayImport
*/

enum CACHE_status
{
	file_available,   //File available on disk
	file_cooling,     //File currently in cool down mode after error
	file_downloading, //File is currently download from the net
	file_error        //File has had some kind of error, see CACHE_error_type
	
};

//What type of cache error
enum CACHE_error_type
{
	none,        //No error
	client_side, //Error determined to be client's, see curl_http.h's UTL_http_is_error_bad_net
	disk_write,  //Error saving file to disk
	server_side, //Error determined to be server's, likely HTTP 300 - 500's
	unknown,     //Error origin could not be determined, probably WED's fault
	
};

//Content_type is used to determine the life span of a cached file
enum CACHE_content_type
{
	no_cache,   //Should not be cached, TODO: Why is this needed?
	temporary,  //Refreshed multiple times a session
	content,    //Refreshed once per session or every few uses
	stationary, //Refreshed once every few weeks
	initially_unknown //NOT FOR CLIENTS! Only used internally during cache intialization
};

struct WED_file_cache_request
{
	WED_file_cache_request();
	WED_file_cache_request(int buf_reserve_size, string cert, CACHE_content_type content_type, string url);

	//Our guess as to how big our download (in number of chars)
	int in_buf_reserve_size;
	
	//Our security certification
	string in_cert;
	
	//Content type we are requesting, cached inside CACHE_CacheObject
	CACHE_content_type in_content_type;

	//The URL to request from, cached inside CACHE_CacheObject
	string in_url;

	//The folder prefix to place this cached file in, usually the class name
	//string in_folder_prefix

	bool operator==(const WED_file_cache_request& rhs) const;
	bool operator!=(const WED_file_cache_request& rhs) const;
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

//Initialize the file cache, called once at the start of the program
void WED_file_cache_init();

//Attempts give client a file path for file, downloading said file if need be. Feedback on progress and ability is given in the form status, error codes, and status updates.
//This is intended to be called a timer until a client gets their file or sufficient indication they should stop trying.
WED_file_cache_response WED_file_cache_request_file(WED_file_cache_request& req);

//Blocks until all previous cURL handles are finished or are forcibly stopped. Called once at the end of the program.
void WED_file_cache_shutdown();

#if DEV && 0 //TODO: Rewrite test
void WED_file_cache_test();
#endif

#endif
