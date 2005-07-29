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



HTTPClient::HTTPClient(
	const char *		inServerIP,
	unsigned short		inPort,		// Usually 80
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
	
	unsigned long	ip = PCSBSocket::LookupAddress(inServerIP);
	if (ip == 0)
		return;

	mResponseNum = -1;
	
	mSocket = new PCSBSocket(0, false);
	mSocket->Connect(ip, inPort);
	
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

	mOutBuf.insert(mOutBuf.end(), request.begin(), request.end());
	if (inContentBuffer)
		mOutBuf.insert(mOutBuf.end(), inContentBuffer, inContentBuffer + inContentBufferLength);	
}	
	
HTTPClient::~HTTPClient()
{
	if (mDestFile)	fclose(mDestFile);
	delete mSocket;
}

int		HTTPClient::GetData(void * outBuffer, int inSize)
{
	int	readSize = min(inSize, (int) mInBuf.size());
	memcpy(outBuffer, &*mInBuf.begin(), readSize);
	mInBuf.erase(mInBuf.begin(), mInBuf.begin() + readSize);
	return readSize;
}
void	HTTPClient::GetData(vector<char>& foo)
{
	foo = mInBuf;
	mInBuf.clear();
}
	
int		HTTPClient::GetTotalExpected(void)
{
	return mIncomingLength;
}

HTTPClient::Status	HTTPClient::GetStatus(void)
{
	if (mSocket == NULL) return status_Error;
	PCSBSocket::Status stat = mSocket->GetStatus();
	if (stat == PCSBSocket::status_Connecting) return status_Connecting;
	if (stat == PCSBSocket::status_Error) return status_Error;
	if (stat == PCSBSocket::status_Disconnected) return status_Done;
	
	if (mOutBuf.size() > 0) return status_Requesting; 
	
	return mGotWholeHeader ? status_ReceivingPayload : status_ReceivingHeaders;
}

float	HTTPClient::GetPercentDone(void)
{
	if (mIncomingLength == -1) return -1.0;
	return ((float) mReceivedPayload) / ((float) mIncomingLength);
}
	
int		HTTPClient::GetResponseNum(void)
{
	return mResponseNum;
}

string	HTTPClient::GetResponseName(void)
{
	return mResponseName;
}

void	HTTPClient::GetResponseFields(FieldMap& outFields)
{
	outFields = mFields;
}

void	HTTPClient::DoProcessing(void)
{
	if (mSocket == NULL) return;
	if (mSocket->GetStatus() == PCSBSocket::status_Disconnected)
	{
		if (mDestFile)
		{
			fclose(mDestFile);
			mDestFile = NULL;
		}	
	}	

	if (mSocket->GetStatus() == PCSBSocket::status_Error)
	{
		if (mDestFile)
		{
			fclose(mDestFile);
			mDestFile = NULL;
		}	
	}	
	
	if (mSocket->GetStatus() == PCSBSocket::status_Connected)
	{
		if (!mOutBuf.empty())
		{
			int	writeLen = mSocket->WriteData(&*mOutBuf.begin(), mOutBuf.size());
			if (writeLen > 0)
				mOutBuf.erase(mOutBuf.begin(), mOutBuf.begin() + writeLen);
			if (mOutBuf.empty())
				mSocket->Release();
		}
		
		char	readChunk[1024];
		int	readLen = mSocket->ReadData(readChunk, sizeof(readChunk));
		if (readLen > 0)
		{
			mInBuf.insert(mInBuf.end(), readChunk, readChunk + readLen);
			ParseMore();
		}
	}
}

bool	HTTPClient::IsDone(void)
{
	HTTPClient::Status stat = this->GetStatus();
	return (stat == status_Done || stat == status_Error);
}

void		HTTPClient::ParseMore(void)
{
	if (!mGotWholeHeader)
	{
		for (int n = 1; n < mInBuf.size(); ++n)
		{
			if (mInBuf[n-1] == '\r' &&
				mInBuf[n  ] == '\n')
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
					mInBuf.erase(mInBuf.begin(), mInBuf.begin() + 2);
					break;
				} else {
					string	oneLine(mInBuf.begin(), mInBuf.begin() + n - 1);
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
				
				mInBuf.erase(mInBuf.begin(), mInBuf.begin() + n + 1);
				n = 0;				
			}
		}
	}
	if (mGotWholeHeader)
	{
		if (((mReceivedPayload + mInBuf.size()) >= mIncomingLength) && (mIncomingLength != -1))
		{
			if (mSocket) mSocket->Disconnect();
		}
		
		if (mDestFile && !mInBuf.empty())
		{
			mReceivedPayload += mInBuf.size();
			fwrite(&*mInBuf.begin(), 1, mInBuf.size(), mDestFile);
			mInBuf.clear();
		}
	}	
}