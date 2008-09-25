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
#include "XResources.h"
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "ErrMsg.h"

/*
MACINTOSH "RESOURCE" IMPLEMENTATION.

My original design used the Mac resource manager, but it turns out that
this doesn't work - a file's rseource fork is limited to 16 MB which
is not enough for a serious installer.  So here's what we do instead:

We store our resources in a giant data chunk that is after the
app's code fragment in the data fork of the app.  This means that we
must adjust the cfrg resource (if necessary) to not use kWholeFrag
as the fragment length.  We can then use the rest of the file as a data
dump.

The format of this data dump is: a 4-byte count for the number of
"resources", followed by (for each resource) a 4-byte res length
and then the resource data.

One implementation note: if we see the fragment has a length of 0,
we know we haven't worked with it yet - we set the length to the data
fork length and don't erase any old resources.  If the fragment length
is non zero we nuke anything after the fragment to reset the file.
*/

using namespace std;

static short	gMyResFile = 0;
static bool		gSetupMe = 0;
static int		gNumResources = 0;

static	void	SetupReadSelf(void)
{
	int offset = 0;
	Handle	cfrg = Get1Resource('cfrg', 0);
	if (cfrg == NULL)
		if (ReportError("Could not locate Cfrg 0", paramErr, "")) return;
	HLock(cfrg);
	CFragResource *	frag = (CFragResource*) *cfrg;
	if (frag->memberCount != 1) {
		if (ReportError("Installer does not have one code fragment", paramErr, "")) return;
	} else if (frag->firstMember.length == 0) {
		if (ReportError("Not a finished installer.",ResError(), "")) return;
	} else
		offset = frag->firstMember.length;
	HUnlock(cfrg);
	ReleaseResource(cfrg);

	FSSpec	me;
	ProcessInfoRec	rec = { 0 };
	Str255 name;
	rec.processInfoLength = sizeof(rec);
	rec.processName = name;
	rec.processAppSpec = &me;
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	if (GetProcessInformation(&psn, &rec) != noErr) return;

	if (ReportError("Could not reopen installer", FSpOpenDF(&me, fsRdPerm, &gMyResFile), "")) return;
	if (ReportError("Could not reposition", SetFPos(gMyResFile, fsFromStart, offset), "")) return;
	long len = 4;
	if (ReportError("Could not read number of items", FSRead(gMyResFile, &len, &gNumResources), "")) return;
	gSetupMe = 1;
}

int		XRES_CountResources()
{
	if (!gSetupMe) SetupReadSelf();
	if (!gSetupMe) return 0;
	return gNumResources;
}

int		XRES_GetResourceData(char ** outPtr, int * outSize)
{
	if (!gSetupMe) SetupReadSelf();
	if (!gSetupMe) return 0;
	long	l = 4;
	int		c;
	if (ReportError("Could not read number of items", FSRead(gMyResFile, &l, &c), "")) return 0;
	*outPtr = (char *) malloc(c);
	*outSize = c;
	if (*outPtr == NULL)
	{
		ReportError("Out of memory", memFullErr, "");
		return 0;
	}
	l = c;
	if (ReportError("Could not read data item", FSRead(gMyResFile, &l, *outPtr), "")) return 0;
	return 1;
}


struct	ResInfo {
	short	fileNum;
	long	base;
	long	count;
};


void *	XRES_BeginSettingResources(const char * inFilePath)
{
	long df, rf;
	long start_of_data = 0;
	if (!GetForkSizes(inFilePath,df,rf))
	{
		ReportError("Could not determine contents of file.", paramErr, inFilePath);
		return 0;
	}
	FSSpec	spec;
	if (!FilePathToFSSpec(inFilePath, spec)) return false;
	short resFile = FSpOpenResFile(&spec, fsRdWrPerm);
	if (resFile == -1)
	{
		ReportError("could not open installer", ResError(), inFilePath);
		return NULL;
	}

	Handle	cfrg = Get1Resource('cfrg', 0);
	if (cfrg == NULL)
	{
		CloseResFile(resFile);
		ReportError("Could not locate Cfrg 0", paramErr, inFilePath);
		return 0;
	}
	HLock(cfrg);
	CFragResource *	frag = (CFragResource*) *cfrg;
	bool ok = true;
	if (frag->memberCount != 1) {
		ok = false;
		ReportError("Installer does not have one code fragment", paramErr, inFilePath);
	} else if (frag->firstMember.length == 0)
	{
		frag->firstMember.length = df;
		start_of_data = df;
		ChangedResource(cfrg);
		WriteResource(cfrg);
		if (ReportError("Could not modify code fragment.",ResError(), inFilePath)) ok = false;
	} else
		start_of_data = frag->firstMember.length;
	HUnlock(cfrg);
	CloseResFile(resFile);
	if (!ok) return 0;
	short fileNum;
	if (ReportError("Could not open installer DF", FSpOpenDF(&spec, fsRdWrPerm, &fileNum), inFilePath)) return 0;
	if (ReportError("Could not set LEOF", SetEOF(fileNum, start_of_data), inFilePath)) return 0;
	if (ReportError("Could not set position", SetFPos(fileNum, fsFromStart, start_of_data), inFilePath)) return 0;
	ResInfo * info = new ResInfo;
	info->fileNum = fileNum;
	info->base = start_of_data;
	info->count = 0;
	int d = 0;
	long l = 4;
	if (ReportError("Could not write installer data header", FSWrite(fileNum, &l, &d), inFilePath)) return 0;
	return info;
}

int		XRES_AddResource(void * inFile, char * inPtr, int inSize)
{
	ResInfo * info = (ResInfo *) inFile;
	long l = 4;
	if (ReportError("Could not write installer data item header", FSWrite(info->fileNum, &l, &inSize), "")) return 0;
	l = inSize;
	if (ReportError("Could not write installer data header", FSWrite(info->fileNum, &l, inPtr), "")) return 0;
	info->count++;
	return 1;
}

int		XRES_EndSettingResources(void * inFile)
{
	ResInfo * info = (ResInfo *) inFile;
	if (ReportError("Could not reset installer file marker", SetFPos(info->fileNum, fsFromStart, info->base), "")) return 0;
	long l = 4;
	if (ReportError("Could not write number of files to installer", FSWrite(info->fileNum, &l, &info->count), "")) return 0;
	FSClose(info->fileNum);
	delete info;
	return 1;
}

#if 0
static	int	gAddCount = 0;
static	int	gBytes = 0;

const ResType kInstallerType = 'XPLN';

int		XRES_CountResources()
{
	return Count1Resources(kInstallerType);
}

int		XRES_GetResourceData(int id, char ** outPtr, int * outSize)
{
	Handle	res = Get1Resource(kInstallerType, id);
	if (res == NULL) return 0;

	*outSize = GetHandleSize(res);

	*outPtr = (char*) malloc(*outSize);
	if (*outPtr == NULL)
	{
		ReportError("out of memory", memFullErr, "");
		ReleaseResource(res);
		return 0;
	}

	HLock(res);
	memcpy(*outPtr, *res, *outSize);
	HUnlock(res);
	ReleaseResource(res);
	return 1;
}

void *	XRES_BeginSettingResources(const char * inFilePath)
{
	gAddCount = 0;
	gBytes = 0;
	FSSpec	spec;
	if (!FilePathToFSSpec(inFilePath, spec)) return false;
	short resFile = FSpOpenResFile(&spec, fsRdWrPerm);
	if (resFile == -1)
	{
		ReportError("could not open installer", ResError(), inFilePath);
		return NULL;
	}
	SetResLoad(FALSE);

	vector<Handle>	nuke;
	int max = Count1Resources(kInstallerType);
	for (int index = 1; index <= max; ++index)
		nuke.push_back(Get1IndResource(kInstallerType, index));
	SetResLoad(TRUE);
	for (int n = 0; n < nuke.size(); ++n)
	if (nuke[n] != NULL)
		RemoveResource(nuke[n]);

	return (void *) resFile;
}

int		XRES_AddResource(void * inFile, char * inPtr, int inSize)
{
	ReportError("internal res error!",ResError(), "");
	++gAddCount;
	gBytes += inSize;
	short resfile = (short) inFile;
	Handle	mem = NewHandle(inSize);
	if (mem == NULL)
	{
		ReportError("Out of memory", memFullErr, "");
		return 0;
	}
	HLock(mem);
	memcpy(*mem, inPtr, inSize);
	HUnlock(mem);
	short old_cur = CurResFile();
	UseResFile(resfile);
	AddResource(mem, kInstallerType, Count1Resources(kInstallerType), "\p");
	if (ReportError("could not add resource to installer",ResError(), ""))
		return 0;
	UseResFile(old_cur);
	static int counter = 0;
	++counter;
	if (counter > 50)
	{
		counter = 0;
		UpdateResFile(resfile);
		if (ReportError("could not update resource file",ResError(), ""))
			return 0;
	}
	return 1;
}

int		XRES_EndSettingResources(void * inFile)
{
	short resFile = (short) inFile;
	UpdateResFile(resFile);
	int ret = 1;
	if (ReportError("could not update res file",ResError(), "")) ret = 0;
	CloseResFile(resFile);
	if (ReportError("could not close res file",ResError(), "")) ret = 0;
	return ret;
}


#endif