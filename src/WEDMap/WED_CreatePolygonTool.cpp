#include "WED_CreatePolygonTool.h"
#include "WED_AirportChain.h"
#include "WED_Taxiway.h"
#include "IResolver.h"
#include "WED_AirportNode.h"

WED_CreatePolygonTool::WED_CreatePolygonTool(
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer, 
									IResolver *			resolver,
									WED_Archive *		archive) :
	WED_CreateToolBase(host, zoomer, resolver,archive,
	3,				// min pts
	99999999,		// max pts
	1,				// curve allowed
	0,				// curve required?
	1,				// close allowed
	1)				// close required?
{
}	
									
WED_CreatePolygonTool::~WED_CreatePolygonTool()
{
}

void	WED_CreatePolygonTool::AcceptPath(
							const vector<Point2>&	pts,
							const vector<int>		has_dirs,
							const vector<Point2>&	dirs,
							int						closed)
{
		char buf[256];

	GetArchive()->StartCommand("Create Taxiway");

	WED_AirportChain *	outer_ring = WED_AirportChain::CreateTyped(GetArchive());
	WED_Taxiway *		tway = WED_Taxiway::CreateTyped(GetArchive());
	
	outer_ring->SetParent(tway,0);
	
	IUnknown * host = GetResolver()->Resolver_Find("world");
	
	tway->SetParent(SAFE_CAST(WED_Thing,host),0);
	
	outer_ring->SetClosed(closed);
	
	for(int n = 0; n < pts.size(); ++n)
	{
		WED_AirportNode * node = WED_AirportNode::CreateTyped(GetArchive());
		node->SetLocation(pts[n]);
		node->SetControlHandleHi(dirs[n]);
		node->SetParent(outer_ring, n);
	
		sprintf(buf,"Node %d",n+1);
		node->SetName(buf);
	}

	static int n = 0;
	++n;
	sprintf(buf,"New Taxiway %d",n);
	tway->SetName(buf);
	sprintf(buf,"Taxiway %d Outer Ring",n);
	outer_ring->SetName(buf);

	GetArchive()->CommitCommand();

}
