#include "GUI_TImer.h"

#if APL
EventLoopTimerUPP GUI_Timer::sTimerCB = NewEventLoopTimerUPP(GUI_Timer::TimerCB);
#endif


GUI_Timer::GUI_Timer(void)
{
	#if APL
		mTimer = NULL;	
	#endif
}

GUI_Timer::~GUI_Timer(void)
{
	#if APL
		if (mTimer != NULL)
			RemoveEventLoopTimer(mTimer);
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
		SetTimer(NULL,reinterpret_cast<UINT_PTR>(this),seconds * 1000.0f, TimerCB);
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
		KillTimer(NULL,reinterpret_cast<UINT_PTR>(this));
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
	GUI_Timer * me = (GUI_Timer *) idEvent;
	me->TimerFired();
}
#endif
