#ifndef WED_APPLICATION_H
#define WED_APPLICATION_H

#include "GUI_Application.h"

class	GUI_Window;

class	WED_Application : public GUI_Application {
public:
					 WED_Application();
	virtual			~WED_Application();

			void	SetAbout(GUI_Window * about_box);

	virtual	void	OpenFiles(const vector<string>& inFiles);
	virtual	void	AboutBox(void);
	virtual	void	Preferences(void);
	virtual	bool	CanQuit(void);
	
	virtual	int		HandleCommand(int command);
	virtual	int		CanHandleCommand(int command, string& ioName, int& ioCheck);

private:
	
	GUI_Window * mAboutBox;

};

#endif
