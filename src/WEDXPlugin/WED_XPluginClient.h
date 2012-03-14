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

#ifndef WED_XPLUGINCLIENT_H
#define WED_XPLUGINCLIENT_H

#include <vector>

class PCSBSocket;
class WED_XPLuginMgr;

class	WED_XPluginClient  {

public:

				WED_XPluginClient();
			virtual	~WED_XPluginClient();

				void 	Connect();
				void 	Disconnect();
				int 	IsReady(){return mIsReady;}
		  		string	GetStatus(){return mStatus;}

				void	SetPort(unsigned short inPort){mPort=inPort;}
				void	SetAddress(const char * inIPAddr);

				int		DoConnection();
				int 	DoProcessing();
				int 	SendData(const char * inBuffer ,int inSize);
				int		SendData(const char* hdr,int type,int id,const string& args);


private:

	WED_XPluginMgr *	mMgr;
				void	DoParseMore();
				string 	mStatus;
				int		mIsReady;

	unsigned long		mIdent;
	unsigned short	mPort;
	unsigned long		mIPAddr;

			int			ident;

		vector<char>	mOutBuf;			// Outgoing buffer
		vector<char>	mInBuf;				// Incoming buffer

		PCSBSocket * 	mSocket;

static float WEDXPluginLoopCB(float elapsedMe, float elapsedSim, int counter, void * refcon);

};

#endif // WED_XPLUGINCLIENT_H
