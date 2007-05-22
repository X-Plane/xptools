#include "GUI_TImer.h"

#if APL
EventLoopTimerUPP GUI_Timer::sTimerCB = NewEventLoopTimerUPP(GUI_Timer::TimerCB);
#endif

#if !APL
#error not coded
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
}

pascal void GUI_Timer::TimerCB(EventLoopTimerRef inTimer, void *inUserData)
{
	GUI_Timer * me = reinterpret_cast<GUI_Timer *>(inUserData);
	me->TimerFired();
}
