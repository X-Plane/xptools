/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "RF_Application.h"
#include "PlatformUtils.h"
#include "RF_FileCommands.h"

#if LIN
RF_Application::RF_Application(int& argc, char* argv[])
: GUI_Application(argc, argv)
#else
RF_Application::RF_Application()
#endif
{
}

RF_Application::~RF_Application()
{
}

void	RF_Application::OpenFiles(const vector<string>& inFiles)
{
}

int		RF_Application::HandleCommand(int command)
{
	return GUI_Application::HandleCommand(command);
}

int		RF_Application::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	return GUI_Application::CanHandleCommand(command, ioName, ioCheck);
}

bool	RF_Application::CanQuit(void)
{
	return true;
}

void	RF_Application::AboutBox(void)
{
	DoUserAlert("RenderFarm UI.");
}

void	RF_Application::Preferences(void)
{
}
