#include "GUI_Pane.h"

GUI_Pane::GUI_Pane() :
	mParent(NULL),
	mID(0),
	mVisible(false)
{
	mBounds[0] = mBounds[1] = mBounds[2] = mBounds[3] = 0;
	mSticky[0] = mSticky[1] = mSticky[2] = mSticky[3] = 0;	
}

GUI_Pane::~GUI_Pane()
{
	for (vector<GUI_Pane *>::iterator p = mChildren.begin(); p != mChildren.end(); ++p)
		delete *p;
}

int			GUI_Pane::CountChildren(void) const
{
	return mChildren.size();
}

GUI_Pane *	GUI_Pane::GetNthChild(int n) const
{
	return mChildren[n];
}

GUI_Pane *	GUI_Pane::GetParent(void) const
{
	return mParent; 
}

void		GUI_Pane::SetParent(GUI_Pane * inParent)
{
	if (mParent != NULL)
	{
		vector<GUI_Pane *>::iterator me = find(mParent->mChildren.begin(),mParent->mChildren.end(), this);
		if (me != mParent->mChildren.end())
			mParent->mChildren.erase(me);
	}
	mParent = inParent;
	if (mParent != NULL)
		mParent->mChildren.push_back(this);
}

void		GUI_Pane::GetBounds(int outBounds[4])
{
	outBounds[0] = mBounds[0];
	outBounds[1] = mBounds[1];
	outBounds[2] = mBounds[2];
	outBounds[3] = mBounds[3];
}

void		GUI_Pane::SetBounds(int x1, int y1, int x2, int y2) 
{
	int b[4] = { x1, y1, x2, y2 };
	SetBounds(b);
}

void		GUI_Pane::SetBounds(int inBounds[4])
{
	int oldBounds[4] = { mBounds[0], mBounds[1], mBounds[2], mBounds[3] };
	mBounds[0] = inBounds[0];
	mBounds[1] = inBounds[1];
	mBounds[2] = inBounds[2];
	mBounds[3] = inBounds[3];
	
	if (oldBounds[0] != mBounds[0] ||
		oldBounds[1] != mBounds[1] ||
		oldBounds[2] != mBounds[2] ||
		oldBounds[3] != mBounds[3])
	{
		for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
			(*c)->ParentResized(oldBounds, mBounds);
		Refresh();
	}	
}

void		GUI_Pane::GetSticky(int outSticky[4])
{
	outSticky[0] = mSticky[0];
	outSticky[1] = mSticky[1];
	outSticky[2] = mSticky[2];
	outSticky[3] = mSticky[3];
}

void		GUI_Pane::SetSticky(int x1, int y1, int x2, int y2)
{
	mSticky[0] = x1;
	mSticky[1] = y1;
	mSticky[2] = x2;
	mSticky[3] = y2;
}

void		GUI_Pane::SetSticky(int inSticky[4])
{
	mSticky[0] = inSticky[0];
	mSticky[1] = inSticky[1];
	mSticky[2] = inSticky[2];
	mSticky[3] = inSticky[3];
}

void		GUI_Pane::ParentResized(int inOldBounds[4], int inNewBounds[4])
{
	int new_bounds[4] = { mBounds[0], mBounds[1], mBounds[2], mBounds[3] };
	
	if (mSticky[0])
		new_bounds[0] = mBounds[0] + (inNewBounds[0] - inOldBounds[0]);
	else
		new_bounds[0] = mBounds[0] + (inNewBounds[2] - inOldBounds[2]);

	if (mSticky[2])
		new_bounds[2] = mBounds[2] + (inNewBounds[2] - inOldBounds[2]);
	else
		new_bounds[2] = mBounds[2] + (inNewBounds[0] - inOldBounds[0]);



	if (mSticky[1])
		new_bounds[1] = mBounds[1] + (inNewBounds[1] - inOldBounds[1]);
	else
		new_bounds[1] = mBounds[1] + (inNewBounds[3] - inOldBounds[3]);

	if (mSticky[3])
		new_bounds[3] = mBounds[3] + (inNewBounds[3] - inOldBounds[3]);
	else
		new_bounds[3] = mBounds[3] + (inNewBounds[1] - inOldBounds[1]);

	SetBounds(new_bounds);
}

int			GUI_Pane::GetID(void) const
{
	return mID;
}

void		GUI_Pane::SetID(int id)
{
	mID = id;
}

GUI_Pane *	GUI_Pane::FindByID(int id)
{
	if (id == mID) return this;
	for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
	{
		GUI_Pane * who = (*c)->FindByID(id);
		if (who) return who;
	}
	return NULL;
}


void		GUI_Pane::GetDescriptor(string& outDesc) const
{
	outDesc = mDesc;
}

void		GUI_Pane::SetDescriptor(const string& inDesc)
{
	mDesc = inDesc;
}

bool		GUI_Pane::IsVisible(void) const
{
	return mVisible;
}

bool		GUI_Pane::IsVisibleNow(void) const
{
	return mVisible && mParent && mParent->IsVisibleNow();
}

void		GUI_Pane::Show(void)
{
	if (!mVisible)
	{
		mVisible = true;
		if (IsVisibleNow())	Refresh();
	}		
}

void		GUI_Pane::Hide(void)
{
	if (mVisible)
	{
		if (IsVisibleNow())	Refresh();
		mVisible = false;
	}
}
		
void		GUI_Pane::Refresh(void)
{
	if (mParent != NULL) mParent->Refresh();
}

// Ben sez: big gotcha - it's really dangerous to let TakeFocus and LoseFocus call each
// other -- the potential for infinite loops is, well, infinite!  So our strategy is:
// each one of theese non-virtual routines is totally all-encompassing...guaranteeing
// no recursion!


// TAKE FOCUS: This routine ASSUMES the new focus doesn't mind having it - it never
// asks.  It does ask to lose focus from the current focus widget, and will fail
// if (1) the current focus won't give it up or (2) the widgets are not in a window.
int			GUI_Pane::TakeFocus(void)
{
	// First see if some other widget is focused.
	GUI_Pane * cur_focus = this->GetFocus();
	if (cur_focus == this) return 1;	// Quick exit!
	
	if (cur_focus != NULL)
	{
		// If so, try to lose focus, but bail if that 
		// pane refuses.
		if (!cur_focus->AcceptLoseFocus(0))
			return 0;
	}
	
	// Try to set the focus variable.  This is
	// handled by some smart superclass.  If this
	// barfs, well, we're not focused.
	if (this->InternalSetFocus(this))
		return 1;

	return 0;
}

// LOSE FOCUS: Attempts to get focus off a widget, possibly by force.  Dumps it
// on the next higher widget that will accept it!  Fails if (1) the widget won't
// accept it or (2) we're not rooted.
int			GUI_Pane::LoseFocus(int inForce)
{
	// Okay to lose focus?
	int okay_to_lose = this->AcceptLoseFocus(inForce);
	// If we're focusing, then damn straight it is!
	if (inForce) okay_to_lose = true;
	
	if (okay_to_lose)
	{
		// Going to really lose..okay any parent who can take focus?  If so,
		// they take focus.
		for (GUI_Pane * who = this->mParent; who != NULL; who = who->mParent)
		{
			if (who->AcceptTakeFocus())
			{
				return who->InternalSetFocus(who);
			}
		}
		
		// No parnet took it?  Well, as long as we can move the focus we're good
		return this->InternalSetFocus(NULL);
	}	
	// Failed ot lose focus - refusal!
	return 0;
}

GUI_Pane *	GUI_Pane::GetFocus(void)
{
	if (mParent) return mParent->GetFocus();
	return NULL;
}


void		GUI_Pane::InternalDraw(GUI_GraphState * state)
{
	if (mVisible)
	{
		this->Draw(state);
		for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
		{
			(*c)->InternalDraw(state);
		}
	}
}

GUI_Pane *	GUI_Pane::InternalMouseDown(int x, int y, int button)
{
	if (mVisible)
	{
		if (x >= mBounds[0] && x <= mBounds[2] &&
			y >= mBounds[1] && y <= mBounds[3])
		{
			GUI_Pane * target;
			for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
			{
				target = (*c)->InternalMouseDown(x, y, button);
				if (target) return target;
			}
			
			if (this->MouseDown(x, y, button))
				return this;			
		}
	}
	return NULL;
}

int			GUI_Pane::InternalMouseWheel(int x, int y, int dist, int axis)
{
	if (mVisible)
	{
		if (x >= mBounds[0] && x <= mBounds[2] &&
			y >= mBounds[1] && y <= mBounds[3])
		{
			for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
			{
				if ((*c)->InternalMouseWheel(x, y, dist, axis)) return 1;
			}
			
			if (this->ScrollWheel(x, y, dist, axis))
				return 1;			
		}
	}
	return NULL;
}

void		GUI_Pane::InternalKeyPress(char inKey, int inVK, int inFlags)
{
	if (!this->KeyPress(inKey, inVK, inFlags))
	if (mParent != NULL)
		mParent->InternalKeyPress(inKey, inVK, inFlags);
}

int			GUI_Pane::InternalSetFocus(GUI_Pane * who)
{
	if (mParent) return mParent->InternalSetFocus(who);
	return false;	// no parent?  no focus!
}

