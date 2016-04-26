#ifndef RAII_CURL_HNDL_H
#define RAII_CURL_HNDL_H

#include "curl_http.h"
//starting a new one clears off the old one
class RAII_CURL_HNDL
{
public:
	RAII_CURL_HNDL();
	~RAII_CURL_HNDL();

	void create_HNDL(const string& inURL, const string& inCert, int bufferReserveSize);
	
	curl_http_get_file * const get_curl_handle() const;
	const vector<char>& get_JSON_BUF();

private:
	RAII_CURL_HNDL(const RAII_CURL_HNDL & copy);
	RAII_CURL_HNDL & operator= (const RAII_CURL_HNDL & rhs);

	curl_http_get_file * curl_handle;
	
	//A buffer of chars to be filled
	vector<char> rawJSONBuf;
};

class RAII_file 
{
public:

 RAII_file(const char * fname, const char * mode) :
  mFile(fopen(fname,mode))
 {
 }

 ~RAII_file()
 {
	if(mFile)
	{
		fclose(mFile);
	}
 }

 int close() 
 { 
	 if(mFile) 
	 { 
		 int retVal = fclose(mFile); 
		 mFile = NULL;
		 return retVal;
	 }
	 return 0;
 }

 operator bool() const { return mFile != NULL; }
 FILE * operator()() { return mFile; }

private:
 FILE * mFile;
};
#endif