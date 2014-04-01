/* 
 * Copyright (c) 2014, Laminar Research.
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

#include "GUI_FormWindow.h"
#include "GUI_Button.h"
#include "GUI_Packer.h"
#include "GUI_Resources.h"
#include "GUI_TextField.h"
#include "GUI_Label.h"
#include "GUI_Messages.h"
#include "AssertUtils.h"
#include "WED_Colors.h"		// This should not be here but we need some theming stuff, not build into GUI itself..maybe move this whole thing into WED?
/* 

	TODO:
		no cleartext password
		key limiting per form field
		tab, escape, return handling
		kill window when parent dies (check if apt needs this too)
		better layout
		investigate 'big' text fields?

 */

static int form_bounds_default[4] = { 0, 0, 500, 50 };

enum { 
	gui_form_ok = GUI_APP_MESSAGES,
	gui_form_cancel
};


GUI_FormWindow::GUI_FormWindow(
								GUI_Commander *			cmdr,
								const string&			title,
								const string&			ok_label,
								const string&			cancel_label,
								void (*					submit_func)(GUI_FormWindow *, void *),
								void *					submit_ref) :
	GUI_Window(title.c_str(), xwin_style_centered|xwin_style_movable|xwin_style_modal, form_bounds_default, cmdr),
	mSubmitFunc(submit_func),
	mSubmitRef(submit_ref)
{
	int bounds[4];
	GUI_Packer * packer = new GUI_Packer;
	packer->SetParent(this);
	packer->SetSticky(1,1,1,1);
	packer->Show();
	GUI_Pane::GetBounds(bounds);
	packer->SetBounds(bounds);
	packer->SetBkgkndImage ("gradient.png");

	int k_reg[4] = { 0, 0, 1, 3 };
	int k_hil[4] = { 0, 1, 1, 3 };
	
	GUI_Button * okay_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	okay_btn->SetBounds(105,5,205,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	okay_btn->Show();
	okay_btn->SetSticky(0,1,1,0);
	okay_btn->SetDescriptor(ok_label);
	okay_btn->SetMsg(gui_form_ok,0);

	GUI_Button * cncl_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	cncl_btn->SetBounds(5,5,105,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	cncl_btn->Show();
	cncl_btn->SetSticky(1,1,0,0);
	cncl_btn->SetDescriptor(cancel_label);
	cncl_btn->SetMsg(gui_form_cancel,0);
	
	GUI_Pane * holder = new GUI_Pane;
	holder->SetBounds(0,0,210,GUI_GetImageResourceHeight("push_buttons.png") / 3 + 10);
	
	
	okay_btn->SetParent(holder);
	okay_btn->AddListener(this);
	cncl_btn->SetParent(holder);
	cncl_btn->AddListener(this);
	
	holder->SetParent(packer);
	holder->Show();
	holder->SetSticky(1,1,1,0);
	
	packer->PackPane(holder,gui_Pack_Bottom);
	
	int final_ok_bounds[4];
	okay_btn->GetBounds(final_ok_bounds);
	
	mInsertBottom = final_ok_bounds[3];
}

GUI_FormWindow::~GUI_FormWindow()
{
}

void		GUI_FormWindow::AddField(
								int						id,
								const string&			label_text,
								const string&			default_text)
{
	DebugAssert(id > 0);
	GUI_Label * label = new GUI_Label();
	GUI_TextField * text = new GUI_TextField(false, this);
	text->SetID(id);
	label->SetDescriptor(label_text);
	text->SetDescriptor(default_text);

	int wbounds[4];
	this->GUI_Pane::GetBounds(wbounds);
	
	int split = (wbounds[0] + wbounds[2]) / 5;
	
	label->SetBounds(wbounds[0] + 10, mInsertBottom, split - 5, mInsertBottom + 20);
	text->SetBounds(split + 5, mInsertBottom, wbounds[2] - 10, mInsertBottom + 20);
	
	label->SetColors(WED_Color_RGBA(wed_Table_Text));
	text->SetColors(
			WED_Color_RGBA(wed_TextField_Text),
			WED_Color_RGBA(wed_TextField_Hilite),
			WED_Color_RGBA(wed_TextField_Bkgnd),
			WED_Color_RGBA(wed_TextField_FocusRing));
	
	label->SetParent(this);
	text->SetParent(this);
	label->SetSticky(1,0,0,1);
	text->SetSticky(0,0,1,1);
	label->Show();
	text->Show();
	
	this->Resize(wbounds[2],wbounds[3] + 30);
	
}


void		GUI_FormWindow::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	if(inMsg == gui_form_ok)
		mSubmitFunc(this,mSubmitRef);
	
	this->AsyncDestroy();
}

string		GUI_FormWindow::GetField(
								int						id)
{
	GUI_Pane * who = FindByID(id);
	string ret;
	if(who) who->GetDescriptor(ret);
	return ret;
}
