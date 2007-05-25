#include "WED_AirportSign.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"

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

void		WED_AirportSign::Import(const AptSign_t& x)
{
	SetLocation(x.location);
	SetHeading(x.heading);
	style = ENUM_Import(Sign_Style, x.style_code);
	height = ENUM_Import(Sign_Size, x.size_code);
	SetName(x.text);
}

void		WED_AirportSign::Export(		 AptSign_t& x) const
{
	GetLocation(x.location);
	x.heading = GetHeading();
	x.style_code = ENUM_Export(style.value);
	x.size_code = ENUM_Export(height.value);
	GetName(x.text);
}
