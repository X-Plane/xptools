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

#include "GUI_Broadcaster.h"
#include "GUI_Listener.h"
#include "AssertUtils.h"

GUI_Broadcaster::GUI_Broadcaster()
{
}

GUI_Broadcaster::~GUI_Broadcaster()
{
	for (set<GUI_Listener*>::iterator who = mListeners.begin();
		who != mListeners.end(); ++who)
	{
		(*who)->mBroadcasters.erase(this);
	}
}

void	GUI_Broadcaster::BroadcastMessage(intptr_t inMsg, intptr_t inParam)
{
	// We need to be careful because a listener may respond to a message by
	// destroying itself and/or other listeners. So create a copy of the
	// listener set that is guaranteed not to change while we're iterating, and
	// check that a listener still exists before sending a message to it.
	set<GUI_Listener*> listeners = mListeners;
	for (GUI_Listener* who : listeners)
	{
		if (mListeners.count(who))
			who->ReceiveMessage(this, inMsg, inParam);
	}
}

void	GUI_Broadcaster::AddListener(GUI_Listener * inListener)
{
	DebugAssert(this->mListeners.count(inListener) == 0);
	DebugAssert(inListener->mBroadcasters.count(this) == 0);

	inListener->mBroadcasters.insert(this);
	this->mListeners.insert(inListener);
}

void	GUI_Broadcaster::RemoveListener(GUI_Listener * inListener)
{
	DebugAssert(this->mListeners.count(inListener) != 0);
	DebugAssert(inListener->mBroadcasters.count(this) != 0);

	inListener->mBroadcasters.erase(this);
	this->mListeners.erase(inListener);
}

