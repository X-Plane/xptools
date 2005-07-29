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

#include "PCSBSocket.h"
#include <stdio.h>
//#include "StringUtils.h"

#define MAJOR_VERSION_REQUIRED 2
#define MINOR_VERSION_REQUIRED 0
#define BLOCKING 0
#define NON_BLOCKING 1

//FILE * hexdump = fopen("hexdump.txt","w");
//extern FILE * hexdump;

/* Construct a new socket.  If the socket is to be a server listening port,
	 * Pass 0 to dynamically allocate a port or a positive number to pick a
	 * well-known one. */
PCSBSocket::PCSBSocket(unsigned short inPort, bool inServer)
{
	int nResult;
	int off = 0;
	int on =  1;
	socketStatus = status_Ready;
	mReceivedRelease = false;
	mSentRelease = false;
	mIsAServer = false;

	sIn.sin_family = AF_INET;
	sIn.sin_addr.S_un.S_addr = INADDR_ANY;					//Let windows pick a suitable addy
	sIn.sin_port = htons(inPort);							//Set the local port or if 0 let windows assign one

	mWinSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);	//start the socket for TCP/IP comms

	if(mWinSocket == INVALID_SOCKET)
	{
		socketStatus = status_Error;
		return;
	}
	if (setsockopt(mWinSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&on,
      sizeof(on))<0)
   {
	   printf("-----------------------Could not set TCP_NODELAY on port %d-------------\n",inPort);
   }
	if (bind(mWinSocket, (LPSOCKADDR)&sIn, sizeof(sIn)) != 0) //Bind the socket to the local addy
	{
		socketStatus = status_Error;
		return;
	}

	unsigned long nSetSocketType = NON_BLOCKING;
	nResult = ioctlsocket(mWinSocket,FIONBIO,&nSetSocketType);	//set to non-blocking

	//We're a server so start listening!
	if(inServer)
	{
		mIsAServer = true;
		if(listen(mWinSocket, SOMAXCONN))
		{
			printf("Could not enable listening!");
			socketStatus = status_Error;
			return;
		}
	}

}

PCSBSocket::PCSBSocket(SOCKET inWorkerSocket)
{
	mWinSocket = inWorkerSocket;
	socketStatus = status_Connected;
	mReceivedRelease = false;
	mSentRelease = false;
	mIsAServer = false;

	unsigned long nSetSocketType = NON_BLOCKING;
	int nResult = ioctlsocket(mWinSocket,FIONBIO,&nSetSocketType);	//set to non-blocking

}
/* Start up the networking DLL for the first time.  Returns true if
	the DLL is ok and sockets can be used. */
	
/* BAS - note: we ignore the init and shutdown DLL since we can recursively
 * open winsock. :-) */
bool PCSBSocket::StartupNetworking(bool inInitDLL)
{
	WORD wVersionRequired = MAKEWORD(MAJOR_VERSION_REQUIRED,MINOR_VERSION_REQUIRED);
	WSADATA wsaData;

	if(WSAStartup(wVersionRequired, &wsaData) != NO_ERROR)	//Startup the Winsock2 dll
		return false;

	if (wsaData.wVersion != wVersionRequired)				//Check for version
		return false;

	return true;

}
/* Tear down the network.  Used when the network is all done and ready
	  to be closed down. */
void PCSBSocket::ShutdownNetworking(bool inCloseDLL)
{
	WSACleanup();
}
/* Close the socket.  you will be forcefully disconnected if necessary. */

PCSBSocket::~PCSBSocket()
{
	closesocket(mWinSocket);
}

PCSBSocket * PCSBSocket::Accept()
{
	printf("Socket was accepted!\n");
	SOCKET workerSocket = accept(mWinSocket,NULL,NULL);
	socketStatus = status_Ready;
	if(workerSocket == SOCKET_ERROR)
			return NULL;	// No one called.

	return new PCSBSocket(workerSocket);	// Return a new socket object.
}

/* Get the socket's internal status.  */

PCSBSocket::Status PCSBSocket::GetStatus(void)
{
	TIMEVAL timeout;
	timeout.tv_sec = 0;				//no timeout!
	timeout.tv_usec = 0;			//no timeout!
	if ((socketStatus == status_Ready) && mIsAServer)
	{
		fd_set socketSetRead;
		FD_ZERO(&socketSetRead);
		FD_SET(mWinSocket,&socketSetRead);
		int nReturn = select(NULL,&socketSetRead,NULL,NULL,&timeout);
		if (nReturn > 0)
		{
			if(FD_ISSET(mWinSocket,&socketSetRead))	//Able to read on it?
				{
					socketStatus = status_Connecting;
					return socketStatus;
				}
				else
				{
					socketStatus = status_Disconnected;
					return socketStatus;
				}
		}
		else if(nReturn == 0)		//Didn't have enough time to complete request fully (non-blocking)
		{
		}
		else	//Must be a socket error :(
		{
			socketStatus = status_Error;
		}
	}
	if (socketStatus == status_Connecting)
	{
		fd_set socketSetWrite,socketSetError;
		FD_ZERO(&socketSetWrite);
		FD_ZERO(&socketSetError);
		FD_SET(mWinSocket,&socketSetWrite);		//macro to form set
		FD_SET(mWinSocket,&socketSetError);
		int nReturn = select(NULL,NULL,&socketSetWrite,&socketSetError,&timeout);
		// Check to see if our connection completed.
		if (nReturn > 0)
		{
			if((FD_ISSET(mWinSocket,&socketSetWrite)) && !(FD_ISSET(mWinSocket,&socketSetError)))
			{	//If the socket is ready to write with no errors
				socketStatus = status_Connected;
				return socketStatus;
			}
			else
			{
				socketStatus = status_Disconnected;
				return socketStatus;
			}
		}
		else if(nReturn == 0)		//Didn't have enough time to complete request fully (non-blocking)
		{
		}
		else	//Must be a socket error :(
		{
			socketStatus = status_Error;
		}

	}
	return socketStatus;
}

/* Whether an orderly release has been received.  Note that it may
       not be possible to tell if one has been issued without first reading
       buffered data. */

bool PCSBSocket::ReceivedRelease(void)
{
	return mReceivedRelease;
}

/* Connect to a server at a port and IP as a client. */

void PCSBSocket::Connect(unsigned long inIP, unsigned short inPort)
{
	socketStatus = status_Connecting;
	SOCKADDR_IN sOut;
	int nResult;

	sOut.sin_family = AF_INET;
	sOut.sin_port = htons(inPort);			//set the remote connection port
	sOut.sin_addr.S_un.S_addr = inIP;		//set the remote IP to connect to
	nResult = connect(mWinSocket,(LPSOCKADDR)&sOut,sizeof(sOut));	//connect
	if(nResult == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSAEWOULDBLOCK)		//If it's a mere blocking error, check if connection completed by using select()
		{
			socketStatus = status_Error;
		}
	}
	else
		socketStatus = status_Connected;	//If it didn't have an error to begin with
}

/* Read as much data from the socket as you want and is possible.
       Positive numbers indicate a read, 0 indicates that no data is
       available (either due to a lack of data sent or an orderly release).
       A negative number indicates an error, either an unknown error or
       a disconnect.  To further diagnose 0 or -1 responses, use
       GetStatus() and ReceivedRelease(). */

long PCSBSocket::ReadData(void* outBuf,long inLength)
{
	int nReturn;
	nReturn = recv(mWinSocket,(char*)outBuf,inLength,NULL);
	if(nReturn == SOCKET_ERROR)
	{
		nReturn = WSAGetLastError();
		if(nReturn == WSAEWOULDBLOCK)
		{
			//printf("No Data on Socket yet!\n");
			return 0;	// ** Signal no data
		}
		else if((nReturn == WSAECONNRESET)||(nReturn == WSAENOTCONN)||(nReturn == WSAETIMEDOUT)||(nReturn == WSAECONNABORTED))
		{
			socketStatus = status_Disconnected;
			return -1;
		}
		else
		{
			socketStatus = status_Error;	// signal a real error.
			return -1;	//
		}
	}
	else if(nReturn == 0)
	{
		mReceivedRelease = true;
		if(mSentRelease)
		{
			socketStatus = status_Disconnected;
			return -1;
		}
		else
			return 0;
	}
	else
	{
		//printf("%s\n",(char*)outBuf);
		return nReturn;
	}

}

/* Write as much data to the socket as you want and is possible.  Positive
       numbers indicate a write, zero indicates that no data was written due to
       flow control.  -1 indicates that an error occured, either a socket closure
       or otherwise. */

long PCSBSocket::WriteData(const void* inBuf,long inLength)
{
	int nReturn;
	
	//fprintf(hexdump,"\n");
	//DumpHex(hexdump,(char*)inBuf,inLength);
	//fflush(hexdump);
	
	nReturn = send(mWinSocket,(char*)inBuf,inLength,NULL);
	if(nReturn == SOCKET_ERROR)
	{
		nReturn = WSAGetLastError();
		if(nReturn == WSAEWOULDBLOCK)
		{
			return 0;	// ** Signal no data
		}
		else if((nReturn == WSAECONNRESET)||  //These errors are okay. They signal the obvious.
				(nReturn == WSAENOTCONN)||
				(nReturn == WSAETIMEDOUT)||
				(nReturn == WSAECONNABORTED)||
				(nReturn == WSAESHUTDOWN))
		{
			socketStatus = status_Disconnected;
			return -1;
		}
		else
		{
			socketStatus = status_Error;	// signal a real error.
			return -1;
		}
	}
	else
		return nReturn;
}

/* Disconnect the socket now.  This will be a disorderly disconnect */

void PCSBSocket::Disconnect(void)
{
	shutdown(mWinSocket,SD_SEND|SD_RECEIVE);
	socketStatus = status_Disconnected;
}

/* Indicate that you have no more data to send.  The socket will be
     * closed after the other side finishes sending data.  You can still
     * call disconnect to force the issue. */

void PCSBSocket::Release(void)
{
	if(shutdown(mWinSocket,SD_SEND)==SOCKET_ERROR)
		socketStatus = status_Error;
	mSentRelease = true;
}

/* Get the local IP address of the machine and port we are bound to.
     * Also return the remote side port and IP, or 0 if we're not connected.
     * Pass NULL for anything you don't want. */

void PCSBSocket::GetAddresses(ConnectionData* dataStruct)
{
	int nReturn;
	SOCKADDR_IN tempStruct;
	int sizeStruct = sizeof(tempStruct);
	/***********************Remote Addresses****************************/
	nReturn = getpeername(mWinSocket,(LPSOCKADDR)&tempStruct,&sizeStruct);
	dataStruct->remoteIP = tempStruct.sin_addr.S_un.S_addr;
	dataStruct->remotePort = ntohs(tempStruct.sin_port);
	/***********************Local Addresses****************************/
	nReturn = getsockname(mWinSocket,(LPSOCKADDR)&tempStruct,&sizeStruct);
	dataStruct->localIP = ntohl(tempStruct.sin_addr.S_un.S_addr);
	dataStruct->localPort = ntohs(tempStruct.sin_port);

}
/**********************************************************************
* DNS Access
**********************************************************************/

/* Looks up an IP address (dot-number form or DNS form. */

unsigned long PCSBSocket::LookupAddress(const char* inAddress)
{
	HOSTENT* host;
	struct in_addr *ipAddress;
	unsigned long ip;
	
	//try it as an IP first
	ip = inet_addr(inAddress);
	if (ip != INADDR_NONE)
		return ip;

	host = gethostbyname(inAddress);
	if(host == NULL)
		return 0;
	ipAddress = (struct in_addr *)(host->h_addr);
	if (!ipAddress)
		return 0;
	return ipAddress->s_addr;
}
