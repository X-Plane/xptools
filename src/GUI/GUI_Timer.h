#ifndef GUI_TIMER_H
#define GUI_TIMER_H

#if APL
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#endif

class	GUI_Timer {
public:

						 GUI_Timer(void);
	virtual				~GUI_Timer(void);
	
			void		Start(float seconds);
			void		Stop(void);
	virtual	void		TimerFired(void)=0;

private:

	#if APL
	
				EventLoopTimerRef	mTimer;
		static	EventLoopTimerUPP	sTimerCB;
		
	static pascal void TimerCB(EventLoopTimerRef inTimer, void *inUserData);
	
	#elif IBM	
		static void CALLBACK	TimerCB(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
		UINT_PTR	mID;
	
	#else
		#error NOT IMPLEMENTED

	#endif
	
};			

#endif