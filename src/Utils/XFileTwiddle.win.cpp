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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ErrMsg.h"
#include <string>
#include "PlatformUtils.h"
#include <Windows.h>

/*

Windows encoding of files into blocks:
4-byte length
data fork follows


 */

using std::string;

int		MakeDirExist(const char * inPath)
{
	string master(inPath), chunk;
	string::size_type pos = master.find(DIR_CHAR);
	while (1)
	{
		pos = master.find(DIR_CHAR, pos+1);
		if (pos == master.npos) return 1;
	
		chunk = master.substr(0, pos);
		
		bool ok = CreateDirectory(chunk.c_str(), NULL);
		if (!ok)
		{
			int result = GetLastError();
			if (result != ERROR_ALREADY_EXISTS)
			{
//				ReportError("Could not make dir", result, chunk.c_str());
				return 0;
			}
		}
	}
	return 1;
}

int		NukeFile(const char * inPath)
{
	bool ok = DeleteFile(inPath);
	if (!ok)
		ReportError("Could not delete file", GetLastError(), inPath);
	return ok ? 1 : 0;
}

int		GetFileBlockSizeIfExists(const char * inPath)
{
	FILE * fi = fopen(inPath, "rb");
	if (fi == NULL) return -1;
	fseek(fi, 0L, SEEK_END);
	int file_size = ftell(fi);
	fclose(fi);
	return 4 + file_size;
}

int		FileToBlock(const char * inPath, char ** outPtr, int * outSize)
{
	FILE * fi = fopen(inPath, "rb");
	if (fi == NULL) 
	{
		ReportError("out of memory", errno, inPath);
		return 0;
	}
	
	fseek(fi, 0L, SEEK_END);
	int file_size = ftell(fi);
	fseek(fi, 0L, SEEK_SET);
	*outPtr = (char *) malloc(file_size+4);
	if (*outPtr == NULL)
	{
		fclose(fi);
		ReportError("out of memory", errno, inPath);
		return 0;
	}
	
	*((int *) *outPtr) = file_size;
	int b = fread((*outPtr)+4, 1, file_size, fi);
	if (b != file_size) 
	{
		ReportError("could not read file", errno, inPath);
		free(*outPtr);
		fclose(fi);
		return 0;
	}
	*outSize = file_size+4;
	fclose(fi);
	return 1;	
}		

int		BlockToFile(const char * inPath, char * inPtr)
{
	string	dirPath(inPath);
	dirPath.erase(dirPath.rfind(DIR_CHAR)+1);
	if (!MakeDirExist(dirPath.c_str())) return 0;

	FILE * fi = fopen(inPath, "wb");
	if (fi == NULL)
	{
		ReportError("could not open file for write", errno, inPath);
		return 0;
	}
	int real_len = *((long *) inPtr);
	int write_len = fwrite(inPtr+4, 1, real_len, fi);
	if (write_len != real_len)
		ReportError("could not write data to file", errno, inPath);
	fclose(fi);
	return (write_len == real_len);
}
