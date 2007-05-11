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
