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
	WED_FileCache theory of operation

	Here we ask can we retrieve this from the OS first?
	
	This class is designed as a black box: clients only need file status, and where to continue IO when finished.
	
	An example use: A UI box begins a download. A timer asks the cache "Where is my file "example.com/my_data.csv"?"
	Responses 
	We tell the client "Its at X on disk", In theory a timer in some UI component will interact with the cache on a timer, asking for its file path and being told would call the cache, check the error, and possibly advance the state machine.

	TODO:
	1. The bit about "We keep returning the same error"
	2. Cooldown system in place
	3. File catagories, lifespans, and per catagory cool down timers
	4. Thourough testing, done as 48245211b2a977d
	5. Better theory of operation description
	6. Client pfrefix folder "place all my cached objects in CACHE_FOLDER/myfolder/
*/

enum CACHE_status
{
	file_available,    //File available on disk
	file_not_started,  //File not on disk or downloading
	file_downloading,  //File is currently download from the net
	file_error,        //File has had some kind of error
	file_cooling       //File is current cooling down after error TODO: Does a client need to know about cooling mechanism?
};

//What type of cache error
enum CACHE_error_type
{
	none,        //No error
	server_side, //Error determined to be server's, likely HTTP 300 - 500's
	client_side, //Error determined to be client's, TODO: currently all cURL errors are blindly taken as client
	unknown,     //Error origin could not be determined, probably WED's fault
};

//Content_type is used to determine the life span of a cached file
enum CACHE_content_type
{
	no_cache,   //Should not be cached
	temporary,  //Refreshed multiple times a session
	content,    //Refreshed once per session or every few uses
	stationary, //Refreshed once every few weeks
	initially_unknown     //NOT FOR CLIENTS! Only used internally during cache intialization
};

//Initialize the file cache, called once at the start of the program
void WED_file_cache_init();

struct WED_file_cache_request
{
	//Our guess as to how big our download (in number of chars)
	int    in_buf_reserve_size;
	
	//Our security certification
	string in_cert;
	
	//Content type we are requesting
	CACHE_content_type in_content_type;

	//The URL to request
	string in_url;

	//The folder prefix to place this cached file in, usually the class name
	//string in_folder_prefix
};

struct WED_file_cache_response
{
	//From a range from -1.0 (download not started), to 100.0
	float out_download_progress;

	//The type of error we just occured (who is to blame)
	CACHE_error_type out_error_type;

	//Human readable error string
	string out_error_human;

	//Path to load downloaded file from
	string out_path;

	//Status of the cache
	CACHE_status out_status;
};

//Attempts give client a file path for file, downloading said file if need be. Feedback on progress and ability is given in the form status, error codes, and status updates.
//This is intended to be called a timer until a client gets their file or sufficient indication they should stop trying.
WED_file_cache_response WED_file_cache_request_file(WED_file_cache_request& req);

//Blocks until all previous cURL handles are finished or are forcibly stopped. Called once at the end of the program.
void WED_file_cache_shutdown();

#if DEV
void WED_file_cache_test();
#endif

#endif
