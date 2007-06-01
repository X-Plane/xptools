#include "GUI_Timer.h"

#if APL
EventLoopTimerUPP GUI_Timer::sTimerCB = NewEventLoopTimerUPP(GUI_Timer::TimerCB);
#endif

#if IBM
typedef map<UINT_PTR,GUI_Timer *>	TimerMap;
static TimerMap						sTimerMap;
#endif

GUI_Timer::GUI_Timer(void)
{
	#if APL
		mTimer = NULL;	
	#endif
	#if IBM	
		mID = 0;
	#endif
}

GUI_Timer::~GUI_Timer(void)
{
	#if APL
		if (mTimer != NULL)
			RemoveEventLoopTimer(mTimer);
	#endif
#if IBM
		if (mID != 0)
		{
			KillTimer(NULL, mID);
			sTimerMap.erase(mID);
		}
#endif
}

void GUI_Timer::Start(float seconds)
{
	#if APL
		if (mTimer != NULL)
			RemoveEventLoopTimer(mTimer);
		InstallEventLoopTimer(
				GetMainEventLoop(),	
				seconds,
				seconds,
				sTimerCB,
				reinterpret_cast<void*>(this),
				&mTimer);
	#endif
	#if IBM
		if (mID)
		{
			SetTimer(NULL, mID, seconds * 1000.0f, TimerCB);
		}
		else
		{
			mID = SetTimer(NULL,0,seconds * 1000.0f, TimerCB);
			sTimerMap.insert(TimerMap::value_type(mID, this));
		}	
#endif	
}

void GUI_Timer::Stop(void)
{
	#if APL
		if (mTimer != NULL)
		{
			RemoveEventLoopTimer(mTimer);
			mTimer = NULL;
		}
	#endif
	#if IBM
		if (mID)
		{
			sTimerMap.erase(mID);
			KillTimer(NULL, mID);
			mID = 0;
		}
	#endif
}

#if APL
pascal void GUI_Timer::TimerCB(EventLoopTimerRef inTimer, void *inUserData)
{
	GUI_Timer * me = reinterpret_cast<GUI_Timer *>(inUserData);
	me->TimerFired();
}
#endif
#if IBM
void CALLBACK	GUI_Timer::TimerCB(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	GUI_Timer * me = sTimerMap[idEvent];
	me->TimerFired();
}
#endif
