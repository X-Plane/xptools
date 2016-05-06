#include "WED_FileCache.h"
#include "WED_CacheObject.h"

#include <sstream>
#include <fstream>
#include "FileUtils.h"
#include "GUI_Resources.h"
#include "PlatformUtils.h"
#include "AssertUtils.h"

#include "curl_http.h"
#include "curl/curl.h"
#include <time.h>

#include "RAII_Classes.h"
//The fully qualified path to the file cache folder
static string CACHE_folder;

//Default number of milliseconds
static const int CACHE_COOL_DOWN_START = -1;

//Number of milliseconds to wait before we are allowed to contact server again
static int CACHE_cool_down_time;

//Our vector of CacheObjects
static vector<CACHE_CacheObject* > CACHE_file_cache;

//Fills the file_cache with initial physical files and defaults
void WED_file_cache_init()
{
	static bool cache_initialized = false;
	if(cache_initialized == true)
	{
		return;
	}
	else
	{
		//Get the cache folder path
		{
			char    base[TEMP_FILES_DIR_LEN];
			CACHE_folder = GetTempFilesFolder(base, TEMP_FILES_DIR_LEN) + string("WED\\wed_file_cache");
		}

		CACHE_cool_down_time = 0;

		//Attempt to get the folder, if non-existant make it
		vector<string> files;
		int num_files = FILE_get_directory(CACHE_folder, &files, NULL);
		if(num_files == -1)
		{
			int res = FILE_make_dir_exist(CACHE_folder.c_str());
			if(res != 0)
			{
				AssertPrintf("Could not find or make the file cache, please check if you have sufficient rights to use the folder %s", CACHE_folder.c_str());
			}
		}
		else
		{
			//Add each file in the folder to the cache
			for (int i = 0; i < files.size(); ++i)
			{
				//We always delete during shut WED_file_cacheshutdown()
				CACHE_CacheObject * obj = new CACHE_CacheObject(CACHE_content_type::initially_unknown);
				obj->set_disk_location(CACHE_folder + "\\" + files[i]);
				obj->set_status(CACHE_status::file_available);
				CACHE_file_cache.push_back(obj);
			}
		}
		
		cache_initialized = true;
		return;
	}
}

//returns an error string if there is one
static string HandleNetworkError(curl_http_get_file& mCurl)
{
	int err = mCurl.get_error();
	bool bad_net = mCurl.is_net_fail();

	stringstream ss;
	ss.str() = "";
	if(err <= CURL_LAST)
	{
		string msg = curl_easy_strerror((CURLcode) err);
		ss << "Download failed: " << msg << ". (" << err << ")";
				
		if(bad_net) ss << "(Please check your internet connectivity.)";
	}
	else if(err >= 100)
	{
		//Get the string of error data
		vector<char>    errdat;
		mCurl.get_error_data(errdat);
				
		bool is_ok = !errdat.empty();
		for(vector<char>::iterator i = errdat.begin(); i != errdat.end(); ++i)
		{
			//If the charecter is not printable
			if(!isprint(*i))
			{
				is_ok = false;
				break;
			}
		}

		if(is_ok)
		{
			string errmsg = string(errdat.begin(),errdat.end());
			ss << "Error Code " << err << ": " << errmsg;
		}
		else
		{
			//Couldn't get a useful error message, displaying this instead
			ss << "Download failed due to unknown error: " << err << ".";
		}
	}
	else
	{
		ss << "Download failed due to unknown error: " << err << ".";
	}

	return ss.str();    
}

WED_file_cache_response WED_file_cache_request_file(WED_file_cache_request& req)
{
	//The cache must be initialized!
	DebugAssert(CACHE_folder != "");
	/* 
	-------------------------Method outline------------------------------------
	1. Is it on disk?
		- Set out_path, return file_available
	2. Not on disk, has a cool down even been triggered?
	3. Not on disk, is it in CACHE_file_cache with active cURL_handle?
		- Yes it is. It's currently...
            * Done! If okay, save to disk, set_disk_location
            * Experiencing an error. Set out_error, activate cool_down, return file_error
			* Downloading as normal. return file_downloading
	4. Not on disk, not in cache downloading, not in cool_down.
		- Add file, return file_downloading
	5. Not on disk, not in cache downloading, in cool_down
		- Set out_error to error message, return file_cache_cooling
	---------------------------------------------------------------------------
	*/

	//Search through on disk cache
	for (int i = 0; i < CACHE_file_cache.size(); i++)
	{
		string file_name = FILE_get_file_name(req.in_url);
		if(CACHE_file_cache[i]->get_disk_location().find(file_name) != string::npos)
		{
			WED_file_cache_response res;
			res.out_download_progress = -1;
			res.out_error_type = CACHE_error_type::none;
			res.out_error_human = "";
			res.out_path = CACHE_file_cache[i]->get_disk_location();
			res.out_status = CACHE_status::file_available;
			return res;
		}
	}

	//Check if URL is cooling down, downloading, or having a problem
	for (vector<CACHE_CacheObject* >::iterator itr = CACHE_file_cache.begin();
		 itr != CACHE_file_cache.end();
		 ++itr)
	{
		if((*itr)->get_url() == req.in_url)
		{
			//Has this CacheObject even started a cURL handle?
			if((*itr)->get_RAII_curl_hndl() != NULL)
			{
				curl_http_get_file & hndl = (*itr)->get_RAII_curl_hndl()->get_curl_handle();
				if(hndl.is_done())
				{
					if(hndl.is_ok())
					{
						//Yay! We're done!
						const vector<char>& buf = (*itr)->get_RAII_curl_hndl()->get_dest_buffer();
						string result(buf.begin(),buf.end());

						WED_file_cache_response res;
						res.out_download_progress = hndl.get_progress();
						res.out_error_type = CACHE_error_type::none;
						res.out_error_human = "";
						
						string file_path = CACHE_folder + "\\" + FILE_get_file_name(req.in_url);
						
						//Save file to disk
						RAII_FileHandle f(file_path.c_str(),"w");

						//TODO: What if we can't open the file here?
						//if(f() != NULL)
						//{
						for(string::iterator itr = result.begin(); itr != result.end(); ++itr)
						{
							fprintf(f(),"%c",*itr);
						}
						(*itr)->set_disk_location(file_path);
						
						res.out_path = file_path;
						res.out_status = CACHE_status::file_available;

						//Delete handle
						(*itr)->close_RAII_curl_hndl();

						return res;
					}
					else
					{
						WED_file_cache_response res;
						res.out_download_progress = hndl.get_progress();
						res.out_error_type = CACHE_error_type::none;
						res.out_error_human = HandleNetworkError(hndl);
						res.out_path = "";
						
						//Activate cooldown if were not already counting down
						//TODO:Remember we're changing CACHE_cooldowntime
						if(CACHE_cool_down_time == 0)
						{
							CACHE_cool_down_time = CACHE_COOL_DOWN_START;
							res.out_status = CACHE_status::file_cooling;
							return res;
						}
						else
						{
							res.out_status = CACHE_status::file_error;
							return res;
						}
					}//end if(hndl.is_ok())
				}
				else
				{
					WED_file_cache_response res;
					res.out_download_progress = hndl.get_progress();
					res.out_error_type = CACHE_error_type::none;
					res.out_error_human = "";
					res.out_path = "";
					res.out_status = CACHE_status::file_downloading;
					return res;
				}//end if(hndl.is_done())
			}
		}
	}

	//If it is not on disk, not cooling down, and not in the download_queue, we finally get to download it
	CACHE_CacheObject * obj = new CACHE_CacheObject(CACHE_content_type::initially_unknown);
	obj->create_RAII_curl_hndl(req.in_url, req.in_cert, req.in_buf_reserve_size);
	obj->set_url(req.in_url);
	obj->set_status(CACHE_status::file_downloading);
	CACHE_file_cache.push_back(obj);

	WED_file_cache_response res;
	res.out_download_progress = obj->get_RAII_curl_hndl()->get_curl_handle().get_progress();
	res.out_error_type = CACHE_error_type::none;
	res.out_error_human = "";
	res.out_path = "";
	res.out_status = CACHE_status::file_downloading;
	return res;
}


void WED_file_cache_shutdown()
{
	for(vector<CACHE_CacheObject* >::iterator co = CACHE_file_cache.begin();
		co != CACHE_file_cache.end();
		++co)
	{
		delete *co;
	}
}

#if DEV
#include <ostream>
void WED_file_cache_test()
{
	WED_file_cache_init();
    //Note: You'll have to set up your environment accordingly
    vector<string> test_files;

	//Test finding files already on disk, not available online
	//test_files.push_back("file://" + string(CACHE_folder + "\\ondisk1.txt"));

	//Test finding files on disk, also available online
	test_files.push_back("http://www.example.com/index.html");
	test_files.push_back("https://gatewayapi.x-plane.com:3001/apiv1/airport/ICAO");
	test_files.push_back("https://gatewayapi.x-plane.com:3001/apiv1/secenery/3192");
	//Test finding files not on disk, online
	test_files.push_back("https://gateway.x-plane.com/airport_metadata.csv");

	//test finding files not on disk, not online
	test_files.push_back("http://www.x-plane.com/thisisnotreal.txt");

	//Get Certification
	string cert = "";
	//int res = GUI_GetTempResourcePath("gateway.crt", cert);
	
	int i = 0;
	while(i < test_files.size())
	{
		CACHE_status status = CACHE_status::file_not_started;

        int max_error = 100;
        int error_count = 0;
		//while(status != CACHE_status::file_available && error_count < max_error)
		{
			string in_path = test_files.at(i);
			string out_path;
			string error;
//			status = WED_file_cache_request_file(in_path, cert, out_path, error);
			if(status == CACHE_status::file_error || status == CACHE_status::file_cooling)
			{
				++error_count;
			}

			if(error_count % 20 == 0 && status > 2)
			{
				printf("in_path %s, status: %d out_path: %s error_path: %s errors: %d \n", in_path.c_str(), status, out_path.c_str(), error.c_str(), error_count);
			}
		}
		++i;
	}
	//WED_file_cache_shutdown();
}
#endif
