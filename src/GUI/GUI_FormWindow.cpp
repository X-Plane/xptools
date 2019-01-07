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
#include "GUI_Fonts.h"
#include "AssertUtils.h"
#include "WED_Colors.h"		// This should not be here but we need some theming stuff, not build into GUI itself..maybe move this whole thing into WED?
#include "STLUtils.h"
/* 

	TODO:
		no cleartext password
		key limiting per form field
		tab, escape, return handling
		kill window when parent dies (check if apt needs this too)
		better layout
		investigate 'big' text fields?

 */

static const int * calc_form_bounds(int b[4], int w, int h)
{
	b[0] = 0;
	b[1] = 0;
	b[2] = w;
	b[3] = h;
	return b;
}

static int form_bounds_default[4] = { 0, 0, 500, 50 };

enum { 
	gui_form_aux = GUI_APP_MESSAGES,
	gui_form_ok,
	gui_form_cancel
};


GUI_FormWindow::GUI_FormWindow(
								GUI_Commander *			cmdr,
								const string&			title,
								int						width,
								int						height) :
	GUI_Window(title.c_str(), xwin_style_centered|xwin_style_movable|xwin_style_modal|xwin_style_visible, calc_form_bounds(mFormBounds, width, height), cmdr)
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
	
	GUI_Button * aux_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	mAux = aux_btn;
	//Looks good as long as nobody resizes the window
	aux_btn->SetBounds(205,5,305, GUI_GetImageResourceHeight("push_buttons.png") / 3);
	aux_btn->Show();
	aux_btn->SetSticky(1,1,0,0);
	aux_btn->SetDescriptor("Learn More");
	aux_btn->SetMsg(gui_form_aux,0);

	GUI_Button * okay_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	mOK = okay_btn;
	okay_btn->SetBounds(105,5,205,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	okay_btn->Show();
	okay_btn->SetSticky(0,1,1,0);
	okay_btn->SetDescriptor("OK");
	okay_btn->SetMsg(gui_form_ok,0);
	mReturnSubmit = true;

	GUI_Button * cncl_btn = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
	cncl_btn->SetBounds(5,5,105,GUI_GetImageResourceHeight("push_buttons.png") / 3);
	cncl_btn->Show();
	cncl_btn->SetSticky(1,1,0,0);
	cncl_btn->SetDescriptor("Cancel");
	cncl_btn->SetMsg(gui_form_cancel,0);
	mCancel = cncl_btn;
	
	GUI_Pane * holder = new GUI_Pane;
	holder->SetBounds(0,0,210,GUI_GetImageResourceHeight("push_buttons.png") / 3 + 10);
	
	aux_btn->SetParent(holder);
	aux_btn->AddListener(this);

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
}

GUI_FormWindow::~GUI_FormWindow()
{
}

void		GUI_FormWindow::Reset(
								const string&			aux_label,
								const string&			ok_label,
								const string&			cancel_label,
								bool					auto_return)
{
	mReturnSubmit = auto_return;
	while(!mParts.empty())
	{
		delete mParts.back();
		mParts.pop_back();
	}
	
	mFocusRing.clear();
	
	if(cancel_label.empty())
	{
		mCancel->Hide();
	}
	else
	{
		mCancel->Show();
		mCancel->SetDescriptor(cancel_label);
	}

	if(aux_label.empty())
	{
		mAux->Hide();
	}
	else
	{
		mAux->Show();
		mAux->SetDescriptor(aux_label);
	}

	if(ok_label.empty())
	{
		mOK->Hide();
	}
	else
	{
		mOK->Show();
		mOK->SetDescriptor(ok_label);
	}

	if(cancel_label.empty())
	{
		mCancel->Hide();
	}
	else
	{
		mCancel->Show();
		mCancel->SetDescriptor(cancel_label);
	}

	mInsertY = mFormBounds[3] - 10;
}

void		GUI_FormWindow::AddLabel(const string&			msg)
{
	vector<string>	lines;
	tokenize_string(msg.begin(),msg.end(),back_inserter(lines), '\n');
	
	for(vector<string>::iterator l = lines.begin(); l != lines.end(); ++l)
	{

		GUI_Label * label = new GUI_Label();
		label->SetDescriptor(*l);

		int wbounds[4];
		this->GUI_Pane::GetBounds(wbounds);
		
		label->SetBounds(wbounds[0] + 10, mInsertY - 20, wbounds[2] - 10, mInsertY);
		
		mInsertY -= 30;
		
		label->SetColors(WED_Color_RGBA(wed_Table_Text));
		
		label->SetParent(this);
		label->SetSticky(1,0,1,1);
		label->Show();
		
		mParts.push_back(label);
	}

}



void		GUI_FormWindow::AddField(
								int						id,
								const string&			label_text,
								const string&			default_text,
								field_type				ft)
{
	DebugAssert(id > 0);
	GUI_Label * label = new GUI_Label();
	GUI_TextField * text = new GUI_TextField(false, this);
	if(ft == ft_password)
		text->SetPasswordChar('*');
	text->SetID(id);
	label->SetDescriptor(label_text);
	text->SetDescriptor(default_text);

	int wbounds[4];
	this->GUI_Pane::GetBounds(wbounds);
	
	int split = (wbounds[0] + wbounds[2]) / 5;
	
	int fh = (ft == ft_multi_line || ft == ft_big) ? 100 : 20;
	
	label->SetBounds(wbounds[0] + 10, mInsertY - 20, split - 5, mInsertY);
	text->SetBounds(split + 5, mInsertY - fh, wbounds[2] - 10, mInsertY);
	
	for(int i = 0; i <256;++i)
		text->SetKeyAllowed(i,isprint(i));
		
	text->SetKeyAllowed(GUI_KEY_BACK, true);
	text->SetKeyAllowed(GUI_KEY_DELETE, true);
	text->SetKeyAllowed(GUI_KEY_LEFT, true);
	text->SetKeyAllowed(GUI_KEY_RIGHT, true);
	text->SetKeyAllowed(GUI_KEY_UP, true);
	text->SetKeyAllowed(GUI_KEY_DOWN, true);
	text->SetKeyAllowed(GUI_KEY_RETURN, ft == ft_multi_line);

	text->SetVKAllowed(GUI_VK_RETURN, ft == ft_multi_line);
	text->SetVKAllowed(GUI_VK_ESCAPE, false);
	text->SetVKAllowed(GUI_VK_ENTER, false);
	
	float	line_h = GUI_GetLineHeight(font_UI_Basic);
	float	cell_h = line_h + 4.0f;
	float descent = GUI_GetLineDescent(font_UI_Basic);
	float	cell2line = (cell_h - line_h ) * 0.5f + descent;

	mInsertY -= (10+fh);
	
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
	
	mParts.push_back(label);
	mParts.push_back(text);
	mFocusRing.push_back(text);
}

void		GUI_FormWindow::AddFieldNoEdit(
								int						id,
								const string&			label_text,
								const string&			default_text)
{
	DebugAssert(id > 0);
	GUI_Label * label = new GUI_Label();
	GUI_Label * text = new GUI_Label();
	text->SetID(id);
	label->SetDescriptor(label_text);
	text->SetDescriptor(default_text);

	int wbounds[4];
	this->GUI_Pane::GetBounds(wbounds);
	
	int split = (wbounds[0] + wbounds[2]) / 5;
	
	label->SetBounds(wbounds[0] + 10, mInsertY - 20, split - 5, mInsertY);
	text->SetBounds(split + 5, mInsertY - 20, wbounds[2] - 10, mInsertY);
	
	mInsertY -= 30;
	
	label->SetColors(WED_Color_RGBA(wed_Table_Text));
	text->SetColors(WED_Color_RGBA(wed_Table_Text));
	
	label->SetParent(this);
	text->SetParent(this);
	label->SetSticky(1,0,0,1);
	text->SetSticky(0,0,1,1);
	label->Show();
	text->Show();
	
	mParts.push_back(label);
	mParts.push_back(text);
	
}

void		GUI_FormWindow::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	if(inMsg == gui_form_aux)
	{
		this->AuxiliaryAction();
	}
	else if(inMsg == gui_form_ok)
	{
		this->Submit();
	}
	else
	{
		this->Cancel();
	}
}

string		GUI_FormWindow::GetField(
								int						id)
{
	GUI_Pane * who = FindByID(id);
	string ret;
	if(who) who->GetDescriptor(ret);
	return ret;
}

int			GUI_FormWindow::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if(inVK == GUI_VK_RETURN && mReturnSubmit)
	{
		this->Submit();
		return 1;
	}
	if(inVK == GUI_VK_ESCAPE)
	{
		this->Cancel();
		return 1;
	}
	if(inVK == GUI_VK_TAB && !mFocusRing.empty())
	{
		GUI_Commander * now = this->GetFocusForCommander();
#if LIN
		vector<GUI_Commander*>::iterator i = std::find(mFocusRing.begin(),mFocusRing.end(),now);
#else
		vector<GUI_Commander*>::iterator i = find(mFocusRing.begin(),mFocusRing.end(),now);
#endif	
		int idx = distance(mFocusRing.begin(),i);
		
		int advance = (inFlags & gui_ShiftFlag) ? -1 : 1;
		idx += advance;
		if(idx < 0) idx += mFocusRing.size();
		idx = idx % mFocusRing.size();
		mFocusRing[idx]->TakeFocus();
	}
	return 0;
}
