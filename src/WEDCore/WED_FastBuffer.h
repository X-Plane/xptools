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

#ifndef WED_FastBuffer_H
#define WED_FastBuffer_H

/*

	WED_FastBuffer - THEORY OF OPERATION
	
	Well, it's not that fast.  Basically WED_FastBuffer and WED_FastBufferPool are a subdivision API off of WED_Buffer.  
	
	The problem with WED_Buffer is that it has to allocate big chunks to be fast, and that's wasteful of memory.
	The fast buffer interface lets us carve off many buffers with the following rules:
	
	- All buffers are deallocated at once by destroying the fast buffer pool -- individual buffers cannot be freed
	  (and thus not recycled).
	- Writing to one buffer must STOP before the next buffer is created!!
	
	The fast buffer pool uses a single wed-buffer for its underlying storage, limiting wastage due to big blocks.  Each
	fast buffer has its object actually allocated from the mass storage and then its stream follows.
	
	Each fast buffer knows the stream position of its data.

*/

#include "IODefs.h"
#include "WED_Buffer.h"

class	WED_FastBuffer : public IOReader, public IOWriter {
public:

	virtual	void	ReadShort(short&);
	virtual	void	ReadInt(int&);
	virtual	void	ReadFloat(float&);
	virtual	void	ReadDouble(double&);
	virtual	void	ReadBulk(char * inBuf, int inLength, bool inZip);
	
	virtual	void	WriteShort(short);
	virtual	void	WriteInt(int);
	virtual	void	WriteFloat(float);
	virtual	void	WriteDouble(double);
	virtual	void	WriteBulk(const char * inBuf, int inLength, bool inZip);

			void	ResetRead(void);

private:

	friend	class WED_FastBufferGroup;
	
					WED_FastBuffer(WED_Buffer * source);
	
			long					mP1, mP2;
			WED_Buffer *			mSource;

					WED_FastBuffer();
					WED_FastBuffer(const WED_FastBuffer& rhs);
					~WED_FastBuffer();

	WED_FastBuffer&	operator=(const WED_FastBuffer& rhs);

};	

class	WED_FastBufferGroup {
public:

							WED_FastBufferGroup();
							~WED_FastBufferGroup();

	WED_FastBuffer *		MakeNewBuffer(void);

private:

	WED_Buffer			mStorage;

	WED_FastBufferGroup&	operator=(const WED_FastBufferGroup& rhs);
							WED_FastBufferGroup(const WED_FastBufferGroup& rhs);

};

#endif /* WED_FastBuffer_H */
