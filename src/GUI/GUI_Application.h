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

#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#if APL
#define __DEBUGGING__
#include <Carbon/Carbon.h>
#endif

#if LIN
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#endif

#include "GUI_Commander.h"

class GUI_QtMenu;

/*
	WINDOWS WARNING: MENUS

	Windows has the following limitations on the menu system:

	1. App menus cannot be created dynamically on the fly because:

	- They are replicated as each window is made, but existing windows will not receive the new additions.
	- Accelerators are only set up at startup (so all menus must be set up before app-run.

*/

class	GUI_Application : public GUI_Commander {
public:
#if LIN
	GUI_Application(int& argc, char* argv[]);
#else
					 GUI_Application();
#endif
	virtual			~GUI_Application();

	// APPLICATION API
	void			Run(void);
	void			Quit(void);

	// MENU API
	GUI_Menu		GetMenuBar(void);
	GUI_Menu		GetPopupContainer(void);

	GUI_Menu		CreateMenu(const char * inTitle, const GUI_MenuItem_t	items[], GUI_Menu parent, int parent_item);
	void			RebuildMenu(GUI_Menu menu, const GUI_MenuItem_t	items[]);

	virtual	void	AboutBox(void)=0;
	virtual	void	Preferences(void)=0;
	virtual	bool	CanQuit(void)=0;
	virtual	void	OpenFiles(const vector<string>& inFiles)=0;

	// From GUI_Commander - app never refuses focus!
	virtual	int				AcceptTakeFocus(void) 	{ return 1; }
	virtual	int				HandleCommand(int command);
	virtual	int				CanHandleCommand(int command, string& ioName, int& ioCheck);
#if LIN
    QMenuBar* getqmenu();
#endif
private:

#if APL

	EventHandlerUPP		mMacEventHandlerUPP;
	EventHandlerUPP		mSiouxSnifferUPP;
	EventHandlerRef		mMacEventHandlerRef;
	EventHandlerRef		mSiouxSnifferRef;
	AEEventHandlerUPP	mHandleOpenDocUPP;

	static pascal OSStatus	MacEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSStatus 	SiouxSniffer(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSErr 	HandleOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);


#endif
#if !LIN
	set<GUI_Menu>		mMenus;
#endif
	bool                mDone;
#if LIN
	QList<GUI_QtMenu*>	mMenus;
public:
	QApplication*		qapp;
#endif
};
#if LIN
class GUI_QtMenu : public QMenu
{
	Q_OBJECT
public:
	GUI_QtMenu(const QString& text, GUI_Application *app);
	~GUI_QtMenu();

private:
	GUI_Application* app;
protected:
    void showEvent ( QShowEvent * event );
};

class GUI_QtAction : public QObject
{
	Q_OBJECT
public:
	GUI_QtAction(const QString& text,const QString& sc , int cmd, GUI_Application *app, bool checkable);
	~GUI_QtAction();

	QAction* qaction;
public slots:
	void ontriggered();
private:
	GUI_Application* app;
};
#endif

extern	GUI_Application *	gApplication;


#endif


