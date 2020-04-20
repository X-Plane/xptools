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
#if IBM
#include "GUI_Unicode.h"
#endif

#include "AssertUtils.h"
#include "GUI_Menus.h"
#include "XWin.h"
#include "GUI_Window.h"
#include "ObjCUtils.h"
#if IBM
#include <commctrl.h>
#endif

GUI_Application *	gApplication = NULL;

static bool IsDisabledString(string& ioString)
{
	if(!ioString.empty() && ioString[0] == ';')
	{
		ioString.erase(ioString.begin());
		return true;
	}
	return false;
}

#if APL

static void	NukeAmpersand(string& ioString)
{
	string::size_type loc;
	while ((loc = ioString.find('&')) != ioString.npos)
	{
		ioString.erase(loc,1);
	}
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
static void clearSubmenusRecursive(const Fl_Menu_Item *  menu)
{
	if(!menu) return;
	int sz = menu->size();
	printf("-> menu      %s %p\n",menu->label(),menu);
	for ( int t=0; t < sz ; t++)
	{
		Fl_Menu_Item * item = (Fl_Menu_Item *) &menu[t];
		printf("%d menu     %s %p\n",t,item->label(),item);
		if(!item->label()) break;

		if(item->flags&FL_SUBMENU_POINTER)
		{
			const Fl_Menu_Item * submenu = (const Fl_Menu_Item *) item->user_data();
			clearSubmenusRecursive(submenu);
			item->user_data(nullptr);
		}
		else
		{
			if(item->user_data_ != nullptr)
			{
				delete (xmenu_cmd *)item->user_data_;
				item->user_data_ = nullptr;
			}
		}

		if(item->text != nullptr)
		{
			free((void*)item->text);
			item->text = nullptr;
		}
	}

	printf("-> delete      %s %p\n",menu->label(),menu);
	delete [] menu;
}

static void clear_menu(Fl_Menu_Item* menu)
{
	for (int i=0; i < MENU_SIZE; i++)
	{
		const Fl_Menu_Item * m = menu + i;
		//printf("%d menu clear  %s %p %p\n",i,m->label(),m->text,m->user_data_);
		if(m->text != nullptr)
			free((void*)m->text);
		if(m->user_data_ != nullptr)
			delete (xmenu_cmd *) m->user_data_;
	}

	memset(menu,0,MENU_SIZE*sizeof(Fl_Menu_Item ));
}

static void update_menu_recursive(const Fl_Menu_Item *  menu)
{
	for ( int t=0; t < menu->size(); t++)
	{
		Fl_Menu_Item * item = (Fl_Menu_Item *) &menu[t];
		//printf("%d  %s \n",t,item->label());
		if(!item->label())break;

		if(item->flags&FL_SUBMENU_POINTER)
		{
			const Fl_Menu_Item * submenu = (const Fl_Menu_Item *) item->user_data();
			update_menu_recursive(submenu);
			continue;
		}

		xmenu_cmd * mc = (xmenu_cmd *) item->user_data();
		if(!mc) continue;
		int cmd = mc->cmd;
		if(cmd == 0) continue;
		GUI_Application * app = (GUI_Application *) mc->data;
		int ioCheck = 0;
		string ioName;
		int enabled = app->DispatchCanHandleCommand(cmd,ioName,ioCheck);
		//printf("enabled:%d checked:%d  name: %s \n",enabled,ioCheck,ioName.c_str());
		if(!ioName.empty())
		{
			if(item->text != nullptr)
				free((void*)item->text);

			char * new_name  = (char *) malloc(ioName.size()+1);
			strncpy(new_name,ioName.c_str(),ioName.size()+1);

			item->label(new_name);
		}

		enabled ? item->activate():item->deactivate();
		if(ioCheck)
		{
			item->flags |= FL_MENU_TOGGLE;
			item->set();
		}
		else
		{
			item->flags &= ~FL_MENU_TOGGLE;
			item->clear();
		}
	}
}


static void menu_cb(Fl_Widget *w, void * data)
{

	if(!w || ! data) return;

	Fl_Menu_Bar * bar = (Fl_Menu_Bar *) w;

	int idx = bar->value();

	printf("menu idx %d \n", idx);

	xmenu_cmd * mc = (xmenu_cmd *) data;
	int cmd = mc->cmd;
	GUI_Application * app = (GUI_Application *) mc->data;
	int  ioCheck = 0;
    string ioName;
    //mroe : We must check again if the cmd can be handled ,
    //		 because shortcut-actions allways enabled
    if(app->DispatchCanHandleCommand(cmd,ioName,ioCheck))
			app->DispatchHandleCommand(cmd);
}

void GUI_Application::update_menus_cb(Fl_Widget *w, void * data)
{
	printf("GUI_Application::update_menus_cb \n");
	if(!w ) return;
	Fl_Menu_Bar * bar = (Fl_Menu_Bar *) w;
	update_menu_recursive(bar->menu());

}

void GUI_Application::setCutnPasteShortcuts(GUI_Window * parent)
{
//	GUI_QtAction * HK;
//	HK = new GUI_QtAction("", parent, "Ctrl+X" , gui_Cut,   this, false);  // use same definitions as in WED_Menus.cpp
//	parent->addAction(HK);
//	HK = new GUI_QtAction("", parent, "Ctrl+C" , gui_Copy,  this, false);
//	parent->addAction(HK);
//	HK = new GUI_QtAction("", parent, "Ctrl+V" , gui_Paste, this, false);
//	parent->addAction(HK);
}


#endif
#if IBM


HACCEL			gAccel = NULL;
vector<ACCEL>	gAccelTable;

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
GUI_Application::GUI_Application(int& argc, char* argv[]) : args(argc, argv),
#elif APL
GUI_Application::GUI_Application(int argc, char const * const * argv, const char * menu_nib) : args(argc, argv),
#else
GUI_Application::GUI_Application(const char * arg) : args(arg),
#endif
		GUI_Commander(NULL)
{
	DebugAssert(gApplication == NULL);
	gApplication = this;
	mDone = false;
#if APL

	app_callbacks cb = { this, MenuUpdateCB, TryQuitCB };
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
	mPopup = NULL;
	mMenu  = NULL;
#endif
}

GUI_Application::~GUI_Application()
{
#if LIN
if(mPopup)
{
	clear_menu(mPopup);
	delete [] mPopup;
	mPopup=nullptr;
	clearSubmenusRecursive(mMenu);
	mMenu=nullptr;
}
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
	while (!mDone)
	{
		Fl::wait();
	}
#endif
}

void			GUI_Application::Quit(void)
{
	mDone = true;
#if APL
	// On quit, use NS to kill the runloop.
	stop_app();
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

	if(mMenu) return mMenu;
	GUI_Window* w = GUI_Window::AnyXWND();
	if (w)
	{
		mMenu = w->GetMenuBar();
		//w->mMenuBar->global();					//this makes shortcuts works
	}
	return mMenu;
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
		MENUITEMINFOA	mif = { 0 };
		mif.cbSize = sizeof(mif);
		mif.hSubMenu = (HMENU) new_menu;
		mif.fType = MFT_STRING;
		mif.dwTypeData = const_cast<char*>(inTitle);
		mif.fMask = (parent == GetPopupContainer()) ? MIIM_TYPE : (MIIM_TYPE | MIIM_SUBMENU);

		if (parent == GetMenuBar())
		{
			InsertMenuItemA((HMENU) parent, -1, true, &mif);
		} else {
			SetMenuItemInfoA((HMENU) parent, parentItem, true, &mif);
		}
	}
#endif
#if LIN

	GUI_Menu new_menu = NULL;

	if(parent == GetPopupContainer())
	{
		if (mPopup == NULL)
		{
			mPopup = new Fl_Menu_Item[MENU_SIZE*sizeof(Fl_Menu_Item )];
			memset(mPopup,0,MENU_SIZE*sizeof(Fl_Menu_Item ));
		}
		new_menu = mPopup;
	}
	else
	{
		Fl_Menu_Item * parent_menu = (Fl_Menu_Item *) parent;

	    if(parent_menu->size() > MENU_SIZE) return NULL;

		Fl_Menu_Item * menu = new Fl_Menu_Item[MENU_SIZE*sizeof(Fl_Menu_Item )];
		memset(menu,0,MENU_SIZE*sizeof(Fl_Menu_Item ));
		if (parent == this->GetMenuBar())
		{
			parent_menu->add(inTitle,0,0,menu,FL_SUBMENU_POINTER);
		}
		else
		{
			parent_menu->insert(parentItem,inTitle,0,0,menu,FL_SUBMENU_POINTER);
		}

		new_menu = menu;
	}
#endif
	RebuildMenu(new_menu, items);

	mMenus.insert(new_menu);

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

			MENUITEMINFOA mif = { 0 };
			mif.cbSize = sizeof(mif);
			mif.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
			mif.fType = (itemname=="-") ? MFT_SEPARATOR : MFT_STRING;
			mif.fState = 0;
			if(items[n].checked) mif.fState |= MFS_CHECKED;
			if(is_disable) mif.fState |= MFS_DISABLED;
			mif.wID = items[n].cmd;
			mif.dwItemData = items[n].cmd;
			mif.dwTypeData = const_cast<char*>(itemname.c_str());
			int err = InsertMenuItemA((HMENU) new_menu, -1, true, &mif);

			++n;
		}
	#elif LIN
	if(new_menu == NULL) return ;

	Fl_Menu_Item * menu = (Fl_Menu_Item *) new_menu;

	clear_menu(menu);

	int n = 0;
	while (items[n].name)
	{
		if( menu->size() > MENU_SIZE ) return ;

		string	itemname(items[n].name);
		bool is_disable = IsDisabledString(itemname);

		if (!strcmp(items[n].name, "-"))
		{   /*addSeparator()*/
			XWin::AppendSeparator(menu);
		}
		else
		{
			if (!items[n].cmd )
			{/*is single popup  or a submenu */

				if(menu == mPopup)
				{/*we have popup already */
				    int idx = menu->add(itemname.c_str(),0,0);

					Fl_Menu_Item * m = menu + idx;

					if(items[n].checked)
					{
						m->flags |= FL_MENU_TOGGLE;
						m->set();
					}
					else
						m->clear();
					is_disable ? m->deactivate() : m->activate();
				}
				else
				{
					menu->add(itemname.c_str(),0,0);
				}
			}
			else
			{/*is part of a menu structure */
				string sc = "";
				if(items[n].key != 0)
				{
					if (items[n].flags & gui_OptionAltFlag) {sc += "#";}
					if (items[n].flags & gui_ShiftFlag)     {sc += "+";}
					if (items[n].flags & gui_ControlFlag)   {sc += "^";}

					char key_cstr[2] = { items[n].key , 0 };
					sc += tolower(key_cstr[0]);
//					switch(items[n].key)
//					{
//						case GUI_KEY_UP    :    sc += "Up";     break;
//						case GUI_KEY_DOWN  :    sc += "Down";   break;
//						case GUI_KEY_RIGHT :	sc += "Right";  break;
//						case GUI_KEY_LEFT  :    sc += "Left";   break;
//						case GUI_KEY_BACK  :    sc += "Del";    break;
//						case GUI_KEY_RETURN:    sc += "Return"; break;
//						default            :    sc += key_cstr; break;
//					}
				}

   			    xmenu_cmd * menu_cmd = new xmenu_cmd();
				menu_cmd->cmd  = items[n].cmd;
				menu_cmd->data = this;

				int idx = menu->add(itemname.c_str(),sc.c_str(),menu_cb,menu_cmd,FL_MENU_INACTIVE);
			    Fl_Menu_Item * m = menu + idx;
				if(items[n].checked)  m->flags = m->flags| FL_MENU_TOGGLE;
				items[n].checked ? m->set() : m->clear();
				is_disable ? m->deactivate() : m->activate();
			}
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

