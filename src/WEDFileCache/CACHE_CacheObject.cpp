#include "CACHE_CacheObject.h"
#include "CACHE_DomainPolicy.h"
#include "AssertUtils.h"
#include <iostream>
#include "FileUtils.h"

CACHE_CacheObject::CACHE_CacheObject()
	: m_cool_down_timestamp(0),
	  m_disk_location(""),
	  m_domain(cache_domain_none),
	  m_last_error_type(cache_error_type_none),
	  m_last_time_modified(0),
	  m_last_url(""),
	  m_RAII_curl_hndl(NULL)
{
}

CACHE_CacheObject::~CACHE_CacheObject()
{
	if(m_RAII_curl_hndl != NULL)
	{
		delete m_RAII_curl_hndl;
		m_RAII_curl_hndl = NULL;
	}
}

const time_t CACHE_CacheObject::get_cool_down_time() const
{
	return m_cool_down_timestamp;
}

void CACHE_CacheObject::reset_cool_down_time()
{
	m_cool_down_timestamp = 0;
}

const string& CACHE_CacheObject::get_disk_location() const 
{
	return m_disk_location;
}

void CACHE_CacheObject::set_disk_location(const string& location)
{
	m_disk_location = location;
}

CACHE_domain CACHE_CacheObject::get_domain() const
{
	return m_domain;
}

const string& CACHE_CacheObject::get_last_url() const
{
	return m_last_url;
}

CACHE_error_type CACHE_CacheObject::get_last_error_type() const
{
	return m_last_error_type;
}

void             CACHE_CacheObject::set_last_error_type(CACHE_error_type error_type)
{
	m_last_error_type = error_type;
}

time_t           CACHE_CacheObject::get_last_time_modified() const
{
	return m_last_time_modified;
}

void             CACHE_CacheObject::set_last_time_modified(time_t mtime)
{
	m_last_time_modified = mtime;
}

void CACHE_CacheObject::create_RAII_curl_hndl(const string& url, const string& cert, int buf_reserve_size)
{
	//Close off any previous handles to make way for this new one
	this->close_RAII_curl_hndl();
	m_RAII_curl_hndl = new RAII_CurlHandle(url, cert, buf_reserve_size);
	m_last_url = m_RAII_curl_hndl->get_curl_handle().get_url();
}

void CACHE_CacheObject::close_RAII_curl_hndl()
{
	if(m_RAII_curl_hndl != NULL)
	{
		delete m_RAII_curl_hndl;
		m_RAII_curl_hndl = NULL;
	}
}

WED_file_cache_response CACHE_CacheObject::get_response_from_object_state(CACHE_status status) const
{
	return WED_file_cache_response(m_RAII_curl_hndl->get_curl_handle().get_progress(),
								   "",
								   get_last_error_type(),
								   get_disk_location(),
								   status);
}

RAII_CurlHandle* const CACHE_CacheObject::get_RAII_curl_hndl()
{
	return m_RAII_curl_hndl;
}

//Trigger the cool down clock to reset and start counting down
void CACHE_CacheObject::trigger_cool_down()
{
	//If we are triggering cool down twice we and not deleting and restarting objects after a cooldown is finished
	DebugAssert(m_cool_down_timestamp == 0);
	m_cool_down_timestamp = time(NULL);

#if DEV
	std::cout << "Cooldown triggered: " << asctime(localtime(&m_cool_down_timestamp)) << endl;
#endif
}

//Is the file cooling down
int CACHE_CacheObject::cool_down_seconds_left(const CACHE_domain_policy& policy) const
{
	double time_delta = difftime(time(NULL), m_cool_down_timestamp);
	int diff_seconds = policy.cache_domain_pol_min_client_cool_down_snds - time_delta;

	int seconds_left = 0;

	//For each error type, has enough time passed?
	switch(this->m_last_error_type)
	{
	case cache_error_type_disk_write:
	case cache_error_type_none:        seconds_left = 0; break;
	case cache_error_type_client_side: seconds_left = time_delta > policy.cache_domain_pol_min_client_cool_down_snds ? 0 : diff_seconds; break;
	case cache_error_type_server_side: seconds_left = time_delta > policy.cache_domain_pol_min_client_cool_down_snds ? 0 : diff_seconds; break;
	case cache_error_type_unknown:     seconds_left = time_delta > policy.cache_domain_pol_min_client_cool_down_snds ? 0 : diff_seconds; break;
	default: AssertPrintf("%d is an unknown CACHE_error", m_last_error_type);
	}
#if DEV
	//std::cout << "Time since last is_cooling_down: " << time_delta << endl;
	//std::cout << "cool_down_seconds_left: " << seconds_left << endl;
#endif
	return seconds_left;
}

//Has cache object gone stale?
bool CACHE_CacheObject::needs_refresh(const CACHE_domain_policy& policy) const
{
	if(m_disk_location == "")
	{
		return true;
	}

	double time_diff = difftime(time(NULL), m_last_time_modified);

	return time_diff > policy.cache_domain_pol_max_seconds_on_disk ? true : false;
}
