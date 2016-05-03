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
*/

enum WED_file_cache_status {
	file_available,    //File available on disk
	file_not_started,  //File not on disk or downloading
	file_downloading,  //File is currently download from the net
	file_error,        //File has had some kind of error
	file_cache_cooling //File cache is current cooling down after error
};

//What type of cache error
enum WED_file_cache_error {
	server_side, //Error determined to be server's, likely HTTP 300 - 500's
	client_side, //Error determined to be client's, TODO: currently all cURL errors are blindly taken as client
	unknown      //Error origin could not be determined, probably WED's fault
};

//Content_type is used to determine the life span of a cached file
enum WED_file_cache_content_type {
		no_cache,  //Should not be cached
		temporary, //Refreshed multiple times a session
		content,   //Refreshed once per session or every few uses
		stationary //Refreshed once every few weeks
};

//Initialize the file cache, called once at the start of the program
void WED_file_cache_init();

//Attempts to get a file path for client to use, if a file is not present the cache will attempt to download it
//providing feedback in the form of status, error codes, and status updates.
//This method is meant to be called on a timer until a client gets what they want or enough errors to make them stop trying.
WED_file_cache_status WED_get_file_from_cache(
            const string& in_uri,
			const string& in_cert,
            string& out_path,
            string& out_error);//- We get file_error if the URL returned some kind of error code (E.g 404).  Additional calls to the file cache should KEEP returning file_error In the case of an error, out_error can contain some kind of human-readable error message, so we can tell a 404 from a timeout.

//Blocks until all previous cURL handles are finished or are forcibly stopped. Called once at the end of the program.
void WED_file_cache_shutdown();

#if DEV
void WED_file_cache_test();
#endif

#endif
