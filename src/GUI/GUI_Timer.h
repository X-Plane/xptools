#ifndef GUI_TIMER_H
#define GUI_TIMER_H

#if APL
#include <Carbon/Carbon.h>
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
	
	#else
	
		#error NOT IMPLEMENTED
		
	#endif
	
};			

#endif