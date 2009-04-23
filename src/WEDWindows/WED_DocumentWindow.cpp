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

#include "WED_Document.h"
//#include "WED_Progress.h"
//#include "XESIO.h"
#include "AptIO.h"
//#include "MapAlgs.h"
#include "WED_Messages.h"
#include "GUI_Menus.h"
#include "WED_UndoMgr.h"
#include "WED_DocumentWindow.h"
#include "WED_MapPane.h"
#include "WED_TCEPane.h"
#include "WED_PropertyPane.h"
#include "WED_AptIE.h"
#include "GUI_TabPane.h"
#include "WED_Thing.h"
#include "WED_UIMeasurements.h"
#include "WED_Menus.h"
#include "WED_Select.h"
#include "WED_Colors.h"
#include "GUI_Splitter.h"
#include "WED_GroupCommands.h"
#include "WED_DSFExport.h"
#include "WED_DSFImport.h"
#include "WED_PropertyHelper.h"
#include "WED_LibraryPane.h"

#if LIN
// temporary, testing stuff here
#include "GUI_Fonts.h"
#include "GUI_Resources.h"
#include "WED_ToolInfoAdapter.h"
#endif

int kDefaultDocSize[4] = { 0, 0, 512,384 };

WED_DocumentWindow::WED_DocumentWindow(
	 		const char * 	inTitle,
	 		GUI_Commander * inCommander,
	 		WED_Document *	inDocument) :
	GUI_Window(inTitle, xwin_style_resizable|xwin_style_visible|xwin_style_fullscreen, kDefaultDocSize, inCommander),
	mDocument(inDocument)
{
	#if LIN
	//XWin::mMenuOffset = 20;
	#endif

	GUI_Window::SetDescriptor(mDocument->GetFilePath());
	mDocument->AddListener(this);

//	WED_Thing * root = SAFE_CAST(WED_Thing,mDocument->GetArchive()->Fetch(1));
//	WED_Select * s = SAFE_CAST(WED_Select,root->GetNamedChild("selection"));
//	DebugAssert(root);
//	DebugAssert(s);

	GUI_Packer * packer = new GUI_Packer;
	packer->SetParent(this);
	packer->SetSticky(1,1,1,1);
	packer->Show();
	int		splitter_b[4];
	GUI_Pane::GetBounds(splitter_b);
	packer->SetBounds(splitter_b);

	/****************************************************************************************************************************************************************
	 * MAP VIEW
	****************************************************************************************************************************************************************/

//	int		splitter_b[4];
	mMainSplitter  = new GUI_Splitter(gui_Split_Horizontal);
	mMainSplitter2 = new GUI_Splitter(gui_Split_Horizontal);
	if (WED_UIMeasurement("one_big_gradient"))		mMainSplitter->SetImage ("gradient.png");
	else											mMainSplitter->SetImage1("gradient.png");
	mMainSplitter->SetParent(packer);
	mMainSplitter->Show();
//	GUI_Pane::GetBounds(splitter_b);
//	mMainSplitter->SetBounds(splitter_b);
	mMainSplitter->SetSticky(1,1,1,1);

	WED_LibraryPane * lib = new WED_LibraryPane(this, inDocument->GetLibrary());
	lib->SetParent(mMainSplitter);
	lib->Show();
	lib->SetSticky(1,1,0,1);

	mMainSplitter2->SetParent(mMainSplitter);
	mMainSplitter2->Show();
	mMainSplitter2->SetSticky(1,1,1,1);

	double	lb[4];
	mDocument->GetBounds(lb);
	mMapPane = new WED_MapPane(this, lb, inDocument,inDocument->GetArchive(),lib->GetAdapter());
	lib->GetAdapter()->SetMap(mMapPane);
	mMapPane->SetParent(mMainSplitter2);
	mMapPane->Show();
	mMapPane->SetSticky(1,1,0.5,1);

#if 0
// temporary, testing stuff here
	WED_ToolInfoAdapter* mInfoAdapter = new WED_ToolInfoAdapter(GUI_GetImageResourceHeight("property_bar.png") / 2);
	GUI_TextTable* mTextTable = new GUI_TextTable(inCommander,10);
	GUI_Table* mTable = new GUI_Table(1);
	mTextTable->SetColors(WED_Color_RGBA(wed_Table_Gridlines),
						  WED_Color_RGBA(wed_Table_Select),
						  WED_Color_RGBA(wed_Table_Text),
						  WED_Color_RGBA(wed_PropertyBar_Text),
						  WED_Color_RGBA(wed_Table_Drag_Insert),
						  WED_Color_RGBA(wed_Table_Drag_Into));

	mTextTable->SetFont(font_UI_Small);
	mTable->SetGeometry(mInfoAdapter);
	mTable->SetContent(mTextTable);
	//mTextTable->SetProvider(mInfoAdapter);
	mTable->SizeShowAll();
	mTextTable->SetParentTable(mTable);
	mTextTable->AddListener(mTable);
	mTextTable->SetImage("property_bar.png", 2);
	//mInfoAdapter->AddListener(mTable);
	mTable->SetParent(packer);
	mTable->Show();
	packer->PackPane(mTable, gui_Pack_Top);
	mTable->SetSticky(1,0,1,1);
#endif

	GUI_Pane * top_bar = mMapPane->GetTopBar();
	top_bar->SetParent(packer);
	top_bar->Show();
	packer->PackPane(top_bar, gui_Pack_Top);
	top_bar->SetSticky(1,0,1,1);
	packer->PackPane(mMainSplitter, gui_Pack_Center);

	/****************************************************************************************************************************************************************
	 * PROPERTY-SIDE
	****************************************************************************************************************************************************************/

	// --------------- Splitter and tabs ---------------

	mPropSplitter = new GUI_Splitter(gui_Split_Vertical);
	if (!WED_UIMeasurement("one_big_gradient")) {
		mPropSplitter->SetImage1("gradient.png");
		mPropSplitter->SetImage2("gradient.png");
	}
	mPropSplitter->SetParent(mMainSplitter2);
	mPropSplitter->Show();
	GUI_Pane::GetBounds(splitter_b);
	mPropSplitter->SetBounds(splitter_b);
	mPropSplitter->SetSticky(0.5,1,1,1);

	GUI_TabPane * prop_tabs = new GUI_TabPane(this);
	prop_tabs->SetParent(mPropSplitter);
	prop_tabs->Show();
	prop_tabs->SetSticky(1,1,1,0.5);
	prop_tabs->SetTextColor(WED_Color_RGBA(wed_Tabs_Text));

	// --------------- Selection ---------------


	static const char * sel_t[] = { "Name", "Type", NULL };
	static		 int	sel_w[] = { 100, 100 };

	WED_PropertyPane * prop_pane1 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);
	prop_tabs->AddPane(prop_pane1, "Selection");

	// --------------- AIRPORT

	static const char * air_t[] = { "Name", "Type", "Field Elevation", "Has ATC", "ICAO Identifier", "Frequency", NULL };
	static		 int	air_w[] = { 200, 100, 100, 75, 100, 150  };
	static const char * air_f[] = { "WED_Airport", "WED_ATCFrequency", NULL };

	WED_PropertyPane * prop_pane2 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, air_t, air_w,inDocument->GetArchive(), propPane_Filtered, air_f);
	prop_tabs->AddPane(prop_pane2, "Airports");

	// --------------- LIGHTS, SIGNS, BEACONS ---------------

	static const char * sin_t[] = { "Name", "Type", "Size", "Angle", 0 };
	static		 int	sin_w[] = { 200, 100, 100, 100  };
	static const char * sin_f[] = { "WED_Airport", "WED_LightFixture", "WED_AirportBeacon", "WED_AirportSign", "WED_Group", NULL };

	WED_PropertyPane * prop_pane3 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sin_t, sin_w,inDocument->GetArchive(), propPane_Filtered, sin_f);
	prop_tabs->AddPane(prop_pane3, "Signs");

	// --------------- RUNWAYS ---------------

	static const char * rwy_t[] = { "REIL 2", "TDZ Lights 2", "Approach Lights 2", "Markings 2", "Blastpad 2", "Displaced Threshhold 2",
									"REIL 1", "TDZ Lights 1", "Approach Lights 1", "Markings 1", "Blastpad 1", "Displaced Threshhold 1",
									"Distance Signs", "Edge Lights", "Centerline Lights", "Roughness", "Shoulder", "Surface", "Name", 0 };
	static		 int	rwy_w[] = { 150, 150, 150, 150, 150, 150,
									150, 150, 150, 150, 150, 150,
									150, 150, 150, 150, 150, 150, 150 };
	static const char * rwy_f[] = { "WED_Airport", "WED_Runway", NULL };

	WED_PropertyPane * prop_pane4 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, rwy_t, rwy_w,inDocument->GetArchive(), propPane_FilteredVertical, rwy_f);
	prop_tabs->AddPane(prop_pane4, "Runways");

	// --------------- TAXIWAYS ---------------

	static const char * tax_t[] = { "Name", "Surface", "Roughness", "Texture Heading", 0 };
	static		 int	tax_w[] = { 200, 150, 100, 150  };
	static const char * tax_f[] = { "WED_Airport", "WED_Taxiway", "WED_Group", NULL };

	WED_PropertyPane * prop_pane5 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, tax_t, tax_w,inDocument->GetArchive(), propPane_Filtered, tax_f);
	prop_tabs->AddPane(prop_pane5, "Taxiways");

	// --------------- HELIPADS ---------------

	static const char * hel_t[] = { "Name", "Surface", "Markings", "Shoulder", "Roughness", "Lights", 0 };
	static		 int	hel_w[] = { 200, 130, 130, 130, 100, 130 };
	static const char * hel_f[] = { "WED_Airport", "WED_Helipad", NULL };

	WED_PropertyPane * prop_pane6 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, hel_t, hel_w,inDocument->GetArchive(), propPane_Filtered, hel_f);
	prop_tabs->AddPane(prop_pane6, "Helipads");

	// ---------------- TCE -------------
	mTCEPane = new WED_TCEPane(this, inDocument,inDocument->GetArchive());
	prop_tabs->AddPane(mTCEPane, "Texture");

	// --------------- Hierarchy  View ---------------

	static const char * titles[] =  { "Locked", "Hidden", "Name", 0 };
	static int widths[] =			{ 50,		50,		200		};

	WED_PropertyPane * prop_pane = new WED_PropertyPane(this, inDocument, titles, widths,inDocument->GetArchive(), propPane_Hierarchy, 0);
	prop_pane->SetParent(mPropSplitter);
	prop_pane->Show();
	prop_pane->SetSticky(1,0.5,1,1);

	/****************************************************************************************************************************************************************
	 * FINAL CLEANUP
	****************************************************************************************************************************************************************/

	int zw[2];
	XWin::GetBounds(zw,zw+1);

//	int main_split = inDocument->ReadIntPref("window/main_split",(zw[0]) * 0.5f);
	int prop_split = inDocument->ReadIntPref("window/prop_split",(zw[1]) * 0.5f);

//	if (main_split > (zw[0])) main_split = (zw[0]) * 0.5f;
	if (prop_split > (zw[1])) prop_split = (zw[1]) * 0.5f;

	mMainSplitter->AlignContentsAt(300);
	mMainSplitter2->AlignContentsAt(600);
	mPropSplitter->AlignContentsAt(prop_split);
	mMapPane->ZoomShowAll();

	mMapPane->FromPrefs(inDocument);
	gIsFeet = inDocument->ReadIntPref("doc/use_feet",gIsFeet);

}

WED_DocumentWindow::~WED_DocumentWindow()
{
}

int	WED_DocumentWindow::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if ( mMapPane->Map_KeyPress(inKey, inVK, inFlags)) return 1;
	if (inKey == GUI_KEY_DELETE && (inFlags & gui_DownFlag))
	if (WED_CanClear(mDocument)) { DispatchHandleCommand(gui_Clear); return 1; }	// run through dispatcher to make sure we call the appropriate hooks!
	return 0;
}

int	WED_DocumentWindow::HandleCommand(int command)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();
	switch(command) {
	case wed_RestorePanes:
		{
			int zw[2];
			XWin::GetBounds(zw,zw+1);
			int main_split = (zw[0]) * 0.5f;
			int prop_split = (zw[1]) * 0.5f;
			mMainSplitter->AlignContentsAt(main_split);
			mPropSplitter->AlignContentsAt(prop_split);
		}
		return 1;
	case gui_Undo:	if (um->HasUndo()) { um->Undo(); return 1; }	break;
	case gui_Redo:	if (um->HasRedo()) { um->Redo(); return 1; }	break;
	case gui_Clear:		WED_DoClear(mDocument); return 1;
	case wed_Crop:		WED_DoCrop(mDocument); return 1;
	case wed_Split:		WED_DoSplit(mDocument); return 1;
	case wed_Reverse:	WED_DoReverse(mDocument); return 1;
	case gui_Duplicate:	WED_DoDuplicate(mDocument, true); return 1;
	case wed_Group:		WED_DoGroup(mDocument); return 1;
	case wed_Ungroup:	WED_DoUngroup(mDocument); return 1;
	case wed_MoveFirst:	WED_DoReorder(mDocument,-1,1);	return 1;
	case wed_MovePrev:	WED_DoReorder(mDocument,-1,0);	return 1;
	case wed_MoveNext:	WED_DoReorder(mDocument, 1,0);	return 1;
	case wed_MoveLast:	WED_DoReorder(mDocument, 1,1);	return 1;

	case wed_AddATCFreq:WED_DoMakeNewATC(mDocument); return 1;
	case wed_CreateApt:	WED_DoMakeNewAirport(mDocument); return 1;
	case wed_EditApt:	WED_DoSetCurrentAirport(mDocument); return 1;
	case gui_Close:		mDocument->TryClose();	return 1;
	case gui_Save:		mDocument->Save();	return 1;
	case gui_Revert:	mDocument->Revert();	return 1;

	case gui_SelectAll:		WED_DoSelectAll(mDocument);		return 1;
	case gui_SelectNone:	WED_DoSelectNone(mDocument);		return 1;
	case wed_SelectParent:	WED_DoSelectParent(mDocument);		return 1;
	case wed_SelectChild:	WED_DoSelectChildren(mDocument);	return 1;
	case wed_SelectVertex:	WED_DoSelectVertices(mDocument);	return 1;
	case wed_SelectPoly:	WED_DoSelectPolygon(mDocument);	return 1;

	case wed_ExportApt:		WED_DoExportApt(mDocument); return 1;
	case wed_ExportDSF:		WED_DoExportDSF(mDocument);	return 1;
	case wed_ImportApt:		WED_DoImportApt(mDocument,mDocument->GetArchive()); return 1;
	case wed_ImportDSF:		WED_DoImportDSF(mDocument); return 1;
	case wed_Validate:		if (WED_ValidateApt(mDocument)) DoUserAlert("Your layout is valid - no problems were found."); return 1;

	case wed_UnitFeet:	gIsFeet=1;Refresh(); return 1;
	case wed_UnitMeters:gIsFeet=0;Refresh(); return 1;

	default: return mMapPane->Map_HandleCommand(command);	break;
	}
	return 0;
}

int	WED_DocumentWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();
	switch(command) {
	case wed_RestorePanes:	return 1;
	case gui_Undo:		if (um->HasUndo())	{ ioName = um->GetUndoName();	return 1; }
						else				{								return 0; }
	case gui_Redo:		if (um->HasRedo())	{ ioName = um->GetRedoName();	return 1; }
						else				{								return 0; }
	case gui_Clear:		return	WED_CanClear(mDocument);
	case wed_Crop:		return	WED_CanCrop(mDocument);
	case gui_Close:															return 1;
	case wed_Split:		return WED_CanSplit(mDocument);
	case wed_Reverse:	return WED_CanReverse(mDocument);
	case gui_Duplicate:	return WED_CanDuplicate(mDocument);
	case wed_Group:		return WED_CanGroup(mDocument);
	case wed_Ungroup:	return WED_CanUngroup(mDocument);
	case wed_AddATCFreq:return WED_CanMakeNewATC(mDocument);
	case wed_CreateApt:	return WED_CanMakeNewAirport(mDocument);
	case wed_EditApt:	return WED_CanSetCurrentAirport(mDocument, ioName);
	case wed_MoveFirst:	return WED_CanReorder(mDocument,-1,1);
	case wed_MovePrev:	return WED_CanReorder(mDocument,-1,0);
	case wed_MoveNext:	return WED_CanReorder(mDocument, 1,0);
	case wed_MoveLast:	return WED_CanReorder(mDocument, 1,1);

	case gui_Save:		return mDocument->IsDirty();
	case gui_Revert:	return mDocument->IsDirty();

	case gui_SelectAll:		return WED_CanSelectAll(mDocument);
	case gui_SelectNone:	return WED_CanSelectNone(mDocument);
	case wed_SelectParent:	return WED_CanSelectParent(mDocument);
	case wed_SelectChild:	return WED_CanSelectChildren(mDocument);
	case wed_SelectVertex:	return WED_CanSelectVertices(mDocument);
	case wed_SelectPoly:	return WED_CanSelectPolygon(mDocument);

	case wed_ExportApt:		return WED_CanExportApt(mDocument);
	case wed_ExportDSF:		return WED_CanExportDSF(mDocument);
	case wed_ImportApt:		return WED_CanImportApt(mDocument);
	case wed_ImportDSF:		return WED_CanImportApt(mDocument);
	case wed_Validate:		return 1;

	case wed_UnitFeet:	ioCheck= gIsFeet;return 1;
	case wed_UnitMeters:ioCheck=!gIsFeet;return 1;

	default:																return mMapPane->Map_CanHandleCommand(command, ioName, ioCheck);
	}
}

void	WED_DocumentWindow::ReceiveMessage(
				GUI_Broadcaster *		inSrc,
				intptr_t				inMsg,
				intptr_t				inParam)
{
	if(inMsg == msg_DocumentDestroyed)
		delete this;
	if (inMsg == msg_DocWillSave)
	{
		IDocPrefs * prefs = reinterpret_cast<IDocPrefs *>(inParam);
		mMapPane->ToPrefs(prefs);
		prefs->WriteIntPref("doc/use_feet",gIsFeet);
		prefs->WriteIntPref("window/main_split",mMainSplitter->GetSplitPoint());
		prefs->WriteIntPref("window/prop_split",mPropSplitter->GetSplitPoint());
	}
	if (inMsg == msg_DocLoaded)
	{
		IDocPrefs * prefs = reinterpret_cast<IDocPrefs *>(inParam);
		mMapPane->FromPrefs(prefs);
		gIsFeet = prefs->ReadIntPref("doc/use_feet",gIsFeet);
	}
}

bool	WED_DocumentWindow::Closed(void)
{
	mDocument->TryClose();
	return false;
}

