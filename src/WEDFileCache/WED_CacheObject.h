#include "WED_FileCache.h"
#include "curl_http.h"
#include "RAII_Classes.h"

//All time is is in seconds
class CACHE_LifespanInfo
{
public:
	CACHE_LifespanInfo(WED_file_cache_content_type type);
#if DEV
	CACHE_LifespanInfo(int cool_down_left, int life_left);
#endif

	~CACHE_LifespanInfo();

	bool is_cooling_down();
	int  cool_down_time_left();

	//
	bool needs_refresh();
private:
	int m_cooldown_left;
	int m_life_left;
};
/*
bool IsInCooldown
int  TimeLeft
CoolDown Activate
Cool

RAII like object, time is always "decreasing". When asked it'll quickly calculate time left. Anything negative is just counted as nothing


Default constructor auto figures it out
CACHE_LifeSpanInfo
variables
int resettime - manually configurable or instantly assaigned based on static vs dynamic type
int timeleft
*/

	
class CACHE_CacheObject
{
public:
	CACHE_CacheObject() { status = WED_file_cache_status::file_not_started; }

	//Getters for all properties
	string get_url()           const { return url; }
	void   set_url(const string& new_url) { url = new_url; }

	string get_disk_location() const { return disk_location; }
	void   set_disk_location(const string& location) { disk_location = location; }

	RAII_CurlHandle& get_curl_file() { return curl_file; }
	WED_file_cache_status get_status() const { return status; }
	void                  set_status(WED_file_cache_status stat) { status = stat; }

private:
	//The file url. Lives from first request to program end
	string url;

	//Real FQPN on disk, "" if non-existant. Lives from program start to program end
	string disk_location;

	//The curl_http_get_file that is associated with this cache_object
	//Info life span: Download start to download finish
	RAII_CurlHandle curl_file;

	//The status of the file's progress
	WED_file_cache_status status;

	CACHE_CacheObject(const CACHE_CacheObject& copy);
	CACHE_CacheObject & operator= (const CACHE_CacheObject& rhs);
};