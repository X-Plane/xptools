#include "WED_CreatePolygonTool.h"
#include "WED_AirportChain.h"
#include "WED_Taxiway.h"
#include "IResolver.h"
#include "ISelection.h"
#include "WED_AirportNode.h"
#include "WED_ToolUtils.h"
#include "WED_EnumSystem.h"
#include "WED_AirportBoundary.h"
#include "WED_Airport.h"

const char * kCreateCmds[] = { "Taxiway", "Boundary", "Marking", "Hole" };



WED_CreatePolygonTool::WED_CreatePolygonTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer, 
									IResolver *			resolver,
									WED_Archive *		archive,
									CreateTool_t		tool) :
	WED_CreateToolBase(tool_name,host, zoomer, resolver,archive,
	tool == create_Marks ? 2 : 3,// min pts
	99999999,		// max pts
	1,				// curve allowed
	0,				// curve required?
	1,				// close allowed
	tool != create_Marks),		// close required?
	mType(tool),
		mPavement(tool == create_Taxi ? this : NULL,"Pavement","","",Surface_Type,surf_Concrete),
		mRoughness(tool == create_Taxi ? this : NULL,"Roughness","","",0.25),
		mHeading(tool == create_Taxi ? this : NULL,"Heading","","",0),
		mMarkings(this,"Markings", "", "", LinearFeature)
{
	mPavement.value = surf_Concrete;
}	
									
WED_CreatePolygonTool::~WED_CreatePolygonTool()
{
}

void	WED_CreatePolygonTool::AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed)
{
		char buf[256];

	WED_Thing * host = GetHost();
	if (host == NULL) return;

	switch(mType) {
	case create_Taxi:		GetArchive()->StartCommand("Create Taxiway");	break;
	case create_Boundary:	GetArchive()->StartCommand("Create Airport Boundary");	break;
	case create_Marks:		GetArchive()->StartCommand("Create Markings");	break;
	case create_Hole:		GetArchive()->StartCommand("Create Hole");	break;
	}

	WED_AirportChain *	outer_ring = WED_AirportChain::CreateTyped(GetArchive());
	
	
	static int n = 0;
	++n;
	
	bool is_ccw = (mType == create_Marks) ? true : is_ccw_polygon_pt(pts.begin(), pts.end());
	if (mType == create_Hole) is_ccw = !is_ccw;
	
	switch(mType) {
	case create_Taxi:
		{
			WED_Taxiway *		tway = WED_Taxiway::CreateTyped(GetArchive());	
			outer_ring->SetParent(tway,0);	
			tway->SetParent(host, host->CountChildren());
			
			sprintf(buf,"New Taxiway %d",n);
			tway->SetName(buf);
			sprintf(buf,"Taxiway %d Outer Ring",n);
			outer_ring->SetName(buf);
			
			tway->SetRoughness(mRoughness.value);
			tway->SetHeading(mHeading.value);
			tway->SetSurface(mPavement.value);
			
		}
		break;
	case create_Boundary:
		{
			WED_AirportBoundary *		bwy = WED_AirportBoundary::CreateTyped(GetArchive());	
			outer_ring->SetParent(bwy,0);	
			bwy->SetParent(host, host->CountChildren());
			
			sprintf(buf,"Airport Boundary %d",n);
			bwy->SetName(buf);
			sprintf(buf,"Airport Boundary %d Outer Ring",n);
			outer_ring->SetName(buf);
			
		}
		break;
	case create_Marks:
	case create_Hole:
		{
			outer_ring->SetParent(host, host->CountChildren());
			sprintf(buf,"Linear Feature %d",n);
			outer_ring->SetName(buf);
		}
		break;
	}
	
	outer_ring->SetClosed(closed);
	
	for(int n = 0; n < pts.size(); ++n)
	{
		int idx = is_ccw ? n : pts.size()-n-1;
		WED_AirportNode * node = WED_AirportNode::CreateTyped(GetArchive());
		node->SetLocation(pts[idx]);
		if (!has_dirs[idx])
		{
			node->DeleteHandleHi();
			node->DeleteHandleLo();
		}
		else
		{
			node->SetSplit(has_split[idx]);
			if (is_ccw) 
			{
				node->SetControlHandleHi(dirs_hi[idx]);
				node->SetControlHandleLo(dirs_lo[idx]);
			} else {
				node->SetControlHandleHi(dirs_lo[idx]);
				node->SetControlHandleLo(dirs_hi[idx]);
			}
		}
		node->SetParent(outer_ring, n);
		node->SetAttributes(mMarkings.value);
		sprintf(buf,"Node %d",n+1);
		node->SetName(buf);
	}


	GetArchive()->CommitCommand();

}

const char *	WED_CreatePolygonTool::GetStatusText(void)
{
	static char buf[256];
	if (GetHost() == NULL)
	{
		if (mType == create_Hole)
			sprintf(buf,"You must selet a polygon before you can insert a hole into it.");
		else
			sprintf(buf,"You must create an airport before you can add a %s.",kCreateCmds[mType]);
		return buf;
	}
	return NULL;
}

bool		WED_CreatePolygonTool::CanCreateNow(void)
{
	return GetHost() != NULL;
}

WED_Thing *		WED_CreatePolygonTool::GetHost()
{
	if (mType == create_Hole)
	{
		ISelection * sel = WED_GetSelect(GetResolver());		
		if (sel->GetSelectionCount() != 1) return NULL;
		return dynamic_cast<WED_GISPolygon *>(sel->GetNthSelection(0));
	} else
		return WED_GetCurrentAirport(GetResolver());
}
