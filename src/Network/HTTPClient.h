/* 
 * Copyright (c) 2004, Laminar Research.
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
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

/*

	HTTPClient - THEORY OF OPERATION
	
	This class communicates with a web server via the HTTP protocol.
	The protocol is simple: the client makes a request, and the server
	responds.  All TCP transactions are in the form of a header and then
	an optional payload.  The header is cr/lf terminated text strings,
	with a blank line indicating the header end.  The header's first line
	is the request or response command, the rest are fields.
	
	If a payload is present, it is binary following the header; its length
	is either specified in the header or it runs until the connection dies.
	
	This client can do either GET or POST type requests; GET is usually
	right for web pages.  A map of strings is used to specify extra fields.
	The one that is required is a field named "Host" with
	the DNS name of the web server as its contents.  For commands
	with a payload, the object creates the Content-Length field for you.

	RUNNING THE OBJECT
	
	Once you create the object, call DoProcessing() until either IsDone
	returns true or you decide to time out the connection.
	
	The object provides a status code, indicating what is happening; this
	is mostly for informational purposes if you want to show the user a 
	progress report.  For known-sized downloads, a percent of retrieved
	payload may be available.
	
	SENDING DATA
	If the content buffer you pass is not NULL, this data is passed
	as a payload in the HTTP reequest.  Make sure to add a Content-Type
	header or you'll probably get a server error.
	
	GETTING DATA
	
	The data from the download is available in three manners:
	
	1. Specify a filename.  A binary file is written with the content
	of the download.  The file is closed when the object is destroyed
	or the download completes.
	
	2. Use GetData with a buffer to fetch part of the download.
	
	3. Use GetData with a vector of chars to fetch the entire download.
	
	These last two methods may be used while the download is taking place.
	The file cannot be accessed because it is opened write-exclusive.
	
	Headers passed down from the server 
	
	COMPLETED CODES
	
	Once the download completes, you must figure out what happened.  There
	are two final status codes.  If the cilent is in status_Error, some kind
	of unexpected low level networking problem happened with the socket.  
	For status_Done, the transaction completed in some way.  First look 
	at the response code.  If it is -1, no valid HTTP response header was
	received, or the connection never went through.  This probably indicates 
	that the web server is unreachable or totally broken, or maybe not even
	a web server at all.  If the response code is 0, the DNS name could not be
	looked up, so the transaction was never started.
	
	For a positive response number, refer to the standard HTTP protocol
	response numbers.  Typical numbers include:
	
	http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html	
	
	A rough summary:
	
	The 100 errors indicate the process is ongoing; HTTPClient automatically
	keeps processing.  If this code is returned, it indicates that the 
	conection died mid-download, and should be treated as some kind of 
	disconnect error.
	
	100	Continue
	101 Protocol Change
	
	The 200 errors indicate (at least marginal) success; you may use the
	downloaded content.
	
	200 OK
	201 Created
	202 Accepted
	203 Non-Authoritative Information
	204 No Content
	205 Reset Content
	206 Partial Content
	
	The 300 codes indicate redirection.  You must fetch a new resource;
	while there is some subtlety in this code set, the safest action
	for trivial fetching code is to get a new URL out of the Location
	field and retry.
	
	300 Multiple Choices
	301 Moved Permanently
	302 Found
	303 See Other
	304 Not Modified
	305 Use Proxy
	306 (not used)
	307 Temporary Redirect
	
	The 400 codes indicate an error; the request has failed.  In the case of
	401 and 405 codes, a sophisticated client could authenticate and retry,
	but this class does not automatically do this.  400 range error codes
	typically indicate a client error; e.g. what you asked for can't be served
	because it was bogus.
	
	400 Bad Request
	401 Unauthorized
	402 Payment Required
	403 Forbidden
	404 Not Found
	405 Method Not Allowed
	406 Not Acceptable
	407 Proxy Authentication Required
	408 Request Timeout
	409 Conflict
	410 Gone
	411 Length Required
	412 Precondition Failed
	413 Request Entity Too Large
	414 Request URI Too Long
	415 Unsupported Media Type
	416 Requested Range Not Satisfiable
	417 Expectation Failed
	
	The 500 codes indicate an internal server problem.  The request was
	valid but the server's having a bad day.
	
	500 Internal Server Error
	501 Not Implemented
	502 Bad Gateway
	503 Service Unavailablae
	504 Gateway Timeout
	505 HTTP Version Not Supported
	
 */

#include <string>

class	PCSBSocket;

typedef	map<string, string>	FieldMap;

class	HTTPClient {
public:

	enum	Status {
		status_Connecting = 0,		// Trying to connect to the server
		status_Requesting,			// We are connected, sending request
		status_ReceivingHeaders,	// Receiving the header for the response
		status_ReceivingPayload,	// All headers received, receiving payload
		status_Done,				// Transaction done, not necessary successfully.
		status_Error				// Some kind of socket error caused us to barf.
	};

	/* This routine establishes the HTTP transaction. */
			HTTPClient(
				const char *		inServerIP,
				unsigned short		inPort,					// Usually 80
				const char *		inURL,					// Starts with slash, no server name
				bool				inIsPost,				// True for POST, false for GET
				const FieldMap&		inExtraFields,			// Extra fields to attach to the request
				const char *		inContentBuffer,		// Optional buffer of payload or NULL
				int					inContentBufferLength,	// Size of payload in bytes if there is one
				const char *		inDestFile);			// Name of a file to save to or NULL to keep in memory.
				
			~HTTPClient();

	/* This routine does a little bit more work to continue processing; call
	 * repeatedly while IsDone is false. */			
	void	DoProcessing(void);
			
	/* Getting diagnostic informatino about the transaction. */
	Status	GetStatus(void);
	bool	IsDone(void);

	/* Getting Header Information from the transaction.  Header info is
	 * only valid if status > status_receivingHeaders. */
	int		GetResponseNum(void);
	string	GetResponseName(void);
	void	GetResponseFields(FieldMap& outFields);

	/* Getting data from the transaction.  This is only good if
	 * status > status_ReceivingHeaders. */
	int		GetData(void * outBuffer, int inSize);
	void	GetData(vector<char>&);
	int		GetTotalExpected(void);	// Returns -1 if we don't know)
	float	GetPercentDone(void);
	

private:

	void				ParseMore(void);

	PCSBSocket *		mSocket;

	// Sending info
	vector<char>		mOutBuf;			// Outgoing buffer

	// Receiving info
	vector<char>		mInBuf;				// Incoing buffer
	bool				mGotWholeHeader;	// Already saw the whole HTTP header

	// Header info
	int					mResponseNum;		// E.g. 200
	string				mResponseName;		// E.g. OK
	FieldMap			mFields;			// All fields verbatim
	
	// Payload info
	FILE *				mDestFile;			// File to save to
	string				mDestFileName;		// Name of file 	
	int					mIncomingLength;	// How many bytes payload do we expect?
	int					mReceivedPayload;	// How much of the incoming data have we seen?

};	

#endif