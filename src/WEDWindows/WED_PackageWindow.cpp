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

#include "WED_PackageWindow.h"
#include "WED_Package.h"
#include "WED_Menus.h"
#include "WED_Messages.h"
#include "WED_PackageStatusPane.h"

#include "GUI_ScrollerPane.h"
#include "GUI_SimpleScroller.h"

WED_PackageWindow::WED_PackageWindow(
				 		const char * 	inTitle,
				 		int 			inBounds[4],
				 		GUI_Commander * inCommander,
				 		WED_Package *	inPackage) :
	GUI_Window(inTitle, xwin_style_resizable|xwin_style_visible|xwin_style_centered, inBounds, inCommander),
	mPackage(inPackage)
{
	inPackage->AddListener(this);

	GUI_ScrollerPane * scroller = new GUI_ScrollerPane(1,1);
	scroller->SetParent(this);
	scroller->SetBounds(0,16,inBounds[2] - inBounds[0], inBounds[3] - inBounds[1]);
	scroller->SetSticky(1,1,1,1);
	scroller->Show();

	GUI_SimpleScroller * contents = new GUI_SimpleScroller;
	contents->SetParent(scroller);
	contents->Show();
	scroller->SetContent(contents);
	scroller->PositionInContentArea(contents);

	WED_PackageStatusPane * p = new WED_PackageStatusPane(inPackage, inCommander);
	p->SetParent(contents);
	p->SetBounds(0,0,360 * 8, 180 * 8);
	p->Show();

	scroller->ContentGeometryChanged();
}

WED_PackageWindow::~WED_PackageWindow()
{
	printf("Package window (empty dtor.)\n");
}

int	WED_PackageWindow::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	return 0;
}

int	WED_PackageWindow::HandleCommand(int command)
{
	switch(command) {
	case gui_Close:
		mPackage->TryClose();
		return 1;
	default:
		return 0;
	}
}

int	WED_PackageWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case gui_Close:	return 1;
	default:		return 0;
	}
}

void	WED_PackageWindow::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	if (inSrc == mPackage && inMsg == msg_PackageDestroyed)
		delete this;
}

bool	WED_PackageWindow::Closed(void)
{
	mPackage->TryClose();
	return false;
}
