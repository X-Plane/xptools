#include "WED_Helipad.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_Helipad)

WED_Helipad::WED_Helipad(WED_Archive * a, int i) : WED_GISPoint_HeadingWidthLength(a,i),

	surface(this,"Surface","WED_helipad","surface",Surface_Type,surf_Concrete),
	markings(this,"Markings","WED_helipad","markings",Helipad_Markings,heli_Mark_Default),
	shoulder(this,"Shoulder","WED_helipad","shoulder",Shoulder_Type,shoulder_None),
	roughness(this,"Roughness","WED_helipad","roughness",0.25),
	edgelights(this,"Lights","WED_helipad","lights",Heli_Lights,heli_Yellow)
{
}

WED_Helipad::~WED_Helipad()
{
}

void		WED_Helipad::SetSurface(int x)
{
	surface = x;
}

void		WED_Helipad::SetMarkings(int x)
{
	markings = x;
}

void		WED_Helipad::SetShoulder(int x)
{
	shoulder = x;
}

void		WED_Helipad::SetRoughness(double x)
{
	roughness = x;
}

void		WED_Helipad::SetEdgeLights(int x)
{
	edgelights = x;
}

int			WED_Helipad::GetSurface(void) const
{
	return surface.value;
}

