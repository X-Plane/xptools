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

/*

	GUI_Commander - THEORY OF OPERATION
	
	GUI_Commander represents the chain of keyboard and command focus.  (Commands are expected to be issued to the same
	location as keystrokes - that is, their destination  must be inferred because the device creating the commands doesn't
	have natural spatial info to find a pane the way a mouse click does.)
	
	While commanders sit in a tree, this is a SEPARATE tree from the GUI pane structure.  You can mix a pane tree and
	commander tree, but please note that there doesn't have to be a direct correspondence.
	
	One exception is that windows are ALWAYS the root of both pane and commander trees!
	
	MEMORY MANAGEMENT
	
	Because commanders are meant to be mix-ins for other classes, and very often at least some of those classes WILL be
	panes, commanders do not attempt any memory management!
	
*/
#if !DEV
	#warning refactor KeyPress symbol (conflicts with /SDK/ac3d/Tk/X11/X.h)
#endif
#undef KeyPress

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
	static	GUI_Commander *	GetCommanderRoot(void);	
			GUI_Commander *	GetFocusForCommander(void);
			int				FocusChain(int inForce);		// Make sure that we are participating in focus - force if needed.  
			
			int				IsFocused(void);				// Are we THE focused commander?
			int				IsFocusedChain(void);			// Is the focus belwo us (we might have a shot)?
	
			GUI_Commander *	GetCmdParent(void);
	
	static	void			RegisterNotifiable(GUI_Commander_Notifiable * notif);
	static	void			UnregisterNotifiable(GUI_Commander_Notifiable * notif);
	
	// Handler Dispatchers
			int				DispatchKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);
			int				DispatchHandleCommand(int command);
			int				DispatchCanHandleCommand(int command, string& ioName, int& ioCheck);
	
	// Commander handler messages:
	virtual	int				KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)	 	{ return 0; }
	virtual	int				HandleCommand(int command) 									{ return 0; }
	virtual	int				CanHandleCommand(int command, string& ioName, int& ioCheck) { return 0; }

	virtual	int				AcceptTakeFocus(void) 										{ return 0; }
	virtual int				AcceptLoseFocus(int inForce) 								{ return 1; }
	virtual	int				AcceptFocusChain(void)										{ return 1; }

private:

		static	GUI_Commander *				mCmdRoot;
				GUI_Commander *				mCmdParent;
				GUI_Commander *				mCmdFocus;
				vector<GUI_Commander *>		mCmdChildren;

};

#endif

