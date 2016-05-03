/* 
 * Copyright (c) 2016, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#ifndef RAII_CLASSES_H
#define RAII_CLASSES_H

/*
	This header contains a collection of RAII classes for XPTools.
*/
#include "curl_http.h"

//Can opens a cURL Handle to a specified URL, closes handle and clears buffer on destruction
class RAII_CurlHandle
{
public:
	RAII_CurlHandle();
	~RAII_CurlHandle();

	//Create the connection
	void create_HNDL(const string& inURL, const string& inCert, int bufferReserveSize);
	
	//Get curl_http_get_file handle
	curl_http_get_file* const get_curl_handle();

	//Closes handle and clears JSON buffer
	void close_curl_handle();

	//Gets the curl_http_get_file buffer
	const vector<char>& get_dest_buffer() const;

private:
	RAII_CurlHandle(const RAII_CurlHandle& copy);
	RAII_CurlHandle& operator= (const RAII_CurlHandle& rhs);

	curl_http_get_file* curl_handle;
	
	//A buffer of chars to be filled
	vector<char> m_dest_buffer;
};

//Opens a FILE* handle upon creation, closes file on dtor
class RAII_File 
{
public:

	RAII_File(const char* fname, const char* mode);
	~RAII_File();

	int close();

	operator bool() const { return mFile != NULL; }
	FILE* operator()() { return mFile; }

private:
	FILE* mFile;
};
#endif