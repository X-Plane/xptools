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
#ifndef _PCSBSocket_h_
#define _PCSBSocket_h_



#if IBM
        #include <Winsock2.h>
        #include <Ws2tcpip.h>
#elif 0
	#include <Carbon/Carbon.h>
//	#include <OpenTransport.h>
//	#include <OpenTransportProviders.h>
#elif LIN || APL
	#include <sys/socket.h>
	#include <netinet/in.h>
#else
	#error YOU MUST DEFINE A PLATFORM!
#endif


class	PCSBSocket {
public:

	/* This enum defines a socket status.  A socket starts out ready to connect,
	 * connects asynchronously, is connected for a while, then is disconnected.
	 * It may also go to disconnected without ever becoming connected if the
	 * server refuses us.
	 *
	 * If an error occurs, we end up in the error state and do not recover.  Errors
	 * typically are fatal things like resource contention or API errors.
	 *
	 * Also, once disconnected, we cannot be reconnected again; destroy the socket
	 * and create a new one. */
         enum Status {

	       /* The socket is instantiated, bound, and ready	to connect. */
	       status_Ready = 1,

	       /* The socket is attempting to connect, but has not yet.  Or
		* an incoming connection has come in. */
	       status_Connecting = 2,

	       /* The socket is connected and ready to send data. */
	       status_Connected = 3,

	       /* The other side is disconnected, either because the connection
		failed, the connection was disconnected in a disorderly manner
		from either side, or because both sides issued an orderly
		disconnect. */
	       status_Disconnected = 4,

	       /* An unknown error has happened; the socket is hosed. */
	       status_Error = 5

        };


	/* ConnectionData
	 *
	 * this structure contains the local and remote IP/port pairs for our
	 * socket.  Remote ones will be zero if we are not connected. */

	struct ConnectionData {
		unsigned long	localIP;
		unsigned short	localPort;
		unsigned long	remoteIP;
		unsigned short	remotePort;
	};

	/**********************************************************************
	 * SOCKET API
	 **********************************************************************/

	/* Start up the networking DLL for the first time.  Returns true if
	 * the DLL is ok and sockets can be used.  Pass true in for inInitDLL
	 * to initialize the networking DLL, false to not init it.
	 * this function! */

	static	bool			        StartupNetworking(bool inInitDLL);

	/* Tear down the network.  Used when the network is all done and ready
	 * to be closed down.  You specify whether you want the networking DLL
	 * (open transport, etc.) to be closed down. */

	static	void			        ShutdownNetworking(bool inCloseDLL);

	/* Construct a new socket.  If the socket is to be a server listening port,
	 * Pass 0 to dynamically allocate a port or a positive number to pick a
	 * well-known one. */

					        PCSBSocket(unsigned short inPort, bool inServer);

	/* Close the socket.  you will be forcefully disconnected if necessary. */

	virtual					~PCSBSocket();

	/* Get the socket's internal status.  */

	                Status		        GetStatus(void);

	/* Whether an orderly release has been received.  Note that it may
		not be possible to tell if one has been issued without first reading
		buffered data. */

			bool			ReceivedRelease(void);

	/* Connect to a server at a port and IP as a client. */

			void			Connect(unsigned long inIP, unsigned short inPort);

	/* Receive an incoming connection. */

			PCSBSocket *	        Accept(void);

	/* Read as much data from the socket as you want and is possible.
		Positive numbers indicate a read, 0 indicates that no data is
		available (either due to a lack of data sent or an orderly release).
		A negative number indicates an error, either an unknown error or
		a disconnect.  To further diagnose 0 or -1 responses, use
		GetStatus() and ReceivedRelease(). */

			long			ReadData(
								void * 			outBuf,
								long 			inLength);

	/* Write as much data to the socket as you want and is possible.  Positive
		numbers indicate a write, zero indicates that no data was written due to
		flow control.  -1 indicates that an error occured, either a socket closure
		or otherwise. */

			long			WriteData(
								const void * 	inBuf,
								long 			inLength);

	/* Disconnect the socket now.  This will be a disorderly disconnect */

			void			Disconnect(void);

	/* Indicate that you have no more data to send.  The socket will be
	 * closed after the other side finishes sending data.  You can still
	 * call disconnect to force the issue. */

			void			Release(void);

	/* Get the local IP address of the machine and port we are bound to.
	 * Also return the remote side port and IP, or 0 if we're not connected.
	 * Pass NULL for anything you don't want. */

	 		void			GetAddresses(
	 							ConnectionData *	outData);

	/**********************************************************************
	 * DNS Access
	 **********************************************************************/

	 /* Looks up an IP address (dot-number form or DNS form. */

	 static unsigned long	LookupAddress(
	 							const char *	inAddress);

	/* This function traverses an array of PCSBSockets and returns the
	* number of sockets that have I/O that needs to be done. The function
	* also sets any array element to 0 that does not need work done to it.
	* Therefore, after the function returns normally, the only array elements
	* that are non-null are ones that need to have work done. The function
	* shall return a -1 if the timeout has elapsed and -2 if there was a socket
	* error.*/
	 static int 			WaitForSockets(
		 						PCSBSocket ** sockets,
		 						int count,
		 						long timeout);

private:

#if IBM
		SOCKET		mWinSocket; //our socket
		SOCKADDR_IN	sIn; //our address struct

		ConnectionData 	dataStruct;
		Status		socketStatus;
		bool		mReceivedRelease;
		bool		mSentRelease;
		bool		mIsAServer;
				PCSBSocket(SOCKET inWorkerSocket);
#elif 0
	/* Mac-Specific private instance variables: */
		void		MopupEvents(void);
				PCSBSocket(EndpointRef inSocket);

		EndpointRef	mMacSocket;

		unsigned long	mLocalIP;
		unsigned short	mLocalPort;
		unsigned long	mRemoteIP;
		unsigned short	mRemotePort;

		bool		mDone;
		bool		mErr;
		TCall		mIncoming;
		InetAddress	mIncomingIP;
		bool		mHasIncoming;
		EndpointRef	mWorker;
#else  //Linux
		int		mLinSocket;
	struct	sockaddr_in	sIn;
		ConnectionData	dataStruct;
		Status		socketStatus;
		bool		mReceivedRelease;
		bool		mSentRelease;
		bool		mIsAServer;
				PCSBSocket(int inWorkerSocket);

#endif

	/* Prohibited C++ auto-geneerated ctors and operators: */
				PCSBSocket();
				PCSBSocket(const PCSBSocket&);
				PCSBSocket& operator=(const PCSBSocket&);

};

#endif /*INCLUDE GUARD*/
