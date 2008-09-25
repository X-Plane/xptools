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
#include "XFileTwiddle.h"
#if defined(__MWERKS__)
#include <Files.h>
#include <Script.h>
#else
#include <Carbon/Carbon.h>
#endif
#include <string.h>
#include <stdlib.h>
#include "MacSpec.h"
#include "ErrMsg.h"
#include <string>
#include "PlatformUtils.h"

/*

MACINTOSH ENCODING OF FILES INTO MEM BLOCKS:
Mac files are scanned first in a 20-byte header:

Finder type signature
Creator type signature
Flags (32 bits - lower 16 are valid)
Length of data fork
Length of resource fork
<Data fork data>
<Resource fork data>

 */

using std::string;

int		MakeDirExist(const char * inPath)
{
	string	path(inPath), chunk;
	string::size_type	pos = 0;
	FSSpec	spec;
	while (1)
	{
		pos = path.find(DIR_CHAR, pos+1);
		if (pos == path.npos) break;
		chunk = path.substr(0, pos);
		if (FilePathToFSSpec(chunk.c_str(), spec))
		{
			FSSpec spec2;
			if (FSMakeFSSpec(spec.vRefNum, spec.parID, spec.name, &spec2) == fnfErr)
			{
				long id;
				if (FSpDirCreate(&spec, smSystemScript, &id) != noErr)
					return 0;
			}
		} else
			return 0;
	}
	return 1;
}

int		NukeFile(const char * inPath)
{
	FSSpec	spec;
	if (!FilePathToFSSpec(inPath, spec)) return 0;
	OSErr err = FSpDelete(&spec);
	if (err == noErr || err == fnfErr)
		return 1;
	ReportError("Could not delete file", err, inPath);
	return 0;
}

int		GetFileBlockSizeIfExists(const char * inPath)
{
	FSSpec	spec;
	if (!FilePathToFSSpec(inPath, spec)) return -1;

	CInfoPBRec	block;
	block.hFileInfo.ioNamePtr = spec.name;
	block.hFileInfo.ioVRefNum = spec.vRefNum;
	block.hFileInfo.ioFDirIndex = 0;
	block.hFileInfo.ioDirID = spec.parID;
	if (PBGetCatInfoSync(&block) != noErr) return -1;

	return 20 + block.hFileInfo.ioFlLgLen + block.hFileInfo.ioFlRLgLen;
}

int		FileToBlock(const char * inPath, char ** outPtr, int * outSize)
{
	FSSpec	spec;
	FInfo	finfo;
	OSErr	err;
	if (!FilePathToFSSpec(inPath, spec)) return 0;
	err = FSpGetFInfo(&spec, &finfo);
	ReportError("Unable to get info about file", err, inPath);

	long	df_len = 0;
	long	rf_len = 0;
	bool opened_df = false, opened_rf = false;
	short df_fork, rf_fork;
	if (!GetForkSizes(inPath, df_len, rf_len)) return 0;
	if (df_len > 0)
	{
		err = FSpOpenDF(&spec, fsRdPerm, &df_fork);		if (ReportError("Unable to open df file", err, inPath)) return 0;
		opened_df = true;
		err = SetFPos(df_fork, fsFromLEOF, 0);			if (ReportError("Unable to set df file position to end", err, inPath)) return 0;
		err = GetFPos(df_fork, &df_len);				if (ReportError("Unable to read DF length", err, inPath)) return 0;
		err = SetFPos(df_fork, fsFromStart, 0);			if (ReportError("Unable to set df file position to start", err, inPath)) return 0;
	}
	if (rf_len > 0)
	{
		err = FSpOpenRF(&spec, fsRdPerm, &rf_fork);		if (ReportError("Unable to open rf file", err, inPath)) return 0;
		opened_rf = true;
		err = SetFPos(rf_fork, fsFromLEOF, 0);			if (ReportError("Unable to set rf file position to end", err, inPath)) return 0;
		err = GetFPos(rf_fork, &rf_len);				if (ReportError("Unable to read rf length", err, inPath)) return 0;
		err = SetFPos(rf_fork, fsFromStart, 0);			if (ReportError("Unable to set rf file position to start", err, inPath)) return 0;
	}
	long	mem_needed = 20 + df_len + rf_len;
	*outPtr = (char *) malloc(mem_needed);
	if (*outPtr == NULL)
	{
		if (opened_rf) FSClose(rf_fork);
		if (opened_df) FSClose(df_fork);
		ReportError("Out of memory reading file", memFullErr, inPath);
		return 0;
	}
	char * p = *outPtr;
	memcpy(p, &finfo.fdType, 4);			p += 4;
	memcpy(p, &finfo.fdCreator, 4);			p += 4;
	int	flags32 = finfo.fdFlags;
	memcpy(p, &flags32, 4);					p += 4;
	memcpy(p, &df_len, 4);					p += 4;
	memcpy(p, &rf_len, 4);					p += 4;
	if (opened_df && df_len > 0)
	{
		err = FSRead(df_fork, &df_len, p);
		ReportError("Unable to read file df", err, inPath);
	}
	p += df_len;
	if (opened_rf && rf_len > 0)
	{
		err = FSRead(rf_fork, &rf_len, p);
		ReportError("Unable to read file rf", err, inPath);
	}

	*outSize = mem_needed;
	if (opened_rf) FSClose(rf_fork);
	if (opened_df) FSClose(df_fork);

	return 1;
}

int		BlockToFile(const char * inPath, char * inPtr)
{
	FSSpec	spec;

	string	dirPath(inPath);
	string::size_type pos = dirPath.rfind(DIR_CHAR);
	dirPath.erase(pos+1);
	MakeDirExist(dirPath.c_str());

	if (!FilePathToFSSpec(inPath, spec))
	{
		ReportError("file path may be bad", fnfErr, inPath);
		return 0;
	}

	long * l = (long *) inPtr;
	OSType	fdType = *l;			++l;
	OSType	fdCreator = *l;			++l;
	UInt16	fdFlags = *l;			++l;
	long	df_len = *l;			++l;
	long	 rf_len = *l;

	OSErr err = FSpCreate(&spec,  fdCreator, fdType, smSystemScript);
	if (err != noErr && err != dupFNErr)  {
		ReportError("could not create file", err, inPath);
		return 0;
	}
	short df_fork, rf_fork;
	if (rf_len > 0)
	{
		err = FSpOpenRF(&spec, fsRdWrPerm, &rf_fork);				if (ReportError("Unable to open resource fork", err, inPath)) return 0;
		err = SetFPos(rf_fork, fsFromStart, 0);						if (ReportError("Unable to reset file pos rf", err, inPath)) return 0;
		err = SetEOF(rf_fork, 0);									if (ReportError("Unable to zero out rf", err, inPath)) return 0;
		err = FSWrite(rf_fork, &rf_len, inPtr + df_len + 20);		if (ReportError("Unable to wrirte to rf", err, inPath)) return 0;
		FSClose(rf_fork);
	}
	if (df_len > 0)
	{
		err = FSpOpenDF(&spec, fsRdWrPerm, &df_fork);				if (ReportError("Unable to open data fork", err, inPath)) return 0;
		err = SetFPos(df_fork, fsFromStart, 0);						if (ReportError("Unable to reset file pos df", err, inPath)) return 0;
		err = SetEOF(df_fork, 0);									if (ReportError("Unable to zero out df", err, inPath)) return 0;
		err = FSWrite(df_fork, &df_len, inPtr + 20);				if (ReportError("Unable to wrirte to df", err, inPath)) return 0;
		FSClose(df_fork);
	}
	FInfo	info;
	err = FSpGetFInfo(&spec, &info);								if (ReportError("Unable to read finder info", err, inPath)) return 0;
	info.fdType = fdType;
	info.fdCreator = fdCreator;
	info.fdFlags = fdFlags;
	err = FSpSetFInfo(&spec, &info);								if (ReportError("Unable to write finder info", err, inPath)) return 0;

	return 1;
}
