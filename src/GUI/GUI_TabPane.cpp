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

#include "GUI_TabPane.h"
#include "GUI_TabControl.h"
#include "GUI_ChangeView.h"
#include "GUI_Messages.h"

GUI_TabPane::GUI_TabPane(GUI_Commander * parent) :
	GUI_Commander(parent), GUI_Packer()
{
	mTabs = new GUI_TabControl;
	int b[4] = { 0, 0, 30, mTabs->GetNaturalHeight() };
	mTabs->SetParent(this);
	mTabs->Show();
	mTabs->SetBounds(b);
	mTabs->SetMin(0.0f);
	mTabs->SetMax(0.0f);
	mTabs->SetValue(0.0f);
	this->PackPane(mTabs, gui_Pack_Top);
	mTabs->SetSticky(1,0,1,1);

	mChangeView = new GUI_ChangeView(this);
	mChangeView->SetParent(this);
	mChangeView->Show();
	this->PackPane(mChangeView, gui_Pack_Center);
	mChangeView->SetSticky(1,1,1,1);

	mTabs->AddListener(this);
}

GUI_TabPane::~GUI_TabPane()
{
}

void			GUI_TabPane::SetTextColor(float color[4])
{
	mTabs->SetTextColor(color);
}

void			GUI_TabPane::SetTab(int n)
{
	mTabs->SetValue(n);
}

int				GUI_TabPane::GetTab(void) const
{
	return mTabs->GetValue();
}

GUI_Commander *	GUI_TabPane::GetPaneOwner(void)
{
	return mChangeView;
}

void			GUI_TabPane::AddPane(GUI_Pane * who, const char * title)
{
	if (mTabs->GetValue() == mChangeView->CountChildren())
		who->Show();
	else
		who->Hide();

	who->SetParent(mChangeView);
	int bounds[4];
	mChangeView->GetBounds(bounds);
	who->SetBounds(bounds);
	who->SetSticky(1,1,1,1);

	mTabs->SetMax(mChangeView->CountChildren()-1);

	string desc;
	mTabs->GetDescriptor(desc);
	if (!desc.empty()) desc += "\n";
	desc += title;
	mTabs->SetDescriptor(desc);
}


void	GUI_TabPane::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
	if (inMsg == GUI_CONTROL_VALUE_CHANGED)
	{
		mChangeView->SetSubView(mTabs->GetValue());
	}
}
