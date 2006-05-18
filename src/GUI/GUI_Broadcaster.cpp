#include "GUI_Broadcaster.h"
#include "GUI_Listener.h"
#include "AssertUtils.h"

GUI_Broadcaster::GUI_Broadcaster()
{
}

GUI_Broadcaster::~GUI_Broadcaster()
{
	for (set<GUI_Listener*>::iterator who = mListeners.begin();
		who != mListeners.end(); ++who)
	{
		(*who)->mBroadcasters.erase(this);
	}
}
	
void	GUI_Broadcaster::BroadcastMessage(int inMsg, int inParam)
{
	// BEN SEZ: for now - just make sure that a suicidal listener doesn't
	// @#$#@ us up by editing the list while we're on it!
	for (set<GUI_Listener*>::iterator next, who = mListeners.begin();
		who != mListeners.end(); )
	{
		next = who;
		++next;
		(*who)->ReceiveMessage(this, inMsg, inParam);
		who = next;
	}
}
	
void	GUI_Broadcaster::AddListener(GUI_Listener * inListener)
{
	DebugAssert(this->mListeners.count(inListener) == 0);
	DebugAssert(inListener->mBroadcasters.count(this) == 0);
	
	inListener->mBroadcasters.insert(this);
	this->mListeners.insert(inListener);
}

void	GUI_Broadcaster::RemoveListener(GUI_Listener * inListener)
{
	DebugAssert(this->mListeners.count(inListener) != 0);
	DebugAssert(inListener->mBroadcasters.count(this) != 0);
	
	inListener->mBroadcasters.erase(this);
	this->mListeners.erase(inListener);
}
	
