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


#include "PCSBSocket.h"
#include <Carbon/Carbon.h>
//#include <OpenTransport.h>
//#include <OpenTransportProviders.h>

// Use this to dump printfs out for the very low level open transport code!
#define	LOG_MAC_SOCKETS	0
#define	DEBUG_IN_XPLANE	0

#if LOG_MAC_SOCKETS
	#if DEBUG_IN_XPLANE
		#include "XSBDebug.h"
	#else
		#define	dprintf	printf
		#include <stdio.h>
	#endif
#endif

static	InetSvcRef		sSvcRef; 
	
bool PCSBSocket::StartupNetworking(bool inInitDLL)
{
		OSStatus 		result;
	
	if (inInitDLL)
	{	
		result = ::InitOpenTransportInContext(kInitOTForApplicationMask, NULL);
		if (result != noErr)
			return false;
	}
			
	sSvcRef = ::OTOpenInternetServicesInContext(kDefaultInternetServicesPath, 0, &result, NULL);
	if (result != noErr)
		return false;
	
	return true;
}

void PCSBSocket::ShutdownNetworking(bool inCloseDLL)
{
	::OTCloseProvider(sSvcRef);
	if (inCloseDLL)
		::CloseOpenTransportInContext(NULL);
}

PCSBSocket::PCSBSocket(
				unsigned short 	inPort, bool inServer) :
	mRemoteIP(0), mRemotePort(0),
	mLocalIP(0), mLocalPort(0),
	mMacSocket(NULL),
	mDone(false),
	mErr(false)
{
		OSStatus	err;

	mIncoming.addr.buf = (unsigned char *) &mIncomingIP;
	mIncoming.addr.len = mIncoming.addr.maxlen = sizeof(mIncomingIP);
	mIncoming.udata.buf = mIncoming.opt.buf = nil;
	mIncoming.udata.maxlen = mIncoming.udata.len = mIncoming.opt.len = mIncoming.opt.maxlen = 0;
	mIncomingIP.fAddressType = AF_INET;
	mIncomingIP.fPort = 0;
	mIncomingIP.fHost = 0;
	mIncoming.sequence = 0;
	mHasIncoming = false;
			
	mWorker = NULL;			
	mMacSocket = ::OTOpenEndpointInContext(
			::OTCreateConfiguration(kTCPName),
			0, 					// Options must be 0
			NULL, 				// Don't care about endpoint info
			&err,
			NULL);
	if (err != noErr)	throw err;
	
	// If we're a server, reuse the port address...
	
		TOptMgmt    optreq;
		TOption*	Opt;
		UInt8 		OptionBuf[kOTFourByteOptionSize];
	Opt = (TOption*)OptionBuf;
	optreq.opt.buf = OptionBuf;
	optreq.opt.len = sizeof(OptionBuf);
	optreq.opt.maxlen = sizeof(OptionBuf);
	optreq.flags   = T_NEGOTIATE;

	Opt->level  = INET_IP;
	Opt->name   = kIP_REUSEADDR;
	Opt->len    = kOTFourByteOptionSize;
	*(UInt32*)Opt->value = true;

	::OTOptionManagement(mMacSocket, &optreq, &optreq);	
	
	// bind to a port immediately.
	
		InetAddress		localAddress, realAddress;
		TBind			req, ret;	

	localAddress.fAddressType = AF_INET;
	localAddress.fPort = inPort;
	localAddress.fHost = kOTAnyInetAddress;
	
	req.addr.maxlen = req.addr.len = sizeof(localAddress);
	req.addr.buf = (unsigned char *) &localAddress;
	req.qlen = inServer ? 1 : 0;

	ret.addr.maxlen = sizeof(realAddress);
	ret.addr.buf = (unsigned char *) &realAddress;
		
	err = ::OTBind(mMacSocket, &req, &ret);
	if (err != noErr)
	{
		OTCloseProvider(mMacSocket);
		throw err;
	}
	
	mRemoteIP = realAddress.fHost;
	mRemotePort = realAddress.fPort;

	// Now that we are bound, go to async I/O and we will poll.

	::OTSetAsynchronous(mMacSocket);
	
}				
								
PCSBSocket::~PCSBSocket()
{
	if (mWorker)
		::OTCloseProvider(mWorker);
	::OTCloseProvider(mMacSocket);
}

PCSBSocket::Status			PCSBSocket::GetStatus(void)
{
	if (mErr) return status_Error;
	
	MopupEvents();
	switch(::OTGetEndpointState(mMacSocket)) {
	case T_UNINIT:
	case T_UNBND:
		return status_Error;	// Why didn't this already work?
	case T_IDLE:
		return mDone ? status_Disconnected : status_Ready;
	case T_OUTCON:
	case T_INCON:
		return status_Connecting;
	case T_DATAXFER:
	case T_OUTREL:
	case T_INREL:
		return status_Connected;
	default:
		return status_Error;
	}
}
			
bool			PCSBSocket::ReceivedRelease(void)
{
	return (::OTGetEndpointState(mMacSocket) == T_INREL);
	
}

long			PCSBSocket::ReadData(
								void * 			outBuf, 
								long 			inLength)
{
	OTResult	result;
	OTFlags		flags;
	
	while (1)
	{	
		result = ::OTGetEndpointState(mMacSocket);
		
		if (result == T_INREL)
			return 0;
		if ((result != T_DATAXFER) && (result != T_OUTREL))
			return -1;
		
		result = ::OTRcv(mMacSocket, outBuf, inLength, &flags);
		
		if (result > 0)
		{
			// We got data.  If we didn't get enough, we can clean up the data flags.
			return result;
			
		} else switch(result) {
		
		case kOTLookErr:
			// An event came up.  We'll mop it up and retry; if our state changes,
			// we pick it up at the top of the loop.
			MopupEvents();
			break;
		case kOTNoDataErr:
			// No data, return 0...
			return 0;
		default:
			// Not sure what happened, simply return an error and flag our state.
			mErr = true;
			return -1;
		}
	}							
}
								
long			PCSBSocket::WriteData(
								const void * 	inBuf, 
								long 			inLength)
{
	OTResult	result;
	OTFlags		flags = 0;
	
	while (1)
	{
		// Now check out the event state.  If we are in a bad state,
		// return -1.  Note that unlike reading, if you try to write 
		// after issueing a release, you get a -1 error since you
		// were a dork.
		result = ::OTGetEndpointState(mMacSocket);
		if ((result != T_INREL) && (result != T_DATAXFER))
			return -1;
	
		// I am 99.9% sure that OTSnd really doesn't @#$@# with the data...
		result = ::OTSnd(mMacSocket, const_cast<void *>(inBuf), inLength, flags);
		if (result >= 0)
		{
			return result;
		} else switch(result) {
		case kOTLookErr:
			// If something came up, process it; we'll change state later.
			MopupEvents();
			break;
		case kOTFlowErr:
			// Flow control, return nothing written.
			return 0;
		default:
			// Unknown error.
			mErr = true;
			return -1;
		}
	}								
}

void	PCSBSocket::Disconnect(void)
{
	// Only issue disconnects once!
	if (mDone)	return;

	mDone = true;
	mRemotePort = 0;
	mRemoteIP = 0;
	
	while (1)
	{
		OTResult	state = ::OTGetEndpointState(mMacSocket);
		if ((state != T_OUTCON) && (state != T_DATAXFER) && 
			(state != T_INREL) && (state != T_OUTREL))
		{
			// We're not even remotely connected, just bail.
			return;
		}

		OSStatus err = ::OTSndDisconnect(mMacSocket, NULL/*our socket*/);
		switch(err) {
		case kOTLookErr:
			// Maybe the disconnect was already sent?
			MopupEvents();
			break;
		case noErr:
			return;
		default:
			mErr = true;
			return;
		}
	}
}
			
void			PCSBSocket::Release(void)
{
	while (1)
	{
		OTResult	state = ::OTGetEndpointState(mMacSocket);
		if ((state != T_DATAXFER) && (state != T_INREL))
		{
			// We're not even remotely connected, just bail.
			return;
		}

		OSStatus err = ::OTSndOrderlyDisconnect(mMacSocket);
		switch(err) {
		case kOTLookErr:
			// Maybe the disconnect was already sent?
			MopupEvents();
			break;
		case noErr:
			return;
		default:
			mErr = true;
			return;
		}
	}
}

void			PCSBSocket::GetAddresses(
	 							ConnectionData*	outInfo)
{
	if (outInfo == NULL)
		return;
	
	outInfo->localIP = mLocalIP;
	outInfo->remoteIP = mRemoteIP;
	outInfo->localPort = mLocalPort;
	outInfo->remotePort = mRemotePort;
}	

#pragma mark -

void			PCSBSocket::Connect( 
					unsigned long 	inIP, 
					unsigned short 	inPort)
{
	if (mDone)
	{
		mErr = true;
		return;
	}
	
	mRemoteIP = inIP;
	mRemotePort = inPort;
	
	InetAddress		serverAddr;
	TCall			remoteCall;
	
	serverAddr.fAddressType = AF_INET;
	serverAddr.fPort = inPort;
	serverAddr.fHost = inIP;
	
	remoteCall.addr.buf = (unsigned char *) &serverAddr;
	remoteCall.addr.maxlen = remoteCall.addr.len = sizeof(serverAddr);

	remoteCall.udata.buf = remoteCall.opt.buf = nil;
	remoteCall.udata.len = remoteCall.opt.len = 0;
	remoteCall.udata.maxlen = remoteCall.opt.maxlen = 0;
	remoteCall.sequence = 0;

	// OPEN ISSUE: How do we clear connect here?!?
	
	while (1)
	{
		OTResult	status = ::OTGetEndpointState(mMacSocket);
		if (status != T_IDLE)
		{
			if (status != T_OUTCON)
				mErr = true;
			return;
		}

#if LOG_MAC_SOCKETS
		dprintf("Connecting.\n");
#endif			

		OSStatus	err = ::OTConnect(mMacSocket, &remoteCall, NULL/* what we really get*/);
		switch(err) {
		case kOTNoDataErr:
		case noErr:
#if LOG_MAC_SOCKETS
		dprintf("Connect is async, returning.\n");
#endif			
		
			return;
		case kOTLookErr:			
#if LOG_MAC_SOCKETS
			dprintf("Clearing look events.\n");
#endif			
			MopupEvents();
			break;
		default:
			mErr = true;
			return;
		}
	}
}					
			
PCSBSocket * PCSBSocket::Accept(void)
{
	OSStatus	err = noErr;
	if (mWorker == NULL)
		mWorker = ::OTOpenEndpointInContext(
									::OTCreateConfiguration(kTCPName),
									0, 					// Options must be 0
									NULL, 				// Don't care about endpoint info
									&err,
									NULL);
	if (err != noErr)	
		return NULL;

	while (1)
	{
		MopupEvents();
		if (!mHasIncoming)
			return NULL;

		OSStatus result = OTAccept(mMacSocket, mWorker, &mIncoming);
		switch(result) {
		case noErr:
			EndpointRef	newSock = mWorker;
			mWorker = NULL;
			return new PCSBSocket(newSock);
		case kOTLookErr:
			continue;
		default:
			mHasIncoming = false;
			return NULL;
		}
	}
}
			
#pragma mark -
		 
unsigned long		PCSBSocket::LookupAddress(
	 							const char *	inAddress)
{
		OSStatus 		result;
		InetHostInfo	theInfo;
	
	result = ::OTInetStringToAddress(sSvcRef, const_cast<char *>(inAddress), &theInfo);

	if (result == noErr)
		return theInfo.addrs[0];
	else
		return 0;
}

void	PCSBSocket::MopupEvents(void)
{
	OTResult	result = ::OTLook(mMacSocket);
	
	switch(result) {
	case T_CONNECT:
#if LOG_MAC_SOCKETS
		dprintf("Received connect.\n");
#endif				
		if (::OTRcvConnect(mMacSocket, NULL) != noErr)
			mErr = true;
		break;
	case T_ORDREL:
#if LOG_MAC_SOCKETS
		dprintf("Received orderly release.\n");
#endif				
		if (::OTRcvOrderlyDisconnect(mMacSocket) != noErr)
			mErr = true;
		mDone = true;
		break;
	case T_DISCONNECT:
#if LOG_MAC_SOCKETS
		dprintf("Received disconnect.\n");
#endif				
		if (::OTRcvDisconnect(mMacSocket, NULL) != noErr)
			mErr = true;
		mDone = true;
		break;
	case T_LISTEN:
		if (::OTListen(mMacSocket, &mIncoming) == noErr)
			mHasIncoming = true;
#if LOG_MAC_SOCKETS
		dprinf("Received listen, %s, IP = 0x%08X:%d.\n", mHasIncoming ? "success" : "failure" ,
			mIncomingIP.fHost, mIncomingIP.fPort);
#endif
		break;
	}
}

PCSBSocket::PCSBSocket(EndpointRef inSocket)
{
	mErr = false;
	mDone = false;
	
	mIncoming.addr.buf = (unsigned char *) &mIncomingIP;
	mIncoming.addr.len = mIncoming.addr.maxlen = sizeof(mIncomingIP);
	mIncoming.udata.buf = mIncoming.opt.buf = nil;
	mIncoming.udata.maxlen = mIncoming.udata.len = mIncoming.opt.len = mIncoming.opt.maxlen = 0;
	mIncomingIP.fAddressType = AF_INET;
	mIncomingIP.fPort = 0;
	mIncomingIP.fHost = 0;
	mIncoming.sequence = 0;
	mHasIncoming = false;
			
	mWorker = NULL;			
	mMacSocket = inSocket;

	TBind		local, remote;
	InetAddress	localIP, remoteIP;

	localIP.fAddressType = remoteIP.fAddressType = AF_INET;
	local.addr.maxlen = local.addr.len = remote.addr.maxlen = remote.addr.len = sizeof(localIP);
	local.addr.buf = (unsigned char *) &localIP;
	remote.addr.buf = (unsigned char *) &remoteIP;

	if (::OTGetProtAddress(mMacSocket, &local, &remote) == noErr)
	{
		mLocalIP = localIP.fHost;
		mRemoteIP = remoteIP.fHost;
		mLocalPort = localIP.fPort;
		mRemotePort = remoteIP.fPort;
	}

	::OTSetAsynchronous(mMacSocket);
}

