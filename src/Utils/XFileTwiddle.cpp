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
#include "MD5.h"
#if APL
#if defined(__MWERKS__)
#include <Files.h>
#else
#include <Carbon/Carbon.h>
#endif
#endif
#include <zlib.h>
#include <string.h>

bool	MD5_Sig::operator==(const MD5_Sig& rhs) const
{
	return memcmp(digest, rhs.digest, sizeof(digest)) == 0;
}


void	MD5_Block(const char * inMem, int inSize, MD5_Sig& outSig)
{
	MD5_CTX ctx;
	MD5Init(&ctx);
	while (inSize > 0)
	{
		int	copy = inSize;
		if (copy > 65535) copy = 65535;
		MD5Update(&ctx, (unsigned char *) inMem, copy);
		inSize -= copy;
		inMem += copy;		
	}
	MD5Final(&ctx);
	memcpy(outSig.digest, ctx.digest, sizeof(ctx.digest));
}

int	ZipBlock(const char * inRaw, int inSize,
				 		char * ioZipped, int * ioZippedSize)
{
	z_stream stream;
	stream.zalloc = NULL;
	stream.zfree = NULL;
	if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK) 
		return 0;
	stream.next_in = (unsigned char *) inRaw;
	stream.avail_in = inSize;
	stream.total_in = 0;
	stream.next_out = (unsigned char *) ioZipped;
	stream.avail_out = *ioZippedSize;
	stream.total_out = NULL;
	int deflate_result = deflate(&stream, Z_FINISH);
	if (deflate_result != Z_STREAM_END)	
		return 0;
	*ioZippedSize = stream.total_out;
	deflate_result = deflateEnd(&stream);
	if (deflate_result != Z_OK)
		return 0;
	return 1;
}				 		

int	UnzipBlock(const char * inZipped, int inZippedSize, char * outRaw, int inRawSize)
{
	z_stream stream;
	stream.zalloc = NULL;
	stream.zfree = NULL;
	if (inflateInit(&stream) != Z_OK) 
		return 0;
	stream.next_in = (unsigned char *) inZipped;
	stream.avail_in = inZippedSize;
	stream.total_in = 0;
	stream.next_out = (unsigned char *) outRaw;
	stream.avail_out = inRawSize;
	stream.total_out = 0;
	if (inflate(&stream, Z_FINISH) != Z_STREAM_END) 
		return 0;
	if (inflateEnd(&stream) != Z_OK) 
		return 0;
	return 1;
}

