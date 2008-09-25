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
#include "XResources.h"
#include "ErrMsg.h"
#include <vector>

int		kMyResourceID = 1001;
//RT_RCDATA

static	int	gCount = 0;
static	char * gBlock = NULL;

static	int	SetupInternalRes(void)
{
	HRSRC	my_res = FindResource(NULL, MAKEINTRESOURCE(kMyResourceID), RT_RCDATA);
	if (my_res == NULL) {
		ReportError("Could not locate resource", GetLastError(), "");
		return 0;
	}
	HGLOBAL my_res_loaded = LoadResource(NULL, my_res);
	if (my_res_loaded == NULL) {
		ReportError("Could not load resource", GetLastError(), "");
		return 0;
	}
	gBlock = (char *) LockResource(my_res_loaded);
	if (gBlock == NULL) {
		ReportError("Could not locate resource", GetLastError(), "");
		return 0;
	}

	gCount = *((int *) gBlock);
	gBlock += 4;
	return 1;
}

int		XRES_CountResources()
{
	if (gBlock == NULL)
	{
		if (!SetupInternalRes()) return 0;
	}
	return gCount;
}

int		XRES_GetResourceData(char ** outPtr, int * outSize)
{
	if (gBlock == NULL)
	{
		if (!SetupInternalRes()) return 0;
	}
	if (gCount <= 0)
		return 0;

	int sz = *((int *) gBlock);
	gBlock += 4;
	gCount--;
	*outPtr = (char *) malloc(sz);
	*outSize = sz;
	if (*outPtr == NULL)
	{
		ReportError("Could not allocate memory for resource", ENOMEM, "");
		return 0;
	}
	memcpy(*outPtr, gBlock, sz);
	gBlock += sz;
	return 1;
}

static	std::vector<char>	sData;
static	int					sCount;

void *	XRES_BeginSettingResources(const char * inFilePath)
{
	sData.clear();
	HANDLE	rez = BeginUpdateResource(inFilePath, false);
	if (rez == NULL)
		ReportError("Unable to open resource file", GetLastError(), inFilePath);
	sCount = 0;
	char * cntPtr = (char *) &sCount;
	sData.insert(sData.end(), cntPtr, cntPtr+4);
	return rez;
}

int		XRES_AddResource(void * inFile, char * inPtr, int inSize)
{
	char * szPtr = (char *) &inSize;
	sData.insert(sData.end(), szPtr, szPtr + 4);
	sData.insert(sData.end(), inPtr, inPtr + inSize);
	++sCount;
	return 1;
}

int		XRES_EndSettingResources(void * inFile)
{
	int *	pCount = (int *) &*sData.begin();
	*pCount = sCount;

	int result = UpdateResource(inFile, RT_RCDATA, MAKEINTRESOURCE(kMyResourceID),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		&*sData.begin(),
		sData.size());

	if (!result)
	{
		ReportError("Could not write resources to installer", GetLastError(), "");
	}

	sData.clear();
	bool ok = EndUpdateResource((HANDLE) inFile, false);
	if (!ok)
		ReportError("Unable to accumulate changes", GetLastError(), "");
	return (ok && result) ? 1 : 0;
}
