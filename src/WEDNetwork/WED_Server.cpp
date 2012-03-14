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

#include "WED_Server.h"
#include "WED_Connection.h"
#include "WED_Messages.h"
#include "WED_NWDefs.h"
#include "WED_Version.h"
#include "PlatformUtils.h"
#include "PCSBSocket.h"

WED_Server::WED_Server(const string& inPkgName,unsigned short inPort):
mServerSocket(NULL),mStarted(false),mPkgName(inPkgName),mPort(inPort),mConnection(NULL)

{
	static bool first_time = true;
	if (first_time)
		PCSBSocket::StartupNetworking(true);
	first_time = false;
	mIdent     = (unsigned long) this;
}

WED_Server::~WED_Server()
{
	delete mConnection;
	delete mServerSocket;
	PCSBSocket::ShutdownNetworking(true);
}

int WED_Server::DoStart()
{
	if (mStarted) return true;

	mServerSocket = new PCSBSocket(mPort,true);
	PCSBSocket::Status stat= mServerSocket->GetStatus();
	if (stat == PCSBSocket::status_Error)
	{
		delete mServerSocket;
		mServerSocket = NULL;
		mStarted = false;
		char buf[256];
		sprintf(buf,"error bind on port %d\n",mPort);
		DoUserAlert(buf);
	}
	else
	{
		this->Start(INTERVALT);
		mStarted = true;
		BroadcastMessage(msg_NetworkStatusInfo,s_started);
	}
	return mStarted;
}

void WED_Server::DoStop()
{
	if (!mStarted) return ;
	if (mConnection)
	{
		mConnection->SendData("\r\nbye\r\n",7);
		mConnection->DoProcessing();
		mConnection->Kill();
		delete mConnection;
		mConnection = NULL;
	}
	this->Stop();
	if(mServerSocket)
	{
		mServerSocket->Disconnect();
		delete mServerSocket;
		mServerSocket = NULL;
	}

	mStarted = false;
	BroadcastMessage(msg_NetworkStatusInfo,s_stopped);
}

int  WED_Server::IsReady()
{
	if (mServerSocket == NULL)   return 0;
	if (mServerSocket->GetStatus() == PCSBSocket::status_Error) return 0;
	if (mConnection == NULL)     return 0;
	if (!mConnection->IsAlive()) return 0;

	return (mConnection->HasClient);
}

int WED_Server::SendData(const char* hdr,int type,int id,const string& args )
{
	char buf[256];
	sprintf(buf,"%s:%d:%d:",hdr,type,id);
	string str(buf);
	str += args + "\r\n";
	return SendData(str.c_str(),str.size());
}

int WED_Server::SendData(const char * inBuf ,int inSize)
{
	if (mConnection) return mConnection->SendData(inBuf,inSize);
	return false;
}

int WED_Server::GetData(vector<string>& outData)
{
	if(mQueue.empty()) return 0;
	outData = mQueue.back();
	mQueue.pop_back();
	return 1;
}

void WED_Server::DoConnection(vector<string>& inList)
{
	if (inList.size() < 2) return;
	int type = atoi(inList[1].c_str());
	switch(type)
	{
		case nw_con_login :
		{
			if (inList.size() != 5) return;
			mConnection->rev  	= atoi(inList[4].c_str());
			mConnection->name   = inList[3];
			if(mConnection->name == CLIENT_NAME &&
				mConnection->rev >= MIN_CLIENT_VERS &&
				!mConnection->HasClient)
			{
				mConnection->ident  = atoi(inList[2].c_str());
				mConnection->HasClient = true;
				SendData(WED_NWP_CON,nw_con_go_on,mIdent,mPkgName);
				BroadcastMessage(msg_NetworkStatusInfo,s_changed);
			}
			else
				SendData(WED_NWP_CON,nw_con_refused,mIdent,"");
			break;
		}
		case nw_con_leave :
		{
			//not implemented
		}
	}
}

int WED_Server::DoParseMore(vector<char>& io_buf)
{
	vector<string> tokens;

	for (int n = 1; n < io_buf.size(); ++n)
	{
		if (io_buf[n-1] == '\r' && io_buf[n] == '\n')
		{
			//we have a line
			tokens.clear();
			vector<char>::iterator it = io_buf.begin();
			vector<char>::iterator eit = io_buf.begin() + n - 1;
			// break to tokens
			while (it < eit)
			{
				vector<char>::iterator s = it;
				while (s < eit && *s ==':')
					++s;
				vector<char>::iterator e = s;
				while (e < eit && *e !=':')
					++e;
				if (s < e) tokens.push_back(string(s,e));
				it = e;
			}
			if (!tokens.empty())
			{
				if (tokens[0] == WED_NWP_CON)
					DoConnection(tokens);
				else if (tokens[0] == WED_NWP_GET)
				{
					//FIXME:mroe nobody wants the data ,
					//		what now ?
					if (mQueue.size() < MAX_QUEUE_SIZE)
						mQueue.push_back(tokens);
				}
				else {;}
			}

			io_buf.erase(io_buf.begin(), io_buf.begin() + n + 1);
			n = 0;
		}
	}
	//no line found after max len ,must be garbage in the inbuffer ;simply delete all
	if(io_buf.size() > MAX_LINE_LEN) io_buf.clear();

	return io_buf.empty();
}


int  WED_Server::DoProcessing(void)
{
	if (mConnection == NULL)  return 0;

	//TODO:mroe something wrong with the serversocket we should manage this
	if (mServerSocket == NULL)  return 0;
	if (mServerSocket->GetStatus() == PCSBSocket::status_Error) return 0;

	int result = mConnection->DoProcessing();

	// 0 = error , 1 = done , 2 = incoming data ,3 = inBuf overflow
	if (!result)
	{
		delete mConnection;
		mConnection = NULL;
		BroadcastMessage(msg_NetworkStatusInfo,s_changed);
		return 0;
	}

	if (result > 1 ) DoParseMore(mConnection->mInBuf);

	return 1;
}

void WED_Server::TimerFired()
{
	//TODO:mroe something wrong with the serversocket we should manage this
	 if(mServerSocket == NULL) return;
	 PCSBSocket::Status constat = mServerSocket->GetStatus();
	 if (constat == PCSBSocket::status_Error) return;


 	 if(constat == PCSBSocket::status_Connecting )
	 {
	 	PCSBSocket * socket = mServerSocket->Accept();
	 	if (socket)
	 	{
			if((mConnection == NULL) )
			{
				mConnection = new WED_Connection(socket);
				//newline first because there could garbage in the inBuf on the otherside.
				char str[] = {"\r\nWED "WED_VERSION_STRING" , Hello !\r\n"};
				mConnection->SendData(str,sizeof(str)-1);
			}
			else
			{
				// only one connection is allowed
				socket->Disconnect();
				delete socket;
			}
	 	}
	 }
	 if (constat == PCSBSocket::status_Ready )
	 {
		if (!DoProcessing()) return;
		if (!mQueue.empty())BroadcastMessage(msg_NetworkStatusInfo,s_newdata);
		// check for pending signin
		if (mConnection->HasClient) return;
		//waittime out , a client has connected but has not signed within given time.
		if ( ++mConnection->mWaitCntr > MAX_WAIT_FORLOGIN )
		{
			char str[] = "\r\nwaittime out\r\n";
			mConnection->SendData(str,sizeof(str)-1);
			mConnection->DoProcessing();
			mConnection->Kill();
		}
	 }
}
#endif
