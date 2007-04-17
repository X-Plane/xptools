#include "WED_MapPane.h"

WED_MapPane::WED_MapPane(double map_bounds[4], IResolver * resolver, GUI_Broadcaster * archive_broadcaster) :
	mBkgnd(&mMap, resolver),
	mMarquee(&mMap, &mMap, resolver, "world", "selection"),
	mCreatePoly(&mMap, &mMap, resolver)
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
	mMap.AddLayer(&mCreatePoly);
	mMap.SetTool(&mCreatePoly);
	
	// This is a bit of a hack.  The archive provides whole-doc "changed" messages at the minimum global times:
	// 1. On the commit of any operation.
	// 2. On the undo or redo of any operation.
	// So ... for lack of a better idea right now, we simply broker a connection between the source opf these
	// messages (secretly it's our document's GetArchive() member) and anyone who needs it (our map).
	
	archive_broadcaster->AddListener(&mMap);
}

WED_MapPane::~WED_MapPane()
{
}



void	WED_MapPane::ZoomShowAll(void)
{
	mMap.ZoomShowAll();
}
