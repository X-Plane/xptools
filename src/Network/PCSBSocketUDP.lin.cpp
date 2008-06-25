/* 
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
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

#include "PCSBSocketUDP.h"

#define BLOCKING 0
#define NON_BLOCKING 1

extern int errno;
/* Construct a UDP socket around a port number.  The port has no connection
* status. */
	
PCSBSocketUDP::PCSBSocketUDP(unsigned short inPort)
{
	int nResult;
	sIn.sin_family = AF_INET;
	sIn.sin_addr.s_addr=htonl(INADDR_ANY);					//Let linux pick a suitable addy
	sIn.sin_port = htons(inPort);							//Set the local port or if 0 let windows assign one

	mWinSocket = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);	//start the socket for UDP comms

	if(mWinSocket == -1)
	{
		throw udp_Exception_UnknownError;
	}

	if (bind(mWinSocket, (sockaddr*)&sIn, sizeof(sIn)) != 0) //Bind the socket to the local addy
	{
		// This is almost always because our port is in use.
		close(mWinSocket);
		throw udp_Exception_BindFailed;
	}

	nResult = fcntl(mWinSocket, F_SETFL, O_NONBLOCK); //set to non-blocking
	if(nResult != 0)
	{
		close(mWinSocket);
		throw udp_Exception_UnknownError;
	}
}
PCSBSocketUDP::~PCSBSocketUDP()
{
	close(mWinSocket);
}
		
/* This method reads data if there is any.  It returns the number of 
* bytes read or -1 if no data could be read.  It fills out the IP and
* port of the incoming data.  It will not read more than the bufLength,
* but may return -1 if the buffer is too small, so the buffer should 
* always be bigger than the TSDU for UDP (which is like 512 bytes or something. */	
		
long PCSBSocketUDP::ReadData(
				void * 				outBuf,
				long				outBufLength,
				unsigned long *		outSrcIP,
				unsigned short *	outSrcPort)
{
	sockaddr_in remoteAddress;
	int fromlen = sizeof(remoteAddress);
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = 0;
	remoteAddress.sin_addr.s_addr = 0;
	
	int result = recvfrom(mWinSocket, 
						 (char*)outBuf, 
						 outBufLength,
						 0,
						 (sockaddr*)&remoteAddress,
						 (socklen_t*)&fromlen);
		
	*outSrcIP = ntohl(remoteAddress.sin_addr.s_addr);
	*outSrcPort = ntohs(remoteAddress.sin_port);
	
	if (result == -1)
	{
		if(errno != EAGAIN)
		{
			return -1;
		}
	}
	else
	{
		return result;
	}
	return result;
}
							
/* This method writes data to the UDP socket and returns the number of 
* bytes written or -1 for an error.  Pass in the IP and port youare sending to. */
							
long PCSBSocketUDP::WriteData(
				void *				inBuf,
				long				inBufLength,
				unsigned long		inDstIP,
				unsigned short		inDstPort)
{
	sockaddr_in destAddress;
	int tolen = sizeof(destAddress);
	destAddress.sin_family = AF_INET;
	destAddress.sin_port = htons(inDstPort);
	destAddress.sin_addr.s_addr = htonl(inDstIP);
	int result = sendto(mWinSocket, 
						 (char*)inBuf, 
						 inBufLength,
						 0,
						 (sockaddr*)&destAddress,
						 tolen);
	
	if (result == -1)
	{
		return -1;
	}
	else
	{
		return result;
	}
}
