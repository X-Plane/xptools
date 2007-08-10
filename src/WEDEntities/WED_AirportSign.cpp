/* 
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#include "WED_AirportSign.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_AirportSign)
TRIVIAL_COPY(WED_AirportSign, WED_GISPoint_Heading)

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
	SetHeading(x.heading <= 180.0 ? x.heading + 180.0 : x.heading - 180.0);
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
	x.heading += 180.0;
	if (x.heading > 360.0) x.heading -= 360.0;
	x.style_code = ENUM_Export(style.value);
	x.size_code = ENUM_Export(height.value);
	GetName(x.text);
}
