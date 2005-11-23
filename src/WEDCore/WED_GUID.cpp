#include "WED_GUID.h"
#include "IODefs.h"
#include <time.h>

const WED_GUID	NULL_GUID(NULL);

WED_GUID::WED_GUID()
{
	static int	counter = 1;
	mCounter = counter++;
	mTicks = clock();	
	mClock = time(NULL);
	mNode = 0;
}

void	WED_GUID::ReadFrom(IOReader * reader)
{
	reader->ReadInt(mCounter);
	reader->ReadInt(mTicks);	
	reader->ReadInt(mClock);	
	reader->ReadInt(mNode);	
}

void	WED_GUID::WriteTo(IOWriter * writer)
{
	writer->WriteInt(mCounter);
	writer->WriteInt(mTicks);	
	writer->WriteInt(mClock);	
	writer->WriteInt(mNode);	
}
	