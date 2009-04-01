/*
 * Copyright (c) 2004, Laminar Research.
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
#include "RF_Notify.h"

#include <set>
static	set<WED_Notifiable *>	sNotifiables;
static	set<WED_Notify_f>		sNotifyFuncs;

WED_Notifiable::WED_Notifiable()
{
	sNotifiables.insert(this);
}
WED_Notifiable::~WED_Notifiable()
{
	sNotifiables.erase(this);
}

void	WED_Notifiable::Notify(int catagory, int message, void * param)
{
	for (set<WED_Notifiable *>::iterator iter = sNotifiables.begin(); iter != sNotifiables.end(); ++iter)
	{
		(*iter)->HandleNotification(catagory, message, param);
	}
	for (set<WED_Notify_f>::iterator iter = sNotifyFuncs.begin(); iter != sNotifyFuncs.end(); ++iter)
	{
		(*iter)(catagory, message, param);
	}
}

void	WED_RegisterNotifyFunc(WED_Notify_f inFunc)
{
	sNotifyFuncs.insert(inFunc);
}

void	WED_UnregisterNotifyFunc(WED_Notify_f inFunc)
{
	sNotifyFuncs.erase(inFunc);
}
