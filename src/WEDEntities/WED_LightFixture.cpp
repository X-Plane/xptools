#include "WED_LightFixture.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_LightFixture)
TRIVIAL_COPY(WED_LightFixture, WED_GISPoint_Heading)


WED_LightFixture::WED_LightFixture(WED_Archive * a, int i) : WED_GISPoint_Heading(a, i),
	light_type(this, "Type", "WED_lightfixture", "kind", Light_Fixt, light_VASI),
	angle(this,"Angle","WED_lightfixture","angle",3.0,4,2)
{
}

WED_LightFixture::~WED_LightFixture()
{
}

void	WED_LightFixture::SetLightType(int x)
{
	light_type = x;
}

void	WED_LightFixture::SetAngle(double x)
{
	angle = x;
}

void	WED_LightFixture::Import(const AptLight_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(x.location);
	SetHeading(x.heading);
	angle = x.angle;
	light_type = ENUM_Import(Light_Fixt, x.light_code);
	if (light_type == -1)
	{
		print_func(ref,"Error importing light fixture: light type code %d is illegal (not a member of type %s).\n", x.light_code, DOMAIN_Fetch(light_type.domain));
		light_type = light_VASI;
	}	
	
	SetName(x.name);
}

void	WED_LightFixture::Export(		 AptLight_t& x) const
{
	GetLocation(x.location);
	x.heading = GetHeading();
	x.angle = angle;
	x.light_code = ENUM_Export(light_type.value);
	GetName(x.name);
}
