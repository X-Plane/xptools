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
#include "SimpleIO.h"
#include "EndianUtils.h"
#include "AssertUtils.h"

#if IBM
typedef	unsigned short UInt16;
typedef unsigned long UInt32;
typedef unsigned long long UInt64;
#endif

FileReader::FileReader(const char * inFileName, PlatformType platform)
{
	mFile = fopen(inFileName, "rb");
	mClose = true;
	mPlatform = platform;
}

FileReader::FileReader(FILE * inFile, PlatformType platform)
{
	mFile = inFile;
	mClose = false;
	mPlatform = platform;
}

FileReader::~FileReader()
{
	if (mClose)
		fclose(mFile);
}

void	FileReader::ReadShort(short& x)
{
	if (1 != fread(&x, sizeof(x), 1, mFile))
		throw "fread error";
	EndianSwapBuffer(mPlatform, platform_Native, kSwapTwo, &x);
}

void	FileReader::ReadInt(int& x)
{
	if (1 != fread(&x, sizeof(x), 1, mFile))
		throw "fread error";
	EndianSwapBuffer(mPlatform, platform_Native, kSwapFour, &x);
}

void	FileReader::ReadFloat(float& x)
{
	if (1 != fread(&x, sizeof(x), 1, mFile))
		throw "fread error";
	EndianSwapBuffer(mPlatform, platform_Native, kSwapFour, &x);
}

void	FileReader::ReadDouble(double& x)
{
	if (1 != fread(&x, sizeof(x), 1, mFile))
		throw "fread error";
	EndianSwapBuffer(mPlatform, platform_Native, kSwapEight, &x);
}

void	FileReader::ReadBulk(char * inBuf, int inLength, bool inZip)
{
	if (1 != fread(inBuf, inLength, 1, mFile))
		throw "fread error";
}

MemFileReader::MemFileReader(const char * inStart, const char * inEnd, PlatformType platform)
{
	mPtr = inStart;
	mEnd = inEnd;
	mPlatform = platform;
}

MemFileReader::~MemFileReader()
{
}

ATTR_DISABLE_UB_SAN // Allow misaligned pointer use
void	MemFileReader::ReadShort(short& x)
{
	if (mPtr >= mEnd)	return;
	x = *((short *) mPtr);
	mPtr += sizeof(short);
	EndianSwapBuffer(mPlatform, platform_Native, kSwapTwo, &x);
}

ATTR_DISABLE_UB_SAN // Allow misaligned pointer use
void	MemFileReader::ReadInt(int& x)
{
	if (mPtr >= mEnd)	return;
	x = *((int *) mPtr);
	mPtr += sizeof(int);
	EndianSwapBuffer(mPlatform, platform_Native, kSwapFour, &x);
}

ATTR_DISABLE_UB_SAN // Allow misaligned pointer use
void	MemFileReader::ReadFloat(float& x)
{
	if (mPtr >= mEnd)	return;
	x = *((float *) mPtr);
	mPtr += sizeof(float);
	EndianSwapBuffer(mPlatform, platform_Native, kSwapFour, &x);
}

ATTR_DISABLE_UB_SAN // Allow misaligned pointer use
void	MemFileReader::ReadDouble(double& x)
{
	if (mPtr >= mEnd)	return;
	x = *((double *) mPtr);
	mPtr += sizeof(double);
	EndianSwapBuffer(mPlatform, platform_Native, kSwapEight, &x);
}

void	MemFileReader::ReadBulk(char * inBuf, int inLength, bool inZip)
{
	if (mPtr >= mEnd) return;
	memcpy(inBuf, mPtr, inLength);
	mPtr += inLength;
}






FileWriter::FileWriter(const char * inFileName, PlatformType platform)
{
	mFile = fopen(inFileName, "wb");
	mClose = true;
	mPlatform = platform;
}

FileWriter::FileWriter(FILE * inFile, PlatformType platform)
{
	mFile = inFile;
	mClose = false;
	mPlatform = platform;
}

FileWriter::~FileWriter()
{
	if (mClose)
		fclose(mFile);
}

void	FileWriter::WriteShort(short x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapTwo, &x);
	fwrite(&x, sizeof(x), 1, mFile);
}

void	FileWriter::WriteInt(int x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapFour, &x);
	fwrite(&x, sizeof(x), 1, mFile);
}

void	FileWriter::WriteFloat(float x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapFour, &x);
	fwrite(&x, sizeof(x), 1, mFile);
}

void	FileWriter::WriteDouble(double x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapEight, &x);
	fwrite(&x, sizeof(x), 1, mFile);
}

void	FileWriter::WriteBulk(const char * inBuf, int inLength, bool inZip)
{
	fwrite(inBuf, inLength, 1, mFile);
}

ZipFileWriter::ZipFileWriter(const char * inFileName, const char * inEntryName, PlatformType platform)
{
	mFile = zipOpen(inFileName, 0);
	mPlatform = platform;
	if (mFile)
	{
		zipOpenNewFileInZip(mFile, inEntryName, NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	}
}

ZipFileWriter::~ZipFileWriter()
{
	if (mFile)
	{
		zipCloseFileInZip(mFile);
		zipClose(mFile, NULL);
	}
}


void	ZipFileWriter::WriteShort(short x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapTwo, &x);
	zipWriteInFileInZip(mFile, &x, sizeof(x));
}

void	ZipFileWriter::WriteInt(int x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapFour, &x);
	zipWriteInFileInZip(mFile, &x, sizeof(x));
}

void	ZipFileWriter::WriteFloat(float x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapFour, &x);
	zipWriteInFileInZip(mFile, &x, sizeof(x));
}

void	ZipFileWriter::WriteDouble(double x)
{
	EndianSwapBuffer(platform_Native, mPlatform, kSwapEight, &x);
	zipWriteInFileInZip(mFile, &x, sizeof(x));
}

void	ZipFileWriter::WriteBulk(const char * inBuf, int inLength, bool inZip)
{
	zipWriteInFileInZip(mFile, (void * const) inBuf, inLength);
}


