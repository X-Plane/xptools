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
#ifndef IODEFS_H
#define IODEFS_H

class	IOReader {
public:

	virtual	void	ReadShort(short&)=0;
	virtual	void	ReadInt(int&)=0;
	virtual	void	ReadFloat(float&)=0;
	virtual	void	ReadDouble(double&)=0;
	virtual	void	ReadBulk(char * inBuf, int inLength, bool inZip)=0;

};

class	IOWriter {
public:

	virtual	void	WriteShort(short)=0;
	virtual	void	WriteInt(int)=0;
	virtual	void	WriteFloat(float)=0;
	virtual	void	WriteDouble(double)=0;
	virtual	void	WriteBulk(const char * inBuf, int inLength, bool inZip)=0;

};

#endif
