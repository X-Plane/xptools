#include "WED_Listener.h"
#include "WED_Broadcaster.h"

WED_Listener::WED_Listener()
{
}

WED_Listener::~WED_Listener()
{
	for (set<WED_Broadcaster*>::iterator who = mBroadcasters.begin();
		who != mBroadcasters.end(); ++who)
	{
		(*who)->mListeners.erase(this);
	}
}
