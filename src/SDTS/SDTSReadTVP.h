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
#ifndef SDTSREADTVP_H
#define SDTSREADTVP_H

#include "MapDefsCGAL.h"

class	sb_ForeignID;
class	sc_Record;
class	sb_Spatial;

struct ForeignKey : public pair<string, int> {
	ForeignKey();
	ForeignKey(const sb_ForeignID&);
	ForeignKey(const string&, int);
	ForeignKey(const ForeignKey&);
};

enum SDTSErrorType {

	sdts_Ok = 0,
	sdts_BadForeignKey,
	sdts_BadLink,
	sdts_BadRecordFile,
	sdts_WrongRecordType,
	sdts_MissingInfo,
	sdts_BadSpatialID,
	sdts_LogicError

};

class	SDTSException : public exception {
public:

	SDTSException(SDTSErrorType errType, const ForeignKey& theKey, const ForeignKey& us);
	SDTSException(SDTSErrorType errType, const ForeignKey& theKey, const char * info);
	SDTSException(SDTSErrorType errType, const sc_Record& theRecord, const char * info);
	SDTSException(SDTSErrorType errType, const sb_Spatial&);
	SDTSException(SDTSErrorType errType, const char *, const char *);

	virtual const char* what () const _MSL_THROW {return mBuf;}

private:

	char	mBuf[512];

};

void	ImportSDTSTransferTVP(const char * path, const char * external, Pmwx& pmwx);


#endif