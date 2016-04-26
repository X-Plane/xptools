#include "RAII_CURL_HNDL.h"

RAII_CURL_HNDL::RAII_CURL_HNDL(): curl_handle(NULL) { }

RAII_CURL_HNDL::~RAII_CURL_HNDL()
{
	delete curl_handle;
	curl_handle = NULL;
}

void RAII_CURL_HNDL::create_HNDL( const string& inURL, const string& inCert, int bufferReserveSize)
{
	if(curl_handle != NULL)
	{
		delete curl_handle;
		curl_handle = NULL;
	}

	rawJSONBuf = vector<char>(bufferReserveSize);
	curl_handle = new curl_http_get_file(inURL,&rawJSONBuf,inCert);
}

curl_http_get_file * const RAII_CURL_HNDL::get_curl_handle() const
{
	return curl_handle;
}

const vector<char>& RAII_CURL_HNDL::get_JSON_BUF()
{
	return rawJSONBuf;
}
