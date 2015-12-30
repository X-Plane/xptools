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

#include "WED_RampPosition.h"
#include "AptDefs.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_RampPosition)
TRIVIAL_COPY(WED_RampPosition, WED_GISPoint_Heading)

WED_RampPosition::WED_RampPosition(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i),
	ramp_type	(this, "Ramp Start Type",	SQL_Name("",""),	XML_Name("ramp_start","type"   ), ATCRampType, atc_Ramp_Misc),
	equip_type  (this, "Equipment Type",		SQL_Name("",""),	XML_Name("ramp_start","traffic"), ATCTrafficType, 0),
	width		(this, "Size",				SQL_Name("",""), XML_Name("ramp_start","width"), ATCIcaoWidth, width_F),
	airlines(this,"Airlines",					SQL_Name("",""), XML_Name("ramp_start","airlines"),"")
{
}

WED_RampPosition::~WED_RampPosition()
{
}

void	WED_RampPosition::Import(const AptGate_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(gis_Geo,x.location);
	SetHeading(x.heading);
	SetName(x.name);
	SetAirlines(x.airlines);
	
	ramp_type			= ENUM_Import(ATCRampType,		x.type	);
	if(ramp_type == -1)
	{
		print_func(ref,"Illegal ramp type: %d\n",x.type);
		ramp_type = atc_Ramp_Misc;
	}
	width = ENUM_Import(ATCIcaoWidth, x.width);
	if(width == -1)
	{
		print_func(ref,"Illegal ramp size: %d\n",x.type);
		ramp_type = width_F;
	}
	ENUM_ImportSet(equip_type.domain,x.equipment,equip_type.value);
}

void	WED_RampPosition::Export(		 AptGate_t& x) const
{
	GetLocation(gis_Geo,x.location);
	x.heading = GetHeading();
	GetName(x.name);
	x.type = ENUM_Export(ramp_type.value);
	x.equipment = ENUM_ExportSet(equip_type.value);
	x.width = ENUM_Export(width.value);
	x.airlines = airlines.value;

}

void	WED_RampPosition::SetType(int	rt)
{
	ramp_type = rt;
}

void	WED_RampPosition::SetEquipment(const set<int>&	et)
{
	equip_type = et;
}

void	WED_RampPosition::SetWidth(int		w)
{
	width = w;
}

void	WED_RampPosition::SetAirlines(const string &a)
{
	airlines = a;
}