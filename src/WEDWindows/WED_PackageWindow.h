#ifndef WED_PACKAGEWINDOW_H
#define WED_PACKAGEWINDOW_H

#include "GUI_Window.h"
#include "GUI_Listener.h"

class	WED_Package;

class	WED_PackageWindow : public GUI_Window, public GUI_Listener {
public:

				 WED_PackageWindow(
				 		const char * 	inTitle, 
				 		int 			inBounds[4],
				 		GUI_Commander * inCommander,
				 		WED_Package *	inPackage);
	virtual		~WED_PackageWindow();
	
	virtual	int	KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	int	HandleCommand(int command);
	virtual	int	CanHandleCommand(int command, string& ioName, int& ioCheck);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

	virtual	bool	Closed(void);

private:

	WED_Package *		mPackage;
	
};

#endif
