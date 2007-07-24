#include "WED_MapToolNew.h"


WED_MapToolNew::WED_MapToolNew(const char * tname, GUI_Pane * h, WED_MapZoomerNew * z, IResolver * i) : WED_MapLayer(h, z, i), tool_name(tname)
{
}

WED_MapToolNew::~WED_MapToolNew()
{
}

void		WED_MapToolNew::PropEditCallback(int before)
{
}

int					WED_MapToolNew::CountSubs(void) { return 0; }
IPropertyObject *	WED_MapToolNew::GetNthSub(int n) { return NULL; }

const char *	WED_MapToolNew::GetToolName(void) const
{
	return tool_name.c_str();
}
