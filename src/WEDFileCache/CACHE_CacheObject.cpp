#include "CACHE_CacheObject.h"
#include "AssertUtils.h"
#include <iostream>

CACHE_CacheObject::CACHE_CacheObject(CACHE_content_type type)
	: m_content_type(type),
	  m_cool_down_timestamp(0),
	  m_disk_location(""),
	  m_last_error_type(CACHE_error_type::none),
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

/*
CACHE_status CACHE_CacheObject::get_status() const
{
	return m_status;
}

void CACHE_CacheObject::set_status(CACHE_status stat)
{
	m_status = stat;
}
*/

//Trigger the cool down clock to reset and start counting down
void CACHE_CacheObject::trigger_cool_down()
{
	m_cool_down_timestamp = time(NULL);

#if DEV
	std::cout << "Cooldown triggered: " << asctime(localtime(&m_cool_down_timestamp)) << endl;
#endif
}

//Is the file cooling down
int CACHE_CacheObject::cool_down_seconds_left() const
{
	int seconds_left = 0;

	time_t now = time(NULL);
	time_t t = m_cool_down_timestamp;
	double time_delta = difftime(now,t);

#if DEV
	std::cout << "Time since last is_cooling_down: " << time_delta << endl;
	//For the catagory, has enough time passed? - NOTE: Edit the values in this switch statement to change cache behavior
	switch(this->m_last_error_type)
	{
	case CACHE_error_type::none:        seconds_left = true; break;
	case CACHE_error_type::disk_write:
	case CACHE_error_type::client_side: seconds_left = time_delta > 60 ? 0 : 60 - time_delta; break;
	case CACHE_error_type::server_side: seconds_left = time_delta > 60 ? 0 : 60 - time_delta; break;
	case CACHE_error_type::unknown:     seconds_left = time_delta > 10 ? 0 : 10 - time_delta; break;
	default: AssertPrintf("%d is an unknown CACHE_error_type", m_last_error_type);
	}
	std::cout << "cool_down_seconds_left: " << seconds_left << endl;
#else
	switch(error_type)
	{
	case CACHE_error_type::none:        seconds_left = true; break;
	case CACHE_error_type::disk_write:
	case CACHE_error_type::client_side: seconds_left = time_delta > 60 ? 0 : 60 - time_delta; break;
	case CACHE_error_type::server_side: seconds_left = time_delta > 60 ? 0 : 60 - time_delta; break;
	case CACHE_error_type::unknown:     seconds_left = time_delta > 10 ? 0 : 10 - time_delta; break;
	default: AssertPrintf("%d is an unknown CACHE_error_type", m_last_error_type);
	}
#endif
	return seconds_left;
}

//Has cache object gone stale? 
/*TODO: bool CACHE_CacheObject::needs_refresh() const
{
	return false;
}
*/