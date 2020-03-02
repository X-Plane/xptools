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

#ifndef WED_APPLICATION_H
#define WED_APPLICATION_H

#include "GUI_Application.h"
#include "GUI_Window.h"
#include "GUI_Listener.h"
#include "GUI_Broadcaster.h"
//#include "GUI_Destroyable.h"

class WED_Settings;
class GUI_Button;
class GUI_Packer;

class	WED_Application : public GUI_Application {
public:
#if LIN
	WED_Application(int & argc, char* argv[]);
#elif APL
	WED_Application(int argc, char const * const * argv);
#else // Windows groups all of its cmdline args into a single string
	WED_Application(const char * arg);
#endif
	virtual			~WED_Application();

			void	SetAbout(GUI_Window * about_box);

	virtual	void	OpenFiles(const vector<string>& inFiles);
	virtual	void	AboutBox(void);
	virtual	void	Preferences(void);
	virtual	bool	CanQuit(void);

	virtual	int		HandleCommand(int command);
	virtual	int		CanHandleCommand(int command, string& ioName, int& ioCheck);

private:

	GUI_Window   * mAboutBox;
	WED_Settings * mSettingsWin;
};

class WED_Settings : public GUI_Window , public GUI_Listener, public GUI_Broadcaster {
public:
						 WED_Settings(GUI_Commander * cmdr);
	virtual				~WED_Settings() {};
	
	virtual bool Closed(void);

	virtual	void ReceiveMessage(
					GUI_Broadcaster *	inSrc,
					intptr_t    		inMsg,
					intptr_t			inParam);
};

#endif

