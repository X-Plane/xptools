#include "WED_CreatePolygonTool.h"


WED_CreatePolygonTool::WED_CreatePolygonTool(
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer, 
									IResolver *			resolver) :
	WED_CreateToolBase(host, zoomer, resolver,
	1,				// min pts
	1,				// max pts
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
}
