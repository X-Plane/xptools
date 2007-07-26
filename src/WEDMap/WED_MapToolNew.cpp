#include "WED_MapToolNew.h"


WED_MapToolNew::WED_MapToolNew(const char * tname, GUI_Pane * h, WED_MapZoomerNew * z, IResolver * i) : WED_MapLayer(h, z, i), tool_name(tname)
{
	has_anchor1 = false;
	has_anchor2 = false;
	has_distance = false;
	has_heading = false;
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

bool				WED_MapToolNew::GetAnchor1(Point2& a)	{ if (has_anchor1 ) a = anchor1 ; return has_anchor1 ; }
bool				WED_MapToolNew::GetAnchor2(Point2& a)	{ if (has_anchor2 ) a = anchor2 ; return has_anchor2 ; }
bool				WED_MapToolNew::GetDistance(double& d)	{ if (has_distance) d = distance; return has_distance; }
bool				WED_MapToolNew::GetHeading(double& h)	{ if (has_heading ) h = heading ; return has_heading ; }

void				WED_MapToolNew::SetAnchor1(const Point2& a)	{ has_anchor1  = true; anchor1  = a; }
void				WED_MapToolNew::SetAnchor2(const Point2& a)	{ has_anchor2  = true; anchor2  = a; }
void				WED_MapToolNew::SetDistance(double d)		{ has_distance = true; distance = d; }
void				WED_MapToolNew::SetHeading(double h)		{ has_heading  = true; heading  = h; }

void				WED_MapToolNew::ClearAnchor1(void)	{ has_anchor1  = false; }
void				WED_MapToolNew::ClearAnchor2(void)	{ has_anchor2  = false; }
void				WED_MapToolNew::ClearDistance(void)	{ has_distance = false; }
void				WED_MapToolNew::ClearHeading(void)	{ has_heading  = false; }
