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
/*
 * PLEASE DO NOT REDISTRIBUTE THIS FILE!!!!!!!!!!!!!!!!!!
 *
 * This file is (c) Copyright 2003 Ben Supnik and Chris Serio, all rights reserved.
 *
 * This file is donated to Austin Meyer for use in X-Plane projects; it originally
 * comes from XSquawkBox.  Please use it for X-Plane and X-Plane related code,
 * but please do not redistribute it.
 *
 */
#include "PCSBSocketUDP.h"

#define BLOCKING 0
#define NON_BLOCKING 1

/* Construct a UDP socket around a port number.  The port has no connection
* status. */
	
PCSBSocketUDP::PCSBSocketUDP(unsigned short inPort)
{
	int nResult;
	sIn.sin_family = AF_INET;
	sIn.sin_addr.S_un.S_addr = INADDR_ANY;					//Let windows pick a suitable addy
	sIn.sin_port = htons(inPort);							//Set the local port or if 0 let windows assign one

	mWinSocket = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);	//start the socket for UDP comms

	if(mWinSocket == INVALID_SOCKET)
	{
		WSAGetLastError();
		throw udp_Exception_UnknownError;
	}

	BOOL	can_broadcast=1;
	if(setsockopt(mWinSocket,SOL_SOCKET,SO_BROADCAST,(const char*)&can_broadcast,sizeof(can_broadcast))!=0)
	{
		int error = WSAGetLastError();
		// This is almost always because our port is in use.
		closesocket(mWinSocket);
		throw udp_Exception_BindFailed;
	}

	if (bind(mWinSocket, (SOCKADDR*)&sIn, sizeof(sIn)) != 0) //Bind the socket to the local addy
	{
		int error = WSAGetLastError();
		// This is almost always because our port is in use.
		closesocket(mWinSocket);
		throw udp_Exception_BindFailed;
	}

	unsigned long nSetSocketType = NON_BLOCKING;
	nResult = ioctlsocket(mWinSocket,FIONBIO,&nSetSocketType);	//set to non-blocking
	if(nResult == SOCKET_ERROR)
	{
		closesocket(mWinSocket);
		throw udp_Exception_UnknownError;
	}
	
	SOCKADDR_IN	addy;
	DWORD	ret;
	if (WSAIoctl(mWinSocket, SIO_GET_BROADCAST_ADDRESS, NULL, 0,
		&addy, sizeof(addy), &ret, NULL, NULL) == 0)
	{
		mBroadcastAddress = addy.sin_addr.s_addr;
	}	
}

PCSBSocketUDP::~PCSBSocketUDP()
{
	closesocket(mWinSocket);
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
	SOCKADDR_IN remoteAddress;
	int fromlen = sizeof(remoteAddress);
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = 0;
	remoteAddress.sin_addr.S_un.S_addr = 0;
	
	WSASetLastError(0);
	int result = recvfrom(mWinSocket, 
						 (char*)outBuf, 
						 outBufLength,
						 0,
						 (SOCKADDR*)&remoteAddress,
						 &fromlen);
		
	*outSrcIP = remoteAddress.sin_addr.S_un.S_addr;
	*outSrcPort = htons(remoteAddress.sin_port);
	
	if (result == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSAEWOULDBLOCK)
		{
			printf("UDP READ ERROR!\n");
			return -1;
		}
	}
	else
	{
		printf("UDP DATA WAS RECEIVED!\n");
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
	if (inDstIP = 0xFFFFFFFF)
		inDstIP = mBroadcastAddress;
	SOCKADDR_IN destAddress;
	int tolen = sizeof(destAddress);
	destAddress.sin_family = AF_INET;
	destAddress.sin_port = ntohs(inDstPort);
	destAddress.sin_addr.S_un.S_addr = ntohl(inDstIP);
	WSASetLastError(0);
	int result = sendto(mWinSocket, 
						 (char*)inBuf, 
						 inBufLength,
						 0,
						 (sockaddr*)&destAddress,
						 tolen);
	
	if (result == SOCKET_ERROR)
	{
		printf("UDP WRITE ERROR!\n");
		return -1;
	}
	else
	{
		printf("UDP DATA WAS SENT!\n");
		return result;
	}
}