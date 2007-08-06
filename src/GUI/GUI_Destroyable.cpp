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

#include "GUI_Destroyable.h"
#include "GUI_Timer.h"

class	GUI_DestroyableTask : public GUI_Timer {
public:

						 GUI_DestroyableTask(void);
	virtual				~GUI_DestroyableTask(void);
	
	virtual	void		TimerFired(void);

};

static set<GUI_Destroyable *>		mDeadList;
static GUI_DestroyableTask *		mDeadTask = NULL;



GUI_Destroyable::~GUI_Destroyable()
{
	mDeadList.erase(this);
}

void GUI_Destroyable::AsyncDestroy(void)
{
	if (mDeadTask == NULL) mDeadTask = new GUI_DestroyableTask;
	mDeadList.insert(this);
	mDeadTask->Start(0.0f);
}

GUI_DestroyableTask::GUI_DestroyableTask()
{
}

GUI_DestroyableTask::~GUI_DestroyableTask()
{
}

void GUI_DestroyableTask::TimerFired(void)
{
	set<GUI_Destroyable *> who;
	mDeadList.swap(who);
	for(set<GUI_Destroyable *>::iterator i = who.begin(); i != who.end(); ++i)
		delete *i;
	this->Stop();
}
