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
#ifndef _PCSBSocket_UDP_h_
#define _PCSBSocket_UDP_h_

#if _WIN32
	#include <Winsock2.h>
	#include <Ws2tcpip.h>
#else
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#endif

/* These longs are thrown from the constructor if we can't init. */

const long	udp_Exception_UnknownError = 1;
const long	udp_Exception_BindFailed = 2;

class	PCSBSocketUDP {
public:

	/* Construct a UDP socket around a port number.  The port has no connection
	 * status. */
	
					PCSBSocketUDP(unsigned short inPort);
	virtual			~PCSBSocketUDP();
		
	/* This method reads data if there is any.  It returns the number of 
	 * bytes read or -1 if no data could be read.  It fills out the IP and
	 * port of the incoming data.  It will not read more than the bufLength,
	 * but may return -1 if the buffer is too small, so the buffer should 
	 * always be bigger than the TSDU for UDP (which is like 512 bytes or something. */	
		
			long	ReadData(
							void * 				outBuf,
							long				outBufLength,
							unsigned long *		outSrcIP,
							unsigned short *	outSrcPort);
							
	/* This method writes data to the UDP socket and returns the number of 
	 * bytes written or -1 for an error.  Pass in the IP and port youare sending to. */
							
			long	WriteData(
							void *				inBuf,
							long				inBufLength,
							unsigned long		inDstIP,
							unsigned short		inDstPort);

private:

#if _WIN32
		SOCKET				mWinSocket;		//our socket
		SOCKADDR_IN			sIn;			//our address struct
public:
		ULONG				mBroadcastAddress;
#else		
		EndpointRef			mMacSocket;
		InetHost			mBroadcastAddress;
#endif		
							
};


#endif