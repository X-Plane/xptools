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

#include "WED_FastBuffer.h"
#include "WED_Buffer.h"


void	WED_FastBuffer::ReadShort(short& x)
{
	mSource->ReadShort(x);
}
void	WED_FastBuffer::ReadInt(int& x)
{
	mSource->ReadInt(x);
}
void	WED_FastBuffer::ReadFloat(float& x)
{
	mSource->ReadFloat(x);
}
void	WED_FastBuffer::ReadDouble(double& x)
{
	mSource->ReadDouble(x);
}
void	WED_FastBuffer::ReadBulk(char * inBuf, int inLength, bool inZip)
{
	mSource->ReadBulk(inBuf,inLength,inZip);
}
void	WED_FastBuffer::WriteShort(short x)
{
	mSource->WriteShort(x);
}
void	WED_FastBuffer::WriteInt(int x)
{
	mSource->WriteInt(x);
}
void	WED_FastBuffer::WriteFloat(float x)
{
	mSource->WriteFloat(x);
}
void	WED_FastBuffer::WriteDouble(double x)
{
	mSource->WriteDouble(x);
}
void	WED_FastBuffer::WriteBulk(const char * inBuf, int inLength, bool inZip)
{
	mSource->WriteBulk(inBuf,inLength,inZip);
}

WED_FastBuffer::WED_FastBuffer(WED_Buffer * source)
{
	mSource = source;
	mSource->GetWritePos(mP1, mP2);
}

void	WED_FastBuffer::ResetRead(void)
{
	mSource->SetReadPos(mP1,mP2);
}

WED_FastBufferGroup::WED_FastBufferGroup()
{
}

WED_FastBufferGroup::~WED_FastBufferGroup()
{
}

WED_FastBuffer *		WED_FastBufferGroup::MakeNewBuffer(void)
{
	void * storage = mStorage.AllocContiguous(sizeof(WED_FastBuffer));
	WED_FastBuffer * nb = new (storage) WED_FastBuffer(&mStorage);
	return nb;
}

