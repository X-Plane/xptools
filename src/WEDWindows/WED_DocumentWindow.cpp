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

#include "WED_DocumentWindow.h"
#include "WED_Document.h"
//#include "WED_Progress.h"
//#include "XESIO.h"
#include "AptIO.h"
//#include "MapAlgs.h"
#include "WED_Messages.h"
#include "GUI_Menus.h"
#include "WED_UndoMgr.h"
#include "WED_AptIE.h"
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
#include "WED_SceneryPackExport.h"

#include "WED_MetadataUpdate.h"
#include "WED_GatewayExport.h"
#include "WED_GatewayImport.h"

#include "WED_DSFImport.h"
#include "WED_PropertyHelper.h"
#include "WED_LibraryPane.h"
#include "WED_LibraryPreviewPane.h"
//#include "WED_Orthophoto.h"
#include "WED_Routing.h"
#include "WED_ToolUtils.h"
#include "WED_Validate.h"


#if WITHNWLINK
#include "WED_Server.h"
#endif
#if LIN
// temporary, testing stuff here
#include "GUI_Fonts.h"
#include "GUI_Resources.h"
#include "WED_ToolInfoAdapter.h"
#endif

int kDefaultDocSize[4] = { 0, 0, 1024, 768 };

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
	XWin::SetFilePath(mDocument->GetFilePath().c_str(),false);
	mDocument->AddListener(this);
	mDocument->GetArchive()->AddListener(this);

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
	//BEWARE! HORIZONTAL IS VERTICAL AND VERTICAL IS HORIZONTAL!
	mMainSplitter  = new GUI_Splitter(gui_Split_Horizontal);
	mMainSplitter2 = new GUI_Splitter(gui_Split_Horizontal);
	if (WED_UIMeasurement("one_big_gradient"))		mMainSplitter->SetImage ("gradient.png");
	else											mMainSplitter->SetImage1("gradient.png");
	mMainSplitter->SetParent(packer);
	mMainSplitter->Show();
//	GUI_Pane::GetBounds(splitter_b);
//	mMainSplitter->SetBounds(splitter_b);
	mMainSplitter->SetSticky(1,1,1,1);
	
	/****************************************************************************************************************************************************************
	 * LIBRARY-SIDE
	****************************************************************************************************************************************************************/

	mLibSplitter = new GUI_Splitter(gui_Split_Vertical);
	if (!WED_UIMeasurement("one_big_gradient")) {
		mLibSplitter->SetImage1("gradient.png");
		mLibSplitter->SetImage2("gradient.png");
	}
	mLibSplitter->SetParent(mMainSplitter);
	mLibSplitter->Show();
	GUI_Pane::GetBounds(splitter_b);
	mLibSplitter->SetBounds(splitter_b);
	mLibSplitter->SetSticky(1,1,0,1);

	WED_LibraryPreviewPane * libprev = new WED_LibraryPreviewPane(mDocument->GetResourceMgr(), WED_GetTexMgr(mDocument));
	libprev->SetParent(mLibSplitter);
	libprev->Show();
	libprev->SetSticky(1,1,1,0);
	
	WED_LibraryPane * lib = new WED_LibraryPane(this, inDocument->GetLibrary());
	lib->SetParent(mLibSplitter);
	lib->Show();
	lib->SetSticky(1,1,1,1);

	/****************************************************************************************************************************************************************
	 * DA MAP
	****************************************************************************************************************************************************************/

	mMainSplitter2->SetParent(mMainSplitter);
	mMainSplitter2->Show();
	mMainSplitter2->SetSticky(1,1,1,1);


	double	lb[4];
	mDocument->GetBounds(lb);
	mMapPane = new WED_MapPane(this, lb, inDocument,inDocument->GetArchive(),lib->GetAdapter());
	lib->GetAdapter()->SetMap(mMapPane, libprev);
	mMapPane->SetParent(mMainSplitter2);
	mMapPane->Show();
	mMapPane->SetSticky(1,1,1,1);

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
	mPropSplitter->SetSticky(0,1,1,1);

	GUI_TabPane * prop_tabs = new GUI_TabPane(this);
	prop_tabs->SetParent(mPropSplitter);
	prop_tabs->Show();
	prop_tabs->SetSticky(1,1,1,0.5);
	prop_tabs->SetTextColor(WED_Color_RGBA(wed_Tabs_Text));
	prop_tabs->AddListener(mMapPane);

	// --------------- Selection ---------------

	static const char * sel_t[] = { "Name", "Type", NULL };
	static		 int	sel_w[] = { 100, 100 };

	WED_PropertyPane * prop_pane1 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);
	prop_tabs->AddPane(prop_pane1, "Selection");

	// --------------- Pavement Tab Mode ---------------

	WED_PropertyPane * prop_pane2 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);
	prop_tabs->AddPane(prop_pane2, "Pavement");

	// --------------- ATC Taxi + Flow ---------------

	WED_PropertyPane * prop_pane3 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);
	prop_tabs->AddPane(prop_pane3, "Taxi+Flow");

	// --------------- Lights and Markings ---------------

	WED_PropertyPane * prop_pane4 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);
	prop_tabs->AddPane(prop_pane4, "Light+Marking");

	// ---------------- 3D Mode ---------------------

	WED_PropertyPane * prop_pane5 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);
	prop_tabs->AddPane(prop_pane5, "3D Objects");

	// ---------------- Exclusions ------------------

	WED_PropertyPane * prop_pane6 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);
	prop_tabs->AddPane(prop_pane6, "Exclude+Boundary");

	// ---------------- TCE -------------
	mTCEPane = new WED_TCEPane(this, inDocument,inDocument->GetArchive());
	prop_tabs->AddPane(mTCEPane, "Texture");

	// --------------- Hierarchy  View ---------------

	static const char * titles[] =  { "Locked", "Hidden", "Name", 0 };
	static int widths[] =			{ 50,		50,		200		};

	mPropPane = new WED_PropertyPane(this, inDocument, titles, widths,inDocument->GetArchive(), propPane_Hierarchy, 0);
	mPropPane->SetParent(mPropSplitter);
	mPropPane->Show();
	mPropPane->SetSticky(1,0.5,1,1);

	/****************************************************************************************************************************************************************
	 * FINAL CLEANUP
	****************************************************************************************************************************************************************/

	int xy[2];
	int zw[2];
	XWin::GetBounds(zw,zw+1);
	XWin::GetWindowLoc(xy,xy+1);
	
	// This is a safety-hack.  The user's prefs may specify the window at a location that
	// is off screen,either because the prefs are borked or because the doc came from
	// a machine with a much larger dekstop.  So...
	//
	// Coming in we have the default rect for a window - hopefully it is BIG because we
	// pass xwin_style_fullscreen to XWin.  So if our currnet location does not overlap
	// with that AT ALL assume the window is off in space.
	//
	// TODO: someday check the window against the real desktop per platform.
	int safe_rect[4] = { xy[0] ,xy[1], xy[0] + zw[0], xy[1] + zw[1] };
	
	xy[0]  = inDocument->ReadIntPref("window/x_loc",xy[0]);
	xy[1]  = inDocument->ReadIntPref("window/y_loc",xy[1]);
	zw[0] = inDocument->ReadIntPref("window/width",zw[0]);
	zw[1] = inDocument->ReadIntPref("window/height",zw[1]);
	
	if(xy[0] < safe_rect[2] && xy[1] < safe_rect[3] && 
	  (xy[0] + zw[0]) >= safe_rect[0] && (xy[1] + zw[1]) >= safe_rect[1])
	{
		SetBounds(xy[0],xy[1],xy[0]+zw[0],xy[1]+zw[1]);
	}

	int main_split = inDocument->ReadIntPref("window/main_split",zw[0] / 5);
	int main_split2 = inDocument->ReadIntPref("window/main_split2",zw[0] * 2 / 3);
	int prop_split = inDocument->ReadIntPref("window/prop_split",zw[1] / 2);
	int prev_split = inDocument->ReadIntPref("window/prev_split",zw[1] / 3);


	mMainSplitter->AlignContentsAt(main_split);
	mMainSplitter2->AlignContentsAt(main_split2);
	mPropSplitter->AlignContentsAt(prop_split);
	mLibSplitter->AlignContentsAt(prev_split);
	mMapPane->ZoomShowAll();


	mMapPane->FromPrefs(inDocument);
	mPropPane->FromPrefs(inDocument,0);
	gIsFeet = inDocument->ReadIntPref("doc/use_feet",gIsFeet);
	gExportTarget = (WED_Export_Target) inDocument->ReadIntPref("doc/export_target",gExportTarget);
	
	//#if DEV
	//	PrintDebugInfo(0);
	//#endif
}

WED_DocumentWindow::~WED_DocumentWindow()
{
}

int	WED_DocumentWindow::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if ( mMapPane->Map_KeyPress(inKey, inVK, inFlags)) return 1;
	if ((inKey == GUI_KEY_BACK || inKey == GUI_KEY_DELETE ) && (inFlags & gui_DownFlag))
	if (WED_CanClear(mDocument)) { DispatchHandleCommand(gui_Clear); return 1; }	// run through dispatcher to make sure we call the appropriate hooks!
	return 0;
}

int	WED_DocumentWindow::HandleCommand(int command)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();

	//--Add Meta Data Sub Menu-----------------
	if(command > wed_AddMetaDataBegin && command < wed_AddMetaDataEnd)
	{
		WED_DoAddMetaData(mDocument, command);
		return 1;
	}
	//------------------------------------------//

	switch(command) {
	case wed_RestorePanes:
		{
			int zw[2];
			XWin::GetBounds(zw,zw+1);
			int main_split = zw[0] / 5;
			int main_split2 = zw[0] * 2 / 3;
			int prop_split = zw[1] / 2;
			int prev_split = zw[1] / 3;

			mMainSplitter->AlignContentsAt(main_split);
			mMainSplitter2->AlignContentsAt(main_split2);
			mPropSplitter->AlignContentsAt(prop_split);
			mLibSplitter->AlignContentsAt(prev_split);

		}
		return 1;
	case gui_Undo:	if (um->HasUndo()) { um->Undo(); return 1; }	break;
	case gui_Redo:	if (um->HasRedo()) { um->Redo(); return 1; }	break;
	case gui_Clear:		WED_DoClear(mDocument); return 1;
	case wed_Crop:		WED_DoCrop(mDocument); return 1;
	//case wed_Overlay:	WED_MakeOrthos(mDocument); return 1;
#if AIRPORT_ROUTING
//	case wed_MakeRouting:WED_MakeRouting(mDocument); return 1;
	case wed_Merge:		WED_DoMerge(mDocument); return 1;
#endif
	case wed_Split:		WED_DoSplit(mDocument); return 1;
	case wed_Align:		WED_DoAlign(mDocument); return 1;
	case wed_Orthogonalize:	WED_DoOrthogonalize(mDocument); return 1;
	case wed_RegularPoly:	WED_DoMakeRegularPoly(mDocument); return 1;
	case wed_Reverse:	WED_DoReverse(mDocument); return 1;
	case wed_Rotate:	WED_DoRotate(mDocument); return 1;
	case gui_Duplicate:	WED_DoDuplicate(mDocument, true); return 1;
	case wed_Group:		WED_DoGroup(mDocument); return 1;
	case wed_Ungroup:	WED_DoUngroup(mDocument); return 1;
	case wed_MoveFirst:	WED_DoReorder(mDocument,-1,1);	return 1;
	case wed_MovePrev:	WED_DoReorder(mDocument,-1,0);	return 1;
	case wed_MoveNext:	WED_DoReorder(mDocument, 1,0);	return 1;
	case wed_MoveLast:	WED_DoReorder(mDocument, 1,1);	return 1;
	case wed_BreakApartSpecialAgps: WED_DoBreakApartSpecialAgps(mDocument); return 1;
	case wed_ReplaceVehicleObj:  WED_DoReplaceVehicleObj(mDocument); return 1;
	case wed_AddATCFreq:WED_DoMakeNewATCFreq(mDocument); return 1;
#if AIRPORT_ROUTING
	case wed_AddATCFlow: WED_DoMakeNewATCFlow(mDocument); return 1;
	case wed_AddATCRunwayUse:WED_DoMakeNewATCRunwayUse(mDocument); return 1;
	case wed_AddATCTimeRule: WED_DoMakeNewATCTimeRule(mDocument); return 1;
	case wed_AddATCWindRule: WED_DoMakeNewATCWindRule(mDocument); return 1;	
#endif
	case wed_UpgradeRamps:	WED_UpgradeRampStarts(mDocument);	return 1;
	case wed_CreateApt:	WED_DoMakeNewAirport(mDocument); return 1;
	case wed_EditApt:	WED_DoSetCurrentAirport(mDocument); return 1;
	case gui_Close:		mDocument->TryClose();	return 1;
	case gui_Save:		mDocument->Save(); return 1;

	case gui_Revert:	mDocument->Revert();	return 1;

	case gui_SelectAll:		WED_DoSelectAll(mDocument);		return 1;
	case gui_SelectNone:	WED_DoSelectNone(mDocument);		return 1;
	case wed_SelectParent:	WED_DoSelectParent(mDocument);		return 1;
	case wed_SelectChild:	WED_DoSelectChildren(mDocument);	return 1;
	case wed_SelectVertex:	WED_DoSelectVertices(mDocument);	return 1;
	case wed_SelectPoly:	WED_DoSelectPolygon(mDocument);	return 1;
	case wed_SelectConnected:WED_DoSelectConnected(mDocument);	return 1;

#if AIRPORT_ROUTING
	case wed_SelectZeroLength:	if(!WED_DoSelectZeroLength(mDocument))		DoUserAlert("Your project has no zero-length ATC routing lines.");	return 1;
	case wed_SelectDoubles:		if(!WED_DoSelectDoubles(mDocument))			DoUserAlert("Your project has no doubled ATC routing nodes.");	return 1;
	case wed_SelectCrossing:	if(!WED_DoSelectCrossing(mDocument))		DoUserAlert("Your project has no crossed ATC routing lines.");	return 1;
	
	case wed_SelectLocalObjects:		WED_DoSelectLocalObjects(mDocument); return 1;
	case wed_SelectLibraryObjects:		WED_DoSelectLibraryObjects(mDocument); return 1;
	case wed_SelectDefaultObjects:		WED_DoSelectDefaultObjects(mDocument); return 1;
	case wed_SelectThirdPartyObjects:	WED_DoSelectThirdPartyObjects(mDocument); return 1;
	case wed_SelectMissingObjects:		WED_DoSelectMissingObjects(mDocument); return 1;
#endif
	case wed_UpdateMetadata:     WED_DoUpdateMetadata(mDocument); return 1;
	case wed_ExportApt:		WED_DoExportApt(mDocument); return 1;
	case wed_ExportPack:	WED_DoExportPack(mDocument); return 1;
#if HAS_GATEWAY	
	case wed_ExportToGateway:		WED_DoExportToGateway(mDocument); return 1;
#endif	
	case wed_ImportApt:		WED_DoImportApt(mDocument,mDocument->GetArchive(), mMapPane); return 1;
	case wed_ImportDSF:		WED_DoImportDSF(mDocument); return 1;
	case wed_ImportOrtho:
		mMapPane->Map_HandleCommand(command);
		return 1;
#if HAS_GATEWAY		
	case wed_ImportGateway: WED_DoImportFromGateway(mDocument,mMapPane); return 1;
#endif	
#if GATEWAY_IMPORT_FEATURES
	case wed_ImportGatewayExtract:	WED_DoImportDSFText(mDocument); return 1;
#endif	
	case wed_Validate:		if (WED_ValidateApt(mDocument)) DoUserAlert("Your layout is valid - no problems were found."); return 1;

	case wed_UnitFeet:	gIsFeet=1;Refresh(); return 1;
	case wed_UnitMeters:gIsFeet=0;Refresh(); return 1;

	case wed_Export900:	gExportTarget = wet_xplane_900;	Refresh(); return 1;
	case wed_Export1000:gExportTarget = wet_xplane_1000;	Refresh(); return 1;
	case wed_Export1021:gExportTarget = wet_xplane_1021;	Refresh(); return 1;
	case wed_Export1050:gExportTarget = wet_xplane_1050;	Refresh(); return 1;
	case wed_Export1100:gExportTarget = wet_xplane_1100;	Refresh(); return 1;
	case wed_ExportGateway:gExportTarget = wet_gateway;	Refresh(); return 1;
	
#if WITHNWLINK
	case wed_ToggleLiveView :
		{
			WED_Server * Serv = mDocument->GetServer();
			if(Serv)
			{
				if	(Serv->IsStarted()) Serv->DoStop();
				else					Serv->DoStart();
			}
		}
		return 1;
#endif
	default: return mMapPane->Map_HandleCommand(command);	break;
	}
	return 0;
}

int	WED_DocumentWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();
	
	//--Add Meta Data Sub Menu-----------------
	if(command > wed_AddMetaDataBegin && command < wed_AddMetaDataEnd)
	{
		return WED_CanAddMetaData(mDocument, command);
	}
	//------------------------------------------//

	switch(command) {
	case wed_RestorePanes:	return 1;
	case gui_Undo:		if (um->HasUndo())	{ ioName = um->GetUndoName();	return 1; }
						else				{								return 0; }
	case gui_Redo:		if (um->HasRedo())	{ ioName = um->GetRedoName();	return 1; }
						else				{								return 0; }
	case gui_Clear:		return	WED_CanClear(mDocument);
	case wed_Crop:		return	WED_CanCrop(mDocument);
#if AIRPORT_ROUTING
//	case wed_MakeRouting:
	case wed_Merge:		return WED_CanMerge(mDocument);
#endif
	case wed_Overlay:														return 1;
	case gui_Close:															return 1;
	case wed_Split:		return WED_CanSplit(mDocument);
	case wed_Align:		return WED_CanAlign(mDocument);
	case wed_Orthogonalize:	return WED_CanOrthogonalize(mDocument);
	case wed_RegularPoly:	return WED_CanMakeRegularPoly(mDocument);
	case wed_Reverse:	return WED_CanReverse(mDocument);
	case wed_Rotate:	return WED_CanRotate(mDocument);
	case gui_Duplicate:	return WED_CanDuplicate(mDocument);
	case wed_Group:		return WED_CanGroup(mDocument);
	case wed_Ungroup:	return WED_CanUngroup(mDocument);
	case wed_AddATCFreq:return WED_CanMakeNewATCFreq(mDocument);
#if AIRPORT_ROUTING
	case wed_AddATCFlow:return WED_CanMakeNewATCFlow(mDocument);
	case wed_AddATCRunwayUse: return WED_CanMakeNewATCRunwayUse(mDocument);
	case wed_AddATCTimeRule:return WED_CanMakeNewATCTimeRule(mDocument);
	case wed_AddATCWindRule:return WED_CanMakeNewATCWindRule(mDocument);
	case wed_UpgradeRamps:	return 1;

#endif
	case wed_CreateApt:	return WED_CanMakeNewAirport(mDocument);
	case wed_EditApt:	return WED_CanSetCurrentAirport(mDocument, ioName);
	case wed_UpdateMetadata:     return WED_CanUpdateMetadata(mDocument);
	case wed_MoveFirst:	return WED_CanReorder(mDocument,-1,1);
	case wed_MovePrev:	return WED_CanReorder(mDocument,-1,0);
	case wed_MoveNext:	return WED_CanReorder(mDocument, 1,0);
	case wed_MoveLast:	return WED_CanReorder(mDocument, 1,1);
	case wed_BreakApartSpecialAgps: return WED_CanBreakApartSpecialAgps(mDocument);
	case wed_ReplaceVehicleObj:  return WED_CanReplaceVehicleObj(mDocument);
	case gui_Save:		return mDocument->IsDirty();
	case gui_Revert:	return mDocument->IsDirty() && mDocument->IsOnDisk();

	case gui_SelectAll:		return WED_CanSelectAll(mDocument);
	case gui_SelectNone:	return WED_CanSelectNone(mDocument);
	case wed_SelectParent:	return WED_CanSelectParent(mDocument);
	case wed_SelectChild:	return WED_CanSelectChildren(mDocument);
	case wed_SelectVertex:	return WED_CanSelectVertices(mDocument);
	case wed_SelectPoly:	return WED_CanSelectPolygon(mDocument);
	case wed_SelectConnected:	return WED_CanSelectConnected(mDocument);

#if AIRPORT_ROUTING
	case wed_SelectZeroLength:
	case wed_SelectDoubles:
	case wed_SelectCrossing:	
	case wed_SelectLocalObjects:
	case wed_SelectLibraryObjects:
	case wed_SelectDefaultObjects:
	case wed_SelectThirdPartyObjects:
	case wed_SelectMissingObjects:	return 1;
#endif

	case wed_ExportApt:		return WED_CanExportApt(mDocument);
	case wed_ExportPack:	return WED_CanExportPack(mDocument);
#if HAS_GATEWAY	
	case wed_ExportToGateway:	return WED_CanExportToGateway(mDocument);
#endif	
	case wed_ImportApt:		return WED_CanImportApt(mDocument);
	case wed_ImportDSF:		return WED_CanImportApt(mDocument);
	case wed_ImportOrtho:	return 1;
#if HAS_GATEWAY
	case wed_ImportGateway:	return WED_CanImportFromGateway(mDocument);
#endif	
#if GATEWAY_IMPORT_FEATURES
	case wed_ImportGatewayExtract: return 1;
#endif	
	case wed_Validate:		return 1;

	case wed_UnitFeet:	ioCheck= gIsFeet;return 1;
	case wed_UnitMeters:ioCheck=!gIsFeet;return 1;

	case wed_Export900:	ioCheck = gExportTarget == wet_xplane_900;	return 1;
	case wed_Export1000:ioCheck = gExportTarget == wet_xplane_1000;	return 1;
	case wed_Export1021:ioCheck = gExportTarget == wet_xplane_1021;	return 1;
	case wed_Export1050:ioCheck = gExportTarget == wet_xplane_1050;	return 1;
	case wed_Export1100:ioCheck = gExportTarget == wet_xplane_1100;	return 1;
	
	case wed_ExportGateway:ioCheck = gExportTarget == wet_gateway;	return 1;
	
#if WITHNWLINK
	case wed_ToggleLiveView :
		{
			WED_Server * Serv = mDocument->GetServer();
			if (Serv) {ioCheck = Serv->IsStarted(); return 1;}
			ioCheck = false;
		}
		return 0;
#endif
	default: return mMapPane->Map_CanHandleCommand(command, ioName, ioCheck);
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
		mPropPane->ToPrefs(prefs,0);

		prefs->WriteIntPref("doc/use_feet",gIsFeet);
		prefs->WriteIntPref("doc/export_target",gExportTarget);		
		prefs->WriteIntPref("window/main_split",mMainSplitter->GetSplitPoint());
		prefs->WriteIntPref("window/main_split2",mMainSplitter2->GetSplitPoint());
		prefs->WriteIntPref("window/prop_split",mPropSplitter->GetSplitPoint());
		prefs->WriteIntPref("window/prev_split",mLibSplitter->GetSplitPoint());
		
		int xy[2];
		int zw[2];
		XWin::GetBounds(zw,zw+1);
		XWin::GetWindowLoc(xy,xy+1);
		
		prefs->WriteIntPref("window/x_loc",xy[0]);
		prefs->WriteIntPref("window/y_loc",xy[1]);
		prefs->WriteIntPref("window/width",zw[0]);
		prefs->WriteIntPref("window/height",zw[1]);
		XWin::SetFilePath(NULL,false);
	}
	if (inMsg == msg_DocLoaded)
	{
		IDocPrefs * prefs = reinterpret_cast<IDocPrefs *>(inParam);
		mMapPane->FromPrefs(prefs);
		mPropPane->FromPrefs(prefs,0);

		gIsFeet = prefs->ReadIntPref("doc/use_feet",gIsFeet);
		gExportTarget = (WED_Export_Target) mDocument->ReadIntPref("doc/export_target",gExportTarget);
		XWin::SetFilePath(NULL,mDocument->IsDirty());
	}
	if(inMsg == msg_ArchiveChanged)
		XWin::SetFilePath(NULL,mDocument->IsDirty());
	
}

bool	WED_DocumentWindow::Closed(void)
{
	mDocument->TryClose();
	return false;
}

