#ifndef GUI_COMMANDER_H
#define GUI_COMMANDER_H

#include "GUI_Defs.h"

class	GUI_Commander {
public:

							 GUI_Commander(GUI_Commander * inParent);
	virtual					~GUI_Commander();

			int				TakeFocus(void);
			int				LoseFocus(int inForce);
	static	GUI_Commander *	GetCommanderRoot(void);	
			GUI_Commander *	GetFocusForCommander(void);
			int				FocusChain(int inForce);
			
			int				IsFocused(void);
			int				IsFocusedChain(void);
	
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
