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

#include "GUI_Pane.h"
#include "GUI_Clipboard.h"
#if APL
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#elif IBM
	#include <Windows.h>
#endif

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#include <time.h>
#include <algorithm>
using std::find;

#if LIN
#include <QtGui/QApplication>
#endif


GUI_KeyFlags GUI_Pane::GetModifiersNow(void)
{
#if APL
	// http://developer.apple.com/documentation/Carbon/Reference/Carbon_Event_Manager_Ref/Reference/reference.html#//apple_ref/doc/uid/TP30000135-CH1g-DontLinkElementID_16
	UInt32	mods = GetCurrentEventKeyModifiers();

	GUI_KeyFlags	flags = 0;

	if (mods & shiftKey)
		flags |= gui_ShiftFlag;
	if (mods & cmdKey)
		flags |= gui_ControlFlag;
	if (mods & optionKey)
		flags |= gui_OptionAltFlag;
	return flags;
#elif IBM
	// http://blogs.msdn.com/oldnewthing/archive/2004/11/30/272262.aspx
	GUI_KeyFlags	flags = 0;

	if (::GetKeyState(VK_SHIFT) & ~1)
		flags |= gui_ShiftFlag;
	if (::GetKeyState(VK_CONTROL) & ~1)
		flags |= gui_ControlFlag;
	if (::GetKeyState(VK_MENU) & ~1)
		flags |= gui_OptionAltFlag;
	return flags;
#else
	GUI_KeyFlags	flags = 0;
	Qt::KeyboardModifiers modstate = QApplication::keyboardModifiers();

	if (modstate & Qt::MetaModifier||modstate & Qt::AltModifier)
		flags |= gui_OptionAltFlag;
	if (modstate & Qt::ShiftModifier)
		flags |= gui_ShiftFlag;
	if (modstate & Qt::ControlModifier)
		flags |= gui_ControlFlag;
	return flags;
#endif
}

void		GUI_Pane::GetMouseLocNow(int * out_x, int * out_y)
{
	if (mParent) mParent->GetMouseLocNow(out_x,out_y);
}

float		GUI_Pane::GetTimeNow(void)
{
	return (float) clock() / (float) CLOCKS_PER_SEC;
}

void		GUI_Pane::TrapFocus(void)
{
	GUI_Pane * root = this;
	while (root->GetParent()) root = root->GetParent();
	root->mTrap.insert(this);
}


GUI_Pane::GUI_Pane() :
	mParent(NULL),
	mID(0),
	mVisible(false),
	mEnabled(true)
{
	mBounds[0] = mBounds[1] = mBounds[2] = mBounds[3] = 0;
	mSticky[0] = mSticky[1] = mSticky[2] = mSticky[3] = 0;
}

GUI_Pane::~GUI_Pane()
{
	if (mParent)
	{
		vector<GUI_Pane *>::iterator self = find(mParent->mChildren.begin(),mParent->mChildren.end(), this);
		if (self != mParent->mChildren.end())
			mParent->mChildren.erase(self);
	}
	for (vector<GUI_Pane *>::iterator p = mChildren.begin(); p != mChildren.end(); ++p)
	{
		(*p)->mParent = NULL;	// prevent them from dialing us back!
		delete *p;
	}
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

void		GUI_Pane::GetVisibleBounds(int outBounds[4])
{
	outBounds[0] = mBounds[0];
	outBounds[1] = mBounds[1];
	outBounds[2] = mBounds[2];
	outBounds[3] = mBounds[3];
	if (mParent != NULL)
	{
		int b[4];
		mParent->GetVisibleBounds(b);
		outBounds[0] = max(outBounds[0], b[0]);
		outBounds[1] = max(outBounds[1], b[1]);
		outBounds[2] = min(outBounds[2], b[2]);
		outBounds[3] = min(outBounds[3], b[3]);
	}
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

void		GUI_Pane::GetSticky(float outSticky[4])
{
	outSticky[0] = mSticky[0];
	outSticky[1] = mSticky[1];
	outSticky[2] = mSticky[2];
	outSticky[3] = mSticky[3];
}

void		GUI_Pane::SetSticky(float x1, float y1, float x2, float y2)
{
	mSticky[0] = x1;
	mSticky[1] = y1;
	mSticky[2] = x2;
	mSticky[3] = y2;
}

void		GUI_Pane::SetSticky(float inSticky[4])
{
	mSticky[0] = inSticky[0];
	mSticky[1] = inSticky[1];
	mSticky[2] = inSticky[2];
	mSticky[3] = inSticky[3];
}

void		GUI_Pane::ParentResized(int inOldBounds[4], int inNewBounds[4])
{
	// Basically the rule is: if our sticky bit is set, follow our side, otherwise
	// follow the opposite side.  So both bits means we stretch, but one or the other
	// means we are tied to that side.  We no-op on NO bits, to just leave us alone...
	// having a field that follows both opposite walls is not really useful.

	int new_bounds[4] = { mBounds[0], mBounds[1], mBounds[2], mBounds[3] };

	int delta[4] = {	inNewBounds[0] - inOldBounds[0],
						inNewBounds[1] - inOldBounds[1],
						inNewBounds[2] - inOldBounds[2],
						inNewBounds[3] - inOldBounds[3] };

	new_bounds[0] += (delta[0] * mSticky[0] + delta[2] * (1.0 - mSticky[0]));
	new_bounds[2] += (delta[2] * mSticky[2] + delta[0] * (1.0 - mSticky[2]));
	new_bounds[1] += (delta[1] * mSticky[1] + delta[3] * (1.0 - mSticky[1]));
	new_bounds[3] += (delta[3] * mSticky[3] + delta[1] * (1.0 - mSticky[3]));

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

GUI_Pane *	GUI_Pane::FindByPoint(int x, int y)
{
	int bounds[4];
	if (!IsVisible()) return NULL;
	GetBounds(bounds);
	if (x < bounds[0] || x > bounds[2] || y < bounds[1] || y > bounds[3]) return NULL;

	for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
	{
		GUI_Pane * who = (*c)->FindByPoint(x,y);
		if (who) return who;
	}
	return this;
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

bool		GUI_Pane::IsEnabled(void) const
{
	return mEnabled;
}

bool		GUI_Pane::IsEnabledNow(void) const
{
	return mEnabled && (mParent == NULL || mParent->IsEnabledNow());
}

void		GUI_Pane::Enable(void)
{
	if (!mEnabled)
	{
		mEnabled = true;
		if (IsEnabledNow()) Refresh();
	}
}

void		GUI_Pane::Disable(void)
{
	if (mEnabled)
	{
		if (IsEnabledNow()) Refresh();
		mEnabled = false;
	}
}

bool		GUI_Pane::IsActiveNow(void) const
{
	return mParent && mParent->IsActiveNow();
}


void		GUI_Pane::Refresh(void)
{
	if (mParent != NULL) mParent->Refresh();
}

void		GUI_Pane::PopupMenu(GUI_Menu menu, int x, int y)
{
	if (mParent) mParent->PopupMenu(menu, x, y);
}

int		GUI_Pane::PopupMenuDynamic(const GUI_MenuItem_t items[], int x, int y, int current)
{
	return (mParent) ? mParent->PopupMenuDynamic(items, x, y, current) : -1;
}



/*
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

*/

void		GUI_Pane::InternalDraw(GUI_GraphState * state)
{
	if (mVisible)
	{
		int vb[4];
		this->GetVisibleBounds(vb);
		if (vb[0] >= vb[2] ||
			vb[1] >= vb[3])			return;
		glScissor(vb[0], vb[1], vb[2]-vb[0], vb[3]-vb[1]);

		this->Draw(state);
		for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
		{
			(*c)->InternalDraw(state);
		}
	}
}

GUI_Pane *	GUI_Pane::InternalMouseMove(int x, int y)
{
	if (mVisible)
	{
		if (x >= mBounds[0] && x <= mBounds[2] &&
			y >= mBounds[1] && y <= mBounds[3])
		{
			GUI_Pane * target;
			for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
			{
				target = (*c)->InternalMouseMove(x, y);
				if (target) return target;
			}
			if (this->MouseMove(x, y))
				return this;
		}
	}
	return NULL;
}

GUI_Pane *	GUI_Pane::InternalMouseDown(int x, int y, int button)
{
	if (mVisible)
	{
		for (set<GUI_Pane *>::iterator t = mTrap.begin(); t != mTrap.end();)
		{
			set<GUI_Pane *>::iterator w(t);
			++t;
			if (!(*w)->TrapNotify(x,y,button))
			mTrap.erase(w);
		}

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
	return 0;
}

int			GUI_Pane::InternalGetCursor(int x, int y)
{
	int ret = gui_Cursor_None;
	if (mVisible)
	{
		if (x >= mBounds[0] && x <= mBounds[2] &&
			y >= mBounds[1] && y <= mBounds[3])
		{
			for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
			{
				ret = (*c)->InternalGetCursor(x, y);
				if (ret != gui_Cursor_None) return ret;
			}

			ret = this->GetCursor(x,y);
		}
	}
	return ret;
}

int		GUI_Pane::InternalGetHelpTip(int x, int y, int tip_bounds[4], string& tip)
{
	if (mVisible)
	{
		if (x >= mBounds[0] && x <= mBounds[2] &&
			y >= mBounds[1] && y <= mBounds[3])
		{
			for (vector<GUI_Pane *>::iterator c = mChildren.begin(); c != mChildren.end(); ++c)
			{
				if ((*c)->InternalGetHelpTip(x, y, tip_bounds, tip)) return 1;
			}

			if (GetHelpTip(x,y,tip_bounds,tip)) return 1;
		}
	}
	return 0;
}


GUI_DragOperation		GUI_Pane::InternalDragEnter	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	mDragTarget = FindByPoint(x,y);
	if (mDragTarget == NULL) return gui_Drag_None;
							 return mDragTarget->DragEnter(x,y,drag,allowed, recommended);
}

GUI_DragOperation		GUI_Pane::InternalDragOver	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	GUI_Pane * target = FindByPoint(x,y);

	if (target == mDragTarget)
	{
		if (mDragTarget == NULL) return gui_Drag_None;
								 return mDragTarget->DragOver(x,y,drag,allowed, recommended);
	}
	else
	{
		if (mDragTarget) mDragTarget->DragLeave();
		mDragTarget = target;
		if (mDragTarget) return mDragTarget->DragEnter(x,y,drag,allowed, recommended);
						 return gui_Drag_None;
	}
}

void GUI_Pane::InternalDragScroll(int x, int y)
{
	if (mDragTarget)	mDragTarget->DragScroll(x,y);
}

void					GUI_Pane::InternalDragLeave	(void)
{
	if (mDragTarget) mDragTarget->DragLeave();
	mDragTarget = NULL;
}

GUI_DragOperation		GUI_Pane::InternalDrop		(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	GUI_Pane * target = FindByPoint(x,y);
	if (target) return target->Drop(x,y,drag,allowed, recommended);
	else		return gui_Drag_None;
}


bool				GUI_Pane::IsDragClick(int x, int y, int button)
{
	if (mParent)	return mParent->IsDragClick(x,y,button);
	else			return false;
}

GUI_DragOperation	GUI_Pane::DoDragAndDrop(
							int						x,
							int						y,
							int						where[4],
							GUI_DragOperation		operations,
							int						type_count,
							GUI_ClipType			inTypes[],
							int						sizes[],
							const void *			ptrs[],
							GUI_GetData_f			fetch_func,
							void *					ref)
{
	if (mParent)	return	mParent->DoDragAndDrop(x,y,where,operations,type_count,inTypes,sizes,ptrs,fetch_func,ref);
	else			return	gui_Drag_None;
}
