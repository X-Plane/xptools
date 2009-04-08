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
#include "XChunkyFileUtils.h"
#include "AssertUtils.h"
#include <vector>
#include <string.h>

// BENTODO - resolve hl_types dependency!!
//#if !defined(APL) && !defined(IBM)
//#include "hl_types.h"
//#endif

using std::vector;

inline short	SwapValueTyped(int16_t v) { return (int16_t) SWAP16(v); }
inline int		SwapValueTyped(int32_t v) { return (int32_t) SWAP32(v); }
inline float	SwapValueTyped(float v	) { return (float  ) SWAP32(v); }
inline double	SwapValueTyped(double v	) { return (double ) SWAP64(v); }

#pragma mark class FlatDecoder

template <class T>
class	FlatDecoder {
public:

	unsigned char * p;

	FlatDecoder(unsigned char * mem) : p(mem)
	{
	}

	T	Fetch(void)
	{
		T retval = *((T *) p);
		p += sizeof(T);
		return retval;
	}

	unsigned char * EndPos(void)
	{
		return p;
	}
};

#pragma mark class RLEDecoder
template <class T>
class	RLEDecoder {
public:

	unsigned char *	p;
	bool	is_run;
	bool	is_individual;
	int		run_length;

	RLEDecoder(unsigned char * mem)
	{
		p = mem;
		is_run = is_individual = false;
		run_length = 0;
	}

	T	Fetch(void)
	{
		T	retVal;
		if (is_run)
		{
			retVal = *((T *) p);
			if (run_length == 1)
			{
				p += sizeof(T);
				is_run = false;
			} else
				--run_length;
			return retVal;
		}
		else if (is_individual)
		{
			retVal = *((T *) p);
			p += sizeof(T);
			if (run_length == 1)
				is_individual = false;
			else
				--run_length;
			return retVal;
		}
		else
		{
			unsigned char code = *p++;
			if (code & 0x80)
			{
				is_run = true;
			} else {
				is_individual = true;
			}
			run_length = code & 0x7F;
			return Fetch();
		}
	}

	unsigned char *	EndPos(void)
	{
		return p;
	}
};


#pragma mark class FlatEncoder
template <class T>
class	FlatEncoder {
public:

		FILE *		file;

	FlatEncoder(FILE * inFile) : file(inFile)
	{
	}

	void Accum(T value)
	{
		fwrite(&value, sizeof(value), 1, file);
	}

	void Done(void)
	{
	}
};

#pragma mark class RLEEncoder
template <class T>
class	RLEEncoder {
public:

	// A note on how this encoder works: it can be in one of four states:
	// having no data and neutral, having one item and neutral, or having
	// two or more items and being in a heterogenous or homogenous run.

		FILE *		file;
		vector<T>	run;
		bool		is_run;
		bool		is_individual;
		int			run_length;

	RLEEncoder(FILE * inFile)
	{
		file = inFile;
		run_length = 0;
		is_run = false;
		is_individual = false;
	}

	void Accum(T value)
	{
		unsigned char	token;
		T				item;

		if (is_run)
		{
			if (value == run.back())
			{
				if (run_length == 127)
				{
					// Run is max length - emit the run and go to neutral
					// with this one item.
					token = 0x80 | run_length;
					fwrite(&token, sizeof(token), 1, file);
					item = run[0];
					fwrite(&item, sizeof(item), 1, file);
					is_run = false;
					run.clear();
					run.push_back(value);
				} else {
					// Accum onto run
					++run_length;
				}
			} else {
				// Emit the run, accum this one, but stay neutral
				token = 0x80 | run_length;
				fwrite(&token, sizeof(token), 1, file);
				item = run[0];
				fwrite(&item, sizeof(item), 1, file);
				is_run = false;
				run.clear();
				run.push_back(value);
			}

		} else if (is_individual)
		{
			if (value != run.back())
			{
				if (run.size() == 127)
				{
					// The run is too long.  Emit,
					// go to neutral with this one item.
					token = run.size();
					fwrite(&token, sizeof(token), 1, file);
					fwrite(&*run.begin(), sizeof(T), run.size(), file);
					is_individual = false;
					run.clear();
					run.push_back(value);
				} else {
					// Accum this onto the run
					run.push_back(value);
				}
			} else {
				// This case is funky: we have a set of individual
				// items and the last matches this new one.  We are
				// going to use that last item from the individual run
				// as the start of a homogenous run.  Why?  Well, first
				// of all we know we can.  Any defined individual run has
				// at least two items.  And since virtually all of the items
				// we store are bigger than a char, it's always cheaper to
				// make a homogenous run even if it's only two items, AND
				// even if it orphans a one-item individual run too.
				// In other words, for 32-bit and larger items, this is good:
				// 1 A 2' B 1 C 2' D for ABBCDD (28 bytes compressed into 20).

				// Pop one off of the run
				// Emit the run
				// Set up a new run with these two items

				run.pop_back();
				token = run.size();
				fwrite(&token, sizeof(token), 1, file);
				fwrite(&*run.begin(), sizeof(T), run.size(), file);
				is_individual = false;
				is_run = true;
				run.clear();
				run.push_back(value);
				run_length = 2;

			}
		} else {
			if (!run.empty())
			{
				if (run.back() == value)
				{
					// start a homgenous run from neutural
					is_run = true;
					run_length = 2;
				} else {
					// start a heterogeneous run neutral
					run.push_back(value);
					is_individual = true;
				}
			} else {
				// Accum this one, stay neutral
				run.push_back(value);
			}
		}

	}

	void Done(void)
	{
		unsigned char token;
		T				item;

		if (is_run)
		{
			// dump the run
			token = 0x80 | run_length;
			fwrite(&token, sizeof(token), 1, file);
			item = run[0];
			fwrite(&item, sizeof(item), 1, file);

		} else if (is_individual) {
			// dump the run
			token = run.size();
			fwrite(&token, sizeof(token), 1, file);
			fwrite(&*run.begin(), sizeof(T), run.size(), file);
		} else if (!run.empty()) {
			// make a one-item individual run
			token = run.size();
			fwrite(&token, sizeof(token), 1, file);
			fwrite(&*run.begin(), sizeof(T), run.size(), file);
		}
	}

};






#pragma mark -

XSpan::XSpan() :
	begin(NULL), end(NULL)
{
}


uint32_t	XAtom::GetID(void)
{
	if (begin == NULL) return 0;
	XAtomHeader_t *	header = (XAtomHeader_t *) begin;
	return SWAP32(header->id);
}

uint32_t	XAtom::GetContentLength(void)
{
	if (begin == NULL) return 0;
	XAtomHeader_t *	header = (XAtomHeader_t *) begin;
	return SWAP32(header->length) - sizeof(XAtomHeader_t);
}

uint32_t	XAtom::GetContentLengthWithHeader(void)
{
	if (begin == NULL) return 0;
	XAtomHeader_t *	header = (XAtomHeader_t *) begin;
	return SWAP32(header->length);
}

void			XAtom::GetContents(XSpan& outContents)
{
	outContents.begin = begin + sizeof(XAtomHeader_t);
	outContents.end = end;
}

bool			XAtom::GetNext(const XSpan& inContainer, XAtom& outNext)
{
	// Check for end of file and also overruns!
	if (end >= inContainer.end)
		return false;

	outNext.begin = end;
	outNext.end = outNext.begin + sizeof(XAtomHeader_t);

	// Make sure we have a real length...if our length is bogus (or worse
	// zero) we're hosed, bail now.  We use a special accessor that doesn't
	// subtract to avoid wrap-around issues.
	if (outNext.GetContentLengthWithHeader() < sizeof(XAtomHeader_t))
		return false;

	outNext.end += outNext.GetContentLength();
	return true;
}

bool	XAtomContainer::GetFirst(XAtom& outAtom)
{
	if (begin == end) return false;
	// I don't have a header - I'm a a span, dude!
	outAtom.begin = begin;
	// Set up an 'empty atom'
	outAtom.end = outAtom.begin + sizeof(XAtomHeader_t);
	// And fix its length
	outAtom.end += outAtom.GetContentLength();
	return true;
}

int		XAtomContainer::CountAtoms(void)
{
	int n = 0;
	XAtom	atom, next;
	if (!GetFirst(atom))
		return 0;
	do {
		++n;
		if (!atom.GetNext(*this, next))
			break;
		atom = next;
	} while (1);

	return n;
}

int 	XAtomContainer::CountAtomsOfID(uint32_t inID)
{
	int n = 0;
	XAtom	atom, next;
	if (!GetFirst(atom))
		return 0;
	do {
		if (atom.GetID() == inID)
			++n;
		if (!atom.GetNext(*this, next))
			break;
		atom = next;
	} while (1);

	return n;
}

bool	XAtomContainer::GetNthAtom(int inIndex, XAtom& outAtom)
{
	XAtom	next;
	if (!GetFirst(outAtom))
		return false;
	do {
		if (inIndex == 0)
			return true;
		--inIndex;
		if (!outAtom.GetNext(*this, next))
			break;
		outAtom = next;
	} while (1);

	return false;
}

bool	XAtomContainer::GetNthAtomOfID(uint32_t inID, int inIndex, XAtom& outAtom)
{
	XAtom	next;
	if (!GetFirst(outAtom))
		return false;
	do {
		if (outAtom.GetID() == inID)
		{
			if (inIndex == 0)
				return true;
			--inIndex;
		}
		if (!outAtom.GetNext(*this, next))
			break;
		outAtom = next;
	} while (1);

	return false;
}




const char *	XAtomStringTable::GetFirstString(void)
{
	char * str = begin + sizeof(XAtomHeader_t);
	if (str == end)
		return NULL;
	return str;
}

const char *	XAtomStringTable::GetNextString(const char * inString)
{
	if (inString == NULL) return NULL;
	int len = strlen(inString);
	const char * str = inString + len + 1;
	DebugAssert(str <= end);
	if (str == end)
		return NULL;
	return str;
}

const char *	XAtomStringTable::GetNthString(int inIndex)
{
	const char * iter = GetFirstString();
	while (iter)
	{
		if (inIndex == 0)
			break;
		inIndex--;
		iter = GetNextString(iter);
	}
	return iter;
}


int	XAtomPlanerNumericTable::GetArraySize(void)
{
	char *	contents = begin + sizeof(XAtomHeader_t);
	return SWAP32(*((int *) contents));
}

int	XAtomPlanerNumericTable::GetPlaneCount(void)
{
	char *	contents = begin + sizeof(XAtomHeader_t) + sizeof(int);
	return *((unsigned char *) contents);
}

template<class T>
static int DecodeNumericPlane(
						int 					inPlaneCount,
						int						inPlaneSize,
						int						inInterleaved,
						unsigned char *			inAtomData,
						unsigned char *			inAtomDataEnd,
						T *						ioPlane)

{
	int i, plane;
	T last, val;
	for (plane = 0; plane < inPlaneCount; ++plane)
	{
		if (inAtomData >= inAtomDataEnd) return plane;
		unsigned char	encodeMode = *inAtomData++;
		if (encodeMode == xpna_Mode_Raw)
		{
			FlatDecoder<T>	decoder(inAtomData);
			for (i = 0; i < inPlaneSize; ++i)
			{
				if (inInterleaved)
					ioPlane[i * inPlaneCount + plane] = SwapValueTyped(decoder.Fetch());
				else
					ioPlane[plane * inPlaneSize + i] = SwapValueTyped(decoder.Fetch());
			}
			inAtomData = decoder.EndPos();
		}
		if (encodeMode == xpna_Mode_Differenced)
		{
			FlatDecoder<T>	decoder(inAtomData);
			last = 0;
			for (i = 0; i < inPlaneSize; ++i)
			{
				if (inInterleaved)
					ioPlane[i * inPlaneCount + plane] = val = last + SwapValueTyped(decoder.Fetch());
				else
					ioPlane[plane * inPlaneSize + i] = val = last + SwapValueTyped(decoder.Fetch());
				last = val;
			}
			inAtomData = decoder.EndPos();
		}
		if (encodeMode == xpna_Mode_RLE)
		{
			RLEDecoder<T>	decoder(inAtomData);
			for (i = 0; i < inPlaneSize; ++i)
			{
				if (inInterleaved)
					ioPlane[i * inPlaneCount + plane] = SwapValueTyped(decoder.Fetch());
				else
					ioPlane[plane * inPlaneSize + i] = SwapValueTyped(decoder.Fetch());
			}
			inAtomData = decoder.EndPos();
		}
		if (encodeMode == xpna_Mode_RLE_Differenced)
		{
			RLEDecoder<T>	decoder(inAtomData);
			last = 0;
			for (i = 0; i < inPlaneSize; ++i)
			{
				if (inInterleaved)
					ioPlane[i * inPlaneCount + plane] = val = last + SwapValueTyped(decoder.Fetch());
				else
					ioPlane[plane * inPlaneSize + i] = val = last + SwapValueTyped(decoder.Fetch());
				last = val;
			}
			inAtomData = decoder.EndPos();
		}
	}
	return inPlaneCount;
}

int	XAtomPlanerNumericTable::DecompressShort(
					int		numberOfPlanes,
					int		planeSize,
					int		interleaved,
					int16_t * ioPlaneBuffer)
{
	return DecodeNumericPlane(numberOfPlanes, planeSize, interleaved,
						(unsigned char *) begin + sizeof(XAtomHeader_t) + sizeof(int) + sizeof(char), (unsigned char *) end,
						ioPlaneBuffer);
}


int	XAtomPlanerNumericTable::DecompressInt(
					int		numberOfPlanes,
					int		planeSize,
					int		interleaved,
					int32_t * ioPlaneBuffer)
{
	return DecodeNumericPlane(numberOfPlanes, planeSize, interleaved,
						(unsigned char *) begin + sizeof(XAtomHeader_t) + sizeof(int) + sizeof(char), (unsigned char *) end,
						ioPlaneBuffer);
}

int	XAtomPlanerNumericTable::DecompressFloat(
					int		numberOfPlanes,
					int		planeSize,
					int		interleaved,
					float * ioPlaneBuffer)
{
	return DecodeNumericPlane(numberOfPlanes, planeSize, interleaved,
						(unsigned char *) begin + sizeof(XAtomHeader_t) + sizeof(int) + sizeof(char), (unsigned char *) end,
						ioPlaneBuffer);
}
int	XAtomPlanerNumericTable::DecompressDouble(
					int		numberOfPlanes,
					int		planeSize,
					int		interleaved,
					double * ioPlaneBuffer)
{
	return DecodeNumericPlane(numberOfPlanes, planeSize, interleaved,
						(unsigned char *) begin + sizeof(XAtomHeader_t) + sizeof(int) + sizeof(char), (unsigned char *) end,
						ioPlaneBuffer);
}



#pragma mark -

StAtomWriter::StAtomWriter(FILE * inFile, uint32_t inID)
{
	mID = inID;
	mFile = inFile;
//	fflush(mFile);
	mAtomStart = ftell(inFile);
	XAtomHeader_t	header;
	header.id = SWAP32(inID);
	header.length = SWAP32(8);
	fwrite(&header, sizeof(header), 1, inFile);
}

StAtomWriter::~StAtomWriter()
{
//	fflush(mFile);
	int end_of_atom = ftell(mFile);
	int len = end_of_atom - mAtomStart;
	fseek(mFile, mAtomStart, SEEK_SET);
	XAtomHeader_t	header;
	header.id = SWAP32(mID);
	header.length = SWAP32(len);
	fwrite(&header, sizeof(header), 1, mFile);
	fseek(mFile, end_of_atom, SEEK_SET);
}


template <class T>
void	WritePlanarNumericAtom(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							T *		ioData)
{
	T	value, diff, last;

	int	psize = SWAP32(planeSize);
	unsigned char nplanes = numberOfPlanes;
	fwrite(&psize, sizeof(psize), 1, file);
	fwrite(&nplanes, sizeof(nplanes), 1, file);

	for (int pln = 0; pln < numberOfPlanes; ++pln)
	{
		unsigned char encode = encodeMode;
		fwrite(&encode, sizeof(encode), 1, file);
		if (encodeMode == xpna_Mode_Raw)
		{
			FlatEncoder<T>	encoder(file);
			for (int i = 0; i < planeSize; ++i)
			{
				value = SwapValueTyped(interleaved ?
								(ioData[i * numberOfPlanes + pln]) :
								(ioData[pln * planeSize + i]));
				encoder.Accum(value);
			}
			encoder.Done();
		}
		if (encodeMode == xpna_Mode_Differenced)
		{
			FlatEncoder<T>	encoder(file);
			last = 0;
			for (int i = 0; i < planeSize; ++i)
			{
				value = (interleaved ?
								(ioData[i * numberOfPlanes + pln]) :
								(ioData[pln * planeSize + i]));

				diff = SwapValueTyped((T)(value - last));
				encoder.Accum(diff);
				last = value;
			}
			encoder.Done();
		}
		if (encodeMode == xpna_Mode_RLE)
		{
			RLEEncoder<T>	encoder(file);
			for (int i = 0; i < planeSize; ++i)
			{
				value = SwapValueTyped(interleaved ?
								(ioData[i * numberOfPlanes + pln]) :
								(ioData[pln * planeSize + i]));
				encoder.Accum(value);
			}
			encoder.Done();
		}
		if (encodeMode == xpna_Mode_RLE_Differenced)
		{
			RLEEncoder<T>	encoder(file);
			last = 0;
			for (int i = 0; i < planeSize; ++i)
			{
				value = (interleaved ?
								(ioData[i * numberOfPlanes + pln]) :
								(ioData[pln * planeSize + i]));

				diff = SwapValueTyped((T)(value - last));
				encoder.Accum(diff);
				last = value;
			}
			encoder.Done();
		}
	}
}

void	WritePlanarNumericAtomShort(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							int16_t *	ioData)
{
	WritePlanarNumericAtom(file, numberOfPlanes, planeSize, encodeMode, interleaved, ioData);
}

void	WritePlanarNumericAtomInt(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							int32_t *	ioData)
{
	WritePlanarNumericAtom(file, numberOfPlanes, planeSize, encodeMode, interleaved, ioData);
}

void	WritePlanarNumericAtomFloat(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							float *	ioData)
{
	WritePlanarNumericAtom(file, numberOfPlanes, planeSize, encodeMode, interleaved, ioData);
}

void	WritePlanarNumericAtomDouble(
							FILE *	file,
							int		numberOfPlanes,
							int		planeSize,
							int		encodeMode,
							int		interleaved,
							double *	ioData)
{
	WritePlanarNumericAtom(file, numberOfPlanes, planeSize, encodeMode, interleaved, ioData);
}


//#erro TODO: rewrite decoder to take interleaved param and do swapping, always work one at a time!

void			WriteUInt8  (FILE * fi, unsigned char	v)
{
	fwrite(&v, 1, sizeof(v), fi);
}

void			WriteSInt8  (FILE * fi, 		 char	v)
{
	fwrite(&v, 1, sizeof(v), fi);
}

void			WriteUInt16 (FILE * fi, uint16_t	v)
{
	v = SWAP16(v);
	fwrite(&v, 1, sizeof(v), fi);
}

void			WriteSInt16 (FILE * fi, 		int16_t	v)
{
	v = SWAP16(v);
	fwrite(&v, 1, sizeof(v), fi);
}

void			WriteUInt32 (FILE * fi, uint32_t	v)
{
	v = SWAP32(v);
	fwrite(&v, 1, sizeof(v), fi);
}

void			WriteSInt32 (FILE * fi, 		 int32_t	v)
{
	v = SWAP32(v);
	fwrite(&v, 1, sizeof(v), fi);
}

void			WriteFloat32(FILE * fi, float			v)
{
	*((int *) &v) = SWAP32(*((int32_t *) &v));
	fwrite(&v, 1, sizeof(v), fi);
}

void			WriteFloat64(FILE * fi, double			v)
{
	*((long long *) &v) = SWAP64(*((int64_t *) &v));
	fwrite(&v, 1, sizeof(v), fi);
}
