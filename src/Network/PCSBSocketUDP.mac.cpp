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
#include <Carbon/Carbon.h>

PCSBSocketUDP::PCSBSocketUDP(unsigned short inPort)
{
		OSStatus	err;

	mMacSocket = ::OTOpenEndpointInContext(
			::OTCreateConfiguration(kUDPName),
			0, 					// Options must be 0
			NULL, 				// Don't care about endpoint info
			&err,
			NULL);
	if (err != noErr)	throw udp_Exception_UnknownError;

		InetAddress		localAddress, realAddress;
		TBind			req, ret;

	localAddress.fAddressType = AF_INET;
	localAddress.fPort = inPort;
	localAddress.fHost = kOTAnyInetAddress;

	req.addr.maxlen = req.addr.len = sizeof(localAddress);
	req.addr.buf = (unsigned char *) &localAddress;
	req.qlen = 0;

	ret.addr.maxlen = sizeof(realAddress);
	ret.addr.buf = (unsigned char *) &realAddress;

	err = ::OTBind(mMacSocket, &req, &ret);
	if (err != noErr)
	{
		OTCloseProvider(mMacSocket);
		throw udp_Exception_BindFailed;
	}

	::OTSetNonBlocking(mMacSocket);

}

PCSBSocketUDP::~PCSBSocketUDP()
{
	::OTCloseProvider(mMacSocket);
}

long	PCSBSocketUDP::ReadData(
							void * 				outBuf,
							long				outBufLength,
							unsigned long *		outSrcIP,
							unsigned short *	outSrcPort)
{
		OTFlags	flags;

	InetAddress		remoteAddress;
	TUnitData		packet;

	remoteAddress.fAddressType = AF_INET;
	remoteAddress.fPort = 0;
	remoteAddress.fHost = 0;
	packet.addr.maxlen = packet.addr.len = sizeof(remoteAddress);
	packet.addr.buf = (unsigned char *) &remoteAddress;
	packet.opt.len = packet.opt.maxlen = 0;
	packet.udata.	maxlen = packet.udata.len = outBufLength;
	packet.udata.buf = (unsigned char *) outBuf;

	OSStatus result = ::OTRcvUData(
		mMacSocket, &packet, &flags);

	*outSrcIP = remoteAddress.fHost;
	*outSrcPort = remoteAddress.fPort;

	if (result == noErr)
		return packet.udata.len;
	else
		return -1;
}

long	PCSBSocketUDP::WriteData(
							void *				inBuf,
							long				inBufLength,
							unsigned long		inDstIP,
							unsigned short		inDstPort)
{
		InetAddress		remoteAddress;
		TUnitData		packet;

	remoteAddress.fAddressType = AF_INET;
	remoteAddress.fPort = inDstPort;
	remoteAddress.fHost = inDstIP;
	packet.addr.maxlen = packet.addr.len = sizeof(remoteAddress);
	packet.addr.buf = (unsigned char *) &remoteAddress;
	packet.opt.len = packet.opt.maxlen = 0;
	packet.udata.maxlen = packet.udata.len = inBufLength;
	packet.udata.buf = (unsigned char *) inBuf;

	OSStatus result = ::OTSndUData(mMacSocket, &packet);

	if (result == noErr)
		return inBufLength;
	else
		return -1;
}
