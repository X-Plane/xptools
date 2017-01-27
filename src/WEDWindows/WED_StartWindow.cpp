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

#include "WED_StartWindow.h"
#include "GUI_Button.h"
#include "WED_Menus.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"
#include "GUI_Resources.h"
#include "BitmapUtils.h"
#include "GUI_Fonts.h"
#include "WED_PackageMgr.h"
#include "GUI_Table.h"
#include "WED_Colors.h"
#include "GUI_TextTable.h"
#include "WED_PackageListAdapter.h"
#include "WED_Messages.h"
#include "PlatformUtils.h"
#include "FileUtils.h"
#include "WED_UIDefs.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "WED_Version.h"

static int *	SizeOfPng(const char * png)
{
	static int bounds[4];
	bounds[0] = 0; bounds[1] = 0;
	GUI_GetImageResourceSize(png, bounds+2);
	return bounds;
}

const char * LastPart(const char * s)
{
	const char * ret = s;
	while (*s)
	{
		if (*s==':'||*s=='/'||*s=='\\') ret=s;
		++s;
	}
	return ret;
}

struct open_doc_t {
	WED_Document *			d;
	WED_DocumentWindow *	w;
	string					n;
};

static vector<open_doc_t>	sDocs;

static int kDefaultBounds[4] = { 50, 50, 850, 650 };

WED_StartWindow::WED_StartWindow(GUI_Commander * cmder) : GUI_Window("WED", xwin_style_centered|xwin_style_resizable, kDefaultBounds, cmder)
{
	int btns[2];
//	GUI_GetImageResourceSize("startup_bkgnd.png", bkgnd);
	GUI_GetImageResourceSize("startup_btns.png", btns);

//	SetBounds(100, 100, bkgnd.width + 100, bkgnd.height + 100);

	int btn_width = btns[0] / 3;
	int btn_height = btns[1] / 3;

	int btn_width1  = btn_width  / 2; int btn_width2  = btn_width  - btn_width1 ;
	int btn_height1 = btn_height / 2; int btn_height2 = btn_height - btn_height1;

	int p2 = (kDefaultBounds[2] - kDefaultBounds[0]) * 0.50;

	int	new_off[4] = { 0, 0, 3, 3 };
	int	new_on [4] = { 0, 1, 3, 3 };
	int	opn_off[4] = { 1, 0, 3, 3 };
	int	opn_on [4] = { 1, 1, 3, 3 };
	int	chs_off[4] = { 2, 0, 3, 3 };
	int	chs_on [4] = { 2, 1, 3, 3 };

	mNew  = new GUI_Button("startup_btns.png", btn_Web, new_off, new_on, new_on, new_on);
	mOpen = new GUI_Button("startup_btns.png", btn_Web, opn_off, opn_on, opn_on, opn_on);
	mChange = new GUI_Button("startup_btns.png", btn_Web, chs_off, chs_on, chs_on, chs_on);

	mNew->AddRadioFriend(mOpen);
	mNew->AddRadioFriend(mChange);
	mOpen->AddRadioFriend(mNew);
	mOpen->AddRadioFriend(mChange);
	mChange->AddRadioFriend(mOpen);
	mChange->AddRadioFriend(mNew);


	mNew->SetBounds(p2 - btn_width - btn_width1, 10,
					p2 - btn_width + btn_width2, 10 + btn_height);
	mOpen->SetBounds(p2 - btn_width1, 10,
					 p2 + btn_width2, 10 + btn_height);
	mChange->SetBounds(p2 + btn_width- btn_width1, 10,
					p2 + btn_width + btn_width2, 10 + btn_height);
	mNew->SetParent(this);
	mOpen->SetParent(this);
	mChange->SetParent(this);
//	mNew->Show();
//	mOpen->Show();
	mNew->AddListener(this);						// We listen to all 3 buttons for clicks
	mOpen->AddListener(this);
	mChange->AddListener(this);
	mNew->SetSticky(0.5f,1,0.5f,0);
	mOpen->SetSticky(0.5f,1,0.5f,0);
	mChange->SetSticky(0.5f,1,0.5f,0);

	mScroller = new GUI_ScrollerPane(false, true);
	mScroller->SetParent(this);
	mScroller->SetBounds(10, 20 + btn_height + 10, kDefaultBounds[2] - kDefaultBounds[0] - 10, kDefaultBounds[3] - kDefaultBounds[1] - 30);
	mScroller->SetSticky(1,1,1,1);

	mTable = new GUI_Table(1);		// 1=  fill right
	mPackageList = new WED_PackageListAdapter(this);	// back-ptr to us so it can send an open command msg to SOMEONE in the command chain!
	mTextTable = new GUI_TextTable(this, 10,0);	// 10 = indent

	mTextTable->SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_PropertyBar_Text),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));

	mTable->SetGeometry(mPackageList);
	mTable->SetContent(mTextTable);
	mTextTable->SetProvider(mPackageList);
	mTextTable->SetParentTable(mTable);
	mTextTable->AddListener(mTable);			// Table listens to text table to find out when content changed due to syntactic stuff
	mPackageList->AddListener(mTable);			// Table listens to package list ot find out when content changed due to semantic stuff
	mPackageList->AddListener(this);			// We listen to package list to know when content changes - recompute buttons then
//	mTextTable->SetImage("property_bar.png", 2);

	mTable->Show();
	mTable->SetParent(mScroller);
	mScroller->SetContent(mTable);
	mScroller->PositionInContentArea(mTable);
//	table->AddListener(mScroller);				// Not needed - scroller listens to contents automatically

	mScroller->SetImage("gradient.png");

	gPackageMgr->AddListener(this);				// We listen to package mgr to know when x-system folder changed - hide whole list if needed.
	mTextTable->FocusChain(false);

	//#if DEV
	//	PrintDebugInfo(0);
	//#endif
}

WED_StartWindow::~WED_StartWindow()
{
	delete mTextTable;
	delete mPackageList;
}

void	WED_StartWindow::ShowMessage(const string& msg)
{
	mCaption = msg;
	if (mCaption.empty())
	{
		mNew->Show();
		mOpen->Show();
		mChange->Show();
		if (gPackageMgr->HasSystemFolder())
			mScroller->Show();
		else
			mScroller->Hide();
	}
	else
	{
		mNew->Hide();
		mOpen->Hide();
		mChange->Hide();
		mScroller->Hide();
	}
	Refresh();
	UpdateNow();
}


void	WED_StartWindow::Draw(GUI_GraphState * state)
{
	int me[4], child[4];
	this->GUI_Pane::GetBounds(me);
	mNew->GetBounds(child);
	child[0] = me[0];
	child[2] = me[2];

	glColor3f(1,1,1);

	int kTileAll[4] = { 0, 0, 1, 1 };

	GUI_DrawStretched(state,"gradient.png", me, kTileAll);

	if (!mScroller->IsVisible())
	{
		GUI_DrawCentered(state,"startup_bkgnd.png", me, 0, 1, kTileAll, NULL, NULL);

		float f = GUI_GetLineHeight(font_UI_Basic);
		float color[4] = { 1.0, 1.0, 1.0, 0.7 };

		const char * main_text[] = {
			"WorldEditor " WED_VERSION_STRING_SHORT,
			"©Copyright 2007-2016, Laminar Research.",
			"",
			"This software is available under an open license.",
			"Visit http://developer.x-plane.com/code/ for more info.",
			0
			
		};
		
		int n = 0;
		while(main_text[n])
		{
			GUI_FontDraw(state, font_UI_Basic, color, me[0] * 0.5 + me[2] * 0.5, me[3] - 30 - f * n, main_text[n]);
			++n;
		}

	}

	if (mCaption.empty())
	{
		GUI_DrawStretched(state, "startup_bar.png", child, kTileAll);
	}

	{
		float color[4] = { 1.0, 1.0, 1.0, 1.0 };

		float f = GUI_GetLineHeight(font_UI_Basic);

		string m(mCaption);
		if (m.empty())
		{
			if (mScroller->IsVisible()) {
				gPackageMgr->GetXPlaneFolder(m);
				m = string("Scenery packages in: ") + m;
				child[1] = me[3] - 15;
				child[3] = me[3] - 15;
			} else {
				m = "Please Pick Your X-System Folder";
				int ch = (child[3]-child[1]);
				child[1] += ch;
				child[3] += ch;
			}
		}
		GUI_FontDrawScaled(state, font_UI_Basic, color, child[0], (child[1]+child[3]-f)*0.5f, child[2], (child[1]+child[3]+f)*0.5f, &*m.begin(), &*m.end(), align_Center);

	}
}

bool	WED_StartWindow::Closed(void)
{
	this->DispatchHandleCommand(gui_Quit);
    return false;
}

void	WED_StartWindow::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
	if (inSrc == mNew && inMsg == GUI_CONTROL_VALUE_CHANGED)	this->DispatchHandleCommand(wed_NewPackage);
	if (inSrc == mOpen && inMsg == GUI_CONTROL_VALUE_CHANGED)	this->DispatchHandleCommand(wed_OpenPackage);
	if (inSrc == mChange && inMsg == GUI_CONTROL_VALUE_CHANGED) this->DispatchHandleCommand(wed_ChangeSystem);

	if(inMsg == msg_SystemFolderChanged || inMsg == msg_SystemFolderUpdated)
	{
		if (gPackageMgr->HasSystemFolder())
		{
			mScroller->Show();
			mScroller->Refresh();
		} else
			mScroller->Hide();
	}

	if (inMsg == msg_DocumentDestroyed)
	{
		for (int n = 0; n < sDocs.size(); ++n)
		{
			if (sDocs[n].d == inSrc)
			{
				mPackageList->UnlockPackage(sDocs[n].n);
				sDocs.erase(sDocs.begin()+n);
				return;
			}
		}
	}
	RecomputeButtonEnables();
}

int		WED_StartWindow::MouseMove(int x, int y)
{
	mNew->SetHilite(0);
	mOpen->SetHilite(0);
	mChange->SetHilite(0);
	return 1;
}

int			WED_StartWindow::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
#if IBM && DEV
	//Press C to show to maximize the debug console
	if(inVK == GUI_VK_C)
	{
		//Minimizes the console, uncomment the next line to restore it for use
		ShowWindow( GetConsoleWindow(), SW_SHOWNOACTIVATE );

		return 1;
	}
	//Press H to minimize
	if(inVK = GUI_VK_H)
	{
		ShowWindow( GetConsoleWindow(), SW_SHOWMINNOACTIVE );
		return 1;
	}
#endif
	return 0;
}

// check for the presence of a "Custom Scenery" and a "Resources" folder (actually, even files of these names will do)
static bool is_XSystemDir(string path)
{
	string dir = path + DIR_STR + "Custom Scenery";

	if (FILE_exists(dir.c_str()))
	{
		dir = path + DIR_STR + "Resources";
		if (FILE_exists(dir.c_str()))
			return true;
	}
	return false;
}

int			WED_StartWindow::HandleCommand(int command)
{
	char buf[1024];

	switch(command) {
	case wed_ChangeSystem:
		if (GetFilePathFromUser(getFile_PickFolder, "Please select your X-Plane folder", "Select", FILE_DIALOG_PICK_XSYSTEM, buf, sizeof(buf) ))
		{
			if (is_XSystemDir(buf))
				gPackageMgr->SetXPlaneFolder(buf);
			else
			{
				string msg = string("'") + buf + "' is not the base of a X-Plane installation.\n"
				             + "It needs to have a 'Custom Scenery' and a 'Resources' folder inside it.";
				DoUserAlert(msg.c_str());
			}	
		}
		return 1;
	case wed_NewPackage:
		if (!gPackageMgr->HasSystemFolder()) return 1;					// Ben says: buttons do NOT check whether we are command-enabled, so recheck!
		mPackageList->SelectPackage(gPackageMgr->CreateNewCustomPackage());
		mTable->RevealRow(0);
		return 1;
	case wed_OpenPackage:
		//if(HasSystemFolder == false) is another way of saying this.
		if (!gPackageMgr->HasSystemFolder()) return 1;
		if (mPackageList->HasSelection())
		{
			string name;
			int n = mPackageList->GetSelection(&name);
			//gPackageMgr->GetNthCustomPackagePath(n,path);

			for(int i = 0; i < sDocs.size(); ++i)
			if (sDocs[i].n == name)
			{
				sDocs[i].w->Show();
				return 1;
			}

				try {
					open_doc_t nd;
					double b[4] = { -180, -90, 180, 90 };
					nd.n = name;
					nd.d = new WED_Document(name.c_str(), b);
					nd.w = new WED_DocumentWindow(name.c_str(), this->GetCmdParent(), nd.d);
					sDocs.push_back(nd);
					mPackageList->LockPackage(nd.n);
					nd.d->AddListener(this);
				} catch(exception& e) {
					DoUserAlert(e.what());				
				} catch (...) {
					DoUserAlert("An unknown error occurred.");
				}
		}
		return 1;
	default:
		return 0;
	}

}

int			WED_StartWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case wed_NewPackage:	return gPackageMgr->HasSystemFolder();
	case wed_ChangeSystem:	return 1;
	case wed_OpenPackage:	return gPackageMgr->HasSystemFolder() && mPackageList->HasSelection();
	default: return 0;
	}
}



void			WED_StartWindow::Activate(int inActive)
{
	GUI_Window::Activate(inActive);
	gPackageMgr->Rescan();
}

void			WED_StartWindow::RecomputeButtonEnables()
{
	bool	enable_open = gPackageMgr->HasSystemFolder() && mPackageList->HasSelection();
	bool	enable_new = gPackageMgr->HasSystemFolder();

	int	new_off[4] = { 0, 0, 3, 3 };
	int	new_on [4] = { 0, 1, 3, 3 };
	int	new_dis[4] = { 0, 2, 3, 3 };
	int	opn_off[4] = { 1, 0, 3, 3 };
	int	opn_on [4] = { 1, 1, 3, 3 };
	int	opn_dis[4] = { 1, 2, 3, 3 };

	if (enable_new)
		mNew->SetTiles(new_off, new_on, new_on, new_on);
	else
		mNew->SetTiles(new_dis, new_dis, new_dis, new_dis);

	if (enable_open)
		mOpen->SetTiles(opn_off, opn_on, opn_on, opn_on);
	else
		mOpen->SetTiles(opn_dis, opn_dis, opn_dis, opn_dis);
}
