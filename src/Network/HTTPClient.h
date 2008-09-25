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
	This revision is a slight step backward in that we cannot download
	indeterminate-length streams (we must have a content) length.  This
	was done to simplify the design and still allow pipelining.

	Two classes make up the tools for HTTP fetches:

	1.	The HTTPConnection object represents a single connection to the
		server.  Connection objects are disposable - that is, once you
		are done talking to the server (whether by being done or because
		the connection dies unexpecetedly) that object has no further
		use and should be disposed of.  HTTPConnection objects contain
		very little "stuff" - that is, just the name and port of the
		server to call up.

	2.	The HTTPRequest object encapsulates a particular request to the
		server - one connection can handle a number of requests.  These
		requests are "persistent" in that, if you fail, you can keep
		the object around, attach it to a new connection and try
		again.  (Attaching it to a dead connection is legal but stupid,
		as there is no way the conncetion can possibly come back to life
		once it reaches the dead state.)

	PAYLOADS

	On the send-side, a request can contain a "payload".  (For example, in
	a SOAP request, the XML request is the content for the POST command.)
	In this case, the payload is a binary buffer that must be fully
	provided in memory when the request object is made - all info about
	the request, including fields and payload, are copied into internal
	memory so that the client doesn't need to keep track of such things to
	retry the request.

	On the receive side, clients can specify a file name for download-to-disk,
	otherwise any payload is retained in memory.  Payload can be "drained"
	on the fly, that is, you can get additional bytes during  download.
	Write-to-disk happens incrementally, so theoretically download-to-disk
	should be able to handle large chunks of data.

	WARNING: download-to-disk does not pre-reserve disk space, so we could
	run out of disk space mid-download...the class does NOT yet handle this.

	RUNNING THE OBJECT

	To run a download, you need to:

	1.	Create an HTTPConnection object if you don't have one you want
		to use (or if your previous connction is dead.)
	2.	Create an HTTPRequest for whatever it is you're trying to do,
		using that connection.
	3.	While the request is not "done", all do-processing on the connection.
	4.	Check the result of the HTTPRequest once it's done, an error may
		have occurred.  (At this point, a file on disk should be downloaded
		and closed.)
	5.	Check the status of the connection.
		- Once it is idle you can kill it off.
		- Once it is idle and not alive, you should kill it off.
		- If it is not idle, call do processing.  (Even if it's dead, it
		  may take calls to DoProcessing to go idle.  This is because the
		  "alive" status is checked against the socket, but only
		  DoProcessing does the ral work of detaching any requests.)

	COPING WITH DEAD CONNECTIONS

	If a connection dies, the order of handling to retry it must be:

	1.	If the connection is not idle, call do-processing until
		do-processing returns 0.
	2. 	Then call "retry" on the request with a new connection.

	(Retrying a request on a new connection before the old one is idle
	can cause the request object to get trashed.)

	COMPLETED CODES

	Once the download completes, you must figure out what happened.

	-	The connection is eitiher alive or dead, depending on what the socket
		is doing.  Connections only die due to TCP being disconncted.
	-	The connection is idle if do-processing has nothing to do.
	-	The requset, when finished, has a response code, indicating a network
		error, or an HTTP response code (which can be successful or a failure).

	Note that if we get, say, a 500 error because the server is busy, the
	connection may still be alive.  This is because the server told us it was
	busy but didn't "hang up" (which I believe is correct server behavior).

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

enum {
	status_Pending = 0,				// The request is in progress
	status_SocketError = -1			// The socket died while servicing us, or before getting to us.
};


#include <string>
#include <list>
#include <vector>

using std::list;
using std::vector;
using std::string;

class	PCSBSocket;
class	HTTPRequest;

typedef	map<string, string>	FieldMap;

class	HTTPConnection {
public:

			 HTTPConnection(
					const char *			inServerIP,
					unsigned short			inPort);

			~HTTPConnection();

	void	Kill(void);						// Attempt to abort the connection.  You'll have to run until we get a 0 from DoProcessing, but that should be quick.
	int		DoProcessing(void);				// Process incoming requests.  Returns 1 if successful, 0 if something has gone wrong.
	bool	IsIdle(void);					// Returns true if there is no work to be done, e.g. all requests have been processed.  (Dead connections may be idle.)
	bool	IsAlive(void);					// Tells if socket is alive.  Note that if we have NOT gotten a "0" from do-processing we SHOULD call do-processing to "clear out" bad connections so we can retry them.
	int		QueueDepth(void);				// How many queued requests so far?

private:

	friend class	HTTPRequest;

	void			SendData(const char * p1, const char * p2);

	PCSBSocket *		mSocket;			// Socket
	vector<char>		mOutBuf;			// Outgoing buffer
	vector<char>		mInBuf;				// Incoming buffer
	list<HTTPRequest *>	mReqs;				// All pipelined requests.  Front of list is first one tried.

};

class	HTTPRequest {
public:

	/* This routine establishes the HTTP transaction. */
			HTTPRequest(
				HTTPConnection *	inConnection,
				const char *		inURL,					// Starts with slash, no server name
				bool				inIsPost,				// True for POST, false for GET
				const FieldMap&		inExtraFields,			// Extra fields to attach to the request
				const char *		inContentBuffer,		// Optional buffer of payload or NULL
				int					inContentBufferLength,	// Size of payload in bytes if there is one
				const char *		inDestFile);			// Name of a file to save to or NULL to keep in memory.

			~HTTPRequest();

	void	Retry(HTTPConnection *	inConnection);			// Retry on a new connection.  IMPORTANT: only use if we are done.

	bool	IsQueued(void);									// Are we on a queue?  If we're not done, this is useful to know.  (If not, we can retry.)
	bool	IsDone(void);									// True if there is no more network activity we're waiting on.  (Includes success and failure.)
	bool	IsError(void);									// True if something went wrong, network or otherwise.
	int		GetResponseNum(void);							// Response code...-1 (status_Error) for low level socket failure, otherwise it's an HTTP response code.
	string	GetResponseName(void);							// Text from the response code, human readable.
	void	GetResponseFields(FieldMap& outFields);			// Total set of response fields.

	int		GetData(void * outBuffer, int inSize);			// Data fetchers.  Use only if we are not writing to a file.
	void	GetData(vector<char>& outBuffer);
	int		GetTotalExpected(void);							// Returns -1 if we don't know, or expected payload in bytes.
	float	GetPercentDone(void);							// Percent done or -1 if we are not receiving paylod.

private:

	friend class	HTTPConnection;

	int			ParseMore(vector<char>& io_buf);

	HTTPConnection *	mConnection;		// Our connection - set to NULL when we are completed.

	vector<char>		mRequest;			// Copy of the total request, used for retrying.

	bool				mGotWholeHeader;	// Already saw the whole HTTP header?  (Then we are in payload.)
	int					mResponseNum;		// E.g. 200
	string				mResponseName;		// E.g. OK
	FieldMap			mFields;			// All fields verbatim
	vector<char>		mPayload;			// If not saving to a file, this is where our data shows up!
	FILE *				mDestFile;			// File to save to, if open.
	string				mDestFileName;		// Name of file to save to.
	int					mIncomingLength;	// How many bytes payload do we expect?
	int					mReceivedPayload;	// How much of the incoming data have we seen?

};

#endif
