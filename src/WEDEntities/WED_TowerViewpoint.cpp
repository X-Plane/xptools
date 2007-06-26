#include "WED_TowerViewpoint.h"
#include "AptDefs.h"
#include "XESConstants.h"

DEFINE_PERSISTENT(WED_TowerViewpoint)

WED_TowerViewpoint::WED_TowerViewpoint(WED_Archive * a, int i) : WED_GISPoint(a,i),
	height(this,"Height","WED_towerviewpoint","height",0,3,0)
{
}

WED_TowerViewpoint::~WED_TowerViewpoint()
{
}

void	WED_TowerViewpoint::SetHeight(double h)
{
	height = h;
}

void		WED_TowerViewpoint::Import(const AptTowerPt_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(x.location);
	height = x.height_ft * FT_TO_MTR;
	SetName(x.name);
}

void		WED_TowerViewpoint::Export(		 AptTowerPt_t& x) const
{
	GetLocation(x.location);
	x.height_ft = height * MTR_TO_FT;
	GetName(x.name);
	x.draw_obj = 0;
}
