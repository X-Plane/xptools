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
#include "HTTPClient.h"
#include "PCSBSocket.h"


HTTPConnection::HTTPConnection(
		const char *			inServerIP,
		unsigned short			inPort)
{
	mSocket = NULL;
	unsigned long	ip = PCSBSocket::LookupAddress(inServerIP);
	if (ip == 0)
		return;

	mSocket = new PCSBSocket(0, false);
	mSocket->Connect(ip, inPort);	
}

HTTPConnection::~HTTPConnection()
{
	delete mSocket;
}
	
void	HTTPConnection::DoProcessing(void)
{
	if (mSocket == NULL) return;
	if (mSocket->GetStatus() == PCSBSocket::status_Disconnected)
	{
		#if !DEV 
			handle this		
		#endif
	}	

	if (mSocket->GetStatus() == PCSBSocket::status_Error)
	{
		#if !DEV 
			handle this		
		#endif
	}	
	
	if (mSocket->GetStatus() == PCSBSocket::status_Connected)
	{
		if (!mOutBuf.empty())
		{
			int	writeLen = mSocket->WriteData(&*mOutBuf.begin(), mOutBuf.size());
			if (writeLen > 0)
				mOutBuf.erase(mOutBuf.begin(), mOutBuf.begin() + writeLen);
		}
		
		char	readChunk[1024];
		int	readLen = mSocket->ReadData(readChunk, sizeof(readChunk));
		if (readLen > 0)
			mInBuf.insert(mInBuf.end(), readChunk, readChunk + readLen);

		if (!mReqs.empty() && !mInBuf.empty())
		if (mReqs.front()->ParseMore(mInBuf))
			mReqs.pop_front();
	}

}

bool	HTTPConnection::IsIdle(HTTPRequest * req)
{
	return mReqs.empty();
}

void			HTTPConnection::SendData(const char * p1, const char * p2)
{
	mOutBuf.insert(mOutBuf.end(),p1,p2);
}



HTTPRequest::HTTPRequest(
	HTTPConnection *	inConnection,
	const char *		inURL,
	bool				inIsPost,
	const FieldMap&		inExtraFields,
	const char *		inContentBuffer,
	int					inContentBufferLength,
	const char *		inDestFile)
{
	mDestFile = NULL;
	mDestFileName = inDestFile ? inDestFile : "";
	mIncomingLength = -1;
	mResponseNum = 0;
	mReceivedPayload = 0.0;
	mGotWholeHeader = false;	
	mResponseNum = -1;
	mConnection = inConnection;
	
	string	request;

	if (inIsPost)
		request = "POST "; 
	else
		request = "GET ";
	request += inURL;
	request += " HTTP/1.1\r\n";
	for (FieldMap::const_iterator iter = inExtraFields.begin();
		iter != inExtraFields.end(); ++iter)
	{
		request += iter->first;
		request += ": ";
		request += iter->second;
		request += "\r\n";
	}
	if (inContentBuffer)
	{
		char	buf[256];
		sprintf(buf, "Content-Length: %d\r\n", inContentBufferLength);
		request += buf;
	}
	request += "\r\n";

	mConnection->mReqs.push_back(this);
	mConnection->SendData(&*request.begin(), &*request.end());
	if (inContentBuffer) mConnection->SendData(inContentBuffer,inContentBuffer+inContentBufferLength);
}	
	
HTTPRequest::~HTTPRequest()
{
	if (mDestFile)	fclose(mDestFile);
}

int		HTTPRequest::GetData(void * outBuffer, int inSize)
{
	int	readSize = min(inSize, (int) mPayload.size());
	memcpy(outBuffer, &*mPayload.begin(), readSize);
	mPayload.erase(mPayload.begin(), mPayload.begin() + readSize);
	return readSize;
}
void	HTTPRequest::GetData(vector<char>& foo)
{
	foo.clear();
	foo.swap(mPayload);
}
	
int		HTTPRequest::GetTotalExpected(void)
{
	return mIncomingLength;
}

float	HTTPRequest::GetPercentDone(void)
{
	if (mIncomingLength == -1) return -1.0;
	return ((float) mReceivedPayload) / ((float) mIncomingLength);
}
	
int		HTTPRequest::GetResponseNum(void)
{
	return mResponseNum;
}

string	HTTPRequest::GetResponseName(void)
{
	return mResponseName;
}

void	HTTPRequest::GetResponseFields(FieldMap& outFields)
{
	outFields = mFields;
}

bool	HTTPRequest::IsDone(void)
{
	return mGotWholeHeader && mIncomingLength == mReceivedPayload;
}

int		HTTPRequest::ParseMore(vector<char>& io_buf)
{
	if (!mGotWholeHeader)
	{
		for (int n = 1; n < io_buf.size(); ++n)
		{
			if (io_buf[n-1] == '\r' &&
				io_buf[n  ] == '\n')
			{
				// We have a line!!
				if (n == 1)
				{
					if (mResponseNum >= 200)
					{
						mGotWholeHeader = true;
						if (!mDestFileName.empty())
							mDestFile = fopen(mDestFileName.c_str(), "wb");
						FieldMap::iterator i = mFields.find("Content-Length");
						if (i != mFields.end())
							mIncomingLength = atoi(i->second.c_str());
					}
					io_buf.erase(io_buf.begin(), io_buf.begin() + 2);
					break;
				} else {
					string	oneLine(io_buf.begin(), io_buf.begin() + n - 1);
				if (oneLine.substr(0, 5) == "HTTP/")
					{
						string	rev = oneLine.substr(5, 3);
						string	code = oneLine.substr(9, 3);
						mResponseName = oneLine.substr(13);
						mResponseNum = atoi(code.c_str());
					} else {
						string::size_type p = oneLine.find(": ");
						if (p != oneLine.npos)
						{
							mFields.insert(FieldMap::value_type(
								oneLine.substr(0, p),
								oneLine.substr(p+2)));
						}
					}
				}
				
				io_buf.erase(io_buf.begin(), io_buf.begin() + n + 1);
				n = 0;				
			}
		}
	}
	if (mGotWholeHeader)
	{
		if (((mReceivedPayload + io_buf.size()) >= mIncomingLength) && (mIncomingLength != -1))
		{
			#if !DEV
				handle this
			#endif
		}
		
		if (!io_buf.empty() && mIncomingLength != mReceivedPayload)
		{
			int write_size = mIncomingLength - mReceivedPayload;
			if (write_size > io_buf.size()) write_size = io_buf.size();
			
			if (mDestFile)
				fwrite(&*io_buf.begin(), 1, write_size, mDestFile);
			else
				mPayload.insert(mPayload.end(),io_buf.begin(),io_buf.begin() + write_size);
			
			mReceivedPayload += write_size;
			io_buf.erase(io_buf.begin(), io_buf.begin()+write_size);
		}
	}
	
	if (IsDone())
	{
		if (mDestFile) { fclose(mDestFile); mDestFile = NULL; }
		return 1;
	}
	return 0;
}
