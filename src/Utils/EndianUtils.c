/*
 * Copyright (c) 2007, Laminar Research.
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

#include "EndianUtils.h"

/*
 * These are the actual swapping macros.
 *
 */

#ifndef Endian16_Swap
#define Endian16_Swap(value)                 \
        (((((unsigned short)value)<<8) & 0xFF00)   | \
         ((((unsigned short)value)>>8) & 0x00FF))
#endif

#ifndef Endian32_Swap
#define Endian32_Swap(value)                     \
        (((((unsigned long)value)<<24) & 0xFF000000)  | \
         ((((unsigned long)value)<< 8) & 0x00FF0000)  | \
         ((((unsigned long)value)>> 8) & 0x0000FF00)  | \
         ((((unsigned long)value)>>24) & 0x000000FF))
#endif

#ifndef Endian64_Swap
#define Endian64_Swap(value)                                \
                (((((unsigned long long)value)<<56) & 0xFF00000000000000ULL)  | \
                 ((((unsigned long long)value)<<40) & 0x00FF000000000000ULL)  | \
                 ((((unsigned long long)value)<<24) & 0x0000FF0000000000ULL)  | \
                 ((((unsigned long long)value)<< 8) & 0x000000FF00000000ULL)  | \
                 ((((unsigned long long)value)>> 8) & 0x00000000FF000000ULL)  | \
                 ((((unsigned long long)value)>>24) & 0x0000000000FF0000ULL)  | \
                 ((((unsigned long long)value)>>40) & 0x000000000000FF00ULL)  | \
                 ((((unsigned long long)value)>>56) & 0x00000000000000FFULL))
#endif


PlatformType	GetNativePlatformType(void)
{
#if BIG
	return platform_BigEndian;
#elif LIL
	return platform_LittleEndian;
#else
	#error BIG and LIL are both undefined.  Please define one macro to 1 to define platform endian-ness!
#endif
}

void	EndianSwapBuffer(
				PlatformType		inStart,
				PlatformType		inEnd,
				const char *		inFormat,
				void *				ioBuffer)
{
	char *	buf = (char *) ioBuffer;
	if (inStart == platform_Native)
		inStart = GetNativePlatformType();
	if (inEnd == platform_Native)
		inEnd = GetNativePlatformType();

	if (inStart != inEnd)
	{
		while (*inFormat)
		{
			if ((*inFormat) > 0)
			{
				switch(*inFormat) {
				case 2:
					*((short *) buf) =
						Endian16_Swap(*((short *) buf));
					break;
				case 4:
					*((long *) buf) =
						Endian32_Swap(*((long *) buf));
					break;
				case 8:
					*((long long *) buf) =
						Endian64_Swap(*((long long *) buf));
/*				default:
					DebugAssert(!"Bad format for EndianSwapBuffer!");
*/
				}
				buf += (*inFormat);
			} else {
				buf -= (*inFormat);
			}
			++inFormat;
		}
	}
}


void	EndianSwapArray(
				PlatformType		inStart,
				PlatformType		inEnd,
				int					inCount,
				int					inElementSize,
				void *				ioBuffer)
{
	char * buf = (char *) ioBuffer;

	if (inStart == platform_Native)
		inStart = GetNativePlatformType();
	if (inEnd == platform_Native)
		inEnd = GetNativePlatformType();

	if (inStart != inEnd)
	{
		switch(inElementSize) {
		case 2:
			while (inCount--)
			{
				*((short *) buf) = Endian16_Swap(*((short *) buf));
				buf += 2;
			}
			break;
		case 4:
			while (inCount--)
			{
				*((long *) buf) = Endian32_Swap(*((long *) buf));
				buf += 4;
			}
			break;
		case 8:
			while (inCount--)
			{
				*((long long *) buf) = Endian64_Swap(*((long long *) buf));
				buf += 8;
			}
			break;
		}
	}
}

/* Endian routines for the Mac use Apple's Endian macros. */

void	EndianFlipShort(short * ioShort)
{
	#if BIG
		*ioShort = Endian16_Swap(*ioShort);
	#endif
}

void	EndianFlipLong(long * ioLong)
{
	#if BIG
		*ioLong = Endian32_Swap(*ioLong);
	#endif
}
