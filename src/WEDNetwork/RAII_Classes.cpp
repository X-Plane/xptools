#include "RAII_Classes.h"

//--RAII_CurlHandle------------------------------------------------------------
RAII_CurlHandle::RAII_CurlHandle(const string& url, const string& cert, int buf_reserve_size) :
	m_dest_buffer(vector<char>(buf_reserve_size)),
	m_curl_handle(curl_http_get_file(url, &m_dest_buffer, cert))
{
}

curl_http_get_file& RAII_CurlHandle::get_curl_handle()
{
	return m_curl_handle;
}

const vector<char>& RAII_CurlHandle::get_dest_buffer() const
{
	return m_dest_buffer;
}
//---------------------------------------------------------------------------//

//--RAII_File------------------------------------------------------------------
RAII_FileHandle::RAII_FileHandle(const char * fname, const char * mode) : mFile(fopen(fname,mode))
{
}

RAII_FileHandle::~RAII_FileHandle()
{
	if(mFile)
	{
		fclose(mFile);
		mFile = NULL;
	}
}

int RAII_FileHandle::close()
{
	if(mFile)
	{
		int retVal = fclose(mFile); 
		mFile = NULL;
		return retVal;
	}
	return 0;
}
//---------------------------------------------------------------------------//
