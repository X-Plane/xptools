#include "WED_FileCache.h"
#include "CACHE_CacheObject.h"

#if DEV
#include <iostream>
#define SAVE_TO_DISK 1
#else
#define SAVE_TO_DISK 1
#endif

#include <algorithm>

#include <sstream>
#include <fstream>
#include "FileUtils.h"
#include "RAII_Classes.h"
#include "json/json.h"

#include "PlatformUtils.h"
#include "AssertUtils.h"

#include "curl_http.h"
#include "curl/curl.h"
#include <time.h>

#include <sstream>

string itoa_cpp98(int i) // C++98 doesn't have itoa(), so we have to do this... :(
{
	stringstream out;
	out << i;
	return out.str();
}


//--WED_file_cache_request---------------------------------------------------
WED_file_cache_request::WED_file_cache_request()
	: in_cert(""),
	  in_domain(cache_domain_none),
	  in_folder_prefix(""),
	  in_url("")
{
}
//---------------------------------------------------------------------------//

//--WED_file_cache_response--------------------------------------------------
WED_file_cache_response::WED_file_cache_response(float download_progress, string error_human, CACHE_error_type error_type, string path, CACHE_status status)
		: out_download_progress(download_progress),
		  out_error_human(error_human),
		  out_error_type(error_type),
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

static const string CACHE_INFO_FILE_EXT = ".cache_object_info";

//The fully qualified path to the file cache folder
static string CACHE_folder;

//Our vector of CacheObjects
static vector<CACHE_CacheObject* > CACHE_file_cache;

//This tiny wrapper class lets CACHE_CacheObject engage in friendship with this.
//That way when we add another field to save in the .cache_object_info file,
//we don't need to update CACHE_CacheObject's constructor everywhere
class CACHE_FileCacheInitializer
{
public:
	void init();
};

void CACHE_FileCacheInitializer::init()
{
	//Get the cache folder path
	{
		char    base[TEMP_FILES_DIR_LEN];
		const char * cache_folder = GetCacheFolder(base, TEMP_FILES_DIR_LEN);
		if(cache_folder == NULL)
		{
			AssertPrintf("Could not get OS cache folder");
		}

		CACHE_folder = string(cache_folder) + string(DIR_STR "wed_file_cache");
	}

	vector<string> files;
	vector<string> dirs;
	//Attempt to get the folder, if non-existant make it
	int num_files = FILE_get_directory_recursive(CACHE_folder, files, dirs);

	time_t t = time(NULL);
	sort(files.begin(), files.end(), less<string>());

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
		//Where pair is <file,file.cache_object_info>
		vector<pair<string, string> > paired_files;

		int i = 0;
		while(i < files.size())
		{
			//Files must come in pairs
			//[i] = file1.txt
			//[i+1] = file1.txt.cache_object_info
			//
			//If one is missing we delete both

			//If we only have 1 file remaining to inspect
			if((i + 1) >= files.size())
			{
				FILE_delete_file(files[i].c_str(), false);
				break;
			}

			//file_name_A should be "whatever" or "whatever.txt"
			//file_name_B should always be file_name_A.append(CACHE_INFO_FILE_EXT)
			string file_name_A = FILE_get_file_name(files[i]);
			string actual_file_name_B = FILE_get_file_name(files[i+1]);
			string required_file_name_B(file_name_A);
			required_file_name_B.append(CACHE_INFO_FILE_EXT);

			if(!(actual_file_name_B == required_file_name_B))
			{
				FILE_delete_file(files[i].c_str(), false);
				i += 1;
				continue;
			}
			else
			{
				paired_files.push_back(make_pair<string,string>(files[i], files[i+1]));
				i += 2;
			}
		}

		//Add each file in the folder to the cache
		for (int i = 0; i < paired_files.size(); ++i)
		{
			//We always delete during shut WED_file_cache_shutdown()
			CACHE_file_cache.push_back(new CACHE_CacheObject());

			bool info_read_success = false;

			string content;
			int file_content_read = FILE_read_file_to_string(paired_files[i].second, content);

			if(file_content_read == 0)
			{
				Json::Value root;
				Json::Reader reader;
				
				bool json_parse_result = reader.parse(content, root);

				if(json_parse_result == true)
				{
					CACHE_file_cache.back()->m_last_time_modified = root["last_time_modified"].asInt();
					CACHE_file_cache.back()->m_domain = static_cast<CACHE_domain>(root["domain"].asInt());
					CACHE_file_cache.back()->set_disk_location(paired_files[i].first);
					info_read_success = true;
				}
			}

			if(info_read_success == false)
			{
				delete CACHE_file_cache.back();

				FILE_delete_file(paired_files[i].first.c_str(), false);
				FILE_delete_file(paired_files[i+1].second.c_str(), false);
				continue;
			}
		}
	}
	return;
}

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
		CACHE_FileCacheInitializer initializer;
		initializer.init();
		cache_initialized = true;
	}
}

//returns an error string if there is one
static void interpret_error(curl_http_get_file& mCurl, string& out_error_human, CACHE_error_type& out_error_type)
{
	out_error_human = "";
	out_error_type = cache_error_type_unknown;

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
			out_error_type = cache_error_type_client_side;
		}
		else
		{
			out_error_type = cache_error_type_server_side;
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
			out_error_type = cache_error_type_server_side;
		}
		else
		{
			//Couldn't get a useful error message, displaying this instead
			ss << "Download failed due to unknown error: " << err << ".";
			out_error_type = cache_error_type_unknown;
		}
	}
	else
	{
		ss << "Download failed due to unknown error: " << err << ".";
		out_error_type = cache_error_type_unknown;
	}

	out_error_human = ss.str();
}

static WED_file_cache_response start_new_cache_object(WED_file_cache_request req)
{
	CACHE_file_cache.push_back(new CACHE_CacheObject());
	CACHE_CacheObject& co = *CACHE_file_cache.back();
	
	co.create_RAII_curl_hndl(req.in_url, req.in_cert, GetDomainPolicy(req.in_domain).cache_domain_pol_buffer_reserve_size);
	
	return WED_file_cache_response(co.get_RAII_curl_hndl()->get_curl_handle().get_progress(),
								   "",
								   co.get_last_error_type(),
								   co.get_disk_location(),
								   cache_status_downloading);
}

static void remove_cache_object(vector<CACHE_CacheObject* >::iterator itr)
{
	delete *itr;
	CACHE_file_cache.erase(itr);
}

WED_file_cache_response WED_file_cache_request_file(const WED_file_cache_request& req)
{
	//The cache must be initialized!
	DebugAssert(CACHE_folder != "");

	/*
	-------------------------Method outline------------------------------------
	
	We ask is the URL....
	0. Search through cache searching for an matching cache object (on disk or not)
	1. Not in CACHE_file_cache?
		- Add new cache object, status is file_downloading
	2. In CACHE_file_cache with active cURL_handle?
		- cURL done?
			* If okay, attempt to save to disk
				o if save is success, set_disk_location to path, status is file_availible.
				o else set_disk_location to "", out_error_human to "Bad disk write", set last_error_type to disk_write, status is file_error
			* Else, we're experiencing an error.
				o get out_error_human message and last_error, activate cool_down, close hdnl, status is file_error
		- cURL downloading as normal.
			* status is file_downloading
	3. In CACHE_file_cache without active cURL_handle?
		- CO on cooling?
			* Set out_error_human/type, status is file_error
		- CO on disk?
			* If not stale, set out_path, return file_available
			* Else delete old CO, retry with new CO start a new handle

	Note: out_error_human is for client, last_error is for cool down
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
											cache_error_type_none,
											"",
											cache_status_available);

#if SAVE_TO_DISK //Testing cooldown
				/*
				1. Create if prefixed dir does not exist
				2. Attempt to save the file itself
				3. Attempt to save the cache_object_info json
				
				Either it all is saved perfectly or we delete it all and report an error. This is an all or nothing situation.
				*/

				res.out_path = CACHE_folder + DIR_STR + req.in_folder_prefix + DIR_STR + FILE_get_file_name(req.in_url);
				FILE_make_dir_exist(string(CACHE_folder + DIR_STR + req.in_folder_prefix).c_str());

				//We test if file and cache_file_info file save PERFECECTLY, with NO issues
				//If anything went wrong we call it an error
				bool good_file_save = false;

				//Attempt to save the file content
				RAII_FileHandle f(res.out_path,"w");
				
				if(f() != NULL)
				{
					const vector<char>& buf = co.get_RAII_curl_hndl()->get_dest_buffer();
					for(vector<char>::const_iterator itr = buf.begin(); itr != buf.end(); ++itr)
					{
						fprintf(f(),"%c",*itr);
					}

					good_file_save = ferror(f()) == 0 ? true : false;
				}

				bool good_info_save = false;

				RAII_FileHandle cache_object_info(string(res.out_path).append(CACHE_INFO_FILE_EXT), "w");
				if(cache_object_info() != NULL && good_file_save == true)
				{
					Json::Value root(Json::objectValue);

					root["domain"] = Json::Value(static_cast<int>(req.in_domain));

					struct stat meta_data;
					if(FILE_get_file_meta_data(res.out_path, meta_data) == 0)
					{
						root["last_time_modified"] = Json::Value((Json::Value::Int64)meta_data.st_mtime);

						fprintf(cache_object_info(),"%s", Json::FastWriter().write(root).c_str());

						good_info_save = ferror(cache_object_info()) == 0 ? true : false;
					}
				}

				if(good_file_save == false || good_info_save == false)
				{
					FILE_delete_file(f.path().c_str(), false);
					FILE_delete_file(cache_object_info.path().c_str(), false);
					res.out_error_human = res.out_path + " could not be saved, check if the folder or file is in use or if you have sufficient privaleges";
					co.set_last_error_type(cache_error_type_disk_write);
				}
#endif
				res.out_error_type = co.get_last_error_type();
				co.set_disk_location(res.out_path);
				co.close_RAII_curl_hndl();

				return res;
			}
			else
			{
				WED_file_cache_response res(hndl.get_progress(), "", cache_error_type_unknown, "", cache_status_error);
				interpret_error(hndl, res.out_error_human, res.out_error_type);

				co.trigger_cool_down();

				co.set_last_error_type(res.out_error_type);

				co.close_RAII_curl_hndl();
				return res;
			}//end if(hndl.is_ok())
		}
		else
		{
			DebugAssert(co.get_response_from_object_state(cache_status_downloading) == WED_file_cache_response(hndl.get_progress(), "", cache_error_type_none, "", cache_status_downloading));
			return co.get_response_from_object_state(cache_status_downloading);
		}//end if(hndl.is_done())
	}//end if((**itr).get_RAII_curl_hndl() != NULL)
	else
	{
		int seconds_left = co.cool_down_seconds_left(GetDomainPolicy(req.in_domain));
		if(seconds_left > 0)
		{
			return WED_file_cache_response(-1, "Cache cooling after failed network attempt, please wait: " + itoa_cpp98(seconds_left) + " seconds...", cache_error_type_none, "", cache_status_cooling);
		}
		else if(FILE_exists((*itr)->get_disk_location().c_str()) == true) //Check if file was deleted between requests
		{
			CACHE_domain_policy pol = GetDomainPolicy(cache_domain_metadata_csv);
			if(co.needs_refresh(pol) == false)
			{
				DebugAssert((*itr)->get_disk_location() != "");
				return WED_file_cache_response(-1, "", cache_error_type_none, (*itr)->get_disk_location(), cache_status_available);
			}
			else
			{
				remove_cache_object(itr);
				return start_new_cache_object(req);
			}
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
//---------------------------------------------------------------------------//
