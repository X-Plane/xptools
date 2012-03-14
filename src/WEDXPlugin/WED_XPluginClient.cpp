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


#include "WED_NWDefs.h"
#include "PCSBSocket.h"
#include "WED_XPluginMgr.h"
#include "WED_XPluginClient.h"
#include "XPLMProcessing.h"
#include <stdio.h>

#define DEFAULT_IPAddr 		"localhost"
#define DEFAULT_Port		10300
#define CLIENT_VERS			"100"

WED_XPluginClient::WED_XPluginClient():
	mIsReady(false),mSocket(NULL)

{
    mStatus = "disconnected";
	PCSBSocket::StartupNetworking(true);
	mIdent  = (unsigned long) this;
	mIPAddr = PCSBSocket::LookupAddress(DEFAULT_IPAddr);
	mPort   = DEFAULT_Port;
	mMgr = new WED_XPluginMgr(this);
}

WED_XPluginClient::~WED_XPluginClient()
{
	Disconnect();
	delete mMgr;
	PCSBSocket::ShutdownNetworking(true);
}

float WED_XPluginClient::WEDXPluginLoopCB(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
	WED_XPluginClient * client = static_cast<WED_XPluginClient *>(refcon);
	if(client->DoProcessing()) return -1;
	return 1.0;
}

void WED_XPluginClient::Connect()
{
	mSocket = new PCSBSocket(0, false);
	XPLMRegisterFlightLoopCallback(WEDXPluginLoopCB,1.0, this);
	mStatus = "connecting";
}

void  WED_XPluginClient::Disconnect()
{
	XPLMUnregisterFlightLoopCallback(WEDXPluginLoopCB, this);
	if(mSocket)
	{
		mSocket->Disconnect();
		delete mSocket;
		mSocket = NULL;
	}
	mOutBuf.clear();
	mInBuf.clear();
	mIsReady = false;
	mMgr->SetPackage("unknown");
	mStatus = "disconnected";
}

int WED_XPluginClient::SendData(const char* hdr,int type,int id,const string& args )
{
	char buf[256];
	sprintf(buf,"%s:%d:%d:",hdr,type,id);
	string str(buf);
	str += args + "\r\n";
	SendData(str.c_str(),str.size());
	return 1;
}

int WED_XPluginClient::SendData(const char * inBuf ,int inSize)
{
	//FIXME:mroe outbuffer overflow ,simple do not write-in anymore
	//           data lost !
	if (mOutBuf.size() > MAX_BUF_SIZE) return false;
	 mOutBuf.insert(mOutBuf.end(),inBuf,inBuf+inSize);
	return true ;
}


int WED_XPluginClient::DoConnection()
{
	mStatus = "try connect";
	if (mSocket->GetStatus() == PCSBSocket::status_Error ||
		mSocket->GetStatus() == PCSBSocket::status_Disconnected)
	{
		delete mSocket;
		mSocket = new PCSBSocket(0, false);
	}

	if( mSocket->GetStatus() == PCSBSocket::status_Ready )
	{
		mSocket->Connect(mIPAddr,mPort);
		return 1;
	}

	return 0;
}

int  WED_XPluginClient::DoProcessing()
{

	if ( mSocket == NULL || mSocket->GetStatus() != PCSBSocket::status_Connected)
	{
		mIsReady = false;
		return DoConnection();
	}

	if(mSocket->ReceivedRelease())
	{
		mSocket->Release();
	}

	if (!mOutBuf.empty())
	{
		int	writeLen = mSocket->WriteData(&*mOutBuf.begin(), mOutBuf.size());
		if (writeLen > 0)
		mOutBuf.erase(mOutBuf.begin(), mOutBuf.begin() + writeLen);
	}

	if (mInBuf.size() > MAX_BUF_SIZE) return 3;

	char readChunk[1024];
	int	readLen = mSocket->ReadData(readChunk, sizeof(readChunk));
	if (readLen > 0)
		mInBuf.insert(mInBuf.end(), readChunk, readChunk + readLen);

	if (!mInBuf.empty()) DoParseMore();

	return 1;
}

void WED_XPluginClient::DoParseMore()
{
	int id = 0;
	int type = 0;
	vector<string> tokens;
	for (unsigned int n = 1; n < mInBuf.size(); ++n)
	{
		if (mInBuf[n-1] == '\r' && mInBuf[n] == '\n')
		{
			//we have a line
			tokens.clear();
			vector<char>::iterator it = mInBuf.begin();
			vector<char>::iterator eit = mInBuf.begin() + n - 1;
			// break to tokens
			while (it < eit)
			{
				vector<char>::iterator s = it;
				while (s < eit && *s ==':')
					++s;
				vector<char>::iterator e = s;
				while (e < eit && *e !=':')
					++e;
				if (s < e)
				{
					tokens.push_back(string(s,e));
				}
				it = e;
			}
			if (!tokens.empty())
			{
				if(!mIsReady)
				{
					if(tokens[0].find("WED") != string::npos)//TODO:mroe check WED vers here ?
					{
						char str[] = { CLIENT_NAME ":" CLIENT_VERS ":"};
						SendData(WED_NWP_CON,nw_con_login,mIdent,str);
						mStatus = "connected";
					}
					else
					if(tokens[0] == WED_NWP_CON && tokens.size() == 4)
					{
						if( sscanf(tokens[1].c_str(),"%d",&type) == 1 && type == nw_con_go_on)
						{
							mIsReady = true;
							mStatus  = "ready " + tokens[3];
							mMgr->SetPackage(tokens[3]);
							mMgr->Sync();
						}
					}
				}
				else if (tokens.size() >= 3)
				{
					sscanf(tokens[1].c_str(),"%d",&type);
					sscanf(tokens[2].c_str(),"%d",&id);

					if 		 (tokens[0] == WED_NWP_CMD) {;}
					else if (tokens[0] == WED_NWP_DEL) mMgr->Del(id);
					else if (tokens[0] == WED_NWP_ADD) mMgr->Add(id,type,tokens);
					else if (tokens[0] == WED_NWP_CHG) mMgr->Chg(id,type,tokens);
				}
			}

			mInBuf.erase(mInBuf.begin(), mInBuf.begin() + n + 1);
			n = 0;
		}
	}
	//no line found after max len ,must be garbage in the inbuffer ;simply delete all
	if(mInBuf.size() > MAX_LINE_LEN) mInBuf.clear();
}
