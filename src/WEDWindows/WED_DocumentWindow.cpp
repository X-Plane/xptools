#include "WED_Document.h"
#include "WED_Progress.h"
#include "XESIO.h"
#include "AptIO.h"
#include "MapAlgs.h"
#include "WED_Messages.h"
#include "GUI_Menus.h"
#include "WED_UndoMgr.h"
#include "WED_DocumentWindow.h"
#include "WED_MapPane.h"
#include "WED_PropertyPane.h"
#include "GUI_TabPane.h"
#include "WED_Thing.h"
#include "WED_Menus.h"
#include "WED_Select.h"
#include "GUI_Splitter.h"
#include "WED_GroupCommands.h"


WED_DocumentWindow::WED_DocumentWindow(
	 		const char * 	inTitle, 
	 		int 			inBounds[4],
	 		GUI_Commander * inCommander,
	 		WED_Document *	inDocument) :
	GUI_Window(inTitle, inBounds, inCommander),
	mDocument(inDocument)
{
	
	GUI_Window::SetDescriptor(mDocument->GetFilePath());
	mDocument->AddListener(this);
					
//	WED_Thing * root = SAFE_CAST(WED_Thing,mDocument->GetArchive()->Fetch(1));
//	WED_Select * s = SAFE_CAST(WED_Select,root->GetNamedChild("selection"));
//	DebugAssert(root);
//	DebugAssert(s);
	
	/****************************************************************************************************************************************************************
	 * MAP VIEW
	****************************************************************************************************************************************************************/

	int		splitter_b[4];	
	GUI_Splitter * main_splitter = new GUI_Splitter(gui_Split_Horizontal);
	main_splitter->SetParent(this);
	main_splitter->Show();
	GUI_Pane::GetBounds(splitter_b);
	main_splitter->SetBounds(splitter_b);
	main_splitter->SetSticky(1,1,1,1);
		
	double	lb[4];
	mDocument->GetBounds(lb);
	mMapPane = new WED_MapPane(this, lb, inDocument,inDocument->GetArchive());
	mMapPane->SetParent(main_splitter);
	mMapPane->Show();
	mMapPane->SetSticky(1,1,0,1);

	/****************************************************************************************************************************************************************
	 * PROPERTY-SIDE
	****************************************************************************************************************************************************************/

	// --------------- Splitter and tabs ---------------
	
	GUI_Splitter * prop_splitter = new GUI_Splitter(gui_Split_Vertical);
	prop_splitter->SetParent(main_splitter);
	prop_splitter->Show();
	GUI_Pane::GetBounds(splitter_b);
	prop_splitter->SetBounds(splitter_b);
	prop_splitter->SetSticky(1,1,1,1);

	GUI_TabPane * prop_tabs = new GUI_TabPane(this);
	prop_tabs->SetParent(prop_splitter);
	prop_tabs->Show();
	prop_tabs->SetSticky(1,1,1,0);

	// --------------- Selection ---------------


	static const char * sel_t[] = { "Name", "Type", NULL };
	static		 int	sel_w[] = { 100, 100 };
	
	WED_PropertyPane * prop_pane1 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sel_t, sel_w,inDocument->GetArchive(), propPane_Selection, 0);	
	prop_tabs->AddPane(prop_pane1, "Selection");

	// --------------- AIRPORT

	static const char * air_t[] = { "Name", "Type", "Field Elevation", "Has ATC", "ICAO Identifier", NULL };
	static		 int	air_w[] = { 100, 100, 100, 50, 100  };
	static const char * air_f[] = { "WED_Airport", NULL };
	
	WED_PropertyPane * prop_pane2 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, air_t, air_w,inDocument->GetArchive(), propPane_Filtered, air_f);	
	prop_tabs->AddPane(prop_pane2, "Airports");
	
	// --------------- LIGHTS, SIGNS, BEACONS ---------------

	static const char * sin_t[] = { "Name", "Type", "Size", "Angle", 0 };
	static		 int	sin_w[] = { 100, 100, 100, 100  };
	static const char * sin_f[] = { "WED_Airport", "WED_LightFixture", "WED_AirportBeacon", "WED_AirportSign", "WED_Group", NULL };
	
	WED_PropertyPane * prop_pane3 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, sin_t, sin_w,inDocument->GetArchive(), propPane_Filtered, sin_f);	
	prop_tabs->AddPane(prop_pane3, "Signs");

	// --------------- RUNWAYS ---------------

	static const char * rwy_t[] = { "REIL 2", "TDZ Lights 2", "Approach Lights 2", "Markings 2", "Blastpad 2", "Displaced Threshhold 2", "Identifier 2",
									"REIL 1", "TDZ Lights 1", "Approach Lights 1", "Markings 1", "Blastpad 1", "Displaced Threshhold 1", "Identifier 1",
									"Distance Signs", "Edge Lights", "Centerline Lights", "Roughness", "Shoulder", "Surface", "Name", 0 };
	static		 int	rwy_w[] = { 100, 100, 100, 100, 100, 100, 100, 
									100, 100, 100, 100, 100, 100, 100, 
									100, 100, 100, 100, 100, 100, 100 };
	static const char * rwy_f[] = { "WED_Airport", "WED_Runway", NULL };
	
	WED_PropertyPane * prop_pane4 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, rwy_t, rwy_w,inDocument->GetArchive(), propPane_FilteredVertical, rwy_f);	
	prop_tabs->AddPane(prop_pane4, "Runways");

	// --------------- TAXIWAYS ---------------

	static const char * tax_t[] = { "Name", "Surface", "Roughness", "Texture Heading", 0 };
	static		 int	tax_w[] = { 100, 150, 100, 100  };
	static const char * tax_f[] = { "WED_Airport", "WED_Taxiway", "WED_Group", NULL };
	
	WED_PropertyPane * prop_pane5 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, tax_t, tax_w,inDocument->GetArchive(), propPane_Filtered, tax_f);	
	prop_tabs->AddPane(prop_pane5, "Taxiways");

	// --------------- HELIPADS ---------------

	static const char * hel_t[] = { "Name", "Surface", "Markings", "Shoulder", "Roughness", "Lights", 0 };
	static		 int	hel_w[] = { 100, 150, 150, 150, 100, 150 };
	static const char * hel_f[] = { "WED_Airport", "WED_Helipad", NULL };
	
	WED_PropertyPane * prop_pane6 = new WED_PropertyPane(prop_tabs->GetPaneOwner(), inDocument, hel_t, hel_w,inDocument->GetArchive(), propPane_Filtered, hel_f);	
	prop_tabs->AddPane(prop_pane6, "Helipads");

	// --------------- Hierarchy  View ---------------

	static const char * titles[] =  { "Name", "Locked", "Hidden", 0 };
	static int widths[] =			{ 100,		50,		50		};

	WED_PropertyPane * prop_pane = new WED_PropertyPane(this, inDocument, titles, widths,inDocument->GetArchive(), propPane_Hierarchy, 0);	
	prop_pane->SetParent(prop_splitter);
	prop_pane->Show();
	prop_pane->SetSticky(1,1,1,1);

	/****************************************************************************************************************************************************************
	 * FINAL CLEANUP
	****************************************************************************************************************************************************************/

	main_splitter->AlignContentsAt((inBounds[2]-inBounds[0]>inBounds[3]-inBounds[1]) ? (inBounds[3]-inBounds[1]) : ((inBounds[2]-inBounds[0])/2));
	prop_splitter->AlignContentsAt(300);	
	mMapPane->ZoomShowAll();
}

WED_DocumentWindow::~WED_DocumentWindow()
{
}

int	WED_DocumentWindow::KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{	
	return mMapPane->Map_KeyPress(inKey, inVK, inFlags);
}

int	WED_DocumentWindow::HandleCommand(int command)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();
	switch(command) {
	case gui_Undo:	if (um->HasUndo()) { um->Undo(); return 1; }	break;
	case gui_Redo:	if (um->HasRedo()) { um->Redo(); return 1; }	break;
	case wed_Group:		WED_DoGroup(mDocument); return 1;
	case wed_Ungroup:	WED_DoUngroup(mDocument); return 1;
	case wed_MoveFirst:	WED_DoReorder(mDocument,-1,1);	return 1;
	case wed_MovePrev:	WED_DoReorder(mDocument,-1,0);	return 1;
	case wed_MoveNext:	WED_DoReorder(mDocument, 1,0);	return 1;
	case wed_MoveLast:	WED_DoReorder(mDocument, 1,1);	return 1;
	
	case wed_CreateApt:	WED_DoMakeNewAirport(mDocument); return 1;
	case wed_EditApt:	WED_DoSetCurrentAirport(mDocument); return 1;
	case gui_Close:	return 1;
	default: return mMapPane->Map_HandleCommand(command);	break;
	}
	return 0;
}

int	WED_DocumentWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();
	switch(command) {
	case gui_Undo:		if (um->HasUndo())	{ ioName = um->GetUndoName();	return 1; }
						else				{								return 0; }
	case gui_Redo:		if (um->HasRedo())	{ ioName = um->GetRedoName();	return 1; }
						else				{								return 0; }
	case gui_Close:															return 1;
	case wed_Group:		return WED_CanGroup(mDocument);
	case wed_Ungroup:	return WED_CanUngroup(mDocument);
	case wed_CreateApt:	return WED_CanMakeNewAirport(mDocument); 
	case wed_EditApt:	return WED_CanSetCurrentAirport(mDocument, ioName);
	case wed_MoveFirst:	return WED_CanReorder(mDocument,-1,1);	
	case wed_MovePrev:	return WED_CanReorder(mDocument,-1,0);	
	case wed_MoveNext:	return WED_CanReorder(mDocument, 1,0);	
	case wed_MoveLast:	return WED_CanReorder(mDocument, 1,1);	

	default:																return mMapPane->Map_CanHandleCommand(command, ioName, ioCheck);
	}
}

void	WED_DocumentWindow::ReceiveMessage(
				GUI_Broadcaster *		inSrc,
				int						inMsg,
				int						inParam)
{
	if(inMsg == msg_DocumentDestroyed)
		delete this;
}

bool	WED_DocumentWindow::Closed(void)
{
	mDocument->AsyncDestroy();
	return false;
}

