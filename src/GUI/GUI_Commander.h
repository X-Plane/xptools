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

#ifndef GUI_COMMANDER_H
#define GUI_COMMANDER_H

#include "GUI_Defs.h"
#include <stdint.h>

/*

	GUI_Commander - THEORY OF OPERATION

	GUI_Commander represents the chain of keyboard and command focus.  (Commands are expected to be issued to the same
	location as keystrokes - that is, their destination  must be inferred because the device creating the commands doesn't
	have natural spatial info to find a pane the way a mouse click does.)

	While commanders sit in a tree, this is a SEPARATE tree from the GUI pane structure.  You can mix a pane tree and
	commander tree, but please note that there doesn't have to be a direct correspondence.

	One exception is that windows are ALWAYS the root of both pane and commander trees!

	DEFERMENT
	
	There's a nasty category of bugs that can occur: while the user cannot easily do two commands at once with the mouse
	(e.g. select while moving an object) because there is only one left mouse button) the user CAN do two things at once
	by running a command off the keyboard while editign with the mouse.
	
	We provide an API to _defer_ command processing until later - other code can thus tell commands to 'go wait' until we
	are ready to accept them.  The GUI_Window mouse code uses this to defer commands while drags are in place to avoid 
	double-commanding.

	MEMORY MANAGEMENT

	Because commanders are meant to be mix-ins for other classes, and very often at least some of those classes WILL be
	panes, commanders do not attempt any memory management!

*/

class	GUI_Commander;

class	GUI_Commander_Notifiable {
public:

	virtual	void			PreCommandNotification(GUI_Commander * focus_target, int command)=0;

};

class	GUI_Commander {
public:

							 GUI_Commander(GUI_Commander * inParent);
	virtual					~GUI_Commander();

			int				TakeFocus(void);				// Try to focus this commander - returns 1 if successful.
			int				LoseFocus(int inForce);			// Unfocus this commander.  Pass 1 to disallow veto.  Returns true if successful.
			int				FocusChain(int inForce);		// Make sure that we are participating in focus - force if needed.

			GUI_Commander *	GetRootForCommander(void);		// Who is last in line in the chain that WE participate in?
			GUI_Commander *	GetFocusForCommander(void);		// Who is in focus in the chain that WE participate in?
			int				IsFocused(void);				// Are we THE focused commander?
			int				IsFocusedChain(void);			// Is the focus below us (we might have a shot)?

			GUI_Commander *	GetCmdParent(void);
			
			void			BeginDefer(void);
			void			EndDefer(void);

	static	void			RegisterNotifiable(GUI_Commander_Notifiable * notif);
	static	void			UnregisterNotifiable(GUI_Commander_Notifiable * notif);

	// Handler Dispatchers - external code can call DispatchHandleCommand to "run" a command.
	// The commands are sent to the focused commander in the CHAIN that we participate in. 
	// In other words, if we DispatchHandleCommand to a window, the text field in the window gets
	// first crack, the window goes second, and the app goes third.
			int				DispatchKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);
			int				DispatchHandleCommand(int command);
			int				DispatchCanHandleCommand(int command, string& ioName, int& ioCheck);

#if DEV
			void			PrintCommandChain(int indent);
#endif

protected:

	// Commander handler messages:
	virtual	int				HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)	 	{ return 0; }
	virtual	int				HandleCommand(int command) 									{ return 0; }
	virtual	int				CanHandleCommand(int command, string& ioName, int& ioCheck) { return 0; }

	virtual	int				AcceptTakeFocus(void) 										{ return 0; }
	virtual int				AcceptLoseFocus(int inForce) 								{ return 1; }
	virtual	int				AcceptFocusChain(void)										{ return 1; }

private:

			struct	deferred_cmd_or_key {
				deferred_cmd_or_key(int c) : cmd(c) { }
				deferred_cmd_or_key(uint32_t k, int v, GUI_KeyFlags f) : cmd(0), key(k), vk(v), flags(f) { }
				int				cmd;
				uint32_t		key;
				int				vk;
				GUI_KeyFlags	flags;
			};

				int							mDeferLevel;
				vector<deferred_cmd_or_key>	mDeferredActions;
				
				GUI_Commander *				mCmdParent;
				GUI_Commander *				mCmdFocus;
				vector<GUI_Commander *>		mCmdChildren;

};

#endif

