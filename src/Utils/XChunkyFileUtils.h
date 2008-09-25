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
#ifndef XCHUNKYFILEUTILS_H
#define XCHUNKYFILEUTILS_H

#include <stdio.h>

#if BIG
	#if APL
		#if defined (__MACH__)
			#include <libkern/OSByteOrder.h>
			#define SWAP16(x) (OSSwapConstInt16(x))
			#define SWAP32(x) (OSSwapConstInt32(x))
			#define SWAP64(x) (OSSwapConstInt64(x))
		#else
			#include <Endian.h>
			#define SWAP16(x) (Endian16_Swap(x))
			#define SWAP32(x) (Endian32_Swap(x))
			#define SWAP64(x) (Endian64_Swap(x))
		#endif
	#else
		#error we do not have non-apple big endian swapping routines.
	#endif
#elif LIL
	#define SWAP16(x) (x)
	#define SWAP32(x) (x)
	#define SWAP64(x) (x)
#else
	#error endian not defined
#endif

#if APL
#pragma options align=mac68k
#endif
#if IBM
#pragma pack(push, 2)
#endif

struct	XAtomHeader_t {
	unsigned long	id;
	unsigned long	length;
};


#if APL
#pragma options align=reset
#endif
#if IBM
#pragma pack(pop)
#endif


enum {
	xpna_Mode_Raw = 0,
	xpna_Mode_Differenced = 1,
	xpna_Mode_RLE = 2,
	xpna_Mode_RLE_Differenced = 3
};


/********************************************************************************
 * CHUNKY FILE READING UTILITIES
 ********************************************************************************
 * All of our reading utilities work on memory - we just use XMappedFile to memory-
 * map the file and away we go.
 *
 */

/*
 * XSpan - this is just a range of memory.  'begin' points to the first byte,
 * and 'end' points to one byte AFTER the last byte in the span.  Its length is
 * end - begin.
 *
 */
struct	XSpan {

	XSpan();

	char *	begin;
	char *	end;
};

/*
 * An atom is a span...the first 8 bytes are the header, and the rest
 * are the contents.  The contents can be returned as a span, which
 * can be handy...
 *
 */
struct	XAtom : public XSpan {

	unsigned long	GetID(void);
	unsigned long	GetContentLength(void);
	unsigned long	GetContentLengthWithHeader(void);
	void			GetContents(XSpan& outContents);

	bool			GetNext(const XSpan& inContainer, XAtom& outNext);

};

/*
 * An atom container is a span as well...it is simply the memory for
 * all of the atoms in a row.
 * From this we can extract individual atoms.
 *
 */
struct	XAtomContainer : public XSpan {

	bool	GetFirst(XAtom& outAtom);

	int		CountAtoms(void);
	int 	CountAtomsOfID(unsigned long inID);

	bool	GetNthAtom(int inIndex, XAtom& outAtom);
	bool	GetNthAtomOfID(unsigned long inID, int inIndex, XAtom& outAtom);

};

/*
 * An atom of null-terminated C strings.
 *
 */
struct	XAtomStringTable : public XAtom {

	const char *	GetFirstString(void);
	const char *	GetNextString(const char * inString);
	const char *	GetNthString(int inIndex);

};


/*
 * An atom of compressed numeric data.
 *
 */
struct	XAtomPlanerNumericTable : public XAtom {

	int		GetArraySize(void);
	int		GetPlaneCount(void);

	/* These routines decompress the data into a set of planes.
	 * They return the number of planes filled, but will never
	 * exceed numberOfPlanes. */
	int		DecompressShort(
						int		numberOfPlanes,
						int		planeSize,
						int		interleaved,
						short * ioPlaneBuffer);
	int		DecompressInt(
						int		numberOfPlanes,
						int		planeSize,
						int		interleaved,
						int * ioPlaneBuffer);
	int		DecompressFloat(
						int		numberOfPlanes,
						int		planeSize,
						int		interleaved,
						float * ioPlaneBuffer);
	int		DecompressDouble(
						int		numberOfPlanes,
						int		planeSize,
						int		interleaved,
						double * ioPlaneBuffer);

};

/*
 * An atom of packed data...useful for reading by type
 * and dealing with endian swaps.
 *
 */
struct	XAtomPackedData : public XAtom {

	void	Reset(void)			{ position = begin + sizeof(XAtomHeader_t); }
	bool	Done(void)			{ return position >= end;					}
	bool	Overrun(void)		{ return position > end;					}

	unsigned char				ReadUInt8 (void)	{ unsigned char  v = *((unsigned char *	) position); position += sizeof(v); return v; 		  }
			 char				ReadSInt8 (void)	{ char 			 v = *((char *			) position); position += sizeof(v);	return v; 		  }
	unsigned short				ReadUInt16(void)	{ unsigned short v = *((unsigned short *) position); position += sizeof(v);	return SWAP16(v); }
			 short				ReadSInt16(void)	{ short 		 v = *((short *		    ) position); position += sizeof(v);	return SWAP16(v); }
	unsigned int				ReadUInt32(void)	{ unsigned int   v = *((unsigned int *	) position); position += sizeof(v);	return SWAP32(v); }
			 int				ReadSInt32(void)	{ int 			 v = *((int *		  	) position); position += sizeof(v);	return SWAP32(v); }
			 float				ReadFloat32(void)	{ float 		 v = *((float *			) position); *((int *	   ) &v) = SWAP32(*((int *		) &v));	position += sizeof(v);	return v; }
			 double				ReadFloat64(void) 	{ double 		 v = *((double *		) position); *((long long *) &v) = SWAP32(*((long long *) &v));	position += sizeof(v);	return v; }

	void						Advance(int bytes)	{ position += bytes; }

	char *		position;

};


/********************************************************************************
 * CHUNKY FILE WRITING UTILITIES
 ********************************************************************************
 *
 *
 */

struct	StAtomWriter {
	StAtomWriter(FILE * inFile, int inID);
	~StAtomWriter();

	FILE *		mFile;
	int			mAtomStart;
	int			mID;
};

void	WritePlanarNumericAtomShort(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							short *	ioData);

void	WritePlanarNumericAtomInt(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							int *	ioData);

void	WritePlanarNumericAtomFloat(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							float *	ioData);

void	WritePlanarNumericAtomDouble(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							double *	ioData);

void			WriteUInt8  (FILE * fi, unsigned char	v);
void			WriteSInt8  (FILE * fi, 		 char	v);
void			WriteUInt16 (FILE * fi, unsigned short	v);
void			WriteSInt16 (FILE * fi, 		 short	v);
void			WriteUInt32 (FILE * fi, unsigned int	v);
void			WriteSInt32 (FILE * fi, 		 int	v);
void			WriteFloat32(FILE * fi, float			v);
void			WriteFloat64(FILE * fi, double			v);


#endif
