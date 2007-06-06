#include "WED_Helipad.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_Helipad)

WED_Helipad::WED_Helipad(WED_Archive * a, int i) : WED_GISPoint_HeadingWidthLength(a,i),

	surface(this,"Surface","WED_helipad","surface",Surface_Type,surf_Concrete),
	markings(this,"Markings","WED_helipad","markings",Helipad_Markings,heli_Mark_Default),
	shoulder(this,"Shoulder","WED_helipad","shoulder",Shoulder_Type,shoulder_None),
	roughness(this,"Roughness","WED_helipad","roughness",0.25,4,2),
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

void	WED_Helipad::Import(const AptHelipad_t& x)
{
	SetName(x.id);
	SetLocation(x.location);
	SetHeading(x.heading);
	SetWidth(x.width_mtr);
	SetLength(x.length_mtr);
	surface = ENUM_Import(Surface_Type, x.surface_code);
	markings = ENUM_Import(Helipad_Markings, x.marking_code);
	shoulder = ENUM_Import(Shoulder_Type, x.shoulder_code);
	roughness = x.roughness_ratio;
	edgelights = ENUM_Import(Heli_Lights, x.edge_light_code);
}


void	WED_Helipad::Export(		 AptHelipad_t& x) const
{
	GetName(x.id);
	GetLocation(x.location);
	x.heading = GetHeading();
	x.width_mtr = GetWidth();
	x.length_mtr = GetLength();
	x.surface_code = ENUM_Export(surface.value);
	x.marking_code = ENUM_Export(markings.value);
	x.shoulder_code = ENUM_Export(shoulder.value);
	x.roughness_ratio = roughness;
	x.edge_light_code = ENUM_Export(edgelights.value);

}

