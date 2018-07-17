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
#ifndef _EndianUtils_h_
#define _EndianUtils_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	platform_Native,		/* This is whatever your native code is compiled to. */
	platform_BigEndian,
	platform_LittleEndian
} PlatformType;

/*
 *	Format is a NULL-terminated list of data lengths.  Negative numbers
 *	mean a length of data to not be swapped (e.g. strings), positive numbers
 *	mean a length of data to be swapped.  Only 2 or 4 should ever be passed
 *	as positive numbers; larger endian swaps are not yet supported.
 *
 *	EXAMPLE:
 *
 *	{ 2, 4, -2, 2, 0 }
 *
 *	Does a two-byte endian conversion, then a 4-byte conversion, then skips two
 *	bytes, then does another two bytes, then stops.
 *
 */

void	EndianSwapBuffer(
				PlatformType		inStart,
				PlatformType		inEnd,
				const char *		inFormat,
				void *				ioBuffer);

void	EndianSwapArray(
				PlatformType		inStart,
				PlatformType		inEnd,
				int					inCount,
				int					inElementSize,
				void *				ioBuffer);

PlatformType	GetNativePlatformType(void);



/*
 * These routines convert between little endian and native endian.  This means they
 * swap byte order on the Mac and do nothing on the PC.  These are good for reading
 * PC File structures, but EndianUtils.h contains more powerful stuff.
 *
 */

void	EndianFlipShort(int16_t * ioShort);
void	EndianFlipLong(int32_t * ioLong);




#ifdef __cplusplus
};
#endif

#endif

