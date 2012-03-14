/*
 * Copyright (c) 2011, mroe.
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

#if WITHNWLINK

#include "WED_NWDefs.h"
#include "WED_Connection.h"
#include "PlatformUtils.h"

WED_Connection::WED_Connection(PCSBSocket* inSocket) :

	mWaitCntr(0),HasClient(false) ,mSocket(inSocket)
{

}

WED_Connection::~WED_Connection()
{
	delete mSocket;
}

void 	WED_Connection::Kill()
{
	if(mSocket) mSocket->Disconnect();
}

bool 	WED_Connection::IsAlive()
{
	if( mSocket == NULL ) return false;
	if( mSocket->ReceivedRelease()) return false;
	return (mSocket->GetStatus() == PCSBSocket::status_Connected);
}

int 	WED_Connection::SendData( const char * inBuf, int inSize)
{
	//FIXME:mroe outbuffer overflow ,simple do not write-in anymore
	//           data lost !
	 if (mOutBuf.size() > MAX_BUF_SIZE) return 0;
	 mOutBuf.insert(mOutBuf.end(),inBuf,inBuf+inSize);
	 return 1;
}

int  	WED_Connection::DoProcessing(void)
{
	if (!IsAlive()) return 0;

	if (!mOutBuf.empty())
	{
		int	writeLen = mSocket->WriteData(&*mOutBuf.begin(), mOutBuf.size());
		if (writeLen > 0)
			mOutBuf.erase(mOutBuf.begin(), mOutBuf.begin() + writeLen);
	}
	//FIXME:mroe inbuffer overflow ,simple block reading
	//      overflow is somewhere before now  !
	if (mInBuf.size() > MAX_BUF_SIZE) return 3;

	char readChunk[1024];
	int	readLen  = mSocket->ReadData(readChunk, sizeof(readChunk));
	if(readLen > 0 )
		mInBuf.insert(mInBuf.end(), readChunk, readChunk + readLen);

	if(!mInBuf.empty())	return 2;

	return 1;
}
#endif
