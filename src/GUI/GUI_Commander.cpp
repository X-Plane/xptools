#include "GUI_Commander.h"
#include "AssertUtils.h"

GUI_Commander * GUI_Commander::mCmdRoot = NULL;

GUI_Commander::GUI_Commander(GUI_Commander * inParent) : mCmdParent(inParent), mCmdFocus(NULL)
{
	if (inParent == NULL)
	{
		Assert(mCmdRoot == NULL);
		mCmdRoot = inParent;
	} else
		inParent->mCmdChildren.push_back(this);
}

GUI_Commander::~GUI_Commander()
{
	if (mCmdRoot == this) mCmdRoot = NULL;
	
	if (mCmdParent != NULL)
	{
		if (mCmdParent->mCmdFocus == this)
			mCmdParent->mCmdFocus = NULL;
		
		mCmdParent->mCmdChildren.erase(find(mCmdParent->mCmdChildren.begin(),mCmdParent->mCmdChildren.end(), this));
	}
	for (vector<GUI_Commander *>::iterator child = mCmdChildren.begin(); child != mCmdChildren.end(); ++child)
	{
		(*child)->mCmdParent = NULL;
	}
}

int				GUI_Commander::TakeFocus(void)
{
	GUI_Commander *	loser = mCmdRoot ? mCmdRoot->GetFocusForCommander() : NULL;
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
	if (!this->AcceptLoseFocus(inForce) && !inForce) return 0;

	GUI_Commander *	new_focus = this->mCmdParent;
	while (new_focus != NULL)
	{
		if (new_focus->AcceptTakeFocus())
		{
			new_focus->mCmdFocus = NULL;
			return 1;
		} else
			new_focus = new_focus->mCmdParent;
	}
	return 0;
}

GUI_Commander *	GUI_Commander::GetCommanderRoot(void)
{
	return mCmdRoot;
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
		if (!this->AcceptFocusChain()) return 0;
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


int				GUI_Commander::DispatchKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	GUI_Commander * who = this->GetFocusForCommander();
	while (who != NULL)
	{
		if (who->KeyPress(inKey, inVK, inFlags)) return 1;
		who = who->mCmdParent;
	}
	return 0;
}

int				GUI_Commander::DispatchHandleCommand(int command)
{
	GUI_Commander * who = this->GetFocusForCommander();
	while (who != NULL)
	{
		if (who->HandleCommand(command)) return 1;
		who = who->mCmdParent;
	}
	return 0;
}

int				GUI_Commander::DispatchCanHandleCommand(int command, string& ioName, int& ioCheck)
{
	GUI_Commander * who = this->GetFocusForCommander();
	while (who != NULL)
	{
		if (who->CanHandleCommand(command, ioName, ioCheck)) return 1;
		who = who->mCmdParent;
	}
	return 0;

}
