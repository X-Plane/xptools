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

#if APL
EventLoopTimerUPP GUI_Timer::sTimerCB = NewEventLoopTimerUPP(GUI_Timer::TimerCB);
#endif

#if IBM
typedef map<UINT_PTR,GUI_Timer *>	TimerMap;
static TimerMap						sTimerMap;
#endif


#if LIN

#include <X11/Xlib.h>
#include <XWin.h>
extern Display*  mDisplay;
extern std::map<Window, XWin*> sWindows;

void* GUI_Timer::teh_threadroutine(void* args)
{
	teh_args_t* arg = reinterpret_cast<teh_args_t*>(args);
	struct timespec ts;
	long us = arg->sec * 1000000;
	ts.tv_sec = us/1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
	XEvent xevent;
	// this pointer stuff is really ugly, any other ideas?
	intptr_t t = reinterpret_cast<intptr_t>(arg->callme);
	int tlow = t & 0xFFFFFFFF;
	int thigh = (t >> 32) & 0xFFFFFFFF;
	Display* tDisplay = XOpenDisplay(0);
	while (arg->callme->is_running)
	{
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
			XSendEvent(tDisplay, sWindows.begin()->first, False, 0, &xevent);
			XFlush(tDisplay);
		}
		// one shot
		if (ts.tv_sec == 0 && ts.tv_nsec == 0)
			break;
		nanosleep(&ts, NULL);
	}
	XCloseDisplay(tDisplay);
	pthread_exit(0);
}

void GUI_Timer::TimerCB(void *args)
{
	GUI_Timer * me = reinterpret_cast<GUI_Timer *>(args);
	me->TimerFired();
}

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
		teh_thread = 0;
		is_running = false;
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
#if LIN
	targ.sec = seconds;
	targ.callme = this;

	if (teh_thread)
	{
		is_running = false;
		pthread_join(*teh_thread, 0);
		delete teh_thread;
		teh_thread = 0;
		//flushing all remaining timer messages
		Display* tDisplay = XOpenDisplay(0);
		XSync(tDisplay, False);
		XCloseDisplay(tDisplay);
	}
	teh_thread = new pthread_t;
	is_running = true;
	pthread_create(teh_thread, NULL, teh_threadroutine, &targ);
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
	#if LIN
	if (teh_thread)
	{
		is_running = false;
		pthread_join(*teh_thread, 0);
		delete teh_thread;
		teh_thread = 0;
		//flushing all remaining timer messages
		Display* tDisplay = XOpenDisplay(0);
		XSync(tDisplay, False);
		XCloseDisplay(tDisplay);
		
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

