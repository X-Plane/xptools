/* 
 * Copyright (c) 2011, Laminar Research.
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

#include "WED_ATCWindRule.h"
#include "AptDefs.h"

#if AIRPORT_ROUTING

DEFINE_PERSISTENT(WED_ATCWindRule)
TRIVIAL_COPY(WED_ATCWindRule, WED_Thing)


WED_ATCWindRule::WED_ATCWindRule(WED_Archive * a, int id) : WED_Thing(a,id),
	icao		(this,PROP_Name("METAR ICAO",		XML_Name("atc_windrule","weather_icao")),	""),
	heading_lo	(this,PROP_Name("Direction From",	XML_Name("atc_windrule","dir_lo_degs_mag")),0,3),
	heading_hi	(this,PROP_Name("Direction To",		XML_Name("atc_windrule","dir_hi_degs_mag")),0,3),
	speed_knots	(this,PROP_Name("Maximum Speed",	XML_Name("atc_windrule","speed_knots")),0,3,0, "kn")
{
}

WED_ATCWindRule::~WED_ATCWindRule()
{
}

void		WED_ATCWindRule::Import(const AptWindRule_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	icao = info.icao;
	heading_lo = info.dir_lo_degs_mag;
	heading_hi = info.dir_hi_degs_mag;
	speed_knots = info.max_speed_knots;
	PropEditCallback(0);           // give it a meaningfull name on Gateway/apt.dat import (it has no name in that data, anyways)
}

void		WED_ATCWindRule::Export(		 AptWindRule_t& info) const
{
	info.icao = icao.value;
	info.dir_lo_degs_mag = heading_lo.value;
	info.dir_hi_degs_mag = heading_hi.value;
	info.max_speed_knots = speed_knots.value;
}

void		WED_ATCWindRule::SetICAO(const string &t)
{ 
	icao.value = t; 
}

void		WED_ATCWindRule::PropEditCallback(int before)
{
	if (before)
		StateChanged(wed_Change_Properties);
	else
	{
		char buf[20];
		snprintf(buf,20,"Wind %.0lf@%d-%d",speed_knots.value,heading_lo.value,heading_hi.value);
		string old_name;
		GetName(old_name);
		if (old_name != buf)     // Prevent infinite recursion by calling SetName() if name actually changes
			SetName(buf);
	}
}
#endif
