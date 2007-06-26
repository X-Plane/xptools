#include "WED_AirportSign.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_Errors.h"

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

void		WED_AirportSign::Import(const AptSign_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(x.location);
	SetHeading(x.heading);
	style = ENUM_Import(Sign_Style, x.style_code);
	if (style == -1)
	{
		print_func(ref,"Error importing airport sign: sign style code %d is illegal (not a member of type %s).\n", x.style_code, DOMAIN_Fetch(style.domain));
		style = style_Default;
	}	
	height = ENUM_Import(Sign_Size, x.size_code);
	if (height == -1)
	{
		print_func(ref,"Error importing airport sign: sign height code %d is illegal (not a member of type %s).\n", x.size_code, DOMAIN_Fetch(height.domain));
		height = size_MediumTaxi;
	}
	
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
