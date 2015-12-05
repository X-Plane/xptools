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
#include <CoreFoundation/CoreFoundation.h>
#endif

#if LIN
#if 0
#include <pthread.h>
/*
#include <signal.h>
#include <time.h>
#include <unistd.h>
*/
class GUI_Timer;

typedef struct teh_args_t
{
	float sec;
	GUI_Timer* callme;
} teh_args_t;
#endif
#include <QtCore/QTimer>

#endif

class	GUI_Timer
#if LIN
: public QTimer {
	Q_OBJECT
#else
{
#endif
public:

						 GUI_Timer(void);
	virtual				~GUI_Timer(void);

			void		Start(float seconds);
			void		Stop(void);

	virtual	void		TimerFired(void)=0;
#if LIN
public slots:
	void		TimerCB(void);
#endif

private:

	#if APL
	
		CFRunLoopTimerRef					mTimer;

		static void				TimerCB(CFRunLoopTimerRef timer, void * data);

	#elif IBM
		static void CALLBACK	TimerCB(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
		UINT_PTR	mID;

	#elif LIN
#if 0
	public:
        static void TimerCB(void *args);
        bool is_running;
/*
		timer_t mTimer;
*/
    private:
		static void* teh_threadroutine(void* args);
		pthread_t* teh_thread;
		teh_args_t targ;
/*
		struct sigaction action;
		struct timespec tsres;
		struct itimerspec its;
		struct sigevent ev;
*/
#endif
	#endif

};

#endif
