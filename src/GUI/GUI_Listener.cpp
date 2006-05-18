#include "GUI_Listener.h"
#include "GUI_Broadcaster.h"

GUI_Listener::GUI_Listener()
{
}

GUI_Listener::~GUI_Listener()
{
	for (set<GUI_Broadcaster*>::iterator who = mBroadcasters.begin();
		who != mBroadcasters.end(); ++who)
	{
		(*who)->mListeners.erase(this);
	}
}
