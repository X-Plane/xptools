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

#include "WED_TCEPane.h"
#include "WED_TCE.h"
#include "WED_TCEToolNew.h"
#include "WED_Archive.h"
#include "GUI_ToolBar.h"
#include "IDocPrefs.h"
#include "WED_TCEToolAdapter.h"
#include "WED_TCEVertexTool.h"
#include "WED_TCEMarqueeTool.h"
#include "GUI_Resources.h"
#include "GUI_Table.h"
#include "WED_ToolInfoAdapter.h"
#include "WED_Colors.h"
#include "GUI_Fonts.h"
#include "WED_TCEDebugLayer.h"

static char	kToolKeys[] = {
	'v', 'e'
};



WED_TCEPane::WED_TCEPane(GUI_Commander * cmdr, IResolver * resolver, WED_Archive * archive) : GUI_Commander(cmdr), mResolver(resolver)
{
	mTCE = new WED_TCE(resolver);
	mTools.push_back(new WED_TCEToolAdapter("Vertex Tool",mTCE, mTCE, resolver, new WED_TCEVertexTool("Vertex Tool", mTCE, mTCE, resolver)));
	mTools.push_back(new WED_TCEToolAdapter("Marquee Tool",mTCE, mTCE, resolver, new WED_TCEMarqueeTool("Marquee Tool", mTCE, mTCE, resolver)));




	mInfoAdapter = new WED_ToolInfoAdapter(GUI_GetImageResourceHeight("property_bar.png") / 2);
	mTextTable = new GUI_TextTable(cmdr,10);
	mTable = new GUI_Table(1);

	mTextTable->SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_PropertyBar_Text),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable->SetFont(font_UI_Small);


	mToolbar = new GUI_ToolBar(2,1,"tce_tools.png");
	mToolbar->SizeToBitmap();
	mToolbar->Show();
	mToolbar->SetParent(this);
	mToolbar->SetSticky(1,0,0,1);
	this->PackPane(mToolbar,gui_Pack_Top);
	mToolbar->SizeToBitmap();
	mToolbar->AddListener(this);
	vector<string>	tips;

	for (int n = 0; n < mTools.size(); ++n)
	{
		string tip(mTools[n]->GetToolName());
		if (kToolKeys[n])
		{
			char buf[5] = { " [x]" };
			buf[2] = toupper(kToolKeys[n]);
			tip += buf;
		}
		tips.push_back(tip);
	}

	mToolbar->SetToolTips(tips);





	mTable->SetGeometry(mInfoAdapter);
	mTable->SetContent(mTextTable);
	mTextTable->SetProvider(mInfoAdapter);
	mTable->SetParent(this);
	mTable->Show();
	mTable->SizeShowAll();
	mTextTable->SetParentTable(mTable);
	mTable->SetSticky(1,0,1,1);
//	this->PackPane(mTable, gui_Pack_Top);
	this->PackPaneToRight(mTable, gui_Pack_Top, mToolbar);
	mTextTable->AddListener(mTable);
	mTextTable->SetImage("property_bar.png", 2);

	mInfoAdapter->AddListener(mTable);


	


	GUI_ScrollerPane * map_scroller = new GUI_ScrollerPane(1,1);
	map_scroller->SetParent(this);
	map_scroller->Show();
	map_scroller->SetSticky(1,1,1,1);

	this->PackPane(map_scroller, gui_Pack_Center);

	mTCE->SetParent(map_scroller);
	mTCE->Show();
	map_scroller->PositionInContentArea(mTCE);
	map_scroller->SetContent(mTCE);
	mTCE->SetBounds(0,0,10,10);

//	mTCE->SetMapVisibleBounds(0,0,1,1);
	mTCE->SetMapLogicalBounds(0,0,1,1);

	mTCE->ZoomShowAll();

	mLayers.push_back(					new WED_TCEDebugLayer(mTCE, mTCE, resolver));

	for(vector<WED_TCELayer *>::iterator l = mLayers.begin(); l != mLayers.end(); ++l)
		mTCE->AddLayer(*l);
	for(vector<WED_TCEToolNew *>::iterator t = mTools.begin(); t != mTools.end(); ++t)
		mTCE->AddLayer(*t);

	mTCE->SetTool(mTools[0]);
	mInfoAdapter->SetTool(mTools[0]);
	
	mToolbar->SetValue(mTools.size()-1);

	// This is a bit of a hack.  The archive provides whole-doc "changed" messages at the minimum global times:
	// 1. On the commit of any operation.
	// 2. On the undo or redo of any operation.
	// So ... for lack of a better idea right now, we simply broker a connection between the source opf these
	// messages (secretly it's our document's GetArchive() member) and anyone who needs it (our map).

	archive->AddListener(mTCE);
}

WED_TCEPane::~WED_TCEPane()
{
	for(vector<WED_TCELayer *>::iterator l = mLayers.begin(); l != mLayers.end(); ++l)
		delete *l;
	for(vector<WED_TCEToolNew *>::iterator t = mTools.begin(); t != mTools.end(); ++t)
		delete *t;

	delete mTextTable;
	delete mInfoAdapter;
}

void	WED_TCEPane::ZoomShowAll(void)
{
//	double l,b,r,t;
//	mTCE->GetMapLogicalBounds(l,b,r,t);
//	mTCE->SetAspectRatio(1.0 / cos((b+t) * 0.5 * DEG_TO_RAD));
	mTCE->ZoomShowAll();
}

int		WED_TCEPane::TCE_KeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (mTCE->HandleKeyPress(inKey, inVK, inFlags)) return 1;
	for (int n = 0; n < sizeof(kToolKeys) / sizeof(kToolKeys[0]); ++n)
	if (kToolKeys[n])
	if (kToolKeys[n]==inKey)
	{
		mToolbar->SetValue(n);
		return 1;
	}
	return 0;
}

int		WED_TCEPane::TCE_HandleCommand(int command)
{
	switch(command) {
	default:		return 0;
	}
}

int		WED_TCEPane::TCE_CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	default:		return 0;
	}
	return 0;
}

void	WED_TCEPane::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
	int i = mToolbar->GetValue();
	WED_TCEToolNew * t = NULL;
	if (i >= 0 && i < mTools.size())
		t = mTools[i];
	mTCE->SetTool(t);
	mInfoAdapter->SetTool(t);	
}

void			WED_TCEPane::FromPrefs(IDocPrefs * prefs)
{
	double w,s,e,n;
	mTCE->GetMapVisibleBounds(w,s,e,n);
	mTCE->ZoomShowArea(
		prefs->ReadDoublePref("tce/top" ,w),
		prefs->ReadDoublePref("tce/bottom",s),
		prefs->ReadDoublePref("tce/right", e),
		prefs->ReadDoublePref("tce/top",n));
}

void			WED_TCEPane::ToPrefs(IDocPrefs * prefs)
{
	double w,s,e,n;
	mTCE->GetMapVisibleBounds(w,s,e,n);
	prefs->WriteDoublePref("tce/left" ,w);
	prefs->WriteDoublePref("tce/bottom",s);
	prefs->WriteDoublePref("tce/right", e);
	prefs->WriteDoublePref("tce/top",n);
}

