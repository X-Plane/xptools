#include "WED_Buffer.h"
#include "AssertUtils.h"

#define	WED_BUFFER_ALLOC_SIZE	1024

struct WED_Buffer::Storage {
	Storage *		next;			// ptr to next storage block
	int				capacity;		// max number of bytes not including header
	int				size;			// size of bytes really used
	char			data[0];		// effective location of first byte.
};

WED_Buffer::WED_Buffer()
{
	mStorage = NULL;
	mReadIterator = NULL;
	mReadSubpos = 0;
	mWriteIterator = NULL;
}

WED_Buffer::~WED_Buffer()
{
	while (mStorage)
	{
		Storage * block = mStorage;
		mStorage = mStorage->next;
		delete [] mStorage;
	}
}

void	WED_Buffer::ResetRead(void)
{
	mReadIterator = mStorage;
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
		unsigned long a = mReadIterator == NULL ? 0 : mReadIterator->size - mReadSubpos;

		// Quick check for out of data.
		DebugAssert(a > 0 || mReadIterator->next);

		
		if (a > 0)
		{
			// Read the minimum of what we want or what we have in this blokc
			unsigned long r = l > a ? a : l;
			memcpy(p, mReadIterator->data + mReadSubpos, r);
			mReadSubpos += r;
			l -= r;
			p += r;
		}
			
		// If we got to the end of the block, go to the next one.
		if (mReadIterator->size == mReadSubpos)
		{
			mReadIterator = mReadIterator->next;;
			mReadSubpos = 0;
		}
	}
}

void	WED_Buffer::WriteInternal(const char * p, unsigned long l)
{
	while (l > 0)
	{
		unsigned long a = mStorage == NULL ? 0 : mWriteIterator->capacity - mWriteIterator->size;
		if (a > 0)
		{
			unsigned long w = l > a ? a : l;
			memcpy(mWriteIterator->data + mWriteIterator->size, p, w);
			p += w;
			l -= w;
			mWriteIterator->size += w;
			
		} else {
			Storage * block = (Storage *) new char[WED_BUFFER_ALLOC_SIZE];
			block->next = NULL;
			block->capacity = WED_BUFFER_ALLOC_SIZE - sizeof(Storage);
			block->size = 0;
			if (mStorage)
				mWriteIterator->next = block;
			else
				mStorage = block;
			mWriteIterator = block;
		}
	}
}

void *	WED_Buffer::AllocContiguous(int len)
{
	// How much capacity do we already have?
	unsigned long a = mStorage == NULL ? 0 : mWriteIterator->capacity - mWriteIterator->size;
	if (a >= len)
	{
		// If our block has enough space, simply cut off an area.
		void * p = (mWriteIterator->data + mWriteIterator->size);
		mWriteIterator->size += len;
		return p;
	}
	
	// If we don't have enough space, waste the rest, add a new block at least big enough.
	// Note that we do NOT resize the old vector because doing so would move its location and
	// we NEVER want to reallocte the block, because we want to keep ptrs legit.
	int req_len = len + sizeof(Storage);
	int alloc_size = req_len > WED_BUFFER_ALLOC_SIZE ? req_len : WED_BUFFER_ALLOC_SIZE;
	Storage * buf = (Storage *) new char[alloc_size];
	buf->next = NULL;
	if (mStorage == NULL)	mStorage = buf;
	else					mWriteIterator->next = buf;
	buf->capacity = alloc_size - sizeof(Storage);
	buf->size = len;
	mWriteIterator = buf;
	return buf->data;
}

void WED_Buffer::GetWritePos(int& a, int& b)
{
	a = (int) mWriteIterator;
	// Careful: if we still have spare space left, the next write will go into THIS last buffer,
	// otherwise a new one gets chopped.  This is correct for normal writes but NOT for contigous alloc.
	// That's okay.
	b = mWriteIterator ? mWriteIterator->size : 0;
}

void WED_Buffer::SetReadPos(int a, int b)
{
	mReadIterator = (Storage *) a;
	if (mReadIterator == NULL && mStorage != NULL)
		mReadIterator = mStorage;
	mReadSubpos = b;
}

