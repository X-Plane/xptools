/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef GUI_TIMER_H
#define GUI_TIMER_H

#if APL
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#endif
#if LIN

#include <pthread.h>

class GUI_Timer;

typedef struct teh_args_t
{
	float sec;
	GUI_Timer* callme;
} teh_args_t;
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

	#elif LIN
        static void TimerCB(void *args);
		static void* teh_threadroutine(void* args);
		pthread_t* teh_thread;
		bool is_running;
		teh_args_t targ;
	#endif

};

#endif
