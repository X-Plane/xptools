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

class CACHE_CacheObject;

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

	string in_cert;           // Our security certification
	CACHE_domain in_domain;   // Domain policy for the file, stores information on how files should be downloaded and kept
	string in_folder_prefix;  // A folder prefix to place this cached file in, no leading or trailing slash
	string in_url;            // The URL to request from, cached inside CACHE_CacheObject
};

ostream& operator << (ostream& os, const WED_file_cache_request& rhs);

struct WED_file_cache_response
{
	WED_file_cache_response(float download_progress, string error_human, CACHE_error_type error_type, string path, CACHE_status status);
	
	float out_download_progress;	// From a range from -1.0 (download not started), to 100.0
	string out_error_human;	      // Human readable error string
	CACHE_error_type out_error_type;// The type of error we just occured (who is to blame.) cached inside CACHE_CacheObject
	string out_path;	            // Path to load downloaded file from, cached inside CACHE_CacheObject and file existing on disk
	CACHE_status out_status;      // Status of the cache

	bool operator==(const WED_file_cache_response& rhs) const;
	bool operator!=(const WED_file_cache_response& rhs) const;
};

class WED_FileCache
{
	public:
							WED_FileCache(void) {};
							~WED_FileCache(void); // WED_file_cache_shutdown()
		void				init(void);           // WED_file_cache_init()

		WED_file_cache_response	request_file(const WED_file_cache_request& req);
		string			file_in_cache(const WED_file_cache_request& req);
		string			url_to_cache_path(const WED_file_cache_request& req);

	private:

		vector<string>	get_files_available(CACHE_domain domain, string folder_prefix);
		WED_file_cache_response Request_file(const WED_file_cache_request& req);
		WED_file_cache_response start_new_cache_object(WED_file_cache_request req);
		void 				remove_cache_object(vector<CACHE_CacheObject* >::iterator itr);

		const string 	CACHE_INFO_FILE_EXT = ".cache_object_info";
		string 			CACHE_folder;	                  // The fully qualified path to the file cache folder
		vector<CACHE_CacheObject* > CACHE_file_cache;   // Our vector of CacheObjects
};

extern WED_FileCache gFileCache;

#endif
