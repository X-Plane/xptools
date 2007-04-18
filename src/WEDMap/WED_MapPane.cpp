#include "WED_MapPane.h"
#include "WED_Archive.h"
#include "WED_Menus.h"

WED_MapPane::WED_MapPane(double map_bounds[4], IResolver * resolver, WED_Archive * archive) :
	mMap(resolver, "selection","world"),
	mBkgnd(&mMap, &mMap, resolver),
	mStructure(&mMap, &mMap, resolver),
	mMarquee(&mMap, &mMap, resolver, "world", "selection"),
	mImageOverlay(&mMap, &mMap, resolver, "world", "selection"),
	mTerraserver(&mMap, &mMap, resolver),
	mCreatePoly(&mMap, &mMap, resolver, archive)
{
	GUI_ScrollerPane * map_scroller = new GUI_ScrollerPane(1,1);
	map_scroller->SetParent(this);
	map_scroller->Show();
	map_scroller->SetSticky(1,1,1,1);
	
	this->PackPane(map_scroller, gui_Pack_Center);
	
	mMap.SetParent(map_scroller);
	mMap.Show();
	map_scroller->PositionInContentArea(&mMap);
	map_scroller->SetContent(&mMap);
	
//	mMap.SetMapVisibleBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);
	mMap.SetMapLogicalBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);

	mMap.ZoomShowAll();

	mMap.AddLayer(&mBkgnd);
	mMap.AddLayer(&mStructure);
	mMap.AddLayer(&mTerraserver);
	mMap.AddLayer(&mImageOverlay);
	mMap.AddLayer(&mCreatePoly);
	mMap.SetTool(&mCreatePoly);
	
	// This is a bit of a hack.  The archive provides whole-doc "changed" messages at the minimum global times:
	// 1. On the commit of any operation.
	// 2. On the undo or redo of any operation.
	// So ... for lack of a better idea right now, we simply broker a connection between the source opf these
	// messages (secretly it's our document's GetArchive() member) and anyone who needs it (our map).
	
	archive->AddListener(&mMap);
}

WED_MapPane::~WED_MapPane()
{
}



void	WED_MapPane::ZoomShowAll(void)
{
	mMap.ZoomShowAll();
}

int		WED_MapPane::Map_KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	return 0;
}

int		WED_MapPane::Map_HandleCommand(int command)
{
	switch(command) {
	case wed_PickOverlay:	mImageOverlay.PickFile();	return 1;
	case wed_ToggleOverlay:	if (mImageOverlay.CanShow()) { mImageOverlay.ToggleVisible(); return 1; }
	default:		return 0;
	}	
}

int		WED_MapPane::Map_CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	switch(command) {
	case wed_PickOverlay:	return 1;
	case wed_ToggleOverlay:	if (mImageOverlay.CanShow()) { ioCheck = mImageOverlay.IsVisible(); return 1; }
	default:		return 0;
	}	
}

