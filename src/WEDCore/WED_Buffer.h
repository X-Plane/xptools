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

#ifndef WED_BUFFER_H
#define WED_BUFFER_H

/*

	WED_Buffer - THEORY OF OPERATION

	WED_Buffer provides a dynamic memory buffer taht we can stream into and out of using the
	IODefs reader/writer APIs.  Some properties:

	- It is meant to only be written once (although technically we could keep on writing to it.
	- It gives us ways to go back to arbitrary points.

	This buffer allocates fixed size blocks, so the overhead of a single buffer could be a bit
	steep.  Reads and writes are split across buffers.

	AllocContiguous gives us a ptr to a block of memory in the buffer that is guaranteed not
	split; the buffer may waste some space to do this.  The pointer is stable for the life
	of the buffer.

*/

#include "IODefs.h"

#include <list>
#include <vector>
#include <stdint.h>
using std::list;
using std::vector;


class	WED_Buffer : public IOReader, public IOWriter {
public:

					WED_Buffer();
					~WED_Buffer();

//			void	Reserve(unsigned long inBytes);

	// IOReader
	virtual	void	ReadShort(short&);
	virtual	void	ReadInt(int&);
	virtual	void	ReadFloat(float&);
	virtual	void	ReadDouble(double&);
	virtual	void	ReadBulk(char * inBuf, int inLength, bool inZip);

	// IOWriter
	virtual	void	WriteShort(short);
	virtual	void	WriteInt(int);
	virtual	void	WriteFloat(float);
	virtual	void	WriteDouble(double);
	virtual	void	WriteBulk(const char * inBuf, int inLength, bool inZip);

			void	ResetRead(void);				// Resets reading to the beginning.
			void *	AllocContiguous(int len);		// Allocates a chunk of memory that is not split across buffers.
			void	GetWritePos(intptr_t& a, intptr_t& b);	// Get and set arbitrary positions as two ints.
			void	SetReadPos(uintptr_t a, int b);

private:

			void	ReadInternal (char * p, unsigned long l);
			void	WriteInternal(const char * p, unsigned long l);

	struct						Storage;

	Storage *					mStorage;			// Ptr to the slist of blocks.
	Storage *					mWriteIterator;		// Ptr to the block we are writing in now.

	Storage *					mReadIterator;		// Ptr to the block we are reading and its offset.
	int							mReadSubpos;

					WED_Buffer(const WED_Buffer& rhs);
	WED_Buffer&		operator=(const WED_Buffer& rhs);

};


#endif /* WED_BUFFER_H */
