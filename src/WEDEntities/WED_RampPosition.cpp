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
#include "GISUtils.h"

DEFINE_PERSISTENT(WED_RampPosition)
TRIVIAL_COPY(WED_RampPosition, WED_GISPoint_Heading)

WED_RampPosition::WED_RampPosition(WED_Archive * a, int i) : WED_GISPoint_Heading(a,i),
	ramp_type	(this,PROP_Name("Ramp Start Type",     XML_Name("ramp_start","type"   )), ATCRampType, atc_Ramp_Misc),
	equip_type	(this,PROP_Name("Equipment Type",      XML_Name("ramp_start","traffic")), ATCTrafficType, 0),
	width		(this,PROP_Name("Size",                XML_Name("ramp_start","width")), ATCIcaoWidth, width_C),
	ramp_op_type(this,PROP_Name("Ramp Operation Type", XML_Name("ramp_start","ramp_op_type")), RampOperationType, ramp_operation_None),
	airlines	(this,PROP_Name("Airlines",            XML_Name("ramp_start","airlines")),"")
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
	ramp_op_type = ENUM_Import(RampOperationType, x.ramp_op_type);
	if(ramp_op_type == -1)
	{
		print_func(ref,"Illegal oerations tye: %d\n", x.ramp_op_type);
		ramp_op_type = ramp_operation_None;
	}
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
		ramp_type = width_E;
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
	x.ramp_op_type = ENUM_Export(ramp_op_type.value);
	x.airlines = WED_RampPosition::CorrectAirlinesString(airlines.value);
}

Bbox3	WED_RampPosition::GetVisibleBounds() const
{
	Bbox2 bbox2;
	Point2 tips[4];
	GetTips(tips);

	for (const auto& tip : tips)
		bbox2 += tip;

	Bbox3 bbox3(bbox2);
	bbox3.p2.z = GetMaxHeight();

	return bbox3;
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

void	WED_RampPosition::SetRampOperationType(int ait)
{
	ramp_op_type = ait;
}

int 	WED_RampPosition::GetRampOperationType() const
{
	return ramp_op_type;
}


static bool two_adjacent_spaces(char lhs, char rhs)
{
	return (lhs == rhs) && (lhs == ' ');
}

string	WED_RampPosition::CorrectAirlinesString(const string &a)
{
	string cleaned_airlines_str = "";
	for(string::const_iterator itr = a.begin(); itr != a.end(); ++itr)
	{
		//Make lowercase
		cleaned_airlines_str += static_cast<char>(tolower(static_cast<unsigned char>(*itr)));
	}

	//Thanks Plamen for this concise trim http://stackoverflow.com/a/22711818
	//Ben says: except - the stack overflow answer is WRONG - missing a check for
	//the empty string case.
	while(!cleaned_airlines_str.empty() && isspace(*cleaned_airlines_str.begin()))
	{
		cleaned_airlines_str.erase(cleaned_airlines_str.begin());
	}

	while(!cleaned_airlines_str.empty() && isspace(*cleaned_airlines_str.rbegin()))
	{
		cleaned_airlines_str.erase(cleaned_airlines_str.length()-1);
	}
	
	cleaned_airlines_str.erase(std::unique(cleaned_airlines_str.begin(), cleaned_airlines_str.end(), two_adjacent_spaces), cleaned_airlines_str.end());   

	return cleaned_airlines_str;
}

void	WED_RampPosition::SetAirlines(const string &a)
{
	airlines = a;
}

string  WED_RampPosition::GetAirlines() const
{
	return airlines.value;
}

int	WED_RampPosition::GetWidth() const
{
	return width.value;
}

void WED_RampPosition::GetTips(Point2 c[4]) const
{
	double fuse_len, nose_offset, wingspan, wing_offset;
	switch (width.value)
	{
			case width_A: fuse_len = 11.0; wingspan = 14.0; nose_offset = 1.0; wing_offset =  -4.5; break;
			case width_B: fuse_len = 28.0; wingspan = 27.0; nose_offset = 2.7; wing_offset = -15.0; break;
			case width_C: fuse_len = 43.0; wingspan = 41.0; nose_offset = 4.7; wing_offset = -23.0; break;
			case width_D: fuse_len = 56.0; wingspan = 56.0; nose_offset = 9.5; wing_offset = -38.0; break;
			case width_E: fuse_len = 72.0; wingspan = 72.0; nose_offset = 8.2; wing_offset = -45.0; break;
			case width_F: fuse_len = 80.0; wingspan = 80.0; nose_offset = 8.8; wing_offset = -50.0; break;
	}

	Point2 nosewheel_loc;
	GetLocation(gis_Geo, nosewheel_loc);
	
	Vector2 nose_dir(0,1);
	nose_dir.rotate_by_degrees(-GetHeading());
	Vector2 right_dir(nose_dir.perpendicular_cw());

	if(ramp_type.value == atc_Ramp_Misc)
		c[0] = Point2(0,0) + nose_dir * fuse_len / 2.0;     // tip of nose
	else
		c[0] = Point2(0,0) + nose_dir * nose_offset;
		
	c[1] = c[0] + right_dir * wingspan / 2.0 + nose_dir * wing_offset;
	c[2] = c[0] - nose_dir * fuse_len;
	c[3] = c[0] - right_dir * wingspan / 2.0 + nose_dir * wing_offset;

	MetersToLLE(nosewheel_loc, 4, c);
}

double	WED_RampPosition::GetMaxHeight() const
{
	switch (width.value)
	{
	case width_A: return 6.1;
	case width_B: return 9.1;
	case width_C: return 13.7;
	case width_D: return 18.3;
	case width_E: return 20.1;
	case width_F: return 24.4;
	default: return 0.0;
	}
}

int		WED_RampPosition::GetType() const
{
	return ramp_type.value;
}

void		WED_RampPosition::GetEquipment(set<int>& out_eq) const
{
	out_eq = equip_type.value;
}
