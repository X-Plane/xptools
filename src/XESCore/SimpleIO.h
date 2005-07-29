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
#ifndef SIMPLEIO_H
#define SIMPLEIO_H

#if APL && !defined(__MACH__)
#include <Files.h>
#endif

#include "IODefs.h"
#include <zlib.h>
#include <zip.h>
#include "EndianUtils.h"

const char	kSwapTwo[] = { 2, 0 };
const char	kSwapFour[] = { 4, 0 };
const char	kSwapEight[] =  { 8, 0 };


class	FileReader : public IOReader {
public:

					FileReader(const char * inFileName, PlatformType platform = platform_LittleEndian);
					FileReader(FILE * inFile, PlatformType platform = platform_LittleEndian);
	virtual			~FileReader();

	virtual	void	ReadShort(short&);
	virtual	void	ReadInt(int&);
	virtual	void	ReadFloat(float&);
	virtual	void	ReadDouble(double&);
	virtual	void	ReadBulk(char * inBuf, int inLength, bool inZip);

private:

	FILE *			mFile;	
	bool			mClose;
	PlatformType	mPlatform;
	
};

class	MemFileReader : public IOReader {
public:

					MemFileReader(const char * inStart, const char * inEnd, PlatformType platform = platform_LittleEndian);
	virtual			~MemFileReader();

	virtual	void	ReadShort(short&);
	virtual	void	ReadInt(int&);
	virtual	void	ReadFloat(float&);
	virtual	void	ReadDouble(double&);
	virtual	void	ReadBulk(char * inBuf, int inLength, bool inZip);

private:

	const char *	mPtr;
	const char *	mEnd;
	PlatformType	mPlatform;	
	
};

class	FileWriter : public IOWriter {
public:
					FileWriter(const char * inFileName, PlatformType platform = platform_LittleEndian);
					FileWriter(FILE * inFile, PlatformType platform = platform_LittleEndian);
	virtual			~FileWriter();


	virtual	void	WriteShort(short);
	virtual	void	WriteInt(int);
	virtual	void	WriteFloat(float);
	virtual	void	WriteDouble(double);
	virtual	void	WriteBulk(const char * inBuf, int inLength, bool inZip);

private:

	FILE *			mFile;	
	bool			mClose;
	PlatformType	mPlatform;
	
};	

class	ZipFileWriter : public IOWriter {
public:
					ZipFileWriter(const char * inFileName, const char * inEntryName, PlatformType platform = platform_LittleEndian);
	virtual			~ZipFileWriter();


	virtual	void	WriteShort(short);
	virtual	void	WriteInt(int);
	virtual	void	WriteFloat(float);
	virtual	void	WriteDouble(double);
	virtual	void	WriteBulk(const char * inBuf, int inLength, bool inZip);

private:

	zipFile			mFile;
	PlatformType	mPlatform;
	
};	

#define WRITER_BUFFER_SIZE 65536
#define WRITER_BUFFER_PAD 1024
class	WriterBuffer {

	char			mBuffer[WRITER_BUFFER_SIZE + WRITER_BUFFER_PAD];
	int				mPos;
	bool			mSwap;
	IOWriter *	mWriter;
	PlatformType	mPlatform;

	void	Flush(void)
	{
		if (mPos > 0) 
			mWriter->WriteBulk(mBuffer, mPos, false); 
		mPos = 0;
	}

public:

	WriterBuffer(IOWriter * inWriter, PlatformType platform = platform_LittleEndian) 
	{
		mWriter = inWriter;
		mPos = 0;
		mPlatform = platform;
		mSwap = (platform != platform_Native && platform != GetNativePlatformType());
	}

	~WriterBuffer() 
	{ 
		Flush();
	}

	inline void	WriteShort(short x)
	{
		if (mSwap)
			EndianSwapBuffer(platform_Native, mPlatform, kSwapTwo, &x);
		*((short *) (mBuffer + mPos)) = x;
		mPos += sizeof(x);
		if (mPos > WRITER_BUFFER_SIZE) 
			Flush();
	}
	
	inline void	WriteInt(int x)
	{
		if (mSwap)
			EndianSwapBuffer(platform_Native, mPlatform, kSwapFour, &x);
		*((int *) (mBuffer + mPos)) = x;
		mPos += sizeof(x);
		if (mPos > WRITER_BUFFER_SIZE) 
			Flush();
	}
		
	inline void	WriteFloat(float x)
	{
		if (mSwap)
			EndianSwapBuffer(platform_Native, mPlatform, kSwapFour, &x);
		*((float *) (mBuffer + mPos)) = x;
		mPos += sizeof(x);
		if (mPos > WRITER_BUFFER_SIZE) 
			Flush();
	}
		
	inline void	WriteDouble(double x)
	{
		if (mSwap)
			EndianSwapBuffer(platform_Native, mPlatform, kSwapEight, &x);
		*((double *) (mBuffer + mPos)) = x;
		mPos += sizeof(x);
		if (mPos > WRITER_BUFFER_SIZE) 
			Flush();
	}
		
	inline void	WriteBulk(const char * inBuf, int inLength, bool inZip)
	{
		if (inLength > WRITER_BUFFER_PAD)
		{
			mWriter->WriteBulk(inBuf, inLength, inZip);
		} else {
			memcpy(mBuffer + mPos, inBuf, inLength);
			mPos += inLength;
			if (mPos > WRITER_BUFFER_SIZE) Flush();			
		}
	}

};

#endif