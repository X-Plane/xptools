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
		if (mTimer != 0)
		{
			Fl::remove_timeout(timeout_cb,this);
			mTimer=0;
		}
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

		if(mTimer)
			Fl::remove_timeout(timeout_cb,this);

		Fl::add_timeout(seconds,timeout_cb,this);
		mTimer=seconds;
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
	if (mTimer != 0)
	{
		Fl::remove_timeout(timeout_cb,this);
		mTimer=0;
	}
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
void GUI_Timer::timeout_cb(void *args)
{
	if(!args) return;

	GUI_Timer* me = reinterpret_cast<GUI_Timer*>(args);
	me->TimerFired();
}
#endif

