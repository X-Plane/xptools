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
#include "MacSpec.h"
#if defined(__MWERKS__)
	#include <Files.h>
#else
	#include <Carbon/Carbon.h>
#endif
#include <string>
using std::string;
#include "PlatformUtils.h"
#include <string.h>

int		GetForkSizes(const char * inPath, long& outDF, long& outRF)
{
	FSSpec	spec;
	if (!FilePathToFSSpec(inPath, spec)) return 0;

	CInfoPBRec	block;
	block.hFileInfo.ioNamePtr = spec.name;
	block.hFileInfo.ioVRefNum = spec.vRefNum;
	block.hFileInfo.ioFDirIndex = 0;
	block.hFileInfo.ioDirID = spec.parID;
	if (PBGetCatInfoSync(&block) != noErr) return 0;
	outDF = block.hFileInfo.ioFlLgLen;
	outRF = block.hFileInfo.ioFlRLgLen;
	return 1;
}

bool	FilePathToFSSpec(const char * inPath, FSSpec& outSpec)
{
		CInfoPBRec 			pb = { 0 };
		string				path(inPath), comp;
		string::size_type 	sep;
		Str255				fnameP;

	sep = path.find(DIR_CHAR);
	if (sep == path.npos)
	{
		path += DIR_CHAR;
		comp = path;
	} else
		comp = path.substr(0, sep+1);
	memcpy(fnameP+1,comp.c_str(), comp.size());
	fnameP[0] = comp.size();
	if (::FSMakeFSSpec(0, 0, fnameP, &outSpec) != noErr) return false;
	if (outSpec.parID == fsRtParID)
		outSpec.parID = fsRtDirID;

	if (sep != path.npos)
		path.erase(0,sep+1);
	else
		path.clear();

	while(!path.empty())
	{
		sep = path.find(DIR_CHAR);
		if (sep == path.npos)
			comp = path;
		else
			comp = path.substr(0, sep);
		memcpy(fnameP+1,comp.c_str(), comp.size());
		fnameP[0] = comp.size();

		pb.dirInfo.ioNamePtr = fnameP;
		pb.dirInfo.ioVRefNum = outSpec.vRefNum;
		pb.dirInfo.ioDrDirID = outSpec.parID;
		pb.dirInfo.ioFDirIndex = 0;

		OSErr err = PBGetCatInfoSync(&pb);
		if (err != noErr)
		{
			if (err == fnfErr && path.find(DIR_CHAR) == path.npos)
				break;
			return false;
		}
		if ((pb.hFileInfo.ioFlAttrib & ioDirMask) != 0)
			outSpec.parID = pb.dirInfo.ioDrDirID;
//		else
//			outSpec.parID = pb.hFileInfo.ioDirID;

		if (sep == path.npos)
			path.clear();
		else
			path.erase(0, sep+1);

		if ((pb.dirInfo.ioFlAttrib & ioDirMask) == 0 && !path.empty())
			return false;
		if (path.empty() && (pb.hFileInfo.ioFlAttrib & ioDirMask) != 0)
			outSpec.parID = pb.dirInfo.ioDrParID;
	}

	outSpec.name[0] = fnameP[0];
	memcpy(outSpec.name+1,fnameP+1,fnameP[0]);
	return true;
}
