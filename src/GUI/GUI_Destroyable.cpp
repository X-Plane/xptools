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
