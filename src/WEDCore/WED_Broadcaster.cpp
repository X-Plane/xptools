#include "WED_Broadcaster.h"
#include "WED_Listener.h"
#include "AssertUtils.h"

WED_Broadcaster::WED_Broadcaster()
{
}

WED_Broadcaster::~WED_Broadcaster()
{
	for (set<WED_Listener*>::iterator who = mListeners.begin();
		who != mListeners.end(); ++who)
	{
		(*who)->mBroadcasters.erase(this);
	}
}
	
void	WED_Broadcaster::BroadcastMessage(int inMsg, int inParam)
{
	for (set<WED_Listener*>::iterator who = mListeners.begin();
		who != mListeners.end(); ++who)
	{
		(*who)->ReceiveMessage(this, inMsg, inParam);
	}
}
	
void	WED_Broadcaster::AddListener(WED_Listener * inListener)
{
	DebugAssert(this->mListeners.count(inListener) == 0);
	DebugAssert(inListener->mBroadcasters.count(this) == 0);
	
	inListener->mBroadcasters.insert(this);
	this->mListeners.insert(inListener);
}

void	WED_Broadcaster::RemoveListener(WED_Listener * inListener)
{
	DebugAssert(this->mListeners.count(inListener) != 0);
	DebugAssert(inListener->mBroadcasters.count(this) != 0);
	
	inListener->mBroadcasters.erase(this);
	this->mListeners.erase(inListener);
}
	
