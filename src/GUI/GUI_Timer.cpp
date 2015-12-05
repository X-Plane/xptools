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

#include "GUI_Timer.h"

#if IBM
typedef map<UINT_PTR,GUI_Timer *>	TimerMap;
static TimerMap						sTimerMap;
#endif


#if LIN
#if 0
//#include <X11/Xlib.h>
#include <XWin.h>

extern Display*  mDisplay;
extern std::map<Window, XWin*> sWindows;
std::set<GUI_Timer*> timer_objects;
/*
sig_atomic_t have_signal = 0;
sig_atomic_t processing = 0;
GUI_Timer* curr;

std::map<GUI_Timer*, sig_atomic_t> _timers;

void _handle_timersignal(int signal, siginfo_t* info, void* context)
{
	have_signal = 1;
	curr = reinterpret_cast<GUI_Timer*>(info->si_value.sival_ptr);
	return;
	//if (have_signal) return;
	if ((info->si_code == SI_TIMER) && (info->si_signo == SIGCONT))
	{
		curr = reinterpret_cast<GUI_Timer*>(info->si_value.sival_ptr);
		//_timers[reinterpret_cast<GUI_Timer*>(info->si_value.sival_ptr)] = 1;
		//_timers.push_back(reinterpret_cast<GUI_Timer*>(info->si_value.sival_ptr));
		have_signal = 1;
		printf("timer registered\n");
	}
}

void _do_timercallbacks(void)
{
	if (!have_signal) return;
	curr->TimerFired();
	have_signal = 0;
	printf("timer fired\n");
	//while
	//if (curr_timer) curr_timer->TimerFired();
	//have_signal = 0;
}
*/
void* GUI_Timer::teh_threadroutine(void* args)
{
	teh_args_t* arg = reinterpret_cast<teh_args_t*>(args);
	struct timespec ts;
	long long ns = arg->sec * 1000000000;
	ts.tv_sec = ns/1000000000;
    ts.tv_nsec = (ns % 1000000000);
	if (ts.tv_sec > 0)
		ts.tv_nsec = (ns % 1000000000);
	else
		ts.tv_nsec = ns;
	XEvent xevent;
	// this pointer stuff is really ugly, any other ideas?
	intptr_t t = reinterpret_cast<intptr_t>(arg->callme);
	int32_t tlow = t & 0xFFFFFFFF;
	int32_t thigh = (t >> 32) & 0xFFFFFFFF;
	Display* tDisplay = XOpenDisplay(0);
	while (arg->callme->is_running)
	{
		nanosleep(&ts, NULL);
		if (!sWindows.empty())
		{
			xevent.xclient.type			= ClientMessage;
			xevent.xclient.send_event	= True;
			xevent.xclient.window		= sWindows.begin()->first;
			xevent.xclient.display		= tDisplay;
			xevent.xclient.message_type	= XInternAtom(mDisplay, "_WED_TIMER", False);
			xevent.xclient.data.l[0]	= tlow;
			xevent.xclient.data.l[1]	= thigh;
			xevent.xclient.format		= 32;
			xevent.xclient.serial		= 0;
			XLockDisplay(tDisplay);
			XSendEvent(tDisplay, sWindows.begin()->first, False, 0, &xevent);
			XFlush(tDisplay);
			XUnlockDisplay(tDisplay);
		}
		// one shot
		if (!ts.tv_sec && !ts.tv_nsec)
			break;
	}
	XCloseDisplay(tDisplay);
	pthread_exit(0);
}

void GUI_Timer::TimerCB(void *args)
{
	// we need to do this since the timer message might be handled when
	// the object which derived from us doesn't exist anymore
	if (timer_objects.find(reinterpret_cast<GUI_Timer*>(args)) != timer_objects.end())
	{
		GUI_Timer* me = reinterpret_cast<GUI_Timer*>(args);
		me->TimerFired();
	}
}
#endif
#endif

GUI_Timer::GUI_Timer(void)
{
	#if APL
		mTimer = NULL;
	#endif
	#if IBM
		mID = 0;
	#endif
	#if LIN
	connect(this, SIGNAL(timeout()), this, SLOT(TimerCB()));
#if 0
		timer_objects.insert(this);
		teh_thread = 0;
		is_running = false;
#endif
/*
		mTimer = 0;
		clock_getres(CLOCK_MONOTONIC, &tsres);
		ev.sigev_notify = SIGEV_SIGNAL;
		ev.sigev_signo = SIGCONT;
		ev.sigev_value.sival_ptr = reinterpret_cast<void*>(this);
		memset(&action, 0, sizeof(action));
		sigemptyset(&action.sa_mask);
		sigaddset(&action.sa_mask, SIGCONT);
		action.sa_sigaction = _handle_timersignal;
		action.sa_flags = SA_SIGINFO | SA_RESTART;
		sigaction(SIGCONT, &action, 0);
		timer_create(CLOCK_MONOTONIC, &ev, &mTimer);
*/
	#endif
}

GUI_Timer::~GUI_Timer(void)
{
	#if APL
		if (mTimer != NULL)
			CFRunLoopTimerInvalidate(mTimer);
	#endif
#if IBM
		if (mID != 0)
		{
			KillTimer(NULL, mID);
			sTimerMap.erase(mID);
		}
#endif
#if LIN
#if 0
	timer_objects.erase(this);
	//if (mTimer) timer_delete(mTimer);
#endif
#endif
}

void GUI_Timer::Start(float seconds)
{
	#if APL
		if (mTimer != NULL)
			CFRunLoopTimerInvalidate(mTimer);

		CFRunLoopTimerContext ctx = { 0, this, NULL, NULL, NULL };
	
		mTimer = CFRunLoopTimerCreate(
						kCFAllocatorDefault, seconds, seconds, 0, 0, TimerCB, &ctx);
		CFRunLoopAddTimer(CFRunLoopGetCurrent(), mTimer, kCFRunLoopCommonModes);

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
#if LIN
	start(seconds*1000);
#if 0
	targ.sec = seconds;
	targ.callme = this;

	Stop();
	teh_thread = new pthread_t;
	is_running = true;
	pthread_create(teh_thread, NULL, teh_threadroutine, &targ);
/*
	long long ns = seconds * 1000000000;
	// the interval
	its.it_interval.tv_sec = ns/1000000000;
	if (its.it_interval.tv_sec > 0)
		its.it_interval.tv_nsec = (ns % 1000000000);
	else
		its.it_interval.tv_nsec = ns;
	// delay before timer starts after timer_settime() is finished.
	// when set to zero an already armed timer stops
	its.it_value.tv_sec = its.it_interval.tv_sec;
	its.it_value.tv_nsec = its.it_interval.tv_nsec;
	timer_settime(mTimer, 0, &its, 0);
	// one shot without invoking the signalhandler to avoid locks
	if (!its.it_interval.tv_sec && !its.it_interval.tv_nsec)
		return TimerFired();
*/
#endif
#endif
}

void GUI_Timer::Stop(void)
{
	#if APL
		if (mTimer != NULL)
		{
			CFRunLoopTimerInvalidate(mTimer);
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
	#if LIN
	stop();
#if 0
	if (teh_thread)
	{
		is_running = false;
		pthread_join(*teh_thread, 0);
		delete teh_thread;
		teh_thread = 0;
		//flushing all remaining timer messages
		Display* tDisplay = XOpenDisplay(0);
		XFlush(tDisplay);
		XCloseDisplay(tDisplay);
	}
/*
	if (mTimer)
	{
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 0;
		timer_settime(mTimer, 0, &its, 0);
		have_signal = 0;
	}
*/
#endif
	#endif
}

#if APL
pascal void GUI_Timer::TimerCB(CFRunLoopTimerRef inTimer, void *inUserData)
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
#if LIN
void GUI_Timer::TimerCB(void)
{
	TimerFired();
}
#endif

