#include "WED_CreatePointTool.h"
#include "WED_AirportBeacon.h"
#include "WED_ToolUtils.h"
#include "PlatformUtils.h"
#include "WED_MapZoomerNew.h"

WED_CreatePointTool::WED_CreatePointTool(WED_MapZoomerNew * zoomer, IResolver * resolver) : WED_MapToolNew(zoomer, resolver)
{
	clicking = false;
}

WED_CreatePointTool::~WED_CreatePointTool()
{
}

void		WED_CreatePointTool::DrawSelected			(int inCurrent, GUI_GraphState * g)
{
}

int			WED_CreatePointTool::HandleClickDown(int inX, int inY, int inButton)
{
	WED_Thing * cur_airport = WED_GetCurrentAirport(GetResolver());
	if (cur_airport == NULL)
	{
		DoUserAlert("You cannot add an airport beacon until you create an airport.");
		return 1;
	}
	
	WED_Thing * add_to = WED_FindParent(
							WED_GetSelect(GetResolver()),
							cur_airport,
							cur_airport);
							
	add_to->StartCommand("Create Airport Beacon");
							
	WED_AirportBeacon * new_beacon = WED_AirportBeacon::CreateTyped(add_to->GetArchive(),add_to->GetArchive()->NewID());

	new_beacon->SetLocation(Point2(
		GetZoomer()->XPixelToLon(inX),
		GetZoomer()->YPixelToLat(inY)));

	new_beacon->SetParent(add_to, add_to->CountChildren());
	
	new_beacon->SetName("New Airport Beacon");
	
	add_to->CommitCommand();

}

void		WED_CreatePointTool::HandleClickDrag(int inX, int inY, int inButton)
{
}

void		WED_CreatePointTool::HandleClickUp  (int inX, int inY, int inButton)
{
}
							
int			WED_CreatePointTool::GetNumProperties(void)
{
	return 0;
}

void		WED_CreatePointTool::GetNthPropertyName(int, string&)
{
}

double		WED_CreatePointTool::GetNthPropertyValue(int)
{
	return 0.0;
}

void		WED_CreatePointTool::SetNthPropertyValue(int, double)
{
}
	
int			WED_CreatePointTool::GetNumButtons(void)
{
	return 0;
}

void		WED_CreatePointTool::GetNthButtonName(int, string&)
{
}

void		WED_CreatePointTool::NthButtonPressed(int)
{
}
	
char *		WED_CreatePointTool::GetStatusText(void)
{
	return NULL;
}
