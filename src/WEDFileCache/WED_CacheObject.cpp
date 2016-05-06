#include "WED_CacheObject.h"
#include "AssertUtils.h"

CACHE_CacheObject::CACHE_CacheObject(CACHE_content_type type) : m_url(""),
																m_disk_location(""),
																m_RAII_curl_hndl(NULL),
																m_status(CACHE_status::file_not_started),
																m_content_type(type),
																m_trigger_event(tm())
{
}

CACHE_CacheObject::~CACHE_CacheObject()
{
	if(m_RAII_curl_hndl != NULL)
	{
		delete m_RAII_curl_hndl;
		m_RAII_curl_hndl;
	}
}

//Getters for all properties
string CACHE_CacheObject::get_url() const
{
	return m_url;
}

void CACHE_CacheObject::set_url(const string& new_url) 
{
	m_url = new_url;
}

string CACHE_CacheObject::get_disk_location() const 
{
	return m_disk_location;
}

void CACHE_CacheObject::set_disk_location(const string& location)
{
	m_disk_location = location;
}

void CACHE_CacheObject::create_RAII_curl_hndl(const string& url, const string& cert, int buf_reserve_size)
{
	//Close off any previous handles to make way for this new one
	this->close_RAII_curl_hndl();
	m_RAII_curl_hndl = new RAII_CurlHandle(url, cert, buf_reserve_size);
}

RAII_CurlHandle* const CACHE_CacheObject::get_RAII_curl_hndl()
{
	return m_RAII_curl_hndl;
}

CACHE_status CACHE_CacheObject::get_status() const
{
	return m_status;
}

void CACHE_CacheObject::close_RAII_curl_hndl()
{
	if(m_RAII_curl_hndl != NULL)
	{
		delete m_RAII_curl_hndl;
		m_RAII_curl_hndl = NULL;
	}
}

void CACHE_CacheObject::set_status(CACHE_status stat)
{
	m_status = stat;
}

//Trigger the cool down clock to reset and start counting down
void CACHE_CacheObject::trigger_cool_down()
{
	
}

//Is the file cooling down
bool CACHE_CacheObject::is_cooling_down() const
{
	return false;
}

int  CACHE_CacheObject::cool_down_time_left() const
{
	return 0;
}

//Does the Cache object need 
bool CACHE_CacheObject::needs_refresh() const
{
	return false;
}
