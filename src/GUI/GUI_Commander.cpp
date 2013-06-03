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

#include "GUI_Commander.h"
#include "AssertUtils.h"
#include <algorithm>
#include <typeinfo>

static set<GUI_Commander_Notifiable *>	sNotify;

GUI_Commander::GUI_Commander(GUI_Commander * inParent) : 
	mCmdParent(inParent), 
	mCmdFocus(NULL),
	mDeferLevel(0)
{
	if (inParent != NULL)
		inParent->mCmdChildren.push_back(this);
}

GUI_Commander::~GUI_Commander()
{
	DebugAssert(mDeferLevel==0);

	if (mCmdParent != NULL)
	{
		// Note: we must lose focus by asking each parent ... each parent might kick to the next one up.  So..
		// use LoseFocus, which already does this.  Passign force by-passes the "u sure u wnat to lose" check which
		// is critical because: 1. we can't abord now - we are in the dtor and 2. we don't want a virtual function call,
		// our vtable is bad at this point.
		this->LoseFocus(1);

		mCmdParent->mCmdChildren.erase(find(mCmdParent->mCmdChildren.begin(),mCmdParent->mCmdChildren.end(), this));
	}
	for (vector<GUI_Commander *>::iterator child = mCmdChildren.begin(); child != mCmdChildren.end(); ++child)
	{
		(*child)->mCmdParent = NULL;
	}
}

void			GUI_Commander::BeginDefer(void)
{
	DebugAssert(mDeferLevel < 1);
	++mDeferLevel;
}

void			GUI_Commander::EndDefer(void)
{
	DebugAssert(mDeferLevel > 0);
	--mDeferLevel;
	if(mDeferLevel == 0)
	{
		for(int i = 0; i < mDeferredActions.size(); ++i)
		{
			if(mDeferredActions[i].cmd)
				DispatchHandleCommand(mDeferredActions[i].cmd);
			else
				DispatchKeyPress(mDeferredActions[i].key,mDeferredActions[i].vk,mDeferredActions[i].flags);
		}
		mDeferredActions.clear();
	}
}

int				GUI_Commander::TakeFocus(void)
{
	GUI_Commander * root = this->GetRootForCommander();
	GUI_Commander *	loser = root->GetFocusForCommander();
	if (loser == NULL || loser->AcceptLoseFocus(0))
	{
		if (!mCmdParent || mCmdParent->FocusChain(0))
		{
			if (this->AcceptTakeFocus())
			{
				this->mCmdFocus = NULL;
				if (mCmdParent)
					mCmdParent->mCmdFocus = this;
				return 1;
			}
		}
	}
	return 0;
}

int				GUI_Commander::LoseFocus(int inForce)
{
	if (!inForce && !this->AcceptLoseFocus(inForce)) return 0;

	GUI_Commander *	new_focus = this->mCmdParent;
	while (new_focus != NULL)
	{
		if (new_focus->AcceptTakeFocus())
		{
			new_focus->mCmdFocus = NULL;
			return 1;
		} else {
			// EVEN if we don't accept focus, NULL out our child - they might be dtored.
			new_focus->mCmdFocus = NULL;
			new_focus = new_focus->mCmdParent;
		}
	}
	return 0;
}

GUI_Commander *	GUI_Commander::GetRootForCommander(void)
{
	GUI_Commander * root = this;
	while(root->mCmdParent)
		root = root->mCmdParent;
	return root;
}

GUI_Commander *	GUI_Commander::GetFocusForCommander(void)
{
	GUI_Commander *	who = this;
	while (who->mCmdFocus)
		who = who->mCmdFocus;
	return who;
}

int				GUI_Commander::FocusChain(int inForce)
{
	// Root is always in focus.
	if (mCmdParent == NULL) return 1;

	// We have real work to do - point our parents at us!
	if (mCmdParent->mCmdFocus != this)
	{
		if (!inForce) if (!this->AcceptFocusChain()) return 0;
		mCmdParent->mCmdFocus = this;
	}

	// Finally recurse
	if (mCmdParent) return mCmdParent->FocusChain(inForce);
	return 1;
}


int				GUI_Commander::IsFocused(void)
{
	if (mCmdFocus != NULL) return false;
	return this->IsFocusedChain();
}

int				GUI_Commander::IsFocusedChain(void)
{
	GUI_Commander * who = this;
	while (who->mCmdParent)
	{
		if (who->mCmdParent->mCmdFocus != who) return 0;
		who = who->mCmdParent;
	}
	return 1;
}

GUI_Commander * GUI_Commander::GetCmdParent(void) { return mCmdParent; }


int				GUI_Commander::DispatchKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	GUI_Commander * who = this->GetFocusForCommander();
	GUI_Commander * root = this->GetRootForCommander();
	
	if(root->mDeferLevel > 0)
	{
		root->mDeferredActions.push_back(deferred_cmd_or_key(inKey, inVK, inFlags));
		return 0;
	}	
	
	
	while (who != NULL)
	{
		if (who->HandleKeyPress(inKey, inVK, inFlags)) return 1;
		who = who->mCmdParent;
	}
	return 0;
}


void		GUI_Commander::RegisterNotifiable(GUI_Commander_Notifiable * notif)
{
	sNotify.insert(notif);
}

void		GUI_Commander::UnregisterNotifiable(GUI_Commander_Notifiable * notif)
{
	sNotify.erase(notif);
}

int				GUI_Commander::DispatchHandleCommand(int command)
{
	DebugAssert(command != 0);
	GUI_Commander * who = this->GetFocusForCommander();
	GUI_Commander * root = this->GetRootForCommander();

	if(root->mDeferLevel > 0)
	{
		root->mDeferredActions.push_back(deferred_cmd_or_key(command));
		return 0;
	}	

	for(set<GUI_Commander_Notifiable *>::iterator i = sNotify.begin(); i != sNotify.end();)
	{
		set<GUI_Commander_Notifiable *>::iterator j(i);
		++i;
		(*j)->PreCommandNotification(who, command);
	}
	while (who != NULL)
	{
		if (who->HandleCommand(command)) return 1;
		who = who->mCmdParent;
	}
	return 0;
}

int				GUI_Commander::DispatchCanHandleCommand(int command, string& ioName, int& ioCheck)
{
	DebugAssert(command != 0);

	GUI_Commander * who = this->GetFocusForCommander();
	while (who != NULL)
	{
		if (who->CanHandleCommand(command, ioName, ioCheck)) return 1;
		who = who->mCmdParent;
	}
	return 0;

}

#if DEV
void			GUI_Commander::PrintCommandChain(int indent)
{
	GUI_Commander * my_root = GetRootForCommander();
	GUI_Commander * global_focus = my_root->GetFocusForCommander();
	GUI_Commander * my_focus = this->GetFocusForCommander();
	const char * focus_str = "";
	
	if(mCmdParent && mCmdParent->mCmdFocus == this)
		focus_str = ".";
	
	if(my_focus == global_focus)
		focus_str = "*";
	
	if(this == global_focus)
		focus_str = "#";
		
	printf("%*s%s\n",indent, focus_str,typeid(*this).name());
	for(int n = 0; n < mCmdChildren.size(); ++n)
		mCmdChildren[n]->PrintCommandChain(indent+2);
	
}

#endif