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

#include "WED_Application.h"
#include "WED_Document.h"
#include "WED_Menus.h"
#include "WED_Url.h"
#include "WED_Messages.h"

#include "GUI_Resources.h"
#include "GUI_Fonts.h"
#include "GUI_Help.h"
#include "GUI_Packer.h"
#include "GUI_Button.h"
#include "GUI_Label.h"

//#include "WED_Globals.h"
int gIsFeet = 0;
int gInfoDMS = 0;

static int settings_bounds[4] = { 0, 0, 512, 384};

class RadioButton {
public:
	RadioButton(int x0, int y0, WED_Settings * parent,  int * var, const string& desc, const char * text0, const char * text1);
	~RadioButton() {};
};

RadioButton::RadioButton(int x0, int y0, WED_Settings * parent,  int * var, const string& desc, const char * text0, const char * text1)
{
	char * texture = "check_buttons.png";
	int r_yes[4] = { 0, 1, 1, 3 };
	int r_nil[4] = { 0, 0, 1, 3 };

	int h = GUI_GetImageResourceHeight(texture)/3;

	float white[4] = { 1, 1, 1, 1 };
	
	int x1 = x0 + 130;

	GUI_Label * label = new GUI_Label();
	label->SetBounds(x0,y0-h,x1,y0+h);
	label->SetColors(white);
	label->SetDescriptor(desc);
	label->SetParent(parent);
	label->Show();
	
	GUI_Button * btn_0 = new GUI_Button(texture, btn_Radio, r_nil, r_nil, r_yes, r_yes);
	btn_0->SetBounds(x1,y0,x1+100,y0+h);
	btn_0->SetDescriptor(text0);
	btn_0->SetParent(parent);
	btn_0->Show();
	btn_0->SetMsg((intptr_t) var, 1);

	GUI_Button * btn_1 = new GUI_Button(texture, btn_Radio, r_nil, r_nil, r_yes, r_yes);
	btn_1->SetBounds(x1,y0-h,x1+200,y0);
	btn_1->SetDescriptor(text1);
	btn_1->SetParent(parent);
	btn_1->Show();
	btn_1->SetMsg((intptr_t) var, 0);
	
	btn_0->AddRadioFriend(btn_1);
	btn_1->AddRadioFriend(btn_0);
	
	if (*var == 0) 
		btn_0->SetValue(1.0);
	else
		btn_1->SetValue(1.0);
		
	btn_0->AddListener(parent);
	btn_1->AddListener(parent);
	
}

void WED_Settings::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
//	printf("WS Msg %p %ld %ld\n", inSrc, inMsg, inParam);
	
	if(inMsg == (intptr_t) &gIsFeet)
	{
			gIsFeet = inParam;
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
	}
	if(inMsg == (intptr_t) &gInfoDMS)
	{
			gInfoDMS = inParam;
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
	}
	
}


WED_Settings::WED_Settings(GUI_Commander * cmdr) : GUI_Window("WED Preferences", xwin_style_movable | xwin_style_centered | xwin_style_modal, settings_bounds, cmdr)
{
	char * texture = "check_buttons.png";
	int k_yes[4] = { 0, 1, 1, 3 };
	int k_no[4]  = { 0, 2, 1, 3 };

	int bounds[4];
	GUI_Packer * packer = new GUI_Packer;
	packer->SetParent(this);
	packer->SetSticky(1,1,1,1);
	packer->Show();
	GUI_Pane::GetBounds(bounds);
	packer->SetBounds(bounds);
	packer->SetBkgkndImage("about.png");
	
	RadioButton(220, 360 , this, &gIsFeet, "Length Units", "Meters", "Feet");
	RadioButton(220, 320 , this, &gInfoDMS, "Info Bar\nCoordinates", "DD.DDDDD", "DD MM SS");
	
	GUI_Button * moderator_btn = new GUI_Button(texture,btn_Check,k_no, k_no, k_yes, k_yes);
	moderator_btn->SetBounds(350,280,500,280+GUI_GetImageResourceHeight(texture)/3);
	moderator_btn->Show();
	moderator_btn->SetDescriptor("Moderator Mode");
	moderator_btn->SetParent(this);
		
//	okay_btn->SetMsg(AsyncDestroy(),0);
}

#if LIN
WED_Application::WED_Application(int& argc, char* argv[]) : GUI_Application(argc, argv),
#elif APL
WED_Application::WED_Application() : GUI_Application("WEDMainMenu"),
#else
	WED_Application::WED_Application() :
#endif
		mAboutBox(NULL)
{
}

WED_Application::~WED_Application()
{
}

void	WED_Application::SetAbout(GUI_Window * about_box)
{
	mAboutBox = about_box;
}

void	WED_Application::OpenFiles(const vector<string>& inFiles)
{
}

int		WED_Application::HandleCommand(int command)
{
	switch(command) {
	case wed_HelpScenery:
		// LR maintains a forwarding directory for all v10-class products 
		// so that we can restructure our content management without breaking binary
		// apps in-field.  So...this is the perma-marker for WED 1.1 scenery help.
		GUI_LaunchURL(WED_URL_HELP_SCENERY);
		return 1;
	case wed_HelpManual:
		{
			// We used to have a nice PDF published with WED, but...WED is changing fast
			// and it stops going final to have to wait for doc complete.  So let's put
			// the manual online and off we go.
//			string path;
//			if (GUI_GetTempResourcePath("WEDManual.pdf",path))
//			{
//				GUI_LaunchURL(path.c_str());
//			}
			GUI_LaunchURL(WED_URL_MANUAL);
		}
		return 1;
	default:
		return GUI_Application::HandleCommand(command);
	}
}

int		WED_Application::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case gui_Undo:		ioName = "&Undo"; return 0;
	case gui_Redo:		ioName = "&Redo"; return 0;
	case wed_HelpScenery:	return 1;
	case wed_HelpManual:	return 1;
	default:				return GUI_Application::CanHandleCommand(command, ioName, ioCheck);
	}
}

void	WED_Application::AboutBox(void)
{
	if (mAboutBox)	mAboutBox->Show();
}

void	WED_Application::Preferences(void)
{
//	DoUserAlert("WED does not yet have any preferences.");
	GUI_Window * mSettingsWin = new WED_Settings( this );
	mSettingsWin->Show();
}

bool	WED_Application::CanQuit(void)
{
//	if (ConfirmMessage("Are you sure you want to quit WED", "Quit", "Cancel"))	return true;	return false;
	return WED_Document::TryCloseAll();
}

