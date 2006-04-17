#ifndef WED_APPLICATION_H
#define WED_APPLICATION_H

#include "GUI_Application.h"

class	WED_Application : public GUI_Application {
public:
					 WED_Application();
	virtual			~WED_Application();

	virtual	void	OpenFiles(const vector<string>& inFiles);

};

#endif
