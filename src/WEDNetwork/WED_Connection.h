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

#ifndef WED_CONNECTION_H
#define WED_CONNECTION_H

#include "PCSBSocket.h"

class	WED_Server;

class	WED_Connection {
public:

					WED_Connection(PCSBSocket* inSocket);
	virtual		~WED_Connection();

		void		Kill(void);
		bool		IsAlive(void);
		int   		DoProcessing(void);
		int 		SendData( const char * inBuf, int inSize);
	    void		GetData(vector<char>& io_buf);


private:

	friend class 	WED_Server;

		int			mWaitCntr;
		bool		HasClient;

		int			ident;
		int			rev;
		string		name;

	vector<char>	mOutBuf;			// Outgoing buffer
	vector<char>	mInBuf;				// Incoming buffer

	PCSBSocket *	mSocket;

};


#endif // WED_CONNECTION_H
