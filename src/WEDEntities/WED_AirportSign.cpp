#include "WED_AirportSign.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_AirportSign)

WED_AirportSign::WED_AirportSign(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i),
	style(this,"Type","WED_airportsign","style",Sign_Style,style_Default),
	height(this,"Size","WED_airportsign","size",Sign_Size,size_MediumTaxi)
{
}
	
WED_AirportSign::~WED_AirportSign()
{
}

void	WED_AirportSign::SetStyle(int s)
{
	style = s;
}

void	WED_AirportSign::SetHeight(int h)
{
	height = h;
}
