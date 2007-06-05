#include "WED_MapPane.h"
#include "WED_Archive.h"
#include "WED_Menus.h"
#include "GUI_ToolBar.h"
#include "XESConstants.h"
#include "WED_Map.h"
#include "WED_MapBkgnd.h"
#include "WED_MarqueeTool.h"
#include "WED_CreatePolygonTool.h"
#include "WED_CreatePointTool.h"
#include "WED_CreateLineTool.h"
#include "WED_StructureLayer.h"
#include "WED_ImageOverlayTool.h"
#include "WED_VertexTool.h"
#include "WED_TerraserverLayer.h"
#include "GUI_Table.h"
#include "GUI_TextTable.h"
#include "WED_ToolInfoAdapter.h"
#include "WED_UIMeasurements.h"

char	kToolKeys[] = { 
	'b', 'w', 'e', 'o',
	'a', 'f', 'g', 'l',
	'k', 't', 'h', 's',
	'r', 'p', 'm', 'v' 
};


WED_MapPane::WED_MapPane(GUI_Commander * cmdr, double map_bounds[4], IResolver * resolver, WED_Archive * archive)
{
	mMap = new WED_Map(resolver);

	// Visualizatoin layers
	mLayers.push_back(					new WED_MapBkgnd(mMap, mMap, resolver));
	mLayers.push_back(mStructureLayer = new WED_StructureLayer(mMap, mMap, resolver));
	mLayers.push_back(mTerraserver = 	new WED_TerraserverLayer(mMap, mMap, resolver));

	// TOOLS
	mTools.push_back(					new WED_CreatePolygonTool("Boundary",mMap, mMap, resolver, archive, create_Boundary));
	mTools.push_back(					new WED_CreatePointTool("Windsock", mMap, mMap, resolver, archive, create_Windsock));
	mTools.push_back(					new WED_CreatePointTool("Airport Beacon", mMap, mMap, resolver, archive, create_Beacon));
	mTools.push_back(					new WED_CreatePointTool("Tower Viewpoint", mMap, mMap, resolver, archive, create_TowerViewpoint));
	mTools.push_back(					new WED_CreatePointTool("Ramp Start", mMap, mMap, resolver, archive, create_RampStart));
	mTools.push_back(					new WED_CreatePointTool("Light Fixture", mMap, mMap, resolver, archive, create_Lights));
	mTools.push_back(					new WED_CreatePointTool("Sign", mMap, mMap, resolver, archive, create_Sign));
	mTools.push_back(					new WED_CreatePolygonTool("Taxilines",mMap, mMap, resolver, archive, create_Marks));
	mTools.push_back(					new WED_CreatePolygonTool("Hole",mMap, mMap, resolver, archive, create_Hole));
	mTools.push_back(					new WED_CreatePolygonTool("Taxiway",mMap, mMap, resolver, archive, create_Taxi));
	mTools.push_back(					new WED_CreatePointTool("Helipad", mMap, mMap, resolver, archive, create_Helipad));
	mTools.push_back(					new WED_CreateLineTool("Sealane", mMap, mMap, resolver, archive, create_Sealane));
	mTools.push_back(					new WED_CreateLineTool("Runway", mMap, mMap, resolver, archive, create_Runway));
	mTools.push_back(mImageOverlay = 	new WED_ImageOverlayTool("Overlay Picture",mMap, mMap, resolver));
	mTools.push_back(					new WED_MarqueeTool("Marquee",mMap, mMap, resolver));
	mTools.push_back(					new WED_VertexTool("Vertex",mMap, mMap, resolver, 1));

	mInfoAdapter = new WED_ToolInfoAdapter;
	mTextTable = new GUI_TextTable(cmdr,WED_UIMeasurement("table_indent_width"));
	mTable = new GUI_Table;

	mTable->SetGeometry(mInfoAdapter);
	mTable->SetContent(mTextTable);
	mTextTable->SetProvider(mInfoAdapter);
	mTable->SetParent(this);
	mTable->Show();
	mTable->SizeShowAll();
	mTextTable->SetParentTable(mTable);
	mTable->SetSticky(1,1,1,0);
	this->PackPane(mTable, gui_Pack_Bottom);
	mTextTable->AddListener(mTable);
	mInfoAdapter->AddListener(mTable);

	mToolbar = new GUI_ToolBar(1,16,"map_tools.png");
	mToolbar->SizeToBitmap();
	mToolbar->Show();
	mToolbar->SetParent(this);
	mToolbar->SetSticky(1,0,0,1);
	this->PackPane(mToolbar,gui_Pack_Left);
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
	

	GUI_ScrollerPane * map_scroller = new GUI_ScrollerPane(1,1);
	map_scroller->SetParent(this);
	map_scroller->Show();
	map_scroller->SetSticky(1,1,1,1);
	
	this->PackPane(map_scroller, gui_Pack_Center);
	
	mMap->SetParent(map_scroller);
	mMap->Show();
	map_scroller->PositionInContentArea(mMap);
	map_scroller->SetContent(mMap);
	
//	mMap->SetMapVisibleBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);
	mMap->SetMapLogicalBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);

	mMap->ZoomShowAll();

	for(vector<WED_MapLayer *>::iterator l = mLayers.begin(); l != mLayers.end(); ++l)
		mMap->AddLayer(*l);
	for(vector<WED_MapToolNew *>::iterator t = mTools.begin(); t != mTools.end(); ++t)
		mMap->AddLayer(*t);

	mMap->SetTool(mTools[0]);
	mInfoAdapter->SetTool(mTools[0]);
	mToolbar->SetValue(mTools.size()-1);
	
	// This is a bit of a hack.  The archive provides whole-doc "changed" messages at the minimum global times:
	// 1. On the commit of any operation.
	// 2. On the undo or redo of any operation.
	// So ... for lack of a better idea right now, we simply broker a connection between the source opf these
	// messages (secretly it's our document's GetArchive() member) and anyone who needs it (our map).
	
	archive->AddListener(mMap);
}

WED_MapPane::~WED_MapPane()
{
	for(vector<WED_MapLayer *>::iterator l = mLayers.begin(); l != mLayers.end(); ++l)
		delete *l;
	for(vector<WED_MapToolNew *>::iterator t = mTools.begin(); t != mTools.end(); ++t)
		delete *t;

	delete mTextTable;
	delete mInfoAdapter;
	
}



void	WED_MapPane::ZoomShowAll(void)
{
	double l,b,r,t;
	mMap->GetMapLogicalBounds(l,b,r,t);
	mMap->SetAspectRatio(1.0 / cos((b+t) * 0.5 * DEG_TO_RAD));
	mMap->ZoomShowAll();
}

int		WED_MapPane::Map_KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (mMap->KeyPress(inKey, inVK, inFlags)) return 1;
	for (int n = 0; n < sizeof(kToolKeys) / sizeof(kToolKeys[0]); ++n)
	if (kToolKeys[n])
	if (kToolKeys[n]==inKey)
	{
		mToolbar->SetValue(n);
		return 1;
	}
	return 0;
}

int		WED_MapPane::Map_HandleCommand(int command)
{
	switch(command) {
	case wed_PickOverlay:	mImageOverlay->PickFile();	return 1;
	case wed_ToggleOverlay:	if (mImageOverlay->CanShow()) { mImageOverlay->ToggleVisible(); return 1; }
	case wed_ToggleTerraserver:	mTerraserver->ToggleVis(); return 1;

	case wed_Pavement0:		mStructureLayer->SetPavementTransparency(0.0f);		return 1;
	case wed_Pavement25:	mStructureLayer->SetPavementTransparency(0.25f);	return 1;
	case wed_Pavement50:	mStructureLayer->SetPavementTransparency(0.5f);		return 1;
	case wed_Pavement75:	mStructureLayer->SetPavementTransparency(0.75f);	return 1;
	case wed_Pavement100:	mStructureLayer->SetPavementTransparency(1.0f);		return 1;
	case wed_ToggleLines:	mStructureLayer->SetRealLinesShowing(!mStructureLayer->GetRealLinesShowing());				return 1;

	default:		return 0;
	}	
}

int		WED_MapPane::Map_CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case wed_PickOverlay:	return 1;
	case wed_ToggleOverlay:	if (mImageOverlay->CanShow()) { ioCheck = mImageOverlay->IsVisible(); return 1; }
	case wed_ToggleTerraserver: ioCheck = mTerraserver->IsVis(); return 1;
	case wed_Pavement0:		ioCheck = mStructureLayer->GetPavementTransparency() == 0.0f;	return 1;
	case wed_Pavement25:	ioCheck = mStructureLayer->GetPavementTransparency() == 0.25f;	return 1;
	case wed_Pavement50:	ioCheck = mStructureLayer->GetPavementTransparency() == 0.5f;	return 1;
	case wed_Pavement75:	ioCheck = mStructureLayer->GetPavementTransparency() == 0.75f;	return 1;
	case wed_Pavement100:	ioCheck = mStructureLayer->GetPavementTransparency() == 1.0f;	return 1;
	case wed_ToggleLines:	ioCheck = mStructureLayer->GetRealLinesShowing();				return 1;
	default:		return 0;
	}	
}

void	WED_MapPane::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	int i = mToolbar->GetValue();
	WED_MapToolNew * t = NULL;
	if (i >= 0 && i < mTools.size())
		t = mTools[i];
	mMap->SetTool(t);		
	mInfoAdapter->SetTool(t);
}