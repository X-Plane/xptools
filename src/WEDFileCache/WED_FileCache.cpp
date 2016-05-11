#include "WED_FileCache.h"
#include "CACHE_CacheObject.h"

#if DEV
#include <iostream>
#define SAVE_TO_DISK 1
#else
#define SAVE_TO_DISK 1
#endif

#include <sstream>
#include <fstream>
#include "FileUtils.h"
#include "RAII_Classes.h"

#include "PlatformUtils.h"
#include "AssertUtils.h"

#include "curl_http.h"
#include "curl/curl.h"
#include <time.h>

//--WED_file_cache_request---------------------------------------------------
WED_file_cache_request::WED_file_cache_request()
	: in_buf_reserve_size(0),
	  in_cert(""),
	  in_content_type(CACHE_content_type::initially_unknown),
	  in_url("")
{
}

WED_file_cache_request::WED_file_cache_request(int buf_reserve_size, string cert, CACHE_content_type content_type, string url)
	: in_buf_reserve_size(buf_reserve_size),
	  in_cert(cert),
	  in_content_type(content_type),
	  in_url(url)
{
}

bool WED_file_cache_request::operator==(const WED_file_cache_request& rhs) const
{
	bool result = true;
	result &= this->in_buf_reserve_size == rhs.in_buf_reserve_size ? true : false;
	result &= this->in_cert             == rhs.in_cert             ? true : false;
	result &= this->in_content_type     == rhs.in_content_type     ? true : false;
	result &= this->in_url              == rhs.in_url              ? true : false;

	return result;
}

bool WED_file_cache_request::operator!=(const WED_file_cache_request& rhs) const
{
	return !(*this == rhs);
}
//---------------------------------------------------------------------------//

//--WED_file_cache_response--------------------------------------------------
WED_file_cache_response::WED_file_cache_response(float download_progress, string error_human, CACHE_error_type error_type, string path, CACHE_status status)
		: out_download_progress(download_progress),
		  out_error_type(error_type),
		  out_error_human(error_human),
		  out_path(path),
		  out_status(status)
{
}

bool WED_file_cache_response::operator==(const WED_file_cache_response& rhs) const
{
		bool result = true;
		result &= this->out_download_progress == rhs.out_download_progress ? true : false;
		result &= this->out_error_human == rhs.out_error_human             ? true : false;
		result &= this->out_error_type  == rhs.out_error_type              ? true : false;
		result &= this->out_path        == rhs.out_path                    ? true : false;
		result &= this->out_status      == rhs.out_status                  ? true : false;
		
		return result;
	}

bool WED_file_cache_response::operator!=(const WED_file_cache_response& rhs) const
{
	return !(*this == rhs);
}
//---------------------------------------------------------------------------//

//--WED_FileCache------------------------------------------------------------
//The fully qualified path to the file cache folder
static string CACHE_folder;

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
				//We always delete during shut WED_file_cache_shutdown()
				CACHE_file_cache.push_back(new CACHE_CacheObject());
				CACHE_file_cache.back()->set_disk_location(CACHE_folder + "\\" + files[i]);
			}
		}
		
		cache_initialized = true;
		return;
	}
}

//returns an error string if there is one
static void interpret_error(curl_http_get_file& mCurl, string& out_error_human, CACHE_error_type& out_error_type)
{
	out_error_human = "";
	out_error_type = CACHE_error_type::unknown;

	int err = mCurl.get_error();
	bool bad_net = mCurl.is_net_fail();

	stringstream ss;
	ss.str() = "";
	if(err <= CURL_LAST)
	{
		string msg = curl_easy_strerror((CURLcode) err);
		ss << "Download failed: " << msg << ". (" << err << ")";
				
		if(bad_net)
		{
			ss << "(Please check your internet connectivity.)";
			out_error_type = CACHE_error_type::client_side;
		}
		else
		{
			out_error_type = CACHE_error_type::server_side;
		}
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
			out_error_type = CACHE_error_type::server_side;
		}
		else
		{
			//Couldn't get a useful error message, displaying this instead
			ss << "Download failed due to unknown error: " << err << ".";
			out_error_type = CACHE_error_type::unknown;
		}
	}
	else
	{
		ss << "Download failed due to unknown error: " << err << ".";
		out_error_type = CACHE_error_type::unknown;
	}

	out_error_human = ss.str();
}

static WED_file_cache_response start_new_cache_object(WED_file_cache_request req)
{
	CACHE_file_cache.push_back(new CACHE_CacheObject());
	CACHE_CacheObject& co = *CACHE_file_cache.back();
	
	co.create_RAII_curl_hndl(req.in_url, req.in_cert, req.in_buf_reserve_size);
	
	return WED_file_cache_response(co.get_RAII_curl_hndl()->get_curl_handle().get_progress(),
								   co.get_last_url(),
								   co.get_last_error_type(),
								   co.get_disk_location(),
								   CACHE_status::file_downloading);
}

static void remove_cache_object(vector<CACHE_CacheObject* >::iterator itr)
{
	delete *itr;
	CACHE_file_cache.erase(itr);
}

WED_file_cache_response WED_file_cache_request_file(WED_file_cache_request& req)
{
	//The cache must be initialized!
	DebugAssert(CACHE_folder != "");
	
	/* 
	-------------------------Method outline------------------------------------
	
	We ask is the URL....
	0. Search through cache searching for an existing existing active cache objects also on disk
	1. Not in CACHE_file_cache?
		- Add new cache object, status is file_downloading
	2. In CACHE_file_cache with active cURL_handle?
		- cURL done?
			* If okay, attempt to save to disk
				o if save is success, set_disk_location to path, status is file_availible.
				o else set_disk_location to "", out_error_human to "Bad disk write", set last_error_type to disk_write, status is file_error
			* Else, we're experiencing an error.
			  get out_error_human message and last_error_type, activate cool_down, close hdnl, status is file_error
		- cURL downloading as normal.
			* status is file_downloading
	3. In CACHE_file_cache without active cURL_handle?
		- CO on cooling?
			* Set out_error
		- CO on disk?
			* TODO: If not stale, set out_path, return file_available
			* Else delete old CO, retry with new CO start a new handle
		- Time to try it again (or start a new one)

	Note: out_error_human is for client, last_error_type is for cool down
	---------------------------------------------------------------------------
	*/
	
	vector<CACHE_CacheObject* >::iterator itr = CACHE_file_cache.begin();
	for ( ; itr != CACHE_file_cache.end(); ++itr)
	{
		if(FILE_get_file_name((**itr).get_disk_location()) == FILE_get_file_name(req.in_url))
		{
			break;
		}
		else if ((**itr).get_last_url() == req.in_url)
		{
			break;
		}
	}

	if(itr == CACHE_file_cache.end()) //1. Not in CACHE_file_cache?
	{
		//If it is not on disk, not cooling down, and not in the download_queue, we finally get to download it
		return start_new_cache_object(req);
	}
	
	CACHE_CacheObject & co = **itr;

	//2. In CACHE_file_cache with active cURL_handle?
	if((**itr).get_RAII_curl_hndl() != NULL)
	{
		curl_http_get_file & hndl = co.get_RAII_curl_hndl()->get_curl_handle();
		
		if(hndl.is_done())
		{
			if(hndl.is_ok())
			{
				//Yay! We're done!
				WED_file_cache_response res(hndl.get_progress(),
											"",
											CACHE_error_type::none,
											"",
											CACHE_status::file_available);

#if SAVE_TO_DISK //Testing cooldown
				res.out_path = CACHE_folder + "\\" + FILE_get_file_name(req.in_url);
				RAII_FileHandle f(res.out_path.c_str(),"w");

				//TODO: content_type != CACHE_content_type::no_cache)?
				//What if we can't open the file here?
				const vector<char>& buf = co.get_RAII_curl_hndl()->get_dest_buffer();
				if(f() != NULL)
				{
					for(vector<char>::const_iterator itr = buf.begin(); itr != buf.end(); ++itr)
					{
						fprintf(f(),"%c",*itr);
					}

					DebugAssert(co.get_last_error_type() == CACHE_error_type::none);
				}
				else
				{
					res.out_error_human = res.out_path + " could not be saved, check if the folder or file is in use or if you have sufficient privaleges";
					co.set_last_error_type(CACHE_error_type::disk_write);
				}
#endif
				res.out_error_type = co.get_last_error_type();
				co.set_disk_location(res.out_path);
				co.close_RAII_curl_hndl();

				return res;
			}
			else
			{
				WED_file_cache_response res(hndl.get_progress(), "", CACHE_error_type::unknown, "", CACHE_status::file_error);
				interpret_error(hndl, res.out_error_human, res.out_error_type);

				co.trigger_cool_down();

				co.set_last_error_type(res.out_error_type);

				co.close_RAII_curl_hndl();
				return res;
			}//end if(hndl.is_ok())
		}
		else
		{
			DebugAssert(co.get_response_from_object_state(CACHE_status::file_downloading) == WED_file_cache_response(hndl.get_progress(), "", CACHE_error_type::none, "", CACHE_status::file_downloading));
			return co.get_response_from_object_state(CACHE_status::file_downloading);
		}//end if(hndl.is_done())
	}//end if((**itr).get_RAII_curl_hndl() != NULL)
	else
	{
		int seconds_left = co.cool_down_seconds_left();
		if(seconds_left > 0)
		{
			char buf[128] = { 0 };
			return WED_file_cache_response(-1, "Cache cooling after failed network attempt, please wait: " + string(itoa(seconds_left, buf, 10)) + " seconds...", CACHE_error_type::none, "", CACHE_status::file_cooling); //TODO: Is this really something to show the user?
		}
		else if(FILE_exists((*itr)->get_disk_location().c_str()) == true) //Check if file was deleted between requests
		{
			//TODO: Is it stale?
			DebugAssert((*itr)->get_disk_location() != "");
			return WED_file_cache_response(100, "", CACHE_error_type::none, (*itr)->get_disk_location(), CACHE_status::file_available);
		}
		else
		{
			remove_cache_object(itr);
			return start_new_cache_object(req);
		}
	}
}

void WED_file_cache_shutdown()
{
	for(vector<CACHE_CacheObject* >::iterator co = CACHE_file_cache.begin();
		co != CACHE_file_cache.end();
		++co)
	{
		delete *co;
	}
	CACHE_file_cache.clear();
}

#if DEV && 0
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
		CACHE_status status = CACHE_status::file_downloading;

        int max_error = 100;
        int error_count = 0;
		//while(status != CACHE_status::file_available && error_count < max_error)
		{
			string in_path = test_files.at(i);
			string out_path;
			string error;
//			status = WED_file_cache_request_file(in_path, cert, out_path, error);
			if(status == CACHE_status::file_error)
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
//---------------------------------------------------------------------------//
#endif
