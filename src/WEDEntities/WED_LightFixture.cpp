#include "WED_LightFixture.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_LightFixture)

WED_LightFixture::WED_LightFixture(WED_Archive * a, int i) : WED_GISPoint_Heading(a, i),
	light_type(this, "Type", "WED_lightfixture", "kind", Light_Fixt, light_VASI),
	angle(this,"Angle","WED_lightfixture","angle",3.0)
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
