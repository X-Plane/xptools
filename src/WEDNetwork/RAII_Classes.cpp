#include "RAII_Classes.h"

RAII_CurlHandle::RAII_CurlHandle() : curl_handle(NULL)
{
}

RAII_CurlHandle::~RAII_CurlHandle()
{
	this->close_curl_handle();
}

void RAII_CurlHandle::create_HNDL(const string& inURL, const string& inCert, int bufferReserveSize)
{
	if(curl_handle != NULL)
	{
		delete curl_handle;
		curl_handle = NULL;
	}

	m_dest_buffer = vector<char>(bufferReserveSize);
	curl_handle = new curl_http_get_file(inURL,&m_dest_buffer,inCert);
}

curl_http_get_file * const RAII_CurlHandle::get_curl_handle()
{
	return curl_handle;
}

void RAII_CurlHandle::close_curl_handle()
{
	if(curl_handle != NULL)
	{
		delete curl_handle;
		curl_handle = NULL;
	}
	m_dest_buffer.clear();
}

const vector<char>& RAII_CurlHandle::get_dest_buffer() const
{
	return m_dest_buffer;
}

RAII_File::RAII_File(const char * fname, const char * mode) : mFile(fopen(fname,mode))
{
}

RAII_File::~RAII_File()
{
	if(mFile)
	{
		fclose(mFile);
		mFile = NULL;
	}
}

int RAII_File::close()
{
	if(mFile)
	{
		int retVal = fclose(mFile); 
		mFile = NULL;
		return retVal;
	}
	return 0;
}