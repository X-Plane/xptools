#include "WED_FileCache.h"
#include "curl_http.h"
#include "RAII_Classes.h"
#include <time.h>

class CACHE_CacheObject
{
public:
	CACHE_CacheObject(CACHE_content_type type);
	~CACHE_CacheObject();

	//Getters for all properties
	string get_url() const;
	void   set_url(const string& new_url);

	string get_disk_location() const;
	void   set_disk_location(const string& location);

	//Creates and opens a cURL handle to a given url, with a certificate and optional reserve size
	void             create_RAII_curl_hndl(const string& url, const string& cert, int buf_reserve_size=0);

	//Gets the current RAII_CurlHandle object
	RAII_CurlHandle* const get_RAII_curl_hndl();

	//Forces RAII_CurlHandle to close and be destroyed early
	void             close_RAII_curl_hndl();

	CACHE_status get_status() const;
	void         set_status(CACHE_status stat);

	//Trigger the cool down clock to reset and start counting down
	void trigger_cool_down();

	//Is the file cooling down
	bool is_cooling_down() const;

	//Does the Cache object need 
	bool needs_refresh() const;
private:
#if DEV
	int  cool_down_time_left() const;
#endif

	//The file url. Lives from first request to program end
	string m_url;

	//Real FQPN on disk, "" if non-existant. Lives from program start to program end
	string m_disk_location;

	//The curl_http_get_file that is associated with this cache_object
	//Info life span: Download start to download finish or cache shutdown
	RAII_CurlHandle* m_RAII_curl_hndl;

	//The status of the file's progress
	CACHE_status m_status;

	//The content of cache object
	CACHE_content_type m_content_type;

	//What timestamp the trigger
	tm m_trigger_event;

	CACHE_CacheObject(const CACHE_CacheObject& copy);
	CACHE_CacheObject & operator= (const CACHE_CacheObject& rhs);
};
