#include "WED_Helipad.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_Helipad)
START_CASTING(WED_Helipad)
INHERITS_FROM(WED_GISPoint_HeadingWidthLength)
END_CASTING

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
