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

#ifndef WED_SERVER_H
#define WED_SERVER_H

#include "GUI_Timer.h"
#include "GUI_Broadcaster.h"

class PCSBSocket;
class WED_Connection;

class	WED_Server : public GUI_Broadcaster ,public GUI_Timer {

public:

				WED_Server(const string& inPkgName,unsigned short inPort);
			virtual	~WED_Server();

				int		IsReady();
				int 	DoStart();
				int 	IsStarted(){return  mStarted;}
				void 	DoStop();

				void	SetPort(unsigned short inPort);
	unsigned short		GetPort(){return  mPort;}

				int 	DoProcessing(void);
				int 	SendData(const char * inBuffer ,int inSize);
				int		SendData(const char* hdr,int type,int id,const string& args);
				int	 	GetData(vector<string>& outData);

				enum	server_msg_kind {

						s_error   = 0,
						s_started    ,
						s_stopped    ,
						s_changed    ,
						s_newdata    ,
				};

private:

				void	TimerFired();

				int 	DoParseMore(vector<char>& io_buf);
				void	DoConnection(vector<string>& inList);

				int		mStarted;
	unsigned long		mIdent;
	unsigned short		mPort;
	string              mPkgName;

	PCSBSocket * 		mServerSocket;
	WED_Connection * 	mConnection;
	vector<vector<string> >		mQueue;

};

#endif // WED_SERVER_H
