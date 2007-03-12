#include "WED_Buffer.h"
#include "AssertUtils.h"

#define	WED_BUFFER_ALLOC_SIZE	1024

WED_Buffer::WED_Buffer()
{
	mReadIterator = mStorage.begin();
	mReadSubpos = 0;
}

WED_Buffer::WED_Buffer(const WED_Buffer& rhs) : mStorage(rhs.mStorage)
{
	// Subpositions are NOT copied when we clone a buffer.
	// TODO: would be a lot nicer to just declare an iterator to this or 
	// a reader stream class??
	mReadIterator = mStorage.begin();
	mReadSubpos = 0;
}

WED_Buffer::~WED_Buffer()
{
}

WED_Buffer&		WED_Buffer::operator=(const WED_Buffer& rhs)
{
	mStorage = rhs.mStorage;
	mReadIterator = mStorage.begin();
	mReadSubpos = 0;
	return *this;
}

void	WED_Buffer::Reserve(unsigned long inBytes)
{
	mStorage.push_back(Block());
	mStorage.back().reserve(inBytes);
	
}

void	WED_Buffer::ResetRead(void)
{
	mReadIterator = mStorage.begin();
	mReadSubpos = 0;	
}

void	WED_Buffer::ReadShort(short& v)
{
	ReadInternal((char *) &v, sizeof(v));
}
void	WED_Buffer::ReadInt(int& v)
{
	ReadInternal((char *) &v, sizeof(v));
}

void	WED_Buffer::ReadFloat(float& v)
{
	ReadInternal((char *) &v, sizeof(v));
}

void	WED_Buffer::ReadDouble(double& v)
{
	ReadInternal((char *) &v, sizeof(v));
}

void	WED_Buffer::ReadBulk(char * inBuf, int inLength, bool inZip)
{
	ReadInternal(inBuf, inLength);
}

void	WED_Buffer::WriteShort(short v)
{
	WriteInternal((const char *) &v, sizeof(v));
}

void	WED_Buffer::WriteInt(int v)
{
	WriteInternal((const char *) &v, sizeof(v));
}

void	WED_Buffer::WriteFloat(float v)
{
	WriteInternal((const char *) &v, sizeof(v));
}

void	WED_Buffer::WriteDouble(double v)
{
	WriteInternal((const char *) &v, sizeof(v));
}

void	WED_Buffer::WriteBulk(const char * inBuf, int inLength, bool inZip)
{
	WriteInternal(inBuf, inLength);
}

void	WED_Buffer::ReadInternal(char* p, unsigned long l)
{
	while (l > 0)
	{
		// How much storage do we have left in this block.  If we're not at the end,
		// should be non-zero because we never have a zero-length block and we shouldn't
		// leave our subpos at the end.
		DebugAssert(mReadIterator == mStorage.end() || mReadIterator->size() != mReadSubpos);
		unsigned long a = mReadIterator == mStorage.end() ? 0 : mReadIterator->size() - mReadSubpos;

		// Quick check for out of data.
		DebugAssert(a > 0);
		if (a == 0) 
			break;
			
		// Read the minimum of what we want or what we have in this blokc
		unsigned long r = l > a ? a : l;
		memcpy(p, &*mReadIterator->begin() + mReadSubpos, r);
		mReadSubpos += r;
		l -= r;
		p += r;
		
		// If we got to the end of the block, go to the next one.
		if (mReadIterator->size() == mReadSubpos)
		{
			++mReadIterator;
			mReadSubpos = 0;
		}
	}
}

void	WED_Buffer::WriteInternal(const char * p, unsigned long l)
{
	while (l > 0)
	{
		unsigned long a = mStorage.empty() ? 0 : mStorage.back().capacity() - mStorage.back().size();
		if (a > 0)
		{
			unsigned long w = l > a ? a : l;
			mStorage.back().insert(mStorage.back().end(), p, p + w);
//			memcpy(&*mStorage.back().end(), p, w);
			p += w;
			l -= w;
			
		} else {
			mStorage.push_back(Block());
			mStorage.back().reserve(WED_BUFFER_ALLOC_SIZE);
		}
	}
}
