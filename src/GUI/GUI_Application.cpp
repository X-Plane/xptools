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

#include "GUI_Application.h"
#include "AssertUtils.h"
#include "GUI_Menus.h"
#include "XWin.h"
#include "GUI_Window.h"
#include "ObjCUtils.h"
#if IBM
#include <commctrl.h>
#endif

GUI_Application *	gApplication = NULL;

#if APL || LIN
static void	NukeAmpersand(string& ioString)
{
	string::size_type loc;
	while ((loc = ioString.find('&')) != ioString.npos)
	{
		ioString.erase(loc,1);
	}
}
#endif

static bool IsDisabledString(string& ioString)
{
	if(!ioString.empty() && ioString[0] == ';')
	{
		ioString.erase(ioString.begin());
		return true;
	}
	return false;
}


#if IBM
HACCEL			gAccel = NULL;
vector<ACCEL>	gAccelTable;
#endif

#include "XUtils.h"

#if APL

void GUI_Application::MenuCommandCB(void * ref, int cmd)
{
	GUI_Application * me = reinterpret_cast<GUI_Application *>(ref);

	if(cmd)
		me->DispatchHandleCommand(cmd);
}


void GUI_Application::MenuUpdateCB(void * ref, int cmd, char * io_name, int * io_check, int * io_enable)
{
	GUI_Application * me = reinterpret_cast<GUI_Application *>(ref);

	if(cmd == 0) return;
	
	string name(io_name);
	
	if (me->DispatchCanHandleCommand(cmd, name, *io_check))
	{
		NukeAmpersand(name);
		strcpy(io_name,name.c_str());
		*io_enable = 1;
	}
	else
	{
		NukeAmpersand(name);
		strcpy(io_name,name.c_str());
		*io_enable = 0;
	}
}

void GUI_Application::TryQuitCB(void * ref)
{
	// If we can quit, we induce quit on ourselvse - [terminate] is not called.
	GUI_Application * me = reinterpret_cast<GUI_Application *>(ref);
	if (me->CanQuit())
		me->Quit();
}


#endif
#if LIN

GUI_QtMenu::GUI_QtMenu
(const QString& text , GUI_Application *app)
:QMenu(text),app(app)
{}

GUI_QtMenu::~GUI_QtMenu()
{}

void GUI_QtMenu::showEvent( QShowEvent * e )
{
    QList<QAction*> actlist = this->actions();
    QList<QAction*>::iterator it = actlist.begin();
    for (it ; it != actlist.end(); ++it)
    {
        int cmd = (*it)->data().toInt();
        if (cmd)
        {
            int checked = 0;
            string new_name;
            (*it)->setEnabled(app->DispatchCanHandleCommand(cmd,new_name,checked));
            if (!new_name.empty())(*it)->setText(QString::fromStdString(new_name));
            (*it)->setCheckable(checked);
            (*it)->setChecked(checked);
        }
    }
}

void GUI_QtMenu::hideEvent( QHideEvent * e )
{
    // mroe:
    // We must set to 'enabled' again, since we have disabled
    // items and their shortcut-action while showevent .
    QList<QAction*> actlist = this->actions();
    QList<QAction*>::iterator it = actlist.begin();
    for (it ; it != actlist.end(); ++it)
    {
        int cmd = (*it)->data().toInt();
        if (cmd)  (*it)->setEnabled(true);
    }
    this->QMenu::hideEvent(e);
}

GUI_QtAction::GUI_QtAction
(const QString& text,QObject * parent,const QString& sc ,int cmd, GUI_Application *app, bool checkable)
: QAction(text,parent) ,app(app)
{	
	setData(cmd);
	setShortcut(sc);
	setCheckable(checkable);
	setChecked(checkable);
	connect(this, SIGNAL(triggered()), this, SLOT(ontriggered()));
}

GUI_QtAction::~GUI_QtAction()
{}

void GUI_QtAction::ontriggered()
{	
    int cmd = data().toInt();
    if (!cmd) return;
    int  ioCheck = 0;
    string ioName;
    //mroe : We must check again if the cmd can be handled ,
    //		because shortcut-actions allways enabled .
    if(app->DispatchCanHandleCommand(cmd,ioName,ioCheck))
			app->DispatchHandleCommand(cmd);
}

QMenuBar* GUI_Application::getqmenu()
{
    QMenuBar * mbar = new QMenuBar(0);
    QList<GUI_QtMenu*>::iterator iter = mMenus.begin();
    while (iter != mMenus.end())
	{
		mbar->addMenu(*iter);
		++iter;
	}
    return mbar;
}
#endif

#if IBM
void	RegisterAccel(const ACCEL& inAccel)
{
	gAccelTable.push_back(inAccel);
}

static	void		BuildAccels(void)
{
	gAccel = CreateAcceleratorTable(&*gAccelTable.begin(), gAccelTable.size());
}
#endif

#if LIN
GUI_Application::GUI_Application(int& argc, char* argv[])
#elif APL
GUI_Application::GUI_Application(const char * menu_nib)
#else
GUI_Application::GUI_Application()
#endif
 : GUI_Commander(NULL)
{
	DebugAssert(gApplication == NULL);
	gApplication = this;
	mDone = false;
#if APL

	app_callbacks cb = { this, MenuCommandCB, MenuUpdateCB, TryQuitCB };
	
	set_delegate(&cb, menu_nib);

	void * mbar = get_menu_bar();

#endif
#if IBM
	// Note: GetModuleHandle(NULL) returns the process instance/module handle which
	// is what we want UNLESS this code is put in a DLL, which would need some re-evaluation.

	XWin::RegisterClass(GetModuleHandle(NULL));
	if(OleInitialize(NULL) != S_OK)
	{
#if ERROR_CHECK
		uh oh
#endif
	}
	InitCommonControls();
#endif
#if LIN
	qapp = new QApplication(argc, argv);
	qapp->setAttribute(Qt::AA_DontUseNativeMenuBar);
	mPopup = NULL;
#endif
}

GUI_Application::~GUI_Application()
{
#if LIN	
	if(mPopup) delete mPopup;
#endif
	DebugAssert(gApplication == this);
	gApplication = NULL;
}

void			GUI_Application::Run(void)
{
#if APL
	run_app();
#endif
#if IBM

	MSG msg;

	BuildAccels();
	while (!mDone && GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, gAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
#endif
#if LIN
	qapp->connect(qapp, SIGNAL(lastWindowClosed()), qapp, SLOT(quit()));
	qapp->exec();
#endif
}

void			GUI_Application::Quit(void)
{
	mDone = true;
#if APL
	// On quit, use NS to kill the runloop.
	stop_app();
#endif
#if LIN
	qapp->quit();
#endif
}

GUI_Menu		GUI_Application::GetMenuBar(void)
{
	#if APL
		return get_menu_bar();
	#elif IBM
		HWND hwnd = GUI_Window::AnyHWND();
		if (hwnd == NULL) return NULL;
		HMENU mbar = ::GetMenu(hwnd);
		if (mbar != NULL) return mbar;
		mbar = ::CreateMenu();
		::SetMenu(hwnd, mbar);
		return mbar;
	#else
	QMainWindow* mwindow = ((QMainWindow*)qapp->activeWindow());
	if (mwindow)
	{
		QMenuBar* mbar =  mwindow->menuBar();
		return mbar;
	}
	return NULL;
	#endif
}

GUI_Menu		GUI_Application::GetPopupContainer(void)
{
	#if APL
	return NULL;
	#elif IBM
	return NULL;
	#else
	return NULL;
	#endif
}

GUI_Menu	GUI_Application::CreateMenu(const char * inTitle, const GUI_MenuItem_t items[], GUI_Menu	parent, int parentItem)
{

#if IBM
	static int		gIDs = 1000;
#endif
#if LIN
	static int		gIDs = 1000;
#endif

#if APL
	string title(inTitle);
	NukeAmpersand(title);
	void * new_menu = create_menu(title.c_str(), parent, parentItem);
#endif

#if IBM
	//Makes the standard windows ui bar at the top
	HMENU	new_menu;

	if (parent == GetPopupContainer())	new_menu = CreatePopupMenu();
	else								new_menu = ::CreateMenu();

	if (parent)
	{
		MENUITEMINFO	mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.hSubMenu = (HMENU) new_menu;
		mif.fType = MFT_STRING;
		mif.dwTypeData = const_cast<char *>(inTitle);
		mif.fMask = (parent == GetPopupContainer()) ? MIIM_TYPE : (MIIM_TYPE | MIIM_SUBMENU);

		if (parent == GetMenuBar())
		{
			InsertMenuItem((HMENU) parent, -1, true, &mif);
		} else {
			SetMenuItemInfo((HMENU) parent, parentItem, true, &mif);
		}
	}
#endif
#if LIN
	GUI_Menu new_menu = NULL;
	
	if(parent == GetPopupContainer()) 
	{
		if (mPopup == NULL) mPopup = new QMenu(0);			 
		new_menu = mPopup;
	}
	else
	{
		GUI_QtMenu* new_qtmenu = new GUI_QtMenu(inTitle ,this);

		if (parent==GetMenuBar())
		{
			mMenus << new_qtmenu;
			((QMenuBar*) parent)->addMenu(new_qtmenu);
		}
		else
		{
			((GUI_QtMenu*) parent)->actions().at(parentItem)->setMenu(new_qtmenu);	
		}	
		
		new_menu = new_qtmenu;
	}
#endif
	RebuildMenu(new_menu, items);
#if !LIN
	mMenus.insert(new_menu);
#endif
#if IBM
	if (parent)
		DrawMenuBar(GUI_Window::AnyHWND());
#endif
	return new_menu;
}

void	GUI_Application::RebuildMenu(GUI_Menu new_menu, const GUI_MenuItem_t	items[])
{
	#if APL
		clear_menu(new_menu);
	
		int n = 0;
		while (items[n].name)
		{
			string	itemname(items[n].name);
			NukeAmpersand(itemname);
			bool is_disable = IsDisabledString(itemname);
			
			char shortcut[4] = { 0 };
			if(items[n].key != 0)
				shortcut[0] = tolower(items[n].key);
			
			if(itemname=="-")
			add_separator(new_menu);
			else
			add_menu_item(new_menu, itemname.c_str(), items[n].cmd, items[n].checked, is_disable ? 0 : 1, shortcut, items[n].flags);

			++n;
		}
	#elif IBM
		while (GetMenuItemCount((HMENU) new_menu) > 0)
			if (RemoveMenu((HMENU) new_menu, 0, MF_BYPOSITION) == 0) break;


		int n = 0;
		while (items[n].name)
		{
			string	itemname(items[n].name);
			bool is_disable = IsDisabledString(itemname);
			if(items[n].key != 0)
			{
				ACCEL accel = { 0 };
				accel.fVirt = FVIRTKEY;
				accel.cmd = items[n].cmd;
				itemname += "\t";
				if (items[n].flags & gui_ControlFlag)	{itemname += "Ctrl+";	accel.fVirt |= FCONTROL;	}
				if (items[n].flags & gui_ShiftFlag)		{itemname += "Shift+";	accel.fVirt |= FSHIFT;		}
				if (items[n].flags & gui_OptionAltFlag) {itemname += "Alt+";	accel.fVirt |= FALT;		}
				char key_cstr[2] = { items[n].key, 0 };
				switch(items[n].key)
				{
					case GUI_KEY_UP:	itemname += "Up";	accel.key = VK_UP;	break;
					case GUI_KEY_DOWN:	itemname += "Down";	accel.key = VK_DOWN;	break;
					case GUI_KEY_RIGHT:	itemname += "Right";	accel.key = VK_RIGHT;	break;
					case GUI_KEY_LEFT:	itemname += "Left";	accel.key = VK_LEFT;	break;
					case GUI_KEY_BACK:	itemname += "Del";	accel.key = VK_BACK;	break;
					case GUI_KEY_RETURN:	itemname += "Return";	accel.key = VK_RETURN;	break;
					default:		itemname += key_cstr;	accel.key = VkKeyScan(items[n].key) & 0xFF;	break;
				}
				RegisterAccel(accel);
			}

			MENUITEMINFO mif = { 0 };
			mif.cbSize = sizeof(mif);
			mif.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
			mif.fType = (itemname=="-") ? MFT_SEPARATOR : MFT_STRING;
			mif.fState = 0;
			if(items[n].checked) mif.fState |= MFS_CHECKED;
			if(is_disable) mif.fState |= MFS_DISABLED;
			mif.wID = items[n].cmd;
			mif.dwItemData = items[n].cmd;
			mif.dwTypeData = const_cast<char*>(itemname.c_str());
			int err = InsertMenuItem((HMENU) new_menu, -1, true, &mif);

			++n;
		}
	#elif LIN
	if(new_menu == NULL) return ;
	QMenu * menu = (QMenu*) new_menu;
	menu->clear();
	
	QAction * act;
	int n = 0;
	while (items[n].name)
	{
		string	itemname(items[n].name);
		bool is_disable = IsDisabledString(itemname);
	
		if (!strcmp(items[n].name, "-"))
			menu->addSeparator();
		else
			if (!items[n].cmd )
			{
				if(menu == mPopup)
				{
					act = menu->addAction(itemname.c_str());
					act->setCheckable(items[n].checked);
					act->setChecked(items[n].checked);
					if (is_disable) act->setEnabled(false);
				}
				else
				{
					menu->addMenu(itemname.c_str());
				}				
			}
			else
			{
				QString	sc = "";
				if(items[n].key != 0)
				{
					if (items[n].flags & gui_ControlFlag)	{sc += "Ctrl+";}
					if (items[n].flags & gui_ShiftFlag)     {sc += "Shift+";}
					if (items[n].flags & gui_OptionAltFlag) {sc += "Alt+";}
					char key_cstr[2] = { items[n].key, 0 };
					switch(items[n].key)
					{
						case GUI_KEY_UP:	sc += "Up";     break;
						case GUI_KEY_DOWN:	sc += "Down";   break;
						case GUI_KEY_RIGHT:	sc += "Right";  break;
						case GUI_KEY_LEFT:	sc += "Left";   break;
						case GUI_KEY_BACK:	sc += "Del";    break;
						case GUI_KEY_RETURN:	sc += "Return"; break;
						default:            	sc += key_cstr; break;
					}
				}

				menu->addAction(act = new GUI_QtAction(itemname.c_str(),menu,sc,items[n].cmd,this,false));
				if(is_disable)
					act->setEnabled(false);
			}
		++n;
	}
	#endif
}

int			GUI_Application::HandleCommand(int command)
{
	switch(command) {
	case gui_About: this->AboutBox(); return 1;
	case gui_Prefs: this->Preferences(); return 1;
	case gui_Quit: if (this->CanQuit()) this->Quit(); return 1;
	default: return 0;
	}
}

int			GUI_Application::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case gui_About: return 1;
	case gui_Prefs: return 1;
	case gui_Quit: return 1;
	default: return 0;
	}
}

